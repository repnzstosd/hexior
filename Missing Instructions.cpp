-- Most of these we implement when needed..
ORI to CCR
ORI to SR
ANDI to CCR
ANDI to SR
EORI to CCR
EORI to SR
MOVE from SR
MOVE to CCR
MOVE to SR
MOVE USP
ILLEGAL
RESET
STOP
RTE
TRAPV
RTR
TRAP

-- These are the ones to be implemented 41 instructions...
BCHG / BCHG IMM
~(<Bitnumber> of Destination) -> Z
Z -> Bit of Destination

BCLR / BCLR IMM
~(<Bitnumber> of Destination) -> Z
0 -> Bit of Destination

BSET / BSET IMM
~(<Bitnumber> of Destination) -> Z
1 -> Bit of Destination

BTST / BTST IMM
~(<Bitnumber> of Destination) -> Z

ANDI		-=- Immediate & Destination -> Destination ;  ANDI #xxx, <ea>
SUBI		-=- Destination - Immediate -> Destination ;  SUBI #xxx, <ea> 
ADDI		-=- Immediate + Destination -> Destination ;  ADDI #xxx, <ea>
EORI		-=- Immediate ^ Destination -> Destination ;  EORI #xxx, <ea>
CMPI		-=- Destination - Immediate                ;  CMPI #xxx, <ea>
MOVEP		-=- READ MANUAL.. Much fun to be had.
NEG			-=- 0 - destination
NEGX		-=- 0 - destination - X
NOT			-=- ~destination
EXT			-=- Signextended B->W ; W-L
NBCD		-=- 0 - Destination(base 10) - X -> Destination; Negate Decimal With Extend
CLR			-=- 0 (B,W,L)
TAS			-=- Set Condition Codes; set bit 7 of destination
TST			-=- Set Condition Codes
PEA			-=- SP-4 -> SP; <ea> -> (SP)
EOR			-=- Source ^ Destination -> Destination;  EOR Dn, <ea>
CHK			-=- if Dn < 0 or Dn > Source; Then TRAP
DIVU
DIVS
SBCD		-=- Destination(Base 10) - Source(Base 10) - X -> Destination;  Dn, Dn ; -(An), -(An)
CMPM		-=- Destination - Source -> CC;   CMPM (Ay)+, (Ax)+
MULU
MULS
ABCD		-=-	Source(Base 10) + Destination(Base 10) + X -> Destination;  Dn, Dn ; -(An), -(An)
EXG			-=- Rx <-> Ry
ASd / ASd <ea>
LSd / LSd <ea>
ROXd / ROXd <ea>
ROd / ROd <ea>

-- Below shouold be more or less done.. 29.
MOVE
MOVEA
SWAP
LINK
UNLK
NOP
RTS
JSR
JMP
MOVEM
LEA
ADDQ
SUBQ
Scc
DBcc
BRA
BSR
Bcc
MOVEQ
OR
SUB
SUBX
SUBA
CMP
CMPA
AND
ADD
ADDX
ADDA
