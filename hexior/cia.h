#pragma once

#include <stdint.h>

struct CIAData {
		uint16_t	mIrqControlRegister;
		uint8_t		mTimerALow;
		uint8_t		mTimerAHigh;
		uint8_t		mTimerBLow;
		uint8_t		mTimerBHigh;


/*
pra		- peripheral data register A
prb		- peripheral data register B
ddra	- data direction register A
ddrb	- data direction register B
talo	- timer A low register
tahi	- timer A high register
tblo	- timer B low register
tbhi	- timer B high register
todlow- time of day low
todmid- time of day mid
todhi	- time of day high
-
sdr		- serial data register
icr		- interrupt control register
cra		- control register A
crb		- control register B

Each interval timer consists of a 16-bit read-only timer counter and a 16-bit write-only timer latch.
Data written to the timer is latched into the timer latch, while data read from the timer is the present contents of the timer counter.
Each timer has an associated control register, providing independent control over each of the following functions:

START/STOP
A control bit allows the timer to be started or stopped by the microprocessor at any time.

PB ON/OFF
A control bit allows the timer output to appear on a port B output line (PB6 for timer A and PB7 for timer B). This function overrides the DDRB
control bit and forces the appropriate PB line to become an output.

TOGGLE/PULSE
A control bit selects the output applied to port B while the PB on/off bit is ON. On every timer underflow, the output can either toggle or
generate a single positive pulse of one cycle duration. The toggle output is set high whenever the timer is started, and set low by RES.

ONE-SHOT/CONTINUOUS
A control bit selects either timer mode.
In one-shot mode, the timer will count down from the latched value to zero, generate an interrupt, reload the latched value, then stop.
In continuous mode, the timer will count down from the latched value to zero, generate an interrupt, reload the latched value, 
and repeat the procedure continuously. In one-shot mode, a write to timer-high (register 5 for timer A, register 7 for Timer B)will transfer the timer latch to the counter and initiate counting regardless of the start bit.
FORCE LOAD
A strobe bit allows the timer latch to be loaded into the timer counter at any time, whether the timer is running or not.

INPUT MODES
Control bits allow selection of the dock used to decrement the timer.
Timer A can count 02 clock pulses or external pulses applied to the CNT pin.
Timer B can count 02 pulses, external CNT pulses, timer A underflow pulses, or timer A underflow pulses while the CNT pin is held high.
The timer latch is loaded into the timer on any timer underflow, on a force load, or following a write to the high byte of the pre-scalar
while the timer is stopped. If the timer is running, a write to the high byte will load the timer latch but not the counter.

BIT NAMES on READ-Register

REG NAME D7 D6 D5 D4 D3 D2 D1 D0
--- ---- -- -- -- -- -- -- -- --
4 TALO TAL7 TAL6 TAL5 TAL4 TAL3 TAL2 TAL1 TAL0
5 TAHI TAH7 TAH6 TAH5 TAH4 TAH3 TAH2 TAH1 TAH0
6 TBLO TBL7 TBL6 TBLS TBL4 TBL3 TBL2 TBL1 TBL0
7 TBHI TBH7 TBH6 TBH5 TBH4 TBH3 TBH2 TBH1 TBH0

BIT NAMES on WRITE-Register

REG NAME D7 D6 D5 D4 D3 D2 D1 D0
--- ---- -- -- -- -- -- -- -- --
4 TALO PAL7 PAL6 PAL5 PAL4 PAL3 PAL2 PAL1 PAL0
5 TAHI PAH7 PAH6 PAH5 PAH4 PAH3 PAH2 PAH1 PAH0
6 TBLO PBL7 PBL6 PBL5 PBL4 PBL3 PBL2 PBL1 PBL0
7 TBHI PBH7 PBH6 PBH5 PBH4 PBH3 PBH2 PBH1 PBH0
INTERRUPT CONTROL REGISTER (ICR)
There are five sources of interrupts on the 8520:
 -Underflow from Timer A (timer counts down past 0)
 -Underflow from Timer B
 -TOD alarm
 -Serial port full/empty
 -Flag

A single register provides masking and interrupt information. The interrupt control register consists of a write-only MASK register and a
read-only DATA register. Any interrupt will set the corresponding bit in the DATA register. Any interrupt that is enabled by a 1-bit in that
position in the MASK will set the IR bit (MSB) of the DATA register and bring the IRQ pin low. In a multichip system, the IR bit can be polled to
detect which chip has generated an interrupt request.

When you read the DATA register, its contents are cleared (set to 0), and the IRQ line returns to a high state. Since it is cleared on a read, you
must assure that your interrupt polling or interrupt service code can preserve and respond to all bits which may have been set in the DATA
register at the time it was read. With proper preservation and response, it is easily possible to intermix polled and direct interrupt service
methods.

You can set or clear one or more bits of the MASK register without affecting the current state of any of the other bits in the register.
This is done by setting the appropriate state of the MSBit, which is called the set/clear bit. In bits 6-0, you yourself form a mask that specifies
which of the bits you wish to affect. Then, using bit 7, you specify HOW the bits in corresponding positions in the mask are to be affected.

o If bit 7 is a 1, then any bit 6-0 in your own mask byte which is set to a 1 sets the corresponding bit in the MASK register. Any bit that you
  have set to a 0 causes the MASK register bit to remain in its current state.

o If bit 7 is a 0, then any bit 6-0 in your own mask byte which is set to a 1 clears the corresponding bit in the MASK register. Again, any 0
  bit in your own mask byte causes no change in the contents of the corresponding MASK register bit.
  If an interrupt is to occur based on a particular condition, then that corresponding MASK bit must be a 1.

Example: Suppose you want to set the Timer A interrupt bit (enable the Timer A interrupt), but want to be sure that all other interrupts are
cleared. Here is the sequence you can use:

 INCLUDE "hardware/cia.i"
 XREF ciaa ; From amiga.lib
 lea ciaa,a0 ; Defined in amiga.lib
 move.b %01111110,ciaicr(a0)						MSB is 0, means clear any bit whose value is 1 in the rest of the byte
 INCLUDE "hardware/cia.i"
 XREF ciaa ; From amiga.lib
 lea ciaa,a0 ; Defined in amiga.lib
 move.b %100000001,ciaicr(a0)						MSB is 1, means set any bit whose value is 1 in the rest of the byte (do not change any values wherein the written value bit is a zero)

READ INTERRUPT CONTROL REGISTER

REG NAME D7 D6 D5 D4  D3  D2  D1 D0
--- ---- -- -- -- --  --  --  -- --
 D  ICR  IR 0  0  FLG SP ALRM TB TA

WRITE INTERRUPT CONTROL MASK

REG NAME D7  D6 D5 D4  D3 D2   D1 D0
--- ---- --  -- -- --  -- --   -- --
D   ICR  S/C x  x  FLG SP ALRM TB TA

CONTROL REGISTERS
There are two control registers in the 8520, CRA and CRB.
CRA is associated with Timer A and CRB is associated with Timer B.
The format of the registers is as follows:

CONTROL REGISTER A
BIT NAME    FUNCTION
 0  START   1 = start Timer A, 0 - top Timer A. This bit is automatically reset (= 0) when underflow occurs during one-shot mode.
 1  PBON    1 = Timer A output on PB6, 0 = PB6 is normal operation.
 2  OUTMODE 1 = toggle, 0 = pulse.
 3  RUNMODE 1 = one-shot mode, 0 = continuous mode.
 4  LOAD    1 = force load (this is a strobe input, there is no data storage; bit 4 will always read back a zero and writing a 0 has no effect.)
 5  INMODE  1 = Timer A count positive CNT transition, 0 = Timer A counts 02 pulses.
 6  SPMODE  1 = Serial port=output (CNT is the source of the shift clock)
            0 = Serial port-input (external shift clock is required)
 7  UNUSED
BIT MAP OF REGISTER CRA
REG# NAME UNUSED SPMODE   INMODE LOAD                   RUNMODE     OUTMODE  PBON     START
E    CRA  unused 0=input  0=02   1=force load (strobe)  0=cont.     0=pulse  0=PB60FF 0=stop
          unused 1=output 1=CNT                         1=one-shot  1=toggle 1-PB60N  1=start
                          |<-----------------------Timer A Variables------------------------>|
All unused register bits are unaffected by a write and forced to 0 on a read.

CONTROL REGISTER B:

BIT NAME    FUNCTION
 0  START   1=start Timer B, 0=stop Timer B. This bit is automatically reset (=0) when underflow occurs during one-shot mode.
 1  PBON    1=Timer B output on PB7, 0= PB7 is normal operation.
 2  OUTMODE 1=toggle, 0=pulse.
    RUNMODE 1=one-shot mode, 0=continuous mode.
 4  LOAD    1=force load (this is a strobe input, there is no data storage; bit 4 will always read back a zero and writing a 0 has no effect.)
6,5 INMODE  Bits CRB6 and CRB5 select one of four possible input modes for Timer B, as follows:
            CRB6 CRB5 Mode Selected
            ---- ---- --------------------------
             0    0   Timer B counts 02 pulses
             0    1   Timer B counts positive CNT transitions
             1    0   Timer B counts Timer A underflow pulses
             1    1   Timer B counts Timer A underflow pulses while CNT pin is held high.
 7  ALARM   1=writing to TOD registers sets Alarm
            0=writing to TOD registers sets TOD clock.
            Reading TOD registers always reads TOD clock, regardless of the state of the Alarm bit.
BIT MAP OF REGISTER CRB
REG# NAME ALARM    INMODE              LOAD          RUNMODE     OUTMODE   PBON      START
 F   CRB  0=TOD    00-02               1=force load  0=cont.     0=pulse   0=PB70FF  0=stop
          1=Alarm  01=CNT                            1=one-shot  1=toggle  1=PB70N   1=start
                   10=Timer A(strobe)
                   11=CNT+Timer A
                   <---------------------------Timer B Variables----------------------------->

All unused register bits are unaffected by a write and forced to 0 on a read.

*** Below is weird as fuck, they write one thing, example is another??

The system hardware selects the CIAs when the upper three address bits are 101. Furthermore, CIAA is selected when A12 is low, A13 high; CIAB is
selected when A12 is high, A13 low. CIAA communicates on data bits 7-0, CIAB communicates on data bits 15-8.

Address bits A11, A10, A9, and A8 are used to specify which of the 16 internal registers you want to access. This is indicated by "r" in the
address. All other bits are don't cares.
So, CIAA is selected by the following binary address: 101x xxxx xx01 rrrr xxxx xxx0.
                                        CIAB address: 101x xxxx xx10 rrrr xxxx xxx1.
**** However, reading the first paragraph, this should be the thing:
                                                CIAA: 101x xxxx xx10 rrrr xxxx xxxx
                                                CIAB: 101x xxxx xx01 rrrr xxxx xxxx
**** bit 0 is not mentioned at all, and the ID is swapped. But the addresses are known, so never mind ...

With future expansion in mind, we have decided on the following addresses:
CIAA = BFEr01: 1011 1111 1110 rrrr xxxx xxx1	**** note, ID is correct in this example.
CIAB = BFDr00: 1011 1111 1101 rrrr xxxx xxx0
Software must use byte accesses to these address, and no other.


*/
};

class Cia {
	public:
		Cia();
		~Cia();

		void	reset();
		void	vsyncHandler();
		void	hsyncHandler();
		void	handler();
		void	diskIndexHandler();
		void	dumpCIA();

};

