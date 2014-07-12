/* 
 * File:   IblisASM.cpp
 * Author: netzapper
 * 
 * Created on July 3, 2014, 10:41 PM
 */

#include "ISA.h"
#include "IblisASM.h"

#define BOOST_SPIRIT_QI_DEBUG

#include <iostream>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_char_class.hpp>
#include <boost/spirit/include/qi_symbols.hpp>

#include <boost/spirit/include/qi_repeat.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_eoi.hpp>
#include <boost/spirit/include/qi_no_case.hpp>
#include <boost/spirit/include/qi_eps.hpp>

#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_function.hpp>

namespace iblis {
//=================GRAMMAR========================
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;


struct error_handler_
{
	template <typename, typename, typename>
	struct result { typedef void type; };
	
	template <typename Iterator>
	void operator()(qi::info const& what, Iterator err_pos, Iterator last) const
	{
		if (err_pos == last){
			return;
		}
		std::cout
			<< "Error! Expecting "
			<< what                         // what failed?
			<< " here: \""
			<< std::string(err_pos, last)   // iterators to error-pos, end
			<< "\""
			<< std::endl
            ;
	}
};
boost::phoenix::function<error_handler_> const error_handler = error_handler_();

template <typename Iterator>
struct AsmSkipper : qi::grammar<Iterator>
{
	AsmSkipper() : AsmSkipper::base_type(start)
	{
		using boost::spirit::eol;
		using boost::spirit::eoi;

		start = ascii::blank | 
				(";" >> *(ascii::char_ - eol) >> &(eol | eoi));
				
	}
	
	qi::rule<Iterator> start;
};

template <typename Iterator, typename SkipType>
struct AsmGrammar : qi::grammar<Iterator, ast::Program(), SkipType>
{
	AsmGrammar() : AsmGrammar::base_type(program, "asm_program")
	{
		
		using ascii::char_;
		using ascii::alnum;
		using ascii::alpha;
		using boost::spirit::repeat;
		using boost::spirit::eol;
		using boost::spirit::eoi;
		using ascii::no_case;
		
		using boost::phoenix::val;
		
		qi::_2_type _2;
        qi::_3_type _3;
        qi::_4_type _4;
		
		opcode.add
			("nop",		Op::NOP)
			("load",	Op::LOAD)
			("loadp",	Op::LOAD_PEER)
			("store",	Op::STORE)
			("storep",	Op::STORE_PEER)
			("push",	Op::PUSH)
			("pop",		Op::POP)
			("copy",	Op::COPY)
			("const",	Op::CONST)
			("add",		Op::ADD)
			("sub",		Op::SUB)
			("mul",		Op::MUL)
			("div",		Op::DIV)
			("mod",		Op::MOD)
			("ceq",		Op::CEQ)
			("cl",		Op::CL)
			("cle",		Op::CLE)
			("cg",		Op::CG)
			("cge",		Op::CGE)
			("jump",	Op::JUMP)
			("jumpt",	Op::JUMP_TRUE)
			("call",	Op::CALL)
			("fork",	Op::FORK);
		
		directive.add
			(".locate", ast::Directive::LOCATE)
			(".def", ast::Directive::DEF);
			
		id_rule = qi::lexeme[ ( ( alpha | char_('_') ) >> *( alnum | char_('_') ) ) ];
		
		label = id_rule - pseudo_op;
		
		index_expr = label |
					 qi::lexeme[ "0x" > boost::spirit::hex] |
					 qi::lexeme[boost::spirit::int_] ;
					 
		
		reg_ref = char_('r') >> "[" > index_expr > ']';
		
		arg = reg_ref | index_expr;
		
		arg_list = arg % ',';
		
		pseudo_op = qi::lexeme[no_case[opcode | directive] >> !ascii::alpha]; //qi::omit[ascii::space]];
		
		instruction = ( -(label > ':') ) 
					  >> ( pseudo_op >> arg_list ) 
					  > (eol | eoi);
		
		program = qi::omit[ *ascii::space ] >> +(instruction >> qi::omit[ *ascii::space ]) >> qi::omit[ *ascii::space ];
		
		
		//===============error handling================
		label.name("label");
		index_expr.name("index_expr");
		reg_ref.name("reg_ref");
		arg.name("arg");
		arg_list.name("arg_list");
		pseudo_op.name("pseudo");
		instruction.name("instr");
		program.name("asm_program");
		
//		BOOST_SPIRIT_DEBUG_NODE(label);
//		BOOST_SPIRIT_DEBUG_NODE(index_expr);
//		BOOST_SPIRIT_DEBUG_NODE(reg_ref);
//		BOOST_SPIRIT_DEBUG_NODE(arg);
//		BOOST_SPIRIT_DEBUG_NODE(arg_list);
//		BOOST_SPIRIT_DEBUG_NODE(pseudo_op);
//		BOOST_SPIRIT_DEBUG_NODE(instruction);
//		BOOST_SPIRIT_DEBUG_NODE(program);
		
		qi::on_error<qi::fail>(program, error_handler(_4, _3, _2));
		qi::on_error<qi::fail>(instruction, error_handler(_4, _3, _2));
		qi::on_error<qi::fail>(pseudo_op, error_handler(_4, _3, _2));
		qi::on_error<qi::fail>(arg, error_handler(_4, _3, _2));
		qi::on_error<qi::fail>(arg_list, error_handler(_4, _3, _2));
		qi::on_error<qi::fail>(reg_ref, error_handler(_4, _3, _2));
		qi::on_error<qi::fail>(index_expr, error_handler(_4, _3, _2));
		qi::on_error<qi::fail>(label, error_handler(_4, _3, _2));
		
		
	}
	
