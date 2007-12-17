/*****************************************************************************

  MAME/MESS NES APU CORE

  Based on the Nofrendo/Nosefart NES N2A03 sound emulation core written by
  Matthew Conte (matt@conte.com) and redesigned for use in MAME/MESS by
  Who Wants to Know? (wwtk@mail.com)

  This core is written with the advise and consent of Matthew Conte and is
  released under the GNU Public License.  This core is freely avaiable for
  use in any freeware project, subject to the following terms:

  Any modifications to this code must be duly noted in the source and
  approved by Matthew Conte and myself prior to public submission.

 *****************************************************************************

   NES_DEFS.H

   NES APU internal type definitions and constants.

 *****************************************************************************/

#ifndef NESTYPES_H
#define NESTYPES_H

/* BOOLEAN CONSTANTS */
#ifndef TRUE
#define TRUE   1
#define FALSE  0
#endif

/* REGULAR TYPE DEFINITIONS */
typedef INT8          int8;
typedef INT16         int16;
typedef INT32         int32;
typedef UINT8         uint8;
typedef UINT16        uint16;
typedef UINT32        uint32;
typedef UINT8         boolean;


/* QUEUE TYPES */
#ifdef USE_QUEUE

#define QUEUE_SIZE 0x2000
#define QUEUE_MAX  (QUEUE_SIZE-1)

typedef struct queue_s
{
  int pos;
  unsigned char reg,val;
} queue_t;

#endif

/* REGISTER DEFINITIONS */
#define  APU_WRA0    0x00
#define  APU_WRA1    0x01
#define  APU_WRA2    0x02
#define  APU_WRA3    0x03
#define  APU_WRB0    0x04
#define  APU_WRB1    0x05
#define  APU_WRB2    0x06
#define  APU_WRB3    0x07
#define  APU_WRC0    0x08
#define  APU_WRC2    0x0A
#define  APU_WRC3    0x0B
#define  APU_WRD0    0x0C
#define  APU_WRD2    0x0E
#define  APU_WRD3    0x0F
#define  APU_WRE0    0x10
#define  APU_WRE1    0x11
#define  APU_WRE2    0x12
#define  APU_WRE3    0x13
#define  APU_SMASK   0x15
#define  APU_IRQCTRL 0x17

#define  NOISE_LONG     0x4000
#define  NOISE_SHORT    93

/* CHANNEL TYPE DEFINITIONS */

/* Square Wave */
typedef struct square_s
{
   uint8 regs[4];
   int vbl_length;
   int freq;
   float phaseacc;
   float output_vol;
   float env_phase;
   float sweep_phase;
   uint8 adder;
   uint8 env_vol;
   boolean enabled;
} square_t;

/* Triangle Wave */
typedef struct triangle_s
{
   uint8 regs[4]; /* regs[1] unused */
   int linear_length;
   int vbl_length;
   int write_latency;
   float phaseacc;
   float output_vol;
   uint8 adder;
   boolean counter_started;
   boolean enabled;
} triangle_t;

/* Noise Wave */
typedef struct noise_s
{
   uint8 regs[4]; /* regs[1] unused */
   int cur_pos;
   int vbl_length;
   float phaseacc;
   float output_vol;
   float env_phase;
   uint8 env_vol;
   boolean enabled;
} noise_t;

/* DPCM Wave */
typedef struct dpcm_s
{
   uint8 regs[4];
   uint32 address;
   uint32 length;
   int bits_left;
   float phaseacc;
   float output_vol;
   uint8 cur_byte;
   boolean enabled;
   boolean irq_occurred;
   uint8 *cpu_mem;
   signed char vol;
} dpcm_t;

/* APU type */
typedef struct apu
{
   /* Sound channels */
   square_t   squ[2];
   triangle_t tri;
   noise_t    noi;
   dpcm_t     dpcm;

   /* APU registers */
   unsigned char regs[0x17];

   /* Sound pointers */
   void *buffer;

#ifdef USE_QUEUE

   /* Event queue */
   queue_t queue[QUEUE_SIZE];
   int head,tail;

#else

   int buf_pos;

#endif

} apu_t;

/* CONSTANTS */

/* vblank length table used for squares, triangle, noise */
static const uint8 vbl_length[32] =
{
   5, 127, 10, 1, 19,  2, 40,  3, 80,  4, 30,  5, 7,  6, 13,  7,
   6,   8, 12, 9, 24, 10, 48, 11, 96, 12, 36, 13, 8, 14, 16, 15
};

/* frequency limit of square channels */
static const int freq_limit[8] =
{
   0x3FF, 0x555, 0x666, 0x71C, 0x787, 0x7C1, 0x7E0, 0x7F0,
};

/* table of noise frequencies */
static const int noise_freq[16] =
{
   4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 2046
};

/* dpcm transfer freqs */
static const int dpcm_clocks[16] =
{
   428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 85, 72, 54
};

/* ratios of pos/neg pulse for square waves */
/* 2/16 = 12.5%, 4/16 = 25%, 8/16 = 50%, 12/16 = 75% */
static const int duty_lut[4] =
{
   2, 4, 8, 12
};

#endif
