/* 
 * File:   AsmErrors.h
 * Author: netzapper
 *
 * Created on July 12, 2014, 7:40 PM
 */

#include <stdexcept>
#include <string>

#ifndef ASMERRORS_H
#define	ASMERRORS_H

namespace iblis{
class ASMException : public std::runtime_error
{
public:
	ASMException(const std::string& err) : std::runtime_error(err) {}	
};

class EncodeException : public ASMException
{
public:
	std::size_t lineNumber;
	EncodeException(const std::string& err, std::size_t lineNumber) : ASMException(err), lineNumber(lineNumber) {}
};

class ArgumentException : public EncodeException
{
public:
	ArgumentException(const std::string& err, std::size_t lineNumber) : EncodeException(err, lineNumber) {}
};

class LabelConflict : public EncodeException
{
public:
	LabelConflict(const std::string& err, std::size_t lineNumber) : EncodeException(err, lineNumber) {}
};

class UnknownLabel : public EncodeException
{
public:
	UnknownLabel(const std::string& err, std::size_t lineNumber) : EncodeException(err, lineNumber) {}
};
}

#endif	/* ASMERRORS_H */

