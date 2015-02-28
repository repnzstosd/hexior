#include <QDir>
#include <QFile>

#include "amiga.h"

#include "memory.h"
#include "m68k.h"
#include "cia.h"

#include <vector>

/*
** Amiga Memory Map (A3000)
**
**  Address Range                Description
** --------------------------   ---------------------------------
** 0x0000 0000 - 0x001f ffff     Amiga Chip Memory
** 0x0020 0000 - 0x009f 0000     Zorro II Memory Expansion Space
** 0x00a0 0000 - 0x00b7 ffff     Zorro II I/O Expansion Space
** 0x00b8 0000 - 0x00be ffff     Reserved
** 0x00bf 0000 - 0x00bf ffff     CIA Ports & Timers
** 0x00c0 0000 - 0x00c7 ffff     Expansion Memory
** 0x00c8 0000 - 0x00d7 ffff     Reserved
** 0x00d8 0000 - 0x00db ffff     Reserved
** 0x00dc 0000 - 0x00dc ffff     Memory Mapped Clock
** 0x00dd 0000 - 0x00dd ffff     SCSI Control
** 0x00de 0000 - 0x00de ffff     Motherboard Resources
** 0x00df 0000 - 0x00df ffff     Amiga Chip Registers
** 0x00e0 0000 - 0x00e7 ffff     Reserved
** 0x00e8 0000 - 0x00ef ffff     Zorro II I/O & configuration
** 0x00f0 0000 - 0x00f7 ffff     Diagnostic ROM (Reserved)
** 0x00f8 0000 - 0x00ff ffff     High ROM (512K)

**
** MC68000 cant address above 0x00ff ffff
**

** 0x0100 0000 - 0x03ff ffff     Reserved
** 0x0400 0000 - 0x07ff ffff     Motherboard Fast Ram
** 0x0800 0000 - 0x0fff ffff     Coprocessor Slot Expansion
** 0x1000 0000 - 0x7fff ffff     Zorro III Expansion
** 0x8000 0000 - 0xfeff ffff     Reserved
** 0xff00 0000 - 0xff00 ffff     Zorro III Configuration Unit
** 0xff01 0000 - 0xffff ffff     Reserved

Type:                         Start:     Size:      Blocks:
----------------------------- ---------  ---------- --------
           Amiga chip memory: 0x00000000 0x200000 = 0x20
   Zorro II Memory Expansion: 0x00200000 0x800000 = 0x80
Zorro II I/O Expansion Space: 0x00a00000 0x180000 = 0x18
                    Reserved: 0x00b80000  0x70000 = 0x07
          CIA Ports & Timers: 0x00bf0000  0x10000 = 0x01
            Expansion Memory: 0x00c00000  0x80000 = 0x08
                    Reserved: 0x00c80000 0x130000 = 0x13
         Memory Mapped Clock: 0x00dc0000  0x10000 = 0x01
                SCSI Control: 0x00dd0000  0x10000 = 0x01
       Motherboard Resources: 0x00de0000  0x10000 = 0x01
        Amiga Chip Registers: 0x00df0000  0x10000 = 0x01
                    Reserved: 0x00e00000  0x80000 = 0x08
Zorro II I/O & Configuration: 0x00e80000  0x80000 = 0x08
   Diagnostic ROM (Reserved): 0x00f00000  0x80000 = 0x08
             High ROM (512K): 0x00f80000  0x80000 = 0x08
                       Total:                       0xff

one horizontal line takes 63us 227.5 color clocks

every line, the audio DMA get 2 words per channel = 16 bytes.

262 lines NTSC
312 lines PAL

227 cycles per line


*/
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
	mM68K.writeLong(0x4, playerAddress + 0x20 + 0x1d6);			// test! where to start execution...
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

