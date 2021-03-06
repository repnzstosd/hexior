#include "m68k.h"



class Core_68k {
	public:
		void foo();
		template<bool dynamic> void op_bset(uint16_t opcode);;
		void (Core_68k::*opcodes[0x10000])(uint16_t opcode);
};


template<bool dynamic> void Core_68k::op_bset(uint16_t opcode) {
	uint8_t reg, bit;
 //   u8 size = ((opcode >> 3) & 7) == 0 ? SizeLong : SizeByte;

 //   if (dynamic) {
        reg = (opcode >> 9) & 7;
//        bit = (size == SizeLong) ? reg_d[reg] & 31 : reg_d[reg] & 7;
 //   } else {
 //       bit = (size == SizeLong) ? reg_irc & 31 : reg_irc & 7;
 //       readExtensionWord();
 //   }

	//u32 data = LoadEA(size, opcode & 0x3F);

	//reg_s.z = !!( 1 ^ ((data >> bit) & 1) );
	//data |= (1 << bit);
 //   prefetch(isRegisterMode());

 //   if (size == SizeLong) {
 //       sync(2);
 //       if (bit > 15) sync(2);
 //   }
 //   writeEA(size, data, true);
}

void Core_68k::foo() {
	uint16_t i = 0;
	opcodes[i + 0] = &Core_68k::op_bset<false>;
	opcodes[i + 1] = &Core_68k::op_bset<true>;
	opcodes[i + 2] = &Core_68k::op_bset<true>;
	opcodes[i + 3] = &Core_68k::op_bset<true>;
	opcodes[i + 4] = &Core_68k::op_bset<true>;
	opcodes[i + 5] = &Core_68k::op_bset<true>;
	opcodes[i + 6] = &Core_68k::op_bset<true>;
}

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
	Core_68k a;
	a.foo();
}

