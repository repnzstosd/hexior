#include <QDir>
#include <QFile>

#include "amiga.h"

#include "memory.h"
#include "m68k.h"
#include "cia.h"


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
	mM68K.writeLong(0x4, playerAddress + 0x20);
	loadFile("Soundtracker-IV", playerAddress);

	mM68K.reset();
}

Amiga::~Amiga() {
}

uint32_t Amiga::loadFile(QString filename, uint32_t offset) {
	QByteArray temp;
	QString foo = QDir::currentPath() + QDir::separator() + filename;
	QFile file(foo);
	file.open(QIODevice::ReadOnly);
	uint32_t filesize =	file.size();
	temp = file.readAll();
	for(int i = 0 ; i < temp.size() ; ++i) {
		mM68K.writeByte(offset + i, temp.at(i));
	}
	file.close();
	return filesize;
}

