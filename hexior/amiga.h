#pragma once

#include <vector>

#include "memory.h"
#include "m68k.h"
#include "cia.h"

class Amiga {
	public:
		Amiga();
		~Amiga();

//	private:
		Cia		mCIAA;
		Cia		mCIAB;
		M68K	mM68K;

		std::vector<Memory> mMemory;

};

