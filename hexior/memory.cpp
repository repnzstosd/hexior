#include "memory.h"
#include <string.h>

Memory::Memory(uint32_t memoryOffset, uint32_t size) : mOffset(memoryOffset) {
	mData = new uint8_t[size];
	memset(mData, 0, size);
}

Memory::~Memory() {
	//if(mData) delete mData;
}
