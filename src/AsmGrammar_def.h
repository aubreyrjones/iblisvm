/* 
 * File:   AsmGrammer_def.h
 * Author: netzapper
 *
 * Created on July 12, 2014, 7:47 PM
 */

#ifndef ASMGRAMMAR_DEF_H
#define	ASMGRAMMAR_DEF_H

#include "AsmGrammar.h"

namespace iblis{
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename Iterator>
AsmSkipper<Iterator>::AsmSkipper() : AsmSkipper::base_type(start)
{
	using boost::spirit::eol;
	using boost::spirit::eoi;
	
	start = ascii::blank | 
			(";" >> *(ascii::char_ - eol) >> &(eol | eoi));
	
}

template <typename Iterator, typename SkipType>
AsmGrammar<Iterator, SkipType>::AsmGrammar() : AsmGrammar::base_type(program, "asm_program")
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
	(".def", ast::Directive::DEF)
	(".data", ast::Directive::DATA);
	
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
}


#endif	/* ASMGRAMMAR_DEF_H */

