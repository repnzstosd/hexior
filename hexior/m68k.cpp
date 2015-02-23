#include "m68k.h"

M68K::M68K() {
}

M68K::~M68K() {
}

void M68K::initialize(std::vector<Memory::mem> *memoryList) {
	mMemory = memoryList;
	for(int i = 0; i < 8; ++i) {
		mDataRegister[i] = mAddressRegister[i] = 0;
	}
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

					switch(mode) {
						case 0: // Dn
							if(size == 0) destination = mDataRegister[reg] & 0xff;
							if(size == 1) destination = mDataRegister[reg] & 0xffff;
							if(size == 2) destination = mDataRegister[reg];
							break;

						case 2: // (An)
							if(size == 0) destination = readWord(mAddressRegister[reg]) & 0xff;
							if(size == 1) destination = readWord(mAddressRegister[reg]);
							if(size == 2) destination = readLong(mAddressRegister[reg]);
							break;

						case 3: // (An)+
							if(size == 0) { destination = readByte(mAddressRegister[reg]) & 0xff;		mAddressRegister[reg] += 1; }
							if(size == 1) { destination = readWord(mAddressRegister[reg]);					mAddressRegister[reg] += 2; }
							if(size == 2) { destination = readLong(mAddressRegister[reg]);					mAddressRegister[reg] += 4; }
							break;

						case 4: // -(An)
							if(size == 0) { mAddressRegister[reg] -= 1; destination = readByte(mAddressRegister[reg]) & 0xff;	 }
							if(size == 1) { mAddressRegister[reg] -= 2; destination = readWord(mAddressRegister[reg]); }
							if(size == 2) { mAddressRegister[reg] -= 4; destination = readLong(mAddressRegister[reg]); }
							break;

						case 5: // (d16,An)
							break;

						case 6: // (d8, An, Xn)
							break;

						case 7: // (misc.. have fun...)
							break;
					}

					if(size == 0) { source = readWord(mPC) & 0xff;	mPC += 2; }
					if(size == 1) { source = readWord(mPC);					mPC += 2; }
					if(size == 2) { source = readLong(mPC);					mPC += 4; }

					int32_t res = destination - source;

					mSR.CCR &= ~(SRF::carry | SRF::negative | SRF::zero | SRF::overflow);
					if(uint32_t(res) > uint32_t(destination)) mSR.CCR |= SRF::carry;

					if(!(((source < 0) && (destination < 0)) || ((source > 0) && (destination > 0)))) {
						if(((res > 0) && (destination < 0)) || ((res < 0) && (destination > 0))) {
							mSR.CCR |= SRF::overflow;
						}
					}

					if(res == 0) mSR.CCR |= SRF::zero;
					if(res < 0) mSR.CCR |= SRF::negative;



			}
			break;

		case 0x1:			// MOVE .B
		case 0x2:	{		// MOVE .L
				uint8_t		destRegister		= (instruction >> 9)	& 7;
				uint8_t		destMode				= (instruction >> 6)	& 7;
				uint8_t		sourceMode			= (instruction >> 3)	& 7;
				uint8_t		sourceRegister	= (instruction)				& 7;
				uint32_t	res							= 0;

				if(sourceMode == 0x0) {					// Dn
					res = mDataRegister[sourceRegister];
				} else if(sourceMode == 0x1) {	// An
					res = mAddressRegister[sourceRegister];
				} else if(sourceMode == 0x2) {	// (an)
					res = readLong(mAddressRegister[sourceRegister]);
				} else if(sourceMode == 0x3) {	// (an)+
					res = readLong(mAddressRegister[sourceRegister]);
					mAddressRegister[sourceRegister] += 4;
				} else if(sourceMode == 0x4) {	// -(an)
					mAddressRegister[sourceRegister] -= 4;
					res = readLong(mAddressRegister[sourceRegister]);
				} else if(sourceMode == 0x5) {	// (d16,An)
					uint32_t displacement = readWord(mPC);
					mPC += 2;
					res = readLong(mAddressRegister[sourceRegister] + displacement);
				} else if(sourceMode == 0x6) {	// (d8,An,Xn)
				} else if((sourceMode == 0x7) && (sourceRegister == 0x0)) {	// (xxx).W
					res = readWord(mPC);
					mPC += 2;
					res &= 0xffff;
					res = readLong(res);
				} else if((sourceMode == 0x7) && (sourceRegister == 0x1)) {	// (xxx).L
					res = readLong(mPC);
					mPC += 4;
					res = readLong(res);
				} else if((sourceMode == 0x7) && (sourceRegister == 0x2)) {	// (d16, PC)
				} else if((sourceMode == 0x7) && (sourceRegister == 0x3)) {	// (d8, PC, Xn)
				} else if((sourceMode == 0x7) && (sourceRegister == 0x4)) {	// #imm
					res = readLong(mPC);
					mPC += 4;
				}

				if(destMode == 0x0) {					// Dn
					mDataRegister[destRegister] = res;
				} else if(destMode == 0x1) {	// An
					mAddressRegister[destRegister] = res;
				} else if(destMode == 0x2) {	// (an)
					writeLong(mAddressRegister[destRegister], res);
				} else if(destMode == 0x3) {	// (an)+
					writeLong(mAddressRegister[destRegister], res);
					mAddressRegister[destRegister] += 4;
				} else if(destMode == 0x4) {	// -(an)
					mAddressRegister[destRegister] -= 4;
					writeLong(mAddressRegister[destRegister], res);
				} else if(destMode == 0x5) {	// (d16,An)
					uint32_t displacement = readWord(mPC);
					mPC += 2;
					writeLong(mAddressRegister[destRegister] + displacement, res);
				} else if(destMode == 0x6) {	// (d8,An,Xn)
				} else if((destMode == 0x7) && (destRegister == 0x0)) {	// (xxx).W
				} else if((destMode == 0x7) && (destRegister == 0x1)) {	// (xxx).L
				} else if((destMode == 0x7) && (destRegister == 0x2)) {	// (d16, PC)
				} else if((destMode == 0x7) && (destRegister == 0x3)) {	// (d8, PC, Xn)
				}


			}
			break;
		case 0x3:			// MOVE .W


		case 0x4:
			// NEGX, MOVE from SR, CHK, LEA, CLR, NEG, MOVE to CCR, NOT, MOVE to SR, NBCD, PEA, SWAP, MOVEM, EXT, TST, TAS, TRAP, LINK, UNLK, MOVE USP, JSR, JMP
			switch (instruction & 0xfb80) {

				case 0x4e80: {	// JSR
						uint8_t mode = (instruction >> 3) & 0x7;
						uint8_t reg = (instruction) & 0x7;

						if(mode == 2) {													// (An)
							mAddressRegister[7] -= 4;
							writeLong(mAddressRegister[7], mPC);
							mPC = readLong(mAddressRegister[reg]);
						} else if(mode == 5) {									// (d16,An)
						} else if(mode == 6) {									// (d8, An, Xn)
						} else if((mode == 7) && (reg == 0)) {	// (xxx).W
						} else if((mode == 7) && (reg == 1)) {	// (xxx).L

						} else if((mode == 7) && (reg == 2)) {	// (d16, PC)
						} else if((mode == 7) && (reg == 3)) {	// (d8, PC, Xn)
						}
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

		case 0x5:
			// ADDQ, Scc, DBcc, SUBQ
			break;

		case 0x6:	{	// Bcc, BRA, BSR

			bool jump = false;

			switch (instruction & 0x6f00) {

/*

The following routine returns whether to take the branch or not, universal for bsr, bra, bcc

Switch on instruction >> 8 & f

// truth table...
	case 0: return	true;
	case 1: return	false;
	case 2: return	!(mSR.CCR & SRF::carry) & !(mSR.CCR & SRF::zero);
	case 3: return	 (mSR.CCR & SRF::carry) |  (mSR.CCR & SRF::zero);
	case 4: return	!(mSR.CCR & SRF::carry);
	case 5: return	 (mSR.CCR & SRF::carry);
	case 6: return	!(mSR.CCR & SRF::zero);
	case 7: return	 (mSR.CCR & SRF::zero);
	case 8: return	!(mSR.CCR & SRF::overflow);
	case 9: return	 (mSR.CCR & SRF::overflow);
	case 10: return	!(mSR.CCR & SRF::negative);
	case 11: return	 (mSR.CCR & SRF::negative);
	case 12: return	!((mSR.CCR & SRF::negative) ^ (mSR.CCR & SRF::overflow));
	case 13: return		(mSR.CCR & SRF::negative) ^ (mSR.CCR & SRF::overflow);

	case 14: return ( (mSR.CCR & SRF::negative) &  (mSR.CCR & SRF::overflow) & !(mSR.CCR & SRF::zero)) |
									(!(mSR.CCR & SRF::negative) & !(mSR.CCR & SRF::overflow) & !(mSR.CCR & SRF::zero));

	case 15: return (mSR.CCR & SRF::zero) | ( (mSR.CCR & SRF::negative) & !(mSR.CCR & SRF::overflow)) |
																					(!(mSR.CCR & SRF::negative) &  (mSR.CCR & SRF::overflow));



*/



				case 0x6000: { // BRA
						int16_t displacement = 0;
						if(!(displacement = instruction & 0xff)) {
							displacement = readWord(mPC);
						} else {
							if(displacement & 0x80) displacement |= 0xffffff00;
						}
						mPC += displacement;
					}
					break;

				case 0x6100: {	// BSR (complete)
						uint16_t displacement = 0;
						mAddressRegister[7] -= 4;
						if(!(displacement = instruction & 0xff)) {
							displacement = readWord(mPC);
							writeLong(mAddressRegister[7], mPC + 2);
						} else {
							if(displacement & 0x80) displacement |= 0xffffff00;
							writeLong(mAddressRegister[7], mPC);
						}
						mPC += displacement;
					}
					break;

				case 0x6200:	// BHI
				case 0x6e00:	// BGT
				case 0x6a00:	// BPL
					if(!((mSR.CCR & SRF::negative) | (mSR.CCR & SRF::zero))) {
						branch(instruction);
					} else {
						if(!(instruction & 0xf)) mPC += 2;
					}
					break;

				case 0x6300:	// BLS
				case 0x6f00:	// BLE
					break;

				case 0x6400:	// BCC
					break;

				case 0x6500:	// BCS
					break;

				case 0x6600:	// BNE
					if(!(mSR.CCR & SRF::zero)) {
						branch(instruction);
					} else {
						if(!(instruction & 0xf)) mPC += 2;
					}
					break;

				case 0x6700:	// BEQ
					if(mSR.CCR & SRF::zero) {
						branch(instruction);
					} else {
						if(!(instruction & 0xf)) mPC += 2;
					}
					break;

				case 0x6800:	// BVC
					break;

				case 0x6900:	// BVS
					break;

				case 0x6b00:	// BMI
				case 0x6d00:	// BLT
					break;

				case 0x6c00:	// BGE
					break;
			}

			break;
		}

		case 0x7:	{		// MOVEQ
				uint8_t reg = uint8_t((instruction >> 9) & 7);
				uint32_t data = instruction & 0xff;
				if(data & 0x80) data = data | 0xffffff00;
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
					uint8_t destRegister		= (instruction >> 9) & 7;
					uint8_t operationSize		= (instruction >> 6) & 3;
					uint8_t sourceMode			= (instruction >> 3) & 7;
					uint8_t sourceRegister	= instruction & 7;

					int32_t source = 0;
					int32_t destination = 0;
					int32_t res = 0;

					if(operationSize == 0) destination	= mDataRegister[destRegister] & 0xff;
					if(operationSize == 1) destination	= mDataRegister[destRegister] & 0xffff;
					if(operationSize == 2) destination	= mDataRegister[destRegister];


					if(sourceMode == 0) {	// Dn
						if(operationSize == 0) source				= mDataRegister[sourceRegister] & 0xff;
						if(operationSize == 1) source				= mDataRegister[sourceRegister] & 0xffff;
						if(operationSize == 2) source				= mDataRegister[sourceRegister];
					}


					res = destination - source;

					mSR.CCR &= ~(SRF::carry | SRF::negative | SRF::zero | SRF::overflow);
					if(uint32_t(res) > uint32_t(destination)) mSR.CCR |= SRF::carry;

					if(!(((source < 0) && (destination < 0)) || ((source > 0) && (destination > 0)))) {
						if(((res > 0) && (destination < 0)) || ((res < 0) && (destination > 0))) {
							mSR.CCR |= SRF::overflow;
						}
					}

					if(res == 0) mSR.CCR |= SRF::zero;
					if(res < 0) mSR.CCR |= SRF::negative;


//					if(data < 0)	mSR.SR |= uint16_t(SRF::negative | SRF::carry);
//					if(data == 0)	mSR.SR |= uint16_t(SRF::zero);
//					if(data == overflow???
					break;
			}
		case 0xc:
			// AND, MULU, EXG, ABCD, MULS

		case 0xd:
			// ADD, ADDA, ADDX

			switch (instruction & 0xf0c0) {	// 0x1111 0000 1100 0000
				case 0xd0c0:	// ADDA
					uint8_t destRegister		= (instruction >> 9) & 7;
					uint8_t operationSize		= (instruction >> 8) & 1;
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
			// ASd, LSd, ROXd, ROd

		case 0xf:
			// Fpu...

		return cycles;
	}

	switch(instruction) {
		case 0x003c:	// ORI to CCR
			break;

		case 0x007c:	// ORI to SR
			break;

		case 0x023c:	// ANDI to CCR
			break;

		case 0x027c:	// ANDI to SR
			break;

		case 0x0a2c:	// EORI to CCR
			break;

		case 0x0a7c:	// EORI to SR
			break;

		case 0x4afb:	// illegal
			break;

		case 0x4e70:	// reset
			cycles += 132;
			break;

		case 0x4e71:	// nop
			cycles += 4;
			break;

		case 0x4e72:	// stop
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
