/* 
 * File:   IblisASM.cpp
 * Author: netzapper
 * 
 * Created on July 3, 2014, 10:41 PM
 */
#include <iostream>


#include "ISA.h"
#include "IblisASM.h"
#include "AsmGrammar.h"

namespace iblis {

//======================== ASSEMBLER ================================


Assembler::Assembler(std::string& source) :
	parsedProgram(),
	labelAddress(),
	ip(0),
	expr_eval(this)
{
	AsmGrammar<std::string::const_iterator, AsmSkipper<std::string::const_iterator> > grammar;
	std::string::const_iterator iter = source.begin();
    std::string::const_iterator end = source.end();
		
	bool parsed = qi::phrase_parse(iter, end, grammar, AsmSkipper<std::string::const_iterator>(), parsedProgram);
	
	if (!parsed || iter != end){
		
		std::string rest(iter, end);
		std::cout << "\n-------------------------\n";
		std::cout << "Parsing failed\n";
		std::cout << rest << "\n";
		std::cout << "-------------------------\n";
        
		
		throw ParseException("error");
	}
	
	std::cout << parsedProgram.size() << " instructions.\n";
}

void Assembler::EncodeOperation(ast::Instruction& instr)
{
	if (instr.op.type() == typeid(ast::Directive) ){
		return;
	}
	
	Op op = boost::get<Op>(instr.op);
	instr.encodedOp = EncodeOp(op);
	
	if (OneArg(op)){ //we're a jump
		if (instr.ArgB()){ //exactly one argument
			throw ArgumentException("Only one argument expected.");
		}
		
		if (ast::IsRegister(instr.ArgC())){
			instr.encodedOp |= IBLIS_LS_MODE_BIT;
		}
	}
	else if (ThreeArgs(op)){
		if (!instr.ArgA() || !instr.ArgB()){
			throw ArgumentException("Three arguments expected.");
		}
				
		if (!ast::IsRegister(instr.ArgC())){
			throw EncodeException("C must be a register for arithmetic/compare operations.");
		}
		
		if (!ast::IsRegister(instr.ArgA().get())){
			instr.encodedOp |= IBLIS_LIT_A_BIT;
		}
		
		if (!ast::IsRegister(instr.ArgB().get())){
			instr.encodedOp |= IBLIS_LIT_B_BIT;
		}
	}
	else {
		if (!instr.ArgB() || instr.ArgA()){ //exactly two arguments
			throw ArgumentException("Two arguments expected, got three.");
		}
		
		if (ast::IsRegister(instr.ArgB().get())){
			if (op == Op::CONST){
				throw ArgumentException("Argument B for CONST cannot be a register reference.");
			}
			instr.encodedOp |= IBLIS_LS_MODE_BIT;
		}
		else {
			if (op == Op::PUSH || op == Op::POP || op == Op::COPY){
				throw EncodeException("Argument B for PUSH/POP/COPY must be a register.");
			}
		}
	}
}

Word Assembler::EvaluateExpression(ast::IndexExpression& expr)
{
	return boost::apply_visitor(expr_eval, expr);
}

Word Assembler::EvaluateArgument(ast::Argument& arg)
{
	if (arg.type() == typeid(ast::RegisterReference)){
		return EvaluateExpression(boost::get<ast::RegisterReference>(arg).indexExpr);
	}
	return EvaluateExpression(boost::get<ast::IndexExpression>(arg));
}

void Assembler::RegisterLabel(const ast::Label& name, const Word& address)
{
	if (labelAddress.find(name) != labelAddress.end()){
		throw LabelConflict("conflict");
	}
	
	labelAddress.insert(LabelMap::value_type(name, address));
}

void Assembler::ExecuteDirective(ast::Instruction& instr)
{
	ast::Directive dir = boost::get<ast::Directive>(instr.op);
	
	if (dir == ast::Directive::DEF){
		if (!instr.ArgB()){
			throw ArgumentException(".def <name>, <literal>");
		}
		
		if (instr.ArgC().type() == typeid(ast::RegisterReference) ||
			instr.ArgB().get().type() == typeid(ast::RegisterReference)){
			throw ArgumentException(".def cannot assign registers.");
		}
		
		ast::IndexExpression& bExpr = boost::get<ast::IndexExpression>(instr.ArgB().get());
		
		if (!instr.ArgB() || bExpr.type() != typeid(ast::Label)){
			throw ArgumentException(".def <name>, <literal>");
		}
		
		RegisterLabel(boost::get<ast::Label>(bExpr), 
					  EvaluateExpression(boost::get<ast::IndexExpression>(instr.ArgC())));
		
	}
	else if (dir == ast::Directive::LOCATE) {
		if (instr.ArgC().type() == typeid(ast::RegisterReference)){
			throw ArgumentException(".locate <literal>");
		}
		
		ip = EvaluateExpression(boost::get<ast::IndexExpression>(instr.ArgC()));
	}
	else if (dir == ast::Directive::DATA) {
		
		ip++;
	}
}

void Assembler::ScanAndFix()
{
	ip = 0;
	for (ast::Instruction& instr : parsedProgram){		
		if (instr.label){
			RegisterLabel(instr.label.get(), ip);
		}
		
		instr.address = ip;
		
		if (instr.op.type() == typeid(ast::Directive)){
			ExecuteDirective(instr);
			continue;
		}
		
		EncodeOperation(instr);
		
		ip++;
	}
}

void Assembler::ResolveArguments()
{
	for (ast::Instruction& instr : parsedProgram){
		if (instr.op.type() != typeid(Op)){
			ast::Directive dir = boost::get<ast::Directive>(instr.op);
			if (dir == ast::Directive::DATA){
				instr.encodedInstruction = EvaluateArgument(instr.ArgC());
			}
			continue;
		}
		
		ast::OptionalArg a(instr.ArgA());
		ast::OptionalArg b(instr.ArgB());
		ast::Argument& c = instr.ArgC();
		
		Op op = boost::get<Op>(instr.op);
		
		if (op == Op::JUMP){ //only op with optional ArgC.
			if (ast::IsRegister(c)){
				instr.encodedInstruction = 
					instr.encodedOp | 
					EncodeC(EvaluateArgument(c));
			}
			else {
				instr.encodedInstruction =
					instr.encodedOp |
					EncodeAddr(EvaluateArgument(c));
			}
		}
		else if (ThreeArgs(op)){
			if (!a || !b){
				throw ArgumentException("Expected three arguments.");
			}
			
			if (!ast::IsRegister(c)){
				throw ArgumentException("C must be a register.");
			}
			
			instr.encodedInstruction = instr.encodedOp |
				EncodeA(EvaluateArgument(a.get())) |
				EncodeB(EvaluateArgument(b.get())) |
				EncodeC(EvaluateArgument(c));
		}
		else {
			if (!b){
				throw ArgumentException("Expected two arguments, got one.");
			}
			if (a){
				throw ArgumentException("Expected two arguments, got three.");
			}
			
			if (ast::IsRegister(b.get())){
				instr.encodedInstruction = instr.encodedOp |
					EncodeB(EvaluateArgument(b.get())) |
					EncodeC(EvaluateArgument(c));
			}
			else {
				instr.encodedInstruction = instr.encodedOp |
					EncodeAddr(EvaluateArgument(b.get())) |
					EncodeC(EvaluateArgument(c));
			}
		}
	}
}

void Assembler::Assemble()
{
	ScanAndFix();
	ResolveArguments();
}

//================
ExpressionEvaluator::ExpressionEvaluator(Assembler* as) : as(as) {}

ExpressionEvaluator::result_type ExpressionEvaluator::operator()(ast::nil& nil){
	return 0;
}

ExpressionEvaluator::result_type ExpressionEvaluator::operator()(int& i){
	return *(reinterpret_cast<Word*>(&i));
}

ExpressionEvaluator::result_type ExpressionEvaluator::operator()(unsigned int& i){
	return i;
}

ExpressionEvaluator::result_type ExpressionEvaluator::operator()(ast::Label& label){
	if (as->labelAddress.find(label) == as->labelAddress.end()){
		throw UnknownLabel("unknown");
	}
	
	return as->labelAddress[label];
}
//================



} //namespace iblis