uint8_t M68K::readByte(uint32_t offset) {
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

/* -   -  - -- --- -=- -==- -==- -===- -===={[ 0x0 ]}====- -===- -==- -==- -=- --- -- -  -   - */

		case 0x0:
			// ORI, ANDI, SUBI, ADDI, EORI, BTST, MOVEP, BCHG, BCLR, BSET, CMPI
			switch (instruction & 0x0f00) {

				case 0x0000:	// ORI
					break;
				case 0x0200:	// andi
					break;
				case 0x0400:	// subi
					break;
				case 0x0600:	// addi
					break;
				case 0x0a00:	// eori
					break;

				case 0x0c00: {		// CMPI
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

				default:
//				case 0x0100:	// BTST/BCHG/BCLR/BSET Immediate
//				case 0x0800:	// BTST/BCHG/BCLR/BSET
					if((instruction & 0x0800) || (instruction & 0x0100)) {
						uint8_t destMode	= (instruction >> 3) & 0x7;
						uint8_t destReg		= (instruction) & 0x7;
						uint8_t shiftReg	= (instruction >> 9) & 0x7;
						uint8_t immShift	= readWord(mPC);	// if the instruction is immediate we should also add 2 to the PC, else not.

						uint32_t dest = readData(destMode, destReg, Size::BYTE);

						mSR.zero = (dest && 1 << mDataRegister[shiftReg]);
						dest |= 1 << mDataRegister[shiftReg];

						writeData(destMode, destReg, Size::LONG, dest);

//btst 0000100000xxxxxx
//bchg 0000100001xxxxxx
	//dest |= z ? 1<<bit : 0;
//bclr 0000100010xxxxxx
	//dest &= ~(1<<bit);
//bset 0000100011xxxxxx
	//dest |= 1<<bit;


//btst 0000xxx100xxxxxx	// immediate
//bchg 0000xxx101xxxxxx // immediate
//bclr 0000xxx110xxxxxx // immediate
//bset 0000xxx111xxxxxx // immediate

					}
					break;

			}
			break;

/* -   -  - -- --- -=- -==- -==- -===- -===={[ 0x1 0x2 0x3 (Move) ]}====- -===- -==- -==- -=- --- -- -  -   - */

		case 0x1:		// MOVE .B
			// No BREAK
		case 0x2:		// MOVE .L
			// No BREAK
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

/* -   -  - -- --- -=- -==- -==- -===- -===={[ 0x4 ]}====- -===- -==- -==- -=- --- -- -  -   - */
		case 0x4:
			switch (instruction & 0xf1c0) {	// LEA, CHK
				case 0x41c0: {	// LEA	0100xxx111xxxxxx
						uint8_t mode		= (instruction >> 3) & 0x7;
						uint8_t reg			= (instruction) & 0x7;
						uint8_t	destReg	= (instruction >> 9) & 0x7;

						mAddressRegister[destReg] = readData(mode, reg, Size::LONG);
						reg = 47; /// REMOVE
					}
					break;

				case 0x4180: {	// CHK 0100xxx110xxxxxx	-- SIZE is WORD for destination. x110xx the 11 is WORD
					}
					break;

			// NEGX, MOVE from SR, CLR, NEG, MOVE to CCR, NOT, MOVE to SR, NBCD, PEA, SWAP, MOVEM, EXT, TST, TAS, TRAP, LINK, UNLK, MOVE USP, JSR, JMP
				case 0x4e50: {	// link		-==- SP - 4 -> SP; An -> (SP); SP -> An; SP + dn -> SP -==-
						uint8_t	reg = instruction & 0x7;
						int16_t		displacement = readWord(mPC);			// if we are to support 68020+ this could be a long.
						mAddressRegister[7] -= 4;
						writeWord(mAddressRegister[7], mAddressRegister[reg]);
						mAddressRegister[reg] = mAddressRegister[7];
						mAddressRegister[7] += displacement;
					}
					break;

				case 0x4e58: {	// unlk		-==- An -> SP; (SP) -> An; SP + 4 -> SP
						uint8_t	reg = instruction & 0x7;
						mAddressRegister[7] = mAddressRegister[reg];
						mAddressRegister[7] += 4;
					}
					break;

			}
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

/* -   -  - -- --- -=- -==- -==- -===- -===={[ 0x5 ]}====- -===- -==- -==- -=- --- -- -  -   - */

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

/* -   -  - -- --- -=- -==- -==- -===- -===={[ 0x6 ]}====- -===- -==- -==- -=- --- -- -  -   - */

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

/* -   -  - -- --- -=- -==- -==- -===- -===={[ 0x7 ]}====- -===- -==- -==- -=- --- -- -  -   - */
		case 0x7:	{		// MOVEQ
				uint8_t reg		= uint8_t((instruction >> 9) & 7);
				uint32_t data = instruction & 0xff;

				if(data & 0x80) data |= 0xffffff00;
				mDataRegister[reg] = data;
				cycles += 4;
			}
			break;

/* -   -  - -- --- -=- -==- -==- -===- -===={[ 0x8 ]}====- -===- -==- -==- -=- --- -- -  -   - */
		case 0x8:		// OR, DIVU, SBCD, DIVS
			break;

/* -   -  - -- --- -=- -==- -==- -===- -===={[ 0x9 ]}====- -===- -==- -==- -=- --- -- -  -   - */
		case 0x9:		// SUB, SUBA, SUBX
			break;

/* -   -  - -- --- -=- -==- -==- -===- -===={[ 0xA Illegal ]}====- -===- -==- -==- -=- --- -- -  -   - */
		case 0xa:		// nope...
				// ILLEGAL
			break;

/* -   -  - -- --- -=- -==- -==- -===- -===={[ 0xB ]}====- -===- -==- -==- -=- --- -- -  -   - */
		case 0xb:		// CMP, CMPA, EOR, CMPM
			switch(instruction & 0xb1c8) {
				case 0xb0c0:	// CMPA
					break;

				case 0xb108:	// CMPM
					break;

				case 0xb100:	// EOR
					break;

				default: // CMP
					uint8_t destReg			= (instruction >> 9) & 0x7;
					uint8_t size				= (instruction >> 6) & 0x3;
					uint8_t sourceMode	= (instruction >> 3) & 0x7;
					uint8_t sourceReg		= instruction & 7;

					int32_t source = 0;
					int32_t destination = 0;
					int64_t res = 0;

					destination	= mDataRegister[destReg];
					if(size == Size::BYTE) {
						destination &= 0xff;
					} else if(size == Size::LONG) {
						destination &= 0xffff;
					}

					source = readData(sourceMode, sourceReg, size);

					res = destination - source;

					setFlags(Flags::CMP, size, res, source, destination);

					break;
			}

/* -   -  - -- --- -=- -==- -==- -===- -===={[ 0xC ]}====- -===- -==- -==- -=- --- -- -  -   - */
		case 0xc:
			switch(instruction & 0x01c0) {
				case 0x00c0:	// MULU
// only the MULU.W on 68000.
//
// Source x Destination -> Destination
//
// Multiplies two unsigned operands yielding an unsigned result.
// In the word form, the multiplier and multiplicand are both word operands, and the result
// is a long-word operand. A register operand is the low-order word; the upper word of the
// register is ignored. All 32 bits of the product are saved in the destination data register.
//
// Mulu.w <ea>, Dn	16 * 16 -> 32
//
					break;
				case 0x01c0:	// MULS
					break;
				case 0x0100:	// ABCD
					break;

				case 0x0140:	// EXG	opmode is 5 bits for EXG, the upper two is "01" or "10".. only way to get these is two cases as AND is 0xc000 masked.
					// no break
				case 0x0180:	// EXG

					break;
				default:			// AND	-- NOT TESTED
/*
opmode field
Byte Word Long    Operation
 000  001  010     <ea> & Dn -> Dn
 100  101  110     Dn & <ea> -> <ea>
*/
					bool		dir		= (instruction >> 8) & 0x1;
					uint8_t size	= (instruction >> 6) & 0x3;
					uint8_t	dn		= (instruction >> 9) & 0x7;
					uint8_t	mode	= (instruction >> 3) & 0x7;
					uint8_t	reg		= (instruction) & 0x7;

					uint32_t	dataRegister			= mDataRegister[dn];
					uint32_t	effectiveAddress	= readData(mode, reg, size);
					uint32_t	result						= 0;

					if(dir) {	// destination is <ea>
						result = dataRegister & effectiveAddress;
						writeData(mode, reg, size, maskValue(result, size));
						setFlags(Flags::LOGICAL, size, result, result, effectiveAddress);

					} else {	// destination is Dn
						result = effectiveAddress & dataRegister;
						mDataRegister[dn] = maskValue(result, size);
						setFlags(Flags::LOGICAL, size, result, result, dataRegister);
					}
			}

			break;

/* -   -  - -- --- -=- -==- -==- -===- -===={[ 0xD ]}====- -===- -==- -==- -=- --- -- -  -   - */
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
							data = int16_t(readWord(mPC));		// should be enough to sign-extend.
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

/* -   -  - -- --- -=- -==- -==- -===- -===={[ 0xE ]}====- -===- -==- -==- -=- --- -- -  -   - */
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
//
// Destination >> Count -> Destination
//
// ASd Dx, Dy
// ASd #data, Dy
// ASd <ea>
// Where d is direction, L or R
//
// 1. Immediate - The shift count is specified in the instruction (shift range 1-8);
// 2. Register - The shift count is the value in the data register specified in instruction modulo 64.
//
// The size of the operation can be specified as byte, word or long. An operand in memory can be shifted one bit only, and the operand size is restricted to word.
//
// For ASL:
//
//  Carry  <---*---[OPERAND] <---- 0
//             |
//  eXtended <-'
//
// For ASR:
//
// .--> [MSB|OPERAND] ---*---> Carry
// |      ^              |
// '------'              '---> eXtended
//
					break;

				case 0xe008: {	// LSd

						uint8_t destination	= (instruction) & 0x7;
						uint8_t	mode				= (instruction >> 5) & 0x1;	// I/R
						uint8_t size				= (instruction >> 6) & 0x3;
						uint8_t	direction		= (instruction >> 8) & 0x1;
						uint8_t	source			= (instruction >> 9) & 0x7;	// Count OR Register

// TODO: Implement
						if(direction == 0) {	// Right
						} else {							// Left
						}

					}
					break;
				case 0xe010:	// ROXd
					break;
				case 0xe018:	// ROd
					break;
			}

/* -   -  - -- --- -=- -==- -==- -===- -===={[ 0xF LINE-F Emulator Trap (FPU) ]}====- -===- -==- -==- -=- --- -- -  -   - */
		case 0xf:
			// Fpu...

		return cycles;
	}

