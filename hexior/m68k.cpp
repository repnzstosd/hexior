#include "m68k.h"

/* -   -  - -- --- -=- -==- -==- -===- -===={[ TODO ]}====- -===- -==- -==- -=- --- -- -  -   -

-={[ Memory handling ]}=-

  Add own memory handling for blocks of memory; dividing the memory into 64kb-blocks makes it easier for everyone.

  Perhaps by creating an array that consists of structs; similar to what UAE uses, and I guess winuae also

	memoryBlock[65536];

	Each of the memoryblocks contain a structure similar to this

struct block {
	uint32_t	*readByte;
	uint32_t	*readWord;
	uint32_t	*readLong;
	uint32_t	*writeByte;
	uint32_t	*writeWord;
	uint32_t	*writeLong;
	uint32_t	*memory;
};

	When we call the readByte()-function we do so with the original pointer, no subtracting the offset or so. ***this may change***
	this way we can add "hardware" by calling a function in the M68K-class to add blocks to a memory area where that hardware take care of the read/writes
  that mean it's easier to implement for example CIA, as it doesn't have to ask the M68K-class for the words it need, it has already stored that information.
  Same for Paula and other similar coprocessors.

*/

M68K::M68K() {
}

M68K::~M68K() {
}

uint32_t M68K::dummy_rw(uint32_t offset) {
	return 0;
}

void M68K::initialize(std::vector<Memory::mem> *memoryList) {
	memoryBank dummy_bank = {
		dummy_rw, dummy_rw, dummy_rw,
		dummy_rw, dummy_rw, dummy_rw,
//		dummy_rw, dummy_rw,								// translate, check
		0, "Dummy", "Dummy memory",
//		dummy_rw, dummy_rw		// readLongInstr, readWordInstr
		0, 0, 0, 0
	};

	for(int i = 0; i < 65536; ++i) {			// initialize all memoryBanks to dummy_bank that will just return 0 for all memory accesses.
		mMemoryBanks[i] = &dummy_bank;
	}

//	mMemoryBanks[0]->baseAddress;

	mMemory = memoryList;	// this should be ripped out and replaced with something the CPU handles...
}

void M68K::reset() {
	mSSP		= readLong(0x0);
	mPC			= readLong(0x4);
	mSR.SR	= 0x2700;
}

uint8_t M68K::readByte(uint32_t offset) {
	// Add check for offset not outside allocated memory
	return mMemory->at(0).mData[offset];
}

uint16_t M68K::readWord(uint32_t offset) {
	return mMemory->at(0).mData[offset] << 8 |  mMemory->at(0).mData[offset+1];
}

uint32_t M68K::readLong(uint32_t offset) {
	return mMemory->at(0).mData[offset] << 24 | mMemory->at(0).mData[offset+1] << 16 | mMemory->at(0).mData[offset+2] << 8 | mMemory->at(0).mData[offset+3];
}

void M68K::writeByte(uint32_t offset, uint8_t data) {
	mMemory->at(0).mData[offset] = data;
}

void M68K::writeWord(uint32_t offset, uint16_t data) {
	mMemory->at(0).mData[offset + 0] = uint8_t(data >> 8);	
	mMemory->at(0).mData[offset + 1] = uint8_t(data >> 0);	
}

void M68K::writeLong(uint32_t offset, uint32_t data) {
	mMemory->at(0).mData[offset + 0] = uint8_t(data >> 24);
	mMemory->at(0).mData[offset + 1] = uint8_t(data >> 16);	
	mMemory->at(0).mData[offset + 2] = uint8_t(data >> 8);
	mMemory->at(0).mData[offset + 3] = uint8_t(data >> 0);	
}

void M68K::branch(uint16_t instruction) {
	uint32_t displacement = 0;
	if(!(instruction & 0xf)) {
		displacement = readWord(mPC);
	} else {
		displacement = (instruction & 0xf) | 0xfffffff0;
	}
	mPC += displacement;
}

