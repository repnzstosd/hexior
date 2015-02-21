#include "m68k.h"

M68K::M68K() {
}

M68K::~M68K() {
}

void M68K::initialize(std::vector<Memory> *memoryList) {
	mMemory = memoryList;
	mSR.SR = 0;
	mPC = 0;
	mSSP = 0;
	for(int i = 0; i < 8; ++i) {
		mDataRegister[i] = 0;
		mAddressRegister[i] = 0;
	}
	reset();
}

void M68K::reset() {
	mSSP = readLong(0x0);
	mPC = readLong(0x4);

	mPC = 0x200020;				// REMOVE THIS

	mSR.SR = 0x2700;
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

int M68K::step() {
	int instructionLength = 2;
	uint16_t instruction = readWord(mPC);

	switch(instruction) {
		case 0x003c:	// ORI to CCR
		case 0x007c:	// ORI to SR
		case 0x4afb:	// illegal
		case 0x4e70:	// reset
		case 0x4e71:	// nop
		case 0x4e72:	// stop
		case 0x4e73:	// rte
		case 0x4e75:	// rts
		case 0x4e76:	// trapv
		case 0x4e77:	// rtr
	}

	switch(instruction & 0xffc0) {
		case 0x080:	// BTST
		case 0x084:	// BCHG
		case 0x088:	// BCLR
		case 0x08c:	// BSET
		case 0x40c:	// Move from SR
		case 0x44c:	// Move to CCR
		case 0x46c:	// Move to SR
		case 0x4ac:	// Tas
		case 0x4e8:	// jsr
		case 0x4ec:	// jmp
	}

	switch(instruction & 0xf1c) {
		case 0x010:	// BTST
		case 0x014:	// BCHG
		case 0x018:	// BCLR
		case 0x01a:	// BSET
		case 0x41c:	// Lea
		case 0x418:	// Chk
	}





	mPC += instructionLength;

	return 0;	// Number of cycles executed... <47
}