/* -   -  - -- --- -=- -==- -==- -===- -===={[ Full word functions ]}====- -===- -==- -==- -=- --- -- -  -   - */

	switch(instruction) {
		case 0x003c:	// ORI to CCR		(Byte, Immediate)

// Source | CCR -> CCR
//
// ori #data, CCR
//
// instruction format
// 0000000000111100
// 00000000dddddddd

			break;

		case 0x007c:	// ORI to SR		(Word, Immediate)

// if SupervisorState
//	source | SR -> SR
// else TRAP
//
// ori #data, SR
//
// instruction format
// 0000000001111100
// dddddddddddddddd

			break;

		case 0x023c:	// ANDI to CCR	(Byte, Immediate)
			break;

		case 0x027c:	// ANDI to SR		(Word, Immediate)

// if SupervisorState
//	source & SR -> SR
// else TRAP
//
// andi #data, SR
//
// instruction format
// 0000001001111100
// dddddddddddddddd

			break;

		case 0x0a2c:	// EORI to CCR	(Byte, Immediate)
			break;

		case 0x0a7c:	// EORI to SR		(Word, Immediate)

// if SupervisorState
//	source ^ SR -> SR
// else TRAP
//
// eori #data, SR
//
// instruction format
// 0000101001111100
// dddddddddddddddd

			break;

		case 0x4afb:	// illegal

// SSP-2 -> SSP; VectorOffset -> (SSP)	// NOT DONE on 68k, only 68020+
// SSP-4 -> SSP; PC -> (ssp)
// SSP-2 -> SSP; SR -> (SSP)
// Illegal Instruction Vector Address -> PC

			break;

		case 0x4e70:	// reset
			cycles += 132;
			break;

		case 0x4e71:	// nop
			cycles += 4;
			break;

		case 0x4e72:	// stop					(Word, Immediate)?
// if SupervisorState
//   Immediate Data -> SR; STOP
// else TRAP
//
// Stop #data
//

			break;

		case 0x4e73:	// rte
// if SupervisorState
//   (SP) -> SR;
//   SP+2 -> SP;
//   (SP) -> PC;
//   SP+4 -> SP;
//   Restore State and Deallocate Stack According to (SP)
// else TRAP
//
//


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

//
// RENAME
//
// rename to msb(uint8_t size)?
// getMSB should return the value, not the mask
//
// bitSize is ok
// bitmask is ok
//
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
		case 3: return	 mSR.carry |  mSR.zero;
		case 4: return	!mSR.carry;
		case 5: return	 mSR.carry;
		case 6: return	!mSR.zero;
		case 7: return	 mSR.zero;
		case 8: return	!mSR.overflow;
		case 9: return	 mSR.overflow;
		case 10: return	!mSR.negative;
		case 11: return	 mSR.negative;
		case 12: return	!(mSR.negative ^ mSR.overflow);
		case 13: return	  mSR.negative ^ mSR.overflow;
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
			// no break
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
// TODO: These will become the new read-/writeData() functions..
//
// We will use two variables, saved privately in the class, to make the writeEA as short as possible to minimize bugs.
// We get away with this as the loadEA and writeEA is always called inside one instruction, so no possibility of fuckup(tm)
//
// Possibly I will split this up further; adding a decodeEA(instruction) to pull out the data to a struct that we then send to readEA(struct)/writeEA(struct)...
//

// these are the class variables.
Register32 *mEARegister;	// should be a pointer to the m(Data/Address)register[x] we wish to write.
uint32_t		mEAAddress;		// should be the calculated address pointed to by ea
Register32			mDataRegister[8];
Register32			mAddressRegister[8];
uint32_t mPC;

uint32_t readByte(uint32_t a) { return 0; }
uint32_t readWord(uint32_t a) { return 0; }
uint32_t readLong(uint32_t a) { return 0; }
uint32_t writeByte(uint32_t a, uint32_t b) { return 0; }
uint32_t writeWord(uint32_t a, uint32_t b) { return 0; }
uint32_t writeLong(uint32_t a, uint32_t b) { return 0; }

void writeEA(uint8_t size, uint32_t value) {

	if(mEARegister) {
		switch(size) {
			case Size::BYTE: mEARegister->l = value & 0xff;		return;
			case Size::WORD: mEARegister->w = value & 0xffff;	return;
			case Size::LONG: mEARegister->d = value;					return;
			default: return;
		}
	}

	switch(size) {
		case Size::BYTE:	writeByte(mEAAddress, value & 0xff);		return;
		case Size::WORD:	writeWord(mEAAddress, value & 0xffff);	return;
		case Size::LONG:	writeLong(mEAAddress, value);						return;
	}
}

uint32_t maskVal(uint32_t a, uint8_t size) { return 0; }

uint32_t loadEA(uint8_t size, uint8_t ea) {
	mEARegister = 0;
	mEAAddress	= 0;

	uint32_t	dispReg;
	uint32_t	operand;
	uint8_t		regPos	= ea & 0x7;
	uint8_t		mode		= (ea >> 3) & 0x7;

	switch(mode) {
		case 0:	// Dn						- Data Register Direct Mode
			mEARegister = &mDataRegister[regPos];
			return maskVal(mDataRegister[regPos], size);

		case 1:	// An						- Address Register Direct Mode
			mEARegister = &mAddressRegister[regPos];
			return maskVal(mAddressRegister[regPos], size);

		case 2:	// (An)					-	Address Register Indirect Mode
			mEAAddress = mAddressRegister[regPos];
			break;

		case 3:	// (An)+				- Address Register Indirect with Postincrement Mode
			mEAAddress = mAddressRegister[regPos];
			if(size == Size::BYTE) {
				if(regPos == 7) mEAAddress += 2;		// if a7 (Stackpointer)
				else mEAAddress += 1;
			} else if(size == Size::WORD) {
				mEAAddress += 2;
			} else if(size == Size::LONG) {
				mEAAddress += 4;
			}
			break;

		case 4:	// -(An)				- Address Register Indirect with Predecrement Mode
			if(size == Size::BYTE) {
				if(regPos == 7) mAddressRegister[regPos] -= 2;		// if a7 (Stackpointer)
				else mAddressRegister[regPos] -= 1;
			} else if(size == Size::WORD) {
				mAddressRegister[regPos] -= 2;
			} else if(size == Size::LONG) {
				mAddressRegister[regPos] -= 4;
			}
			mEAAddress = mAddressRegister[regPos];
			break;

		case 5:	// (d16, An)		- Address Register Indirect with Displacement Mode
			mEAAddress = mAddressRegister[regPos] + int32_t(int16_t(readWord(mPC)));	// sign-extend to 32bit.
			mPC += 2;
			break;

		case 6:	// (d8, An, Xn)	- Address Register Indirect with Index (8-Bit Displacement) Mode
			break;

		case 7:
			switch(regPos) {
				case 0:	// Absolute Word	- Absolute Short Addressing Mode
					mEAAddress = int32_t(int16_t(readWord(mPC)));
					mPC += 2;
					break;

				case 1:	// Absolute Long	- Absolute Long Addressing Mode
					mEAAddress = readLong(mPC);
					mPC += 4;
					break;

				case 2:	// (d16, PC)			- Program Counter Indirect with Displacement Mode
					mEAAddress = mPC + int16_t(readWord(mPC));
					mPC += 2;
					break;

				case 3:	// (d8, PC, Xn)		- Program Counter Indirect with Index (8-Bit Displacement) Mode
					break;

				case 4:	// #imm						- Immediate Data
					if(size == Size::BYTE) {
						operand = readWord(mPC) & 0xff;
					} else if(size == Size::WORD) {
						operand = readWord(mPC);
					} else if(size == Size::LONG) {
						operand = readLong(mPC);
					}
					return operand;
			}
			break;
	}

	switch(size) {
		case Size::BYTE:	operand = readByte(mEAAddress); break;
		case Size::WORD:	operand = readWord(mEAAddress); break;
		case Size::LONG:	operand = readLong(mEAAddress); break;
	}

	return operand;
}



//
//
//  REWRITE readData(...) and writeData(...); 
//
//
// for indirect with post/pre-increment/decrement we need to check A7 for special case
// when reading one byte, a7 can never be odd, so we need to add 2, all other cases 1 *for byte-read*
//
uint32_t M68K::writeData(uint8_t mode, uint8_t reg, uint8_t size, uint32_t res) {
	switch(mode) {
		case 0:									// Dn
			if(size == Size::BYTE) {				mDataRegister[reg] = mDataRegister[reg] & 0xffffff00 | res & 0xff;
			} else if(size == Size::WORD) {	mDataRegister[reg] = mDataRegister[reg] & 0xffff0000 | res & 0xffff;
			} else if(size == Size::LONG) {	mDataRegister[reg] = res;
			}
			break;
		case 1:									// An
			if(size == Size::BYTE) {				mAddressRegister[reg] = mAddressRegister[reg] & 0xffffff00 | res & 0xff;
			} else if(size == Size::WORD) {	mAddressRegister[reg] = mAddressRegister[reg] & 0xffff0000 | res & 0xffff;
			} else if(size == Size::LONG) {	mAddressRegister[reg] = res;
			}
			break;
		case 2:									// (An)
			if(size == Size::BYTE) {				writeByte(mAddressRegister[reg], res);
			} else if(size == Size::WORD) {	writeWord(mAddressRegister[reg], res);
			} else if(size == Size::LONG) {	writeLong(mAddressRegister[reg], res);
			}
			break;
		case 3:									// (An)+
			if(size == Size::BYTE) {
				writeByte(mAddressRegister[reg], res);
				mAddressRegister[reg] += 1;
			} else if(size == Size::WORD) {
				writeWord(mAddressRegister[reg], res);
				mAddressRegister[reg] += 2;
			} else if(size == Size::LONG) {
				writeLong(mAddressRegister[reg], res);
				mAddressRegister[reg] += 4;
			}
			break;
		case 4:									// -(An)
			if(size == Size::BYTE) {
				mAddressRegister[reg] -= 1;
				writeByte(mAddressRegister[reg], res);
			} else if(size == Size::WORD) {
				mAddressRegister[reg] -= 2;
				writeWord(mAddressRegister[reg], res);
			} else if(size == Size::LONG) {
				mAddressRegister[reg] -= 4;
				writeLong(mAddressRegister[reg], res);
			}
			break;
		case 5:	{								// (d16, An)
				uint32_t displacement = readWord(mPC);
				mPC += 2;
				if(size == Size::BYTE) {				writeByte(mAddressRegister[reg] + displacement, res);
				} else if(size == Size::WORD) {	writeWord(mAddressRegister[reg] + displacement, res);
				} else if(size == Size::LONG) {	writeLong(mAddressRegister[reg] + displacement, res);
				}
			}
			break;
		case 6:									// (d8, An, Xn)
			if(size == Size::BYTE) {
			} else if(size == Size::WORD) {
			} else if(size == Size::LONG) {
			}
			break;
		case 7:
			switch(reg) {
				case 0:	{						// (xxx).W
						uint16_t absoluteShort = readWord(mPC);
						if(size == Size::BYTE) {				writeByte(absoluteShort, res);
						} else if(size == Size::WORD) {	writeWord(absoluteShort, res);
						} else if(size == Size::LONG) {	writeLong(absoluteShort, res);
						}
					}
					break;
				case 1: {							// (xxx).L
						uint32_t absoluteShort = readWord(mPC);
						if(size == Size::BYTE) {				writeByte(absoluteShort, res);
						} else if(size == Size::WORD) {	writeWord(absoluteShort, res);
						} else if(size == Size::LONG) {	writeLong(absoluteShort, res);
						}
					}
					break;
				case 2:							// (d16, PC)
					if(size == Size::BYTE) {
					} else if(size == Size::WORD) {
					} else if(size == Size::LONG) {
					}
					break;
				case 3:							// (d8, PC, Xn)
					if(size == Size::BYTE) {
					} else if(size == Size::WORD) {
					} else if(size == Size::LONG) {
					}
					break;
			}
			break;
	}
	return 0;
}

//
// for indirect with post/pre-increment/decrement we need to check A7 for special case when reading one byte,
// a7 can never be odd, so we need to add 2, all other cases 1 *for byte-read*
//
uint32_t M68K::readData(uint8_t mode, uint8_t reg, uint8_t size) {
	uint32_t res = 0;
	switch(mode) {
		case 0:				// Dn
			res = maskValue(mDataRegister[reg], size);
			break;
		case 1:				// An
			res = maskValue(mAddressRegister[reg], size);
			break;
		case 2:				// (An)
			if(size == Size::BYTE) {				res = readByte(mAddressRegister[reg]);
			} else if(size == Size::WORD) {	res = readWord(mAddressRegister[reg]);
			} else if(size == Size::LONG) {	res = readLong(mAddressRegister[reg]);
			}
			break;
		case 3:				// (An)+
			if(size == 0) { // byte
				res = readByte(mAddressRegister[reg]);
				mAddressRegister[reg] += 1;
			} else if(size == Size::WORD) {
				res = readWord(mAddressRegister[reg]);
				mAddressRegister[reg] += 2;
			} else if(size == Size::LONG) {
				res = readLong(mAddressRegister[reg]);
				mAddressRegister[reg] += 4;
			}
			break;
		case 4:				// -(An)
			if(size == Size::BYTE) {
				mAddressRegister[reg] -= 1;
				res = readByte(mAddressRegister[reg]);
			} else if(size == Size::WORD) {
				mAddressRegister[reg] -= 2;
				res = readWord(mAddressRegister[reg]);
			} else if(size == Size::LONG) {
				mAddressRegister[reg] -= 4;
				res = readLong(mAddressRegister[reg]);
			}
			break;
		case 5:	{			// (d16, An)
				uint32_t displacement = readWord(mPC);
				mPC += 2;
				if(size == Size::BYTE) {				res = readByte(mAddressRegister[reg] + displacement);
				} else if(size == Size::WORD) {	res = readWord(mAddressRegister[reg] + displacement);
				} else if(size == Size::LONG) {	res = readLong(mAddressRegister[reg] + displacement);
				}
			}
			break;
		case 6:				// (d8,An,Xn) -- Address Register Indirect with Index (Base Displacement) Mode
//			uint16_t ext = readWord(mPC);
//			mPC += 2;

			//uint8_t	extReg					= (ext >> 12) & 0x7;
			//bool		extSize					= (ext >> 11) & 0x1;
			//uint8_t extScale				= (ext >> 9) & 0x3;
			//int8_t	extDisplacement	= (ext) & 0xff;

// 3D 73 40 02 00 12		move.w 2(a3, d4.w), $12(a6)
//
// instruction: 0010 1101 0111 0011 = 0x3d73
//   extension: 0100 0000 0000 0010 = 0x4002
//          d8: 0000 0000 0001 0010	= 0x12
//
// ext: ARRRBSS0DDDDDDDD
// A = Data/Address (0=Dn, 1=An)
// R = RegNum
// B = Word/Long
// SS = Scale; 00=1; 01=2; 10=4; 11=8
// D = 8bit-displacement (else we read another word)..



			break;

		case 7:
			switch(reg) {
				case 0:		// (xxx).W
					res = readWord(mPC);
					mPC += 2;
					if(size == Size::BYTE) {				res = readByte(res);
					} else if(size == Size::WORD) {	res = readWord(res);
					} else if(size == Size::LONG) {	res = readLong(res);
					}
					break;
				case 1:		// (xxx).L
					res = readLong(mPC);
					mPC += 4;
					if(size == Size::BYTE) {				res = readByte(res);
					} else if(size == Size::WORD) {	res = readWord(res);
					} else if(size == Size::LONG) {	res = readLong(res);
					}
					break;
				case 2:		// (d16, PC)		14(PC)
					res = int16_t(readWord(mPC));
					res += mPC;
					mPC += 2;
					break;
				case 3:		// (d8, PC, Xn)		14(PC, Xn)
//
// move.L 44(PC, Xn * scale)
//
// extension word format:
// D/A = 15
// Register = 14-12
// W/L = 11
// SCALE = 10-9
// 0 = 8
// DISPLACEMENT = 7-0
//
					if(size == Size::WORD) {
					} else if(size == Size::WORD) {
					} else if(size == Size::LONG) {
					}
					break;
				case 4:		// #imm
					if(size == Size::BYTE) {
						res = readWord(mPC) & 0xff;
						mPC += 2;
					} else if(size == Size::WORD) {
						res = readWord(mPC);
						mPC += 2;
					} else if(size == Size::LONG) {
						res = readLong(mPC);
						mPC += 4;
					}
					break;
			}
			break;
	}
	return res;
}


