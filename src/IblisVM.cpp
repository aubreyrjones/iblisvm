/* 
 * File:   IblisVM.cpp
 * Author: netzapper
 * 
 * Created on June 29, 2014, 10:08 AM
 */

#include "IblisVM.h"

using namespace iblis;

IblisVM::IblisVM() :
	threads(IBLIS_MAX_THREADS),
	curThread(0)
{
}

IblisVM::~IblisVM()
{
}

bool IblisVM::SpawnThread(Word segment, Word address)
{
	if (threads.size() >= IBLIS_MAX_THREADS){
		return false;
	}
	
	Thread *newThread = new Thread;
	newThread->segment = &segments[segment];
	newThread->ip() = address;
	
	threads.push_back(newThread);
}

void IblisVM::DecodeAndExecute(Thread* t)
{
	Word instr = (*t->segment)[t->ip()];
	
	Op op = DecodeOp(instr);
	switch (op){
	case Op::LOAD:
		Load(t, instr);
		break;
	case Op::STORE:
		Store(t, instr);
		break;
	case Op::COPY:
		Copy(t, instr);
		break;
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
		Arithmetic(t, instr);
		break;
	case Op::JUMP:
		Jump(t, instr);
		break;
	case Op::JUMP_TRUE:
		JumpTrue(t, instr);
		break;
	case Op::FORK:
		Fork(t, instr);
		break;
	}
	
	if (op != Op::JUMP && op != Op::JUMP_TRUE){
		t->ip()++;
	}
}

bool IblisVM::ExecuteNext()
{
	if (!threads.size()){
		return false;
	}
	
	curThread++;
	if (curThread >= threads.size()){
		curThread = 0;
	}
	
	Thread *t = threads[curThread];
	
	try {
		DecodeAndExecute(t);
	}
	catch (Regfault& regv) {
	}
	catch (Segfault& segv) {
	}
	catch (IllegalOp& illOp) {
	}
	
	return true;
}

//=====================INSTRUCTION IMPLEMENTATIONS====================

void IblisVM::Load(Thread *t, Word instr)
{
	Word loadAddr;
	if (IndirectMode(instr)){
		loadAddr = t->r()[RegB(instr)];
	}
	else {
		loadAddr = Addr(instr);
	}
	
	t->r()[RegC(instr)] = t->m()[loadAddr];
}

void IblisVM::Store(Thread *t, Word instr)
{
	Word storeAddr;
	if (IndirectMode(instr)){
		storeAddr = t->r()[RegB(instr)];
	}
	else {
		storeAddr = Addr(instr);
	}
	
	t->m()[storeAddr] = t->m()[RegC(instr)];
}

void IblisVM::Copy(Thread *t, Word instr)
{
	t->r()[RegB(instr)] = t->r()[RegC(instr)];
}

void IblisVM::Const(Thread* t, Word instr)
{
	Word value = ConstLiteral18(instr);
	t->r()[RegC(instr)] = value;
}

void IblisVM::Arithmetic(Thread* t, Word instr)
{
	Word a, b;
	if (LiteralA(instr)){
		a = SignExtend8(RegA(instr));
	}
	else {
		a = t->r()[RegA(instr)];
	}
	
	if (LiteralB(instr)){
		b = SignExtend8(RegB(instr));
	}
	else {
		b = t->r()[RegB(instr)];
	}
	
	int a_arg, b_arg, result;
	a_arg = *reinterpret_cast<std::int32_t*>(&a);
	b_arg = *reinterpret_cast<std::int32_t*>(&b);
	
	switch (DecodeOp(instr)){
	case Op::ADD:
		result = a_arg * b_arg;
		break;
	case Op::SUB:
		result = a_arg - b_arg;
		break;
	case Op::MUL:
		result = a_arg * b_arg;
		break;
	case Op::DIV:
		result = a_arg / b_arg;
		break;
	case Op::MOD:
		result = a_arg % b_arg;
		break;
	case Op::CEQ:
		result = (a_arg == b_arg) ? 1 : 0;
	case Op::CL:
		result = (a_arg < b_arg) ? 1 : 0;
	case Op::CLE:
		result = (a_arg <= b_arg) ? 1 : 0;
	case Op::CG:
		result = (a_arg > b_arg) ? 1 : 0;
	case Op::CGE:
		result = (a_arg >= b_arg) ? 1 : 0;
	default:
		throw IllegalOp();
	}
	
	Word res = *reinterpret_cast<Word*>(&result);
	
	t->r()[RegC(instr)] = res;
}

void IblisVM::Jump(Thread* t, Word instr)
{
	if (IndirectMode(instr)){
		t->ip() = t->r()[RegC(instr)];
	}
	else {
		t->ip() = Addr(instr);
	}
}

void IblisVM::JumpTrue(Thread* t, Word instr)
{
	Word jumpAddr;
	
	if (IndirectMode(instr)){
		jumpAddr = t->r()[RegB(instr)];
	}
	else {
		jumpAddr = Addr(instr);
	}
	
	if (t->r()[RegC(instr)]){
		t->ip() = jumpAddr;
	}
	else {
		t->ip()++;
	}
}

void IblisVM::Fork(Thread* t, Word instr)
{
	Word segment = RegC(instr);
	
	if (segment > IBLIS_N_SEGMENTS){
		throw Segfault();
	}
	
	Word addr = Addr(instr);
	
	if (addr > IBLIS_MEM_SIZE){
		throw Segfault();
	}
	
	if (!SpawnThread(segment, addr)){
		throw ThreadFault();
	}
}