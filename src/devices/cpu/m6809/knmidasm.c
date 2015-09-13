// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#include "emu.h"
#include "debugger.h"
#include "konami.h"

#ifndef TRUE
#define TRUE    -1
#define FALSE   0
#endif

/*

0x08 leax indexed
0x09 leay indexed (not confirmed)
0x0a leau indexed
0x0b leas indexed (not confirmed)

0x0c pushs xx
0x0d pushu xx (not confirmed)
0x0e pulls xx
0x0f pulls xx (not confirmed)

0x10 lda xx
0x11 ldb xx
0x12 lda indexed
0x13 ldb indexed

0x14 adda xx
0x15 addb xx (not confirmed)
0x16 adda indexed (not confirmed)
0x17 addb indexed

0x18 adca xx
0x19 adcb xx (not confirmed)
0x1a adca indexed (not confirmed)
0x1b adcb indexed (not confirmed)

0x1c suba xx
0x1d subb xx
0x1e suba indexed
0x1f subb indexed

0x20 sbca xx
0x21 sbcb xx
0x22 sbca indexed
0x23 sbcb indexed

0x24 anda xx
0x25 andb xx
0x26 anda indexed
0x27 andb indexed

0x28 bita xx
0x29 bitb xx
0x2a bita indexed
0x2b bitb indexed

0x2c eora xx
0x2d eorb xx
0x2e eora indexed
0x2f eorb indexed

0x30 ora xx
0x31 orb xx
0x32 ora indexed
0x33 orb indexed

0x34 cmpa xx
0x35 cmpb xx
0x36 cmpa indexed
0x37 cmpb indexed

CUSTOM OPCODE: Set Address lines 16-23
--------------------------------------
0x38 setlines xx
0x39 setlines indexed

The eight bits taken as parameter set address lines 16 to 23.


0x3a sta indexed
0x3b stb indexed
0x3c andcc
0x3d orcc

0x3e exg xx
0x3f tfr xx

0x40 ldd xx xx
0x41 ldd indexed
0x42 ldx xx xx
0x43 ldx indexed
0x44 ldy xx xx
0x45 ldy indexed (not confirmed)
0x46 ldu xx xx
0x47 ldu indexed
0x48 lds xx xx
0x49 lds indexed (not confirmed)

0x4a cmpd xx xx
0x4b cmpd indexed
0x4c cmpx xx xx
0x4d cmpx indexed
0x4e cmpy xx xx (not confirmed)
0x4f cmpy indexed (not confirmed)
0x50 cmpu xx xx (not confirmed)
0x51 cmpu indexed (not confirmed)
0x52 cmps xx xx (not confirmed)
0x53 cmps indexed (not confirmed)

0x54 addd xx xx
0x55 addd indexed (not confirmed)
0x56 subd xx xx
0x57 subd indexed (not confirmed)

0x58 std indexed
0x59 stx indexed
0x5a sty indexed
0x5b stu indexed
0x5c sts indexed (not confirmed)


BRANCH OPCODE TABLE :
---------------------

Opcode  M6809   Konami
BRA     20      60
BRN     21      70
BHI     22      61
BLS     23      71
BCC     24      62
BCS     25      72
BNE     26      63
BEQ     27      73
BVC     28      64
BVS     29      74
BPL     2a      65
BMI     2b      75
BGE     2c      66
BLT     2d      76
BGT     2e      67
BLE     2f      77

Long versions of the branchs are the number + 8.

0x80 clra
0x81 clrb
0x82 clr indexed

0x83 coma
0x84 comb
0x85 com indexed

0x86 nega (not confirmed)
0x87 negb (not confirmed)
0x88 neg indexed (not confirmed)

0x89 inca
0x8a incb (not confirmed)
0x8b inc indexed

0x8c deca
0x8d decb
0x8e dec indexed

0x8f rts

0x90 tsta (not confirmed)
0x91 tstb (not confirmed)
0x92 tst indexed (not confirmed)

0x93 lsra
0x94 lsrb
0x95 lsr indexed

0x96 rora
0x97 rorb
0x98 ror indexed

0x99 asra
0x9a asrb
0x9b asr indexed

0x9c asla
0x9d aslb
0x9e asl indexed

0x9f rti

0xa0 rola
0xa1 rolb
0xa2 rol indexed

0xa3 lsrw indexed ( not confirmed )
0xa4 rorw indexed ( not confirmed )
0xa5 asrw indexed ( not confirmed )
0xa6 aslw indexed ( not confirmed )
0xa7 rolw indexed ( not confirmed )

0xa8 jmp indexed
0xa9 jsr indexed
0xaa bsr xx
0xab lbsr xx xx
0xac decb,jnz xx
0xad decx,jnz xx
0xae nop

0xb0 abx
0xb1 daa
0xb2 sex

0xb3 mul

0xb4 lmul    x:y = x * y

0xb5 divx    x = ( x / b ), b = ( x % b )

CUSTOM OPCODE: BlockMove (y,x,u):
---------------------------------
0xb6 bmove y,x,u

y = pointer to source address
x = pointer to destination address
u = bytes to move

One byte is copied at a time and x and y get incremented for each access.

CUSTOM OPCODE: Move (y,x,u):
---------------------------------
0xb7 move y,x,u

y = pointer to source address
x = pointer to destination address
u = counter

Copy ONE byte, increment x and y, decrement u.

0xb8 lsrd xx
0xb9 lsrd indexed
0xba rord xx ( not confirmed )
0xbb rord indexed ( not confirmed )
0xbc asrd xx ( not confirmed )
0xbd asrd indexed ( not confirmed )
0xbe asld xx
0xbf asld indexed ( not confirmed )
0xc0 rold xx ( not confirmed )
0xc1 rold indexed ( not confirmed )

0xc2 clrd
0xc3 clrw indexed ( clears an entire word ) ( not confirmed )

0xc4 negd (not confirmed )
0xc5 negw indexed

0xc6 incd (not confirmed )
0xc7 incw indexed

0xc8 decd (not confirmed )
0xc9 decw indexed

0xca tstd
0xcb tstw indexed

0xcc absa
0xcd absb
0xce absd

CUSTOM OPCODE: BlockSet (a,x,u):
---------------------------------
0xcf bset a,x,u

a = source data
x = pointer to destination address
u = bytes to move

One byte is copied at a time and x get incremented for each access.

CUSTOM OPCODE: BlockSet (d,x,u): (not confirmed)
--------------------------------
0xd0 bset d,x,u

d = source data
x = pointer to destination address
u = bytes to move/2

Two bytes are copied at a time and x get incremented twice for each access.

*/