int M68K::step() {
	int cycles = 0;
	uint16_t instruction = readWord(mPC);
	mPC += 2;

	switch(instruction >> 12) {
		case 0x0:
			// ORI, BTST, MOVEP, BCHG, BCLR, BSET, ANDI, SUBI, ADDI, EORI, CMPI
			switch (instruction & 0xe00) {

				case 0xc00:		// CMPI
					uint8_t size = (instruction >> 6) & 0x3;
					uint8_t mode = (instruction >> 3) & 0x7;
					uint8_t reg = instruction & 7;

					uint32_t destination = 0;
					uint32_t source = 0;

					if(size == 0) { source = readWord(mPC) & 0xff;	mPC += 2; }
					if(size == 1) { source = readWord(mPC);					mPC += 2; }
					if(size == 2) { source = readLong(mPC);					mPC += 4; }
					destination = readData(mode, reg, size);

					int64_t res = destination - source;

					setFlags(Flags::CMP, size, res, source, destination);

			}
			break;

		case 0x1:		// MOVE .B
		case 0x2:		// MOVE .L
		case 0x3: {	// MOVE .W
				uint8_t		dataSize				= (instruction >> 12) & 3;
				uint8_t		destRegister		= (instruction >> 9)	& 7;
				uint8_t		destMode				= (instruction >> 6)	& 7;
				uint8_t		sourceMode			= (instruction >> 3)	& 7;
				uint8_t		sourceRegister	= (instruction)				& 7;
				uint32_t	res							= 0;

				if(dataSize == 1) {					// Byte %01
					dataSize = 0;							// Byte 0
				} else if(dataSize == 3) {	// Word %11
					dataSize = 1;							// Word 1
				}	// dataSize 2 is already Long (for Move.x)

				res = readData(sourceMode, sourceRegister, dataSize);
				writeData(destMode, destRegister, dataSize, res);

				setFlags(Flags::LOGICAL, dataSize, res, 0, 0);

			}
			break;

		case 0x4:
			switch (instruction & 0xf1c0) {	// LEA, CHK
				case 0x41c0: {	// LEA	0100xxx111xxxxxx
						uint8_t mode = (instruction >> 3) & 0x7;
						uint8_t reg = (instruction) & 0x7;
						uint8_t	destReg = (instruction >> 9) & 0x7;

						mAddressRegister[destReg] = readData(mode, reg, Size::LONG);
					}
					break;

				case 0x4180: {	// CHK 0100xxx110xxxxxx	-- SIZE is WORD for destination. x110xx the 11 is WORD
					}
					break;
			}

			// NEGX, MOVE from SR, CLR, NEG, MOVE to CCR, NOT, MOVE to SR, NBCD, PEA, SWAP, MOVEM, EXT, TST, TAS, TRAP, LINK, UNLK, MOVE USP, JSR, JMP

			switch (instruction & 0xfb80) {
				case 0x4e80: {	// JSR
						uint8_t mode = (instruction >> 3) & 0x7;
						uint8_t reg = (instruction) & 0x7;

						uint32_t destination = readData(mode, reg, 3);		// 3 = long

						mAddressRegister[7] -= 4;
						writeLong(mAddressRegister[7], mPC);

						mPC = destination;

					}
					break;

				case 0x4840: {	// SWAP
						uint8_t reg = instruction & 0x7;
						mDataRegister[reg] = mDataRegister[reg] >> 16 | mDataRegister[reg] << 16;
					}
					break;

				case 0x4880: 	// MOVEM
					bool direction = (instruction & 0x400);
					bool size = (instruction & 0x40);

					uint8_t mode = (instruction >> 3 & 7);
					uint8_t reg = (instruction & 7);
					uint16_t regmask = readWord(mPC);
					mPC += 2;

					switch(mode) {
						case 0x3:	// mode (ax)+   NOT TESTED... until exit of routine..
							for(int i = 0; i < 16; ++i) {
								if(regmask & (1<<i)) {
									if(i < 8) {
										mDataRegister[i] = readLong(mAddressRegister[7]);
									} else {
										mAddressRegister[i-8] = readLong(mAddressRegister[7]);
									}
									mAddressRegister[7] += 4;
								}
							}
							break;

						case 0x4:		// mode -(ax)
							for(int i = 0; i < 16; ++i) {
								if(regmask & (1<<i)) {
									mAddressRegister[7] -= 4;
									if(i < 8) {
										writeLong(mAddressRegister[7], mAddressRegister[7-i]);
									} else {
										writeLong(mAddressRegister[7], mDataRegister[7-(i-8)]);
									}
								}
							}
							break;
					}
				break;
			}
			break;

		case 0x5: {
				uint32_t		data		= (instruction >> 9) & 0x7;
				uint32_t		cond		= (instruction >> 8) & 0xf;
				uint32_t		dir			= (instruction >> 8) & 0x1;
				uint32_t		opSize	= (instruction >> 6) & 0x3;
				uint32_t		mode		= (instruction >> 3) & 0x7;
				uint32_t		reg			= (instruction) & 0x7;

				if((instruction & 0x50f8) == 0x50c8) {					// DBcc
					uint8_t reg = instruction & 0x7;
					uint8_t cond = (instruction >> 8) & 0xf;
					int16_t displacement = readWord(mPC);

					if(!checkCondition(cond)) {
						mDataRegister[reg].w -= 1;
						if(int16_t(mDataRegister[reg].w) != -1) {
							mPC += int16_t(displacement);
						} else {
							mPC += 2;
						}
					}

				} else if((instruction & 0x50f8) == 0x50c0) {		// Scc
				} else {																				// ADDQ/SUBQ
					uint32_t res = 0;
					if(dir == 0) {				// AddQ
						res = readData(mode, reg, opSize);
						res += data;
						writeData(mode, reg, opSize, res);
					} else if(dir == 1) {	//SUBQ
						res = readData(mode, reg, opSize);
						res -= data;
						writeData(mode, reg, opSize, res);
					}
				}
			}
			break;

		case 0x6:	{	// Bcc, BRA, BSR
			int32_t displacement = 0;
			displacement = int8_t(instruction & 0xff);

			switch((instruction >> 8) & 0xf ) {
				case 0:		// BRA
					if(displacement == 0) {						// 16bit displacement if 0x00
						displacement = int16_t(readWord(mPC));
					}
					mPC += displacement;
					break;

				case 1:	{		// BSR
						mAddressRegister[7] -= 4;
						if(displacement == 0) {						// 16bit displacement if 0x00
							displacement = int16_t(readWord(mPC));
							writeLong(mAddressRegister[7], mPC + 2);
						} else {
							writeLong(mAddressRegister[7], mPC);
						}
						mPC += displacement;
					}
					break;

				default:	// for all other cases than (TRUE and FALSE (0 and 1))
					if(checkCondition((instruction >> 8) & 0xf)) {
						if(displacement == 0) {
							displacement = int16_t(readWord(mPC));
						}
						mPC += displacement;
					} else {
						if(displacement == 0) {
							mPC += 2;
						}
					}
					break;
				}
			}
			break;

		case 0x7:	{		// MOVEQ
				uint8_t reg		= uint8_t((instruction >> 9) & 7);
				uint32_t data = instruction & 0xff;

				if(data & 0x80) data |= 0xffffff00;
				mDataRegister[reg] = data;
				cycles += 4;
			}
			break;

		case 0x8:		// OR, DIVU, SBCD, DIVS

		case 0x9:		// SUB, SUBA, SUBX

		case 0xa:		// nope...
				// ILLEGAL
			break;

		case 0xb:		// CMP, CMPA, EOR, CMPM
			switch(instruction & 0xb1c8) {
				case 0xb0c0:	// CMPA
					break;

				case 0xb108:	// CMPM
					break;

				case 0xb100:	// EOR
					break;

				default: // CMP
					uint8_t destRegister		= (instruction >> 9) & 0x7;
					uint8_t operationSize		= (instruction >> 6) & 0x3;
					uint8_t sourceMode			= (instruction >> 3) & 0x7;
					uint8_t sourceRegister	= instruction & 7;

					int32_t source = 0;
					int32_t destination = 0;
					int64_t res = 0;

					destination	= mDataRegister[destRegister];
					if(operationSize == 0) {
						destination &= 0xff;
					} else if(operationSize == 1) {
						destination &= 0xffff;
					}

					source = readData(sourceMode, sourceRegister, operationSize);

					res = destination - source;

					setFlags(Flags::CMP, operationSize, res, source, destination);

					break;
			}
		case 0xc:
			// AND, MULU, EXG, ABCD, MULS

		case 0xd:
			// ADD, ADDA, ADDX

			switch (instruction & 0xf0c0) {	// 0x1111 0000 1100 0000
				case 0xd0c0:	// ADDA
					uint8_t destRegister		= (instruction >> 9) & 7;
					uint8_t operationSize		= (instruction >> 8) & 1;		// actually 011 or 111, but we only use the high bit for now.
					uint8_t sourceMode			= (instruction >> 3) & 7;
					uint8_t sourceRegister	= instruction & 7;
					uint32_t	data = 0;

					if((sourceMode == 0x7) && (sourceRegister == 0x4)) { // Immediate
						if(operationSize == 0) {
							data = readWord(mPC);
							if(data & 0x8000) data |= 0xffff0000;
							mPC += 2;
						} else if(operationSize == 1) {
							data = readLong(mPC);
							mPC += 4;
						}
					}

					mAddressRegister[destRegister] += data;
					break;
			}
			break;

		case 0xe:
			switch (instruction & 0xeec0) {
				case 0xe0c0:	// ASd	- Memory
					break;
				case 0xe2c0:	// LSd	- Memory
					break;
				case 0xe4c0:	// ROXd	- Memory
					break;
				case 0xe6c0:	// ROd	- Memory
					break;
			}

			switch (instruction & 0xe018) {
				case 0xe000:	// ASd
					break;
				case 0xe008:	// LSd
					break;
				case 0xe010:	// ROXd
					break;
				case 0xe018:	// ROd
					break;
			}

			// ASd, LSd, ROXd, ROd

		case 0xf:
			// Fpu...

		return cycles;
	}

	switch(instruction) {
		case 0x003c:	// ORI to CCR		(Byte, Immediate)
			break;

		case 0x007c:	// ORI to SR		(Word, Immediate)
			break;

		case 0x023c:	// ANDI to CCR	(Byte, Immediate)
			break;

		case 0x027c:	// ANDI to SR		(Word, Immediate)
			break;

		case 0x0a2c:	// EORI to CCR	(Byte, Immediate)
			break;

		case 0x0a7c:	// EORI to SR		(Word, Immediate)
			break;

		case 0x4afb:	// illegal
/*

SSP -= 2				-> SSP
Vector Offset		-> (SSP)
SSP -= 4				-> SSP
PC							-> (SSP)
SSP -= 2				-> SSP
SR							-> (SSP)
ILLEGAL instruction vector Address -> PC

*/


			break;

		case 0x4e70:	// reset
			cycles += 132;
			break;

		case 0x4e71:	// nop
			cycles += 4;
			break;

		case 0x4e72:	// stop					(Word, Immediate)?
			break;

		case 0x4e73:	// rte
			break;

		case 0x4e75:	// rts
			mPC = readLong(mAddressRegister[7]);
			mAddressRegister[7] += 4;
			cycles += 16;
			break;

		case 0x4e76:	// trapv

			break;

		case 0x4e77:	// rtr
			break;
	}


	return 0;	// Number of cycles executed... <47
}


