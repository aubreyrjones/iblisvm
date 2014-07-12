/* 
 * File:   ISA.h
 * Author: netzapper
 *
 * Created on June 29, 2014, 12:00 PM
 */

#include <cstdint>

#ifndef ISA_H
#define	ISA_H

//TYPE A - op : mode : address : reg
//[5:1:18:8]
//TYPE B - op : mode : <> : reg : reg
//[5:1:<10>:8:8]
//TYPE C - op : mode : mode : <> : reg : reg : reg
//[5:1:1:<1>:8:8:8]

#define IBLIS_OP_MASK 0xf8000000
#define IBLIS_LS_MODE_BIT 0x4000000
#define IBLIS_LS_ADDR_MASK 0x3ffff00
#define IBLIS_REG_A 0xff0000
#define IBLIS_REG_B 0xff00
#define IBLIS_REG_C 0xff
#define IBLIS_LIT_A_BIT 0x4000000
#define IBLIS_LIT_B_BIT 0x2000000


namespace iblis
{
/**
 * Basic word size is 32 bits.
 */
typedef std::uint32_t Word;

/**
 * Operations supported by IBLIS.
 */
enum class Op : Word
{
	NOP,
	LOAD,
	LOAD_PEER,
	STORE,
	STORE_PEER,
	PUSH,
	POP,
	COPY,
	CONST,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	CEQ,
	CL,
	CLE,
	CG,
	CGE,
	JUMP,
	JUMP_TRUE,
	CALL,
	FORK,
	ILLEGAL = 32
};


template <typename OUT>
inline OUT& operator<<(OUT& out, Op op){
	switch (op){
		case Op::NOP:			return out << "NOP";
		case Op::LOAD:			return out << "LOAD";
		case Op::LOAD_PEER:		return out << "LOADP";
		case Op::STORE:			return out << "STORE";
		case Op::STORE_PEER:	return out << "STOREP";
		case Op::PUSH:			return out << "PUSH";
		case Op::POP:			return out << "POP";
		case Op::COPY:			return out << "COPY";
		case Op::CONST:			return out << "CONST";
		case Op::ADD:			return out << "ADD";
		case Op::SUB:			return out << "SUB";
		case Op::MUL:			return out << "MUL";
		case Op::DIV:			return out << "DIV";
		case Op::MOD:			return out << "MOD";
		case Op::CEQ:			return out << "CEQ";
		case Op::CL:			return out << "CL";
		case Op::CLE:			return out << "CLE";
		case Op::CG:			return out << "CG";
		case Op::CGE:			return out << "CGE";
		case Op::JUMP:			return out << "JUMP";
		case Op::JUMP_TRUE:		return out << "JUMPT";
		case Op::CALL:			return out << "CALL";
		case Op::FORK:			return out << "FORK";
		default:				return out << "ILLEGAL";
	}
}

enum class ExeStatus
{
	OK = 0,
	ILLEGAL_OP,
	SEGFAULT
};

inline bool OneArg(const Op& op){
	return op == Op::JUMP;
}

inline bool ThreeArgs(const Op& op){
	switch (op){
		case Op::ADD:
		case Op::SUB:
		case Op::MUL:
		case Op::DIV:
		case Op::MOD:
		case Op::CEQ:
		case Op::CL:
		case Op::CLE:
		case Op::CG:
		case Op::CGE:
			return true;
	}
	
	return false;
}

/**
 * Decode an operation from a word.
 */
inline Op DecodeOp(const Word& i)
{
	Word op = i & IBLIS_OP_MASK;
	return static_cast<Op>(op >> 26);
}

/**
 * Encode an operation.
 */
inline Word EncodeOp(const Op& op){
	return static_cast<Word>(op) << 26;
}

/**
 * Return true if the given instruction has the
 * indirect mode bit set true.
 */
inline bool IndirectMode(const Word& i)
{
	return i & IBLIS_LS_MODE_BIT;
}

/**
 * Is the A argument a literal?
 * @param i
 * @return 
 */
inline bool LiteralA(const Word& i)
{
	return i & IBLIS_LIT_A_BIT;
}

/**
 * Is the B argument a literal?
 * @param i
 * @return 
 */
inline bool LiteralB(const Word& i)
{
	return i & IBLIS_LIT_B_BIT;
}

/**
 * Get LOAD/STORE address.
 */
inline Word Addr(const Word& i)
{
	return (i & IBLIS_LS_ADDR_MASK) >> 8;
}

/**
 * Encode an 18-bit address/literal.
 */
inline Word EncodeAddr(const Word& addr){
	return (addr << 8) & IBLIS_LS_ADDR_MASK;
}

/**
 * Sign extend the 18-bit literal in a CONST
 * instruction.
 */
inline Word ConstLiteral18(const Word& i)
{
	Word l = Addr(i);
	if (l & 0x20000){ //it's a negative number.
		l = l | 0xfffc0000;
	}
	
	return l;
}

inline Word SignExtend8(const Word& i)
{
	if (i & 0x80){
		return i | 0xffffff00;
	}
	
	return i;
}


inline Word RegA(const Word& i)
{
	return (i & IBLIS_REG_A) >> 16;
}

inline Word RegB(const Word& i)
{
	return (i & IBLIS_REG_B) >> 8;
}

inline Word RegC(const Word& i)
{
	return i & IBLIS_REG_C;
}

inline Word EncodeA(const Word& a)
{
	return (a & 0xff) << 16;
}

inline Word EncodeB(const Word& b){
	return (b & 0xff) << 8;
}

inline Word EncodeC(const Word& c){
	return c & 0xff;
}

}


#endif	/* ISA_H */

