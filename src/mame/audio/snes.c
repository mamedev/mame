/***************************************************************************

  snes.c

  File to handle the sound emulation of the Nintendo Super NES.

  By R. Belmont, adapted from OpenSPC 0.3.99 by Brad Martin with permission.
  Thanks to Brad and also to Charles Bilyu? of SNeESe.

  OpenSPC's license terms (the LGPL) follow:

 ---------------------------------------------------------------------------

  Copyright Brad Martin.

  OpenSPC is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  OpenSPC is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

***************************************************************************/

#include "emu.h"
#include "streams.h"
#include "includes/snes.h"

static struct
{
	UINT8 enabled;
	UINT16 counter;
	emu_timer *timer;
} timers[3];
static sound_stream *channel;
static UINT8 DSPregs[256];		/* DSP registers */
static UINT8 snes_ipl_region[64];	/* SPC top 64 bytes */

static const int gauss[]=
{
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001,
	0x001, 0x001, 0x001, 0x002, 0x002, 0x002, 0x002, 0x002,
	0x002, 0x002, 0x003, 0x003, 0x003, 0x003, 0x003, 0x004,
	0x004, 0x004, 0x004, 0x004, 0x005, 0x005, 0x005, 0x005,
	0x006, 0x006, 0x006, 0x006, 0x007, 0x007, 0x007, 0x008,
	0x008, 0x008, 0x009, 0x009, 0x009, 0x00A, 0x00A, 0x00A,
	0x00B, 0x00B, 0x00B, 0x00C, 0x00C, 0x00D, 0x00D, 0x00E,
	0x00E, 0x00F, 0x00F, 0x00F, 0x010, 0x010, 0x011, 0x011,
	0x012, 0x013, 0x013, 0x014, 0x014, 0x015, 0x015, 0x016,
	0x017, 0x017, 0x018, 0x018, 0x019, 0x01A, 0x01B, 0x01B,
	0x01C, 0x01D, 0x01D, 0x01E, 0x01F, 0x020, 0x020, 0x021,
	0x022, 0x023, 0x024, 0x024, 0x025, 0x026, 0x027, 0x028,
	0x029, 0x02A, 0x02B, 0x02C, 0x02D, 0x02E, 0x02F, 0x030,
	0x031, 0x032, 0x033, 0x034, 0x035, 0x036, 0x037, 0x038,
	0x03A, 0x03B, 0x03C, 0x03D, 0x03E, 0x040, 0x041, 0x042,
	0x043, 0x045, 0x046, 0x047, 0x049, 0x04A, 0x04C, 0x04D,
	0x04E, 0x050, 0x051, 0x053, 0x054, 0x056, 0x057, 0x059,
	0x05A, 0x05C, 0x05E, 0x05F, 0x061, 0x063, 0x064, 0x066,
	0x068, 0x06A, 0x06B, 0x06D, 0x06F, 0x071, 0x073, 0x075,
	0x076, 0x078, 0x07A, 0x07C, 0x07E, 0x080, 0x082, 0x084,
	0x086, 0x089, 0x08B, 0x08D, 0x08F, 0x091, 0x093, 0x096,
	0x098, 0x09A, 0x09C, 0x09F, 0x0A1, 0x0A3, 0x0A6, 0x0A8,
	0x0AB, 0x0AD, 0x0AF, 0x0B2, 0x0B4, 0x0B7, 0x0BA, 0x0BC,
	0x0BF, 0x0C1, 0x0C4, 0x0C7, 0x0C9, 0x0CC, 0x0CF, 0x0D2,
	0x0D4, 0x0D7, 0x0DA, 0x0DD, 0x0E0, 0x0E3, 0x0E6, 0x0E9,
	0x0EC, 0x0EF, 0x0F2, 0x0F5, 0x0F8, 0x0FB, 0x0FE, 0x101,
	0x104, 0x107, 0x10B, 0x10E, 0x111, 0x114, 0x118, 0x11B,
	0x11E, 0x122, 0x125, 0x129, 0x12C, 0x130, 0x133, 0x137,
	0x13A, 0x13E, 0x141, 0x145, 0x148, 0x14C, 0x150, 0x153,
	0x157, 0x15B, 0x15F, 0x162, 0x166, 0x16A, 0x16E, 0x172,
	0x176, 0x17A, 0x17D, 0x181, 0x185, 0x189, 0x18D, 0x191,
	0x195, 0x19A, 0x19E, 0x1A2, 0x1A6, 0x1AA, 0x1AE, 0x1B2,
	0x1B7, 0x1BB, 0x1BF, 0x1C3, 0x1C8, 0x1CC, 0x1D0, 0x1D5,
	0x1D9, 0x1DD, 0x1E2, 0x1E6, 0x1EB, 0x1EF, 0x1F3, 0x1F8,
	0x1FC, 0x201, 0x205, 0x20A, 0x20F, 0x213, 0x218, 0x21C,
	0x221, 0x226, 0x22A, 0x22F, 0x233, 0x238, 0x23D, 0x241,
	0x246, 0x24B, 0x250, 0x254, 0x259, 0x25E, 0x263, 0x267,
	0x26C, 0x271, 0x276, 0x27B, 0x280, 0x284, 0x289, 0x28E,
	0x293, 0x298, 0x29D, 0x2A2, 0x2A6, 0x2AB, 0x2B0, 0x2B5,
	0x2BA, 0x2BF, 0x2C4, 0x2C9, 0x2CE, 0x2D3, 0x2D8, 0x2DC,
	0x2E1, 0x2E6, 0x2EB, 0x2F0, 0x2F5, 0x2FA, 0x2FF, 0x304,
	0x309, 0x30E, 0x313, 0x318, 0x31D, 0x322, 0x326, 0x32B,
	0x330, 0x335, 0x33A, 0x33F, 0x344, 0x349, 0x34E, 0x353,
	0x357, 0x35C, 0x361, 0x366, 0x36B, 0x370, 0x374, 0x379,
	0x37E, 0x383, 0x388, 0x38C, 0x391, 0x396, 0x39B, 0x39F,
	0x3A4, 0x3A9, 0x3AD, 0x3B2, 0x3B7, 0x3BB, 0x3C0, 0x3C5,
	0x3C9, 0x3CE, 0x3D2, 0x3D7, 0x3DC, 0x3E0, 0x3E5, 0x3E9,
	0x3ED, 0x3F2, 0x3F6, 0x3FB, 0x3FF, 0x403, 0x408, 0x40C,
	0x410, 0x415, 0x419, 0x41D, 0x421, 0x425, 0x42A, 0x42E,
	0x432, 0x436, 0x43A, 0x43E, 0x442, 0x446, 0x44A, 0x44E,
	0x452, 0x455, 0x459, 0x45D, 0x461, 0x465, 0x468, 0x46C,
	0x470, 0x473, 0x477, 0x47A, 0x47E, 0x481, 0x485, 0x488,
	0x48C, 0x48F, 0x492, 0x496, 0x499, 0x49C, 0x49F, 0x4A2,
	0x4A6, 0x4A9, 0x4AC, 0x4AF, 0x4B2, 0x4B5, 0x4B7, 0x4BA,
	0x4BD, 0x4C0, 0x4C3, 0x4C5, 0x4C8, 0x4CB, 0x4CD, 0x4D0,
	0x4D2, 0x4D5, 0x4D7, 0x4D9, 0x4DC, 0x4DE, 0x4E0, 0x4E3,
	0x4E5, 0x4E7, 0x4E9, 0x4EB, 0x4ED, 0x4EF, 0x4F1, 0x4F3,
	0x4F5, 0x4F6, 0x4F8, 0x4FA, 0x4FB, 0x4FD, 0x4FF, 0x500,
	0x502, 0x503, 0x504, 0x506, 0x507, 0x508, 0x50A, 0x50B,
	0x50C, 0x50D, 0x50E, 0x50F, 0x510, 0x511, 0x511, 0x512,
	0x513, 0x514, 0x514, 0x515, 0x516, 0x516, 0x517, 0x517,
	0x517, 0x518, 0x518, 0x518, 0x518, 0x518, 0x519, 0x519
};