static unsigned byte_count;
static unsigned local_pc;
static unsigned flags;

static const unsigned char *opram_ptr;

static unsigned char get_next_byte( void ) {
	return opram_ptr[byte_count++];
}

/* Table for indexed operations */
static const char index_reg[8][3] = {
	"?", /* 0 - extended mode */
	"?", /* 1 */
	"x", /* 2 */
	"y", /* 3 */
	"?", /* 4 - direct page */
	"u", /* 5 */
	"s", /* 6 */
	"pc" /* 7 - pc */
};

/* Table for tfr/exg operations */
static const char tfrexg_reg[8][3] = {
	"a", /* 0 */
	"b", /* 1 */
	"x", /* 2 */
	"y", /* 3 */
	"s", /* 4 */
	"u", /* 5 */
	"?", /* 6 */
	"?", /* 7 */
};

/* Table for stack S operations */
static const char stack_reg_s[8][3] = {
	"cc",
	"a",
	"b",
	"dp",
	"x",
	"y",
	"u",
	"pc"
};

/* Table for stack U operations */
static const char stack_reg_u[8][3] = {
	"cc",
	"a",
	"b",
	"dp",
	"x",
	"y",
	"s",
	"pc"
};

static void calc_indexed( unsigned char mode, char *buf ) {
	char buf2[30];
	int idx, type;

	idx = ( mode >> 4 ) & 7;
	type = mode & 0x0f;

	/* special modes */
	if ( mode & 0x80 ) {
		if ( type & 8 ) { /* indirect */
			switch ( type & 7 ) {
				case 0x00: /* register a */
					sprintf( buf2, "[a,%s]", index_reg[idx] );
				break;

				case 0x01: /* register b */
					sprintf( buf2, "[b,%s]", index_reg[idx] );
				break;

				case 0x04: /* direct - mode */
					sprintf( buf2, "[$%02x]", get_next_byte() );
				break;

				case 0x07: /* register d */
					sprintf( buf2, "[d,%s]", index_reg[idx] );
				break;

				default:
					sprintf( buf2, "[?,%s]", index_reg[idx] );
				break;
			}
		} else {
			switch ( type & 7 ) {
				case 0x00: /* register a */
					sprintf( buf2, "a,%s", index_reg[idx] );
				break;

				case 0x01: /* register b */
					sprintf( buf2, "b,%s", index_reg[idx] );
				break;

				case 0x04: /* direct - mode */
					sprintf( buf2, "$%02x", get_next_byte() );
				break;

				case 0x07: /* register d */
					sprintf( buf2, "d,%s", index_reg[idx] );
				break;

				default:
					sprintf( buf2, "????,%s", index_reg[idx] );
				break;
			}
		}
	} else {
		if ( type & 8 ) { /* indirect */
			switch ( type & 7 ) {
				case 0: /* auto increment */
					sprintf( buf2, "[,%s+]", index_reg[idx] );
				break;

				case 1: /* auto increment double */
					sprintf( buf2, "[,%s++]", index_reg[idx] );
				break;

				case 2: /* auto decrement */
					sprintf( buf2, "[,-%s]", index_reg[idx] );
				break;

				case 3: /* auto decrement double */
					sprintf( buf2, "[,--%s]", index_reg[idx] );
				break;

				case 4: /* post byte offset */
					{
						int val = get_next_byte();

						if ( val & 0x80 )
							sprintf( buf2, "[#$-%02x,%s]", 0x100 - val, index_reg[idx] );
						else
							sprintf( buf2, "[#$%02x,%s]", val, index_reg[idx] );
					}
				break;

				case 5: /* post word offset */
					{
						int val = get_next_byte() << 8;

						val |= get_next_byte();

						if ( val & 0x8000 )
							sprintf( buf2, "[#$-%04x,%s]", 0x10000 - val, index_reg[idx] );
						else
							sprintf( buf2, "[#$%04x,%s]", val, index_reg[idx] );
					}
				break;

				case 6: /* simple */
					sprintf( buf2, "[,%s]", index_reg[idx] );
				break;

				case 7: /* extended */
					{
						int val = get_next_byte() << 8;

						val |= get_next_byte();

						sprintf( buf2, "[$%04x]", val );
					}
				break;
			}
		} else {
			switch ( type & 7 ) {
				case 0: /* auto increment */
					sprintf( buf2, ",%s+", index_reg[idx] );
				break;

				case 1: /* auto increment double */
					sprintf( buf2, ",%s++", index_reg[idx] );
				break;

				case 2: /* auto decrement */
					sprintf( buf2, ",-%s", index_reg[idx] );
				break;

				case 3: /* auto decrement double */
					sprintf( buf2, ",--%s", index_reg[idx] );
				break;

				case 4: /* post byte offset */
					{
						int val = get_next_byte();

						if ( val & 0x80 )
							sprintf( buf2, "#$-%02x,%s", 0x100 - val , index_reg[idx] );
						else
							sprintf( buf2, "#$%02x,%s", val, index_reg[idx] );
					}
				break;

				case 5: /* post word offset */
					{
						int val = get_next_byte() << 8;

						val |= get_next_byte();

						if ( val & 0x8000 )
							sprintf( buf2, "#$-%04x,%s", 0x10000 - val, index_reg[idx] );
						else
							sprintf( buf2, "#$%04x,%s", val, index_reg[idx] );
					}
				break;

				case 6: /* simple */
					sprintf( buf2, ",%s", index_reg[idx] );
				break;

				case 7: /* extended */
					{
						int val = get_next_byte() << 8;

						val |= get_next_byte();

						sprintf( buf2, "$%04x", val );
					}
				break;

			}
		}
	}

	strcat( buf, buf2 );
}

