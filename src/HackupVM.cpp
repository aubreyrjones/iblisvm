/* 
 * File:   HackupVM.cpp
 * Author: netzapper
 * 
 * Created on June 29, 2014, 9:20 AM
 */

#include "HackupVM.h"

using namespace hackup;

Core::Core(int size) :
	memory(new Word[size])
{
	
}

Core::~Core()
{
	if (memory) delete [] memory;
}

HackupVM::HackupVM(int coreSize)
{
}

HackupVM::~HackupVM()
{
}

