/* 
 * File:   HackupVM.h
 * Author: netzapper
 *
 * Created on June 29, 2014, 9:20 AM
 */

#include <cstdint>

#ifndef HACKUPVM_H
#define	HACKUPVM_H

namespace hackup {

#define N_REGS 256
typedef std::int32_t Word;

class Core {
public:
	Word *memory;
	
	Core(int size);
	virtual ~Core();
};

class Thread {
public:
	Word reg[N_REGS];
};

class HackupVM {
protected:
public:
	HackupVM(int coreSize);
	virtual ~HackupVM();
private:
	
};

}
#endif	/* HACKUPVM_H */