static void do_relative( char *buf ) {
	char buf2[30];
	signed char offs = ( signed char )get_next_byte();

	sprintf( buf2, "$%04x (%d)", local_pc + byte_count + offs, (int)offs );

	strcat( buf, buf2 );
}

static void do_relative_word( char *buf ) {
	char buf2[30];
	signed short offs;
	int val = get_next_byte() << 8;

	val |= get_next_byte();

	offs = ( signed short )val;

	sprintf( buf2, "$%04x (%d)", local_pc + byte_count + offs, (int)offs );

	strcat( buf, buf2 );
}

static void do_addressing( char *buf ) {
	unsigned char mode = get_next_byte();

	calc_indexed( mode, buf );
}

/*********************************************************************************

    Opcodes

*********************************************************************************/

static void illegal( char *buf ) {
	sprintf( buf, "illegal/unknown " );

}

static void leax( char *buf ) {
	sprintf( buf, "leax  " );
	do_addressing( buf );
}

static void leay( char *buf ) {
	sprintf( buf, "leay  " );
	do_addressing( buf );
}

static void leau( char *buf ) {
	sprintf( buf, "leau  " );
	do_addressing( buf );
}

static void leas( char *buf ) {
	sprintf( buf, "leas  " );
	do_addressing( buf );
}

static void lda( char *buf ) {
	sprintf( buf, "lda   #$%02x", get_next_byte() );
}

static void ldb( char *buf ) {
	sprintf( buf, "ldb   #$%02x", get_next_byte() );
}

static void lda2( char *buf ) {
	sprintf( buf, "lda   " );
	do_addressing( buf );
}

static void ldb2( char *buf ) {
	sprintf( buf, "ldb   " );
	do_addressing( buf );
}

static void adda( char *buf ) {
	sprintf( buf, "adda  #$%02x", get_next_byte() );
}

static void addb( char *buf ) {
	sprintf( buf, "addb  #$%02x", get_next_byte() );
}

static void adda2( char *buf ) {
	sprintf( buf, "adda  " );
	do_addressing( buf );
}

static void addb2( char *buf ) {
	sprintf( buf, "addb  " );
	do_addressing( buf );
}

static void suba( char *buf ) {
	sprintf( buf, "suba  #$%02x", get_next_byte() );
}

static void subb( char *buf ) {
	sprintf( buf, "subb  #$%02x", get_next_byte() );
}

static void suba2( char *buf ) {
	sprintf( buf, "suba  " );
	do_addressing( buf );
}

static void subb2( char *buf ) {
	sprintf( buf, "subb  " );
	do_addressing( buf );
}

static void sbca( char *buf ) {
	sprintf( buf, "sbca  #$%02x", get_next_byte() );
}

static void sbcb( char *buf ) {
	sprintf( buf, "sbcb  #$%02x", get_next_byte() );
}

static void sbca2( char *buf ) {
	sprintf( buf, "sbca  " );
	do_addressing( buf );
}

static void sbcb2( char *buf ) {
	sprintf( buf, "sbcb  " );
	do_addressing( buf );
}

static void adca( char *buf ) {
	sprintf( buf, "adca  #$%02x", get_next_byte() );
}

static void adca2( char *buf ) {
	sprintf( buf, "adca  " );
	do_addressing( buf );
}

static void adcb2( char *buf ) {
	sprintf( buf, "adcb  " );
	do_addressing( buf );
}

static void adcb( char *buf ) {
	sprintf( buf, "adcb  #$%02x", get_next_byte() );
}

static void anda( char *buf ) {
	sprintf( buf, "anda  #$%02x", get_next_byte() );
}

static void andb( char *buf ) {
	sprintf( buf, "andb  #$%02x", get_next_byte() );
}

static void anda2( char *buf ) {
	sprintf( buf, "anda  " );
	do_addressing( buf );
}

static void andb2( char *buf ) {
	sprintf( buf, "andb  " );
	do_addressing( buf );
}

static void bita( char *buf ) {
	sprintf( buf, "bita  #$%02x", get_next_byte() );
}

static void bitb( char *buf ) {
	sprintf( buf, "bitb  #$%02x", get_next_byte() );
}

static void bita2( char *buf ) {
	sprintf( buf, "bita  " );
	do_addressing( buf );
}

static void bitb2( char *buf ) {
	sprintf( buf, "bitb  " );
	do_addressing( buf );
}

static void eora( char *buf ) {
	sprintf( buf, "eora  #$%02x", get_next_byte() );
}

static void eorb( char *buf ) {
	sprintf( buf, "eorb  #$%02x", get_next_byte() );
}

static void eora2( char *buf ) {
	sprintf( buf, "eora  " );
	do_addressing( buf );
}

static void eorb2( char *buf ) {
	sprintf( buf, "eorb  " );
	do_addressing( buf );
}

static void ora( char *buf ) {
	sprintf( buf, "ora   #$%02x", get_next_byte() );
}

static void orb( char *buf ) {
	sprintf( buf, "orb   #$%02x", get_next_byte() );
}

static void ora2( char *buf ) {
	sprintf( buf, "ora   " );
	do_addressing( buf );
}

static void orb2( char *buf ) {
	sprintf( buf, "orb   " );
	do_addressing( buf );
}

static void cmpa( char *buf ) {
	sprintf( buf, "cmpa  #$%02x", get_next_byte() );
}

static void cmpb( char *buf ) {
	sprintf( buf, "cmpb  #$%02x", get_next_byte() );
}

static void cmpa2( char *buf ) {
	sprintf( buf, "cmpa  " );
	do_addressing( buf );
}

static void cmpb2( char *buf ) {
	sprintf( buf, "cmpb  " );
	do_addressing( buf );
}

