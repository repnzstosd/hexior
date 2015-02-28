#pragma once

#include <stdint.h>
#include <vector>

class Memory {
	public:
		Memory();
		~Memory();

		void alloc(uint32_t memoryOffset, uint32_t size);

	public:
		struct mem {
			uint32_t	mOffset;
			uint32_t	mSize;
			uint8_t		*mData;
		};

		std::vector<mem> mMemoryList;				// REMOVE ME to become obsolete.
};
