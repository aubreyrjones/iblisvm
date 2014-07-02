/* 
 * File:   IblisVM.h
 * Author: netzapper
 *
 * Created on June 29, 2014, 10:08 AM
 */


#include <string.h>
#include <vector>
#include <exception>
#include <memory>
#include "ISA.h"

#ifndef IBLISVM_H
#define	IBLISVM_H

#define IBLIS_MEM_SIZE 1 << 18
#define IBLIS_N_SEGMENTS 7
#define IBLIS_N_REGISTERS 256
#define IBLIS_MAX_THREADS 8

namespace iblis
{

class ExecutionException : public std::exception {};
class Regfault : public ExecutionException {};
class Segfault : public ExecutionException {};
class IllegalOp : public ExecutionException {};
class ThreadFault : public ExecutionException {};


/**
 * A segment is an area of memory.
 * 
 * Each VM's memory is made up of a number of segments.
 */
struct Segment {
	Word m[IBLIS_MEM_SIZE];
	
	inline void Zero()
	{
		memset(m, 0, sizeof(Word) * IBLIS_MEM_SIZE);
	}
	
	inline Word& operator[](Word index)
	{
		if (index >= IBLIS_MEM_SIZE){
			throw Segfault();
		}
		return m[index];
	}
};

/**
 * Each thread has a list of registers.
 * 
 * r[0] is the instruction pointer.
 */
struct RegisterFile {
	Word r[IBLIS_N_REGISTERS];
	
	Word& operator[](Word index)
	{
		if (index >= IBLIS_N_REGISTERS){
			throw Regfault();
		}
		
		return r[index];
	}
};

enum class ThreadState
{
	INIT,
	RUNNING,
	HALTED
};

class IblisVM;
/**
 * A thread is the unit of execution in Iblis. It has a complete set of
 * registers, and must occupy a specific segment.
 */
struct Thread {
	RegisterFile registers;
	Word segmentIndex;
	
	IblisVM *vm;
	
	ThreadState state;
	
	Thread(IblisVM *vm, Word segment, Word address) : 
		segmentIndex(segment),
		vm(vm), 
		state(ThreadState::INIT) 
	{
		registers[0] = address;
	} 
	
	Word& ip();
	Segment& m();
	Word& m(Word addr);
	RegisterFile& r();
	Word& r(Word addr);
};

typedef std::shared_ptr<Thread> ThreadP;

//======================VM INSTANCE=====================
class IblisVM {
	friend Thread;
protected:
	Segment segments[IBLIS_N_SEGMENTS];
	std::vector<ThreadP> threads;
	
	int curThread;
	
	void push_impl(ThreadP t, Word stackRegister, Word value);
	Word pop_impl(ThreadP t, Word stackRegister);
	
private:
	void DecodeAndExecute(ThreadP thread);
	
	void Load(ThreadP t, Word instr);
	void Store(ThreadP t, Word instr);
	void Copy(ThreadP t, Word instr);
	void Push(ThreadP t, Word instr);
	void Pop(ThreadP t, Word instr);
	void Const(ThreadP t, Word instr);
	void Arithmetic(ThreadP t, Word instr);
	void Jump(ThreadP t, Word instr);
	void JumpTrue(ThreadP t, Word instr);
	void Call(ThreadP t, Word instr);
	void Fork(ThreadP t, Word instr);
		
	/**
	 * Retrieves the value indirectly through the given register.
     */
	inline Word Indirect(ThreadP t, const Word& reg, bool remote = false)
	{
		if (!remote){
			return t->m()[t->r()[reg]];
		}
		else {
			return 0;
		}
	}
	
protected:
	inline Segment& GetSegment(Word index)
	{
		return segments[index];
	}
	
public:
	IblisVM();
	virtual ~IblisVM();
	
	/**
	 * Spawn a new thread.
     * @param segment segment ID for the thread.
     * @param address start address.
	 * @return false if the thread can't be started, true otherwise.
     */
	bool SpawnThread(Word segment, Word address);
	
	/**
	 * Kills the given thread.
     */
	void KillThread(ThreadP t);
	
	/**
	 * Executes the next instruction from the next thread.
	 * 
	 * @return false if all execution has stopped and the VM can be shut down,
	 * true otherwise.
     */
	bool ExecuteNext();
private:
	
};

}
#endif	/* IBLISVM_H */