uint32_t M68K::getMSB(uint8_t size) {
	switch(size) {
		case Size::BYTE:	return 0x80;
		case Size::WORD:	return 0x8000;
		case Size::LONG:	return 0x80000000;
		default:		return 0;
	}
}

uint8_t M68K::bitSize(uint8_t size) {
	switch(size) {
		case Size::BYTE:	return 8;
		case Size::WORD:	return 16;
		case Size::LONG:	return 32;
		default:					return 0;
	}
}

uint32_t M68K::bitMask(uint8_t size) {
	switch(size) {
		case Size::BYTE:	return 0xff;
		case Size::WORD:	return 0xffff;
		case Size::LONG:	return 0xffffffff;
		default:					return 0xffffffff;
	}
}

uint32_t M68K::maskValue(uint32_t value, uint8_t size) {
	return value & bitMask(size);
}

bool M68K::checkCondition(uint8_t conditionCode) {
	switch(conditionCode & 0xf) {
		case 0: return	true;
		case 1: return	false;
		case 2: return	!mSR.carry & !mSR.zero;
		case 3: return	mSR.carry |  mSR.zero;
		case 4: return	!mSR.carry;
		case 5: return	mSR.carry;
		case 6: return	!mSR.zero;
		case 7: return	mSR.zero;
		case 8: return	!mSR.overflow;
		case 9: return	mSR.overflow;
		case 10: return	!mSR.negative;
		case 11: return	mSR.negative;
		case 12: return	!(mSR.negative ^ mSR.overflow);
		case 13: return	mSR.negative ^ mSR.overflow;
		case 14: return	(mSR.negative &  mSR.overflow & !mSR.zero) | (!mSR.negative & !mSR.overflow & !mSR.zero);
		case 15: return	mSR.zero | (mSR.negative & !mSR.overflow) | (!mSR.negative & mSR.overflow);
	}
	return false;
}

