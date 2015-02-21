#pragma once

#include <stdint.h>


class Memory {
	public:
		Memory(uint32_t memoryOffset, uint32_t size);
		~Memory();

	public:
		uint32_t	mOffset;
		uint32_t	mSize;
		uint8_t		*mData;

};