static void setlines( char *buf ) {
	sprintf( buf, "setlines #$%02x", get_next_byte() );
}

static void setlines2( char *buf ) {
	sprintf( buf, "setlines " );
	do_addressing( buf );
}

static void sta2( char *buf ) {
	sprintf( buf, "sta   " );
	do_addressing( buf );
}

static void stb2( char *buf ) {
	sprintf( buf, "stb   " );
	do_addressing( buf );
}

static void ldd( char *buf ) {
	int val = get_next_byte() << 8;

	val |= get_next_byte();

	sprintf( buf, "ldd   #$%04x", val );
}

static void ldd2( char *buf ) {
	sprintf( buf, "ldd   " );
	do_addressing( buf );
}

static void ldx( char *buf ) {
	int val = get_next_byte() << 8;

	val |= get_next_byte();

	sprintf( buf, "ldx   #$%04x", val );
}

static void ldx2( char *buf ) {
	sprintf( buf, "ldx   " );
	do_addressing( buf );
}

static void ldy( char *buf ) {
	int val = get_next_byte() << 8;

	val |= get_next_byte();

	sprintf( buf, "ldy   #$%04x", val );
}

static void ldy2( char *buf ) {
	sprintf( buf, "ldy   " );
	do_addressing( buf );
}

static void ldu( char *buf ) {
	int val = get_next_byte() << 8;

	val |= get_next_byte();

	sprintf( buf, "ldu   #$%04x", val );
}

static void ldu2( char *buf ) {
	sprintf( buf, "ldu   " );
	do_addressing( buf );
}

static void lds( char *buf ) {
	int val = get_next_byte() << 8;

	val |= get_next_byte();

	sprintf( buf, "lds   #$%04x", val );
}

static void lds2( char *buf ) {
	sprintf( buf, "lds   " );
	do_addressing( buf );
}

static void cmpd( char *buf ) {
	int val = get_next_byte() << 8;

	val |= get_next_byte();

	sprintf( buf, "cmpd  #$%04x", val );
}

static void cmpd2( char *buf ) {
	sprintf( buf, "cmpd  " );
	do_addressing( buf );
}

static void cmpx( char *buf ) {
	int val = get_next_byte() << 8;

	val |= get_next_byte();

	sprintf( buf, "cmpx  #$%04x", val );
}

static void cmpx2( char *buf ) {
	sprintf( buf, "cmpx  " );
	do_addressing( buf );
}

static void cmpy( char *buf ) {
	int val = get_next_byte() << 8;

	val |= get_next_byte();

	sprintf( buf, "cmpy  #$%04x", val );
}

static void cmpy2( char *buf ) {
	sprintf( buf, "cmpy  " );
	do_addressing( buf );
}

static void cmpu( char *buf ) {
	int val = get_next_byte() << 8;

	val |= get_next_byte();

	sprintf( buf, "cmpu  #$%04x", val );
}

static void cmpu2( char *buf ) {
	sprintf( buf, "cmpu  " );
	do_addressing( buf );
}

static void cmps( char *buf ) {
	int val = get_next_byte() << 8;

	val |= get_next_byte();

	sprintf( buf, "cmps  #$%04x", val );
}

static void cmps2( char *buf ) {
	sprintf( buf, "cmps  " );
	do_addressing( buf );
}

static void addd2( char *buf ) {
	sprintf( buf, "addd  " );
	do_addressing( buf );
}

static void subd2( char *buf ) {
	sprintf( buf, "subd  " );
	do_addressing( buf );
}

static void std2( char *buf ) {
	sprintf( buf, "std   " );
	do_addressing( buf );
}

static void stx2( char *buf ) {
	sprintf( buf, "stx   " );
	do_addressing( buf );
}

static void sty2( char *buf ) {
	sprintf( buf, "sty   " );
	do_addressing( buf );
}

static void stu2( char *buf ) {
	sprintf( buf, "stu   " );
	do_addressing( buf );
}

static void sts2( char *buf ) {
	sprintf( buf, "sts   " );
	do_addressing( buf );
}

static void bra( char *buf ) {
	sprintf( buf, "bra   " );
	do_relative( buf );
}

static void lbra( char *buf ) {
	sprintf( buf, "lbra  " );
	do_relative_word( buf );
}

static void brn( char *buf ) {
	sprintf( buf, "brn   " );
	do_relative( buf );
}

static void lbrn( char *buf ) {
	sprintf( buf, "lbrn  " );
	do_relative_word( buf );
}

static void bhi( char *buf ) {
	sprintf( buf, "bhi   " );
	do_relative( buf );
}

static void lbhi( char *buf ) {
	sprintf( buf, "lbhi  " );
	do_relative_word( buf );
}

static void bls( char *buf ) {
	sprintf( buf, "bls   " );
	do_relative( buf );
}

static void lbls( char *buf ) {
	sprintf( buf, "lbls  " );
	do_relative_word( buf );
}

static void bcc( char *buf ) {
	sprintf( buf, "bcc   " );
	do_relative( buf );
}

static void lbcc( char *buf ) {
	sprintf( buf, "lbcc  " );
	do_relative_word( buf );
}

static void bcs( char *buf ) {
	sprintf( buf, "bcs   " );
	do_relative( buf );
}

static void lbcs( char *buf ) {
	sprintf( buf, "lbcs  " );
	do_relative_word( buf );
}

static void bne( char *buf ) {
	sprintf( buf, "bne   " );
	do_relative( buf );
}

static void lbne( char *buf ) {
	sprintf( buf, "lbne  " );
	do_relative_word( buf );
}

static void beq( char *buf ) {
	sprintf( buf, "beq   " );
	do_relative( buf );
}

static void lbeq( char *buf ) {
	sprintf( buf, "lbeq  " );
	do_relative_word( buf );
}

static void bvc( char *buf ) {
	sprintf( buf, "bvc   " );
	do_relative( buf );
}

static void lbvc( char *buf ) {
	sprintf( buf, "lbvc  " );
	do_relative_word( buf );
}