void M68K::setFlags(uint8_t type, uint8_t size, uint64_t result, uint32_t source, uint32_t dest) {

	bool resultNegative = (result & getMSB(size));
	bool sourceNegative = (source & getMSB(size));
	bool destNegative		= (dest		& getMSB(size));

	switch(type) {
		case Flags::LOGICAL:
			mSR.carry			= 0;
			mSR.overflow	= 0;
			mSR.zero			= (maskValue(result, size) == 0);
			mSR.negative	= resultNegative;
			break;

		case Flags::SUB:
		case Flags::CMP:
			mSR.zero			= (maskValue(result, size) == 0);
			mSR.negative	= resultNegative;
			// no break
		case Flags::SUBX:
			mSR.carry			= (result >> bitSize(size)) & 1;
			if(type != Flags::CMP) mSR.extend = mSR.carry;
			mSR.overflow	= (sourceNegative ^ destNegative) & (resultNegative ^ destNegative);
			break;

		case Flags::ADD:
			mSR.zero			= (maskValue(result, size) == 0);
			mSR.negative	= resultNegative;
			// no break
		case Flags::ADDX:
			mSR.carry			= (result >> bitSize(size)) & 1;
			mSR.extend		= mSR.carry;
			mSR.overflow	= (sourceNegative ^ resultNegative) & (destNegative ^ resultNegative);
			break;

		case Flags::ZN:
			mSR.zero			= mSR.zero & (maskValue(result, size) == 0);
			mSR.negative	= resultNegative;
			break;
	}
}

