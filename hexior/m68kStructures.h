#pragma once


struct VectorTable {
	uint32_t	resetInitialSSP;
	uint32_t	resetInitialPC;
	uint32_t	accessFault;
	uint32_t	addressError;
	uint32_t	illegalInstruction;
	uint32_t	intDivideByZero;
	uint32_t	ChkInstruction;			// Both CHK and CHK2
	uint32_t	trapV;							// FTRAPcc TRAPcc and TRAPV
	uint32_t	privilegeViolation;
	uint32_t	trace;
	uint32_t	lineAEmulator;
	uint32_t	lineFEmulator;
	uint32_t	reserved12;
	uint32_t	coprocessorProtocolViolation;
	uint32_t	formatError;
	uint32_t	unitinializedInterrupt;
	uint32_t	reserved16;
	uint32_t	reserved17;
	uint32_t	reserved18;
	uint32_t	reserved19;
	uint32_t	reserved20;
	uint32_t	reserved21;
	uint32_t	reserved22;
	uint32_t	reserved23;
	uint32_t	spuriousInterrupt;
	uint32_t	level1Interrupt;
	uint32_t	level2Interrupt;
	uint32_t	level3Interrupt;
	uint32_t	level4Interrupt;
	uint32_t	level5Interrupt;
	uint32_t	level6Interrupt;
	uint32_t	level7Interrupt;
	uint32_t	trap0;
	uint32_t	trap1;
	uint32_t	trap2;
	uint32_t	trap3;
	uint32_t	trap4;
	uint32_t	trap5;
	uint32_t	trap6;
	uint32_t	trap7;
	uint32_t	trap8;
	uint32_t	trap9;
	uint32_t	trap10;
	uint32_t	trap11;
	uint32_t	trap12;
	uint32_t	trap13;
	uint32_t	trap14;
	uint32_t	trap15;
	uint32_t	fpBranch;					// FP Branch or Set on Unordered Condition
	uint32_t	fpInexact;
	uint32_t	fpDivByZero;
	uint32_t	fpUnderflow;
	uint32_t	fpOperandError;
	uint32_t	fpOverflow;
	uint32_t	fpSignalingNAN;
	uint32_t	fpUnimplementedDataType;
	uint32_t	mmuConfigurationError;
	uint32_t	mmuIllegalOperationError;
	uint32_t	mmuAccessLevelViolationError;
	uint32_t	reserved59;
	uint32_t	reserved60;
	uint32_t	reserved61;
	uint32_t	reserved62;
	uint32_t	reserved63;
	uint32_t	userDefinedVectors[192];	// 64-255
};

enum Interrupt {
	USER_VECTOR,
	AUTO_VECTOR,
	SPURIOUS,
	UNINITIALIZED
};

enum Flags {
	ADD,
	ADDX,
	CMP,
	LOGICAL,
	SUB,
	SUBX,
	ZN
};

enum Size {
	BYTE = 0,
	WORD = 1,
	LONG = 2
};

/*

addrbank fastmem_bank = {
	fastmem_lget, fastmem_wget, fastmem_bget,
	fastmem_lput, fastmem_wput, fastmem_bput,
	fastmem_xlate, fastmem_check, NULL, _T("fast"), _T("Fast memory"),
	fastmem_lget, fastmem_wget, ABFLAG_RAM | ABFLAG_THREADSAFE
};

memoryBank	*memBanks[65536];			// entire 32bit memory, 4gb.
uint8_t			*baseAddress[65536];	// baseAddress if even, else if bit 0 is set, its the same pointer as memBanks and is a bank that has baseaddress=0 (no allocated memory).


mem_banks[bankindex(addr)] = &fastmem_bank;		// adds a pointer to a memorybank to the mem_banks-array, so we can use the memory..


	uint8_t *m;
	addr -= name_bank.start & name_bank.mask;
	addr &= name_bank.mask;
	m = name_bank.baseaddr+addr;
	do_put_mem_word((uint16_t *)m, w);

static void do_put_mem_long(void *a, uae_u32 v) {
       uae_u8 *b = (uae_u8 *)a;

       b[0] = v >> 24;
       b[1] = v >> 16;    
       b[2] = v >> 8;
       b[3] = v;
}
*/


struct RegisterState {
	union {
		uint16_t	SR;
		struct {
			uint8_t	CCR;
			uint8_t h;
		};
		struct {
			bool		carry				: 1;
			bool		overflow		: 1;
			bool		zero				: 1;
			bool		negative		: 1;
			bool		extend			: 1;
			bool		notused5		: 1;
			bool		notUsed6		: 1;
			bool		notUsed7		: 1;
			uint8_t	interrupt		: 3;
			bool		notUsed11		: 1;
			bool		notUsed12		: 1;
			bool		supervisor	: 1;
			bool		notUsed14		: 1;	// for 68000, 68010, CPU32 this is always zero, else it's trace0
			bool		trace				: 1;
		};
	};
	inline operator uint16_t() const { return SR; }
	inline uint16_t operator  = (uint16_t data) { return SR = data; }
	inline uint16_t operator |= (uint16_t data) { return SR |= data; }
	inline uint16_t operator ^= (uint16_t data) { return SR ^= data; }
	inline uint16_t operator &= (uint16_t data) { return SR &= data; }
	RegisterState() : SR(0) {}
};

struct Register32 {
	union {
		uint32_t d;
		struct { uint16_t w, noUseW; };
		struct { uint8_t l, noUseH, noUseLb, noUseHb; };
	};
	inline uint32_t operator  = (uint32_t data) { return d = data; }
	inline uint32_t operator -= (uint32_t data) { return d -= data; }
	inline uint32_t operator += (uint32_t data) { return d += data; }
	inline uint32_t operator |= (uint32_t data) { return d |= data; }
	inline operator uint32_t() const { return d; }
	Register32() : d(0) {}
};

typedef uint32_t (*readMemFunc)(uint32_t offset);
typedef uint32_t (*writeMemFunc)(uint32_t offset);
typedef uint32_t (*checkMemFunc)(uint32_t offset);	// check if present.

struct memoryBank {
	readMemFunc		readByte, readWord, readLong;
	writeMemFunc	writeByte, writeWord, writeLong;
	// translate function															// return a uint8_t *mem; pointer that we can use to extract memory without using the normal read/write functions. (debugging et.c)
	// check function																	// returns bool, if we can read/write to this address. if(memBank.check(0x490000)) {...
	uint8_t				*baseAddress;												// where to write is calculated: uint8_t *destAddr = ((addr - memoryBank.start) & memoryBank.mask) + baseAddress;
	const char		*label;															// "batmem"
	const char		*name;															// "Battery backed up clock (MSM6242B)"
//	readMemFunc		readWordInstr, readLongInstr;			// for opcode/operand fetches
	uint32_t			flags;
	uint32_t			mask;																// mask is used to mask the pointer to be inside the allocation. ie  mask = .size-1 as size should be dividable by 64k
	uint32_t			start;
	uint32_t			memSize;
};