/*========== DEFINES ==========*/

#undef DEBUG
#undef DBG_KEY
#undef DBG_ENV
#undef DBG_PMOD
#undef DBG_BRR
#undef DBG_ECHO
#undef DBG_INTRP

#undef NO_PMOD
#undef NO_ECHO

#define CPU_RATE        ( 1024000 )
#define SAMP_FREQ       ( 32000 )

/*========== VARIABLES ==========*/

static int                     keyed_on;
static int                     keys;       /* 8-bits for 8 voices          */
static voice_state_type        voice_state[ 8 ];

/* Noise stuff */
static int		                noise_cnt;
static int                     noise_lev;

/* These are for the FIR echo filter */
#ifndef NO_ECHO
static short            FIRlbuf[ 8 ];
static short            FIRrbuf[ 8 ];
static int              FIRptr;
static int              echo_ptr;
#endif

/*========== CONSTANTS ==========*/

/* Original SPC DSP took samples 32000 times a second, which is once every
   (1024000/32000 = 32) cycles. */
#ifdef UNUSED_DEFINITION
static const int               TS_CYC = CPU_RATE / SAMP_FREQ;
#endif

/* Ptrs to Gaussian table */
static const int *const G1 = &gauss[ 256 ];
static const int *const G2 = &gauss[ 512 ];
static const int *const G3 = &gauss[ 255 ];
static const int *const G4 = &gauss[ -1  ];

static const int        mask = 0xFF;

/* This table is for envelope timing.  It represents the number of counts
   that should be subtracted from the counter each sample period (32kHz).
   The counter starts at 30720 (0x7800). */
static const int        CNT_INIT = 0x7800;
static const int        ENVCNT[ 0x20 ]
  = {
    0x0000, 0x000F, 0x0014, 0x0018, 0x001E, 0x0028, 0x0030, 0x003C,
    0x0050, 0x0060, 0x0078, 0x00A0, 0x00C0, 0x00F0, 0x0140, 0x0180,
    0x01E0, 0x0280, 0x0300, 0x03C0, 0x0500, 0x0600, 0x0780, 0x0A00,
    0x0C00, 0x0F00, 0x1400, 0x1800, 0x1E00, 0x2800, 0x3C00, 0x7800
    };

/*========== MACROS ==========*/

/* Make reading the ADSR code easier */
#define SL( v )         ( DSPregs[ ( ( v ) << 4 ) + 6 ] >> 5 )
                                    /* Returns SUSTAIN level        */
#define SR( v )         ( DSPregs[ ( ( v ) << 4 ) + 6 ] & 0x1F )
                                    /* Returns SUSTAIN rate         */

/* Handle endianness */
#define LEtoME16( x ) LITTLE_ENDIANIZE_INT16(x)
#define MEtoLE16( x ) LITTLE_ENDIANIZE_INT16(x)

/*========== PROCEDURES ==========*/

static int AdvanceEnvelope          /* Run envelope step & retn ENVX*/
    (
    int                 v           /* Voice to process envelope for*/
    );

static STREAM_UPDATE( snes_sh_update );

/* Privately shared functions (for internal library use only) */

/***** DSP_Reset *****/

