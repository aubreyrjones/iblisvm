/* 
 * File:   IblisASM.h
 * Author: netzapper
 *
 * Created on July 3, 2014, 10:41 PM
 */

#include <string>
#include <map>
#include <exception>

#include "AsmAST.h"
#include "AsmErrors.h"

#ifndef IBLISASM_H
#define	IBLISASM_H

namespace iblis {


class Assembler; //forward declaration
/** Used to visit the boost::variant expressions. */
class ExpressionEvaluator : public boost::static_visitor<>
{
	Assembler* as;
public:
	typedef Word result_type;
	
	ExpressionEvaluator(Assembler* as);
	
	result_type operator()(ast::nil& nil);
	
	result_type operator()(int& i);
	
	result_type operator()(unsigned int& i);
	
	result_type operator()(ast::Label& label);
};

typedef std::map<ast::Label, Word> LabelMap;

/**
 * Create a new assembler object.
 * 
 * This will automatically parse the assembly source given.
 */
class Assembler {
	friend class ExpressionEvaluator;
protected:
	/**
	 * The AST for the program.
	 */
	ast::Program parsedProgram;
	
	/**
	 * All labels contained in this program.
	 */
	LabelMap labelAddress;
	
	/**
	 * Instruction pointer for assembling.
	 */
	Word ip;
	
	ExpressionEvaluator expr_eval;
	
	/**
	 * Evaluate the given expression.
     */
	Word EvaluateExpression(ast::IndexExpression& expr);
	
	Word EvaluateArgument(ast::Argument& arg);
	
	/**
	 * Encode the given instruction.
     */
	void EncodeOperation(ast::Instruction& instr);
	
	/** Register a new label, raising an exception if already defined. */
	void RegisterLabel(const ast::Label& name, const Word& address);
	
	/** Immediately execute the given directive. */
	void ExecuteDirective(ast::Instruction& instr);
	
	/**
	 * Scans all instructions for labels, evaluates directives,
	 * and fixes instructions in place.
     */
	void ScanAndFix();
	
	/**
	 * Resolves the arguments for each instruction.
	 */
	void ResolveArguments();
	
public:
	Assembler(std::string& source);
		
	const ast::Program& GetProgram() const
	{
		return parsedProgram;
	}
	
	void Assemble();
};


} //namespace iblis

#endif	/* IBLISASM_H */

