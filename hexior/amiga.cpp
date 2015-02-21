#include "amiga.h"

#include "memory.h"
#include "m68k.h"
#include "cia.h"

Amiga::Amiga() {
	mMemory.push_back(Memory(0x0, 0x400000));
	mM68K.initialize(&mMemory);
}

Amiga::~Amiga() {
}