static void DSP_Reset                      /* Reset emulated DSP           */
    ( void )
{
int                     i;

#ifdef DEBUG
fprintf( stderr,"DSP_Reset\n" );
#endif

for( i = 0; i < 8; i++ )
    {
#ifndef NO_ECHO
    FIRlbuf[ i ]            = 0;
    FIRrbuf[ i ]            = 0;
#endif
    voice_state[ i ].on_cnt = 0;
    }

#ifndef NO_ECHO
FIRptr   = 0;
echo_ptr = 0;
#endif
keys             = 0;
keyed_on         = 0;
noise_cnt        = 0;
noise_lev        = 0x4000;
DSPregs[ 0x6C ] |= 0xE0;
DSPregs[ 0x4C ]  = 0;
DSPregs[ 0x5C ]  = 0;

}   /* DSP_Reset() */


/***** DSP_Update *****/

static void DSP_Update                     /* Mix one sample of audio      */
    (
    short *             sound_ptr   /* Pointer to mix audio into    */
    )
{
int                     V;
#ifndef NO_ECHO
int                     echo_base;
int                     echol;
int                     echor;
#endif
int                     envx;
int                     m;
int                     outl;
int                     outr;
signed long             outx;       /* Smpl height (must be signed) */
src_dir_type *          sd;
int                     v;
int                     vl;
voice_state_type *      vp;
int                     vr;

sd = ( src_dir_type * )&spc_ram[ ( int )DSPregs[ 0x5D ] << 8 ];

/* Check for reset */
if( DSPregs[ 0x6C ] & 0x80 )
    {
    DSP_Reset();
    }

/* Here we check for keys on/off.  Docs say that successive writes to KON/KOF
   must be separated by at least 2 Ts periods or risk being neglected.
   Therefore DSP only looks at these during an update, and not at the time of
   the write.  Only need to do this once however, since the regs haven't
   changed over the whole period we need to catch up with. */
#ifdef DBG_KEY
DSPregs[ 0x4C ] &= mask;
#endif

/* Keying on a voice resets that bit in ENDX */
DSPregs[ 0x7C ] &= ~DSPregs[ 0x4C ];

/* Question: what is the expected behavior when pitch modulation is enabled on
   voice 0?  Jurassic Park 2 does this.  For now, using outx of zero for first
   voice. */
outx = 0;
/* Same table for noise and envelope */
noise_cnt -= ENVCNT[ DSPregs[ 0x6C ] & 0x1F ];
if( noise_cnt <= 0 )
    {
    noise_cnt = CNT_INIT;
    noise_lev = ( ( ( noise_lev << 13 ) ^ ( noise_lev << 14 ) ) & 0x4000 )
              | ( noise_lev >> 1                                         );
    }
outl  = 0;
outr  = 0;
#ifndef NO_ECHO
echol = 0;
echor = 0;
#endif
for( v = 0, m = 1, V = 0; v < 8; v++, V += 16, m <<= 1 )
    {
    vp = &voice_state[ v ];
    if( vp->on_cnt && ( --vp->on_cnt == 0 ) )
        {
        /* Voice was keyed on */
        keys       |= m;
        keyed_on   |= m;
        vl          = DSPregs[ ( v<<4 ) + 4 ];
        vp->samp_id = *( UINT32 * )&sd[ vl ];
        vp->mem_ptr = LEtoME16( sd[ vl ].vptr );

#ifdef DBG_KEY
        fprintf(
               stderr,
               "Keying on voice %d, samp=0x%04X (0x%02X)\n",
               v,
               vp->mem_ptr,
               vl
               );
#endif

        vp->header_cnt = 0;
        vp->half       = 0;
        vp->envx       = 0;
        vp->end        = 0;
        vp->sampptr    = 0;
        vp->mixfrac    = 3 * 4096;

        /* NOTE: Real SNES does *not* appear to initialize the envelope
           counter to anything in particular.  The first cycle always seems to
           come at a random time sooner than expected; as yet, I have been
           unable to find any pattern.  I doubt it will matter though, so
           we'll go ahead and do the full time for now. */
        vp->envcnt   = CNT_INIT;
        vp->envstate = ATTACK;
        }

    if( DSPregs[ 0x4C ] & m & ~DSPregs[ 0x5C ] )
        {
        /* Voice doesn't come on if key off is set */
        DSPregs[ 0x4C ] &= ~m;
        vp->on_cnt       = 8;

#ifdef DBG_KEY
        fprintf( stderr, "Key on set for voice %d\n", v );
#endif
        }

    if( keys & DSPregs[ 0x5C ] & m )
        {
        /* Voice was keyed off */
        vp->envstate = RELEASE;
        vp->on_cnt   = 0;

#ifdef DBG_KEY
        fprintf( stderr, "Keying off voice %d\n", v );
#endif
        }

    if( !( keys & m & mask ) || ( ( envx = AdvanceEnvelope( v ) ) < 0 ) )
        {
        DSPregs[ V + 8 ] = 0;
        DSPregs[ V + 9 ] = 0;
        outx             = 0;
        continue;
        }

    vp->pitch = LEtoME16( *( ( UINT16 * )&DSPregs[ V + 2 ] ) )
              & 0x3FFF;

#ifndef NO_PMOD
    /* Pitch mod uses OUTX from last voice for this one.  Luckily we haven't
       modified OUTX since it was used for last voice. */
    if(DSPregs[ 0x2D ] & m )
        {
#ifdef DBG_PMOD
        fprintf(
               stderr,
               "Pitch Modulating voice %d, outx=%ld, old pitch=%d, ",
               v,
               outx,
               vp->pitch
               );
#endif
        vp->pitch = ( vp->pitch * ( outx + 32768 ) ) >> 15;
        }
#endif

#ifdef DBG_PMOD
    fprintf( stderr, "pitch=%d\n", vp->pitch );
#endif

    for( ; vp->mixfrac >= 0; vp->mixfrac -= 4096 )
        {
        /* This part performs the BRR decode 'on-the-fly'.  This is more
           correct than the old way, which could be fooled if the data and/or
           the loop point changed while the sample was playing, or if the BRR
           decode didn't produce the same result every loop because of the
           filters.  The event interface still has no chance of keeping up
           with those kinds of tricks, though. */
        if( !vp->header_cnt )
            {
            if( vp->end & 1 )
                {
                /* Docs say ENDX bit is set when decode of block with source
                   end flag set is done.  Does this apply to looping samples?
                   Some info I've seen suggests yes. */
                DSPregs[ 0x7C ] |= m;
                if( vp->end & 2 )
                    {
                    vp->mem_ptr = LEtoME16( sd[ DSPregs[ V + 4 ] ].lptr );

#ifdef DBG_BRR
                    fprintf(
                           stderr,
                           "BRR looping to 0x%04X\n",
                           vp->mem_ptr
                           );
#endif
                    }
                else
                    {
#ifdef DBG_KEY
                    fprintf( stderr, "BRR decode end, voice %d\n", v );
#endif

                    keys &= ~m;
                    DSPregs[ V + 8 ] = 0;
                    vp->envx         = 0;
                    while( vp->mixfrac >= 0 )
                        {
                        vp->sampbuf[ vp->sampptr ] = 0;
                        outx         = 0;
                        vp->sampptr  = ( vp->sampptr + 1 ) & 3;
                        vp->mixfrac -= 4096;
                        }
                    break;
                    }
                }
            vp->header_cnt = 8;
            vl             = ( UINT8 )spc_ram[ vp->mem_ptr++ ];
            vp->range      = vl >> 4;
            vp->end        = vl & 3;
            vp->filter     = ( vl & 12 ) >> 2;

#ifdef DBG_BRR
            fprintf(
                   stderr,
                   "V%d: header read, range=%d, end=%d, filter=%d\n",
                   v,
                   vp->range,
                   vp->end,
                   vp->filter
                   );
#endif
            }
        if( vp->half == 0 )
            {
            vp->half = 1;
            outx     = ( ( signed char )spc_ram[ vp->mem_ptr ] ) >> 4;
            }
        else
            {
            vp->half = 0;
            /* Funkiness to get 4-bit signed to carry through */
            outx   = ( signed char )( spc_ram[ vp->mem_ptr++ ] << 4 );
            outx >>= 4;
            vp->header_cnt--;
            }

#ifdef DBG_BRR
        fprintf(
               stderr,
               "V%d: nybble=%X, ptr=%04X, smp1=%d, smp2=%d\n",
               v,
               outx & 0xF,
               vp->mem_ptr,
               vp->smp1,
               vp->smp2
               );
#endif

        /* For invalid ranges (D,E,F): if the nybble is negative, the result
           is F000.  If positive, 0000.  Nothing else like previous range,
           etc. seems to have any effect.  If range is valid, do the shift
           normally.  Note these are both shifted right once to do the filters
           properly, but the output will be shifted back again at the end. */
        if( vp->range <= 0xC )
            {
            outx = ( outx << vp->range ) >> 1;
            }
        else
            {
            outx &= ~0x7FF;

#ifdef DBG_BRR
            fprintf( stderr, "V%d: invalid range! (%X)\n", v, vp->range );
#endif
            }

#ifdef DBG_BRR
        fprintf(
               stderr,
               "V%d: shifted delta=%04X\n",
               v,
               ( UINT16 )outx
               );
#endif

        switch( vp->filter )
            {
            case 0:
                break;

            case 1:
                outx += ( vp->smp1 >> 1 ) + ( ( -vp->smp1 ) >> 5 );
                break;

            case 2:
                outx += vp->smp1
                      + ( ( -( vp->smp1 + ( vp->smp1 >> 1 ) ) ) >> 5 )
                      - ( vp->smp2 >> 1 ) + ( vp->smp2 >> 5 );
                break;

            case 3:
                outx += vp->smp1
                      + ( ( -( vp->smp1 + ( vp->smp1 << 2 )
                             + ( vp->smp1 << 3 )            ) ) >> 7 )
                      - ( vp->smp2 >> 1                              )
                      + ( ( vp->smp2 + ( vp->smp2 >> 1 )      ) >> 4 );
                break;
            }

        if( outx < ( signed short )0x8000 )
            {
            outx = ( signed short )0x8000;
            }
        else if( outx > ( signed short )0x7FFF )
            {
            outx = ( signed short )0x7FFF;
            }

#ifdef DBG_BRR
        fprintf(
               stderr,
               "V%d: filter + delta=%04X\n",
               v,
               ( UINT16 )outx
               );
#endif

        vp->smp2                   = ( signed short )vp->smp1;
        vp->smp1                   = ( signed short )( outx << 1 );
        vp->sampbuf[ vp->sampptr ] = vp->smp1;

#ifdef DBG_BRR
        fprintf(
               stderr,
               "V%d: final output: %04X\n",
               v,
               vp->sampbuf[ vp->sampptr ]
               );
#endif

        vp->sampptr = ( vp->sampptr + 1 ) & 3;
        }

    if(DSPregs[ 0x3D ] & m )
        {
#ifdef DBG_PMOD
        fprintf( stderr, "Noise enabled, voice %d\n", v );
#endif
        outx = ( signed short )( noise_lev << 1 );
        }
    else
        {
        /* Perform 4-Point Gaussian interpolation.  Take an approximation of a
           Gaussian bell-curve, and move it through the sample data at a rate
           determined by the pitch.  The sample output at any given time is
           the sum of the products of each input sample point with the value
           of the bell-curve corresponding to that point. */
        vl  = vp->mixfrac >> 4;
        vr  = ( ( G4[ -vl ] * vp->sampbuf[ vp->sampptr ] ) >> 11 ) & ~1;
        vr += ( ( G3[ -vl ]
                * vp->sampbuf[ ( vp->sampptr + 1 ) & 3 ] ) >> 11 ) & ~1;
        vr += ( ( G2[ vl ]
                * vp->sampbuf[ ( vp->sampptr + 2 ) & 3 ] ) >> 11 ) & ~1;

        /* This is to do the wrapping properly.  Based on my tests with the
           SNES, it appears clipping is done only if it is the fourth addition
           that would cause a wrap.  If it has already wrapped before the
           fourth addition, it is not clipped. */
        vr  = ( signed short )vr;
        vr += ( ( G1[ vl ]
                * vp->sampbuf[ ( vp->sampptr + 3 ) & 3 ] ) >> 11 ) & ~1;
        if( vr > 32767 )
            {
            vr = 32767;
            }
        else if( vr < -32768 )
            {
            vr = -32768;
            }
        outx = ( signed short )vr;

#ifdef DBG_INTRP
        fprintf(
               stderr,
               "V%d: mixfrac=%d: [%d]*%d + [%d]*%d + [%d]*%d + [%d]*%d = %d\n",
               v,
               vl,
               G1[ vl ],
               vp->sampbuf[ ( vp->sampptr + 3 ) & 3 ],
               G2[ vl ],
               vp->sampbuf[ ( vp->sampptr + 2 ) & 3 ],
               G3[ -vl ],
               vp->sampbuf[ ( vp->sampptr + 1 ) & 3 ],
               G4[ -vl ],
               vp->sampbuf[ vp->sampptr ],
               outx
               );
#endif
        }

    /* Advance the sample position for next update. */
    vp->mixfrac += vp->pitch;

    outx = ( ( outx * envx ) >> 11 ) & ~1;
    DSPregs[ V + 9 ] = outx >> 8;

    vl = ( ( ( int )( signed char )DSPregs[ V     ] ) * outx ) >> 7;
    vr = ( ( ( int )( signed char )DSPregs[ V + 1 ] ) * outx ) >> 7;
    outl += vl;
    outr += vr;
    if( DSPregs[ 0x4D ] & m )
        {
#ifndef NO_ECHO
        echol += vl;
        echor += vr;
#endif
        }
    }
outl = ( outl * ( signed char )DSPregs[ 0x0C ] ) >> 7;
outr = ( outr * ( signed char )DSPregs[ 0x1C ] ) >> 7;

#ifndef NO_ECHO
/* Perform echo.  First, read mem at current location, and put those samples
   into the FIR filter queue. */
#ifdef DBG_ECHO
fprintf(
       stderr,
       "Echo delay=%dms, feedback=%d%%\n",
       DSPregs[ 0x7D ] * 16,
       ( ( signed char )DSPregs[ 0x0D ] * 100 ) / 0x7F
       );
#endif

echo_base = ( ( DSPregs[ 0x6D ] << 8 ) + echo_ptr ) & 0xFFFF;
FIRlbuf[ FIRptr ]
  = ( signed short )LEtoME16(
                            *( UINT16 * )
                              &spc_ram[ echo_base ]
                            );
FIRrbuf[ FIRptr ]
  = ( signed short )LEtoME16(
                            *( UINT16 * )
                              &spc_ram[ echo_base + sizeof( short ) ]
                            );

/* Now, evaluate the FIR filter, and add the results into the final output. */
vl     = FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x7F ];
vr     = FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x7F ];
FIRptr = ( FIRptr + 1 ) & 7;
vl    += FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x6F ];
vr    += FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x6F ];
FIRptr = ( FIRptr + 1 ) & 7;
vl    += FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x5F ];
vr    += FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x5F ];
FIRptr = ( FIRptr + 1 ) & 7;
vl    += FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x4F ];
vr    += FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x4F ];
FIRptr = ( FIRptr + 1 ) & 7;
vl    += FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x3F ];
vr    += FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x3F ];
FIRptr = ( FIRptr + 1 ) & 7;
vl    += FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x2F ];
vr    += FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x2F ];
FIRptr = ( FIRptr + 1 ) & 7;
vl    += FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x1F ];
vr    += FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x1F ];
FIRptr = ( FIRptr + 1 ) & 7;
vl    += FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x0F ];
vr    += FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x0F ];

