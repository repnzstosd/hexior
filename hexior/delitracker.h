#pragma once

#include <stdint.h>

class Delitracker {
	public:
		Delitracker(void);
		~Delitracker(void);

	struct DeliTrackerGlobals {
		uint32_t	AslBase;								// Librarybase don't CloseLibrary();
		uint32_t	IntuitionBase;					// Librarybase don't CloseLibrary();
		uint32_t	GfxBase;								// Librarybase don't CloseLibrary();
		uint32_t	GadToolsBase;						// Librarybase (NULL for Kick 1.3 and below)
		uint32_t	ReservedLibraryBase;		// Reserved for future use.
		// 20 0x14
		uint32_t	DirArrayPtr;						// Ptr to the directory of the current module
		uint32_t	FileArrayPtr;						// Ptr to the filename of the current module
		uint32_t	PathArrayPtr;						// Ptr to PathArray (e.g used in LoadFile())
		// 32 0x20
		uint32_t	ChkData;								// pointer to the module to be checked
		uint32_t	ChkSize;								// size of the module
		// 40 0x28
		uint16_t	SndNum;									// current sound number
		uint16_t	SndVol;									// Volume (range from 0 to 64)
		uint16_t	SndLBal;								// Left Volume (range from 0 to 64)
		uint16_t	SndRBal;								// Right Volume (range from 0 to 64)
		uint16_t	SndLED;									// Filter (0 if the LED is off)
		uint16_t	Timer;									// timer-value for the CIA-Timers
		// 64 0x40
		uint32_t	GetListData;						// function pointer.
		uint32_t	LoadFile;								//
		uint32_t	CopyDir;
		uint32_t	CopyFile;
		uint32_t	CopyString;
		uint32_t	AudioAlloc;
		uint32_t	AudioFree;
		uint32_t	StartInt;
		uint32_t	StopInt;
		uint32_t	SongEnd;								// Save to call from interrupt code!
		uint32_t	CutSuffix;

		//-- extension in revision 14
		// 108 0x6c
		uint32_t	SetTimer;								// save to call from interrupt code!

		//-- extension in revision 15
		// 112 0x70
		uint32_t	WaitAudioDMA;						// Save to call from interrupt code!

		//-- extension in revision 15
		// 116 0x74
		uint32_t	LockScreen;
		uint32_t	UnlockScreen;
		uint32_t	NotePlayer;							// save to call from interrupt code!
		uint32_t	AllocListData;
		uint32_t	FreeListData;
		// 136 0x88
		uint32_t	Reserved1;
		uint32_t	Reserved2;
		uint32_t	Reserved3;

		/*
		 GetListData(Num:d0): This function returns the memorylocation of a loaded file in a0 and its size in d0. Num starts with 0
		 (the selected module). Example: GetListData(2) returns the start of the third file loaded (via ExtLoad) in a0 an its size in d0.

		 LoadFile(): this function may only be called in the ExtLoad routine. file/pathname must be in dtg_PathArrayPtr then
		 this function will attempt to load the file into CHIPMEM (and DECRUNCH it). If everything went fine, d0 will be zero.
		 If d0 is not zero this indicates an error (e.g. read error, not enough memory, ...).

		 CopyDir(): this function copies the pathname at the end of the string in dtg_PathArrayPtr(a5).

		 CopyFile(): this function copies the filename at the end of the string in dtg_PathArrayPtr(a5).

		 CopyString(Ptr:a0): this function copies the string in a0 at the end of the string in dtg_PathArrayPtr(a5).

		 AudioAlloc(): this function allocates the audiochannels (only necessary if the player doesn't supply a NoteStruct
		 tag). If d0=0 all is ok, d0<>0 indicates an error.

		 AudioFree(): this function frees the audiochannels allocated with AudioAlloc().

		 StartInt(): this function starts the timer-interrupt.

		 StopInt(): this function stops the timer-interrupt started with StartInt().

		 SongEnd(): signal the songend to DeliTracker. This call is guaranteed to preserve all registers.

		 CutSuffix(): this function removes the suffix '.xpk' or '.pp' from the string in dtg_PathArrayPtr(a5).

		 SetTimer(): programs the CIA-Timer with the value supplied in dtg_Timer(a5). Only useful, if the internal timer-interrupt
		 is used. This call is guaranteed to preserve all registers.

		 WaitAudioDMA(): DMA delay wait. Only allowed, if the internal timer-interrupt is used. This call is guaranteed to preserve all registers.

		 LockScreen(): this function tries to lock DeliTracker's screen. It returns the screenpointer in d0 or NULL on failure.

		 UnlockScreen(): this function unlocks DeliTracker's screen. do not unlock a screen more times than it was locked!

		 NotePlayer(): this call plays the notes specified in the current NoteStruct structure. This function call is not allowed
		 if the active player doesn't have a valid NotePlayer structure. do not call this function in interrupt code at interrupt
		 level 5 or higher! This call is guaranteed to preserve all registers.

		 AllocListData(Size:d0/Flags:d1): This is the memory allocator for module specific memory to be used by all players and genies.
		 It provides a means of specifying that the allocation should be made in a memory area accessible to the chips, or accessible to
		 shared system memory. If the allocation is successful, DeliTracker will keep track of the new block (GetListData() will
		 return the location and size of this block). byteSize - the size of the desired block in bytes.
		 Flags - the flags are passed through to AllocMem(). A pointer to the newly allocated memory block is returned in d0.
		 If there are no free memory regions large enough to satisfy the request, zero will be returned. The pointer must be checked
		 for zero before the memory block may be used!

		 FreeListData(MemBlock:a1): Free a region of memory allocated with AllocListData(), returning it to the system pool from which
		 it came. memoryBlock - pointer to the memory block to free, or NULL.
		*/

	};

};