static void bvs( char *buf ) {
	sprintf( buf, "bvs   " );
	do_relative( buf );
}

static void lbvs( char *buf ) {
	sprintf( buf, "lbvs  " );
	do_relative_word( buf );
}

static void bpl( char *buf ) {
	sprintf( buf, "bpl   " );
	do_relative( buf );
}

static void lbpl( char *buf ) {
	sprintf( buf, "lbpl  " );
	do_relative_word( buf );
}

static void bmi( char *buf ) {
	sprintf( buf, "bmi   " );
	do_relative( buf );
}

static void lbmi( char *buf ) {
	sprintf( buf, "lbmi  " );
	do_relative_word( buf );
}

static void bge( char *buf ) {
	sprintf( buf, "bge   " );
	do_relative( buf );
}

static void lbge( char *buf ) {
	sprintf( buf, "lbge  " );
	do_relative_word( buf );
}

static void blt( char *buf ) {
	sprintf( buf, "blt   " );
	do_relative( buf );
}

static void lblt( char *buf ) {
	sprintf( buf, "lblt  " );
	do_relative_word( buf );
}

static void bgt( char *buf ) {
	sprintf( buf, "bgt   " );
	do_relative( buf );
}

static void lbgt( char *buf ) {
	sprintf( buf, "lbgt  " );
	do_relative_word( buf );
}

static void ble( char *buf ) {
	sprintf( buf, "ble   " );
	do_relative( buf );
}

static void lble( char *buf ) {
	sprintf( buf, "lble  " );
	do_relative_word( buf );
}

static void clra( char *buf ) {
	sprintf( buf, "clra" );
}

static void clrb( char *buf ) {
	sprintf( buf, "clrb" );
}

static void clrd( char *buf ) {
	sprintf( buf, "clrd" );
}

static void clrw( char *buf ) {
	sprintf( buf, "clrw  " );
	do_addressing( buf );
}

static void negd( char *buf ) {
	sprintf( buf, "negd" );
}

static void negw( char *buf ) {
	sprintf( buf, "negw" );
	do_addressing( buf );
}

static void incd( char *buf ) {
	sprintf( buf, "incd" );
}

static void incw( char *buf ) {
	sprintf( buf, "incw  " );
	do_addressing( buf );
}

static void decd( char *buf ) {
	sprintf( buf, "decd" );
}

static void decw( char *buf ) {
	sprintf( buf, "decw  " );
	do_addressing( buf );
}

static void tstd( char *buf ) {
	sprintf( buf, "tstd  " );
}

static void tstw( char *buf ) {
	sprintf( buf, "tstw  " );
	do_addressing( buf );
}

static void clr2( char *buf ) {
	sprintf( buf, "clr   " );
	do_addressing( buf );
}

static void coma( char *buf ) {
	sprintf( buf, "coma" );
}

static void comb( char *buf ) {
	sprintf( buf, "comb" );
}

static void com2( char *buf ) {
	sprintf( buf, "com   " );
	do_addressing( buf );
}

static void nega( char *buf ) {
	sprintf( buf, "nega" );
}

static void negb( char *buf ) {
	sprintf( buf, "negb" );
}

static void neg2( char *buf ) {
	sprintf( buf, "neg   " );
	do_addressing( buf );
}

static void inca( char *buf ) {
	sprintf( buf, "inca" );
}

static void incb( char *buf ) {
	sprintf( buf, "incb" );
}

static void inc2( char *buf ) {
	sprintf( buf, "inc   " );
	do_addressing( buf );
}

static void deca( char *buf ) {
	sprintf( buf, "deca" );
}

static void decb( char *buf ) {
	sprintf( buf, "decb" );
}

static void dec2( char *buf ) {
	sprintf( buf, "dec   " );
	do_addressing( buf );
}

static void rts( char *buf ) {
	sprintf( buf, "rts"  );
	flags = DASMFLAG_STEP_OUT;
}

static void asla( char *buf ) {
	sprintf( buf, "asla" );
}

static void aslb( char *buf ) {
	sprintf( buf, "aslb" );
}

static void asl2( char *buf ) {
	sprintf( buf, "asl   " );
	do_addressing( buf );
}

static void rora( char *buf ) {
	sprintf( buf, "rora" );
}

static void rorb( char *buf ) {
	sprintf( buf, "rorb" );
}

static void ror2( char *buf ) {
	sprintf( buf, "ror   " );
	do_addressing( buf );
}

static void rti( char *buf ) {
	sprintf( buf, "rti"  );
	flags = DASMFLAG_STEP_OUT;
}

static void jsr2( char *buf ) {
	sprintf( buf, "jsr   " );
	do_addressing( buf );
	flags = DASMFLAG_STEP_OVER;
}

static void jmp2( char *buf ) {
	sprintf( buf, "jmp   " );
	do_addressing( buf );
}

static void bsr( char *buf ) {
	sprintf( buf, "bsr   " );
	do_relative( buf );
	flags = DASMFLAG_STEP_OVER;
}

static void lbsr( char *buf ) {
	sprintf( buf, "lbsr  " );
	do_relative_word( buf );
	flags = DASMFLAG_STEP_OVER;
}

static void decbjnz( char *buf ) {
	sprintf( buf, "decb,jnz " );
	do_relative( buf );
	flags = DASMFLAG_STEP_OVER;
}

static void decxjnz( char *buf ) {
	sprintf( buf, "decx,jnz " );
	do_relative( buf );
	flags = DASMFLAG_STEP_OVER;
}

static void addd( char *buf ) {
	int val = get_next_byte() << 8;

	val |= get_next_byte();

	sprintf( buf, "addd  #$%04x", val );
}

static void subd( char *buf ) {
	int val = get_next_byte() << 8;

	val |= get_next_byte();

	sprintf( buf, "subd  #$%04x", val );
}

static void tsta( char *buf ) {
	sprintf( buf, "tsta" );
}

static void tstb( char *buf ) {
	sprintf( buf, "tstb" );
}

static void tst2( char *buf ) {
	sprintf( buf, "tst   " );
	do_addressing( buf );
}