//
// for indirect with post/pre-increment/decrement we need to check A7 for special case when reading one byte,
// a7 can never be odd, so we need to add 2, all other cases 1 *for byte-read*
//
uint32_t M68K::writeData(uint8_t destMode, uint8_t destRegister, uint8_t operationSize, uint32_t res) {
	if(destMode == 0x0) {					// Dn
		if(operationSize == 0) {				mDataRegister[destRegister] = mDataRegister[destRegister] & 0xffffff00 | res & 0xff;
		} else if(operationSize == 1) {	mDataRegister[destRegister] = mDataRegister[destRegister] & 0xffff0000 | res & 0xffff;
		} else if(operationSize == 2) {	mDataRegister[destRegister] = res;
		}

	} else if(destMode == 0x1) {	// An
		if(operationSize == 0) {				mAddressRegister[destRegister] = mAddressRegister[destRegister] & 0xffffff00 | res & 0xff;
		} else if(operationSize == 1) {	mAddressRegister[destRegister] = mAddressRegister[destRegister] & 0xffff0000 | res & 0xffff;
		} else if(operationSize == 2) {	mAddressRegister[destRegister] = res;
		}

	} else if(destMode == 0x2) {	// (an)
		if(operationSize == 0) {				writeByte(mAddressRegister[destRegister], res);
		} else if(operationSize == 1) {	writeWord(mAddressRegister[destRegister], res);
		} else if(operationSize == 2) {	writeLong(mAddressRegister[destRegister], res);
		}

	} else if(destMode == 0x3) {	// (an)+
		if(operationSize == 0) { // byte
			writeByte(mAddressRegister[destRegister], res);
			mAddressRegister[destRegister] += 1;
		} else if(operationSize == 1) { // word
			writeWord(mAddressRegister[destRegister], res);
			mAddressRegister[destRegister] += 2;
		} else if(operationSize == 2) { // long
			writeLong(mAddressRegister[destRegister], res);
			mAddressRegister[destRegister] += 4;
		}

	} else if(destMode == 0x4) {	// -(an)
		if(operationSize == 0) { // byte
			mAddressRegister[destRegister] -= 1;
			writeByte(mAddressRegister[destRegister], res);
		} else if(operationSize == 1) { // word
			mAddressRegister[destRegister] -= 2;
			writeWord(mAddressRegister[destRegister], res);
		} else if(operationSize == 2) { // long
			mAddressRegister[destRegister] -= 4;
			writeLong(mAddressRegister[destRegister], res);
		}

	} else if(destMode == 0x5) {	// (d16,An)
		uint32_t displacement = readWord(mPC);
		mPC += 2;
		if(operationSize == 0) {				writeByte(mAddressRegister[destRegister] + displacement, res);
		} else if(operationSize == 1) {	writeWord(mAddressRegister[destRegister] + displacement, res);
		} else if(operationSize == 2) {	writeLong(mAddressRegister[destRegister] + displacement, res);
		}

	} else if(destMode == 0x6) {	// (d8,An,Xn)
		if(operationSize == 0) { // byte
		} else if(operationSize == 1) { // word
		} else if(operationSize == 2) { // long
		}

	} else if((destMode == 0x7) && (destRegister == 0x0)) {	// (xxx).W
		uint16_t absoluteShort = readWord(mPC);
		if(operationSize == 0) {				writeByte(absoluteShort, res);
		} else if(operationSize == 1) {	writeWord(absoluteShort, res);
		} else if(operationSize == 2) {	writeLong(absoluteShort, res);
		}

	} else if((destMode == 0x7) && (destRegister == 0x1)) {	// (xxx).L
		uint32_t absoluteShort = readWord(mPC);
		if(operationSize == 0) {				writeByte(absoluteShort, res);
		} else if(operationSize == 1) {	writeWord(absoluteShort, res);
		} else if(operationSize == 2) {	writeLong(absoluteShort, res);
		}

	} else if((destMode == 0x7) && (destRegister == 0x2)) {	// (d16, PC)
		if(operationSize == 0) { // byte
		} else if(operationSize == 1) { // word
		} else if(operationSize == 2) { // long
		}

	} else if((destMode == 0x7) && (destRegister == 0x3)) {	// (d8, PC, Xn)
		if(operationSize == 0) { // byte
		} else if(operationSize == 1) { // word
		} else if(operationSize == 2) { // long
		}

	}
	return 0;
}

