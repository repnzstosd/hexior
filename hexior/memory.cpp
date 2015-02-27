#include "memory.h"
#include <string.h>

/*

  Supportclass for M68K

  At start we setup the entire memoryspace; 4GB divided in 64k-blocks, all pointing to functions that will return illegal memory

	We then get called by userspace (amiga.cpp) to set up the various memoryspaces, this will be done something like:
  M68K::mapMem(start, size);

	This will allocate memory and add functions to read/write to that memory for the memoryblocks needed.

	When the CPU writes to memory, we will just do
	writeWord(xxxx, 0)... 
	bank[xxxx >> 16].writeWord(xxxx, 0);

	The object that is saved in the "bank"-list is the same for the entire block of memory,
  there should be no problems when writing to offset 65534 with a word that wraps to the next bank.


*/

Memory::Memory() {
}

void Memory::alloc(uint32_t memoryOffset, uint32_t size) {
	mem temp;
	temp.mOffset = memoryOffset;
	temp.mSize = size;
	temp.mData = new uint8_t[size];
	mMemoryList.push_back(temp);
	memset(temp.mData, 0, size);
}

Memory::~Memory() {
	for(auto mem : mMemoryList) {
		delete mem.mData;
	}
}