static void lsra( char *buf ) {
	sprintf( buf, "lsra" );
}

static void lsrb( char *buf ) {
	sprintf( buf, "lsrb" );
}

static void lsr2( char *buf ) {
	sprintf( buf, "lsr   " );
	do_addressing( buf );
}

static void asra( char *buf ) {
	sprintf( buf, "asra" );
}

static void asrb( char *buf ) {
	sprintf( buf, "asrb" );
}

static void asr2( char *buf ) {
	sprintf( buf, "asr   " );
	do_addressing( buf );
}

static void abx( char *buf ) {
	sprintf( buf, "abx" );
}

static void sex( char *buf ) {
	sprintf( buf, "sex" );
}

static void daa( char *buf ) {
	sprintf( buf, "daa" );
}

static void mul( char *buf ) {
	sprintf( buf, "mul" );
}

static void lmul( char *buf ) {
	sprintf( buf, "lmul" );
}

static void divx( char *buf ) {
	sprintf( buf, "div   x,b" );
}

static void andcc( char *buf ) {
	sprintf( buf, "andcc #$%02x", get_next_byte() );
}

static void orcc( char *buf ) {
	sprintf( buf, "orcc  #$%02x", get_next_byte() );
}

static void pushs( char *buf ) {
	int mask = get_next_byte(), i;

	sprintf( buf, "pushs " );

	for ( i = 0; i < 8; i++ ) {
		if ( ( mask >> i ) & 1 ) {
			strcat( buf, stack_reg_s[i] );
			mask &= ~( 1 << i );
			if ( mask )
				strcat( buf, "," );
			else
				return;
		}
	}
}

static void pushu( char *buf ) {
	int mask = get_next_byte(), i;

	sprintf( buf, "pushu " );

	for ( i = 0; i < 8; i++ ) {
		if ( ( mask >> i ) & 1 ) {
			strcat( buf, stack_reg_u[i] );
			mask &= ~( 1 << i );
			if ( mask )
				strcat( buf, "," );
			else
				return;
		}
	}
}

static void pulls( char *buf ) {
	int mask = get_next_byte(), i;

	sprintf( buf, "pulls " );

	for ( i = 0; i < 8; i++ ) {
		if ( ( mask >> i ) & 1 ) {
			strcat( buf, stack_reg_s[i] );
			if (i == 7)
				flags = DASMFLAG_STEP_OUT;
			mask &= ~( 1 << i );
			if ( mask )
				strcat( buf, "," );
			else
				return;
		}
	}
}

static void pullu( char *buf ) {
	int mask = get_next_byte(), i;

	sprintf( buf, "pullu " );

	for ( i = 0; i < 8; i++ ) {
		if ( ( mask >> i ) & 1 ) {
			strcat( buf, stack_reg_s[i] );
			mask &= ~( 1 << i );
			if ( mask )
				strcat( buf, "," );
			else
				return;
		}
	}
}

static void rola( char *buf ) {
	sprintf( buf, "rola" );
}

static void rolb( char *buf ) {
	sprintf( buf, "rolb" );
}

static void rol2( char *buf ) {
	sprintf( buf, "rol   " );
	do_addressing( buf );
}

static void bmove( char *buf ) {
	sprintf( buf, "bmove y,x,u" );
}

static void move( char *buf ) {
	sprintf( buf, "move  y,x,u" );
}

static void bset( char *buf ) {
	sprintf( buf, "bset  a,x,u" );
}

static void bset2( char *buf ) {
	sprintf( buf, "bset  d,x,u" );
}

static void nop( char *buf ) {
	sprintf( buf, "nop" );
}

static void tfr( char *buf ) {
	int mask = get_next_byte();

	sprintf( buf, "tfr   " );

	strcat( buf, tfrexg_reg[ mask & 0x07 ] );
	strcat( buf, "," );
	strcat( buf, tfrexg_reg[ ( mask >> 4 ) & 0x07 ] );
}

static void exg( char *buf ) {
	int mask = get_next_byte();

	sprintf( buf, "exg   " );

	strcat( buf, tfrexg_reg[ mask & 0x07 ] );
	strcat( buf, "," );
	strcat( buf, tfrexg_reg[ ( mask >> 4 ) & 0x07 ] );
}

static void lsrd( char *buf ) {
	sprintf( buf, "lsrd  #$%02x", get_next_byte() );
}

static void lsrd2( char *buf ) {
	sprintf( buf, "lsrd  " );
	do_addressing( buf );
}

static void rord( char *buf ) {
	sprintf( buf, "rord  #$%02x", get_next_byte() );
}

static void rord2( char *buf ) {
	sprintf( buf, "rord  " );
	do_addressing( buf );
}

static void asrd( char *buf ) {
	sprintf( buf, "asrd  #$%02x", get_next_byte() );
}

static void asrd2( char *buf ) {
	sprintf( buf, "asrd  " );
	do_addressing( buf );
}

static void asld( char *buf ) {
	sprintf( buf, "asld  #$%02x", get_next_byte() );
}

static void asld2( char *buf ) {
	sprintf( buf, "asld  " );
	do_addressing( buf );
}

static void rold( char *buf ) {
	sprintf( buf, "rold  #$%02x", get_next_byte() );
}

static void rold2( char *buf ) {
	sprintf( buf, "rold  " );
	do_addressing( buf );
}

static void lsrw( char *buf ) {
	sprintf( buf, "lsrw  " );
	do_addressing( buf );
}

static void rorw( char *buf ) {
	sprintf( buf, "lsrw  " );
	do_addressing( buf );
}

static void asrw( char *buf ) {
	sprintf( buf, "asrw  " );
	do_addressing( buf );
}

static void aslw( char *buf ) {
	sprintf( buf, "aslw  " );
	do_addressing( buf );
}

static void rolw( char *buf ) {
	sprintf( buf, "rolw  " );
	do_addressing( buf );
}

static void absa( char *buf ) {
	sprintf( buf, "absa" );
}

static void absb( char *buf ) {
	sprintf( buf, "absb" );
}

