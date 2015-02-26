#include <QDir>
#include <QFile>

#include "amiga.h"

#include "memory.h"
#include "m68k.h"
#include "cia.h"

#include <vector>

Amiga::Amiga() {
	kernelAddress			= 0x01000;
	userStack					= 0x08500;
	supervisorStack		= 0x08f00;
	moduleNameAddress	= 0x00400;
	playerAddress			= 0x09000;

	mMemory.alloc(0x0, 0x400000);

	mM68K.initialize(&mMemory.mMemoryList);

	mM68K.mAddressRegister[7] = userStack;
	mM68K.mSSP								= supervisorStack;
	mM68K.writeLong(0x4, kernelAddress);
	mM68K.writeLong(0x4, playerAddress + 0x20 + 0x1b4);			// test! where to start execution...
	loadFile("Soundtracker-IV", playerAddress);

	mM68K.reset();
}

Amiga::~Amiga() {
}

#define getByte(x) uint8_t(data->at(x))
#define getWord(x) (uint8_t(data->at(x)) << 8 | uint8_t(data->at(x+1)))
#define getLong(x) (uint8_t(data->at(x)) << 24 | uint8_t(data->at(x+1)) << 16 | uint8_t(data->at(x+2)) << 8 | uint8_t(data->at(x+3)))

bool Amiga::relocate(QByteArray *data) {
	uint32_t codeStart = 0;
	uint32_t codeLength = 0;
	uint32_t reloc32Start = 0;
	uint32_t reloc32Offsets = 0;
	uint32_t reloc32Data = 0;

	uint32_t offset = 0;
	if(getLong(offset) != 0x3f3) return false;
	offset = 0x18;

	offset += 4;
	codeLength = getLong(offset) << 2;
	offset += 4;
	codeStart = offset;
	offset += codeLength;
	reloc32Start = offset;
	offset += 4;
	reloc32Offsets = getLong(offset);
	offset += 8;
	reloc32Data = offset;

	char *rawData = data->data();

	for(int i = 0; i < reloc32Offsets; ++i) {
		uint32_t relocOffset = codeStart + getLong(reloc32Data);
		reloc32Data += 4;

		uint32_t newAddress = getLong(relocOffset) + playerAddress + 0x20;		// 0x20 = offset hunk's

		rawData[relocOffset] = newAddress >> 24;
		rawData[relocOffset + 1] = newAddress >> 16 & 0xff;
		rawData[relocOffset + 2] = newAddress >> 8 & 0xff;
		rawData[relocOffset + 3] = newAddress & 0xff;
	}

	return true;
}

uint32_t Amiga::loadFile(QString filename, uint32_t offset) {
	QByteArray temp;
	QString foo = QDir::currentPath() + QDir::separator() + filename;
	QFile file(foo);
	file.open(QIODevice::ReadOnly);
	uint32_t filesize =	file.size();
	temp = file.readAll();
	relocate(&temp);
	for(int i = 0 ; i < temp.size() ; ++i) {
		mM68K.writeByte(offset + i, temp.at(i));
	}
	file.close();
	return filesize;
}