#ifdef DBG_ECHO
fprintf(
       stderr,
       "FIR Coefficients: %02X %02X %02X %02X %02X %02X %02X %02X\n",
       DSPregs[ 0x0F ],
       DSPregs[ 0x1F ],
       DSPregs[ 0x2F ],
       DSPregs[ 0x3F ],
       DSPregs[ 0x4F ],
       DSPregs[ 0x5F ],
       DSPregs[ 0x6F ],
       DSPregs[ 0x7F ]
       );
#endif

/* FIRptr is left in the position of the oldest sample, the one that will be
   replaced next update. */
outl += vl * ( signed char )DSPregs[ 0x2C ] >> 14;
outr += vr * ( signed char )DSPregs[ 0x3C ] >> 14;

if( !( DSPregs[ 0x6C ] & 0x20 ) )
    {
    /* Add the echo feedback back into the original result, and save that into
       memory for use later. */
    echol += vl * ( signed char )DSPregs[ 0x0D ] >> 14;
    if( echol > 32767 )
        {
        echol = 32767;
        }
    else if( echol < -32768 )
        {
        echol = -32768;
        }
    echor += vr * ( signed char )DSPregs[ 0x0D ] >> 14;
    if( echor > 32767 )
        {
        echor = 32767;
        }
    else if( echor < -32768 )
        {
        echor = -32768;
        }

#ifdef DBG_ECHO
    fprintf(
           stderr,
           "Echo: Writing %04X,%04X at location %04X\n",
           ( UINT16 )echol,
           ( UINT16 )echor,
           echo_base
           );
#endif

    *( UINT16 * )&spc_ram[ echo_base ]
        = MEtoLE16( ( UINT16 )echol );
    *( UINT16 * )&spc_ram[ echo_base + sizeof( short ) ]
        = MEtoLE16( ( UINT16 )echor );
    }

