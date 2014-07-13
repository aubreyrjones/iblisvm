/* 
 * File:   AsmAST.h
 * Author: netzapper
 *
 * Created on July 8, 2014, 12:44 PM
 */


#include <string>
#include <list>
#include <vector>
#include <iostream>

#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/optional.hpp>

#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include "ISA.h"

#ifndef ASMAST_H
#define	ASMAST_H

namespace iblis { namespace ast {
//======================AST===========================

/**
 * All internal directives recognized by the assembler.
 */
enum class Directive {
	LOCATE,
	DEF,
	DATA
};

//uninitialized member
struct nil {}; //nil voodoo?


/**
 * An alphanumeric identifier for an index or address.
 */
typedef std::string Label;

/**
 * Term expressing an index in memory or register file.
 */
typedef ::boost::variant<nil, int, unsigned int, Label> IndexExpression;

/**
 * Reference to a register.
 */
struct RegisterReference {
	char r;
	IndexExpression indexExpr;
};

/**
 * An argument to an operation or directive.
 */
typedef ::boost::variant<nil, RegisterReference, IndexExpression> Argument;

inline bool IsRegister(const Argument& arg){
	return arg.type() == typeid(RegisterReference);
}

/**
 * A list of arguments to a pseudo op.
 */
typedef std::vector<Argument> ArgList;

typedef ::boost::optional<Argument&> OptionalArg;

/**
 * Either an operation, or a directive.
 */
typedef ::boost::variant<nil, Directive, ::iblis::Op> PseudoOp;

/**
 * A complete instruction.
 */
struct Instruction {
	::boost::optional<Label> label;
	PseudoOp op;
	ArgList args;
	
	/** Address of this instruction. */
	::iblis::Word address;
	
	/** The encoded, bit-shifted operation/mode portion
	 *  of the instruction. */
	::iblis::Word encodedOp;
	
	/** The complete encoded instruction, including arguments. */
	::iblis::Word encodedInstruction;
	
	/**
	 * Gets A, if it exists.
     */
	OptionalArg ArgA(){
		if (args.size() == 3){
			return OptionalArg(args.front());
		}
		return OptionalArg();
	}
	
	/**
	 * Gets B, if it exists.
     */
	OptionalArg ArgB(){
		if (args.size() == 3){
			return OptionalArg(args[1]);
		}
		else if (args.size() == 2){
			return OptionalArg(args.front());
		}
		
		return OptionalArg();
	}
	
	/**
	 * Gets C.
     */
	Argument& ArgC(){
		return args.back();
	}
};

/**
 * A list of instructions, which makes up a program.
 */
typedef std::vector<Instruction> Program;


//================ IO =========================

//============== OUTPUT OPERATORS ================
template <typename OUT>
class VariantPrinter : public boost::static_visitor<>
{
	OUT& out;
public:
	typedef OUT& result_type;
	
	VariantPrinter(OUT& out) : out(out) {}
	
	template <typename VAL_T>
	OUT& operator()(VAL_T& varVal) const {
		out << varVal;
		return out;
	}
};


// ================================================= 

//directive
template <typename OUT>
inline OUT& operator<<(OUT& out, const Directive& d){
	switch (d){
	case Directive::LOCATE:		return out << ".locate";
	case Directive::DEF:		return out << ".def";
	case Directive::DATA:		return out << ".data";
	default:					return out << ".unknown";
	}
	
}

//nil
template <typename OUT>
inline OUT& operator<<(OUT& out, const nil& n){
	return out << "nil";
}


template <typename OUT>
inline OUT& operator<<(OUT& out, const RegisterReference& r){
	return out << "r[" << r.indexExpr << "]";
}

template <typename OUT>
inline OUT& operator<<(OUT& out, const ArgList& al){
	auto it = al.begin();
	for (; it != al.end();){
		boost::apply_visitor(VariantPrinter<OUT>(out), *it);
		it++;
		if (it != al.end()){
			out << ", ";
		}
	}
	
	return out;
}

template <typename OUT>
inline OUT& operator<<(OUT& out, const Instruction& i)
{
	if (i.label.is_initialized()){
		out << i.label.get() << ":\t";
	}
	else {
		out << " \t";
	}
	
	return boost::apply_visitor(VariantPrinter<OUT>(out), i.op) << "\t" << i.args;
}


}} //namespace iblis::ast



BOOST_FUSION_ADAPT_STRUCT(
		::iblis::ast::RegisterReference,
		(char, r)
		(::iblis::ast::IndexExpression, indexExpr)
		)

BOOST_FUSION_ADAPT_STRUCT(
		iblis::ast::Instruction,
		(::boost::optional<::iblis::ast::Label>, label)
		(::iblis::ast::PseudoOp, op)
		(::iblis::ast::ArgList, args)
		)

#endif	/* ASMAST_H */

