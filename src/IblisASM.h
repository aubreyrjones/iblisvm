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

#ifndef IBLISASM_H
#define	IBLISASM_H

namespace iblis {

class ParseException : public std::exception
{
	
};

/**
 * Create a new assembler object.
 * 
 * This will automatically parse the assembly source given.
 */
class Assembler {
	/**
	 * The AST for the program.
	 */
	ast::Program parsedProgram;
	
	/**
	 * All labels contained in this program.
	 */
	std::map<std::string, Word> labelAddress;
	
public:
	Assembler(std::string& source);
		
	const ast::Program GetProgram() const
	{
		return parsedProgram;
	}
};

} //namespace iblis

#endif	/* IBLISASM_H */