echo_ptr += 2 * sizeof( short );
if( echo_ptr >= ( ( DSPregs[ 0x7D ] & 0xF ) << 11 ) )
    {
    echo_ptr = 0;
    }
#endif                              /* !defined( NO_ECHO )          */

if( sound_ptr )
    {
    if( DSPregs[ 0x6C ] & 0x40 )
        {
        /* MUTE */
#ifdef DEBUG
        fprintf( stderr, "MUTED!\n" );
#endif

        *sound_ptr = 0;
        sound_ptr++;
        *sound_ptr = 0;
        sound_ptr++;
        }
    else
        {
        if( outl > 32767 )
            {
            *sound_ptr = 32767;
            }
        else if( outl < -32768 )
            {
            *sound_ptr = -32768;
            }
        else
            {
            *sound_ptr = outl;
            }
        sound_ptr++;
        if( outr > 32767 )
            {
            *sound_ptr = 32767;
            }
        else if( outr < -32768 )
            {
            *sound_ptr = -32768;
            }
        else
            {
            *sound_ptr = outr;
            }
        sound_ptr++;
        }
    }

}   /* DSP_Update() */


/***** AdvanceEnvelope *****/

static int AdvanceEnvelope          /* Run envelope step & retn ENVX*/
    (
    int                 v           /* Voice to process envelope for*/
    )
{
int                     envx;
int                     cnt;
int                     adsr1;
int                     t;

envx = voice_state[ v ].envx;

if( voice_state[ v ].envstate == RELEASE )
    {
    /* Docs: "When in the state of "key off". the "click" sound is prevented
       by the addition of the fixed value 1/256"  WTF???  Alright, I'm going
       to choose to interpret that this way:  When a note is keyed off, start
       the RELEASE state, which subtracts 1/256th each sample period (32kHz).
       Note there's no need for a count because it always happens every
       update. */
    envx -= 0x8;                    /* 0x8 / 0x800 = 1/256th        */
    if( envx <= 0 )
        {
        envx = 0;
        keys &= ~( 1 << v );
        return -1;
        }
    voice_state[ v ].envx = envx;
    DSPregs[ ( v << 4 ) + 8 ] = envx >> 8;

#ifdef DBG_ENV
    fprintf(
           stderr,
           "ENV voice %d: envx=%03X, state=RELEASE\n",
           v,
           envx
           );
#endif

    return( envx );
    }

cnt   = voice_state[ v ].envcnt;
adsr1 = DSPregs[ ( v << 4 ) + 5 ];
if( adsr1 & 0x80 )
    {
    switch( voice_state[ v ].envstate )
        {
        case ATTACK:
            /* Docs are very confusing.  "AR is multiplied by the fixed value
               1/64..."  I believe it means to add 1/64th to ENVX once every
               time ATTACK is updated, and that's what I'm going to implement.
               */
            t = adsr1 & 0xF;
            if( t == 0xF )
                {
#ifdef DBG_ENV
                fprintf( stderr, "ENV voice %d: instant attack\n", v );
#endif

                envx += 0x400;
                }
            else
                {
                cnt -= ENVCNT[ ( t << 1 ) + 1 ];
                if( cnt > 0 )
                    {
                    break;
                    }
                envx += 0x20;       /* 0x020 / 0x800 = 1/64         */
                cnt   = CNT_INIT;
                }

            if( envx > 0x7FF )
                {
                envx = 0x7FF;
                voice_state[ v ].envstate = DECAY;
                }

#ifdef DBG_ENV
            fprintf(
                   stderr,
                   "ENV voice %d: envx=%03X, state=ATTACK\n",
                   v,
                   envx
                   );
#endif

            voice_state[ v ].envx = envx;

            break;

        case DECAY:
            /* Docs: "DR... [is multiplied] by the fixed value 1-1/256."
               Well, at least that makes some sense.  Multiplying ENVX by
               255/256 every time DECAY is updated. */
            cnt -= ENVCNT[ ( ( adsr1 >> 3 ) & 0xE ) + 0x10 ];
            if( cnt <= 0 )
                {
                cnt   = CNT_INIT;
                envx -= ( ( envx - 1 ) >> 8 ) + 1;
                voice_state[ v ].envx = envx;
                }

            if( envx <= 0x100 * ( SL( v ) + 1 ) )
                {
                voice_state[ v ].envstate = SUSTAIN;
                }

#ifdef DBG_ENV
            fprintf(
                   stderr,
                   "ENV voice %d: envx=%03X, state=DECAY\n",
                   v,
                   envx
                   );
#endif

            break;

        case SUSTAIN:
            /* Docs: "SR [is multiplied] by the fixed value 1-1/256."
               Multiplying ENVX by 255/256 every time SUSTAIN is updated. */
#ifdef DBG_ENV
            if( ENVCNT[ SR( v ) ] == 0 )
                {
                fprintf(
                       stderr,
                       "ENV voice %d: envx=%03X, state=SUSTAIN, zero rate\n",
                       v,
                       envx
                       );
                }
#endif

            cnt -= ENVCNT[ SR( v ) ];
            if( cnt > 0 )
                {
                break;
                }
            cnt   = CNT_INIT;
            envx -= ( ( envx - 1 ) >> 8 ) + 1;

#ifdef DBG_ENV
            fprintf(
                   stderr,
                   "ENV voice %d: envx=%03X, state=SUSTAIN\n",
                   v,
                   envx
                   );
#endif

            voice_state[ v ].envx = envx;

            /* Note: no way out of this state except by explicit KEY OFF (or
               switch to GAIN). */
            break;

        case RELEASE:   /* Handled earlier to prevent GAIN mode from stopping
                           KEY OFF events */
            break;
        }
    }
else
    {
    /* GAIN mode is set
       Note: if the game switches between ADSR and GAIN modes partway
       through, should the count be reset, or should it continue from
       where it was?  Does the DSP actually watch for that bit to
       change, or does it just go along with whatever it sees when it
       performs the update?  I'm going to assume the latter and not
       update the count, unless I see a game that obviously wants the
       other behavior.  The effect would be pretty subtle, in any case.
       */
    t = DSPregs[ ( v << 4 ) + 7 ];
    if( t < 0x80 )
        {
        envx                  = t << 4;
        voice_state[ v ].envx = envx;

#ifdef DBG_ENV
        fprintf(
               stderr,
               "ENV voice %d: envx=%03X, state=DIRECT\n",
               v,
               envx
               );
#endif
        }
    else
        {
        switch( t >> 5 )
            {
            case 4:
                /* Docs: "Decrease (linear): Subtraction of the fixed value
                   1/64." */
                cnt -= ENVCNT[ t & 0x1F ];
                if( cnt > 0 )
                    {
                    break;
                    }
                cnt   = CNT_INIT;
                envx -= 0x020;      /* 0x020 / 0x800 = 1/64th       */
                if( envx < 0 )
                    {
                    envx = 0;
                    }

#ifdef DBG_ENV
                fprintf(
                       stderr,
                       "ENV voice %d: envx=%03X, state=DECREASE\n",
                       v,
                       envx
                       );
#endif

                voice_state[ v ].envx = envx;

                break;

            case 5:
                /* Docs: "Drecrease <sic> (exponential): Multiplication by
                   the fixed value 1-1/256." */
                cnt -= ENVCNT[ t & 0x1F ];
                if( cnt > 0 )
                    {
                    break;
                    }
                cnt = CNT_INIT;
                envx -= ( ( envx - 1 ) >> 8 ) + 1;

#ifdef DBG_ENV
                fprintf(
                       stderr,
                       "ENV voice %d: envx=%03X, state=EXP\n",
                       v,
                       envx
                       );
#endif

                voice_state[ v ].envx = envx;

                break;

            case 6:
                /* Docs: "Increase (linear): Addition of the fixed value
                   1/64." */
                cnt -= ENVCNT[ t & 0x1F ];
                if( cnt > 0 )
                {
                    break;
                }
                cnt = CNT_INIT;
                envx += 0x020;      /* 0x020 / 0x800 = 1/64th   */
                if( envx > 0x7FF )
                {
                    envx = 0x7FF;
                }

#ifdef DBG_ENV
                fprintf(
                       stderr,
                       "ENV voice %d: envx=%03X, state=INCREASE\n",
                       v,
                       envx
                       );
#endif

                voice_state[ v ].envx = envx;

                break;

            case 7:
                /* Docs: "Increase (bent line): Addition of the constant
                   1/64 up to .75 of the constaint <sic> 1/256 from .75 to
                   1." */
                cnt -= ENVCNT[ t & 0x1F ];
                if( cnt > 0 )
                    {
                    break;
                    }
                cnt = CNT_INIT;
                if( envx < 0x600 )  /* 0x600 / 0x800 = .75          */
                    {
                    envx += 0x020;  /* 0x020 / 0x800 = 1/64         */
                    }
                else
                    {
                    envx += 0x008;  /* 0x008 / 0x800 = 1/256        */
                    }
                if( envx > 0x7FF )
                    {
                    envx=0x7FF;
                    }

#ifdef DBG_ENV
                fprintf(
                       stderr,
                       "ENV voice %d: envx=%03X, state=INCREASE\n",
                       v,
                       envx
                       );
#endif

                voice_state[ v ].envx = envx;

                break;
            }
        }
    }

voice_state[ v ].envcnt   = cnt;
DSPregs[ ( v << 4 ) + 8 ] = envx >> 4;

return( envx );

}    /* AdvanceEnvelope() */

