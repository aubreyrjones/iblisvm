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
	labelAddress()
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
        
		
		throw ParseException();
	}
	
	std::cout << parsedProgram.size() << " instructions.\n";
}

void Assembler::EncodeInstruction(ast::Instruction& instr)
{
	if (!instr.IsOperation()){
		return;
	}
	
	instr.encodedOp = EncodeOp(boost::get<Op>(instr.op));
}


} //namespace iblis