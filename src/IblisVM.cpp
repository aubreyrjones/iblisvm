/* 
 * File:   IblisVM.cpp
 * Author: netzapper
 * 
 * Created on June 29, 2014, 10:08 AM
 */

#include "IblisVM.h"

using namespace iblis;

Word& Thread::ip()
{
	return registers[0];
}

Segment& Thread::m() 
{
	return vm->GetSegment(segmentIndex);
}

Word& Thread::m(Word addr)
{
	return vm->GetSegment(segmentIndex)[addr];
}

RegisterFile& Thread::r() 
{
	return registers;
}

Word& Thread::r(Word addr)
{
	return registers[addr];
}

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
	
	ThreadP newThread(new Thread(this, segment, address));
	
	threads.push_back(newThread);
}

void IblisVM::KillThread(ThreadP t)
{
	t->state = ThreadState::HALTED;
	for (std::vector<ThreadP>::iterator it = threads.begin(); it != threads.end(); it++){
		if (t == *it){
			threads.erase(it);
		}
	}
}

void IblisVM::DecodeAndExecute(ThreadP t)
{
	Word instr = t->m()[t->ip()];
	
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
	case Op::PUSH:
		Push(t, instr);
		break;
	case Op::POP:
		Pop(t, instr);
		break;
	case Op::ADD:
	case Op::SUB:
	case Op::MUL:
	case Op::DIV:
	case Op::MOD:
	case Op::SHL:
	case Op::SHR:
	case Op::AND:
	case Op::OR:
	case Op::XOR:
	case Op::CEQ:
	case Op::CL:
	case Op::CLE:
	case Op::CG:
	case Op::CGE:
		Arithmetic(t, instr);
		break;
	case Op::NOT:
		Not(t, instr);
		break;
	case Op::JUMP:
		Jump(t, instr);
		break;
	case Op::JUMP_TRUE:
		JumpTrue(t, instr);
		break;
	case Op::CALL:
		Call(t, instr);
		break;
	case Op::FORK:
		Fork(t, instr);
		break;
	}
	
	if (op != Op::JUMP && 
		op != Op::JUMP_TRUE && 
		op != Op::CALL){
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
	
	ThreadP t = threads[curThread];
	
	try {
		DecodeAndExecute(t);
	}
	catch (ExecutionException& ex) {
		KillThread(t);
	}
	
	return threads.size() > 0;
}

//=====================INSTRUCTION IMPLEMENTATIONS====================

void IblisVM::Load(ThreadP t, Word instr)
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

void IblisVM::Store(ThreadP t, Word instr)
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

void IblisVM::Copy(ThreadP t, Word instr)
{
	t->r()[RegB(instr)] = t->r()[RegC(instr)];
}

void IblisVM::push_impl(ThreadP t, Word stackRegister, Word value)
{
	t->r(stackRegister)--;
	t->m(t->r(stackRegister)) = value;
}

Word IblisVM::pop_impl(ThreadP t, Word stackRegister)
{
	return t->m(t->r(stackRegister));
	t->r(stackRegister)++;
}

void IblisVM::Push(ThreadP t, Word instr)
{
	Word b = RegB(instr);
	Word c = RegC(instr);
	
	push_impl(t, c, t->r(b));
}

void IblisVM::Pop(ThreadP t, Word instr)
{
	Word b = RegB(instr);
	Word c = RegC(instr);

	t->r(b) = pop_impl(t, c);
}

void IblisVM::Const(ThreadP t, Word instr)
{
	Word value = ConstLiteral18(instr);
	t->r()[RegC(instr)] = value;
}

void IblisVM::Arithmetic(ThreadP t, Word instr)
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
	case Op::SHL:
		result = a_arg << b_arg;
		break;
	case Op::SHR:
		result = a_arg >> b_arg;
		break;
	case Op::AND:
		result = a_arg & b_arg;
		break;
	case Op::OR:
		result = a_arg | b_arg;
		break;
	case Op::XOR:
		result = a_arg ^ b_arg;
		break;
	case Op::CEQ:
		result = (a_arg == b_arg) ? 1 : 0;
		break;
	case Op::CL:
		result = (a_arg < b_arg) ? 1 : 0;
		break;
	case Op::CLE:
		result = (a_arg <= b_arg) ? 1 : 0;
		break;
	case Op::CG:
		result = (a_arg > b_arg) ? 1 : 0;
		break;
	case Op::CGE:
		result = (a_arg >= b_arg) ? 1 : 0;
		break;
	default:
		throw IllegalOp();
	}
	
	Word res = *reinterpret_cast<Word*>(&result);
	
	t->r()[RegC(instr)] = res;
}

void IblisVM::Not(ThreadP t, Word instr)
{
	Word b;
	
	if (LiteralB(instr)){
		b = SignExtend8(RegB(instr));
	}
	else {
		b = t->r()[RegB(instr)];
	}
	
	t->r()[RegC(instr)] = ~b;
}

void IblisVM::Jump(ThreadP t, Word instr)
{
	if (IndirectMode(instr)){
		t->ip() = t->r()[RegC(instr)];
	}
	else {
		t->ip() = Addr(instr);
	}
}

void IblisVM::JumpTrue(ThreadP t, Word instr)
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

void IblisVM::Call(ThreadP t, Word instr)
{
	Word jumpAddr;
	
	if (IndirectMode(instr)){
		jumpAddr = t->r(RegB(instr));
	}
	else {
		jumpAddr = Addr(instr);
	}
	
	push_impl(t, RegC(instr), t->ip());
	t->ip() = jumpAddr;
}

void IblisVM::Fork(ThreadP t, Word instr)
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