static TIMER_CALLBACK( snes_spc_timer  )
{
	int which = param;

	timers[which].counter++;
	if( timers[which].counter >= spc_ram[0xfa + which] ) // minus =
	{
		timers[which].counter = 0;
		spc_ram[0xfd + which]++;
		spc_ram[0xfd + which]&= 0xf;
	}
}

static DEVICE_START( snes_sound )
{
	running_machine *machine = device->machine;
	UINT8 ii;

	/* put IPL image at the top of RAM */
	memcpy(snes_ipl_region, memory_region(machine, "user5"), 64);

	/* default to ROM visible */
	spc_ram[0xf1] = 0x80;

	/* Sort out the ports */
	for( ii = 0; ii < 4; ii++ )
	{
		spc_port_in[ii] = 0;
		spc_port_out[ii] = 0;
	}

	channel = stream_create( device, 0, 2, 32000, NULL, snes_sh_update );

	/* Initialize the timers */
	timers[0].timer = timer_alloc(machine, snes_spc_timer , NULL);
	timer_adjust_periodic( timers[0].timer, ATTOTIME_IN_HZ(8000), 0, ATTOTIME_IN_HZ(8000) );
	timer_enable( timers[0].timer, 0 );
	timers[1].timer = timer_alloc(machine, snes_spc_timer , NULL);
	timer_adjust_periodic( timers[1].timer, ATTOTIME_IN_HZ(8000), 1, ATTOTIME_IN_HZ(8000) );
	timer_enable( timers[1].timer, 0 );
	timers[2].timer = timer_alloc(machine, snes_spc_timer , NULL);
	timer_adjust_periodic( timers[2].timer, ATTOTIME_IN_HZ(64000), 2, ATTOTIME_IN_HZ(64000) );
	timer_enable( timers[2].timer, 0 );

	DSP_Reset();
}