	qi::rule<Iterator, std::string(), SkipType> id_rule;
	qi::rule<Iterator, ast::Label(), SkipType> label;
	qi::rule<Iterator, ast::IndexExpression(), SkipType> index_expr;
	qi::rule<Iterator, ast::RegisterReference(), SkipType> reg_ref;
	qi::rule<Iterator, ast::Argument(), SkipType> arg;
	qi::rule<Iterator, ast::ArgList(), SkipType> arg_list;
	qi::rule<Iterator, ast::PseudoOp(), SkipType> pseudo_op;
	qi::rule<Iterator, ast::Instruction(), SkipType> instruction;
	qi::rule<Iterator, ast::Program(), SkipType> program;
	
	
	qi::symbols<char, Op> opcode;
	qi::symbols<char, ast::Directive> directive;
};

//======================== ASSEMBLER ================================


Assembler::Assembler(std::string& source) :
	parsedProgram(),
	labelAddress(),
	ip(0),
	expr_eval(this)
{
	AsmGrammar<std::string::const_iterator, AsmSkipper<std::string::const_iterator> > grammar;
	//AsmGrammar<std::string::const_iterator, ascii::space_type> grammar;
	std::string::const_iterator iter = source.begin();
    std::string::const_iterator end = source.end();
		
	bool parsed = qi::phrase_parse(iter, end, grammar, AsmSkipper<std::string::const_iterator>(), parsedProgram);
	//bool parsed = qi::phrase_parse(iter, end, grammar, ascii::space, parsedProgram);
	
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
				throw ArgumentException("Argument B for CONST cannot be a register.");
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
	switch (boost::get<ast::Directive>(instr.op)){
	case ast::Directive::DEF:
		if (!instr.ArgB() || instr.ArgB().get().type() != typeid(ast::Label)){
			throw ArgumentException(".def <name>, <literal>");
		}
		
		if (instr.ArgC().type() == typeid(ast::RegisterReference)){
			throw ArgumentException(".def cannot assign registers.");
		}
		
		RegisterLabel(boost::get<ast::Label>(instr.ArgB().get()), 
					  EvaluateExpression(boost::get<ast::IndexExpression>(instr.ArgC())));
		
		break;
	
	case ast::Directive::LOCATE:
		if (instr.ArgC().type() == typeid(ast::RegisterReference)){
			throw ArgumentException(".locate <literal>");
		}
		
		ip = EvaluateExpression(boost::get<ast::IndexExpression>(instr.ArgC()));
		break;
	}
}

void Assembler::ScanAndFix()
{
	ip = 0;
	for (ast::Instruction& instr : parsedProgram){		
		if (instr.label){
			RegisterLabel(instr.label.get(), ip);
		}
		
		if (instr.op.type() == typeid(ast::Directive)){
			ExecuteDirective(instr);
			continue;
		}
		
		EncodeOperation(instr);
		
		instr.address = ip++;
	}
}

void Assembler::ResolveArguments()
{
	for (ast::Instruction& instr : parsedProgram){
		if (instr.op.type() != typeid(Op)){
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