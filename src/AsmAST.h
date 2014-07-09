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


enum class Directive {
	LOCATE,
	DEF
};

template <typename OUT>
inline OUT& operator<<(OUT& out, const Directive& d){
	return out << "directive";
}

//uninitialized member
struct nil {}; //nil voodoo?

template <typename OUT>
inline OUT& operator<<(OUT& out, const nil& n){
	return out << "nil";
}

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
	IndexExpression indexExpr;
};

/**
 * An argument to an operation or directive.
 */
typedef ::boost::variant<nil, RegisterReference, IndexExpression> Argument;

/**
 * Either an operation, or a directive.
 */
typedef ::boost::variant<nil, ::iblis::Op, Directive> PseudoOp;

/**
 * A complete instruction.
 */
struct Instruction {
	::boost::optional<Label> label;
	PseudoOp op;
	std::list<Argument> args;
};

typedef std::vector<Instruction> Program;

}} //namespace iblis::ast



BOOST_FUSION_ADAPT_STRUCT(
		::iblis::ast::RegisterReference,
		(::iblis::ast::IndexExpression, indexExpr)
		)

BOOST_FUSION_ADAPT_STRUCT(
		iblis::ast::Instruction,
		(::boost::optional<::iblis::ast::Label>, label)
		(::iblis::ast::PseudoOp, op)
		(std::list<::iblis::ast::Argument>, args)
		)

#endif	/* ASMAST_H */

