#pragma once

//
#include <stdint.h>
#include <vector>

//
#include "memory.h"

class M68K {
	public:
		M68K(void);
		~M68K(void);

		void	initialize(std::vector<Memory> *memoryList);
		int		step();

		void	reset();

		uint8_t		readByte(uint32_t offset);
		uint16_t	readWord(uint32_t offset);
		uint32_t	readLong(uint32_t offset);

		void	writeByte(uint32_t offset, uint8_t data);
		void	writeWord(uint32_t offset, uint16_t data);
		void	writeLong(uint32_t offset, uint32_t data);

		void	branch(uint16_t instruction);

		enum SRF {
			carry				= 1 << 0,
			overflow		= 1 << 1,
			zero				= 1 << 2,
			negative		= 1 << 3,
			extend			= 1 << 4,
			intMask0		= 1 << 8,
			intMask1		= 1 << 9,
			intMask2		= 1 << 10,
			supervisor	= 1 << 13,
			traceMode		= 1 << 15,
		};


	public:
		union {
			uint16_t	SR;
			uint8_t		CCR;
		} mSR;
		uint32_t	mDataRegister[8];
		uint32_t	mAddressRegister[8];
		uint32_t	mSSP;
		uint32_t	mPC;				// only the lower 24bits are used on m68k.

	private:
		std::vector<Memory>	*mMemory;
};

