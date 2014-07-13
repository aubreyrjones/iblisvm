/* 
 * File:   AsmErrors.h
 * Author: netzapper
 *
 * Created on July 12, 2014, 7:40 PM
 */

#ifndef ASMERRORS_H
#define	ASMERRORS_H

class ASMException : public std::exception
{
public:
	const char* err;

	ASMException(const char* err) : err(err) {}
	
	virtual const char* what() const noexcept
	{
		return err;
	}
};


class ParseException : public ASMException
{
public:
	ParseException(const char* err) : ASMException(err) {}
};

class EncodeException : public ASMException
{
public:
	EncodeException(const char* err) : ASMException(err) {}
};

class ArgumentException : public ASMException
{
public:
	ArgumentException(const char* err) : ASMException(err) {}
};

class LabelConflict : public ASMException
{
public:
	LabelConflict(const char* err) : ASMException(err) {}
};

class UnknownLabel : public ASMException
{
public:
	UnknownLabel(const char* err) : ASMException(err) {}
};

#endif	/* ASMERRORS_H */