static void absd( char *buf ) {
	sprintf( buf, "absd" );
}

/*********************************************************************************

    Opcode Table

*********************************************************************************/

struct konami_opcode_def {
	void (*decode)( char *buf );
	int confirmed;
};

static const konami_opcode_def op_table[256] = {
	/* 00 */    { illegal, 0 },
	/* 01 */    { illegal, 0 },
	/* 02 */    { illegal, 0 },
	/* 03 */    { illegal, 0 },
	/* 04 */    { illegal, 0 },
	/* 05 */    { illegal, 0 },
	/* 06 */    { illegal, 0 },
	/* 07 */    { illegal, 0 },
	/* 08 */    { leax, 1 },
	/* 09 */    { leay, 1 },
	/* 0a */    { leau, 1 },
	/* 0b */    { leas, 0 },
	/* 0c */    { pushs, 1 },
	/* 0d */    { pushu, 0 },
	/* 0e */    { pulls, 1 },
	/* 0f */    { pullu, 0 },

	/* 10 */    { lda, 1 },
	/* 11 */    { ldb, 1 },
	/* 12 */    { lda2, 1 },
	/* 13 */    { ldb2, 1 },
	/* 14 */    { adda, 1 },
	/* 15 */    { addb, 1 },
	/* 16 */    { adda2, 1 },
	/* 17 */    { addb2, 1 },
	/* 18 */    { adca, 1 },
	/* 19 */    { adcb, 1 },
	/* 1a */    { adca2, 1 },
	/* 1b */    { adcb2, 1 },
	/* 1c */    { suba, 1 },
	/* 1d */    { subb, 1 },
	/* 1e */    { suba2, 1 },
	/* 1f */    { subb2, 1 },

	/* 20 */    { sbca, 0 },
	/* 21 */    { sbcb, 0 },
	/* 22 */    { sbca2, 0 },
	/* 23 */    { sbcb2, 0 },
	/* 24 */    { anda, 1 },
	/* 25 */    { andb, 1 },
	/* 26 */    { anda2, 1 },
	/* 27 */    { andb2, 1 },
	/* 28 */    { bita, 0 },
	/* 29 */    { bitb, 0 },
	/* 2a */    { bita2, 0 },
	/* 2b */    { bitb2, 0 },
	/* 2c */    { eora, 0 },
	/* 2d */    { eorb, 0 },
	/* 2e */    { eora2, 0 },
	/* 2f */    { eorb2, 0 },

	/* 30 */    { ora, 1 },
	/* 31 */    { orb, 1 },
	/* 32 */    { ora2, 1 },
	/* 33 */    { orb2, 1 },
	/* 34 */    { cmpa, 1 },
	/* 35 */    { cmpb, 1 },
	/* 36 */    { cmpa2, 1 },
	/* 37 */    { cmpb2, 1 },
	/* 38 */    { setlines, 0 },
	/* 39 */    { setlines2, 0 },
	/* 3a */    { sta2, 1 },
	/* 3b */    { stb2, 1 },
	/* 3c */    { andcc, 1 },
	/* 3d */    { orcc, 0 },
	/* 3e */    { exg, 0 },
	/* 3f */    { tfr, 0 },

	/* 40 */    { ldd, 1 },
	/* 41 */    { ldd2, 1 },
	/* 42 */    { ldx, 1 },
	/* 43 */    { ldx2, 1 },
	/* 44 */    { ldy, 1 },
	/* 45 */    { ldy2, 1 },
	/* 46 */    { ldu, 1 },
	/* 47 */    { ldu2, 1 },
	/* 48 */    { lds, 1 },
	/* 49 */    { lds2, 1 },
	/* 4a */    { cmpd, 1 },
	/* 4b */    { cmpd2, 1 },
	/* 4c */    { cmpx, 1 },
	/* 4d */    { cmpx2, 1 },
	/* 4e */    { cmpy, 1 },
	/* 4f */    { cmpy2, 1 },

	/* 50 */    { cmpu, 1 },
	/* 51 */    { cmpu2, 1 },
	/* 52 */    { cmps, 1 },
	/* 53 */    { cmps2, 1 },
	/* 54 */    { addd, 0 },
	/* 55 */    { addd2, 0 },
	/* 56 */    { subd, 1 },
	/* 57 */    { subd2, 0 },
	/* 58 */    { std2, 1 },
	/* 59 */    { stx2, 1 },
	/* 5a */    { sty2, 1 },
	/* 5b */    { stu2, 1 },
	/* 5c */    { sts2, 1 },
	/* 5d */    { illegal, 0 },
	/* 5e */    { illegal, 0 },
	/* 5f */    { illegal, 0 },

	/* 60 */    { bra, 1 },
	/* 61 */    { bhi, 1 },
	/* 62 */    { bcc, 1 },
	/* 63 */    { bne, 1 },
	/* 64 */    { bvc, 1 },
	/* 65 */    { bpl, 1 },
	/* 66 */    { bge, 1 },
	/* 67 */    { bgt, 1 },
	/* 68 */    { lbra, 1 },
	/* 69 */    { lbhi, 1 },
	/* 6a */    { lbcc, 1 },
	/* 6b */    { lbne, 1 },
	/* 6c */    { lbvc, 1 },
	/* 6d */    { lbpl, 1 },
	/* 6e */    { lbge, 1 },
	/* 6f */    { lbgt, 1 },

	/* 70 */    { brn, 1 },
	/* 71 */    { bls, 1 },
	/* 72 */    { bcs, 1 },
	/* 73 */    { beq, 1 },
	/* 74 */    { bvs, 1 },
	/* 75 */    { bmi, 1 },
	/* 76 */    { blt, 1 },
	/* 77 */    { ble, 1 },
	/* 78 */    { lbrn, 1 },
	/* 79 */    { lbls, 1 },
	/* 7a */    { lbcs, 1 },
	/* 7b */    { lbeq, 1 },
	/* 7c */    { lbvs, 1 },
	/* 7d */    { lbmi, 1 },
	/* 7e */    { lblt, 1 },
	/* 7f */    { lble, 1 },

	/* 80 */    { clra, 1 },
	/* 81 */    { clrb, 1 },
	/* 82 */    { clr2, 1 },
	/* 83 */    { coma, 1 },
	/* 84 */    { comb, 0 },
	/* 85 */    { com2, 0 },
	/* 86 */    { nega, 0 },
	/* 87 */    { negb, 0 },
	/* 88 */    { neg2, 0 },
	/* 89 */    { inca, 1 },
	/* 8a */    { incb, 1 },
	/* 8b */    { inc2, 1 },
	/* 8c */    { deca, 1 },
	/* 8d */    { decb, 1 },
	/* 8e */    { dec2, 1 },
	/* 8f */    { rts, 1 },

	/* 90 */    { tsta, 0 },
	/* 91 */    { tstb, 0 },
	/* 92 */    { tst2, 0 },
	/* 93 */    { lsra, 1 },
	/* 94 */    { lsrb, 1 },
	/* 95 */    { lsr2, 0 },
	/* 96 */    { rora, 0 },
	/* 97 */    { rorb, 0 },
	/* 98 */    { ror2, 0 },
	/* 99 */    { asra, 0 },
	/* 9a */    { asrb, 0 },
	/* 9b */    { asr2, 0 },
	/* 9c */    { asla, 1 },
	/* 9d */    { aslb, 1 },
	/* 9e */    { asl2, 0 },
	/* 9f */    { rti, 1 },

	/* a0 */    { rola, 1 },
	/* a1 */    { rolb, 0 },
	/* a2 */    { rol2, 0 },
	/* a3 */    { lsrw, 0 },
	/* a4 */    { rorw, 0 },
	/* a5 */    { asrw, 0 },
	/* a6 */    { aslw, 0 },
	/* a7 */    { rolw, 0 },
	/* a8 */    { jmp2, 1 },
	/* a9 */    { jsr2, 1 },
	/* aa */    { bsr, 1 },
	/* ab */    { lbsr, 1 },
	/* ac */    { decbjnz, 0 },
	/* ad */    { decxjnz, 0 },
	/* ae */    { nop, 0 },
	/* af */    { illegal, 0 },

	/* b0 */    { abx, 0 },
	/* b1 */    { daa, 0 },
	/* b2 */    { sex, 0 },
	/* b3 */    { mul, 1 },
	/* b4 */    { lmul, 0 },
	/* b5 */    { divx, 0 },
	/* b6 */    { bmove, 1 },
	/* b7 */    { move, 0 },
	/* b8 */    { lsrd, 0 },
	/* b9 */    { lsrd2, 0 },
	/* ba */    { rord, 0 },
	/* bb */    { rord2, 0 },
	/* bc */    { asrd, 0 },
	/* bd */    { asrd2, 0 },
	/* be */    { asld, 0 },
	/* bf */    { asld2, 0 },

	/* c0 */    { rold, 0 },
	/* c1 */    { rold2, 0 },
	/* c2 */    { clrd, 1 },
	/* c3 */    { clrw, 0 },
	/* c4 */    { negd, 0 },
	/* c5 */    { negw, 0 },
	/* c6 */    { incd, 0 },
	/* c7 */    { incw, 0 },
	/* c8 */    { decd, 0 },
	/* c9 */    { decw, 0 },
	/* ca */    { tstd, 0 },
	/* cb */    { tstw, 0 },
	/* cc */    { absa, 0 },
	/* cd */    { absb, 0 },
	/* ce */    { absd, 0 },
	/* cf */    { bset, 0 },

	/* d0 */    { bset2, 0 },
	/* d1 */    { illegal, 0 },
	/* d2 */    { illegal, 0 },
	/* d3 */    { illegal, 0 },
	/* d4 */    { illegal, 0 },
	/* d5 */    { illegal, 0 },
	/* d6 */    { illegal, 0 },
	/* d7 */    { illegal, 0 },
	/* d8 */    { illegal, 0 },
	/* d9 */    { illegal, 0 },
	/* da */    { illegal, 0 },
	/* db */    { illegal, 0 },
	/* dc */    { illegal, 0 },
	/* dd */    { illegal, 0 },
	/* de */    { illegal, 0 },
	/* df */    { illegal, 0 },

	/* e0 */    { illegal, 0 },
	/* e1 */    { illegal, 0 },
	/* e2 */    { illegal, 0 },
	/* e3 */    { illegal, 0 },
	/* e4 */    { illegal, 0 },
	/* e5 */    { illegal, 0 },
	/* e6 */    { illegal, 0 },
	/* e7 */    { illegal, 0 },
	/* e8 */    { illegal, 0 },
	/* e9 */    { illegal, 0 },
	/* ea */    { illegal, 0 },
	/* eb */    { illegal, 0 },
	/* ec */    { illegal, 0 },
	/* ed */    { illegal, 0 },
	/* ee */    { illegal, 0 },
	/* ef */    { illegal, 0 },

	/* f0 */    { illegal, 0 },
	/* f1 */    { illegal, 0 },
	/* f2 */    { illegal, 0 },
	/* f3 */    { illegal, 0 },
	/* f4 */    { illegal, 0 },
	/* f5 */    { illegal, 0 },
	/* f6 */    { illegal, 0 },
	/* f7 */    { illegal, 0 },
	/* f8 */    { illegal, 0 },
	/* f9 */    { illegal, 0 },
	/* fa */    { illegal, 0 },
	/* fb */    { illegal, 0 },
	/* fc */    { illegal, 0 },
	/* fd */    { illegal, 0 },
	/* fe */    { illegal, 0 },
	/* ff */    { illegal, 0 }
};

CPU_DISASSEMBLE( konami )
{
	buffer[0] = '\0';

	local_pc = pc;
	byte_count = 1;
	opram_ptr = opram;
	flags = 0;

	(op_table[*oprom].decode)( buffer );

	return byte_count | flags | DASMFLAG_SUPPORTED;
}