DEVICE_GET_INFO( snes_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(snes_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "SNES Custom");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


static STREAM_UPDATE( snes_sh_update )
{
	int i;
	short mix[2];

	for (i = 0; i < samples; i++)
	{
		mix[0] = mix[1] = 0;
		DSP_Update(mix);

		/* Update the buffers */
		outputs[0][i] = (stream_sample_t)mix[0];
		outputs[1][i] = (stream_sample_t)mix[1];
	}
}

/***************************
 *       I/O for DSP       *
 ***************************/

static READ8_HANDLER( snes_dsp_io_r )
{
	stream_update(channel);

#ifdef NO_ENVX
	if( 8 == ( SPCRAM[ 0xF2 ] & 0xF ) )
	{
		DSPregs[ SPCRAM[ 0xF2 ] ] = 0;
	}
#endif

	return DSPregs[offset & 0x7f];
}

static WRITE8_HANDLER( snes_dsp_io_w )
{
	stream_update(channel);

	if( 0x7C == offset )
	{
		DSP_WRITE_7C(data);
	}
	else
	{
		DSPregs[ offset ] = data;
	}
}

/***************************
 *     I/O for SPC700      *
 ***************************/
READ8_HANDLER( spc_io_r )
{
	switch( offset )	/* Offset is from 0x00f0 */
	{
		case 0x0: //FIXME: Super Bomberman PBW reads from there, is it really write-only?
			return 0;
		case 0x1:
			return 0; //Super Kick Boxing reads port 1 and wants it to be zero.
		case 0x2:		/* Register address */
			return spc_ram[0xf2];
		case 0x3:		/* Register data */
			return snes_dsp_io_r( space, spc_ram[0xf2] );
		case 0x4:		/* Port 0 */
		case 0x5:		/* Port 1 */
		case 0x6:		/* Port 2 */
		case 0x7:		/* Port 3 */
//          mame_printf_debug("SPC: rd %02x @ %d, PC=%x\n", spc_port_in[offset-4], offset-4, cpu_get_pc(space->cpu));
			return spc_port_in[offset - 4];
		case 0x8: //normal RAM, can be read even if the ram disabled flag ($f0 bit 1) is active
		case 0x9:
			return spc_ram[0xf0 + offset];
		case 0xA:		/* Timer 0 */
		case 0xB:		/* Timer 1 */
		case 0xC:		/* Timer 2 */
			break;
		case 0xD:		/* Counter 0 */
		case 0xE:		/* Counter 1 */
		case 0xF:		/* Counter 2 */
		{
			UINT8 value = spc_ram[0xf0 + offset] & 0xf;
			spc_ram[0xf0 + offset] = 0;
			return value;
		}
	}

	return 0;
}

WRITE8_HANDLER( spc_io_w )
{
	switch( offset )	/* Offset is from 0x00f0 */
	{
		case 0x0:
			printf("Warning: write to SOUND TEST register with data %02x!\n",data);
			break;
		case 0x1:		/* Control */
			if( data & 0x1 && !timers[0].enabled )
			{
				timers[0].counter = 0;
				spc_ram[0xfd] = 0;
			}
			if( data & 0x2 && !timers[1].enabled )
			{
				timers[1].counter = 0;
				spc_ram[0xfe] = 0;
			}
			if( data & 0x4 && !timers[2].enabled )
			{
				timers[2].counter = 0;
				spc_ram[0xff] = 0;
			}
			timers[0].enabled = data & 0x1;
			timer_enable( timers[0].timer, timers[0].enabled );
			timers[1].enabled = (data & 0x2) >> 1;
			timer_enable( timers[1].timer, timers[1].enabled );
			timers[2].enabled = (data & 0x4) >> 2;
			timer_enable( timers[2].timer, timers[2].enabled );
			if( data & 0x10 )
			{
				spc_port_in[0] = 0;
				spc_port_in[1] = 0;
			}
			if( data & 0x20 )
			{
				spc_port_in[2] = 0;
				spc_port_in[3] = 0;
			}

			if ((data & 0x80) != (spc_ram[0xf1] & 0x80))
			{
				if (data & 0x80)
				{
					memcpy(snes_ipl_region, memory_region(space->machine, "user5"), 64);
				}
				else
				{
					memcpy(snes_ipl_region, &spc_ram[0xffc0], 64);
				}
			}
			break;
		case 0x2:		/* Register address */
			break;
		case 0x3:		/* Register data */
			snes_dsp_io_w( space, spc_ram[0xf2], data );
			break;
		case 0x4:		/* Port 0 */
		case 0x5:		/* Port 1 */
		case 0x6:		/* Port 2 */
		case 0x7:		/* Port 3 */
//          mame_printf_debug("SPC: %02x to APU @ %d (PC=%x)\n", data, offset&3, cpu_get_pc(space->cpu));
			spc_port_out[offset - 4] = data;
			cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(20));
			break;
		case 0xA:		/* Timer 0 */
		case 0xB:		/* Timer 1 */
		case 0xC:		/* Timer 2 */
			if (data == 0) data = 255;
			break;
		case 0xD:		/* Counter 0 */
		case 0xE:		/* Counter 1 */
		case 0xF:		/* Counter 2 */
			return;
	}

	spc_ram[0xf0 + offset] = data;
}

READ8_HANDLER( spc_ram_r )
{
	return spc_ram[offset];
}

WRITE8_HANDLER( spc_ram_w )
{
	spc_ram[offset] = data;

	// if RAM is mapped in, mirror accordingly
	if ((!(spc_ram[0xf1] & 0x80)) && (offset >= 0xffc0))
	{
		snes_ipl_region[offset-0xffc0] = data;
	}
}

READ8_HANDLER( spc_ipl_r )
{
	return snes_ipl_region[offset];
}

