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
	FORK,
	ILLEGAL = 32
};

enum class ExeStatus
{
	OK = 0,
	ILLEGAL_OP,
	SEGFAULT
};

/**
 * Decode an operation from a word.
 */
inline Op DecodeOp(const Word& i)
{
	Word op = i & IBLIS_OP_MASK;
	return static_cast<Op>(op >> 26);
}

/**
 * Return true if the given instruction has the
 * indirect mode bit set true.
 */
inline bool IndirectMode(const Word& i)
{
	return i & IBLIS_LS_MODE_BIT;
}

inline bool LiteralA(const Word& i)
{
	return i & IBLIS_LIT_A_BIT;
}

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

}


#endif	/* ISA_H */

