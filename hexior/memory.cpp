#include "memory.h"
#include <string.h>

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