//
// for indirect with post/pre-increment/decrement we need to check A7 for special case when reading one byte,
// a7 can never be odd, so we need to add 2, all other cases 1 *for byte-read*
//
uint32_t M68K::readData(uint8_t sourceMode, uint8_t sourceRegister, uint8_t operationSize) {
	uint32_t res = 0;
	if(sourceMode == 0x0) {																				// Dn
		if(operationSize == 0) {				res = mDataRegister[sourceRegister] & 0xff;
		} else if(operationSize == 1) {	res = mDataRegister[sourceRegister] & 0xffff;
		} else if(operationSize == 2) {	res = mDataRegister[sourceRegister];
		}
	} else if(sourceMode == 0x1) {																// An
		if(operationSize == 0) {				res = mAddressRegister[sourceRegister] & 0xff;
		} else if(operationSize == 1) {	res = mAddressRegister[sourceRegister] & 0xffff;
		} else if(operationSize == 2) {	res = mAddressRegister[sourceRegister];
		}
	} else if(sourceMode == 0x2) {																// (an)
		if(operationSize = 0) {					res = readByte(mAddressRegister[sourceRegister]);
		} else if(operationSize == 1) {	res = readWord(mAddressRegister[sourceRegister]);
		} else if(operationSize == 2) {	res = readLong(mAddressRegister[sourceRegister]);
		}
	} else if(sourceMode == 0x3) {																// (an)+
		if(operationSize == 0) { // byte
			res = readByte(mAddressRegister[sourceRegister]);
			mAddressRegister[sourceRegister] += 1;
		} else if(operationSize == 1) { // word
			res = readWord(mAddressRegister[sourceRegister]);
			mAddressRegister[sourceRegister] += 2;
		} else if(operationSize == 2) { // long
			res = readLong(mAddressRegister[sourceRegister]);
			mAddressRegister[sourceRegister] += 4;
		}

	} else if(sourceMode == 0x4) {																// -(an)
		if(operationSize == 0) { // byte
			mAddressRegister[sourceRegister] -= 1;
			res = readByte(mAddressRegister[sourceRegister]);
		} else if(operationSize == 1) { // word
			mAddressRegister[sourceRegister] -= 2;
			res = readWord(mAddressRegister[sourceRegister]);
		} else if(operationSize == 2) { // long
			mAddressRegister[sourceRegister] -= 4;
			res = readLong(mAddressRegister[sourceRegister]);
		}

	} else if(sourceMode == 0x5) {																// (d16,An)
		uint32_t displacement = readWord(mPC);
		mPC += 2;
		if(operationSize == 0) {				res = readByte(mAddressRegister[sourceRegister] + displacement);
		} else if(operationSize == 1) {	res = readWord(mAddressRegister[sourceRegister] + displacement);
		} else if(operationSize == 2) {	res = readLong(mAddressRegister[sourceRegister] + displacement);
		}

	} else if(sourceMode == 0x6) {																// (d8,An,Xn)

	} else if((sourceMode == 0x7) && (sourceRegister == 0x0)) {		// (xxx).W
		res = readWord(mPC);
		mPC += 2;
		if(operationSize == 0) {				res = readByte(res);
		} else if(operationSize == 1) {	res = readWord(res);
		} else if(operationSize == 2) {	res = readLong(res);
		}

	} else if((sourceMode == 0x7) && (sourceRegister == 0x1)) {		// (xxx).L
		res = readLong(mPC);
		mPC += 4;
		if(operationSize == 0) {				res = readByte(res);
		} else if(operationSize == 1) {	res = readWord(res);
		} else if(operationSize == 2) {	res = readLong(res);
		}

	} else if((sourceMode == 0x7) && (sourceRegister == 0x2)) {		// (d16, PC)
		res = readWord(mPC);
		res += mPC;
		mPC += 2;
		//if(operationSize == 0) { // byte
		//} else if(operationSize == 1) { // word
		//} else if(operationSize == 2) { // long
		//}

	} else if((sourceMode == 0x7) && (sourceRegister == 0x3)) {		// (d8, PC, Xn)
		if(operationSize == 0) { // byte
		} else if(operationSize == 1) { // word
		} else if(operationSize == 2) { // long
		}

	} else if((sourceMode == 0x7) && (sourceRegister == 0x4)) {		// #imm
		if(operationSize == 0) { // byte
			res = readWord(mPC) & 0xff;
			mPC += 2;
		} else if(operationSize == 1) { // word
			res = readWord(mPC);
			mPC += 2;
		} else if(operationSize == 2) { // long
			res = readLong(mPC);
			mPC += 4;
		}

	}
	return res;
}

