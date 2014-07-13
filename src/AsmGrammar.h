/* 
 * File:   AsmGrammar.h
 * Author: netzapper
 *
 * Created on July 12, 2014, 7:31 PM
 */

#define BOOST_SPIRIT_QI_DEBUG

#include <exception>

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

#include "AsmAST.h"
#include "AsmErrors.h"

#ifndef ASMGRAMMAR_H
#define	ASMGRAMMAR_H

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
	AsmSkipper();
	
	qi::rule<Iterator> start;
};

template <typename Iterator, typename SkipType>
struct AsmGrammar : qi::grammar<Iterator, ast::Program(), SkipType>
{
	AsmGrammar();
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

}

#endif	/* ASMGRAMMAR_H */

