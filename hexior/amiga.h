#pragma once

#include <vector>

#include "memory.h"
#include "m68k.h"
#include "cia.h"

#include <QString>

class Amiga {
	public:
		uint32_t kernelAddress;
		uint32_t userStack;
		uint32_t supervisorStack;
		uint32_t moduleNameAddress;
		uint32_t playerAddress;

	public:
		Amiga();
		~Amiga();

		uint32_t loadFile(QString filename, uint32_t offset);

//	bool	loadPlayer();
//	bool	moduletype();
//	bool	loadMusic();
//	bool	loadConfig();



//	private:
		Cia		mCIAA;
		Cia		mCIAB;
		M68K	mM68K;

		Memory mMemory;
};

