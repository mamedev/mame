/*

    Megadrive / Genesis support
    PRELIMINARY

    The Megadrive / Genesis form the basis of a number of official Sega Arcade PCBs
    as well as a selection of bootlegs.

    Current Issues

    Timing is wrong
     -- DMA timing not emulated
     -- Sprite render timing incorrect
     -- Interrupt Timing Problems

    Known Problems
     -- g_lem / Lemmings (JU) (REV01) [!]
      Rasters are off
     -- g_drac / Bram Stoker's Dracula (U) [!]
      Doesn't work, Timing Sensisitve
     -- g_sscc / Sesame Street Counting Cafe (U) [!]
      Doesn't work
     -- g_fatr / Fatal Rewind (UE) [!] (and clones)
      Doesn't work. Timing Sensitive

     -- various
      Rasters off by 1 line, bottom line corrupt? bad frame timing?

      + more

    ToDo:

    Fix bugs - comprehensive testing!

    Add SegaCD

    Fix 32X support (not used by any arcade systems?)
     - this seems to require far greater sync and timing accuracy on rom / ram access than MAME can provide
     - split NTSC / PAL drivers
     - 36greatju: missing backup ram, has issues with golfer select due of that
     - bcracers: write to undefined PWM register?
     - fifa96 / nbajamte: dies on the gameplay, waiting for a comm change that never occurs;
     - marsch1: doesn't boot, Master / Slave communicates through SCI
     - nbajamte: missing I2C hookup, startup fails due of that (same I2C type as plain MD version);
     - nflquart: black screen, missing h irq?
     - sangoku4: black screen after the Sega logo
     - soulstar: OSD and player sprite isn't drawn;
     - tempo: intro is too fast, mostly noticeable with the PWM sound that cuts off too early when it gets to the title screen;
     - tmek: gameplay is clearly too fast
     - vrdxu: has 3d geometry bugs, caused by a SH-2 DRC bug;
     - vrdxu: crashes if you attempt to enter into main menu;
     - wwfraw: writes fb data to the cart area and expects it to be read back, kludging the cart area to be writeable makes the 32x gfxs to appear, why?
     - wwfwre: no 32x gfxs
     - xmen: black screen after that you choose the level, needs bare minimum SH-2 SCI support

    Add PicoDrive support (not arcade)

    Change SegaC2 to use the VDP emulation here
    Change System18 to use the VDP emulation here

Known Non-Issues (confirmed on Real Genesis)
    Castlevania - Bloodlines (U) [!] - Pause text is missing on upside down level
    Blood Shot (E) (M4) [!] - corrupt texture in level 1 is correct...

Cleanup / Rewrite notes:

On SegaC2 the VDP never turns on the IRQ6 enable register
  This is because on the real PCB that line of the VDP isn't
  connected.  Instead the IRQ6 interrupt is triggered by the
  line that is used to generate the Z80 interrupt on a standard
  genesis.  (Once, every frame, on screenline 224)

  I should provide interrupt callback functions for each
  vdp line state change, which can be configured in the init
  rather than hardcoding them.

32x Marsch tests documentation (keep start pressed at start-up for individual tests):

MD side check:
#1 Communication Check
#2 FM Bit
#3 Irq Register
#4 Bank Control Register
#5 DREQ Control FULL bit
#6 DREQ SRC Address
#7 DREQ DST Address
#8 DREQ SIZE Address
#9 SEGA TV Register
#10 H IRQ Vector
#11 PWM Control Register
#12 PWM Frequency Register
#13 PWM Lch Pulse Width Register
#14 PWM Rch Pulse Width Register
#15 PWM MONO Pulse Width Register
32x side check:
#16 SH-2 Master Communication Check
#17 SH-2 Slave Communication Check
#18 SH-2 Master FM Bit
#19 SH-2 Slave FM Bit
#20 SH-2 Master IRQ Mask Register
#21 SH-2 Slave IRQ Mask Register
#22 SH-2 Master H Counter Register
#23 SH-2 Slave H Counter Register
#24 SH-2 Master PWM Timer Register
#25 SH-2 Slave PWM Timer Register
#26 SH-2 Master PWM Cont. Register
#27 SH-2 Slave PWM Cont. Register
#28 SH-2 Master PWM Freq. Register
#29 SH-2 Slave PWM Freq. Register
#30 SH-2 Master PWM Lch Register
#31 SH-2 Slave PWM Lch Register
#32 SH-2 Master PWM Rch Register
#33 SH-2 Slave PWM Rch Register
#34 SH-2 Master PWM Mono Register
#35 SH-2 Slave PWM Mono Register
#36 SH-2 Master ROM Read Check
#37 SH-2 Slave ROM Read Check
#38 SH-2 Serial Communication (ERROR - returns a Timeout Error)
MD & 32x check:
#39 MD&SH-2 Master Communication
#40 MD&SH-2 Slave Communication
#41 MD&SH-2 Master FM Bit R/W
#42 MD&SH-2 Slave FM Bit R/W
#43 MD&SH-2 Master DREQ CTL
#44 MD&SH-2 Slave DREQ CTL
#45 MD&SH-2 Master DREQ SRC address
#46 MD&SH-2 Slave DREQ SRC address
#47 MD&SH-2 Master DREQ DST address
#48 MD&SH-2 Slave DREQ DST address
#49 MD&SH-2 Master DREQ SIZE address
#50 MD&SH-2 Slave DREQ SIZE address
#51 SH-2 Master V IRQ
#52 SH-2 Slave V IRQ
#53 SH2 Master H IRQ (MD 0)
#54 SH2 Slave H IRQ (MD 0)
#55 SH2 Master H IRQ (MD 1)
#56 SH2 Slave H IRQ (MD 1)
#57 SH2 Master H IRQ (MD 2)
#58 SH2 Slave H IRQ (MD 2)
MD VDP check:
#59 Bitmap Mode Register
#60 Shift Register
#61 Auto Fill Length Register
#62 Auto Fill Start Address Register
#63 V Blank BIT
#64 H Blank BIT
#65 Palette Enable BIT
SH-2 VDP check:
#66 Frame Swap BIT
#67 SH-2 Master Bitmap MD
#68 SH-2 Slave Bitmap MD
#69 SH-2 Master Shift
#70 SH-2 Slave Shift
#71 SH-2 Master Fill SIZE
#72 SH-2 Slave Fill SIZE
#73 SH-2 Master Fill START
#74 SH-2 Slave Fill START
#75 SH-2 Master V Blank Bit
#76 SH-2 Slave V Blank Bit
#77 SH-2 Master H Blank Bit
#78 SH-2 Slave H Blank Bit
#79 SH-2 Master Palette Enable Bit
#80 SH-2 Slave Palette Enable Bit
#81 SH-2 Master Frame Swap Bit
#82 SH-2 Slave Frame Swap Bit
Framebuffer Check:
#83 MD Frame Buffer 0
#84 MD Frame Buffer 1
#85 SH-2 Master Frame Buffer 0
#86 SH-2 Slave Frame Buffer 0
#87 SH-2 Master Frame Buffer 1
#88 SH-2 Slave Frame Buffer 1
#89 MD Frame Buffer 0 Overwrite
#90 MD Frame Buffer 1 Overwrite
#91 MD Frame Buffer 0 Byte Write
#92 MD Frame Buffer 1 Byte Write
#93 SH-2 Master Frame Buffer 0 Overwrite
#94 SH-2 Slave Frame Buffer 0 Overwrite
#95 SH-2 Master Frame Buffer 1 Overwrite
#96 SH-2 Slave Frame Buffer 1 Overwrite
#97 SH-2 Master Frame Buffer 0 Byte Write
#98 SH-2 Slave Frame Buffer 0 Byte Write
#99 SH-2 Master Frame Buffer 1 Byte Write
#100 SH-2 Slave Frame Buffer 1 Byte Write
#101 MD Frame Buffer 0 Fill Data
#102 MD Frame Buffer 1 Fill Data
#103 MD Frame Buffer 0 Fill Length & Address
#104 MD Frame Buffer 1 Fill Length & Address
#105 SH-2 Master Frame Buffer 0 Fill Data
#106 SH-2 Slave Frame Buffer 0 Fill Data
#107 SH-2 Master Frame Buffer 1 Fill Data
#108 SH-2 Slave Frame Buffer 1 Fill Data
#109 SH-2 Master Frame Buffer 0 Fill Address
#110 SH-2 Slave Frame Buffer 0 Fill Address
#111 SH-2 Master Frame Buffer 1 Fill Address
#112 SH-2 Slave Frame Buffer 1 Fill Address
#113 MD Palette R/W (Blank Mode)
#114 MD Palette R/W (Display Mode)
#115 MD Palette R/W (Fill Mode)
#116 SH-2 Master Palette R/W (Blank Mode)
#117 SH-2 Slave Palette R/W (Blank Mode)
#118 SH-2 Master Palette R/W (Display Mode)
#119 SH-2 Slave Palette R/W (Display Mode)
#120 SH-2 Master Palette R/W (Fill Mode)
#121 SH-2 Slave Palette R/W (Fill Mode)
MD or SH-2 DMA check:
#122 SH-2 Master CPU Write DMA (68S) (ERROR)
#123 SH-2 Slave CPU Write DMA (68S) (ERROR)
#124 MD ROM to VRAM DMA (asserts after this)
-----
#127 SH-2 Master ROM to SDRAM DMA
#128 SH-2 Slave ROM to SDRAM DMA
#129 SH-2 Master ROM to Frame DMA
#130 SH-2 Slave ROM to Frame DMA
#131 SH-2 Master SDRAM to Frame DMA
#132 SH-2 Slave SDRAM to Frame DMA
#133 SH-2 Master Frame to SDRAM DMA
#134 SH-2 Slave Frame to SDRAM DMA
Sound Test (these don't explicitly fails):
#135 MD 68k Monaural Sound
#136 MD 68k L Sound
#137 MD 68k R Sound
#138 MD 68k L -> R Sound
#139 MD 68k R -> L Sound
#140 SH-2 Master Monaural Sound
#141 SH-2 Master L Sound
#142 SH-2 Master R Sound
#143 SH-2 Master L -> R Pan
#144 SH-2 Master R -> L Pan
#145 SH-2 Slave Monaural Sound
#146 SH-2 Slave L Sound
#147 SH-2 Slave R Sound
#148 SH-2 Slave L -> R Pan
#149 SH-2 Slave R -> L Pan
#150 SH-2 Master PWM Interrupt
#151 SH-2 Slave PWM Interrupt
#152 SH-2 Master PWM DMA Write (!)
#153 SH-2 Slave PWM DMA Write (!)
#154 Z80 PWM Monaural Sound (!)
#155 Z80 PWM L Sound (!)
#156 Z80 PWM R Sound (!)
GFX check (these don't explicitly fails):
#157 Direct Color Mode
#158 Packed Pixel Mode
#159 Runlength Mode
#160 Runlength Mode
#161 Runlength Mode


----------------------------
SegaCD notes
----------------------------

the use of the MAME tilemap system for the SegaCD 'Roz tilemap' isn't intended as a long-term
solution.  (In reality it's not a displayable tilemap anyway, just a source buffer which has
a tilemap-like structure, from which data is copied)

*/


#include "emu.h"
#include "coreutil.h"
#include "cpu/m68000/m68000.h"
#include "cpu/sh2/sh2.h"
#include "cpu/sh2/sh2comn.h"
#include "cpu/z80/z80.h"
#include "sound/2612intf.h"
#include "sound/cdda.h"
#include "sound/dac.h"
#include "sound/rf5c68.h"
#include "sound/sn76496.h"
#include "imagedev/chd_cd.h"
#include "includes/megadriv.h"
#include "machine/nvram.h"


#define MEGADRIV_VDP_VRAM(address) megadrive_vdp_vram[(address)&0x7fff]

#define HAZE_MD 0 // to make appear / disappear the Region DipSwitch

/* timing details */
static int megadriv_framerate;
static int megadrive_total_scanlines;
static int megadrive_visible_scanlines;
static int megadrive_irq6_scanline;
//int megadrive_irq6_hpos = 320;
static int megadrive_z80irq_scanline;
//int megadrive_z80irq_hpos = 320;
static int megadrive_imode = 0;
static int megadrive_imode_odd_frame = 0;
static int megadrive_vblank_flag = 0;
static int megadrive_irq6_pending = 0;
static int megadrive_irq4_pending = 0;

/* 32x! */
static cpu_device *_32x_master_cpu;
static cpu_device *_32x_slave_cpu;
static int _32x_is_connected;

static int sh2_are_running;
static int _32x_adapter_enabled;
static int _32x_access_auth;
static int _32x_screenshift;
static int _32x_videopriority;

static int _32x_displaymode;
static int _32x_240mode;

static UINT16 _32x_68k_a15104_reg;


static int sh2_master_vint_enable, sh2_slave_vint_enable;
static int sh2_master_hint_enable, sh2_slave_hint_enable;
static int sh2_master_cmdint_enable, sh2_slave_cmdint_enable;
static int sh2_master_pwmint_enable, sh2_slave_pwmint_enable;
static int sh2_hint_in_vbl;

static int sh2_master_vint_pending;
static int sh2_slave_vint_pending;
static int _32x_fb_swap;
static int _32x_hcount_reg,_32x_hcount_compare_val;

void _32x_check_irqs(running_machine& machine);

#define SH2_VRES_IRQ_LEVEL 14
#define SH2_VINT_IRQ_LEVEL 12
#define SH2_HINT_IRQ_LEVEL 10
#define SH2_CINT_IRQ_LEVEL 8
#define SH2_PINT_IRQ_LEVEL 6

// Fifa96 needs the CPUs swapped for the gameplay to enter due to some race conditions
// when using the DRC core.  Needs further investigation, the non-DRC core works either
// way
#define _32X_SWAP_MASTER_SLAVE_HACK

static UINT16* _32x_dram0;
static UINT16* _32x_dram1;
static UINT16 *_32x_display_dram, *_32x_access_dram;
static UINT16* _32x_palette;
static UINT16* _32x_palette_lookup;
/* SegaCD! */
static cpu_device *_segacd_68k_cpu;
static emu_timer *segacd_gfx_conversion_timer;
//static emu_timer *segacd_dmna_ret_timer;
static emu_timer *segacd_irq3_timer;
static emu_timer *segacd_hock_timer;
static UINT8 hock_cmd;
static int segacd_wordram_mapped = 0;
static TIMER_CALLBACK( segacd_irq3_timer_callback );

/* SVP (virtua racing) */
static cpu_device *_svp_cpu;


static cpu_device *_genesis_snd_z80_cpu;

int segac2_bg_pal_lookup[4];
int segac2_sp_pal_lookup[4];

// hacks for C2
int genvdp_use_cram = 0; // c2 uses it's own palette ram
int genesis_always_irq6 = 0; // c2 never enables the irq6, different source??
int genesis_other_hacks = 0; // misc hacks

INLINE UINT16 get_hposition(void);

static UINT8* sprite_renderline;
static UINT8* highpri_renderline;
static UINT32* video_renderline;
UINT16* megadrive_vdp_palette_lookup;
UINT16* megadrive_vdp_palette_lookup_sprite; // for C2
UINT16* megadrive_vdp_palette_lookup_shadow;
UINT16* megadrive_vdp_palette_lookup_highlight;
UINT16* megadrive_ram;
static UINT8 megadrive_vram_fill_pending = 0;
static UINT16 megadrive_vram_fill_length = 0;
static int genesis_scanline_counter = 0;
static int megadrive_sprite_collision = 0;
static int megadrive_max_hposition;
int megadrive_region_export;
int megadrive_region_pal;


static timer_device* frame_timer;
static timer_device* scanline_timer;
static timer_device* irq6_on_timer;
static timer_device* irq4_on_timer;
static bitmap_ind16* render_bitmap;
//emu_timer* vblankirq_off_timer;

/* Sega CD stuff */
static int sega_cd_connected = 0x00;
UINT16 segacd_irq_mask;
static UINT16 *segacd_backupram;
static timer_device *stopwatch_timer;
static UINT8 segacd_font_color;
static UINT16* segacd_font_bits;


static UINT16 scd_rammode;
static UINT32 scd_mode_dmna_ret_flags ;

static void segacd_mark_tiles_dirty(running_machine& machine, int offset);

static struct genesis_z80_vars
{
	int z80_is_reset;
	int z80_has_bus;
	UINT32 z80_bank_addr;
	UINT8* z80_prgram;
} genz80;

static void megadriv_z80_bank_w(UINT16 data)
{
	genz80.z80_bank_addr = ( ( genz80.z80_bank_addr >> 1 ) | ( data << 23 ) ) & 0xff8000;
}

static WRITE16_HANDLER( megadriv_68k_z80_bank_write )
{
	//logerror("%06x: 68k writing bit to bank register %01x\n", cpu_get_pc(&space->device()),data&0x01);
	megadriv_z80_bank_w(data&0x01);
}

static WRITE8_HANDLER(megadriv_z80_z80_bank_w)
{
	//logerror("%04x: z80 writing bit to bank register %01x\n", cpu_get_pc(&space->device()),data&0x01);
	megadriv_z80_bank_w(data&0x01);
}


static READ16_HANDLER( megadriv_68k_check_z80_bus );
static WRITE16_HANDLER(megadriv_68k_req_z80_bus);

static READ16_HANDLER( megadriv_68k_read_z80_ram );
static WRITE16_HANDLER( megadriv_68k_write_z80_ram );

static WRITE16_HANDLER( megadriv_68k_req_z80_reset );





/*  The VDP occupies addresses C00000h to C0001Fh.

 C00000h    -   Data port (8=r/w, 16=r/w)
 C00002h    -   Data port (mirror)
 C00004h    -   Control port (8=r/w, 16=r/w)
 C00006h    -   Control port (mirror)
 C00008h    -   HV counter (8/16=r/o)
 C0000Ah    -   HV counter (mirror)
 C0000Ch    -   HV counter (mirror)
 C0000Eh    -   HV counter (mirror)
 C00011h    -   SN76489 PSG (8=w/o)
 C00013h    -   SN76489 PSG (mirror)
 C00015h    -   SN76489 PSG (mirror)
 C00017h    -   SN76489 PSG (mirror)
*/




static int megadrive_vdp_command_pending; // 2nd half of command pending..
static UINT16 megadrive_vdp_command_part1;
static UINT16 megadrive_vdp_command_part2;
static UINT8  megadrive_vdp_code;
static UINT16 megadrive_vdp_address;
static UINT16 megadrive_vdp_register[0x20];
static UINT16* megadrive_vdp_vram;
static UINT16* megadrive_vdp_cram;
static UINT16* megadrive_vdp_vsram;
/* The VDP keeps a 0x400 byte on-chip cache of the Sprite Attribute Table
   to speed up processing */
static UINT16* megadrive_vdp_internal_sprite_attribute_table;

/*

 $00 - Mode Set Register No. 1
 -----------------------------

 d7 - No effect
 d6 - No effect
 d5 - No effect
 d4 - IE1 (Horizontal interrupt enable)
 d3 - 1= Invalid display setting
 d2 - Palette select
 d1 - M3 (HV counter latch enable)
 d0 - Display disable

 */

#define MEGADRIVE_REG0_UNUSED          ((megadrive_vdp_register[0x00]&0xc0)>>6)
#define MEGADRIVE_REG0_BLANK_LEFT      ((megadrive_vdp_register[0x00]&0x20)>>5) // like SMS, not used by any commercial games?
#define MEGADRIVE_REG0_IRQ4_ENABLE     ((megadrive_vdp_register[0x00]&0x10)>>4)
#define MEGADRIVE_REG0_INVALID_MODE    ((megadrive_vdp_register[0x00]&0x08)>>3) // invalid display mode, unhandled
#define MEGADRIVE_REG0_SPECIAL_PAL     ((megadrive_vdp_register[0x00]&0x04)>>2) // strange palette mode, unhandled
#define MEGADRIVE_REG0_HVLATCH_ENABLE  ((megadrive_vdp_register[0x00]&0x02)>>1) // HV Latch, used by lightgun games
#define MEGADRIVE_REG0_DISPLAY_DISABLE ((megadrive_vdp_register[0x00]&0x01)>>0)

/*

 $01 - Mode Set Register No. 2
 -----------------------------

 d7 - TMS9918 / Genesis display select
 d6 - DISP (Display Enable)
 d5 - IE0 (Vertical Interrupt Enable)
 d4 - M1 (DMA Enable)
 d3 - M2 (PAL / NTSC)
 d2 - SMS / Genesis display select
 d1 - 0 (No effect)
 d0 - 0 (See notes)

*/

#define MEGADRIVE_REG01_TMS9918_SELECT  ((megadrive_vdp_register[0x01]&0x80)>>7)
#define MEGADRIVE_REG01_DISP_ENABLE     ((megadrive_vdp_register[0x01]&0x40)>>6)
#define MEGADRIVE_REG01_IRQ6_ENABLE     ((megadrive_vdp_register[0x01]&0x20)>>5)
#define MEGADRIVE_REG01_DMA_ENABLE      ((megadrive_vdp_register[0x01]&0x10)>>4)
#define MEGADRIVE_REG01_240_LINE        ((megadrive_vdp_register[0x01]&0x08)>>3)
#define MEGADRIVE_REG01_SMS_SELECT      ((megadrive_vdp_register[0x01]&0x04)>>2)
#define MEGADRIVE_REG01_UNUSED          ((megadrive_vdp_register[0x01]&0x02)>>1)
#define MEGADRIVE_REG01_STRANGE_VIDEO   ((megadrive_vdp_register[0x01]&0x01)>>0) // unhandled, does strange things to the display

#define MEGADRIVE_REG02_UNUSED1         ((megadrive_vdp_register[0x02]&0xc0)>>6)
#define MEGADRIVE_REG02_PATTERN_ADDR_A  ((megadrive_vdp_register[0x02]&0x38)>>3)
#define MEGADRIVE_REG02_UNUSED2         ((megadrive_vdp_register[0x02]&0x07)>>0)

#define MEGADRIVE_REG03_UNUSED1         ((megadrive_vdp_register[0x03]&0xc0)>>6)
#define MEGADRIVE_REG03_PATTERN_ADDR_W  ((megadrive_vdp_register[0x03]&0x3e)>>1)
#define MEGADRIVE_REG03_UNUSED2         ((megadrive_vdp_register[0x03]&0x01)>>0)

#define MEGADRIVE_REG04_UNUSED          ((megadrive_vdp_register[0x04]&0xf8)>>3)
#define MEGADRIVE_REG04_PATTERN_ADDR_B  ((megadrive_vdp_register[0x04]&0x07)>>0)

#define MEGADRIVE_REG05_UNUSED          ((megadrive_vdp_register[0x05]&0x80)>>7)
#define MEGADRIVE_REG05_SPRITE_ADDR     ((megadrive_vdp_register[0x05]&0x7f)>>0)

/* 6? */

#define MEGADRIVE_REG07_UNUSED          ((megadrive_vdp_register[0x07]&0xc0)>>6)
#define MEGADRIVE_REG07_BGCOLOUR        ((megadrive_vdp_register[0x07]&0x3f)>>0)

/* 8? */
/* 9? */

#define MEGADRIVE_REG0A_HINT_VALUE      ((megadrive_vdp_register[0x0a]&0xff)>>0)

#define MEGADRIVE_REG0B_UNUSED          ((megadrive_vdp_register[0x0b]&0xf0)>>4)
#define MEGADRIVE_REG0B_IRQ2_ENABLE     ((megadrive_vdp_register[0x0b]&0x08)>>3)
#define MEGADRIVE_REG0B_VSCROLL_MODE    ((megadrive_vdp_register[0x0b]&0x04)>>2)
#define MEGADRIVE_REG0B_HSCROLL_MODE    ((megadrive_vdp_register[0x0b]&0x03)>>0)

#define MEGADRIVE_REG0C_RS0             ((megadrive_vdp_register[0x0c]&0x80)>>7)
#define MEGADRIVE_REG0C_UNUSED1         ((megadrive_vdp_register[0x0c]&0x40)>>6)
#define MEGADRIVE_REG0C_SPECIAL         ((megadrive_vdp_register[0x0c]&0x20)>>5)
#define MEGADRIVE_REG0C_UNUSED2         ((megadrive_vdp_register[0x0c]&0x10)>>4)
#define MEGADRIVE_REG0C_SHADOW_HIGLIGHT ((megadrive_vdp_register[0x0c]&0x08)>>3)
#define MEGADRIVE_REG0C_INTERLEAVE      ((megadrive_vdp_register[0x0c]&0x06)>>1)
#define MEGADRIVE_REG0C_RS1             ((megadrive_vdp_register[0x0c]&0x01)>>0)

#define MEGADRIVE_REG0D_UNUSED          ((megadrive_vdp_register[0x0d]&0xc0)>>6)
#define MEGADRIVE_REG0D_HSCROLL_ADDR    ((megadrive_vdp_register[0x0d]&0x3f)>>0)

/* e? */

#define MEGADRIVE_REG0F_AUTO_INC        ((megadrive_vdp_register[0x0f]&0xff)>>0)

#define MEGADRIVE_REG10_UNUSED1        ((megadrive_vdp_register[0x10]&0xc0)>>6)
#define MEGADRIVE_REG10_VSCROLL_SIZE   ((megadrive_vdp_register[0x10]&0x30)>>4)
#define MEGADRIVE_REG10_UNUSED2        ((megadrive_vdp_register[0x10]&0x0c)>>2)
#define MEGADRIVE_REG10_HSCROLL_SIZE   ((megadrive_vdp_register[0x10]&0x03)>>0)

#define MEGADRIVE_REG11_WINDOW_RIGHT   ((megadrive_vdp_register[0x11]&0x80)>>7)
#define MEGADRIVE_REG11_UNUSED         ((megadrive_vdp_register[0x11]&0x60)>>5)
#define MEGADRIVE_REG11_WINDOW_HPOS      ((megadrive_vdp_register[0x11]&0x1f)>>0)

#define MEGADRIVE_REG12_WINDOW_DOWN    ((megadrive_vdp_register[0x12]&0x80)>>7)
#define MEGADRIVE_REG12_UNUSED         ((megadrive_vdp_register[0x12]&0x60)>>5)
#define MEGADRIVE_REG12_WINDOW_VPOS      ((megadrive_vdp_register[0x12]&0x1f)>>0)

#define MEGADRIVE_REG13_DMALENGTH1     ((megadrive_vdp_register[0x13]&0xff)>>0)

#define MEGADRIVE_REG14_DMALENGTH2      ((megadrive_vdp_register[0x14]&0xff)>>0)

#define MEGADRIVE_REG15_DMASOURCE1      ((megadrive_vdp_register[0x15]&0xff)>>0)
#define MEGADRIVE_REG16_DMASOURCE2      ((megadrive_vdp_register[0x16]&0xff)>>0)

#define MEGADRIVE_REG17_DMASOURCE3      ((megadrive_vdp_register[0x17]&0xff)>>0)
#define MEGADRIVE_REG17_DMATYPE         ((megadrive_vdp_register[0x17]&0xc0)>>6)
#define MEGADRIVE_REG17_UNUSED          ((megadrive_vdp_register[0x17]&0x3f)>>0)


static void vdp_vram_write(UINT16 data)
{

	UINT16 sprite_base_address = MEGADRIVE_REG0C_RS1?((MEGADRIVE_REG05_SPRITE_ADDR&0x7e)<<9):((MEGADRIVE_REG05_SPRITE_ADDR&0x7f)<<9);
	int spritetable_size = MEGADRIVE_REG0C_RS1?0x400:0x200;
	int lowlimit = sprite_base_address;
	int highlimit = sprite_base_address+spritetable_size;

	if (megadrive_vdp_address&1)
	{
		data = ((data&0x00ff)<<8)|((data&0xff00)>>8);
	}

	MEGADRIV_VDP_VRAM(megadrive_vdp_address>>1) = data;

	/* The VDP stores an Internal copy of any data written to the Sprite Attribute Table.
       This data is _NOT_ invalidated when the Sprite Base Address changes, thus allowing
       for some funky effects, as used by Castlevania Bloodlines Stage 6-3 */
	if (megadrive_vdp_address>=lowlimit && megadrive_vdp_address<highlimit)
	{
//      mame_printf_debug("spritebase is %04x-%04x vram address is %04x, write %04x\n",lowlimit, highlimit-1, megadrive_vdp_address, data);
		megadrive_vdp_internal_sprite_attribute_table[(megadrive_vdp_address&(spritetable_size-1))>>1] = data;
	}

	megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
	megadrive_vdp_address &= 0xffff;
}

static void vdp_vsram_write(UINT16 data)
{
	megadrive_vdp_vsram[(megadrive_vdp_address&0x7e)>>1] = data;

	//logerror("Wrote to VSRAM addr %04x data %04x\n",megadrive_vdp_address&0xfffe,megadrive_vdp_vsram[megadrive_vdp_address>>1]);

	megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;

	megadrive_vdp_address &=0xffff;
}

static void write_cram_value(running_machine &machine, int offset, int data)
{
	megadrive_vdp_cram[offset] = data;

	//logerror("Wrote to CRAM addr %04x data %04x\n",megadrive_vdp_address&0xfffe,megadrive_vdp_cram[megadrive_vdp_address>>1]);
	if (genvdp_use_cram)
	{
		int r,g,b;
		r = ((data >> 1)&0x07);
		g = ((data >> 5)&0x07);
		b = ((data >> 9)&0x07);
		palette_set_color_rgb(machine,offset,pal3bit(r),pal3bit(g),pal3bit(b));
		megadrive_vdp_palette_lookup[offset] = (b<<2) | (g<<7) | (r<<12);
		megadrive_vdp_palette_lookup_sprite[offset] = (b<<2) | (g<<7) | (r<<12);
		megadrive_vdp_palette_lookup_shadow[offset] = (b<<1) | (g<<6) | (r<<11);
		megadrive_vdp_palette_lookup_highlight[offset] = ((b|0x08)<<1) | ((g|0x08)<<6) | ((r|0x08)<<11);
	}
}

static void vdp_cram_write(running_machine &machine, UINT16 data)
{
	int offset;
	offset = (megadrive_vdp_address&0x7e)>>1;

	write_cram_value(machine, offset,data);

	megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;

	megadrive_vdp_address &=0xffff;
}


static void megadriv_vdp_data_port_w(running_machine &machine, int data)
{
	megadrive_vdp_command_pending = 0;

 /*
 0000b : VRAM read
 0001b : VRAM write
 0011b : CRAM write
 0100b : VSRAM read
 0101b : VSRAM write
 1000b : CRAM read
 */
//  logerror("write to vdp data port %04x with code %04x, write address %04x\n",data, megadrive_vdp_code, megadrive_vdp_address );

	if (megadrive_vram_fill_pending)
	{
		int count;

		megadrive_vdp_address&=0xffff;

		if (megadrive_vdp_address&1)
		{
			MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))   = (MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))&0xff00) | (data&0x00ff);
		}
		else
		{
			MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))   = (MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))&0x00ff) | ((data&0x00ff)<<8);
		}


		for (count=0;count<=megadrive_vram_fill_length;count++) // <= for james pond 3
		{
			if (megadrive_vdp_address&1)
			{
				MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))   = (MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))&0x00ff) | (data&0xff00);
			}
			else
			{
				MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))   = (MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))&0xff00) | ((data&0xff00)>>8);
			}

			megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
			megadrive_vdp_address&=0xffff;

		}

		megadrive_vdp_register[0x13] = 0;
		megadrive_vdp_register[0x14] = 0;

	//  megadrive_vdp_register[0x15] = (source>>1) & 0xff;
	//  megadrive_vdp_register[0x16] = (source>>9) & 0xff;
	//  megadrive_vdp_register[0x17] = (source>>17) & 0xff;


	}
	else
	{

		switch (megadrive_vdp_code & 0x000f)
		{
			case 0x0000:
				logerror("Attempting to WRITE to DATA PORT in VRAM READ MODE\n");
				break;

			case 0x0001:
				vdp_vram_write(data);
				break;

			case 0x0003:
				vdp_cram_write(machine, data);
				break;

			case 0x0004:
				logerror("Attempting to WRITE to DATA PORT in VSRAM READ MODE\n");
				break;

			case 0x0005:
				vdp_vsram_write(data);
				break;

			case 0x0008:
				logerror("Attempting to WRITE to DATA PORT in CRAM READ MODE\n");
				break;

			default:
				logerror("Attempting to WRITE to DATA PORT in #UNDEFINED# MODE %1x %04x\n",megadrive_vdp_code&0xf, data);
				break;
		}
	}



}



static void megadrive_vdp_set_register(running_machine &machine, int regnum, UINT8 value)
{
	megadrive_vdp_register[regnum] = value;

	/* We need special handling for the IRQ enable registers, some games turn
       off the irqs before they are taken, delaying them until the IRQ is turned
       back on */

	if (regnum == 0x00)
	{
	//mame_printf_debug("setting reg 0, irq enable is now %d\n",MEGADRIVE_REG0_IRQ4_ENABLE);

		if (megadrive_irq4_pending)
		{
			if (MEGADRIVE_REG0_IRQ4_ENABLE)
				cputag_set_input_line(machine, "maincpu", 4, HOLD_LINE);
			else
				cputag_set_input_line(machine, "maincpu", 4, CLEAR_LINE);
		}

		/* ??? Fatal Rewind needs this but I'm not sure it's accurate behavior
           it causes flickering in roadrash */
	//  megadrive_irq6_pending = 0;
	//  megadrive_irq4_pending = 0;

	}

	if (regnum == 0x01)
	{
		if (megadrive_irq6_pending)
		{
			if (MEGADRIVE_REG01_IRQ6_ENABLE )
				cputag_set_input_line(machine, "maincpu", 6, HOLD_LINE);
			else
				cputag_set_input_line(machine, "maincpu", 6, CLEAR_LINE);
		}

		/* ??? */
	//  megadrive_irq6_pending = 0;
	//  megadrive_irq4_pending = 0;

	}


//  if (regnum == 0x0a)
//      mame_printf_debug("Set HINT Reload Register to %d on scanline %d\n",value, genesis_scanline_counter);

//  mame_printf_debug("%s: Setting VDP Register #%02x to %02x\n",machine.describe_context(), regnum,value);
}

static void update_megadrive_vdp_code_and_address(void)
{
	megadrive_vdp_code = ((megadrive_vdp_command_part1 & 0xc000) >> 14) |
	                     ((megadrive_vdp_command_part2 & 0x00f0) >> 2);

	megadrive_vdp_address = ((megadrive_vdp_command_part1 & 0x3fff) >> 0) |
                            ((megadrive_vdp_command_part2 & 0x0003) << 14);
}

static UINT16 (*vdp_get_word_from_68k_mem)(running_machine &machine, UINT32 source);

static UINT16 vdp_get_word_from_68k_mem_default(running_machine &machine, UINT32 source)
{
	// should we limit the valid areas here?
	// how does this behave with the segacd etc?
	// note, the RV bit on 32x is important for this to work, because it causes a normal cart mapping - see tempo
	address_space *space68k = machine.device<legacy_cpu_device>("maincpu")->space();

	//printf("vdp_get_word_from_68k_mem_default %08x\n", source);

	if (( source >= 0x000000 ) && ( source <= 0x3fffff ))
	{
		if (_svp_cpu != NULL)
		{
			source -= 2; // the SVP introduces some kind of DMA 'lag', which we have to compensate for, this is obvious even on gfx DMAd from ROM (the Speedometer)
		}

		// likewise segaCD, at least when reading wordram?
		// we might need to check what mode we're in here..
		if (segacd_wordram_mapped)
		{
			source -= 2;
		}

		return space68k->read_word(source);
	}
	else if (( source >= 0xe00000 ) && ( source <= 0xffffff ))
	{
		return space68k->read_word(source);
	}
	else
	{
		printf("DMA Read unmapped %06x\n",source);
		return machine.rand();
	}


}

/*  Table from Charles Macdonald


    DMA Mode      Width       Display      Transfer Count
    -----------------------------------------------------
    68K > VDP     32-cell     Active       16
                              Blanking     167
                  40-cell     Active       18
                              Blanking     205
    VRAM Fill     32-cell     Active       15
                              Blanking     166
                  40-cell     Active       17
                              Blanking     204
    VRAM Copy     32-cell     Active       8
                              Blanking     83
                  40-cell     Active       9
                              Blanking     102

*/


/* Note, In reality this transfer is NOT instant, the 68k isn't paused
   as the 68k address bus isn't accessed */

/* Wani Wani World, James Pond 3, Pirates Gold! */
static void megadrive_do_insta_vram_copy(UINT32 source, UINT16 length)
{
	int x;

	for (x=0;x<length;x++)
	{
		UINT8 source_byte;

		//mame_printf_debug("vram copy length %04x source %04x dest %04x\n",length, source, megadrive_vdp_address );
		if (source&1) source_byte = MEGADRIV_VDP_VRAM((source&0xffff)>>1)&0x00ff;
		else  source_byte = (MEGADRIV_VDP_VRAM((source&0xffff)>>1)&0xff00)>>8;

		if (megadrive_vdp_address&1)
		{
			MEGADRIV_VDP_VRAM((megadrive_vdp_address&0xffff)>>1) = (MEGADRIV_VDP_VRAM((megadrive_vdp_address&0xffff)>>1)&0xff00) | source_byte;
		}
		else
		{
			MEGADRIV_VDP_VRAM((megadrive_vdp_address&0xffff)>>1) = (MEGADRIV_VDP_VRAM((megadrive_vdp_address&0xffff)>>1)&0x00ff) | (source_byte<<8);
		}

		source++;
		megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
		megadrive_vdp_address&=0xffff;
	}
}

/* Instant, but we pause the 68k a bit */
static void megadrive_do_insta_68k_to_vram_dma(running_machine &machine, UINT32 source,int length)
{
	int count;

	if (length==0x00) length = 0xffff;

	/* This is a hack until real DMA timings are implemented */
	device_spin_until_time(machine.device("maincpu"), attotime::from_nsec(length * 1000 / 3500));

	for (count = 0;count<(length>>1);count++)
	{
		vdp_vram_write(vdp_get_word_from_68k_mem(machine, source));
		source+=2;
		if (source>0xffffff) source = 0xe00000;
	}

	megadrive_vdp_address&=0xffff;

	megadrive_vdp_register[0x13] = 0;
	megadrive_vdp_register[0x14] = 0;

	megadrive_vdp_register[0x15] = (source>>1) & 0xff;
	megadrive_vdp_register[0x16] = (source>>9) & 0xff;
	megadrive_vdp_register[0x17] = (source>>17) & 0xff;
}


static void megadrive_do_insta_68k_to_cram_dma(running_machine &machine,UINT32 source,UINT16 length)
{
	int count;

	if (length==0x00) length = 0xffff;

	for (count = 0;count<(length>>1);count++)
	{
		//if (megadrive_vdp_address>=0x80) return; // abandon

		write_cram_value(machine, (megadrive_vdp_address&0x7e)>>1, vdp_get_word_from_68k_mem(machine, source));
		source+=2;

		if (source>0xffffff) source = 0xfe0000;

		megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
		megadrive_vdp_address&=0xffff;
	}

	megadrive_vdp_register[0x13] = 0;
	megadrive_vdp_register[0x14] = 0;

	megadrive_vdp_register[0x15] = (source>>1) & 0xff;
	megadrive_vdp_register[0x16] = (source>>9) & 0xff;
	megadrive_vdp_register[0x17] = (source>>17) & 0xff;

}

static void megadrive_do_insta_68k_to_vsram_dma(running_machine &machine,UINT32 source,UINT16 length)
{
	int count;

	if (length==0x00) length = 0xffff;

	for (count = 0;count<(length>>1);count++)
	{
		if (megadrive_vdp_address>=0x80) return; // abandon

		megadrive_vdp_vsram[(megadrive_vdp_address&0x7e)>>1] = vdp_get_word_from_68k_mem(machine, source);
		source+=2;

		if (source>0xffffff) source = 0xfe0000;

		megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
		megadrive_vdp_address&=0xffff;
	}

	megadrive_vdp_register[0x13] = 0;
	megadrive_vdp_register[0x14] = 0;

	megadrive_vdp_register[0x15] = (source>>1) & 0xff;
	megadrive_vdp_register[0x16] = (source>>9) & 0xff;
	megadrive_vdp_register[0x17] = (source>>17) & 0xff;
}

/* This can be simplified quite a lot.. */
static void handle_dma_bits(running_machine &machine)
{
#if 0
	if (megadrive_vdp_code&0x20)
	{
		UINT32 source;
		UINT16 length;
		source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8) | ((MEGADRIVE_REG17_DMASOURCE3&0xff)<<16))<<1;
		length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8))<<1;
		mame_printf_debug("%s 68k DMAtran set source %06x length %04x dest %04x enabled %01x code %02x %02x\n", machine.describe_context(), source, length, megadrive_vdp_address,MEGADRIVE_REG01_DMA_ENABLE, megadrive_vdp_code,MEGADRIVE_REG0F_AUTO_INC);
	}
#endif
	if (megadrive_vdp_code==0x20)
	{
		mame_printf_debug("DMA bit set 0x20 but invalid??\n");
	}
	else if (megadrive_vdp_code==0x21 || megadrive_vdp_code==0x31) /* 0x31 used by tecmo cup */
	{
		if (MEGADRIVE_REG17_DMATYPE==0x0 || MEGADRIVE_REG17_DMATYPE==0x1)
		{
			UINT32 source;
			UINT16 length;
			source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8) | ((MEGADRIVE_REG17_DMASOURCE3&0x7f)<<16))<<1;
			length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8))<<1;

			/* The 68k is frozen during this transfer, it should be safe to throw a few cycles away and do 'instant' DMA because the 68k can't detect it being in progress (can the z80?) */
			//mame_printf_debug("68k->VRAM DMA transfer source %06x length %04x dest %04x enabled %01x\n", source, length, megadrive_vdp_address,MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE) megadrive_do_insta_68k_to_vram_dma(machine,source,length);

		}
		else if (MEGADRIVE_REG17_DMATYPE==0x2)
		{
			//mame_printf_debug("vram fill length %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE)
			{
				megadrive_vram_fill_pending = 1;
				megadrive_vram_fill_length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8));
			}
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x3)
		{
			UINT32 source;
			UINT16 length;
			source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8)); // source (byte offset)
			length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8)); // length in bytes
			//mame_printf_debug("setting vram copy mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);

			if (MEGADRIVE_REG01_DMA_ENABLE) megadrive_do_insta_vram_copy(source, length);
		}
	}
	else if (megadrive_vdp_code==0x23)
	{
		if (MEGADRIVE_REG17_DMATYPE==0x0 || MEGADRIVE_REG17_DMATYPE==0x1)
		{
			UINT32 source;
			UINT16 length;
			source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8) | ((MEGADRIVE_REG17_DMASOURCE3&0x7f)<<16))<<1;
			length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8))<<1;

			/* The 68k is frozen during this transfer, it should be safe to throw a few cycles away and do 'instant' DMA because the 68k can't detect it being in progress (can the z80?) */
			//mame_printf_debug("68k->CRAM DMA transfer source %06x length %04x dest %04x enabled %01x\n", source, length, megadrive_vdp_address,MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE) megadrive_do_insta_68k_to_cram_dma(machine,source,length);
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x2)
		{
			//mame_printf_debug("vram fill length %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE)
			{
				megadrive_vram_fill_pending = 1;
				megadrive_vram_fill_length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8));
			}
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x3)
		{
			mame_printf_debug("setting vram copy (INVALID?) mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
		}
	}
	else if (megadrive_vdp_code==0x25)
	{
		if (MEGADRIVE_REG17_DMATYPE==0x0 || MEGADRIVE_REG17_DMATYPE==0x1)
		{
			UINT32 source;
			UINT16 length;
			source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8) | ((MEGADRIVE_REG17_DMASOURCE3&0x7f)<<16))<<1;
			length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8))<<1;

			/* The 68k is frozen during this transfer, it should be safe to throw a few cycles away and do 'instant' DMA because the 68k can't detect it being in progress (can the z80?) */
			//mame_printf_debug("68k->VSRAM DMA transfer source %06x length %04x dest %04x enabled %01x\n", source, length, megadrive_vdp_address,MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE) megadrive_do_insta_68k_to_vsram_dma(machine,source,length);
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x2)
		{
			//mame_printf_debug("vram fill length %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE)
			{
				megadrive_vram_fill_pending = 1;
				megadrive_vram_fill_length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8));
			}
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x3)
		{
			mame_printf_debug("setting vram copy (INVALID?) mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
		}
	}
	else if (megadrive_vdp_code==0x30)
	{
		if (MEGADRIVE_REG17_DMATYPE==0x0)
		{
			mame_printf_debug("setting vram 68k->vram (INVALID?) mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x1)
		{
			mame_printf_debug("setting vram 68k->vram (INVALID?) mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x2)
		{
			mame_printf_debug("setting vram fill (INVALID?) mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x3)
		{
			UINT32 source;
			UINT16 length;
			source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8)); // source (byte offset)
			length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8)); // length in bytes
			//mame_printf_debug("setting vram copy mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);

			if (MEGADRIVE_REG01_DMA_ENABLE) megadrive_do_insta_vram_copy(source, length);
		}
	}
}

static void megadriv_vdp_ctrl_port_w(running_machine &machine, int data)
{
//  logerror("write to vdp control port %04x\n",data);
	megadrive_vram_fill_pending = 0; // ??

	if (megadrive_vdp_command_pending)
	{
		/* 2nd part of 32-bit command */
		megadrive_vdp_command_pending = 0;
		megadrive_vdp_command_part2 = data;

		update_megadrive_vdp_code_and_address();
		handle_dma_bits(machine);

		//logerror("VDP Write Part 2 setting Code %02x Address %04x\n",megadrive_vdp_code, megadrive_vdp_address);

	}
	else
	{
		if ((data & 0xc000) == 0x8000)
		{	/* Register Setting Command */
			int regnum = (data & 0x3f00) >> 8;
			int value  = (data & 0x00ff);

			if (regnum &0x20) mame_printf_debug("reg error\n");

			megadrive_vdp_set_register(machine, regnum&0x1f,value);
			megadrive_vdp_code = 0;
			megadrive_vdp_address = 0;
		}
		else
		{
			megadrive_vdp_command_pending = 1;
			megadrive_vdp_command_part1 = data;
			update_megadrive_vdp_code_and_address();
			//logerror("VDP Write Part 1 setting Code %02x Address %04x\n",megadrive_vdp_code, megadrive_vdp_address);
		}

	}
}

WRITE16_HANDLER( megadriv_vdp_w )
{
	switch (offset<<1)
	{
		case 0x00:
		case 0x02:
			if (!ACCESSING_BITS_8_15)
			{
				data = (data&0x00ff) | data<<8;
			//  mame_printf_debug("8-bit write VDP data port access, offset %04x data %04x mem_mask %04x\n",offset,data,mem_mask);
			}
			else if (!ACCESSING_BITS_0_7)
			{
				data = (data&0xff00) | data>>8;
			//  mame_printf_debug("8-bit write VDP data port access, offset %04x data %04x mem_mask %04x\n",offset,data,mem_mask);
			}
			megadriv_vdp_data_port_w(space->machine(), data);
			break;

		case 0x04:
		case 0x06:
			if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) mame_printf_debug("8-bit write VDP control port access, offset %04x data %04x mem_mask %04x\n",offset,data,mem_mask);
			megadriv_vdp_ctrl_port_w(space->machine(), data);
			break;

		case 0x08:
		case 0x0a:
		case 0x0c:
		case 0x0e:
			logerror("Attempt to Write to HV counters!!\n");
			break;

		case 0x10:
		case 0x12:
		case 0x14:
		case 0x16:
			if (ACCESSING_BITS_0_7) sn76496_w(space->machine().device("snsnd"), 0, data & 0xff);
			//if (ACCESSING_BITS_8_15) sn76496_w(space->machine().device("snsnd"), 0, (data >>8) & 0xff);
			break;

		default:
		mame_printf_debug("write to unmapped vdp port\n");
	}
}

static UINT16 vdp_vram_r(void)
{
	return MEGADRIV_VDP_VRAM((megadrive_vdp_address&0xfffe)>>1);
}

static UINT16 vdp_vsram_r(void)
{
	return megadrive_vdp_vsram[(megadrive_vdp_address&0x7e)>>1];
}

static UINT16 vdp_cram_r(void)
{

	return megadrive_vdp_cram[(megadrive_vdp_address&0x7e)>>1];
}

static UINT16 megadriv_vdp_data_port_r(running_machine &machine)
{
	UINT16 retdata=0;

	//return machine.rand();

	megadrive_vdp_command_pending = 0;

	switch (megadrive_vdp_code & 0x000f)
	{
		case 0x0000:
			retdata = vdp_vram_r();
			megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
			megadrive_vdp_address&=0xffff;
			break;

		case 0x0001:
			logerror("Attempting to READ from DATA PORT in VRAM WRITE MODE\n");
			retdata = machine.rand();
			break;

		case 0x0003:
			logerror("Attempting to READ from DATA PORT in CRAM WRITE MODE\n");
			retdata = machine.rand();
			break;

		case 0x0004:
			retdata = vdp_vsram_r();
			megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
			megadrive_vdp_address&=0xffff;
			break;

		case 0x0005:
			logerror("Attempting to READ from DATA PORT in VSRAM WRITE MODE\n");
			break;

		case 0x0008:
			retdata = vdp_cram_r();
			megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
			megadrive_vdp_address&=0xffff;
			break;

		default:
			logerror("Attempting to READ from DATA PORT in #UNDEFINED# MODE\n");
			retdata = machine.rand();
			break;
	}

//  mame_printf_debug("vdp_data_port_r %04x %04x %04x\n",megadrive_vdp_code, megadrive_vdp_address, retdata);

//  logerror("Read VDP Data Port\n");
	return retdata;
}

/*

 NTSC, 256x224
 -------------

 Lines  Description

 224    Active display
 8      Bottom border
 3      Bottom blanking
 3      Vertical blanking
 13     Top blanking
 11     Top border

 V counter values
 00-EA, E5-FF

PAL, 256x224
 ------------

 Lines  Description

 224    Active display
 32     Bottom border
 3      Bottom blanking
 3      Vertical blanking
 13     Top blanking
 38     Top border

 V counter values
 00-FF, 00-02, CA-FF

 PAL, 256x240
 ------------

 Lines  Description

 240    Active display
 24     Bottom border
 3      Bottom blanking
 3      Vertical blanking
 13     Top blanking
 30     Top border

 V counter values
 00-FF, 00-0A, D2-FF



 Pixels H.Cnt   Description
  256 : 00-7F : Active display
   15 : 80-87 : Right border
    8 : 87-8B : Right blanking
   26 : 8B-ED : Horizontal sync
    2 : ED-EE : Left blanking
   14 : EE-F5 : Color burst
    8 : F5-F9 : Left blanking
   13 : F9-FF : Left border

*/



static UINT16 megadriv_vdp_ctrl_port_r(void)
{
	/* Battletoads is very fussy about the vblank flag
       it wants it to be 1. in scanline 224 */

	/* Double Dragon 2 is very sensitive to hblank timing */
	/* xperts is very fussy too */

	/* Game no Kanzume Otokuyou (J) [!] is also fussy
      - it cares about the bits labeled always 0, always 1.. (!)
     */

	/* Megalo Mania also fussy - cares about pending flag*/

	int megadrive_sprite_overflow = 0;
	int megadrive_odd_frame = megadrive_imode_odd_frame^1;
	int megadrive_hblank_flag = 0;
	int megadrive_dma_active = 0;
	int vblank;
	int fifo_empty = 1;
	int fifo_full = 0;

	UINT16 hpos = get_hposition();

	if (hpos>400) megadrive_hblank_flag = 1;
	if (hpos>460) megadrive_hblank_flag = 0;

	vblank = megadrive_vblank_flag;

	/* extra case */
	if (MEGADRIVE_REG01_DISP_ENABLE==0) vblank = 1;

/*

// these aren't *always* 0/1 some of them are open bus return
 d15 - Always 0
 d14 - Always 0
 d13 - Always 1
 d12 - Always 1

 d11 - Always 0
 d10 - Always 1
 d9  - FIFO Empty
 d8  - FIFO Full

 d7  - Vertical interrupt pending
 d6  - Sprite overflow on current scan line
 d5  - Sprite collision
 d4  - Odd frame

 d3  - Vertical blanking
 d2  - Horizontal blanking
 d1  - DMA in progress
 d0  - PAL mode flag
*/

	return (0<<15) | // ALWAYS 0
	       (0<<14) | // ALWAYS 0
	       (1<<13) | // ALWAYS 1
	       (1<<12) | // ALWAYS 1
	       (0<<11) | // ALWAYS 0
	       (1<<10) | // ALWAYS 1
	       (fifo_empty<<9 ) | // FIFO EMPTY
	       (fifo_full<<8 ) | // FIFO FULL
	       (megadrive_irq6_pending << 7) | // exmutants has a tight loop checking this ..
	       (megadrive_sprite_overflow << 6) |
	       (megadrive_sprite_collision << 5) |
	       (megadrive_odd_frame << 4) |
	       (vblank << 3) |
	       (megadrive_hblank_flag << 2) |
	       (megadrive_dma_active << 1 ) |
	       (megadrive_region_pal<<0); // PAL MODE FLAG checked by striker for region prot..
}

static const UINT8 vc_ntsc_224[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,    0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,    0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,    0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,    0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,    0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,    0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,    0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,    0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,    0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,    0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,    0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,    0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,/**/0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4,    0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static const UINT8 vc_ntsc_240[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};

static const UINT8 vc_pal_224[] =
{
    0x00, 0x01, 0x02,    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12,    0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22,    0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32,    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42,    0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52,    0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62,    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72,    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82,    0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92,    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2,    0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2,    0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2,    0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2,    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2,    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2,    0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0x00, 0x01, 0x02,/**/0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9,    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
    0xe7, 0xe8, 0xe9,    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9,    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static const UINT8 vc_pal_240[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,    0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,    0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,    0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,    0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,    0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,    0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,    0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,    0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,    0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,    0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,    0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,    0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,    0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa,    0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,/**/0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1,    0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
    0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1,    0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};



static UINT16 megadriv_read_hv_counters(void)
{
	/* Bubble and Squeek wants vcount=0xe0 */
	/* Dracula is very sensitive to this */
	/* Marvel Land is sensitive to this */

	int vpos = genesis_scanline_counter;
	UINT16 hpos = get_hposition();

//  if (hpos>424) vpos++; // fixes dracula, breaks road rash
	if (hpos>460) vpos++; // when does vpos increase.. also on sms, check game gear manual..

	/* shouldn't happen.. */
	if (vpos<0)
	{
		vpos = megadrive_total_scanlines;
		mame_printf_debug("negative vpos?!\n");
	}

	if (MEGADRIVE_REG01_240_LINE)
	{
		if (!megadrive_region_pal)
		{
			vpos = vc_ntsc_240[vpos%megadrive_total_scanlines];
		}
		else
		{
			vpos = vc_pal_240[vpos%megadrive_total_scanlines];
		}

	}
	else
	{
		if (!megadrive_region_pal)
		{
			vpos = vc_ntsc_224[vpos%megadrive_total_scanlines];
		}
		else
		{
			vpos = vc_pal_224[vpos%megadrive_total_scanlines];
		}
	}

	if (hpos>0xf7) hpos -=0x49;

	return ((vpos&0xff)<<8)|(hpos&0xff);

}

READ16_HANDLER( megadriv_vdp_r )
{
	UINT16 retvalue = 0;



	switch (offset<<1)
	{

		case 0x00:
		case 0x02:
			if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) mame_printf_debug("8-bit VDP read data port access, offset %04x mem_mask %04x\n",offset,mem_mask);
			retvalue = megadriv_vdp_data_port_r(space->machine());
			break;

		case 0x04:
		case 0x06:
		//  if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) mame_printf_debug("8-bit VDP read control port access, offset %04x mem_mask %04x\n",offset,mem_mask);
			retvalue = megadriv_vdp_ctrl_port_r();
		//  retvalue = space->machine().rand();
		//  mame_printf_debug("%06x: Read Control Port at scanline %d hpos %d (return %04x)\n",cpu_get_pc(&space->device()),genesis_scanline_counter, get_hposition(),retvalue);
			break;

		case 0x08:
		case 0x0a:
		case 0x0c:
		case 0x0e:
		//  if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) mame_printf_debug("8-bit VDP read HV counter port access, offset %04x mem_mask %04x\n",offset,mem_mask);
			retvalue = megadriv_read_hv_counters();
		//  retvalue = space->machine().rand();
		//  mame_printf_debug("%06x: Read HV counters at scanline %d hpos %d (return %04x)\n",cpu_get_pc(&space->device()),genesis_scanline_counter, get_hposition(),retvalue);
			break;

		case 0x10:
		case 0x12:
		case 0x14:
		case 0x16:
			logerror("Attempting to read PSG!\n");
			retvalue = 0;
			break;
	}
	return retvalue;
}

READ8_DEVICE_HANDLER( megadriv_68k_YM2612_read)
{
	//mame_printf_debug("megadriv_68k_YM2612_read %02x %04x\n",offset,mem_mask);
	if ( (genz80.z80_has_bus==0) && (genz80.z80_is_reset==0) )
	{
		return ym2612_r(device, offset);
	}
	else
	{
		logerror("%s: 68000 attempting to access YM2612 (read) without bus\n", device->machine().describe_context());
		return 0;
	}

	return -1;
}



WRITE8_DEVICE_HANDLER( megadriv_68k_YM2612_write)
{
	//mame_printf_debug("megadriv_68k_YM2612_write %02x %04x %04x\n",offset,data,mem_mask);
	if ( (genz80.z80_has_bus==0) && (genz80.z80_is_reset==0) )
	{
		ym2612_w(device, offset, data);
	}
	else
	{
		logerror("%s: 68000 attempting to access YM2612 (write) without bus\n", device->machine().describe_context());
	}
}

/* Megadrive / Genesis has 3 I/O ports */
static emu_timer *io_timeout[3];
static int io_stage[3];

static TIMER_CALLBACK( io_timeout_timer_callback )
{
	io_stage[(int)(FPTR)ptr] = -1;
}

static void init_megadri6_io(running_machine &machine)
{
	int i;

	for (i=0; i<3; i++)
	{
		io_timeout[i] = machine.scheduler().timer_alloc(FUNC(io_timeout_timer_callback), (void*)(FPTR)i);
	}
}

/* pointers to our io data read/write functions */
UINT8 (*megadrive_io_read_data_port_ptr)(running_machine &machine, int offset);
void (*megadrive_io_write_data_port_ptr)(running_machine &machine, int offset, UINT16 data);

/*

    A10001h = A0         Version register

    A10003h = 7F         Data register for port A
    A10005h = 7F         Data register for port B
    A10007h = 7F         Data register for port C

    A10009h = 00         Ctrl register for port A
    A1000Bh = 00         Ctrl register for port B
    A1000Dh = 00         Ctrl register for port C

    A1000Fh = FF         TxData register for port A
    A10011h = 00         RxData register for port A
    A10013h = 00         S-Ctrl register for port A

    A10015h = FF         TxData register for port B
    A10017h = 00         RxData register for port B
    A10019h = 00         S-Ctrl register for port B

    A1001Bh = FF         TxData register for port C
    A1001Dh = 00         RxData register for port C
    A1001Fh = 00         S-Ctrl register for port C




 Bit 7 - (Not connected)
 Bit 6 - TH
 Bit 5 - TL
 Bit 4 - TR
 Bit 3 - RIGHT
 Bit 2 - LEFT
 Bit 1 - DOWN
 Bit 0 - UP


*/

INPUT_PORTS_START( md_common )
	PORT_START("PAD1")		/* Joypad 1 (3 button + start) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 B") // b
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 C") // c
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 A") // a
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 START") // start

	PORT_START("PAD2")		/* Joypad 2 (3 button + start) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B") // b
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 C") // c
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A") // a
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 START") // start
INPUT_PORTS_END


INPUT_PORTS_START( megadriv )
	PORT_INCLUDE( md_common )

	PORT_START("RESET")		/* Buttons on Genesis Console */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Reset Button") PORT_IMPULSE(1) // reset, resets 68k (and..?)

	#if HAZE_MD
	PORT_START("REGION")	/* Region setting for Console */
	/* Region setting for Console */
	PORT_DIPNAME( 0x000f, 0x0000, DEF_STR( Region ) )
	PORT_DIPSETTING(      0x0000, "Use HazeMD Default Choice" )
	PORT_DIPSETTING(      0x0001, "US (NTSC, 60fps)" )
	PORT_DIPSETTING(      0x0002, "JAPAN (NTSC, 60fps)" )
	PORT_DIPSETTING(      0x0003, "EUROPE (PAL, 50fps)" )
	#endif
INPUT_PORTS_END

INPUT_PORTS_START( megadri6 )
	PORT_INCLUDE( megadriv )

	PORT_START("EXTRA1")	/* Extra buttons for Joypad 1 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("P1 Z") // z
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Y") // y
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 X") // x
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME("P1 MODE") // mode

	PORT_START("EXTRA2")	/* Extra buttons for Joypad 2 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("P2 Z") // z
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Y") // y
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 X") // x
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(2) PORT_NAME("P2 MODE") // mode
INPUT_PORTS_END

UINT8 megadrive_io_data_regs[3];
UINT8 megadrive_io_ctrl_regs[3];
static UINT8 megadrive_io_tx_regs[3];
int megadrive_6buttons_pad = 0;

static void megadrive_reset_io(running_machine &machine)
{
	int i;

	megadrive_io_data_regs[0] = 0x7f;
	megadrive_io_data_regs[1] = 0x7f;
	megadrive_io_data_regs[2] = 0x7f;
	megadrive_io_ctrl_regs[0] = 0x00;
	megadrive_io_ctrl_regs[1] = 0x00;
	megadrive_io_ctrl_regs[2] = 0x00;
	megadrive_io_tx_regs[0] = 0xff;
	megadrive_io_tx_regs[1] = 0xff;
	megadrive_io_tx_regs[2] = 0xff;

	for (i=0; i<3; i++)
	{
		io_stage[i] = -1;
	}
}

/************* 6 buttons version **************************/
static UINT8 megadrive_io_read_data_port_6button(running_machine &machine, int portnum)
{
	UINT8 retdata, helper = (megadrive_io_ctrl_regs[portnum] & 0x3f) | 0xc0; // bits 6 & 7 always come from megadrive_io_data_regs
	static const char *const pad3names[] = { "PAD1", "PAD2", "IN0", "UNK" };
	static const char *const pad6names[] = { "EXTRA1", "EXTRA2", "IN0", "UNK" };

	if (megadrive_io_data_regs[portnum] & 0x40)
	{
		if (io_stage[portnum] == 2)
		{
			/* here we read B, C & the additional buttons */
			retdata = (megadrive_io_data_regs[portnum] & helper) |
						(((input_port_read_safe(machine, pad3names[portnum], 0) & 0x30) |
							(input_port_read_safe(machine, pad6names[portnum], 0) & 0x0f)) & ~helper);
		}
		else
		{
			/* here we read B, C & the directional buttons */
			retdata = (megadrive_io_data_regs[portnum] & helper) |
						((input_port_read_safe(machine, pad3names[portnum], 0) & 0x3f) & ~helper);
		}
	}
	else
	{
		if (io_stage[portnum] == 1)
		{
			/* here we read ((Start & A) >> 2) | 0x00 */
			retdata = (megadrive_io_data_regs[portnum] & helper) |
						(((input_port_read_safe(machine, pad3names[portnum], 0) & 0xc0) >> 2) & ~helper);
		}
		else if (io_stage[portnum]==2)
		{
			/* here we read ((Start & A) >> 2) | 0x0f */
			retdata = (megadrive_io_data_regs[portnum] & helper) |
						((((input_port_read_safe(machine, pad3names[portnum], 0) & 0xc0) >> 2) | 0x0f) & ~helper);
		}
		else
		{
			/* here we read ((Start & A) >> 2) | Up and Down */
			retdata = (megadrive_io_data_regs[portnum] & helper) |
						((((input_port_read_safe(machine, pad3names[portnum], 0) & 0xc0) >> 2) |
							(input_port_read_safe(machine, pad3names[portnum], 0) & 0x03)) & ~helper);
		}
	}

//  mame_printf_debug("read io data port stage %d port %d %02x\n",io_stage[portnum],portnum,retdata);

	return retdata | (retdata << 8);
}


/************* 3 buttons version **************************/
static UINT8 megadrive_io_read_data_port_3button(running_machine &machine, int portnum)
{
	UINT8 retdata, helper = (megadrive_io_ctrl_regs[portnum] & 0x7f) | 0x80; // bit 7 always comes from megadrive_io_data_regs
	static const char *const pad3names[] = { "PAD1", "PAD2", "IN0", "UNK" };

	if (megadrive_io_data_regs[portnum] & 0x40)
	{
		/* here we read B, C & the directional buttons */
		retdata = (megadrive_io_data_regs[portnum] & helper) |
					(((input_port_read_safe(machine, pad3names[portnum], 0) & 0x3f) | 0x40) & ~helper);
	}
	else
	{
		/* here we read ((Start & A) >> 2) | Up and Down */
		retdata = (megadrive_io_data_regs[portnum] & helper) |
					((((input_port_read_safe(machine, pad3names[portnum], 0) & 0xc0) >> 2) |
						(input_port_read_safe(machine, pad3names[portnum], 0) & 0x03) | 0x40) & ~helper);
	}

	return retdata;
}

/* used by megatech bios, the test mode accesses the joypad/stick inputs like this */
UINT8 megatech_bios_port_cc_dc_r(running_machine &machine, int offset, int ctrl)
{
	UINT8 retdata;

	if (ctrl == 0x55)
	{
			/* A keys */
			retdata = ((input_port_read(machine, "PAD1") & 0x40) >> 2) |
				((input_port_read(machine, "PAD2") & 0x40) >> 4) | 0xeb;
	}
	else
	{
		if (offset == 0)
		{
			retdata = (input_port_read(machine, "PAD1") & 0x3f) | ((input_port_read(machine, "PAD2") & 0x03) << 6);
		}
		else
		{
			retdata = ((input_port_read(machine, "PAD2") & 0x3c) >> 2) | 0xf0;
		}

	}

	return retdata;
}

static UINT8 megadrive_io_read_ctrl_port(int portnum)
{
	UINT8 retdata;
	retdata = megadrive_io_ctrl_regs[portnum];
	//mame_printf_debug("read io ctrl port %d %02x\n",portnum,retdata);

	return retdata | (retdata << 8);
}

static UINT8 megadrive_io_read_tx_port(int portnum)
{
	UINT8 retdata;
	retdata = megadrive_io_tx_regs[portnum];
	return retdata | (retdata << 8);
}

static UINT8 megadrive_io_read_rx_port(int portnum)
{
	return 0x00;
}

static UINT8 megadrive_io_read_sctrl_port(int portnum)
{
	return 0x00;
}


READ16_HANDLER( megadriv_68k_io_read )
{
	UINT8 retdata;

	retdata = 0;
      /* Charles MacDonald ( http://cgfm2.emuviews.com/ )
          D7 : Console is 1= Export (USA, Europe, etc.) 0= Domestic (Japan)
          D6 : Video type is 1= PAL, 0= NTSC
          D5 : Sega CD unit is 1= not present, 0= connected.
          D4 : Unused (always returns zero)
          D3 : Bit 3 of version number
          D2 : Bit 2 of version number
          D1 : Bit 1 of version number
          D0 : Bit 0 of version number
      */

	//return (space->machine().rand()&0x0f0f)|0xf0f0;//0x0000;
	switch (offset)
	{
		case 0:
			logerror("%06x read version register\n", cpu_get_pc(&space->device()));
			retdata = megadrive_region_export<<7 | // Export
			          megadrive_region_pal<<6 | // NTSC
			          (sega_cd_connected?0x00:0x20) | // 0x20 = no sega cd
			          0x00 | // Unused (Always 0)
			          0x00 | // Bit 3 of Version Number
			          0x00 | // Bit 2 of Version Number
			          0x00 | // Bit 1 of Version Number
			          0x01 ; // Bit 0 of Version Number
			break;

		/* Joystick Port Registers */

		case 0x1:
		case 0x2:
		case 0x3:
//          retdata = megadrive_io_read_data_port(offset-1);
			retdata = megadrive_io_read_data_port_ptr(space->machine(), offset-1);
			break;

		case 0x4:
		case 0x5:
		case 0x6:
			retdata = megadrive_io_read_ctrl_port(offset-4);
			break;

		/* Serial I/O Registers */

		case 0x7: retdata = megadrive_io_read_tx_port(0); break;
		case 0x8: retdata = megadrive_io_read_rx_port(0); break;
		case 0x9: retdata = megadrive_io_read_sctrl_port(0); break;

		case 0xa: retdata = megadrive_io_read_tx_port(1); break;
		case 0xb: retdata = megadrive_io_read_rx_port(1); break;
		case 0xc: retdata = megadrive_io_read_sctrl_port(1); break;

		case 0xd: retdata = megadrive_io_read_tx_port(2); break;
		case 0xe: retdata = megadrive_io_read_rx_port(2); break;
		case 0xf: retdata = megadrive_io_read_sctrl_port(2); break;

	}

	return retdata | (retdata << 8);
}


static void megadrive_io_write_data_port_3button(running_machine &machine, int portnum, UINT16 data)
{
	megadrive_io_data_regs[portnum] = data;
	//mame_printf_debug("Writing IO Data Register #%d data %04x\n",portnum,data);

}


/****************************** 6 buttons version*****************************/

static void megadrive_io_write_data_port_6button(running_machine &machine, int portnum, UINT16 data)
{
	if (megadrive_io_ctrl_regs[portnum]&0x40)
	{
		if (((megadrive_io_data_regs[portnum]&0x40)==0x00) && ((data&0x40) == 0x40))
		{
			io_stage[portnum]++;
			io_timeout[portnum]->adjust(machine.device<cpu_device>("maincpu")->cycles_to_attotime(8192));
		}

	}

	megadrive_io_data_regs[portnum] = data;
	//mame_printf_debug("Writing IO Data Register #%d data %04x\n",portnum,data);

}


/*************************** 3 buttons version ****************************/

static void megadrive_io_write_ctrl_port(running_machine &machine, int portnum, UINT16 data)
{
	megadrive_io_ctrl_regs[portnum] = data;
//  mame_printf_debug("Setting IO Control Register #%d data %04x\n",portnum,data);
}

static void megadrive_io_write_tx_port(running_machine &machine, int portnum, UINT16 data)
{
	megadrive_io_tx_regs[portnum] = data;
}

static void megadrive_io_write_rx_port(running_machine &machine, int portnum, UINT16 data)
{

}

static void megadrive_io_write_sctrl_port(running_machine &machine, int portnum, UINT16 data)
{

}


WRITE16_HANDLER( megadriv_68k_io_write )
{
//  mame_printf_debug("IO Write #%02x data %04x mem_mask %04x\n",offset,data,mem_mask);


	switch (offset)
	{
		case 0x0:
			mame_printf_debug("Write to Version Register?!\n");
			break;

		/* Joypad Port Registers */

		case 0x1:
		case 0x2:
		case 0x3:
//          megadrive_io_write_data_port(offset-1,data);
			megadrive_io_write_data_port_ptr(space->machine(), offset-1,data);
			break;

		case 0x4:
		case 0x5:
		case 0x6:
			megadrive_io_write_ctrl_port(space->machine(),offset-4,data);
			break;

		/* Serial I/O Registers */

		case 0x7: megadrive_io_write_tx_port(space->machine(),0,data); break;
		case 0x8: megadrive_io_write_rx_port(space->machine(),0,data); break;
		case 0x9: megadrive_io_write_sctrl_port(space->machine(),0,data); break;


		case 0xa: megadrive_io_write_tx_port(space->machine(),1,data); break;
		case 0xb: megadrive_io_write_rx_port(space->machine(),1,data); break;
		case 0xc: megadrive_io_write_sctrl_port(space->machine(),1,data); break;
			break;

		case 0xd: megadrive_io_write_tx_port(space->machine(),2,data); break;
		case 0xe: megadrive_io_write_rx_port(space->machine(),2,data); break;
		case 0xf: megadrive_io_write_sctrl_port(space->machine(),2,data); break;
			break;
	}

}



static ADDRESS_MAP_START( megadriv_map, AS_PROGRAM, 16, driver_device )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	/*      (0x000000 - 0x3fffff) == GAME ROM (4Meg Max, Some games have special banking too) */

	AM_RANGE(0xa00000, 0xa01fff) AM_READWRITE_LEGACY(megadriv_68k_read_z80_ram,megadriv_68k_write_z80_ram)
	AM_RANGE(0xa02000, 0xa03fff) AM_WRITE_LEGACY(megadriv_68k_write_z80_ram)
	AM_RANGE(0xa04000, 0xa04003) AM_DEVREADWRITE8_LEGACY("ymsnd", megadriv_68k_YM2612_read,megadriv_68k_YM2612_write, 0xffff)

	AM_RANGE(0xa06000, 0xa06001) AM_WRITE_LEGACY(megadriv_68k_z80_bank_write)

	AM_RANGE(0xa10000, 0xa1001f) AM_READWRITE_LEGACY(megadriv_68k_io_read,megadriv_68k_io_write)

	AM_RANGE(0xa11100, 0xa11101) AM_READWRITE_LEGACY(megadriv_68k_check_z80_bus,megadriv_68k_req_z80_bus)
	AM_RANGE(0xa11200, 0xa11201) AM_WRITE_LEGACY(megadriv_68k_req_z80_reset)

	/* these are fake - remove allocs in VIDEO_START to use these to view ram instead */
//  AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_BASE_LEGACY(&megadrive_vdp_vram)
//  AM_RANGE(0xb10000, 0xb1007f) AM_RAM AM_BASE_LEGACY(&megadrive_vdp_vsram)
//  AM_RANGE(0xb10100, 0xb1017f) AM_RAM AM_BASE_LEGACY(&megadrive_vdp_cram)

	AM_RANGE(0xc00000, 0xc0001f) AM_READWRITE_LEGACY(megadriv_vdp_r,megadriv_vdp_w)
	AM_RANGE(0xd00000, 0xd0001f) AM_READWRITE_LEGACY(megadriv_vdp_r,megadriv_vdp_w) // the earth defend

	AM_RANGE(0xe00000, 0xe0ffff) AM_RAM AM_MIRROR(0x1f0000) AM_BASE_LEGACY(&megadrive_ram)
//  AM_RANGE(0xff0000, 0xffffff) AM_READONLY
	/*       0xe00000 - 0xffffff) == MAIN RAM (64kb, Mirrored, most games use ff0000 - ffffff) */
ADDRESS_MAP_END


/* z80 sounds/sub CPU */


static READ16_HANDLER( megadriv_68k_read_z80_ram )
{
	//mame_printf_debug("read z80 ram %04x\n",mem_mask);

	if ( (genz80.z80_has_bus==0) && (genz80.z80_is_reset==0) )
	{
		return genz80.z80_prgram[(offset<<1)^1] | (genz80.z80_prgram[(offset<<1)]<<8);
	}
	else
	{
		logerror("%06x: 68000 attempting to access Z80 (read) address space without bus\n", cpu_get_pc(&space->device()));
		return space->machine().rand();
	}
}

static WRITE16_HANDLER( megadriv_68k_write_z80_ram )
{
	//logerror("write z80 ram\n");

	if ((genz80.z80_has_bus==0) && (genz80.z80_is_reset==0))
	{

		if (!ACCESSING_BITS_0_7) // byte (MSB) access
		{
			genz80.z80_prgram[(offset<<1)] = (data & 0xff00) >> 8;
		}
		else if (!ACCESSING_BITS_8_15)
		{
			genz80.z80_prgram[(offset<<1)^1] = (data & 0x00ff);
		}
		else // for WORD access only the MSB is used, LSB is ignored
		{
			genz80.z80_prgram[(offset<<1)] = (data & 0xff00) >> 8;
		}
	}
	else
	{
		logerror("%06x: 68000 attempting to access Z80 (write) address space without bus\n", cpu_get_pc(&space->device()));
	}
}


static READ16_HANDLER( megadriv_68k_check_z80_bus )
{
	UINT16 retvalue;

	/* Double Dragon, Shadow of the Beast, Super Off Road, and Time Killers have buggy
       sound programs.  They request the bus, then have a loop which waits for the bus
       to be unavailable, checking for a 0 value due to bad coding.  The real hardware
       appears to return bits of the next instruction in the unused bits, thus meaning
       the value is never zero.  Time Killers is the most fussy, and doesn't like the
       read_next_instruction function from system16, so I just return a random value
       in the unused bits */
	UINT16 nextvalue = space->machine().rand();//read_next_instruction(space)&0xff00;


	/* Check if the 68k has the z80 bus */
	if (!ACCESSING_BITS_0_7) // byte (MSB) access
	{
		if (genz80.z80_has_bus || genz80.z80_is_reset) retvalue = nextvalue | 0x0100;
		else retvalue = (nextvalue & 0xfeff);

		//logerror("%06x: 68000 check z80 Bus (byte MSB access) returning %04x mask %04x\n", cpu_get_pc(&space->device()),retvalue, mem_mask);
		return retvalue;

	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		//logerror("%06x: 68000 check z80 Bus (byte LSB access) %04x\n", cpu_get_pc(&space->device()),mem_mask);
		if (genz80.z80_has_bus || genz80.z80_is_reset) retvalue = 0x0001;
		else retvalue = 0x0000;

		return retvalue;
	}
	else
	{
		//logerror("%06x: 68000 check z80 Bus (word access) %04x\n", cpu_get_pc(&space->device()),mem_mask);
		if (genz80.z80_has_bus || genz80.z80_is_reset) retvalue = nextvalue | 0x0100;
		else retvalue = (nextvalue & 0xfeff);

	//  mame_printf_debug("%06x: 68000 check z80 Bus (word access) %04x %04x\n", cpu_get_pc(&space->device()),mem_mask, retvalue);
		return retvalue;
	}
}


static TIMER_CALLBACK( megadriv_z80_run_state )
{
	/* Is the z80 RESET line pulled? */
	if ( genz80.z80_is_reset )
	{
		devtag_reset( machine, "genesis_snd_z80" );
		machine.device<cpu_device>( "genesis_snd_z80" )->suspend(SUSPEND_REASON_HALT, 1 );
		devtag_reset( machine, "ymsnd" );
	}
	else
	{
		/* Check if z80 has the bus */
		if ( genz80.z80_has_bus )
		{
			machine.device<cpu_device>( "genesis_snd_z80" )->resume(SUSPEND_REASON_HALT );
		}
		else
		{
			machine.device<cpu_device>( "genesis_snd_z80" )->suspend(SUSPEND_REASON_HALT, 1 );
		}
	}
}


static WRITE16_HANDLER( megadriv_68k_req_z80_bus )
{
	/* Request the Z80 bus, allows 68k to read/write Z80 address space */
	if (!ACCESSING_BITS_0_7) // byte access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 request z80 Bus (byte MSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_has_bus = 0;
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte MSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_has_bus = 1;
		}
	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		if (data & 0x0001)
		{
			//logerror("%06x: 68000 request z80 Bus (byte LSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_has_bus = 0;
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte LSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_has_bus = 1;
		}
	}
	else // word access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 request z80 Bus (word access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_has_bus = 0;
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte LSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_has_bus = 1;
		}
	}

	/* If the z80 is running, sync the z80 execution state */
	if ( ! genz80.z80_is_reset )
		space->machine().scheduler().timer_set( attotime::zero, FUNC(megadriv_z80_run_state ));
}

static WRITE16_HANDLER ( megadriv_68k_req_z80_reset )
{
	if (!ACCESSING_BITS_0_7) // byte access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 clear z80 reset (byte MSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_is_reset = 0;
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte MSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_is_reset = 1;
		}
	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		if (data & 0x0001)
		{
			//logerror("%06x: 68000 clear z80 reset (byte LSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_is_reset = 0;
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte LSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_is_reset = 1;
		}
	}
	else // word access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 clear z80 reset (word access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_is_reset = 0;
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte LSB access) %04x %04x\n", cpu_get_pc(&space->device()),data,mem_mask);
			genz80.z80_is_reset = 1;
		}
	}
	space->machine().scheduler().timer_set( attotime::zero, FUNC(megadriv_z80_run_state ));
}


// just directly access the 68k space, this makes it easier to deal with
// add-on hardware which changes the cpu mapping like the 32x and SegaCD.
// - we might need to add exceptions for example, z80 reading / writing the
//   z80 area of the 68k if games misbehave
static READ8_HANDLER( z80_read_68k_banked_data )
{
	address_space *space68k = space->machine().device<legacy_cpu_device>("maincpu")->space();
	UINT8 ret = space68k->read_byte(genz80.z80_bank_addr+offset);
	return ret;
}

static WRITE8_HANDLER( z80_write_68k_banked_data )
{
	address_space *space68k = space->machine().device<legacy_cpu_device>("maincpu")->space();
	space68k->write_byte(genz80.z80_bank_addr+offset,data);
}


static WRITE8_HANDLER( megadriv_z80_vdp_write )
{
	switch (offset)
	{
		case 0x11:
		case 0x13:
		case 0x15:
		case 0x17:
			sn76496_w(space->machine().device("snsnd"), 0, data);
			break;

		default:
			mame_printf_debug("unhandled z80 vdp write %02x %02x\n",offset,data);
	}

}



static READ8_HANDLER( megadriv_z80_vdp_read )
{
	mame_printf_debug("megadriv_z80_vdp_read %02x\n",offset);
	return space->machine().rand();
}

static READ8_HANDLER( megadriv_z80_unmapped_read )
{
	return 0xff;
}

static ADDRESS_MAP_START( megadriv_z80_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAMBANK("bank1") AM_MIRROR(0x2000) // RAM can be accessed by the 68k
	AM_RANGE(0x4000, 0x4003) AM_DEVREADWRITE_LEGACY("ymsnd", ym2612_r,ym2612_w)

	AM_RANGE(0x6000, 0x6000) AM_WRITE_LEGACY(megadriv_z80_z80_bank_w)
	AM_RANGE(0x6001, 0x6001) AM_WRITE_LEGACY(megadriv_z80_z80_bank_w) // wacky races uses this address

	AM_RANGE(0x6100, 0x7eff) AM_READ_LEGACY(megadriv_z80_unmapped_read)

	AM_RANGE(0x7f00, 0x7fff) AM_READWRITE_LEGACY(megadriv_z80_vdp_read,megadriv_z80_vdp_write)

	AM_RANGE(0x8000, 0xffff) AM_READWRITE_LEGACY(z80_read_68k_banked_data,z80_write_68k_banked_data) // The Z80 can read the 68k address space this way
ADDRESS_MAP_END

static ADDRESS_MAP_START( megadriv_z80_io_map, AS_IO, 8, driver_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0000, 0xff) AM_NOP
ADDRESS_MAP_END


/************************************ Megadrive Bootlegs *************************************/

// smaller ROM region because some bootlegs check for RAM there
static ADDRESS_MAP_START( md_bootleg_map, AS_PROGRAM, 16, driver_device )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM	/* Cartridge Program Rom */
	AM_RANGE(0x200000, 0x2023ff) AM_RAM // tested

	AM_RANGE(0xa00000, 0xa01fff) AM_READWRITE_LEGACY(megadriv_68k_read_z80_ram, megadriv_68k_write_z80_ram)
	AM_RANGE(0xa02000, 0xa03fff) AM_WRITE_LEGACY(megadriv_68k_write_z80_ram)
	AM_RANGE(0xa04000, 0xa04003) AM_DEVREADWRITE8_LEGACY("ymsnd", megadriv_68k_YM2612_read, megadriv_68k_YM2612_write, 0xffff)
	AM_RANGE(0xa06000, 0xa06001) AM_WRITE_LEGACY(megadriv_68k_z80_bank_write)

	AM_RANGE(0xa10000, 0xa1001f) AM_READWRITE_LEGACY(megadriv_68k_io_read, megadriv_68k_io_write)
	AM_RANGE(0xa11100, 0xa11101) AM_READWRITE_LEGACY(megadriv_68k_check_z80_bus, megadriv_68k_req_z80_bus)
	AM_RANGE(0xa11200, 0xa11201) AM_WRITE_LEGACY(megadriv_68k_req_z80_reset)

	AM_RANGE(0xc00000, 0xc0001f) AM_READWRITE_LEGACY(megadriv_vdp_r, megadriv_vdp_w)
	AM_RANGE(0xd00000, 0xd0001f) AM_READWRITE_LEGACY(megadriv_vdp_r, megadriv_vdp_w) // the earth defend

	AM_RANGE(0xe00000, 0xe0ffff) AM_RAM AM_MIRROR(0x1f0000) AM_BASE_LEGACY(&megadrive_ram)
ADDRESS_MAP_END

MACHINE_CONFIG_DERIVED( md_bootleg, megadriv )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(md_bootleg_map)
MACHINE_CONFIG_END


/****************************************** 32X related ******************************************/

/**********************************************************************************************/
// Function Prototypes
/**********************************************************************************************/


static READ16_HANDLER( _32x_common_vdp_regs_r );
static WRITE16_HANDLER( _32x_common_vdp_regs_w );

static UINT16 _32x_autofill_length;
static UINT16 _32x_autofill_address;
static UINT16 _32x_autofill_data;






static READ16_HANDLER( _32x_68k_palette_r )
{
	return _32x_palette[offset];
}

static WRITE16_HANDLER( _32x_68k_palette_w )
{
	int r,g,b, p;

	COMBINE_DATA(&_32x_palette[offset]);
	data = _32x_palette[offset];

	r = ((data >> 0)  & 0x1f);
	g = ((data >> 5)  & 0x1f);
	b = ((data >> 10) & 0x1f);
	p = ((data >> 15) & 0x01); // priority 'through' bit

	_32x_palette_lookup[offset] = (r << 10) | (g << 5) | (b << 0) | (p << 15);

	palette_set_color_rgb(space->machine(),offset+0x40,pal5bit(r),pal5bit(g),pal5bit(b));

}

static READ16_HANDLER( _32x_68k_dram_r )
{
	return _32x_access_dram[offset];
}

static WRITE16_HANDLER( _32x_68k_dram_w )
{
	if ((mem_mask&0xffff) == 0xffff)
	{
		// 16-bit accesses are normal
		COMBINE_DATA(&_32x_access_dram[offset]);
	}
	else
	{
		// 8-bit writes act as if they were going to the overwrite region!
		// bc-racers and world series baseball rely on this!
		// (tested on real hw)

		if ((mem_mask & 0xffff) == 0xff00)
		{
			if ((data & 0xff00) != 0x0000)
			{
				_32x_access_dram[offset] = (data & 0xff00) |  (_32x_access_dram[offset] & 0x00ff);
			}
		}
		else if ((mem_mask & 0xffff) == 0x00ff)
		{
			if ((data & 0x00ff) != 0x0000)
			{
				_32x_access_dram[offset] = (data & 0x00ff) |  (_32x_access_dram[offset] & 0xff00);
			}
		}
	}
}

static READ16_HANDLER( _32x_68k_dram_overwrite_r )
{
	return _32x_access_dram[offset];
}

static WRITE16_HANDLER( _32x_68k_dram_overwrite_w )
{
	//COMBINE_DATA(&_32x_access_dram[offset+0x10000]);

	if (ACCESSING_BITS_8_15)
	{
		if (data & 0xff00)
		{
			_32x_access_dram[offset] = (_32x_access_dram[offset]&0x00ff) | (data & 0xff00);
		}
	}

	if (ACCESSING_BITS_0_7)
	{
		if (data & 0x00ff)
		{
			_32x_access_dram[offset] = (_32x_access_dram[offset]&0xff00) | (data & 0x00ff);
		}
	}
}

/**********************************************************************************************/
// 68k side a15112
// FIFO
/**********************************************************************************************/

static UINT16 fifo_block_a[4];
static UINT16 fifo_block_b[4];
static UINT16* current_fifo_block;
static UINT16* current_fifo_readblock;
int current_fifo_write_pos;
int current_fifo_read_pos;
int fifo_block_a_full;
int fifo_block_b_full;




/*

15106 DREQ

 ---- ---- F--- -K0R

 F = Fifo FULL
 K = 68k CPU Write mode (0 = no, 1 = CPU write)
 0 = always 0? no, marsch test wants it to be latched or 1
 R = RV (0 = no operation, 1 = DMA Start allowed) <-- RV bit actually affects memory mapping, this is misleading..  it just sets the memory up in a suitable way to use the genesis VDP DMA

*/

static UINT16 a15106_reg;


static READ16_HANDLER( _32x_68k_a15106_r )
{
	UINT16 retval;

	retval = a15106_reg;

	if (fifo_block_a_full && fifo_block_b_full) retval |= 0x8080;

	return retval;
}

static WRITE16_HANDLER( _32x_68k_a15106_w )
{
	if (ACCESSING_BITS_0_7)
	{
		a15106_reg = data & 0x7;

        if (a15106_reg & 0x1) /* NBA Jam TE relies on this */
		{

			// install the game rom in the normal 0x000000-0x03fffff space used by the genesis - this allows VDP DMA operations to work as they have to be from this area or RAM
			// it should also UNMAP the banked rom area...
			space->install_rom(0x0000100, 0x03fffff, space->machine().region("gamecart")->base() + 0x100);
		}
		else
		{
			// we should be careful and map back any rom overlay (hint) and backup ram too I think...

			// this is actually blank / nop area
			// we should also map the banked area back (we don't currently unmap it tho)
			space->install_rom(0x0000100, 0x03fffff, space->machine().region("maincpu")->base()+0x100);
		}

		if((a15106_reg & 4) == 0) // clears the FIFO state
		{
			current_fifo_block = fifo_block_a;
			current_fifo_readblock = fifo_block_b;
			current_fifo_write_pos = 0;
			current_fifo_read_pos = 0;
			fifo_block_a_full = 0;
			fifo_block_b_full = 0;
		}

		//printf("_32x_68k_a15106_w %04x\n", data);
		/*
        if (a15106_reg & 0x4)
            printf(" --- 68k Write Mode enabled\n");
        else
            printf(" --- 68k Write Mode disabled\n");

        if (a15106_reg & 0x1)
            printf(" --- DMA Start Allowed \n");
        else
            printf(" --- DMA Start No Operation\n");

        */
	}
}

static UINT16 dreq_src_addr[2],dreq_dst_addr[2],dreq_size;

static READ16_HANDLER( _32x_dreq_common_r )
{
	address_space* _68kspace = space->machine().device("maincpu")->memory().space(AS_PROGRAM);

	switch (offset)
	{
		case 0x00/2: // a15108 / 4008
		case 0x02/2: // a1510a / 400a
			return dreq_src_addr[offset&1];

		case 0x04/2: // a1510c / 400c
		case 0x06/2: // a1510e / 400e
			return dreq_dst_addr[offset&1];

		case 0x08/2: // a15110 / 4010
			return dreq_size;

		case 0x0a/2: // a15112 / 4012
			if (space == _68kspace)
			{
				printf("attempting to READ FIFO with 68k!\n");
				return 0xffff;
			}

			UINT16 retdat = current_fifo_readblock[current_fifo_read_pos];

			current_fifo_read_pos++;

		//  printf("reading FIFO!\n");

			if (current_fifo_readblock == fifo_block_a && !fifo_block_a_full)
				printf("Fifo block a isn't filled!\n");

			if (current_fifo_readblock == fifo_block_b && !fifo_block_b_full)
				printf("%08x Fifo block b isn't filled!\n",cpu_get_pc(&space->device()));


			if (current_fifo_read_pos==4)
			{
				if (current_fifo_readblock == fifo_block_a)
				{
					fifo_block_a_full = 0;

					if (fifo_block_b_full)
					{
						current_fifo_readblock = fifo_block_b;
						current_fifo_block = fifo_block_a;
					}

					current_fifo_read_pos = 0;
				}
				else if (current_fifo_readblock == fifo_block_b)
				{
					fifo_block_b_full = 0;

					if (fifo_block_a_full)
					{
						current_fifo_readblock = fifo_block_a;
						current_fifo_block = fifo_block_b;
					}

					current_fifo_read_pos = 0;
				}
			}

			return retdat;
	}

	return 0x0000;
}

static WRITE16_HANDLER( _32x_dreq_common_w )
{
	address_space* _68kspace = space->machine().device("maincpu")->memory().space(AS_PROGRAM);

	switch (offset)
	{
		case 0x00/2: // a15108 / 4008
		case 0x02/2: // a1510a / 400a
			if (space != _68kspace)
			{
				printf("attempting to WRITE DREQ SRC with SH2!\n");
				return;
			}

			dreq_src_addr[offset&1] = ((offset&1) == 0) ? (data & 0xff) : (data & 0xfffe);

			//if((dreq_src_addr[0]<<16)|dreq_src_addr[1])
			//  printf("DREQ set SRC = %08x\n",(dreq_src_addr[0]<<16)|dreq_src_addr[1]);

			break;

		case 0x04/2: // a1510c / 400c
		case 0x06/2: // a1510e / 400e
			if (space != _68kspace)
			{
				printf("attempting to WRITE DREQ DST with SH2!\n");
				return;
			}

			dreq_dst_addr[offset&1] = ((offset&1) == 0) ? (data & 0xff) : (data & 0xffff);

			//if((dreq_dst_addr[0]<<16)|dreq_dst_addr[1])
			//  printf("DREQ set DST = %08x\n",(dreq_dst_addr[0]<<16)|dreq_dst_addr[1]);

			break;

		case 0x08/2: // a15110 / 4010
			if (space != _68kspace)
			{
				printf("attempting to WRITE DREQ SIZE with SH2!\n");
				return;
			}

			dreq_size = data & 0xfffc;

			//  if(dreq_size)
			//      printf("DREQ set SIZE = %04x\n",dreq_size);

			break;

		case 0x0a/2: // a15112 / 4012 - FIFO Write (68k only!)
			if (space != _68kspace)
			{
				printf("attempting to WRITE FIFO with SH2!\n");
				return;
			}

			if (current_fifo_block==fifo_block_a && fifo_block_a_full)
			{
				printf("attempt to write to Full Fifo block a!\n");
				return;
			}

			if (current_fifo_block==fifo_block_b && fifo_block_b_full)
			{
				printf("attempt to write to Full Fifo block b!\n");
				return;
			}

			if((a15106_reg & 4) == 0)
			{
				printf("attempting to WRITE FIFO with 68S cleared!\n"); // corpse32
				return;
			}

			current_fifo_block[current_fifo_write_pos] = data;
			current_fifo_write_pos++;

			if (current_fifo_write_pos==4)
			{
				if (current_fifo_block==fifo_block_a)
				{
					fifo_block_a_full = 1;
					if (!fifo_block_b_full)
					{
						current_fifo_block = fifo_block_b;
						current_fifo_readblock = fifo_block_a;
						// incase we have a stalled DMA in progress, let the SH2 know there is data available
						sh2_notify_dma_data_available(space->machine().device("32x_master_sh2"));
						sh2_notify_dma_data_available(space->machine().device("32x_slave_sh2"));

					}
					current_fifo_write_pos = 0;
				}
				else
				{
					fifo_block_b_full = 1;

					if (!fifo_block_a_full)
					{
						current_fifo_block = fifo_block_a;
						current_fifo_readblock = fifo_block_b;
						// incase we have a stalled DMA in progress, let the SH2 know there is data available
						sh2_notify_dma_data_available(space->machine().device("32x_master_sh2"));
						sh2_notify_dma_data_available(space->machine().device("32x_slave_sh2"));

					}

					current_fifo_write_pos = 0;
				}
			}

			break;
	}
}


static UINT8 sega_tv;

static READ16_HANDLER( _32x_68k_a1511a_r )
{
	return sega_tv;
}

static WRITE16_HANDLER( _32x_68k_a1511a_w )
{
	sega_tv = data & 1;

	printf("SEGA TV register set = %04x\n",data);
}

/*
000070 H interrupt vector can be overwritten apparently
*/

static UINT16 hint_vector[2];

static READ16_HANDLER( _32x_68k_hint_vector_r )
{
	return hint_vector[offset];
}

static WRITE16_HANDLER( _32x_68k_hint_vector_w )
{
	hint_vector[offset] = data;
}

// returns MARS, the system ID of the 32x
static READ16_HANDLER( _32x_68k_MARS_r )
{
    switch (offset)
    {
        case 0:
            return 0x4d41;

        case 1:
            return 0x5253;
    }

    return 0x0000;
}


/**********************************************************************************************/
// 68k side a15100
// control register - used to enable 32x etc.
/**********************************************************************************************/

static UINT16 a15100_reg;

static READ16_HANDLER( _32x_68k_a15100_r )
{
	return (_32x_access_auth<<15) | 0x0080;
}

static WRITE16_HANDLER( _32x_68k_a15100_w )
{
	if (ACCESSING_BITS_0_7)
	{
		a15100_reg = (a15100_reg & 0xff00) | (data & 0x00ff);

		if (data & 0x02)
		{
			device_set_input_line(_32x_master_cpu, INPUT_LINE_RESET, CLEAR_LINE);
			device_set_input_line(_32x_slave_cpu, INPUT_LINE_RESET, CLEAR_LINE);
		}

		if (data & 0x01)
		{
			_32x_adapter_enabled = 1;
			space->install_rom(0x0880000, 0x08fffff, space->machine().region("gamecart")->base()); // 'fixed' 512kb rom bank

			space->install_read_bank(0x0900000, 0x09fffff, "bank12"); // 'bankable' 1024kb rom bank
			memory_set_bankptr(space->machine(),  "bank12", space->machine().region("gamecart")->base()+((_32x_68k_a15104_reg&0x3)*0x100000) );

			space->install_rom(0x0000000, 0x03fffff, space->machine().region("32x_68k_bios")->base());

			/* VDP area */
			space->install_legacy_readwrite_handler(0x0a15180, 0x0a1518b, FUNC(_32x_common_vdp_regs_r), FUNC(_32x_common_vdp_regs_w)); // common / shared VDP regs
			space->install_legacy_readwrite_handler(0x0a15200, 0x0a153ff, FUNC(_32x_68k_palette_r), FUNC(_32x_68k_palette_w)); // access to 'palette' xRRRRRGGGGGBBBBB
			space->install_legacy_readwrite_handler(0x0840000, 0x085ffff, FUNC(_32x_68k_dram_r), FUNC(_32x_68k_dram_w)); // access to 'display ram' (framebuffer)
			space->install_legacy_readwrite_handler(0x0860000, 0x087ffff, FUNC(_32x_68k_dram_overwrite_r), FUNC(_32x_68k_dram_overwrite_w)); // access to 'display ram' (framebuffer)



			space->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x000070, 0x000073, FUNC(_32x_68k_hint_vector_r), FUNC(_32x_68k_hint_vector_w)); // h interrupt vector
		}
		else
		{
			_32x_adapter_enabled = 0;

			space->install_rom(0x0000000, 0x03fffff, space->machine().region("gamecart")->base());
			space->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x000070, 0x000073, FUNC(_32x_68k_hint_vector_r), FUNC(_32x_68k_hint_vector_w)); // h interrupt vector
		}
	}

	if (ACCESSING_BITS_8_15)
	{
		a15100_reg = (a15100_reg & 0x00ff) | (data & 0xff00);
		_32x_access_auth = (data & 0x8000)>>15;
	}
}

/**********************************************************************************************/
// 68k side a15102
// command interrupt to SH2
/**********************************************************************************************/

static int _32x_68k_a15102_reg;

static READ16_HANDLER( _32x_68k_a15102_r )
{
	//printf("_32x_68k_a15102_r\n");
	return _32x_68k_a15102_reg;
}

static WRITE16_HANDLER( _32x_68k_a15102_w )
{
	if (ACCESSING_BITS_0_7)
	{
		_32x_68k_a15102_reg = data & 3;

		if (data&0x1)
		{
			if (sh2_master_cmdint_enable) device_set_input_line(_32x_master_cpu,SH2_CINT_IRQ_LEVEL,ASSERT_LINE);
			else printf("master cmdint when masked!\n");
		}

		if (data&0x2)
		{
			if (sh2_slave_cmdint_enable) device_set_input_line(_32x_slave_cpu,SH2_CINT_IRQ_LEVEL,ASSERT_LINE);
			else printf("slave cmdint when masked!\n");
		}
	}
}

/**********************************************************************************************/
// 68k side a15104
// ROM banking for 68k rom
/**********************************************************************************************/


static READ16_HANDLER( _32x_68k_a15104_r )
{
	return _32x_68k_a15104_reg;
}

static WRITE16_HANDLER( _32x_68k_a15104_w )
{
	if (ACCESSING_BITS_0_7)
	{
		_32x_68k_a15104_reg = (_32x_68k_a15104_reg & 0xff00) | (data & 0x00ff);
	}

	if (ACCESSING_BITS_8_15)
	{
		_32x_68k_a15104_reg = (_32x_68k_a15104_reg & 0x00ff) | (data & 0xff00);
	}

	memory_set_bankptr(space->machine(),  "bank12", space->machine().region("gamecart")->base()+((_32x_68k_a15104_reg&0x3)*0x100000) );
}

/**********************************************************************************************/
// 68k side a15120 - a1512f
// Communication Port 0
// access from the SH2 via 4020 - 402f
/**********************************************************************************************/
#define _32X_COMMS_PORT_SYNC 0
static UINT16 commsram[8];

/**********************************************************************************************/

// reads
static READ16_HANDLER( _32x_68k_commsram_r )
{
	if (_32X_COMMS_PORT_SYNC) space->machine().scheduler().synchronize();
	return commsram[offset];
}

// writes
static WRITE16_HANDLER( _32x_68k_commsram_w )
{
	COMBINE_DATA(&commsram[offset]);
	if (_32X_COMMS_PORT_SYNC) space->machine().scheduler().synchronize();
}

/**********************************************************************************************/
// 68k side a15130 - a1513f
// PWM registers
// access from the SH2 via 4030 - 403f
/**********************************************************************************************/

/*
TODO:
- noticeable static noise on Virtua Fighter Sega logo at start-up
- Understand if Speaker OFF makes the FIFO to advance or not
*/

#define PWM_FIFO_SIZE pwm_tm_reg // guess, Marsch calls this register as FIFO width
#define PWM_CLOCK megadrive_region_pal ? ((MASTER_CLOCK_PAL*3) / 7) : ((MASTER_CLOCK_NTSC*3) / 7)

static UINT16 pwm_ctrl,pwm_cycle,pwm_tm_reg;
static UINT16 cur_lch[0x10],cur_rch[0x10];
static UINT16 pwm_cycle_reg; //used for latching
static UINT8 pwm_timer_tick;
static UINT8 lch_index_r,rch_index_r,lch_index_w,rch_index_w;
static UINT16 lch_fifo_state,rch_fifo_state;

static emu_timer *_32x_pwm_timer;

static void calculate_pwm_timer(void)
{
	if(pwm_tm_reg == 0) { pwm_tm_reg = 16; } // zero gives max range
	if(pwm_cycle == 0) { pwm_cycle = 4095; } // zero gives max range

	/* if both RMD and LMD are set to OFF or pwm cycle register is one, then PWM timer ticks doesn't occur */
	if(pwm_cycle == 1 || ((pwm_ctrl & 0xf) == 0))
		_32x_pwm_timer->adjust(attotime::never);
	else
	{
		pwm_timer_tick = 0;
		lch_fifo_state = rch_fifo_state = 0x4000;
		lch_index_r = rch_index_r = 0;
		lch_index_w = rch_index_w = 0;
		_32x_pwm_timer->adjust(attotime::from_hz((PWM_CLOCK) / (pwm_cycle - 1)));
	}
}

static TIMER_CALLBACK( _32x_pwm_callback )
{
	if(lch_index_r < PWM_FIFO_SIZE)
	{
		switch(pwm_ctrl & 3)
		{
			case 0: lch_index_r++; /*Speaker OFF*/ break;
			case 1: dac_signed_data_16_w(machine.device("lch_pwm"), cur_lch[lch_index_r++]); break;
			case 2: dac_signed_data_16_w(machine.device("rch_pwm"), cur_lch[lch_index_r++]); break;
			case 3: popmessage("Undefined PWM Lch value 3, contact MESSdev"); break;
		}

		lch_index_w = 0;
	}

	lch_fifo_state = (lch_index_r == PWM_FIFO_SIZE) ? 0x4000 : 0x0000;

	if(rch_index_r < PWM_FIFO_SIZE)
	{
		switch((pwm_ctrl & 0xc) >> 2)
		{
			case 0: rch_index_r++; /*Speaker OFF*/ break;
			case 1: dac_signed_data_16_w(machine.device("rch_pwm"), cur_rch[rch_index_r++]); break;
			case 2: dac_signed_data_16_w(machine.device("lch_pwm"), cur_rch[rch_index_r++]); break;
			case 3: popmessage("Undefined PWM Rch value 3, contact MESSdev"); break;
		}

		rch_index_w = 0;
	}

	rch_fifo_state = (rch_index_r == PWM_FIFO_SIZE) ? 0x4000 : 0x0000;

	pwm_timer_tick++;

	if(pwm_timer_tick == pwm_tm_reg)
	{
		pwm_timer_tick = 0;
		if(sh2_master_pwmint_enable) { device_set_input_line(_32x_master_cpu, SH2_PINT_IRQ_LEVEL,ASSERT_LINE); }
		if(sh2_slave_pwmint_enable) { device_set_input_line(_32x_slave_cpu, SH2_PINT_IRQ_LEVEL,ASSERT_LINE); }
	}

	_32x_pwm_timer->adjust(attotime::from_hz((PWM_CLOCK) / (pwm_cycle - 1)));
}

static READ16_HANDLER( _32x_pwm_r )
{
	switch(offset)
	{
		case 0x00/2: return pwm_ctrl; //control register
		case 0x02/2: return pwm_cycle_reg; // cycle register
		case 0x04/2: return lch_fifo_state; // l ch
		case 0x06/2: return rch_fifo_state; // r ch
		case 0x08/2: return lch_fifo_state & rch_fifo_state; // mono ch
	}

	printf("Read at undefined PWM register %02x\n",offset);
	return 0xffff;
}

static WRITE16_HANDLER( _32x_pwm_w )
{
	switch(offset)
	{
		case 0x00/2:
			pwm_ctrl = data & 0xffff;
			pwm_tm_reg = (pwm_ctrl & 0xf00) >> 8;
			calculate_pwm_timer();
			break;
		case 0x02/2:
			pwm_cycle = pwm_cycle_reg = data & 0xfff;
			calculate_pwm_timer();
			break;
		case 0x04/2:
			if(lch_index_w < PWM_FIFO_SIZE)
			{
				cur_lch[lch_index_w++] = ((data & 0xfff) << 4) | (data & 0xf);
				lch_index_r = 0;
			}

			lch_fifo_state = (lch_index_w == PWM_FIFO_SIZE) ? 0x8000 : 0x0000;
			break;
		case 0x06/2:
			if(rch_index_w < PWM_FIFO_SIZE)
			{
				cur_rch[rch_index_w++] = ((data & 0xfff) << 4) | (data & 0xf);
				rch_index_r = 0;
			}

			rch_fifo_state = (rch_index_w == PWM_FIFO_SIZE) ? 0x8000 : 0x0000;

			break;
		case 0x08/2:
			if(lch_index_w < PWM_FIFO_SIZE)
			{
				cur_lch[lch_index_w++] = ((data & 0xfff) << 4) | (data & 0xf);
				lch_index_r = 0;
			}

			if(rch_index_w < PWM_FIFO_SIZE)
			{
				cur_rch[rch_index_w++] = ((data & 0xfff) << 4) | (data & 0xf);
				rch_index_r = 0;
			}

			lch_fifo_state = (lch_index_w == PWM_FIFO_SIZE) ? 0x8000 : 0x0000;
			rch_fifo_state = (rch_index_w == PWM_FIFO_SIZE) ? 0x8000 : 0x0000;

			break;
		default:
			printf("Write at undefined PWM register %02x %04x\n",offset,data);
			break;
	}
}

static WRITE16_HANDLER( _32x_68k_pwm_w )
{
	if(offset == 0/2)
		_32x_pwm_w(space,offset,(data & 0x7f) | (pwm_ctrl & 0xff80),mem_mask);
	else
		_32x_pwm_w(space,offset,data,mem_mask);
}

/**********************************************************************************************/
// 68k side a15180
// framebuffer control
// also accessed from the SH2 @ 4100
/**********************************************************************************************/

static UINT16 _32x_a1518a_reg;

static READ16_HANDLER( _32x_common_vdp_regs_r )
{
	// what happens if the z80 accesses it, what authorization do we use?



//  printf("_32x_68k_a15180_r (a15180) %04x\n",mem_mask);

	// read needs authorization too I think, undefined behavior otherwise
	switch (offset)
	{
		case 0x00:

		// the flag is inverted compared to the megadrive
		int ntsc;
		if (megadrive_region_pal) ntsc = 0;
		else ntsc = 1;

		return (ntsc << 15) |
			   (_32x_videopriority << 7 ) |
			   ( _32x_240mode << 6 ) |
			   ( _32x_displaymode << 0 );



		case 0x02/2:
			return _32x_screenshift;

		case 0x04/2:
			return _32x_autofill_length;

		case 0x06/2:
			return _32x_autofill_address;

		case 0x08/2:
			return _32x_autofill_data;

		case 0x0a/2:
			UINT16 retdata = _32x_a1518a_reg;
			UINT16 hpos = get_hposition();
			int megadrive_hblank_flag = 0;

			if (megadrive_vblank_flag) retdata |= 0x8000;

			if (hpos>400) megadrive_hblank_flag = 1;
			if (hpos>460) megadrive_hblank_flag = 0;

			if (megadrive_hblank_flag) retdata |= 0x4000;

			if (megadrive_vblank_flag) { retdata |= 2; } // framebuffer approval (TODO: condition is unknown at current time)

			if (megadrive_hblank_flag && megadrive_vblank_flag) { retdata |= 0x2000; } // palette approval (TODO: active high or low?)

			return retdata;
	}

	return 0x0000;
}


void _32x_check_framebuffer_swap(void)
{

	if(_32x_is_connected)
	{

		// this logic should be correct, but makes things worse?
		//if (genesis_scanline_counter >= megadrive_irq6_scanline)
		{
			_32x_a1518a_reg = _32x_fb_swap & 1;



			if (_32x_fb_swap & 1)
			{
				_32x_access_dram = _32x_dram0;
				_32x_display_dram = _32x_dram1;
			}
			else
			{
				_32x_display_dram = _32x_dram0;
				_32x_access_dram = _32x_dram1;
			}
		}
	}
}


static WRITE16_HANDLER( _32x_common_vdp_regs_w )
{
	// what happens if the z80 accesses it, what authorization do we use? which address space do we get?? the z80 *can* write here and to the framebuffer via the window

	address_space* _68kspace = space->machine().device("maincpu")->memory().space(AS_PROGRAM);

	if (space!= _68kspace)
	{
		if (_32x_access_auth!=1)
			return;
	}

	if (space== _68kspace)
	{
		if (_32x_access_auth!=0)
			return;
	}


	switch (offset)
	{

		case 0x00:
			//printf("_32x_68k_a15180_w (a15180) %04x %04x   source _32x_access_auth %04x\n",data,mem_mask, _32x_access_auth);

			if (ACCESSING_BITS_0_7)
			{
				_32x_videopriority = (data & 0x80) >> 7;
				_32x_240mode   = (data & 0x40) >> 6;
				_32x_displaymode   = (data & 0x03) >> 0;
			}
			break;

		case 0x02/2:
			if (ACCESSING_BITS_0_7)
			{
				_32x_screenshift = data & 1; // allows 1 pixel shifting
			}
			if (ACCESSING_BITS_8_15)
			{

			}
			break;

		case 0x04/2:
			if (ACCESSING_BITS_0_7)
			{
				_32x_autofill_length = data & 0xff;
			}

			if (ACCESSING_BITS_8_15)
			{

			}
			break;

		case 0x06/2:
			if (ACCESSING_BITS_0_7)
			{
				_32x_autofill_address = (_32x_autofill_address & 0xff00) | (data & 0x00ff);
			}

			if (ACCESSING_BITS_8_15)
			{
				_32x_autofill_address = (_32x_autofill_address & 0x00ff) | (data & 0xff00);
			}
			break;

		case 0x08/2:
			if (ACCESSING_BITS_0_7)
			{
				_32x_autofill_data = (_32x_autofill_data & 0xff00) | (data & 0x00ff);
			}

			if (ACCESSING_BITS_8_15)
			{
				_32x_autofill_data = (_32x_autofill_data & 0x00ff) | (data & 0xff00);
			}

			// do the fill - shouldn't be instant..
			{
				int i;
				for (i=0; i<_32x_autofill_length+1;i++)
				{
					_32x_access_dram[_32x_autofill_address] = _32x_autofill_data;
					_32x_autofill_address = (_32x_autofill_address & 0xff00) | ((_32x_autofill_address+1) & 0x00ff);
				}
			}
			break;

		case 0x0a/2:
			// bit 0 is the framebuffer select, change is delayed until vblank;
		//  _32x_a1518a_reg = (_32x_a1518a_reg & 0xfffe);
			if (ACCESSING_BITS_0_7)
			{
				_32x_fb_swap = data & 1;

				_32x_check_framebuffer_swap();
			}

			break;


	}


}


/**********************************************************************************************/
// SH2 side 4000
// IRQ Control
// Different for each SH2

/*
f--- --ec h--- VHCP

f = framebuffer permission (0 md, 1 sh2)
e = Adapter enabled (0 no, 1 yes)
c = Cart Inserted (0 yes, 1 no)
h = H Interrupt allowed within Vblank (0 no, 1 yes)

*** these are independent for each SH2 ***
V = V Interrupt Mask (0 masked, 1 allowed)
H = H Interrupt Mask (0 masked, 1 allowed)
C = Command Interrupt Mask (0 masked, 1 allowed)
P = PWM Interrupt Mask (0 masked, 1 allowed)
*/

/**********************************************************************************************/

/* MASTER */
static READ16_HANDLER( _32x_sh2_master_4000_r )
{
	UINT16 retvalue = 0x0200;
	retvalue |= _32x_access_auth << 15;

	retvalue |=	sh2_hint_in_vbl;
	retvalue |= sh2_master_vint_enable;
	retvalue |= sh2_master_hint_enable;
	retvalue |= sh2_master_cmdint_enable;
	retvalue |= sh2_master_pwmint_enable;

	return retvalue;
}

static WRITE16_HANDLER( _32x_sh2_master_4000_w )
{
	if (ACCESSING_BITS_8_15)
	{
		_32x_access_auth = (data &0x8000) >> 15;
	}

	if (ACCESSING_BITS_0_7)
	{
		sh2_hint_in_vbl = data & 0x80;
		sh2_master_vint_enable = data & 0x8;
		sh2_master_hint_enable = data & 0x4;
		sh2_master_cmdint_enable = data & 0x2;
		sh2_master_pwmint_enable = data & 0x1;

		//if (sh2_master_hint_enable) printf("sh2_master_hint_enable enable!\n");
		//if (sh2_master_pwmint_enable) printf("sh2_master_pwn_enable enable!\n");

		_32x_check_irqs(space->machine());
	}
}

/* SLAVE */

static READ16_HANDLER( _32x_sh2_slave_4000_r )
{
	UINT16 retvalue = 0x0200;
	retvalue |= _32x_access_auth << 15;
	retvalue |=	sh2_hint_in_vbl;
	retvalue |= sh2_slave_vint_enable;
	retvalue |= sh2_slave_hint_enable;
	retvalue |= sh2_slave_cmdint_enable;
	retvalue |= sh2_slave_pwmint_enable;

	return retvalue;
}


static WRITE16_HANDLER( _32x_sh2_slave_4000_w )
{
	if (ACCESSING_BITS_8_15)
	{
		_32x_access_auth = (data &0x8000) >> 15;
	}

	if (ACCESSING_BITS_0_7)
	{
		sh2_hint_in_vbl = data & 0x80;
		sh2_slave_vint_enable = data & 0x8;
		sh2_slave_hint_enable = data & 0x4;
		sh2_slave_cmdint_enable = data & 0x2;
		sh2_slave_pwmint_enable = data & 0x1;

		//if (sh2_slave_hint_enable) printf("sh2_slave_hint_enable enable!\n");
		//if (sh2_slave_pwmint_enable) printf("sh2_slave_pwm_enable enable!\n");

		_32x_check_irqs(space->machine());

	}
}

/**********************************************************************************************/
// SH2 side 4002
// Reserved  ( Stand By Change Register )
// Shouldn't be used
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_common_4002_r )
{
	printf("reading 4002!\n");
	return 0x0000;
}

static WRITE16_HANDLER( _32x_sh2_common_4002_w )
{
	printf("write 4002!\n");
}


/**********************************************************************************************/
// SH2 side 4004
// H Count Register (H Interrupt)
// 0 = every line
/**********************************************************************************************/
static READ16_HANDLER( _32x_sh2_common_4004_r )
{
	return _32x_hcount_reg;
}

static WRITE16_HANDLER( _32x_sh2_common_4004_w )
{
	_32x_hcount_reg = data & 0xff;
}


/**********************************************************************************************/
// SH2 side 4006
// DReq Control Register
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_common_4006_r )
{
	//printf("DREQ read!\n"); // tempo reads it, shut up for now
	return _32x_68k_a15106_r(space,offset,mem_mask);
}

static WRITE16_HANDLER( _32x_sh2_common_4006_w )
{
	printf("DREQ write!\n"); //register is read only on SH-2 side
}


/**********************************************************************************************/
// SH2 side 4014
// VRES (md reset button interrupt) clear
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_4014_w ){ device_set_input_line(_32x_master_cpu,SH2_VRES_IRQ_LEVEL,CLEAR_LINE);}
static WRITE16_HANDLER( _32x_sh2_slave_4014_w ) { device_set_input_line(_32x_slave_cpu, SH2_VRES_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 4016
// VINT (vertical interrupt) clear
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_4016_w ){ sh2_master_vint_pending = 0; _32x_check_irqs(space->machine()); }
static WRITE16_HANDLER( _32x_sh2_slave_4016_w ) { sh2_slave_vint_pending = 0; _32x_check_irqs(space->machine()); }

/**********************************************************************************************/
// SH2 side 4018
// HINT (horizontal interrupt) clear
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_4018_w ){ device_set_input_line(_32x_master_cpu,SH2_HINT_IRQ_LEVEL,CLEAR_LINE);}
static WRITE16_HANDLER( _32x_sh2_slave_4018_w ) { device_set_input_line(_32x_slave_cpu, SH2_HINT_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 401A
// HINT (control register interrupt) clear
// Note: flag cleared here is a guess, according to After Burner behaviour
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_401a_w ){ _32x_68k_a15102_reg &= ~1; device_set_input_line(_32x_master_cpu,SH2_CINT_IRQ_LEVEL,CLEAR_LINE);}
static WRITE16_HANDLER( _32x_sh2_slave_401a_w ) { _32x_68k_a15102_reg &= ~2; device_set_input_line(_32x_slave_cpu, SH2_CINT_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 401C
// PINT (PWM timer interrupt) clear
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_401c_w ){ device_set_input_line(_32x_master_cpu,SH2_PINT_IRQ_LEVEL,CLEAR_LINE);}
static WRITE16_HANDLER( _32x_sh2_slave_401c_w ) { device_set_input_line(_32x_slave_cpu, SH2_PINT_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 401E
// ?? unknown / unused
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_401e_w )
{
	printf("_32x_sh2_master_401e_w\n");
}

static WRITE16_HANDLER( _32x_sh2_slave_401e_w )
{
	printf("_32x_sh2_slave_401e_w\n");
}

/**********************************************************************************************/
// SH2 side 4020 - 402f
// SH2 -> 68k Comms ports,
// access at a15120 - a1512f on 68k
// these just map through to the 68k functions
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_commsram16_r ) { return _32x_68k_commsram_r(space, offset, mem_mask); }
static WRITE16_HANDLER( _32x_sh2_commsram16_w ) { _32x_68k_commsram_w(space, offset, data, mem_mask); }

/**********************************************************************************************/
// SH2 side 4030
// PWM Control Register
/**********************************************************************************************/

/**********************************************************************************************/
// SH2 side 4032
// Cycle Register
/**********************************************************************************************/


/**********************************************************************************************/
// SH2 side 4034
// LCH Pulse Width Register
/**********************************************************************************************/

/**********************************************************************************************/
// SH2 side 4036
// RCH Pulse Width Register
/**********************************************************************************************/

/**********************************************************************************************/
// SH2 side 4038
// Mono Pulse Width Register
/**********************************************************************************************/

/* 4100 - 43ff are VDP registers, you need permission to access them - ensure this is true for all! */

/**********************************************************************************************/
// SH2 side 4200 - 43ff
// palette
// maps through to 68k at a15200 - a153ff
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_paletteram16_r ) { return _32x_68k_palette_r(space,offset,mem_mask); }
static WRITE16_HANDLER( _32x_sh2_paletteram16_w ) { _32x_68k_palette_w(space,offset,data,mem_mask); }

/**********************************************************************************************/
// SH2 side 4000000 - 401ffff
// framebuffer
// maps through to 68k at 840000 - 85ffff
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_framebuffer_dram16_r ) { return _32x_68k_dram_r(space,offset,mem_mask); }
static WRITE16_HANDLER( _32x_sh2_framebuffer_dram16_w ) { _32x_68k_dram_w(space,offset,data,mem_mask); }

/**********************************************************************************************/
// SH2 side 4020000 - 403ffff
// framebuffer overwrite
// maps through to 68k at 860000 - 87ffff
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_framebuffer_overwrite_dram16_r ) { return _32x_68k_dram_overwrite_r(space,offset,mem_mask); }
static WRITE16_HANDLER( _32x_sh2_framebuffer_overwrite_dram16_w ) { _32x_68k_dram_overwrite_w(space,offset,data,mem_mask); }



/**********************************************************************************************/
// SH2 access Macros
/**********************************************************************************************/


/* the 32x treats everything as 16-bit registers, so we remap the 32-bit read & writes
   to 2x 16-bit handlers here (TODO: nuke this eventually) */

#define _32X_MAP_READHANDLERS(NAMEA,NAMEB)                                          \
static READ32_HANDLER( _32x_sh2_##NAMEA##_##NAMEB##_r )                             \
{                                                                                   \
	UINT32 retvalue = 0x00000000;                                                   \
	if (ACCESSING_BITS_16_31)                                                       \
	{                                                                               \
		UINT16 ret = _32x_sh2_##NAMEA##_r(space,0,(mem_mask>>16)&0xffff);         \
		retvalue |= ret << 16;                                                      \
	}                                                                               \
	if (ACCESSING_BITS_0_15)                                                        \
	{                                                                               \
		UINT16 ret = _32x_sh2_##NAMEB##_r(space,0,(mem_mask>>0)&0xffff);          \
		retvalue |= ret << 0;                                                       \
	}                                                                               \
                                                                                    \
	return retvalue;                                                                \
}                                                                                   \

#define _32X_MAP_WRITEHANDLERS(NAMEA,NAMEB)                                             \
static WRITE32_HANDLER( _32x_sh2_##NAMEA##_##NAMEB##_w)                                 \
{                                                                                       \
	if (ACCESSING_BITS_16_31)                                                           \
	{                                                                                   \
		_32x_sh2_##NAMEA##_w(space,0,(data>>16)&0xffff,(mem_mask>>16)&0xffff);        \
	}                                                                                   \
	if (ACCESSING_BITS_0_15)                                                            \
	{                                                                                   \
		_32x_sh2_##NAMEB##_w(space,0,(data>>0)&0xffff,(mem_mask>>0)&0xffff);          \
	}                                                                                   \
}                                                                                       \

/* for RAM ranges, eg. Framebuffer, Comms RAM etc. */

#define _32X_MAP_RAM_READHANDLERS(NAMEA)                                            \
static READ32_HANDLER( _32x_sh2_##NAMEA##_r )                                       \
{                                                                                   \
	UINT32 retvalue = 0x00000000;                                                   \
	if (ACCESSING_BITS_16_31)                                                       \
	{                                                                               \
		UINT16 ret = _32x_sh2_##NAMEA##16_r(space,offset*2,(mem_mask>>16)&0xffff);  \
		retvalue |= ret << 16;                                                      \
	}                                                                               \
	if (ACCESSING_BITS_0_15)                                                        \
	{                                                                               \
		UINT16 ret = _32x_sh2_##NAMEA##16_r(space,offset*2+1,(mem_mask>>0)&0xffff); \
		retvalue |= ret << 0;                                                       \
	}                                                                               \
                                                                                    \
	return retvalue;                                                                \
}                                                                                   \

#define _32X_MAP_RAM_WRITEHANDLERS(NAMEA)                                               \
static WRITE32_HANDLER( _32x_sh2_##NAMEA##_w)                                           \
{                                                                                       \
	if (ACCESSING_BITS_16_31)                                                           \
	{                                                                                   \
		_32x_sh2_##NAMEA##16_w(space,offset*2,(data>>16)&0xffff,(mem_mask>>16)&0xffff); \
	}                                                                                   \
	if (ACCESSING_BITS_0_15)                                                            \
	{                                                                                   \
		_32x_sh2_##NAMEA##16_w(space,offset*2+1,(data>>0)&0xffff,(mem_mask>>0)&0xffff); \
	}                                                                                   \
}                                                                                       \



/**********************************************************************************************/
// SH2 access for Memory Map
/**********************************************************************************************/


_32X_MAP_READHANDLERS(master_4000,common_4002)  // _32x_sh2_master_4000_common_4002_r
_32X_MAP_WRITEHANDLERS(master_4000,common_4002) // _32x_sh2_master_4000_common_4002_w

_32X_MAP_READHANDLERS(slave_4000,common_4002)  // _32x_sh2_slave_4000_common_4002_r
_32X_MAP_WRITEHANDLERS(slave_4000,common_4002) // _32x_sh2_slave_4000_common_4002_w

_32X_MAP_READHANDLERS(common_4004,common_4006)
_32X_MAP_WRITEHANDLERS(common_4004,common_4006)

_32X_MAP_WRITEHANDLERS(master_4014,master_4016) // _32x_sh2_master_4014_master_4016_w
_32X_MAP_WRITEHANDLERS(master_4018,master_401a) // _32x_sh2_master_4018_master_401a_w
_32X_MAP_WRITEHANDLERS(master_401c,master_401e) // _32x_sh2_master_401c_master_401e_w

_32X_MAP_WRITEHANDLERS(slave_4014,slave_4016) // _32x_sh2_slave_4014_slave_4016_w
_32X_MAP_WRITEHANDLERS(slave_4018,slave_401a) // _32x_sh2_slave_4018_slave_401a_w
_32X_MAP_WRITEHANDLERS(slave_401c,slave_401e) // _32x_sh2_slave_401c_slave_401e_w

_32X_MAP_RAM_READHANDLERS(commsram) // _32x_sh2_commsram_r
_32X_MAP_RAM_WRITEHANDLERS(commsram) // _32x_sh2_commsram_w

_32X_MAP_RAM_READHANDLERS(framebuffer_dram) // _32x_sh2_framebuffer_dram_r
_32X_MAP_RAM_WRITEHANDLERS(framebuffer_dram) // _32x_sh2_framebuffer_dram_w

_32X_MAP_RAM_READHANDLERS(framebuffer_overwrite_dram) // _32x_sh2_framebuffer_overwrite_dram_r
_32X_MAP_RAM_WRITEHANDLERS(framebuffer_overwrite_dram) // _32x_sh2_framebuffer_overwrite_dram_w

_32X_MAP_RAM_READHANDLERS(paletteram) // _32x_sh2_paletteram_r
_32X_MAP_RAM_WRITEHANDLERS(paletteram) // _32x_sh2_paletteram_w


/**********************************************************************************************/
// SH2 memory maps
/**********************************************************************************************/

static ADDRESS_MAP_START( sh2_main_map, AS_PROGRAM, 32, driver_device )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM

	AM_RANGE(0x00004000, 0x00004003) AM_READWRITE_LEGACY(_32x_sh2_master_4000_common_4002_r, _32x_sh2_master_4000_common_4002_w )
	AM_RANGE(0x00004004, 0x00004007) AM_READWRITE_LEGACY(_32x_sh2_common_4004_common_4006_r, _32x_sh2_common_4004_common_4006_w)

	AM_RANGE(0x00004008, 0x00004013) AM_READWRITE16_LEGACY(_32x_dreq_common_r, _32x_dreq_common_w, 0xffffffff )

	AM_RANGE(0x00004014, 0x00004017) AM_READNOP AM_WRITE_LEGACY(_32x_sh2_master_4014_master_4016_w ) // IRQ clear
	AM_RANGE(0x00004018, 0x0000401b) AM_READNOP AM_WRITE_LEGACY(_32x_sh2_master_4018_master_401a_w ) // IRQ clear
	AM_RANGE(0x0000401c, 0x0000401f) AM_READNOP AM_WRITE_LEGACY(_32x_sh2_master_401c_master_401e_w ) // IRQ clear

	AM_RANGE(0x00004020, 0x0000402f) AM_READWRITE_LEGACY(_32x_sh2_commsram_r, _32x_sh2_commsram_w )
	AM_RANGE(0x00004030, 0x0000403f) AM_READWRITE16_LEGACY(_32x_pwm_r, _32x_pwm_w, 0xffffffff )

	AM_RANGE(0x00004100, 0x0000410b) AM_READWRITE16_LEGACY(_32x_common_vdp_regs_r, _32x_common_vdp_regs_w , 0xffffffff)
	AM_RANGE(0x00004200, 0x000043ff) AM_READWRITE_LEGACY(_32x_sh2_paletteram_r, _32x_sh2_paletteram_w)

	AM_RANGE(0x04000000, 0x0401ffff) AM_READWRITE_LEGACY(_32x_sh2_framebuffer_dram_r, _32x_sh2_framebuffer_dram_w)
	AM_RANGE(0x04020000, 0x0403ffff) AM_READWRITE_LEGACY(_32x_sh2_framebuffer_overwrite_dram_r, _32x_sh2_framebuffer_overwrite_dram_w)

	AM_RANGE(0x06000000, 0x0603ffff) AM_RAM AM_SHARE("share10")
	AM_RANGE(0x02000000, 0x023fffff) AM_ROM AM_REGION("gamecart_sh2", 0) // program is writeable (wwfraw)

	AM_RANGE(0x22000000, 0x223fffff) AM_ROM AM_REGION("gamecart_sh2", 0) // cart mirror (fifa96)

	AM_RANGE(0xc0000000, 0xc0000fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sh2_slave_map, AS_PROGRAM, 32, driver_device )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM

	AM_RANGE(0x00004000, 0x00004003) AM_READWRITE_LEGACY(_32x_sh2_slave_4000_common_4002_r, _32x_sh2_slave_4000_common_4002_w )
	AM_RANGE(0x00004004, 0x00004007) AM_READWRITE_LEGACY(_32x_sh2_common_4004_common_4006_r, _32x_sh2_common_4004_common_4006_w)

	AM_RANGE(0x00004008, 0x00004013) AM_READWRITE16_LEGACY(_32x_dreq_common_r, _32x_dreq_common_w, 0xffffffff )

	AM_RANGE(0x00004014, 0x00004017) AM_READNOP AM_WRITE_LEGACY(_32x_sh2_slave_4014_slave_4016_w ) // IRQ clear
	AM_RANGE(0x00004018, 0x0000401b) AM_READNOP AM_WRITE_LEGACY(_32x_sh2_slave_4018_slave_401a_w ) // IRQ clear
	AM_RANGE(0x0000401c, 0x0000401f) AM_READNOP AM_WRITE_LEGACY(_32x_sh2_slave_401c_slave_401e_w ) // IRQ clear

	AM_RANGE(0x00004020, 0x0000402f) AM_READWRITE_LEGACY(_32x_sh2_commsram_r, _32x_sh2_commsram_w )
	AM_RANGE(0x00004030, 0x0000403f) AM_READWRITE16_LEGACY(_32x_pwm_r, _32x_pwm_w, 0xffffffff )

	AM_RANGE(0x00004100, 0x0000410b) AM_READWRITE16_LEGACY(_32x_common_vdp_regs_r, _32x_common_vdp_regs_w , 0xffffffff)
	AM_RANGE(0x00004200, 0x000043ff) AM_READWRITE_LEGACY(_32x_sh2_paletteram_r, _32x_sh2_paletteram_w)

	AM_RANGE(0x04000000, 0x0401ffff) AM_READWRITE_LEGACY(_32x_sh2_framebuffer_dram_r, _32x_sh2_framebuffer_dram_w)
	AM_RANGE(0x04020000, 0x0403ffff) AM_READWRITE_LEGACY(_32x_sh2_framebuffer_overwrite_dram_r, _32x_sh2_framebuffer_overwrite_dram_w)

	AM_RANGE(0x06000000, 0x0603ffff) AM_RAM AM_SHARE("share10")
	AM_RANGE(0x02000000, 0x023fffff) AM_ROM AM_REGION("gamecart_sh2", 0) // program is writeable (wwfraw)

	AM_RANGE(0x22000000, 0x223fffff) AM_ROM AM_REGION("gamecart_sh2", 0) // cart mirror (fifa96)

	AM_RANGE(0xc0000000, 0xc0000fff) AM_RAM
ADDRESS_MAP_END

/****************************************** END 32X related *************************************/


/*************************************************************************************************
 Sega CD related
*************************************************************************************************/

// The perfect syncs should make this unnecessary: forcing syncs on reads is a flawed design pattern anyway,
// because the sync will only happen AFTER the read, by which time it's too late.
#define SEGACD_FORCE_SYNCS 0

static UINT8 segacd_ram_writeprotect_bits;
//int segacd_ram_mode;
//static int segacd_ram_mode_old;

//static int segacd_maincpu_has_ram_access = 0;
static int segacd_4meg_prgbank = 0; // which bank the MainCPU can see of the SubCPU PrgRAM
static int segacd_memory_priority_mode = 0;
static int segacd_stampsize;
//int segacd_dmna = 0;
//int segacd_ret = 0;

#define READ_MAIN (0x0200)
#define READ_SUB  (0x0300)
#define DMA_PCM  (0x0400)
#define DMA_PRG  (0x0500)
#define DMA_WRAM (0x0700)

#define REG_W_SBOUT  (0x0)
#define REG_W_IFCTRL (0x1)
#define REG_W_DBCL   (0x2)
#define REG_W_DBCH   (0x3)
#define REG_W_DACL   (0x4)
#define REG_W_DACH   (0x5)
#define REG_W_DTTRG  (0x6)
#define REG_W_DTACK  (0x7)
#define REG_W_WAL    (0x8)
#define REG_W_WAH    (0x9)
#define REG_W_CTRL0  (0xA)
#define REG_W_CTRL1  (0xB)
#define REG_W_PTL    (0xC)
#define REG_W_PTH    (0xD)
#define REG_W_CTRL2  (0xE)
#define REG_W_RESET  (0xF)

#define REG_R_COMIN  (0x0)
#define REG_R_IFSTAT (0x1)
#define REG_R_DBCL   (0x2)
#define REG_R_DBCH   (0x3)
#define REG_R_HEAD0  (0x4)
#define REG_R_HEAD1  (0x5)
#define REG_R_HEAD2  (0x6)
#define REG_R_HEAD3  (0x7)
#define REG_R_PTL    (0x8)
#define REG_R_PTH    (0x9)
#define REG_R_WAL    (0xa)
#define REG_R_WAH    (0xb)
#define REG_R_STAT0  (0xc)
#define REG_R_STAT1  (0xd)
#define REG_R_STAT2  (0xe)
#define REG_R_STAT3  (0xf)

#define CMD_STATUS   (0x0)
#define CMD_STOPALL  (0x1)
#define CMD_GETTOC   (0x2)
#define CMD_READ     (0x3)
#define CMD_SEEK     (0x4)
//                   (0x5)
#define CMD_STOP     (0x6)
#define CMD_RESUME   (0x7)
#define CMD_FF       (0x8)
#define CMD_RW       (0x9)
#define CMD_INIT     (0xa)
//                   (0xb)
#define CMD_CLOSE    (0xc)
#define CMD_OPEN     (0xd)
//                   (0xe)
//                   (0xf)


#define TOCCMD_CURPOS    (0x0)
#define TOCCMD_TRKPOS	 (0x1)
#define TOCCMD_CURTRK    (0x2)
#define TOCCMD_LENGTH    (0x3)
#define TOCCMD_FIRSTLAST (0x4)
#define TOCCMD_TRACKADDR (0x5)

struct segacd_t
{
	cdrom_file	*cd;
	const cdrom_toc   *toc;
	UINT32 current_frame;
};

segacd_t segacd;

#define SECTOR_SIZE (2352)

#define SET_CDD_DATA_MODE \
	CDD_CONTROL |= 0x0100; \

#define SET_CDD_AUDIO_MODE \
	CDD_CONTROL &= ~0x0100; \

#define STOP_CDC_READ \
	SCD_STATUS_CDC &= ~0x01; \

#define SET_CDC_READ \
	SCD_STATUS_CDC |= 0x01; \

#define SET_CDC_DMA \
	SCD_STATUS_CDC |= 0x08; \

#define STOP_CDC_DMA \
	SCD_STATUS_CDC &= ~0x08; \

#define SCD_READ_ENABLED \
	(SCD_STATUS_CDC & 1)

#define SCD_DMA_ENABLED \
	(SCD_STATUS_CDC & 0x08)

#define CLEAR_CDD_RESULT \
	CDD_MIN = CDD_SEC = CDD_FRAME = CDD_EXT = 0; \

#define CHECK_SCD_LV5_INTERRUPT \
	if (segacd_irq_mask & 0x20) \
	{ \
		cputag_set_input_line(machine, "segacd_68k", 5, HOLD_LINE); \
	} \

#define CHECK_SCD_LV4_INTERRUPT \
	if (segacd_irq_mask & 0x10) \
	{ \
		cputag_set_input_line(machine, "segacd_68k", 4, HOLD_LINE); \
	} \

#define CHECK_SCD_LV3_INTERRUPT \
	if (segacd_irq_mask & 0x08) \
	{ \
		cputag_set_input_line(machine, "segacd_68k", 3, HOLD_LINE); \
	} \

#define CHECK_SCD_LV2_INTERRUPT \
	if (segacd_irq_mask & 0x04) \
	{ \
		cputag_set_input_line(machine, "segacd_68k", 2, HOLD_LINE); \
	} \

#define CHECK_SCD_LV1_INTERRUPT \
	if (segacd_irq_mask & 0x02) \
	{ \
		cputag_set_input_line(machine, "segacd_68k", 1, HOLD_LINE); \
	} \

#define CURRENT_TRACK_IS_DATA \
	(segacd.toc->tracks[SCD_CURTRK - 1].trktype != CD_TRACK_AUDIO) \



INLINE int to_bcd(int val, bool byte)
{
	if (val > 99) val = 99;

	if (byte) return (((val) / 10) << 4) + ((val) % 10);
	else return (((val) / 10) << 8) + ((val) % 10);
}



UINT16* segacd_4meg_prgram;  // pointer to SubCPU PrgRAM
UINT16* segacd_dataram;

#define RAM_MODE_2MEG (0)
#define RAM_MODE_1MEG (2)



INLINE void write_pixel(running_machine& machine, UINT8 pix, int pixeloffset )
{

	int shift = 12-(4*(pixeloffset&0x3));
	UINT16 datamask = (0x000f) << shift;

	int offset = pixeloffset>>3;
	if (pixeloffset&4) offset++;

	offset &=0x1ffff;

	switch (segacd_memory_priority_mode)
	{
		case 0x00: // normal write, just write the data
			segacd_dataram[offset] = segacd_dataram[offset] &~ datamask;
			segacd_dataram[offset] |= pix << shift;
			break;

		case 0x01: // underwrite, only write if the existing data is 0
			if ((segacd_dataram[offset]&datamask) == 0x0000)
			{
				segacd_dataram[offset] = segacd_dataram[offset] &~ datamask;
				segacd_dataram[offset] |= pix << shift;
			}
			break;

		case 0x02: // overwrite, only write non-zero data
			if (pix)
			{
				segacd_dataram[offset] = segacd_dataram[offset] &~ datamask;
				segacd_dataram[offset] |= pix << shift;
			}
			break;

		default:
		case 0x03:
			pix = machine.rand() & 0x000f;
			segacd_dataram[offset] = segacd_dataram[offset] &~ datamask;
			segacd_dataram[offset] |= pix << shift;
			break;

	}
}

// 1meg / 2meg swap is interleaved swap, not half/half of the ram?
// Wily Beamish and Citizen X appear to rely on this
// however, it breaks the megacdj bios (megacd2j still works!)
//  (maybe that's a timing issue instead?)
UINT16 segacd_1meg_mode_word_read(int offset, UINT16 mem_mask)
{
	offset *= 2;

	if ((offset&0x20000))
		offset +=1;

	offset &=0x1ffff;

	return segacd_dataram[offset];
}


void segacd_1meg_mode_word_write(running_machine& machine, int offset, UINT16 data, UINT16 mem_mask, int use_pm)
{
	offset *= 2;

	if ((offset&0x20000))
		offset +=1;

	offset &=0x1ffff;

	if (use_pm)
	{
		// priority mode can apply when writing with the double up buffer mode
		// Jaguar XJ220 relies on this
		switch (segacd_memory_priority_mode)
		{
			case 0x00: // normal write, just write the data
				COMBINE_DATA(&segacd_dataram[offset]);
				break;

			case 0x01: // underwrite, only write if the existing data is 0
				if (ACCESSING_BITS_8_15)
				{
					if ((segacd_dataram[offset]&0xf000) == 0x0000) segacd_dataram[offset] |= (data)&0xf000;
					if ((segacd_dataram[offset]&0x0f00) == 0x0000) segacd_dataram[offset] |= (data)&0x0f00;
				}
				if (ACCESSING_BITS_0_7)
				{
					if ((segacd_dataram[offset]&0x00f0) == 0x0000) segacd_dataram[offset] |= (data)&0x00f0;
					if ((segacd_dataram[offset]&0x000f) == 0x0000) segacd_dataram[offset] |= (data)&0x000f;
				}
				break;

			case 0x02: // overwrite, only write non-zero data
				if (ACCESSING_BITS_8_15)
				{
					if ((data)&0xf000) segacd_dataram[offset] = (segacd_dataram[offset] & 0x0fff) | ((data)&0xf000);
					if ((data)&0x0f00) segacd_dataram[offset] = (segacd_dataram[offset] & 0xf0ff) | ((data)&0x0f00);
				}
				if (ACCESSING_BITS_0_7)
				{
					if ((data)&0x00f0) segacd_dataram[offset] = (segacd_dataram[offset] & 0xff0f) | ((data)&0x00f0);
					if ((data)&0x000f) segacd_dataram[offset] = (segacd_dataram[offset] & 0xfff0) | ((data)&0x000f);
				}
				break;

			default:
			case 0x03: // invalid?
				COMBINE_DATA(&segacd_dataram[offset]);
				break;

		}
	}
	else
	{
		COMBINE_DATA(&segacd_dataram[offset]);
	}
}


static UINT16* segacd_dataram2;

UINT8    SCD_BUFFER[2560];
UINT32   SCD_STATUS;
UINT32   SCD_STATUS_CDC;
INT32    SCD_CURLBA;
UINT8    SCD_CURTRK;

UINT16 CDC_DECODE;
INT16 CDC_DMACNT; // can go negative
UINT16 CDC_DMA_ADDRC;
UINT16 CDC_PT;
UINT16 CDC_WA;
UINT16 CDC_REG0;
UINT16 CDC_REG1;
UINT16 CDC_DMA_ADDR;
UINT16 CDC_IFSTAT;
UINT8 CDC_HEADB0;
UINT8 CDC_HEADB1;
UINT8 CDC_HEADB2;
UINT8 CDC_HEADB3;
UINT8 CDC_STATB0;
UINT8 CDC_STATB1;
UINT8 CDC_STATB2;
UINT8 CDC_STATB3;
UINT16 CDC_SBOUT;
UINT16 CDC_IFCTRL;
UINT8 CDC_CTRLB0;
UINT8 CDC_CTRLB1;
UINT8 CDC_CTRLB2;
UINT8 CDC_BUFFER[(32 * 1024 * 2) + SECTOR_SIZE];

UINT32 CDD_STATUS;
UINT32 CDD_MIN;
UINT32 CDD_SEC;

UINT8 CDD_RX[10];
UINT8 CDD_TX[10];
UINT32 CDD_FRAME;
UINT32 CDD_EXT;
UINT16 CDD_CONTROL;
INT16  CDD_DONE;

static void set_data_audio_mode(void)
{
	if (CURRENT_TRACK_IS_DATA)
	{
		SET_CDD_DATA_MODE
	}
	else
	{
		SET_CDD_AUDIO_MODE
		//fatalerror("CDDA unsupported\n");
	}
}


#define CDD_PLAYINGCDDA	0x0100
#define CDD_READY		0x0400
#define CDD_STOPPED		0x0900

void CDD_DoChecksum(void)
{
	int checksum =
		CDD_RX[0] +
		CDD_RX[1] +
		CDD_RX[2] +
		CDD_RX[3] +
		CDD_RX[4] +
		CDD_RX[5] +
		CDD_RX[6] +
		CDD_RX[7] +
		CDD_RX[9];

	checksum &= 0xf;
	checksum ^= 0xf;

	CDD_RX[8] = checksum;
}

void CDD_Export(void)
{
	CDD_RX[0] = (CDD_STATUS  & 0x00ff)>>0;
	CDD_RX[1] = (CDD_STATUS  & 0xff00)>>8;
	CDD_RX[2] = (CDD_MIN  & 0x00ff)>>0;
	CDD_RX[3] = (CDD_MIN  & 0xff00)>>8;
	CDD_RX[4] = (CDD_SEC & 0x00ff)>>0;
	CDD_RX[5] = (CDD_SEC & 0xff00)>>8;
	CDD_RX[6] = (CDD_FRAME   & 0x00ff)>>0;
	CDD_RX[7] = (CDD_FRAME   & 0xff00)>>8;
	/* 8 = checksum */
	CDD_RX[9] = (CDD_EXT     & 0x00ff)>>0;

	CDD_DoChecksum();

	CDD_CONTROL &= ~4; // Clear HOCK bit

}



void CDC_UpdateHEAD(void)
{
	if (CDC_CTRLB1 & 0x01)
	{
		CDC_HEADB0 = CDC_HEADB1 = CDC_HEADB2 = CDC_HEADB3 = 0x00;
	}
	else
	{
		UINT32 msf = lba_to_msf_alt(SCD_CURLBA+150);
		CDC_HEADB0 = to_bcd (((msf & 0x00ff0000)>>16), true);
		CDC_HEADB1 = to_bcd (((msf & 0x0000ff00)>>8), true);
		CDC_HEADB2 = to_bcd (((msf & 0x000000ff)>>0), true);
		CDC_HEADB3 = 0x01;
	}
}


void scd_ctrl_checks(running_machine& machine)
{
	CDC_STATB0 = 0x80;

	(CDC_CTRLB0 & 0x10) ? (CDC_STATB2 = CDC_CTRLB1 & 0x08) : (CDC_STATB2 = CDC_CTRLB1 & 0x0C);
	(CDC_CTRLB0 & 0x02) ? (CDC_STATB3 = 0x20) : (CDC_STATB3 = 0x00);

	if (CDC_IFCTRL & 0x20)
	{
		CHECK_SCD_LV5_INTERRUPT
		CDC_IFSTAT &= ~0x20;
		CDC_DECODE = 0;
	}
}

void scd_advance_current_readpos(void)
{
	SCD_CURLBA++;

	CDC_WA += SECTOR_SIZE;
	CDC_PT += SECTOR_SIZE;

	CDC_WA &= 0x7fff;
	CDC_PT &= 0x7fff;
}

int Read_LBA_To_Buffer(running_machine& machine)
{
	bool data_track = false;
	if (CDD_CONTROL & 0x0100) data_track = true;

	if (data_track)
		cdrom_read_data(segacd.cd, SCD_CURLBA, SCD_BUFFER, CD_TRACK_MODE1);

	CDC_UpdateHEAD();

	if (!data_track)
	{
		scd_advance_current_readpos();
	}

	if (CDC_CTRLB0 & 0x80)
	{
		if (CDC_CTRLB0 & 0x04)
		{
			if (data_track)
			{
				scd_advance_current_readpos();

				memcpy(&CDC_BUFFER[CDC_PT + 4], SCD_BUFFER, 2048);
				CDC_BUFFER[CDC_PT+0] = CDC_HEADB0;
				CDC_BUFFER[CDC_PT+1] = CDC_HEADB1;
				CDC_BUFFER[CDC_PT+2] = CDC_HEADB2;
				CDC_BUFFER[CDC_PT+3] = CDC_HEADB3;
			}
			else
			{
				memcpy(&CDC_BUFFER[CDC_PT], SCD_BUFFER, SECTOR_SIZE);
			}
		}

		scd_ctrl_checks(machine);
	}


	return 0;
}

static void CheckCommand(running_machine& machine)
{
	if (CDD_DONE)
	{
		CDD_DONE = 0;
		CDD_Export();
		CHECK_SCD_LV4_INTERRUPT
	}

	if (SCD_READ_ENABLED)
	{
		set_data_audio_mode();
		Read_LBA_To_Buffer(machine);
	}
}


void CDD_GetStatus(void)
{
	UINT16 s = (CDD_STATUS & 0x0f00);

	if ((s == 0x0200) || (s == 0x0700) || (s == 0x0e00))
		CDD_STATUS = (SCD_STATUS & 0xff00) | (CDD_STATUS & 0x00ff);
}


void CDD_Stop(running_machine &machine)
{
	CLEAR_CDD_RESULT
	STOP_CDC_READ
	SCD_STATUS = CDD_STOPPED;
	CDD_STATUS = 0x0000;
	SET_CDD_DATA_MODE
	cdda_stop_audio( machine.device( "cdda" ) ); //stop any pending CD-DA
}


void CDD_GetPos(void)
{
	CLEAR_CDD_RESULT
	UINT32 msf;
	CDD_STATUS &= 0xFF;
	if(segacd.cd == NULL) // no cd is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	msf = lba_to_msf_alt(SCD_CURLBA+150);
	CDD_MIN = to_bcd(((msf & 0x00ff0000)>>16),false);
	CDD_SEC = to_bcd(((msf & 0x0000ff00)>>8),false);
	CDD_FRAME = to_bcd(((msf & 0x000000ff)>>0),false);
}

void CDD_GetTrackPos(void)
{
	CLEAR_CDD_RESULT
	int elapsedlba;
	UINT32 msf;
	CDD_STATUS &= 0xFF;
	//  UINT32 end_msf = ;
	if(segacd.cd == NULL) // no cd is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	elapsedlba = SCD_CURLBA - segacd.toc->tracks[ cdrom_get_track(segacd.cd, SCD_CURLBA) ].physframeofs;
	msf = lba_to_msf_alt (elapsedlba);
	//popmessage("%08x %08x",SCD_CURLBA,segacd.toc->tracks[ cdrom_get_track(segacd.cd, SCD_CURLBA) + 1 ].physframeofs);
	CDD_MIN = to_bcd(((msf & 0x00ff0000)>>16),false);
	CDD_SEC = to_bcd(((msf & 0x0000ff00)>>8),false);
	CDD_FRAME = to_bcd(((msf & 0x000000ff)>>0),false);
}

void CDD_GetTrack(void)
{
	CLEAR_CDD_RESULT
	CDD_STATUS &= 0xFF;
	if(segacd.cd == NULL) // no cd is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	SCD_CURTRK = cdrom_get_track(segacd.cd, SCD_CURLBA)+1;
	CDD_MIN = to_bcd(SCD_CURTRK, false);
}

void CDD_Length(void)
{
	CLEAR_CDD_RESULT
	CDD_STATUS &= 0xFF;
	if(segacd.cd == NULL) // no cd is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;

	UINT32 startlba = (segacd.toc->tracks[cdrom_get_last_track(segacd.cd)].physframeofs);
	UINT32 startmsf = lba_to_msf_alt( startlba );

	CDD_MIN = to_bcd((startmsf&0x00ff0000)>>16,false);
	CDD_SEC = to_bcd((startmsf&0x0000ff00)>>8,false);
	CDD_FRAME = to_bcd((startmsf&0x000000ff)>>0,false);
}


void CDD_FirstLast(void)
{
	CLEAR_CDD_RESULT
	CDD_STATUS &= 0xFF;
	if(segacd.cd == NULL) // no cd is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;
	CDD_MIN = 1; // first
	CDD_SEC = to_bcd(cdrom_get_last_track(segacd.cd),false); // last
}

void CDD_GetTrackAdr(void)
{
	CLEAR_CDD_RESULT

	int track = (CDD_TX[4] & 0xF) + (CDD_TX[5] & 0xF) * 10;
	int last_track = cdrom_get_last_track(segacd.cd);

	CDD_STATUS &= 0xFF;
	if(segacd.cd == NULL) // no cd is there, bail out
		return;
	CDD_STATUS |= SCD_STATUS;

	if (track > last_track)
		track = last_track;

	if (track < 1)
		track = 1;

	UINT32 startlba = (segacd.toc->tracks[track-1].physframeofs);
	UINT32 startmsf = lba_to_msf_alt( startlba+150 );

	CDD_MIN = to_bcd((startmsf&0x00ff0000)>>16,false);
	CDD_SEC = to_bcd((startmsf&0x0000ff00)>>8,false);
	CDD_FRAME = to_bcd((startmsf&0x000000ff)>>0,false);
	CDD_EXT = track % 10;

	if (segacd.toc->tracks[track - 1].trktype != CD_TRACK_AUDIO)
		CDD_FRAME |= 0x0800;
}

static UINT32 getmsf_from_regs(void)
{
	UINT32 msf = 0;

	msf  = ((CDD_TX[2] & 0xF) + (CDD_TX[3] & 0xF) * 10) << 16;
	msf |= ((CDD_TX[4] & 0xF) + (CDD_TX[5] & 0xF) * 10) << 8;
	msf |= ((CDD_TX[6] & 0xF) + (CDD_TX[7] & 0xF) * 10) << 0;

	return msf;
}

void CDD_Play(running_machine &machine)
{
	CLEAR_CDD_RESULT
	UINT32 msf = getmsf_from_regs();
	SCD_CURLBA = msf_to_lba(msf)-150;
	UINT32 end_msf = segacd.toc->tracks[ cdrom_get_track(segacd.cd, SCD_CURLBA) + 1 ].physframeofs;
	SCD_CURTRK = cdrom_get_track(segacd.cd, SCD_CURLBA)+1;
	CDC_UpdateHEAD();
	SCD_STATUS = CDD_PLAYINGCDDA;
	CDD_STATUS = 0x0102;
	set_data_audio_mode();
	printf("%d Track played\n",SCD_CURTRK);
	CDD_MIN = to_bcd(SCD_CURTRK, false);
	if(!(CURRENT_TRACK_IS_DATA))
		cdda_start_audio( machine.device( "cdda" ), SCD_CURLBA, end_msf - SCD_CURLBA );
	SET_CDC_READ
}


void CDD_Seek(void)
{
	CLEAR_CDD_RESULT
	UINT32 msf = getmsf_from_regs();
	SCD_CURLBA = msf_to_lba(msf)-150;
	SCD_CURTRK = cdrom_get_track(segacd.cd, SCD_CURLBA)+1;
	CDC_UpdateHEAD();
	STOP_CDC_READ
	SCD_STATUS = CDD_READY;
	CDD_STATUS = 0x0200;
	set_data_audio_mode();
}


void CDD_Pause(running_machine &machine)
{
	CLEAR_CDD_RESULT
	STOP_CDC_READ
	SCD_STATUS = CDD_READY;
	CDD_STATUS = SCD_STATUS;
	SET_CDD_DATA_MODE

	//segacd.current_frame = cdda_get_audio_lba( machine.device( "cdda" ) );
	//if(!(CURRENT_TRACK_IS_DATA))
	cdda_pause_audio( machine.device( "cdda" ), 1 );
}

void CDD_Resume(running_machine &machine)
{
	CLEAR_CDD_RESULT
	STOP_CDC_READ
	SCD_CURTRK = cdrom_get_track(segacd.cd, SCD_CURLBA)+1;
	SCD_STATUS = CDD_PLAYINGCDDA;
	CDD_STATUS = 0x0102;
	set_data_audio_mode();
	CDD_MIN = to_bcd (SCD_CURTRK, false);
	SET_CDC_READ
	//if(!(CURRENT_TRACK_IS_DATA))
	cdda_pause_audio( machine.device( "cdda" ), 0 );
}


void CDD_FF(running_machine &machine)
{
	fatalerror("Fast Forward unsupported\n");
}


void CDD_RW(running_machine &machine)
{
	fatalerror("Fast Rewind unsupported\n");
}


void CDD_Open(void)
{
	fatalerror("Close Tray unsupported\n");
	/* TODO: re-read CD-ROM buffer here (Mega CD has multi disc games iirc?) */
}


void CDD_Close(void)
{
	fatalerror("Open Tray unsupported\n");
	/* TODO: clear CD-ROM buffer here */
}


void CDD_Init(void)
{
	CLEAR_CDD_RESULT
	STOP_CDC_READ
	SCD_STATUS = CDD_READY;
	CDD_STATUS = SCD_STATUS;
	CDD_SEC = 1;
	CDD_FRAME = 1;
}


void CDD_Default(void)
{
	CLEAR_CDD_RESULT
	CDD_STATUS = SCD_STATUS;
}


static void CDD_Reset(void)
{
	CLEAR_CDD_RESULT
	CDD_CONTROL = CDD_STATUS = 0;

	for (int i = 0; i < 10; i++)
		CDD_RX[i] = CDD_TX[i] = 0;

	CDD_DoChecksum();

	SCD_CURTRK = SCD_CURLBA = 0;
	SCD_STATUS = CDD_READY;
}

static void CDC_Reset(void)
{
	memset(CDC_BUFFER, 0x00, ((16 * 1024 * 2) + SECTOR_SIZE));
	CDC_UpdateHEAD();

	CDC_DMA_ADDRC = CDC_DMACNT = CDC_PT = CDC_SBOUT = CDC_IFCTRL = CDC_CTRLB0 = CDC_CTRLB1 =
		CDC_CTRLB2 = CDC_HEADB1 = CDC_HEADB2 = CDC_HEADB3 = CDC_STATB0 = CDC_STATB1 = CDC_STATB2 = CDC_DECODE = 0;

	CDC_IFSTAT = 0xFF;
	CDC_WA = SECTOR_SIZE * 2;
	CDC_HEADB0 = 0x01;
	CDC_STATB3 = 0x80;
}


void lc89510_Reset(void)
{
	CDD_Reset();
	CDC_Reset();

	CDC_REG0 = CDC_REG1 = CDC_DMA_ADDR = SCD_STATUS_CDC = CDD_DONE = 0;
}

void CDC_End_Transfer(running_machine& machine)
{
	STOP_CDC_DMA
	CDC_REG0 |= 0x8000;
	CDC_REG0 &= ~0x4000;
	CDC_IFSTAT |= 0x08;

	if (CDC_IFCTRL & 0x40)
	{
		CDC_IFSTAT &= ~0x40;
		CHECK_SCD_LV5_INTERRUPT
	}
}

void CDC_Do_DMA(running_machine& machine, int rate)
{
	address_space* space = machine.device("segacd_68k")->memory().space(AS_PROGRAM);

	UINT32 dstoffset, length;
	UINT8 *dest;
	UINT16 destination = CDC_REG0 & 0x0700;

	if (!(SCD_DMA_ENABLED))
		return;

	if ((destination == READ_MAIN) || (destination==READ_SUB))
	{
		CDC_REG0 |= 0x4000;
		return;
	}

	if (CDC_DMACNT <= (rate * 2))
	{
		length = (CDC_DMACNT + 1) >> 1;
		CDC_End_Transfer(machine);
	}
	else
		length = rate;


	int dmacount = length;

	bool PCM_DMA = false;

	if (destination==DMA_PCM)
	{
		dstoffset = (CDC_DMA_ADDR & 0x03FF) << 2;
		PCM_DMA = true;
	}
	else
	{
		dstoffset = (CDC_DMA_ADDR & 0xFFFF) << 3;
	}

	int srcoffset = 0;

	while (dmacount--)
	{
		UINT16 data = (CDC_BUFFER[CDC_DMA_ADDRC+srcoffset]<<8) | CDC_BUFFER[CDC_DMA_ADDRC+srcoffset+1];

		if (destination==DMA_PRG)
		{
			dest = (UINT8 *) segacd_4meg_prgram;
		}
		else if (destination==DMA_WRAM)
		{
			dest = (UINT8*)segacd_dataram;
		}
		else if (destination==DMA_PCM)
		{
			dest = 0;//fatalerror("PCM RAM DMA unimplemented!\n");
		}
		else
		{
			fatalerror("Unknown DMA Destination!!\n");
		}

		if (PCM_DMA)
		{
			space->write_byte(0xff2000+(((dstoffset*2)+1)&0x1fff),data >> 8);
			space->write_byte(0xff2000+(((dstoffset*2)+3)&0x1fff),data & 0xff);
		//  printf("PCM_DMA writing %04x %04x\n",0xff2000+(dstoffset*2), data);
		}
		else
		{
			if (dest)
			{
				if (destination==DMA_WRAM)
				{

					if ((scd_rammode&2)==RAM_MODE_2MEG)
					{
						dstoffset &= 0x3ffff;

						dest[dstoffset+1] = data >>8;
						dest[dstoffset+0] = data&0xff;

						segacd_mark_tiles_dirty(space->machine(), dstoffset/2);
					}
					else
					{
						dstoffset &= 0x1ffff;

						if (!(scd_rammode & 1))
						{
							segacd_1meg_mode_word_write(space->machine(),(dstoffset+0x20000)/2, data, 0xffff, 0);
						}
						else
						{
							segacd_1meg_mode_word_write(space->machine(),(dstoffset+0x00000)/2, data, 0xffff, 0);
						}
					}

				}
				else
				{
					// main ram
					dest[dstoffset+1] = data >>8;
					dest[dstoffset+0] = data&0xff;
				}

			}
		}

		srcoffset += 2;
		dstoffset += 2;
	}

	if (PCM_DMA)
	{
		CDC_DMA_ADDR += length >> 1;
	}
	else
	{
		CDC_DMA_ADDR += length >> 2;
	}

	CDC_DMA_ADDRC += length*2;

	if (SCD_DMA_ENABLED)
		CDC_DMACNT -= length*2;
	else
		CDC_DMACNT = 0;
}




UINT16 CDC_Host_r(running_machine& machine, UINT16 type)
{
	UINT16 destination = CDC_REG0 & 0x0700;

	if (SCD_DMA_ENABLED)
	{
		if (destination == type)
		{
			CDC_DMACNT -= 2;

			if (CDC_DMACNT <= 0)
			{
				if (type==READ_SUB) CDC_DMACNT = 0;

				CDC_End_Transfer(machine);
			}

			UINT16 data = (CDC_BUFFER[CDC_DMA_ADDRC]<<8) | CDC_BUFFER[CDC_DMA_ADDRC+1];
			CDC_DMA_ADDRC += 2;

			return data;
		}
	}

	return 0;
}


UINT8 CDC_Reg_r(void)
{
	int reg = CDC_REG0 & 0xF;
	UINT8 ret = 0;
	UINT16 decoderegs = 0x73F2;

	if ((decoderegs>>reg)&1)
		CDC_DECODE |= (1 << reg);

	//if (reg!=REG_R_STAT3)
		CDC_REG0 = (CDC_REG0 & 0xFFF0) | ((reg+1)&0xf);


	switch (reg)
	{
		case REG_R_COMIN:  ret = 0/*COMIN*/;            break;
		case REG_R_IFSTAT: ret = CDC_IFSTAT;           break;
		case REG_R_DBCL:   ret = CDC_DMACNT & 0xff;       break;
		case REG_R_DBCH:   ret = (CDC_DMACNT >>8) & 0xff; break;
		case REG_R_HEAD0:  ret = CDC_HEADB0;           break;
		case REG_R_HEAD1:  ret = CDC_HEADB1;           break;
		case REG_R_HEAD2:  ret = CDC_HEADB2;           break;
		case REG_R_HEAD3:  ret = CDC_HEADB3;           break;
		case REG_R_PTL:	   ret = CDC_PT & 0xff;        break;
		case REG_R_PTH:	   ret = (CDC_PT >>8) & 0xff;  break;
		case REG_R_WAL:    ret = CDC_WA & 0xff;        break;
		case REG_R_WAH:    ret = (CDC_WA >>8) & 0xff;  break;
		case REG_R_STAT0:  ret = CDC_STATB0;           break;
		case REG_R_STAT1:  ret = CDC_STATB1;           break;
		case REG_R_STAT2:  ret = CDC_STATB2;           break;
		case REG_R_STAT3:  ret = CDC_STATB3;

			CDC_IFSTAT |= 0x20;

			// ??
			if ((CDC_CTRLB0 & 0x80) && (CDC_IFCTRL & 0x20))
			{
				if ((CDC_DECODE & decoderegs) == decoderegs)
				CDC_STATB3 = 0x80;
			}
			break;
	}

	return ret;
}

void CDC_Reg_w(UINT8 data)
{
	int reg = CDC_REG0 & 0xF;

	int changers0[0x10] = { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0};

	if (changers0[reg])
		CDC_REG0 = (CDC_REG0 & 0xFFF0) | (reg+1);

	switch (reg)
	{
	case REG_W_SBOUT:
			CDC_SBOUT = data;
			break;

	case REG_W_IFCTRL:
			CDC_IFCTRL = data;

			if (!(CDC_IFCTRL & 0x02))
			{
				CDC_DMACNT = 0;
				STOP_CDC_DMA;
				CDC_IFSTAT |= 0x08;
			}
			break;

	case REG_W_DBCL: CDC_DMACNT = (CDC_DMACNT &~ 0x00ff) | (data & 0x00ff) << 0; break;
	case REG_W_DBCH: CDC_DMACNT = (CDC_DMACNT &~ 0xff00) | (data & 0x00ff) << 8; break;
	case REG_W_DACL: CDC_DMA_ADDRC = (CDC_DMA_ADDRC &~ 0x00ff) | (data & 0x00ff) << 0; break;
	case REG_W_DACH: CDC_DMA_ADDRC = (CDC_DMA_ADDRC &~ 0xff00) | (data & 0x00ff) << 8; break;

	case REG_W_DTTRG:
			if (CDC_IFCTRL & 0x02)
			{
				CDC_IFSTAT &= ~0x08;
				SET_CDC_DMA;
				CDC_REG0 &= ~0x8000;
			}
			break;

	case REG_W_DTACK: CDC_IFSTAT |= 0x40; break;
	case REG_W_WAL: CDC_WA = (CDC_WA &~ 0x00ff) | (data & 0x00ff) << 0; break;
	case REG_W_WAH:	CDC_WA = (CDC_WA &~ 0xff00) | (data & 0x00ff) << 8;	break;
	case REG_W_CTRL0: CDC_CTRLB0 = data; break;
	case REG_W_CTRL1: CDC_CTRLB1 = data; break;
	case REG_W_PTL: CDC_PT = (CDC_PT &~ 0x00ff) | (data & 0x00ff) << 0; break;
	case REG_W_PTH: CDC_PT = (CDC_PT &~ 0xff00) | (data & 0x00ff) << 8;	break;
	case REG_W_CTRL2: CDC_CTRLB2 = data; break;
	case REG_W_RESET: CDC_Reset();       break;
	}
}

void CDD_Process(running_machine& machine, int reason)
{
	CDD_Export();
	CHECK_SCD_LV4_INTERRUPT
}

void CDD_Handle_TOC_Commands(void)
{
	int subcmd = CDD_TX[2];
	CDD_STATUS = (CDD_STATUS & 0xFF00) | subcmd;

	switch (subcmd)
	{
		case TOCCMD_CURPOS:	   CDD_GetPos();	  break;
		case TOCCMD_TRKPOS:	   CDD_GetTrackPos(); break;
		case TOCCMD_CURTRK:    CDD_GetTrack();   break;
		case TOCCMD_LENGTH:    CDD_Length();      break;
		case TOCCMD_FIRSTLAST: CDD_FirstLast();   break;
		case TOCCMD_TRACKADDR: CDD_GetTrackAdr(); break;
		default:               CDD_GetStatus();   break;
	}
}

static const char *const CDD_import_cmdnames[] =
{
	"Get Status",			// 0
	"Stop ALL",				// 1
	"Handle TOC",			// 2
	"Play",					// 3
	"Seek",					// 4
	"<undefined>",			// 5
	"Pause",				// 6
	"Resume",				// 7
	"FF",					// 8
	"RWD",					// 9
	"INIT",					// A
	"<undefined>",			// B
	"Close Tray",			// C
	"Open Tray",			// D
	"<undefined>",			// E
	"<undefined>"			// F
};

void CDD_Import(running_machine& machine)
{
	if(CDD_TX[1] != 2 && CDD_TX[1] != 0)
		printf("%s\n",CDD_import_cmdnames[CDD_TX[1]]);

	switch (CDD_TX[1])
	{
		case CMD_STATUS:	CDD_GetStatus();	       break;
		case CMD_STOPALL:	CDD_Stop(machine);		   break;
		case CMD_GETTOC:	CDD_Handle_TOC_Commands(); break;
		case CMD_READ:		CDD_Play(machine);         break;
		case CMD_SEEK:		CDD_Seek();	               break;
		case CMD_STOP:		CDD_Pause(machine);	       break;
		case CMD_RESUME:	CDD_Resume(machine);       break;
		case CMD_FF:		CDD_FF(machine);           break;
		case CMD_RW:		CDD_RW(machine);           break;
		case CMD_INIT:		CDD_Init();	               break;
		case CMD_CLOSE:		CDD_Open();                break;
		case CMD_OPEN:		CDD_Close();	           break;
		default:			CDD_Default();	           break;
	}

	CDD_DONE = 1;
}





static UINT16 segacd_hint_register;
static UINT16 segacd_imagebuffer_vdot_size;
static UINT16 segacd_imagebuffer_vcell_size;
static UINT16 segacd_imagebuffer_hdot_size;

static UINT16 a12000_halt_reset_reg = 0x0000;
int segacd_conversion_active = 0;
static UINT16 segacd_stampmap_base_address;
static UINT16 segacd_imagebuffer_start_address;
static UINT16 segacd_imagebuffer_offset;
static tilemap_t    *segacd_stampmap[4];
//static void segacd_mark_stampmaps_dirty(void);



static WRITE16_HANDLER( scd_a12000_halt_reset_w )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();

	UINT16 old_halt = a12000_halt_reset_reg;

	COMBINE_DATA(&a12000_halt_reset_reg);

	if (ACCESSING_BITS_0_7)
	{
		// reset line
		if (a12000_halt_reset_reg&0x0001)
		{
			cputag_set_input_line(space->machine(), "segacd_68k", INPUT_LINE_RESET, CLEAR_LINE);
			if (!(old_halt&0x0001)) printf("clear reset slave\n");
		}
		else
		{
			cputag_set_input_line(space->machine(), "segacd_68k", INPUT_LINE_RESET, ASSERT_LINE);
			if ((old_halt&0x0001)) printf("assert reset slave\n");
		}

		// request BUS
		if (a12000_halt_reset_reg&0x0002)
		{
			cputag_set_input_line(space->machine(), "segacd_68k", INPUT_LINE_HALT, ASSERT_LINE);
			if (!(old_halt&0x0002)) printf("halt slave\n");
		}
		else
		{
			cputag_set_input_line(space->machine(), "segacd_68k", INPUT_LINE_HALT, CLEAR_LINE);
			if ((old_halt&0x0002)) printf("resume slave\n");
		}
	}

	if (ACCESSING_BITS_8_15)
	{
		if (a12000_halt_reset_reg&0x0100)
		{
			running_machine& machine = space->machine();
			CHECK_SCD_LV2_INTERRUPT
		}

		if (a12000_halt_reset_reg&0x8000)
		{
			// not writable.. but can read irq mask here?
			//printf("a12000_halt_reset_reg & 0x8000 set\n"); // irq2 mask?
		}


	}
}

static READ16_HANDLER( scd_a12000_halt_reset_r )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();

	return a12000_halt_reset_reg;
}


/********************************************************************************
 MEMORY MODE CONTROL
  - main / sub sides differ!
********************************************************************************/

//
// we might need a delay on the segacd_maincpu_has_ram_access registers, as they actually indicate requests being made
// so probably don't change instantly...
//


static READ16_HANDLER( scd_a12002_memory_mode_r )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();

	int temp = scd_rammode;
	int temp2 = 0;

	temp2 |= (scd_mode_dmna_ret_flags>>(temp*4))&0x7;

	return (segacd_ram_writeprotect_bits << 8) |
		   (segacd_4meg_prgbank << 6) |
			temp2;

}


/* I'm still not 100% clear how this works, the sources I have are a bit vague,
   it might still be incorrect in both modes

  for a simple way to swap blocks of ram between cpus this is stupidly convoluted

 */

// DMNA = Decleration Mainram No Access (bit 0)
// RET = Return access (bit 1)


static WRITE8_HANDLER( scd_a12002_memory_mode_w_8_15 )
{
	if (data & 0xff00)
	{
		printf("write protect bits set %02x\n", data);
	}

	segacd_ram_writeprotect_bits = data;
}


static WRITE8_HANDLER( scd_a12002_memory_mode_w_0_7 )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();


	//printf("scd_a12002_memory_mode_w_0_7 %04x\n",data);

	segacd_4meg_prgbank = (data&0x00c0)>>6;

	if (scd_rammode&0x2)
	{ // ==0x2 (1 meg mode)
		if (!(data&2)) // check DMNA bit
		{
			scd_mode_dmna_ret_flags |= 0x2200;
		}
	}
	else // == 0x0 (2 meg mode)
	{
		if (data&2) // check DMNA bit
		{
			scd_rammode = 1;
		}
	}
}


static WRITE16_HANDLER( scd_a12002_memory_mode_w )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();

	if (ACCESSING_BITS_8_15)
		scd_a12002_memory_mode_w_8_15(space, 0, data>>8);

	if (ACCESSING_BITS_0_7)
		scd_a12002_memory_mode_w_0_7(space, 0, data&0xff);
}




static READ16_HANDLER( segacd_sub_memory_mode_r )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();

	int temp = scd_rammode;
	int temp2 = 0;

	temp2 |= (scd_mode_dmna_ret_flags>>(temp*4))&0x7;

	return (segacd_ram_writeprotect_bits << 8) |
		   (segacd_memory_priority_mode << 3) |
			temp2;
}


WRITE8_HANDLER( segacd_sub_memory_mode_w_8_15 )
{
	/* setting write protect bits from sub-cpu has no effect? */
}



WRITE8_HANDLER( segacd_sub_memory_mode_w_0_7 )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();


	segacd_memory_priority_mode = (data&0x0018)>>3;

	// If the mode bit is 0 then we're requesting a change to
	// 2Meg mode?

	//printf("segacd_sub_memory_mode_w_0_7 %04x\n",data);

	if (!(data&4)) // check ram mode bit
	{	// == 0x0 - 2 meg mode
		scd_mode_dmna_ret_flags &= 0xddff;

		if (data&1) // check RET
		{
			// If RET is set and the Mode bit in the write is set to 2M then we want to change to 2M mode
			// If we're already in 2M mode it has no effect
			scd_rammode = 0;

		}
		else
		{
			// == 0x4 - 1 meg mode

			int temp = scd_rammode;
			if (temp&2) // check ram mode
			{ // == 0x2 - 1 meg mode
				scd_mode_dmna_ret_flags &= 0xffde;
				scd_rammode = temp &1;
			}
		}
	}
	else
	{	// == 0x4 - 1 meg mode
		data &=1;
		int temp = data;
		int scd_rammode_old = scd_rammode;
		data |=2;

		temp ^= scd_rammode_old;
		scd_rammode = data;

		if (scd_rammode_old & 2)
		{ // == 0x2 - already in 1 meg mode
			if (temp & 1) // ret bit
			{
				scd_mode_dmna_ret_flags &= 0xddff;
			}
		}
		else
		{ // == 0x0 - currently in 2 meg mode
			scd_mode_dmna_ret_flags &= 0xddff;
		}
	}
}

static WRITE16_HANDLER( segacd_sub_memory_mode_w )
{
	//printf("segacd_sub_memory_mode_w %04x %04x\n", data, mem_mask);
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();

	if (ACCESSING_BITS_8_15)
		segacd_sub_memory_mode_w_8_15(space, 0, data>>8);

	if (ACCESSING_BITS_0_7)
		segacd_sub_memory_mode_w_0_7(space, 0, data&0xff);
}


/********************************************************************************
 END MEMORY MODE CONTROL
********************************************************************************/

/********************************************************************************
 COMMUNICATION FLAGS
  - main / sub sides differ in which bits are write only
********************************************************************************/

static UINT16 segacd_comms_flags = 0x0000;

static READ16_HANDLER( segacd_comms_flags_r )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();
	return segacd_comms_flags;
}

static WRITE16_HANDLER( segacd_comms_flags_subcpu_w )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();

	if (ACCESSING_BITS_8_15) // Dragon's Lair
	{
		segacd_comms_flags = (segacd_comms_flags & 0xff00) | ((data >> 8) & 0x00ff);
	}

	// flashback needs low bits to take priority in word writes
	if (ACCESSING_BITS_0_7)
	{
		segacd_comms_flags = (segacd_comms_flags & 0xff00) | (data & 0x00ff);
	}
}

static WRITE16_HANDLER( segacd_comms_flags_maincpu_w )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();

	if (ACCESSING_BITS_8_15)
	{
		segacd_comms_flags = (segacd_comms_flags & 0x00ff) | (data & 0xff00);
	}

	// flashback needs low bits to take priority in word writes
	if (ACCESSING_BITS_0_7)
	{
		segacd_comms_flags = (segacd_comms_flags & 0x00ff) | ((data << 8) & 0xff00);
	}
}

static READ16_HANDLER( scd_4m_prgbank_ram_r )
{
	UINT16 realoffset = ((segacd_4meg_prgbank * 0x20000)/2) + offset;
	return segacd_4meg_prgram[realoffset];

}

static WRITE16_HANDLER( scd_4m_prgbank_ram_w )
{
	UINT16 realoffset = ((segacd_4meg_prgbank * 0x20000)/2) + offset;

	// todo:
	// check for write protection? (or does that only apply to writes on the SubCPU side?

	COMBINE_DATA(&segacd_4meg_prgram[realoffset]);

}


/* Callback when the genesis enters interrupt code */
static IRQ_CALLBACK(segacd_sub_int_callback)
{
	if (irqline==2)
	{
		// clear this bit
		a12000_halt_reset_reg &= ~0x0100;
		cputag_set_input_line(device->machine(), "segacd_68k", 2, CLEAR_LINE);
	}

	return (0x60+irqline*4)/4; // vector address
}

UINT16 segacd_comms_part1[0x8];
UINT16 segacd_comms_part2[0x8];

static READ16_HANDLER( segacd_comms_main_part1_r )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();
	return segacd_comms_part1[offset];
}

static WRITE16_HANDLER( segacd_comms_main_part1_w )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();
	COMBINE_DATA(&segacd_comms_part1[offset]);
}

static READ16_HANDLER( segacd_comms_main_part2_r )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();
	return segacd_comms_part2[offset];
}

static WRITE16_HANDLER( segacd_comms_main_part2_w )
{
	printf("Sega CD main CPU attempting to write to read only comms regs\n");
}


static READ16_HANDLER( segacd_comms_sub_part1_r )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();
	return segacd_comms_part1[offset];
}

static WRITE16_HANDLER( segacd_comms_sub_part1_w )
{
	printf("Sega CD sub CPU attempting to write to read only comms regs\n");
}

static READ16_HANDLER( segacd_comms_sub_part2_r )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();
	return segacd_comms_part2[offset];
}

static WRITE16_HANDLER( segacd_comms_sub_part2_w )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();
	COMBINE_DATA(&segacd_comms_part2[offset]);
}

/**************************************************************
 CDC Stuff ********
**************************************************************/



static WRITE16_HANDLER( segacd_cdc_mode_address_w )
{
	COMBINE_DATA(&CDC_REG0);
}

static READ16_HANDLER( segacd_cdc_mode_address_r )
{
	return CDC_REG0;
}

static WRITE16_HANDLER( segacd_cdc_data_w )
{
	COMBINE_DATA(&CDC_REG1);

	if (ACCESSING_BITS_0_7)
		CDC_Reg_w(data);
}

static READ16_HANDLER( segacd_cdc_data_r )
{
	UINT16 retdat = 0x0000;

	if (ACCESSING_BITS_0_7)
		retdat |= CDC_Reg_r();

	return retdat;
}





static READ16_HANDLER( segacd_main_dataram_part1_r )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		// is this correct?
		if (!(scd_rammode&1))
		{
			//printf("segacd_main_dataram_part1_r in mode 0 %08x %04x\n", offset*2, segacd_dataram[offset]);

			return segacd_dataram[offset];

		}
		else
		{
			printf("Illegal: segacd_main_dataram_part1_r in mode 0 without permission\n");
			return 0xffff;
		}

	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{

		if (offset<0x20000/2)
		{
			// wordram accees
			//printf("Unspported: segacd_main_dataram_part1_r (word RAM) in mode 1\n");

			// ret bit set by sub cpu determines which half of WorkRAM we have access to?
			if (scd_rammode&1)
			{
				return segacd_1meg_mode_word_read(offset+0x20000/2, mem_mask);
			}
			else
			{
				return segacd_1meg_mode_word_read(offset+0x00000/2, mem_mask);
			}

		}
		else
		{
			// converts data stored in bitmap format (in dataram) to be read out as tiles (for dma->vram purposes)
			// used by Heart of the Alien

			if(offset<0x30000/2)		/* 0x20000 - 0x2ffff */ // 512x256 bitmap. tiles
				offset = BITSWAP24(offset,23,22,21,20,19,18,17,16,15,8,7,6,5,4,3,2,1,14,13,12,11,10,9,0);
			else if(offset<0x38000/2)	/* 0x30000 - 0x37fff */  // 512x128 bitmap. tiles
				offset = BITSWAP24(offset,23,22,21,20,19,18,17,16,15,14,7,6,5,4,3,2,1,13,12,11,10,9,8,0);
			else if(offset<0x3c000/2)	/* 0x38000 - 0x3bfff */  // 512x64 bitmap. tiles
				offset = BITSWAP24(offset,23,22,21,20,19,18,17,16,15,14,13,6,5,4,3,2,1,12,11,10,9,8,7,0);
			else  /* 0x3c000 - 0x3dfff and 0x3e000 - 0x3ffff */  // 512x32 bitmap (x2) -> tiles
				offset = BITSWAP24(offset,23,22,21,20,19,18,17,16,15,14,13,12,5,4,3,2,1,11,10,9,8,7,6,0);

			offset &=0xffff;
			// HOTA cares about this
			if (!(scd_rammode&1))
			{
				return segacd_1meg_mode_word_read(offset+0x00000/2, mem_mask);
			}
			else
			{
				return segacd_1meg_mode_word_read(offset+0x20000/2, mem_mask);
			}
		}
	}

	return 0x0000;
}

static WRITE16_HANDLER( segacd_main_dataram_part1_w )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		// is this correct?
		if (!(scd_rammode&1))
		{
			COMBINE_DATA(&segacd_dataram[offset]);
			segacd_mark_tiles_dirty(space->machine(), offset);
		}
		else
		{
			printf("Illegal: segacd_main_dataram_part1_w in mode 0 without permission\n");
		}

	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		if (offset<0x20000/2)
		{
			//printf("Unspported: segacd_main_dataram_part1_w (word RAM) in mode 1\n");
			// wordram accees

			// ret bit set by sub cpu determines which half of WorkRAM we have access to?
			if (scd_rammode&1)
			{
				segacd_1meg_mode_word_write(space->machine(), offset+0x20000/2, data, mem_mask, 0);
			}
			else
			{
				segacd_1meg_mode_word_write(space->machine(), offset+0x00000/2, data, mem_mask, 0);
			}
		}
		else
		{
		//  printf("Unspported: segacd_main_dataram_part1_w (Cell rearranged RAM) in mode 1 (illega?)\n"); // is this legal??
		}
	}
}

static READ16_HANDLER( scd_hint_vector_r )
{
//  printf("read HINT offset %d\n", offset);

	switch (offset&1)
	{
		case 0x00:
			//return 0x00ff; // doesn't make much sense..
			return 0xffff;
		case 0x01:
			return segacd_hint_register;
	}

	return 0;

}

static READ16_HANDLER( scd_a12006_hint_register_r )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();
	return segacd_hint_register;
}

static WRITE16_HANDLER( scd_a12006_hint_register_w )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();
	COMBINE_DATA(&segacd_hint_register);
}


static TIMER_CALLBACK( segacd_gfx_conversion_timer_callback )
{
	//printf("segacd_gfx_conversion_timer_callback\n");

	CHECK_SCD_LV1_INTERRUPT

	segacd_conversion_active = 0;

	// this ends up as 0 after processing (soniccd bonus stage)
	segacd_imagebuffer_vdot_size = 0;

}


// the tiles in RAM are 8x8 tiles
// they are referenced in the cell look-up map as either 16x16 or 32x32 tiles (made of 4 / 16 8x8 tiles)

#define SEGACD_BYTES_PER_TILE16 (128)
#define SEGACD_BYTES_PER_TILE32 (512)

#define SEGACD_NUM_TILES16 (0x40000/SEGACD_BYTES_PER_TILE16)
#define SEGACD_NUM_TILES32 (0x40000/SEGACD_BYTES_PER_TILE32)

/*
static const gfx_layout sega_8x8_layout =
{
    8,8,
    SEGACD_NUM_TILES16,
    4,
    { 0,1,2,3 },
    { 8,12,0,4,24,28,16,20 },
    { 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
    8*32
};
*/

/* also create pre-rotated versions.. - it might still be possible to use these decodes with our own copying routines */


#define _16x16_SEQUENCE_1  { 8,12,0,4,24,28,16,20, 512+8, 512+12, 512+0, 512+4, 512+24, 512+28, 512+16, 512+20 },
#define _16x16_SEQUENCE_1_FLIP  { 512+20,512+16,512+28,512+24,512+4,512+0, 512+12,512+8, 20,16,28,24,4,0,12,8 },

#define _16x16_SEQUENCE_2  { 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32,10*32,11*32,12*32,13*32,14*32,15*32 },
#define _16x16_SEQUENCE_2_FLIP  { 15*32, 14*32, 13*32, 12*32, 11*32, 10*32, 9*32, 8*32, 7*32, 6*32, 5*32, 4*32, 3*32, 2*32, 1*32, 0*32 },


#define _16x16_START \
{ \
	16,16, \
	SEGACD_NUM_TILES16, \
	4, \
	{ 0,1,2,3 }, \

#define _16x16_END \
		8*128 \
}; \

#define _32x32_START \
{ \
	32,32, \
	SEGACD_NUM_TILES32, \
	4, \
	{ 0,1,2,3 }, \


#define _32x32_END \
	8*512 \
}; \



#define _32x32_SEQUENCE_1 \
	{ 8,12,0,4,24,28,16,20, \
	1024+8, 1024+12, 1024+0, 1024+4, 1024+24, 1024+28, 1024+16, 1024+20, \
	2048+8, 2048+12, 2048+0, 2048+4, 2048+24, 2048+28, 2048+16, 2048+20, \
	3072+8, 3072+12, 3072+0, 3072+4, 3072+24, 3072+28, 3072+16, 3072+20  \
	}, \

#define _32x32_SEQUENCE_1_FLIP \
{ 3072+20, 3072+16, 3072+28, 3072+24, 3072+4, 3072+0, 3072+12, 3072+8, \
  2048+20, 2048+16, 2048+28, 2048+24, 2048+4, 2048+0, 2048+12, 2048+8, \
  1024+20, 1024+16, 1024+28, 1024+24, 1024+4, 1024+0, 1024+12, 1024+8, \
  20, 16, 28, 24, 4, 0, 12, 8}, \


#define _32x32_SEQUENCE_2 \
		{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, \
    	8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32, \
	 16*32,17*32,18*32,19*32,20*32,21*32,22*32,23*32, \
	 24*32,25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32}, \

#define _32x32_SEQUENCE_2_FLIP \
{ 31*32, 30*32, 29*32, 28*32, 27*32, 26*32, 25*32, 24*32, \
  23*32, 22*32, 21*32, 20*32, 19*32, 18*32, 17*32, 16*32, \
  15*32, 14*32, 13*32, 12*32, 11*32, 10*32, 9*32 , 8*32 , \
  7*32 , 6*32 , 5*32 , 4*32 , 3*32 , 2*32 , 1*32 , 0*32}, \


/* 16x16 decodes */
static const gfx_layout sega_16x16_r00_f0_layout =
_16x16_START
	_16x16_SEQUENCE_1
	_16x16_SEQUENCE_2
_16x16_END

static const gfx_layout sega_16x16_r01_f0_layout =
_16x16_START
	_16x16_SEQUENCE_2
	_16x16_SEQUENCE_1_FLIP
_16x16_END

static const gfx_layout sega_16x16_r10_f0_layout =
_16x16_START
	_16x16_SEQUENCE_1_FLIP
	_16x16_SEQUENCE_2_FLIP
_16x16_END

static const gfx_layout sega_16x16_r11_f0_layout =
_16x16_START
	_16x16_SEQUENCE_2_FLIP
	_16x16_SEQUENCE_1
_16x16_END

static const gfx_layout sega_16x16_r00_f1_layout =
_16x16_START
	_16x16_SEQUENCE_1_FLIP
	_16x16_SEQUENCE_2
_16x16_END

static const gfx_layout sega_16x16_r01_f1_layout =
_16x16_START
	_16x16_SEQUENCE_2
	_16x16_SEQUENCE_1
_16x16_END

static const gfx_layout sega_16x16_r10_f1_layout =
_16x16_START
	_16x16_SEQUENCE_1
	_16x16_SEQUENCE_2_FLIP
_16x16_END

static const gfx_layout sega_16x16_r11_f1_layout =
_16x16_START
	_16x16_SEQUENCE_2_FLIP
	_16x16_SEQUENCE_1_FLIP
_16x16_END

/* 32x32 decodes */
static const gfx_layout sega_32x32_r00_f0_layout =
_32x32_START
	_32x32_SEQUENCE_1
	_32x32_SEQUENCE_2
_32x32_END

static const gfx_layout sega_32x32_r01_f0_layout =
_32x32_START
	_32x32_SEQUENCE_2
	_32x32_SEQUENCE_1_FLIP
_32x32_END

static const gfx_layout sega_32x32_r10_f0_layout =
_32x32_START
	_32x32_SEQUENCE_1_FLIP
	_32x32_SEQUENCE_2_FLIP
_32x32_END

static const gfx_layout sega_32x32_r11_f0_layout =
_32x32_START
	_32x32_SEQUENCE_2_FLIP
	_32x32_SEQUENCE_1
_32x32_END

static const gfx_layout sega_32x32_r00_f1_layout =
_32x32_START
	_32x32_SEQUENCE_1_FLIP
	_32x32_SEQUENCE_2
_32x32_END

static const gfx_layout sega_32x32_r01_f1_layout =
_32x32_START
	_32x32_SEQUENCE_2
	_32x32_SEQUENCE_1
_32x32_END

static const gfx_layout sega_32x32_r10_f1_layout =
_32x32_START
	_32x32_SEQUENCE_1
	_32x32_SEQUENCE_2_FLIP
_32x32_END

static const gfx_layout sega_32x32_r11_f1_layout =
_32x32_START
	_32x32_SEQUENCE_2_FLIP
	_32x32_SEQUENCE_1_FLIP
_32x32_END


static void segacd_mark_tiles_dirty(running_machine& machine, int offset)
{
	gfx_element_mark_dirty(machine.gfx[0], (offset*2)/(SEGACD_BYTES_PER_TILE16));
	gfx_element_mark_dirty(machine.gfx[1], (offset*2)/(SEGACD_BYTES_PER_TILE16));
	gfx_element_mark_dirty(machine.gfx[2], (offset*2)/(SEGACD_BYTES_PER_TILE16));
	gfx_element_mark_dirty(machine.gfx[3], (offset*2)/(SEGACD_BYTES_PER_TILE16));
	gfx_element_mark_dirty(machine.gfx[4], (offset*2)/(SEGACD_BYTES_PER_TILE16));
	gfx_element_mark_dirty(machine.gfx[5], (offset*2)/(SEGACD_BYTES_PER_TILE16));
	gfx_element_mark_dirty(machine.gfx[6], (offset*2)/(SEGACD_BYTES_PER_TILE16));
	gfx_element_mark_dirty(machine.gfx[7], (offset*2)/(SEGACD_BYTES_PER_TILE16));

	gfx_element_mark_dirty(machine.gfx[8], (offset*2)/(SEGACD_BYTES_PER_TILE32));
	gfx_element_mark_dirty(machine.gfx[9], (offset*2)/(SEGACD_BYTES_PER_TILE32));
	gfx_element_mark_dirty(machine.gfx[10],(offset*2)/(SEGACD_BYTES_PER_TILE32));
	gfx_element_mark_dirty(machine.gfx[11],(offset*2)/(SEGACD_BYTES_PER_TILE32));
	gfx_element_mark_dirty(machine.gfx[12],(offset*2)/(SEGACD_BYTES_PER_TILE32));
	gfx_element_mark_dirty(machine.gfx[13],(offset*2)/(SEGACD_BYTES_PER_TILE32));
	gfx_element_mark_dirty(machine.gfx[14],(offset*2)/(SEGACD_BYTES_PER_TILE32));
	gfx_element_mark_dirty(machine.gfx[15],(offset*2)/(SEGACD_BYTES_PER_TILE32));
}


// mame specific.. map registers to which tilemap cache we use
static int segacd_get_active_stampmap_tilemap(void)
{
	return (segacd_stampsize & 0x6)>>1;
}

#if 0
static void segacd_mark_stampmaps_dirty(void)
{
	segacd_stampmap[segacd_get_active_stampmap_tilemap(->mark_all_dirty()]);

	//segacd_stampmap[0]->mark_all_dirty();
	//segacd_stampmap[1]->mark_all_dirty();
	//segacd_stampmap[2]->mark_all_dirty();
	//segacd_stampmap[3]->mark_all_dirty();
}
#endif

void SCD_GET_TILE_INFO_16x16_1x1( int& tile_region, int& tileno, int tile_index )
{
	tile_region = 0; // 16x16 tiles
	int tile_base = (segacd_stampmap_base_address & 0xff80) * 4;

	int tiledat = segacd_dataram[((tile_base>>1)+tile_index) & 0x1ffff];
	tileno = tiledat & 0x07ff;
	int xflip =  tiledat & 0x8000;
	int roll  =  (tiledat & 0x6000)>>13;

	if (xflip) tile_region += 4;
	tile_region+=roll;
}

void SCD_GET_TILE_INFO_32x32_1x1( int& tile_region, int& tileno, int tile_index )
{
	tile_region = 8; // 32x32 tiles
	int tile_base = (segacd_stampmap_base_address & 0xffe0) * 4;

	int tiledat = segacd_dataram[((tile_base>>1)+tile_index) & 0x1ffff];
	tileno = (tiledat & 0x07fc)>>2;
	int xflip =  tiledat & 0x8000;
	int roll  =  (tiledat & 0x6000)>>13;

	if (xflip) tile_region += 4;
	tile_region+=roll;
}


void SCD_GET_TILE_INFO_16x16_16x16( int& tile_region, int& tileno, int tile_index )
{
	tile_region = 0; // 16x16 tiles
	int tile_base = (0x8000) * 4; // fixed address in this mode

	int tiledat = segacd_dataram[((tile_base>>1)+tile_index) & 0x1ffff];
	tileno = tiledat & 0x07ff;
	int xflip =  tiledat & 0x8000;
	int roll  =  (tiledat & 0x6000)>>13;

	if (xflip) tile_region += 4;
	tile_region+=roll;
}


void SCD_GET_TILE_INFO_32x32_16x16( int& tile_region, int& tileno, int tile_index )
{
	tile_region = 8; // 32x32 tiles
	int tile_base = (segacd_stampmap_base_address & 0xe000) * 4;

	int tiledat = segacd_dataram[((tile_base>>1)+tile_index) & 0x1ffff];
	tileno = (tiledat & 0x07fc)>>2;
	int xflip =  tiledat & 0x8000;
	int roll  =  (tiledat & 0x6000)>>13;

	if (xflip) tile_region += 4;
	tile_region+=roll;
}

/* Tilemap callbacks (we don't actually use the tilemaps due to the excessive overhead */



static TILE_GET_INFO( get_stampmap_16x16_1x1_tile_info )
{
	int tile_region, tileno;
	SCD_GET_TILE_INFO_16x16_1x1(tile_region,tileno,(int)tile_index);
	SET_TILE_INFO(tile_region, tileno, 0, 0);
}

static TILE_GET_INFO( get_stampmap_32x32_1x1_tile_info )
{
	int tile_region, tileno;
	SCD_GET_TILE_INFO_32x32_1x1(tile_region,tileno,(int)tile_index);
	SET_TILE_INFO(tile_region, tileno, 0, 0);
}


static TILE_GET_INFO( get_stampmap_16x16_16x16_tile_info )
{
	int tile_region, tileno;
	SCD_GET_TILE_INFO_16x16_16x16(tile_region,tileno,(int)tile_index);
	SET_TILE_INFO(tile_region, tileno, 0, 0);
}

static TILE_GET_INFO( get_stampmap_32x32_16x16_tile_info )
{
	int tile_region, tileno;
	SCD_GET_TILE_INFO_32x32_16x16(tile_region,tileno,(int)tile_index);
	SET_TILE_INFO(tile_region, tileno, 0, 0);
}

// non-tilemap functions to get a pixel from a 'tilemap' based on the above, but looking up each pixel, as to avoid the heavy cache bitmap

INLINE UINT8 get_stampmap_16x16_1x1_tile_info_pixel(running_machine& machine, int xpos, int ypos)
{
	const int tilesize = 4; // 0xf pixels
	const int tilemapsize = 0x0f;

	int wraparound = segacd_stampsize&1;

	int xtile = xpos / (1<<tilesize);
	int ytile = ypos / (1<<tilesize);

	if (wraparound)
	{
		// wrap...
		xtile &= tilemapsize;
		ytile &= tilemapsize;
	}
	else
	{
		if (xtile>tilemapsize) return 0;
		if (xtile<0) return 0;

		if (ytile>tilemapsize) return 0;
		if (ytile<0) return 0;
	}

	int tile_index = (ytile * (tilemapsize+1)) + xtile;

	int tile_region, tileno;
	SCD_GET_TILE_INFO_16x16_1x1(tile_region,tileno,(int)tile_index);

	const gfx_element *gfx = machine.gfx[tile_region];
	tileno %= gfx->total_elements;

	if (tileno==0) return 0x00;

	const UINT8* srcdata = gfx_element_get_data(gfx, tileno);
	return srcdata[((ypos&((1<<tilesize)-1))*(1<<tilesize))+(xpos&((1<<tilesize)-1))];
}

INLINE UINT8 get_stampmap_32x32_1x1_tile_info_pixel(running_machine& machine, int xpos, int ypos)
{
	const int tilesize = 5; // 0x1f pixels
	const int tilemapsize = 0x07;

	int wraparound = segacd_stampsize&1;

	int xtile = xpos / (1<<tilesize);
	int ytile = ypos / (1<<tilesize);

	if (wraparound)
	{
		// wrap...
		xtile &= tilemapsize;
		ytile &= tilemapsize;
	}
	else
	{
		if (xtile>tilemapsize) return 0;
		if (xtile<0) return 0;

		if (ytile>tilemapsize) return 0;
		if (ytile<0) return 0;
	}

	int tile_index = (ytile * (tilemapsize+1)) + xtile;

	int tile_region, tileno;
	SCD_GET_TILE_INFO_32x32_1x1(tile_region,tileno,(int)tile_index);

	const gfx_element *gfx = machine.gfx[tile_region];
	tileno %= gfx->total_elements;

	if (tileno==0) return 0x00; // does this apply in this mode?

	const UINT8* srcdata = gfx_element_get_data(gfx, tileno);
	return srcdata[((ypos&((1<<tilesize)-1))*(1<<tilesize))+(xpos&((1<<tilesize)-1))];
}

INLINE UINT8 get_stampmap_16x16_16x16_tile_info_pixel(running_machine& machine, int xpos, int ypos)
{
	const int tilesize = 4; // 0xf pixels
	const int tilemapsize = 0xff;

	int wraparound = segacd_stampsize&1;

	int xtile = xpos / (1<<tilesize);
	int ytile = ypos / (1<<tilesize);

	if (wraparound)
	{
		// wrap...
		xtile &= tilemapsize;
		ytile &= tilemapsize;
	}
	else
	{
		if (xtile>tilemapsize) return 0;
		if (xtile<0) return 0;

		if (ytile>tilemapsize) return 0;
		if (ytile<0) return 0;
	}

	int tile_index = (ytile * (tilemapsize+1)) + xtile;

	int tile_region, tileno;
	SCD_GET_TILE_INFO_16x16_16x16(tile_region,tileno,(int)tile_index);

	const gfx_element *gfx = machine.gfx[tile_region];
	tileno %= gfx->total_elements;

	if (tileno==0) return 0x00; // does this apply in this mode

	const UINT8* srcdata = gfx_element_get_data(gfx, tileno);
	return srcdata[((ypos&((1<<tilesize)-1))*(1<<tilesize))+(xpos&((1<<tilesize)-1))];
}

INLINE UINT8 get_stampmap_32x32_16x16_tile_info_pixel(running_machine& machine, int xpos, int ypos)
{
	const int tilesize = 5; // 0x1f pixels
	const int tilemapsize = 0x7f;

	int wraparound = segacd_stampsize&1;

	int xtile = xpos / (1<<tilesize);
	int ytile = ypos / (1<<tilesize);

	if (wraparound)
	{
		// wrap...
		xtile &= tilemapsize;
		ytile &= tilemapsize;
	}
	else
	{
		if (xtile>tilemapsize) return 0;
		if (xtile<0) return 0;

		if (ytile>tilemapsize) return 0;
		if (ytile<0) return 0;
	}

	int tile_index = (ytile * (tilemapsize+1)) + xtile;

	int tile_region, tileno;
	SCD_GET_TILE_INFO_32x32_16x16(tile_region,tileno,(int)tile_index);

	const gfx_element *gfx = machine.gfx[tile_region];
	tileno %= gfx->total_elements;

	if (tileno==0) return 0x00;

	const UINT8* srcdata = gfx_element_get_data(gfx, tileno);
	return srcdata[((ypos&((1<<tilesize)-1))*(1<<tilesize))+(xpos&((1<<tilesize)-1))];
}

static TIMER_CALLBACK( segacd_access_timer_callback )
{
	CheckCommand(machine);
}

READ16_HANDLER( cdc_data_sub_r )
{
	return CDC_Host_r(space->machine(), READ_SUB);
}

READ16_HANDLER( cdc_data_main_r )
{
	return CDC_Host_r(space->machine(), READ_MAIN);
}



WRITE16_HANDLER( segacd_stopwatch_timer_w )
{
	if(data == 0)
		stopwatch_timer->reset();
	else
		printf("Stopwatch timer %04x\n",data);
}

READ16_HANDLER( segacd_stopwatch_timer_r )
{
	INT32 result = (stopwatch_timer->time_elapsed() * ATTOSECONDS_TO_HZ(ATTOSECONDS_IN_USEC(30.72))).as_double();

	return result & 0xfff;
}


/* main CPU map set up in INIT */
void segacd_init_main_cpu( running_machine& machine )
{
	address_space* space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	segacd_4meg_prgbank = 0;


	space->unmap_readwrite        (0x020000,0x3fffff);

//  space->install_read_bank(0x0020000, 0x003ffff, "scd_4m_prgbank");
//  memory_set_bankptr(space->machine(),  "scd_4m_prgbank", segacd_4meg_prgram + segacd_4meg_prgbank * 0x20000 );
	space->install_legacy_read_handler (0x0020000, 0x003ffff, FUNC(scd_4m_prgbank_ram_r) );
	space->install_legacy_write_handler (0x0020000, 0x003ffff, FUNC(scd_4m_prgbank_ram_w) );
	segacd_wordram_mapped = 1;


	space->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x200000, 0x23ffff, FUNC(segacd_main_dataram_part1_r), FUNC(segacd_main_dataram_part1_w)); // RAM shared with sub

	space->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa12000, 0xa12001, FUNC(scd_a12000_halt_reset_r), FUNC(scd_a12000_halt_reset_w)); // sub-cpu control
	space->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa12002, 0xa12003, FUNC(scd_a12002_memory_mode_r), FUNC(scd_a12002_memory_mode_w)); // memory mode / write protect
	//space->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa12004, 0xa12005, FUNC(segacd_cdc_mode_address_r), FUNC(segacd_cdc_mode_address_w));
	space->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa12006, 0xa12007, FUNC(scd_a12006_hint_register_r), FUNC(scd_a12006_hint_register_w)); // where HINT points on main CPU
	//space->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler     (0xa12008, 0xa12009, FUNC(cdc_data_main_r));


	space->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa1200c, 0xa1200d, FUNC(segacd_stopwatch_timer_r), FUNC(segacd_stopwatch_timer_w)); // starblad

	space->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa1200e, 0xa1200f, FUNC(segacd_comms_flags_r), FUNC(segacd_comms_flags_maincpu_w)); // communication flags block

	space->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa12010, 0xa1201f, FUNC(segacd_comms_main_part1_r), FUNC(segacd_comms_main_part1_w));
	space->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa12020, 0xa1202f, FUNC(segacd_comms_main_part2_r), FUNC(segacd_comms_main_part2_w));



	device_set_irq_callback(machine.device("segacd_68k"), segacd_sub_int_callback);

	space->install_legacy_read_handler (0x0000070, 0x0000073, FUNC(scd_hint_vector_r) );

	segacd_gfx_conversion_timer = machine.scheduler().timer_alloc(FUNC(segacd_gfx_conversion_timer_callback));
	segacd_gfx_conversion_timer->adjust(attotime::never);

	//segacd_dmna_ret_timer = machine.scheduler().timer_alloc(FUNC(segacd_dmna_ret_timer_callback));
	segacd_gfx_conversion_timer->adjust(attotime::never);

	segacd_hock_timer = machine.scheduler().timer_alloc(FUNC(segacd_access_timer_callback));
//  segacd_hock_timer->adjust( attotime::from_nsec(20000000), 0, attotime::from_nsec(20000000));
	segacd_hock_timer->adjust( attotime::from_hz(75),0, attotime::from_hz(75));

	segacd_irq3_timer = machine.scheduler().timer_alloc(FUNC(segacd_irq3_timer_callback));
	segacd_irq3_timer->adjust(attotime::never);



	/* create the char set (gfx will then be updated dynamically from RAM) */
	machine.gfx[0] = gfx_element_alloc(machine, &sega_16x16_r00_f0_layout, (UINT8 *)segacd_dataram, 0, 0);
	machine.gfx[1] = gfx_element_alloc(machine, &sega_16x16_r01_f0_layout, (UINT8 *)segacd_dataram, 0, 0);
	machine.gfx[2] = gfx_element_alloc(machine, &sega_16x16_r10_f0_layout, (UINT8 *)segacd_dataram, 0, 0);
	machine.gfx[3] = gfx_element_alloc(machine, &sega_16x16_r11_f0_layout, (UINT8 *)segacd_dataram, 0, 0);
	machine.gfx[4] = gfx_element_alloc(machine, &sega_16x16_r00_f1_layout, (UINT8 *)segacd_dataram, 0, 0);
	machine.gfx[5] = gfx_element_alloc(machine, &sega_16x16_r11_f1_layout, (UINT8 *)segacd_dataram, 0, 0);
	machine.gfx[6] = gfx_element_alloc(machine, &sega_16x16_r10_f1_layout, (UINT8 *)segacd_dataram, 0, 0);
	machine.gfx[7] = gfx_element_alloc(machine, &sega_16x16_r01_f1_layout, (UINT8 *)segacd_dataram, 0, 0);

	machine.gfx[8] = gfx_element_alloc(machine, &sega_32x32_r00_f0_layout, (UINT8 *)segacd_dataram, 0, 0);
	machine.gfx[9] = gfx_element_alloc(machine, &sega_32x32_r01_f0_layout, (UINT8 *)segacd_dataram, 0, 0);
	machine.gfx[10]= gfx_element_alloc(machine, &sega_32x32_r10_f0_layout, (UINT8 *)segacd_dataram, 0, 0);
	machine.gfx[11]= gfx_element_alloc(machine, &sega_32x32_r11_f0_layout, (UINT8 *)segacd_dataram, 0, 0);
	machine.gfx[12]= gfx_element_alloc(machine, &sega_32x32_r00_f1_layout, (UINT8 *)segacd_dataram, 0, 0);
	machine.gfx[13]= gfx_element_alloc(machine, &sega_32x32_r11_f1_layout, (UINT8 *)segacd_dataram, 0, 0);
	machine.gfx[14]= gfx_element_alloc(machine, &sega_32x32_r10_f1_layout, (UINT8 *)segacd_dataram, 0, 0);
	machine.gfx[15]= gfx_element_alloc(machine, &sega_32x32_r01_f1_layout, (UINT8 *)segacd_dataram, 0, 0);

	segacd_stampmap[0] = tilemap_create(machine, get_stampmap_16x16_1x1_tile_info, tilemap_scan_rows, 16, 16, 16, 16);
	segacd_stampmap[1] = tilemap_create(machine, get_stampmap_32x32_1x1_tile_info, tilemap_scan_rows, 32, 32, 8, 8);
	segacd_stampmap[2] = tilemap_create(machine, get_stampmap_16x16_16x16_tile_info, tilemap_scan_rows, 16, 16, 256, 256); // 128kb!
	segacd_stampmap[3] = tilemap_create(machine, get_stampmap_32x32_16x16_tile_info, tilemap_scan_rows, 32, 32, 128, 128); // 32kb!
}


static timer_device* scd_dma_timer;

static TIMER_DEVICE_CALLBACK( scd_dma_timer_callback )
{
	// todo: accurate timing of this!

	#define RATE 256
	if (sega_cd_connected)
		CDC_Do_DMA(timer.machine(), RATE);

	// timed reset of flags
	scd_mode_dmna_ret_flags |= 0x0021;

	scd_dma_timer->adjust(attotime::from_hz(megadriv_framerate) / megadrive_total_scanlines);
}


static MACHINE_RESET( segacd )
{
	device_set_input_line(_segacd_68k_cpu, INPUT_LINE_RESET, ASSERT_LINE);
	device_set_input_line(_segacd_68k_cpu, INPUT_LINE_HALT, ASSERT_LINE);

	segacd_hint_register = 0xffff; // -1

	/* init cd-rom device */

	lc89510_Reset();

	{
		cdrom_image_device *device = machine.device<cdrom_image_device>("cdrom");
		if ( device )
		{
			segacd.cd = device->get_cdrom_file();
			if ( segacd.cd )
			{
				segacd.toc = cdrom_get_toc( segacd.cd );
				cdda_set_cdrom( machine.device("cdda"), segacd.cd );
				cdda_stop_audio( machine.device( "cdda" ) ); //stop any pending CD-DA
			}
		}
	}


	if (segacd.cd)
		printf("cd found\n");

	scd_rammode = 0;
	scd_mode_dmna_ret_flags = 0x5421;


	hock_cmd = 0;
	stopwatch_timer = machine.device<timer_device>("sw_timer");

	scd_dma_timer->adjust(attotime::zero);


	// HACK!!!! timegal, anettfut, roadaven end up with the SubCPU waiting in a loop for *something*
	// overclocking the CPU, even at the point where the game is hung, allows them to continue and boot
	// I'm not sure what the source of this timing problem is, it's not using IRQ3 or StopWatch at the
	// time.  Changing the CDHock timer to 50hz from 75hz also stops the hang, but then the video is
	// too slow and has bad sound.  -- Investigate!

	_segacd_68k_cpu->set_clock_scale(1.5000f);

}


static int segacd_redled = 0;
static int segacd_greenled = 0;
static int segacd_ready = 1; // actually set 100ms after startup?

static READ16_HANDLER( segacd_sub_led_ready_r )
{
	UINT16 retdata = 0x0000;

	if (ACCESSING_BITS_0_7)
	{
		retdata |= segacd_ready;
	}

	if (ACCESSING_BITS_8_15)
	{
		retdata |= segacd_redled << 8;
		retdata |= segacd_greenled << 9;
	}

	return retdata;
}

static WRITE16_HANDLER( segacd_sub_led_ready_w )
{
	if (ACCESSING_BITS_0_7)
	{
		if ((data&0x01) == 0x00)
		{
			// reset CD unit
		}
	}

	if (ACCESSING_BITS_8_15)
	{
		segacd_redled = (data >> 8)&1;
		segacd_greenled = (data >> 9)&1;

		//popmessage("%02x %02x",segacd_greenled,segacd_redled);
	}

}



static READ16_HANDLER( segacd_sub_dataram_part1_r )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		// is this correct?
		if (scd_rammode&1)
			return segacd_dataram[offset];
		else
		{
			printf("Illegal: segacd_sub_dataram_part1_r in mode 0 without permission\n");
			return 0x0000;
		}
	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
//      printf("Unspported: segacd_sub_dataram_part1_r in mode 1 (Word RAM Expander - 1 Byte Per Pixel)\n");
		UINT16 data;

		if (scd_rammode&1)
		{
			data = segacd_1meg_mode_word_read(offset/2+0x00000/2, 0xffff);
		}
		else
		{
			data = segacd_1meg_mode_word_read(offset/2+0x20000/2, 0xffff);
		}

		if (offset&1)
		{
			return ((data & 0x00f0) << 4) | ((data & 0x000f) << 0);
		}
		else
		{
			return ((data & 0xf000) >> 4) | ((data & 0x0f00) >> 8);
		}


	}

	return 0x0000;
}

static WRITE16_HANDLER( segacd_sub_dataram_part1_w )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		// is this correct?
		if (scd_rammode&1)
		{
			COMBINE_DATA(&segacd_dataram[offset]);
			segacd_mark_tiles_dirty(space->machine(), offset);
		}
		else
		{
			printf("Illegal: segacd_sub_dataram_part1_w in mode 0 without permission\n");
		}
	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		//if (mem_mask==0xffff)
		//  printf("Unspported: segacd_sub_dataram_part1_w in mode 1 (Word RAM Expander - 1 Byte Per Pixel) %04x %04x\n", data, mem_mask);

		data = (data & 0x000f) | (data & 0x0f00)>>4;
		mem_mask = (mem_mask & 0x000f) | (mem_mask & 0x0f00)>>4;

//      data = ((data & 0x00f0) >>4) | (data & 0xf000)>>8;
//      mem_mask = ((mem_mask & 0x00f0)>>4) | ((mem_mask & 0xf000)>>8);


		if (!(offset&1))
		{
			data <<=8;
			mem_mask <<=8;
		}

		if (scd_rammode&1)
		{
			segacd_1meg_mode_word_write(space->machine(), offset/2+0x00000/2, data , mem_mask, 1);
		}
		else
		{
			segacd_1meg_mode_word_write(space->machine(), offset/2+0x20000/2, data, mem_mask, 1);
		}

	//  printf("Unspported: segacd_sub_dataram_part1_w in mode 1 (Word RAM Expander - 1 Byte Per Pixel) %04x\n", data);
	}
}

static READ16_HANDLER( segacd_sub_dataram_part2_r )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		printf("ILLEGAL segacd_sub_dataram_part2_r in mode 0\n"); // not mapped to anything in mode 0
		return 0x0000;
	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		//printf("Unsupported: segacd_sub_dataram_part2_r in mode 1 (Word RAM)\n");
		// ret bit set by sub cpu determines which half of WorkRAM we have access to?
		if (scd_rammode&1)
		{
			return segacd_1meg_mode_word_read(offset+0x00000/2, mem_mask);
		}
		else
		{
			return segacd_1meg_mode_word_read(offset+0x20000/2, mem_mask);
		}

	}

	return 0x0000;
}

static WRITE16_HANDLER( segacd_sub_dataram_part2_w )
{
	if ((scd_rammode&2)==RAM_MODE_2MEG)
	{
		printf("ILLEGAL segacd_sub_dataram_part2_w in mode 0\n"); // not mapped to anything in mode 0
	}
	else if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		//printf("Unsupported: segacd_sub_dataram_part2_w in mode 1 (Word RAM)\n");
		// ret bit set by sub cpu determines which half of WorkRAM we have access to?
		if (scd_rammode&1)
		{
			segacd_1meg_mode_word_write(space->machine(),offset+0x00000/2, data, mem_mask, 0);
		}
		else
		{
			segacd_1meg_mode_word_write(space->machine(),offset+0x20000/2, data, mem_mask, 0);
		}

	}
}



static READ16_HANDLER( segacd_irq_mask_r )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();
	return segacd_irq_mask;
}

static WRITE16_HANDLER( segacd_irq_mask_w )
{
	if (ACCESSING_BITS_0_7)
	{
		UINT16 control = CDD_CONTROL;
		if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();
	//  printf("segacd_irq_mask_w %04x %04x (CDD control is %04x)\n",data, mem_mask, control);

		if (data & 0x10)
		{
			if (control & 0x04)
			{
				if (!(segacd_irq_mask & 0x10))
				{
					segacd_irq_mask = data & 0x7e;
					CDD_Process(space->machine(), 0);
					return;
				}
			}
		}

		segacd_irq_mask = data & 0x7e;
	}
	else
	{

		printf("segacd_irq_mask_w only MSB written\n");

	}
}

static READ16_HANDLER( segacd_cdd_ctrl_r )
{
	if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();
	return CDD_CONTROL;
}


static WRITE16_HANDLER( segacd_cdd_ctrl_w )
{
	if (ACCESSING_BITS_0_7)
	{
		UINT16 control = CDD_CONTROL;
		if (SEGACD_FORCE_SYNCS) space->machine().scheduler().synchronize();

		//printf("segacd_cdd_ctrl_w %04x %04x (control %04x irq %04x\n", data, mem_mask, control, segacd_irq_mask);

		data &=0x4; // only HOCK bit is writable

		if (data & 0x4)
		{
			if (!(control & 0x4))
			{
				if (segacd_irq_mask&0x10)
				{
					CDD_Process(space->machine(), 1);
				}
			}
		}

		CDD_CONTROL |= data;
	}
	else
	{
		printf("segacd_cdd_ctrl_w only MSB written\n");
	}
}



static READ8_HANDLER( segacd_cdd_rx_r )
{
	return CDD_RX[offset^1];
}

static WRITE8_HANDLER( segacd_cdd_tx_w )
{
	CDD_TX[offset^1] = data;

	if(offset == 9)
	{
		CDD_Import(space->machine());
	}
}



static READ16_HANDLER( segacd_stampsize_r )
{
	UINT16 retdata = 0x0000;

	retdata |= segacd_conversion_active<<15;

	retdata |= segacd_stampsize & 0x7;

	return retdata;

}

static WRITE16_HANDLER( segacd_stampsize_w )
{
	//printf("segacd_stampsize_w %04x %04x\n",data, mem_mask);
	if (ACCESSING_BITS_0_7)
	{
		segacd_stampsize = data & 0x07;
		//if (data & 0xf8)
		//  printf("    unused bits (LSB) set in stampsize!\n");

		//if (data&1) printf("    Repeat On\n");
		//else printf("    Repeat Off\n");

		//if (data&2) printf("    32x32 dots\n");
		//else printf("    16x16 dots\n");

		//if (data&4) printf("    16x16 screens\n");
		//else printf("    1x1 screen\n");
	}

	if (ACCESSING_BITS_8_15)
	{
		//if (data&0xff00) printf("    unused bits (MSB) set in stampsize!\n");
	}
}

// these functions won't cope if
//
// the lower 3 bits of segacd_imagebuffer_hdot_size are set

// this really needs to be doing it's own lookups rather than depending on the inefficient MAME cache..
INLINE UINT8 read_pixel_from_stampmap( running_machine& machine, bitmap_ind16* srcbitmap, int x, int y)
{
/*
    if (!srcbitmap)
    {
        return machine.rand();
    }

    if (x >= srcbitmap->width) return 0;
    if (y >= srcbitmap->height) return 0;

    UINT16* cacheptr = &srcbitmap->pix16(y, x);

    return cacheptr[0] & 0xf;
*/

	switch (segacd_get_active_stampmap_tilemap()&3)
	{
		case 0x00: return get_stampmap_16x16_1x1_tile_info_pixel( machine, x, y );
		case 0x01: return get_stampmap_32x32_1x1_tile_info_pixel( machine, x, y );
		case 0x02: return get_stampmap_16x16_16x16_tile_info_pixel( machine, x, y );
		case 0x03: return get_stampmap_32x32_16x16_tile_info_pixel( machine, x, y );
	}

	return 0;
}





// this triggers the conversion operation, which will cause an IRQ1 when finished
WRITE16_HANDLER( segacd_trace_vector_base_address_w )
{
	if ((scd_rammode&2)==RAM_MODE_1MEG)
	{
		printf("ILLEGAL: segacd_trace_vector_base_address_w %04x %04x in mode 1!\n",data,mem_mask);
	}

	//printf("segacd_trace_vector_base_address_w %04x %04x\n",data,mem_mask);

	{
		int base = (data & 0xfffe) * 4;

		//printf("actual base = %06x\n", base + 0x80000);

		// nasty nasty nasty
		//segacd_mark_stampmaps_dirty();

		segacd_conversion_active = 1;

		// todo: proper time calculation
		segacd_gfx_conversion_timer->adjust(attotime::from_nsec(30000));



		int line;
		//bitmap_ind16 *srcbitmap = segacd_stampmap[segacd_get_active_stampmap_tilemap(->pixmap()]);
		bitmap_ind16 *srcbitmap = 0;
		UINT32 bufferstart = ((segacd_imagebuffer_start_address&0xfff8)*2)<<3;

		for (line=0;line<segacd_imagebuffer_vdot_size;line++)
		{
			int currbase = base + line * 0x8;

			// are the 256x256 tile modes using the same sign bits?
			INT16 tilemapxoffs,tilemapyoffs;
			INT16 deltax,deltay;

			tilemapxoffs = segacd_dataram[(currbase+0x0)>>1];
			tilemapyoffs = segacd_dataram[(currbase+0x2)>>1];
			deltax = segacd_dataram[(currbase+0x4)>>1]; // x-zoom
			deltay = segacd_dataram[(currbase+0x6)>>1]; // rotation

			//printf("%06x:  %04x (%d) %04x (%d) %04x %04x\n", currbase, tilemapxoffs, tilemapxoffs>>3, tilemapyoffs, tilemapyoffs>>3, deltax, deltay);

			int xbase = tilemapxoffs * 256;
			int ybase = tilemapyoffs * 256;
			int count;

			for (count=0;count<(segacd_imagebuffer_hdot_size);count++)
			{
				//int i;
				UINT8 pix = 0x0;

				pix = read_pixel_from_stampmap(space->machine(), srcbitmap, xbase>>(3+8), ybase>>(3+8));

				xbase += deltax;
				ybase += deltay;

				// clamp to 24-bits, seems to be required for all the intro effects to work
				xbase &= 0xffffff;
				ybase &= 0xffffff;

				int countx = count + (segacd_imagebuffer_offset&0x7);

				UINT32 offset;

				offset = bufferstart+((((segacd_imagebuffer_vcell_size+1)*0x10)*(countx>>3))<<3);

				offset+= ((line*2)<<3);
				offset+=(segacd_imagebuffer_offset&0x38)<<1;

				offset+=countx & 0x7;

				write_pixel( space->machine(), pix, offset );

				segacd_mark_tiles_dirty(space->machine(), (offset>>3));
				segacd_mark_tiles_dirty(space->machine(), (offset>>3)+1);

			}

		}
	}

}

// actually just the low 8 bits?
READ16_HANDLER( segacd_imagebuffer_vdot_size_r )
{
	return segacd_imagebuffer_vdot_size;
}

WRITE16_HANDLER( segacd_imagebuffer_vdot_size_w )
{
	//printf("segacd_imagebuffer_vdot_size_w %04x %04x\n",data,mem_mask);
	COMBINE_DATA(&segacd_imagebuffer_vdot_size);
}


// basically the 'tilemap' base address, for the 16x16 / 32x32 source tiles
static READ16_HANDLER( segacd_stampmap_base_address_r )
{
	// different bits are valid in different modes, but I'm guessing the register
	// always returns all the bits set, even if they're not used?
	return segacd_stampmap_base_address;

}

static WRITE16_HANDLER( segacd_stampmap_base_address_w )
{ // WORD ACCESS

	// low 3 bitsa aren't used, are they stored?
	COMBINE_DATA(&segacd_stampmap_base_address);
}

// destination for 'rendering' the section of the tilemap(stampmap) requested
static READ16_HANDLER( segacd_imagebuffer_start_address_r )
{
	return segacd_imagebuffer_start_address;
}

static WRITE16_HANDLER( segacd_imagebuffer_start_address_w )
{
	COMBINE_DATA(&segacd_imagebuffer_start_address);

	//int base = (segacd_imagebuffer_start_address & 0xfffe) * 4;
	//printf("segacd_imagebuffer_start_address_w %04x %04x (actual base = %06x)\n", data, segacd_imagebuffer_start_address, base);
}

static READ16_HANDLER( segacd_imagebuffer_offset_r )
{
	return segacd_imagebuffer_offset;
}

static WRITE16_HANDLER( segacd_imagebuffer_offset_w )
{
	COMBINE_DATA(&segacd_imagebuffer_offset);
//  printf("segacd_imagebuffer_offset_w %04x\n", segacd_imagebuffer_offset);
}

static READ16_HANDLER( segacd_imagebuffer_vcell_size_r )
{
	return segacd_imagebuffer_vcell_size;
}

static WRITE16_HANDLER( segacd_imagebuffer_vcell_size_w )
{
	COMBINE_DATA(&segacd_imagebuffer_vcell_size);
}


static READ16_HANDLER( segacd_imagebuffer_hdot_size_r )
{
	return segacd_imagebuffer_hdot_size;
}

static WRITE16_HANDLER( segacd_imagebuffer_hdot_size_w )
{
	COMBINE_DATA(&segacd_imagebuffer_hdot_size);
}

static UINT16 segacd_irq3_timer_reg;

static READ16_HANDLER( segacd_irq3timer_r )
{
	return segacd_irq3_timer_reg; // always returns value written, not current counter!
}

#define SEGACD_IRQ3_TIMER_SPEED (attotime::from_nsec(segacd_irq3_timer_reg*30720))

static WRITE16_HANDLER( segacd_irq3timer_w )
{
	if (ACCESSING_BITS_0_7)
	{
		segacd_irq3_timer_reg = data & 0xff;

		// time = reg * 30.72 us

		if (segacd_irq3_timer_reg)
			segacd_irq3_timer->adjust(SEGACD_IRQ3_TIMER_SPEED);
		else
			segacd_irq3_timer->adjust(attotime::never);

		//printf("segacd_irq3timer_w %02x\n", segacd_irq3_timer_reg);
	}
}



static TIMER_CALLBACK( segacd_irq3_timer_callback )
{
	CHECK_SCD_LV3_INTERRUPT

	segacd_irq3_timer->adjust(SEGACD_IRQ3_TIMER_SPEED);
}



READ16_HANDLER( cdc_dmaaddr_r )
{
	return CDC_DMA_ADDR;
}

WRITE16_HANDLER( cdc_dmaaddr_w )
{
	COMBINE_DATA(&CDC_DMA_ADDR);
}

READ16_HANDLER( segacd_cdfader_r )
{
	return 0;
}

WRITE16_HANDLER( segacd_cdfader_w )
{
	static double cdfader_vol;
	if(data & 0x800f)
		printf("CD Fader register write %04x\n",data);

	cdfader_vol = (double)((data & 0x3ff0) >> 4);

	if(data & 0x4000)
		cdfader_vol = 100.0;
	else
		cdfader_vol = (cdfader_vol / 1024.0) * 100.0;

	//printf("%f\n",cdfader_vol);

	cdda_set_volume(space->machine().device("cdda"), cdfader_vol);
}

READ16_HANDLER( segacd_backupram_r )
{
	if(ACCESSING_BITS_8_15 && !(space->debugger_access()))
		printf("Warning: read to backupram even bytes! [%04x]\n",offset);

	return segacd_backupram[offset] & 0xff;
}

WRITE16_HANDLER( segacd_backupram_w )
{
	if(ACCESSING_BITS_0_7)
		segacd_backupram[offset] = data;

	if(ACCESSING_BITS_8_15 && !(space->debugger_access()))
		printf("Warning: write to backupram even bytes! [%04x] %02x\n",offset,data);
}

READ16_HANDLER( segacd_font_color_r )
{
	return segacd_font_color;
}

WRITE16_HANDLER( segacd_font_color_w )
{
	if (ACCESSING_BITS_0_7)
	{
		segacd_font_color = data & 0xff;
	}
}

READ16_HANDLER( segacd_font_converted_r )
{
	int scbg = (segacd_font_color & 0x0f);
	int scfg = (segacd_font_color & 0xf0)>>4;
	UINT16 retdata = 0;
	int bit;

	for (bit=0;bit<4;bit++)
	{
		if (*segacd_font_bits&((0x1000>>offset*4)<<bit))
			retdata |= scfg << (bit*4);
		else
			retdata |= scbg << (bit*4);
	}

	return retdata;
}

static ADDRESS_MAP_START( segacd_map, AS_PROGRAM, 16, driver_device )
	AM_RANGE(0x000000, 0x07ffff) AM_RAM AM_BASE_LEGACY(&segacd_4meg_prgram)

	AM_RANGE(0x080000, 0x0bffff) AM_READWRITE_LEGACY(segacd_sub_dataram_part1_r, segacd_sub_dataram_part1_w) AM_BASE_LEGACY(&segacd_dataram)
	AM_RANGE(0x0c0000, 0x0dffff) AM_READWRITE_LEGACY(segacd_sub_dataram_part2_r, segacd_sub_dataram_part2_w) AM_BASE_LEGACY(&segacd_dataram2)

	AM_RANGE(0xfe0000, 0xfe3fff) AM_READWRITE_LEGACY(segacd_backupram_r,segacd_backupram_w) AM_SHARE("backupram") AM_BASE_LEGACY(&segacd_backupram)// backup RAM, odd bytes only!

	AM_RANGE(0xff0000, 0xff001f) AM_DEVWRITE8_LEGACY("rfsnd", rf5c68_w, 0x00ff)  // PCM, RF5C164
	AM_RANGE(0xff0020, 0xff003f) AM_DEVREAD8_LEGACY("rfsnd", rf5c68_r, 0x00ff)
	AM_RANGE(0xff2000, 0xff3fff) AM_DEVREADWRITE8_LEGACY("rfsnd", rf5c68_mem_r, rf5c68_mem_w,0x00ff)  // PCM, RF5C164


	AM_RANGE(0xff8000 ,0xff8001) AM_READWRITE_LEGACY(segacd_sub_led_ready_r, segacd_sub_led_ready_w)
	AM_RANGE(0xff8002 ,0xff8003) AM_READWRITE_LEGACY(segacd_sub_memory_mode_r, segacd_sub_memory_mode_w)

	AM_RANGE(0xff8004 ,0xff8005) AM_READWRITE_LEGACY(segacd_cdc_mode_address_r, segacd_cdc_mode_address_w)
	AM_RANGE(0xff8006 ,0xff8007) AM_READWRITE_LEGACY(segacd_cdc_data_r, segacd_cdc_data_w)
	AM_RANGE(0xff8008, 0xff8009) AM_READ_LEGACY(cdc_data_sub_r)
	AM_RANGE(0xff800a, 0xff800b) AM_READWRITE_LEGACY(cdc_dmaaddr_r,cdc_dmaaddr_w) // CDC DMA Address
	AM_RANGE(0xff800c, 0xff800d) AM_READWRITE_LEGACY(segacd_stopwatch_timer_r, segacd_stopwatch_timer_w)// Stopwatch timer
	AM_RANGE(0xff800e ,0xff800f) AM_READWRITE_LEGACY(segacd_comms_flags_r, segacd_comms_flags_subcpu_w)
	AM_RANGE(0xff8010 ,0xff801f) AM_READWRITE_LEGACY(segacd_comms_sub_part1_r, segacd_comms_sub_part1_w)
	AM_RANGE(0xff8020 ,0xff802f) AM_READWRITE_LEGACY(segacd_comms_sub_part2_r, segacd_comms_sub_part2_w)
	AM_RANGE(0xff8030, 0xff8031) AM_READWRITE_LEGACY(segacd_irq3timer_r, segacd_irq3timer_w) // Timer W/INT3
	AM_RANGE(0xff8032, 0xff8033) AM_READWRITE_LEGACY(segacd_irq_mask_r,segacd_irq_mask_w)
	AM_RANGE(0xff8034, 0xff8035) AM_READWRITE_LEGACY(segacd_cdfader_r,segacd_cdfader_w) // CD Fader
	AM_RANGE(0xff8036, 0xff8037) AM_READWRITE_LEGACY(segacd_cdd_ctrl_r,segacd_cdd_ctrl_w)
	AM_RANGE(0xff8038, 0xff8041) AM_READ8_LEGACY(segacd_cdd_rx_r,0xffff)
	AM_RANGE(0xff8042, 0xff804b) AM_WRITE8_LEGACY(segacd_cdd_tx_w,0xffff)
	AM_RANGE(0xff804c, 0xff804d) AM_READWRITE_LEGACY(segacd_font_color_r, segacd_font_color_w)
	AM_RANGE(0xff804e, 0xff804f) AM_RAM AM_BASE_LEGACY(&segacd_font_bits)
	AM_RANGE(0xff8050, 0xff8057) AM_READ_LEGACY(segacd_font_converted_r)
	AM_RANGE(0xff8058, 0xff8059) AM_READWRITE_LEGACY(segacd_stampsize_r, segacd_stampsize_w) // Stamp size
	AM_RANGE(0xff805a, 0xff805b) AM_READWRITE_LEGACY(segacd_stampmap_base_address_r, segacd_stampmap_base_address_w) // Stamp map base address
	AM_RANGE(0xff805c, 0xff805d) AM_READWRITE_LEGACY(segacd_imagebuffer_vcell_size_r, segacd_imagebuffer_vcell_size_w)// Image buffer V cell size
	AM_RANGE(0xff805e, 0xff805f) AM_READWRITE_LEGACY(segacd_imagebuffer_start_address_r, segacd_imagebuffer_start_address_w) // Image buffer start address
	AM_RANGE(0xff8060, 0xff8061) AM_READWRITE_LEGACY(segacd_imagebuffer_offset_r, segacd_imagebuffer_offset_w)
	AM_RANGE(0xff8062, 0xff8063) AM_READWRITE_LEGACY(segacd_imagebuffer_hdot_size_r, segacd_imagebuffer_hdot_size_w) // Image buffer H dot size
	AM_RANGE(0xff8064, 0xff8065) AM_READWRITE_LEGACY(segacd_imagebuffer_vdot_size_r, segacd_imagebuffer_vdot_size_w ) // Image buffer V dot size
	AM_RANGE(0xff8066, 0xff8067) AM_WRITE_LEGACY(segacd_trace_vector_base_address_w)// Trace vector base address
//  AM_RANGE(0xff8068, 0xff8069) // Subcode address

//  AM_RANGE(0xff8100, 0xff817f) // Subcode buffer area
//  AM_RANGE(0xff8180, 0xff81ff) // mirror of subcode buffer area

ADDRESS_MAP_END



/****************************************** SVP related *****************************************/

/*
 * Emulator of memory controller in SVP chip
 *
 * Copyright 2008, Grazvydas Ignotas
 * based on RE work by Tasco Deluxe
 *
 * SSP1601 EXT registers are mapped as I/O ports due to their function
 * (they are interfaced through external bus), and are named as follows
 * (these are unofficial names, official ones are unknown):
 *   EXT0: PM0 - programmable register 0
 *   EXT1: PM1 - ... 1
 *   EXT2: PM2 - ... 2
 *   EXT3: XST - external status. Can also act as PM.
 *   EXT4: PM4 - ... 4
 *   EXT5: (unused)
 *   EXT6: PMC - programmable memory register control (PMAC).
 *   EXT7: AL  - although internal to SSP1601, it still causes bus access
 *
 * Depending on GPO bits in status register, PM0, PM1, PM2 and XST can act as
 * external status registers, os as programmable memory registers. PM4 always
 * acts as PM register (independent on GPO bits).
 */

#include "cpu/ssp1601/ssp1601.h"

#define SSP_PMC_HAVE_ADDR  1  // address written to PMAC, waiting for mode
#define SSP_PMC_SET        2  // PMAC is set, PMx can be programmed

static int get_inc(int mode)
{
	int inc = (mode >> 11) & 7;
	if (inc != 0) {
		if (inc != 7) inc--;
		inc = 1 << inc; // 0 1 2 4 8 16 32 128
		if (mode & 0x8000) inc = -inc; // decrement mode
	}
	return inc;
}

INLINE void overwrite_write(UINT16 *dst, UINT16 d)
{
	if (d & 0xf000) { *dst &= ~0xf000; *dst |= d & 0xf000; }
	if (d & 0x0f00) { *dst &= ~0x0f00; *dst |= d & 0x0f00; }
	if (d & 0x00f0) { *dst &= ~0x00f0; *dst |= d & 0x00f0; }
	if (d & 0x000f) { *dst &= ~0x000f; *dst |= d & 0x000f; }
}

static UINT32 pm_io(address_space *space, int reg, int write, UINT32 d)
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	if (state->m_emu_status & SSP_PMC_SET)
	{
		state->m_pmac_read[write ? reg + 6 : reg] = state->m_pmc.d;
		state->m_emu_status &= ~SSP_PMC_SET;
		return 0;
	}

	// just in case
	if (state->m_emu_status & SSP_PMC_HAVE_ADDR) {
		state->m_emu_status &= ~SSP_PMC_HAVE_ADDR;
	}

	if (reg == 4 || (cpu_get_reg(&space->device(), SSP_ST) & 0x60))
	{
		#define CADDR ((((mode<<16)&0x7f0000)|addr)<<1)
		UINT16 *dram = (UINT16 *)state->m_dram;
		if (write)
		{
			int mode = state->m_pmac_write[reg]>>16;
			int addr = state->m_pmac_write[reg]&0xffff;
			if      ((mode & 0x43ff) == 0x0018) // DRAM
			{
				int inc = get_inc(mode);
				if (mode & 0x0400) {
				       overwrite_write(&dram[addr], d);
				} else dram[addr] = d;
				state->m_pmac_write[reg] += inc;
			}
			else if ((mode & 0xfbff) == 0x4018) // DRAM, cell inc
			{
				if (mode & 0x0400) {
				       overwrite_write(&dram[addr], d);
				} else dram[addr] = d;
				state->m_pmac_write[reg] += (addr&1) ? 31 : 1;
			}
			else if ((mode & 0x47ff) == 0x001c) // IRAM
			{
				int inc = get_inc(mode);
				((UINT16 *)state->m_iram)[addr&0x3ff] = d;
				state->m_pmac_write[reg] += inc;
			}
			else
			{
				logerror("ssp FIXME: PM%i unhandled write mode %04x, [%06x] %04x\n",
						reg, mode, CADDR, d);
			}
		}
		else
		{
			int mode = state->m_pmac_read[reg]>>16;
			int addr = state->m_pmac_read[reg]&0xffff;
			if      ((mode & 0xfff0) == 0x0800) // ROM, inc 1, verified to be correct
			{
				UINT16 *ROM = (UINT16 *) space->machine().region("maincpu")->base();
				state->m_pmac_read[reg] += 1;
				d = ROM[addr|((mode&0xf)<<16)];
			}
			else if ((mode & 0x47ff) == 0x0018) // DRAM
			{
				int inc = get_inc(mode);
				d = dram[addr];
				state->m_pmac_read[reg] += inc;
			}
			else
			{
				logerror("ssp FIXME: PM%i unhandled read  mode %04x, [%06x]\n",
						reg, mode, CADDR);
				d = 0;
			}
		}

		// PMC value corresponds to last PMR accessed (not sure).
		state->m_pmc.d = state->m_pmac_read[write ? reg + 6 : reg];

		return d;
	}

	return (UINT32)-1;
}

static READ16_HANDLER( read_PM0 )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	UINT32 d = pm_io(space, 0, 0, 0);
	if (d != (UINT32)-1) return d;
	d = state->m_XST2;
	state->m_XST2 &= ~2; // ?
	return d;
}

static WRITE16_HANDLER( write_PM0 )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	UINT32 r = pm_io(space, 0, 1, data);
	if (r != (UINT32)-1) return;
	state->m_XST2 = data; // ?
}

static READ16_HANDLER( read_PM1 )
{
	UINT32 r = pm_io(space, 1, 0, 0);
	if (r != (UINT32)-1) return r;
	logerror("svp: PM1 acces in non PM mode?\n");
	return 0;
}

static WRITE16_HANDLER( write_PM1 )
{
	UINT32 r = pm_io(space, 1, 1, data);
	if (r != (UINT32)-1) return;
	logerror("svp: PM1 acces in non PM mode?\n");
}

static READ16_HANDLER( read_PM2 )
{
	UINT32 r = pm_io(space, 2, 0, 0);
	if (r != (UINT32)-1) return r;
	logerror("svp: PM2 acces in non PM mode?\n");
	return 0;
}

static WRITE16_HANDLER( write_PM2 )
{
	UINT32 r = pm_io(space, 2, 1, data);
	if (r != (UINT32)-1) return;
	logerror("svp: PM2 acces in non PM mode?\n");
}

static READ16_HANDLER( read_XST )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	UINT32 d = pm_io(space, 3, 0, 0);
	if (d != (UINT32)-1) return d;

	return state->m_XST;
}

static WRITE16_HANDLER( write_XST )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	UINT32 r = pm_io(space, 3, 1, data);
	if (r != (UINT32)-1) return;

	state->m_XST2 |= 1;
	state->m_XST = data;
}

static READ16_HANDLER( read_PM4 )
{
	return pm_io(space, 4, 0, 0);
}

static WRITE16_HANDLER( write_PM4 )
{
	pm_io(space, 4, 1, data);
}

static READ16_HANDLER( read_PMC )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	if (state->m_emu_status & SSP_PMC_HAVE_ADDR) {
		state->m_emu_status |= SSP_PMC_SET;
		state->m_emu_status &= ~SSP_PMC_HAVE_ADDR;
		return ((state->m_pmc.w.l << 4) & 0xfff0) | ((state->m_pmc.w.l >> 4) & 0xf);
	} else {
		state->m_emu_status |= SSP_PMC_HAVE_ADDR;
		return state->m_pmc.w.l;
	}
}

static WRITE16_HANDLER( write_PMC )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	if (state->m_emu_status & SSP_PMC_HAVE_ADDR) {
		state->m_emu_status |= SSP_PMC_SET;
		state->m_emu_status &= ~SSP_PMC_HAVE_ADDR;
		state->m_pmc.w.h = data;
	} else {
		state->m_emu_status |= SSP_PMC_HAVE_ADDR;
		state->m_pmc.w.l = data;
	}
}

static READ16_HANDLER( read_AL )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	state->m_emu_status &= ~(SSP_PMC_SET|SSP_PMC_HAVE_ADDR);
	return 0;
}

static WRITE16_HANDLER( write_AL )
{
}



static READ16_HANDLER( svp_68k_io_r )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	UINT32 d;
	switch (offset)
	{
		// 0xa15000, 0xa15002
		case 0:
		case 1:  return state->m_XST;
		// 0xa15004
		case 2:  d = state->m_XST2; state->m_XST2 &= ~1; return d;
		default: logerror("unhandled SVP reg read @ %x\n", offset<<1);
	}
	return 0;
}

static WRITE16_HANDLER( svp_68k_io_w )
{
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	switch (offset)
	{
		// 0xa15000, 0xa15002
		case 0:
		case 1:  state->m_XST = data; state->m_XST2 |= 2; break;
		// 0xa15006
		case 3:  break; // possibly halts SSP1601
		default: logerror("unhandled SVP reg write %04x @ %x\n", data, offset<<1);
	}
}

static READ16_HANDLER( svp_68k_cell1_r )
{
	// this is rewritten 68k test code
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	UINT32 a1 = offset;
	a1 = (a1 & 0x7001) | ((a1 & 0x3e) << 6) | ((a1 & 0xfc0) >> 5);
	return ((UINT16 *)state->m_dram)[a1];
}

static READ16_HANDLER( svp_68k_cell2_r )
{
	// this is rewritten 68k test code
	mdsvp_state *state = space->machine().driver_data<mdsvp_state>();
	UINT32 a1 = offset;
	a1 = (a1 & 0x7801) | ((a1 & 0x1e) << 6) | ((a1 & 0x7e0) >> 4);
	return ((UINT16 *)state->m_dram)[a1];
}

static ADDRESS_MAP_START( svp_ssp_map, AS_PROGRAM, 16, driver_device )
	AM_RANGE(0x0000, 0x03ff) AM_ROMBANK("bank3")
	AM_RANGE(0x0400, 0xffff) AM_ROMBANK("bank4")
ADDRESS_MAP_END

static ADDRESS_MAP_START( svp_ext_map, AS_IO, 16, driver_device )
	ADDRESS_MAP_GLOBAL_MASK(0xf)
	AM_RANGE(0*2, 0*2+1) AM_READWRITE_LEGACY(read_PM0, write_PM0)
	AM_RANGE(1*2, 1*2+1) AM_READWRITE_LEGACY(read_PM1, write_PM1)
	AM_RANGE(2*2, 2*2+1) AM_READWRITE_LEGACY(read_PM2, write_PM2)
	AM_RANGE(3*2, 3*2+1) AM_READWRITE_LEGACY(read_XST, write_XST)
	AM_RANGE(4*2, 4*2+1) AM_READWRITE_LEGACY(read_PM4, write_PM4)
	AM_RANGE(6*2, 6*2+1) AM_READWRITE_LEGACY(read_PMC, write_PMC)
	AM_RANGE(7*2, 7*2+1) AM_READWRITE_LEGACY(read_AL, write_AL)
ADDRESS_MAP_END


/* emulate testmode plug */
static UINT8 megadrive_io_read_data_port_svp(running_machine &machine, int portnum)
{
	if (portnum == 0 && input_port_read_safe(machine, "MEMORY_TEST", 0x00))
	{
		return (megadrive_io_data_regs[0] & 0xc0);
	}
	return megadrive_io_read_data_port_3button(machine, portnum);
}


static READ16_HANDLER( svp_speedup_r )
{
	 device_spin_until_time(&space->device(), attotime::from_usec(100));
	return 0x0425;
}


static void svp_init(running_machine &machine)
{
	mdsvp_state *state = machine.driver_data<mdsvp_state>();
	UINT8 *ROM;

	memset(state->m_pmac_read, 0, ARRAY_LENGTH(state->m_pmac_read));
	memset(state->m_pmac_write, 0, ARRAY_LENGTH(state->m_pmac_write));
	state->m_pmc.d = 0;
	state->m_pmc.w.l = 0;
	state->m_pmc.w.h = 0;
	state->m_emu_status = 0;
	state->m_XST = 0;
	state->m_XST2 = 0;

	/* SVP stuff */
	state->m_dram = auto_alloc_array(machine, UINT8, 0x20000);
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_ram(0x300000, 0x31ffff, state->m_dram);
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15000, 0xa150ff, FUNC(svp_68k_io_r), FUNC(svp_68k_io_w));
	// "cell arrange" 1 and 2
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x390000, 0x39ffff, FUNC(svp_68k_cell1_r));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x3a0000, 0x3affff, FUNC(svp_68k_cell2_r));

	machine.device("svp")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x438, 0x438, FUNC(svp_speedup_r));

	state->m_iram = auto_alloc_array(machine, UINT8, 0x800);
	memory_set_bankptr(machine,  "bank3", state->m_iram);
	/* SVP ROM just shares m68k region.. */
	ROM = machine.region("maincpu")->base();
	memory_set_bankptr(machine,  "bank4", ROM + 0x800);

	megadrive_io_read_data_port_ptr	= megadrive_io_read_data_port_svp;
}


INPUT_PORTS_START( megdsvp )
	PORT_INCLUDE( megadriv )

	PORT_START("MEMORY_TEST") /* special memtest mode */
	/* Region setting for Console */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Test ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x01, DEF_STR( On ) )
INPUT_PORTS_END

MACHINE_CONFIG_FRAGMENT( md_svp )
	MCFG_CPU_ADD("svp", SSP1601, MASTER_CLOCK_NTSC / 7 * 3) /* ~23 MHz (guessed) */
	MCFG_CPU_PROGRAM_MAP(svp_ssp_map)
	MCFG_CPU_IO_MAP(svp_ext_map)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( megdsvp, megadriv )

	MCFG_CPU_ADD("svp", SSP1601, MASTER_CLOCK_NTSC / 7 * 3) /* ~23 MHz (guessed) */
	MCFG_CPU_PROGRAM_MAP(svp_ssp_map)
	MCFG_CPU_IO_MAP(svp_ext_map)
	/* IRQs are not used by this CPU */
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( megdsvppal, megadpal )

	MCFG_CPU_ADD("svp", SSP1601, MASTER_CLOCK_PAL / 7 * 3) /* ~23 MHz (guessed) */
	MCFG_CPU_PROGRAM_MAP(svp_ssp_map)
	MCFG_CPU_IO_MAP(svp_ext_map)
	/* IRQs are not used by this CPU */
MACHINE_CONFIG_END

/****************************************** END SVP related *************************************/


//static attotime time_elapsed_since_crap;


VIDEO_START(megadriv)
{
	int x;

	render_bitmap = auto_bitmap_ind16_alloc(machine, machine.primary_screen->width(), machine.primary_screen->height());

	megadrive_vdp_vram  = auto_alloc_array(machine, UINT16, 0x10000/2);
	megadrive_vdp_cram  = auto_alloc_array(machine, UINT16, 0x80/2);
	megadrive_vdp_vsram = auto_alloc_array(machine, UINT16, 0x80/2);
	megadrive_vdp_internal_sprite_attribute_table = auto_alloc_array(machine, UINT16, 0x400/2);

	for (x=0;x<0x20;x++)
		megadrive_vdp_register[x]=0;
//  memset(megadrive_vdp_vram, 0xff, 0x10000);
//  memset(megadrive_vdp_cram, 0xff, 0x80);
//  memset(megadrive_vdp_vsram, 0xff, 0x80);

	memset(megadrive_vdp_vram, 0x00, 0x10000);
	memset(megadrive_vdp_cram, 0x00, 0x80);
	memset(megadrive_vdp_vsram, 0x00, 0x80);
	memset(megadrive_vdp_internal_sprite_attribute_table, 0x00, 0x400);

	megadrive_max_hposition = 480;

	sprite_renderline = auto_alloc_array(machine, UINT8, 1024);
	highpri_renderline = auto_alloc_array(machine, UINT8, 320);
	video_renderline = auto_alloc_array(machine, UINT32, 320);

	megadrive_vdp_palette_lookup = auto_alloc_array(machine, UINT16, 0x40);
	megadrive_vdp_palette_lookup_sprite = auto_alloc_array(machine, UINT16, 0x40);

	megadrive_vdp_palette_lookup_shadow = auto_alloc_array(machine, UINT16, 0x40);
	megadrive_vdp_palette_lookup_highlight = auto_alloc_array(machine, UINT16, 0x40);

	memset(megadrive_vdp_palette_lookup,0x00,0x40*2);
	memset(megadrive_vdp_palette_lookup_sprite,0x00,0x40*2);

	memset(megadrive_vdp_palette_lookup_shadow,0x00,0x40*2);
	memset(megadrive_vdp_palette_lookup_highlight,0x00,0x40*2);

	/* no special lookups */
	segac2_bg_pal_lookup[0] = 0x00;
	segac2_bg_pal_lookup[1] = 0x10;
	segac2_bg_pal_lookup[2] = 0x20;
	segac2_bg_pal_lookup[3] = 0x30;

	segac2_sp_pal_lookup[0] = 0x00;
	segac2_sp_pal_lookup[1] = 0x10;
	segac2_sp_pal_lookup[2] = 0x20;
	segac2_sp_pal_lookup[3] = 0x30;
}

SCREEN_UPDATE_RGB32(megadriv)
{
	/* Copy our screen buffer here */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			UINT16 src = render_bitmap->pix(y, x);
			bitmap.pix32(y, x) = MAKE_RGB(pal5bit(src >> 10), pal5bit(src >> 5), pal5bit(src >> 0));
		}

//  int xxx;
	/* reference */

//  time_elapsed_since_crap = frame_timer->time_elapsed();
//  xxx = screen->machine().device<device>("maincpu")->attotime_to_cycles(time_elapsed_since_crap);
//  mame_printf_debug("update cycles %d, %08x %08x\n",xxx, (UINT32)(time_elapsed_since_crap.attoseconds>>32),(UINT32)(time_elapsed_since_crap.attoseconds&0xffffffff));

	return 0;
}



static TIMER_DEVICE_CALLBACK( frame_timer_callback )
{
	/* callback */
}


// line length = 342

/*
 The V counter counts up from 00h to EAh, then it jumps back to E5h and
 continues counting up to FFh. This allows it to cover the entire 262 line
 display.

 The H counter counts up from 00h to E9h, then it jumps back to 93h and
 continues counting up to FFh. This allows it to cover an entire 342 pixel
 line.
*/

/*

 - The 80th sprite has been drawn in 40-cell mode.
 - The 64th sprite has been drawn in 32-cell mode.
 - Twenty sprites on the same scanline have been drawn in 40 cell mode.
 - Sixteen sprites on the same scanline have been drawn in 32 cell mode.
 - 320 pixels worth of sprite data has been drawn on the same scanline
   in 40 cell mode.
 - 256 pixels worth of sprite data has been drawn on the same scanline
   in 32 cell mode.
 - The currently drawn sprite has a link field of zero.

*/

/*

 $05 - Sprite Attribute Table Base Address
 -----------------------------------------

 Bits 6-0 of this register correspond to bits A15-A09 of the sprite
 attribute table.

 In 40-cell mode, A09 is always forced to zero.

*/

static void genesis_render_spriteline_to_spritebuffer(int scanline)
{
	int screenwidth;
	int maxsprites=0;
	int maxpixels=0;
	UINT16 base_address=0;



	screenwidth = MEGADRIVE_REG0C_RS0 | (MEGADRIVE_REG0C_RS1 << 1);

	switch (screenwidth&3)
	{
		case 0: maxsprites = 64; maxpixels = 256; base_address = (MEGADRIVE_REG05_SPRITE_ADDR&0x7f)<<9; break;
		case 1: maxsprites = 64; maxpixels = 256; base_address = (MEGADRIVE_REG05_SPRITE_ADDR&0x7f)<<9; break;
		case 2: maxsprites = 80; maxpixels = 320; base_address = (MEGADRIVE_REG05_SPRITE_ADDR&0x7e)<<9; break;
		case 3: maxsprites = 80; maxpixels = 320; base_address = (MEGADRIVE_REG05_SPRITE_ADDR&0x7e)<<9; break;
	}


	/* Clear our Render Buffer */
	memset(sprite_renderline, 0, 1024);


	{
		int spritenum;
		int ypos,xpos,addr;
		int drawypos;
		int /*drawwidth,*/ drawheight;
		int spritemask = 0;
		UINT8 height,width=0,link=0,xflip,yflip,colour,pri;

		/* Get Sprite Attribs */
		spritenum = 0;

		//if (scanline==40) mame_printf_debug("spritelist start base %04x\n",base_address);

		do
		{
			//UINT16 value1,value2,value3,value4;

			//value1 = megadrive_vdp_vram[((base_address>>1)+spritenum*4)+0x0];
			//value2 = megadrive_vdp_vram[((base_address>>1)+spritenum*4)+0x1];
			//value3 = megadrive_vdp_vram[((base_address>>1)+spritenum*4)+0x2];
			//value4 = megadrive_vdp_vram[((base_address>>1)+spritenum*4)+0x3];

			ypos  = (megadrive_vdp_internal_sprite_attribute_table[(spritenum*4)+0x0] & 0x01ff)>>0; /* 0x03ff? */ // puyo puyo requires 0x1ff mask, not 0x3ff, see speech bubble corners
			height= (megadrive_vdp_internal_sprite_attribute_table[(spritenum*4)+0x1] & 0x0300)>>8;
			width = (megadrive_vdp_internal_sprite_attribute_table[(spritenum*4)+0x1] & 0x0c00)>>10;
			link  = (megadrive_vdp_internal_sprite_attribute_table[(spritenum*4)+0x1] & 0x007f)>>0;
			xpos  = (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x3) & 0x01ff)>>0; /* 0x03ff? */ // pirates gold has a sprite with co-ord 0x200...

			if(megadrive_imode==3)
			{
				ypos  = (megadrive_vdp_internal_sprite_attribute_table[(spritenum*4)+0x0] & 0x03ff)>>0; /* 0x3ff requried in interlace mode (sonic 2 2 player) */
				drawypos = ypos - 256;
				drawheight = (height+1)*16;
			}
			else
			{
				ypos  = (megadrive_vdp_internal_sprite_attribute_table[(spritenum*4)+0x0] & 0x01ff)>>0; /* 0x03ff? */ // puyo puyo requires 0x1ff mask, not 0x3ff, see speech bubble corners
				drawypos = ypos - 128;
				drawheight = (height+1)*8;
			}



			//if (scanline==40) mame_printf_debug("xpos %04x ypos %04x\n",xpos,ypos);

			if ((drawypos<=scanline) && ((drawypos+drawheight)>scanline))
			{

				addr  = (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x2) & 0x07ff)>>0;
				xflip = (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x2) & 0x0800)>>11;
				yflip = (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x2) & 0x1000)>>12;
				colour= (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x2) & 0x6000)>>13;
				pri   = (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x2) & 0x8000)>>15;

				if(megadrive_imode==3)
				{
					addr<<=1;
					addr &=0x7ff;
				}

				//drawwidth = (width+1)*8;
				if (pri==1) pri = 0x80;
				else pri = 0x40;

				/* todo: fix me, I'm sure this isn't right but sprite 0 + other sprite seem to do something..
                   maybe spritemask|=2 should be set for anything < 0x40 ?*/
				if (xpos==0x00) spritemask|=1;

				//if (xpos==0x01) spritemask|=2;
				//if (xpos==0x04) spritemask|=2;  // sonic 2 title screen
				//if (xpos==0x08) spritemask|=2;  // rocket night adventures
				//if (xpos==0x10) spritemask|=2;  // mercs l1 boss
				//if (xpos==0x0a) spritemask|=2;  // legend of galahad
				//if (xpos==0x21) spritemask|=2;  // shadow of the beast?
				if ((xpos>0) && (xpos<0x40)) spritemask|=2;

				if (spritemask==0x3)
					return;
				/* end todo: */

				{
					//int xdraw;
					int xtile;
					int yline = scanline - drawypos;

					for (xtile=0;xtile<width+1;xtile++)
					{
						int dat;

						if (!xflip)
						{
							UINT16 base_addr;
							int xxx;
							UINT32 gfxdata;
							int loopcount;

							if(megadrive_imode==3)
							{
								if (!yflip) base_addr = (addr<<4)+(xtile*((height+1)*(2*16)))+(yline*2);
								else base_addr = (addr<<4)+(xtile*((height+1)*(2*16)))+((((height+1)*16)-yline-1)*2);
							}
							else
							{
								if (!yflip) base_addr = (addr<<4)+(xtile*((height+1)*(2*8)))+(yline*2);
								else base_addr = (addr<<4)+(xtile*((height+1)*(2*8)))+((((height+1)*8)-yline-1)*2);
							}

							xxx = (xpos+xtile*8)&0x1ff;

							gfxdata = MEGADRIV_VDP_VRAM(base_addr+1) | (MEGADRIV_VDP_VRAM(base_addr+0)<<16);

							for(loopcount=0;loopcount<8;loopcount++)
							{
								dat = (gfxdata & 0xf0000000)>>28; gfxdata <<=4;
								if (dat) { if (!sprite_renderline[xxx]) { sprite_renderline[xxx] = dat | (colour<<4)| pri; } else { megadrive_sprite_collision = 1; } }
								xxx++;xxx&=0x1ff;
								if (--maxpixels == 0x00) return;
							}

						}
						else
						{
							UINT16 base_addr;
							int xxx;
							UINT32 gfxdata;

							int loopcount;

							if(megadrive_imode==3)
							{
								if (!yflip) base_addr = (addr<<4)+(((width-xtile))*((height+1)*(2*16)))+(yline*2);
								else base_addr =      (addr<<4)+(((width-xtile))*((height+1)*(2*16)))+((((height+1)*16)-yline-1)*2);

							}
							else
							{
								if (!yflip) base_addr = (addr<<4)+(((width-xtile))*((height+1)*(2*8)))+(yline*2);
								else base_addr =        (addr<<4)+(((width-xtile))*((height+1)*(2*8)))+((((height+1)*8)-yline-1)*2);
							}

							xxx = (xpos+xtile*8)&0x1ff;

							gfxdata = MEGADRIV_VDP_VRAM((base_addr+1)&0x7fff) | (MEGADRIV_VDP_VRAM((base_addr+0)&0x7fff)<<16);

							for(loopcount=0;loopcount<8;loopcount++)
							{
								dat = (gfxdata & 0x0000000f)>>0; gfxdata >>=4;
								if (dat) { if (!sprite_renderline[xxx]) { sprite_renderline[xxx] = dat | (colour<<4)| pri; } else { megadrive_sprite_collision = 1; } }
								xxx++;xxx&=0x1ff;
								if (--maxpixels == 0x00) return;
							}

						}
					}
				}
			}

			spritenum = link;
			maxsprites--;
		}
		while ((maxsprites>=0) && (link!=0));


	}
}

/* Clean up this function (!) */
static void genesis_render_videoline_to_videobuffer(int scanline)
{
	UINT16 base_a;
	UINT16 base_w=0;
	UINT16 base_b;

	UINT16 size;
	UINT16 hsize = 64;
	UINT16 vsize = 64;
	UINT16 window_right;
//  UINT16 window_hpos;
	UINT16 window_down;
//  UINT16 window_vpos;
	UINT16 hscroll_base;
//  UINT8  vscroll_mode;
//  UINT8  hscroll_mode;
	int window_firstline;
	int window_lastline;
	int window_firstcol;
	int window_lastcol;
	int screenwidth;
	int numcolumns = 0;
	int hscroll_a = 0;
	int hscroll_b = 0;
	int x;
	int window_hsize=0;
	int window_vsize=0;
	int window_is_bugged = 0;
	int non_window_firstcol;
	int non_window_lastcol;
	int screenheight = MEGADRIVE_REG01_240_LINE?240:224;

	/* Clear our Render Buffer */
	for (x=0;x<320;x++)
	{
		video_renderline[x]=MEGADRIVE_REG07_BGCOLOUR | 0x20000; // mark as BG
	}

	memset(highpri_renderline, 0, 320);

	/* is this line enabled? */
	if (!MEGADRIVE_REG01_DISP_ENABLE)
	{
		//mame_printf_debug("line disabled %d\n",scanline);
		return;
	}

	/* looks different? */
	if (MEGADRIVE_REG0_DISPLAY_DISABLE)
	{
		return;
	}



	base_a = MEGADRIVE_REG02_PATTERN_ADDR_A << 13;

	base_b = MEGADRIVE_REG04_PATTERN_ADDR_B << 13;
	size  = MEGADRIVE_REG10_HSCROLL_SIZE | (MEGADRIVE_REG10_VSCROLL_SIZE<<4);
	window_right = MEGADRIVE_REG11_WINDOW_RIGHT;
//  window_hpos = MEGADRIVE_REG11_WINDOW_HPOS;
	window_down = MEGADRIVE_REG12_WINDOW_DOWN;
//  window_vpos = MEGADRIVE_REG12_WINDOW_VPOS;

	screenwidth = MEGADRIVE_REG0C_RS0 | (MEGADRIVE_REG0C_RS1 << 1);

	switch (screenwidth)
	{
		case 0: numcolumns = 32; window_hsize = 32; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1f) << 11; break;
		case 1: numcolumns = 32; window_hsize = 32; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1f) << 11; break;
		case 2: numcolumns = 40; window_hsize = 64; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1e) << 11; break;
		case 3: numcolumns = 40; window_hsize = 64; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1e) << 11; break; // talespin cares about base mask, used for status bar
	}

	//mame_printf_debug("screenwidth %d\n",screenwidth);

	//base_w = Machine->rand()&0xff;

	/* Calculate Exactly where we're going to draw the Window, and if the Window Bug applies */
	window_is_bugged = 0;
	if (window_right)
	{
		window_firstcol = MEGADRIVE_REG11_WINDOW_HPOS*16;
		window_lastcol = numcolumns*8;
		if (window_firstcol>window_lastcol) window_firstcol = window_lastcol;

		non_window_firstcol = 0;
		non_window_lastcol = window_firstcol;
	}
	else
	{
		window_firstcol = 0;
		window_lastcol = MEGADRIVE_REG11_WINDOW_HPOS*16;
		if (window_lastcol>numcolumns*8) window_lastcol = numcolumns*8;

		non_window_firstcol = window_lastcol;
		non_window_lastcol = numcolumns*8;

		if (window_lastcol!=0) window_is_bugged=1;
	}

	if (window_down)
	{
		window_firstline = MEGADRIVE_REG12_WINDOW_VPOS*8;
		window_lastline = screenheight; // 240 in PAL?
		if (window_firstline>screenheight) window_firstline = screenheight;
	}
	else
	{
		window_firstline = 0;
		window_lastline = MEGADRIVE_REG12_WINDOW_VPOS*8;
		if (window_lastline>screenheight) window_lastline = screenheight;
	}

	/* if we're on a window scanline between window_firstline and window_lastline the window is the full width of the screen */
	if (scanline>=window_firstline && scanline < window_lastline)
	{
		window_firstcol = 0; window_lastcol = numcolumns*8; // window is full-width of the screen
		non_window_firstcol = 0; non_window_lastcol=0; // disable non-window
	}


//    vscroll_mode = MEGADRIVE_REG0B_VSCROLL_MODE;
//    hscroll_mode = MEGADRIVE_REG0B_HSCROLL_MODE;
    hscroll_base = MEGADRIVE_REG0D_HSCROLL_ADDR<<10;

	switch (size)
	{
		case 0x00: hsize = 32; vsize = 32; break;
		case 0x01: hsize = 64; vsize = 32; break;
		case 0x02: hsize = 64; vsize = 1; /* mame_printf_debug("Invalid HSize! %02x\n",size);*/ break;
		case 0x03: hsize = 128;vsize = 32; break;

		case 0x10: hsize = 32; vsize = 64; break;
		case 0x11: hsize = 64; vsize = 64; break;
		case 0x12: hsize = 64; vsize = 1; /*mame_printf_debug("Invalid HSize! %02x\n",size);*/ break;
		case 0x13: hsize = 128;vsize = 32;/*mame_printf_debug("Invalid Total Size! %02x\n",size);*/break;

		case 0x20: hsize = 32; vsize = 64; mame_printf_debug("Invalid VSize!\n"); break;
		case 0x21: hsize = 64; vsize = 64; mame_printf_debug("Invalid VSize!\n"); break;
		case 0x22: hsize = 64; vsize = 1; /*mame_printf_debug("Invalid HSize & Invalid VSize!\n");*/ break;
		case 0x23: hsize = 128;vsize = 64; mame_printf_debug("Invalid VSize!\n"); break;

		case 0x30: hsize = 32; vsize = 128; break;
		case 0x31: hsize = 64; vsize = 64; /*mame_printf_debug("Invalid Total Size! %02x\n",size);*/break; // super skidmarks attempts this..
		case 0x32: hsize = 64; vsize = 1; /*mame_printf_debug("Invalid HSize & Invalid Total Size!\n");*/ break;
		case 0x33: hsize = 128;vsize = 128; mame_printf_debug("Invalid Total Size! %02x\n",size);break;
	}

	switch (MEGADRIVE_REG0B_HSCROLL_MODE)
	{
		case 0x00: // Full Screen Scroll
			hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0);
			hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1);
			break;

		case 0x01: // 'Broken' Line Scroll
			if(megadrive_imode==3)
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+((scanline>>1)&7)*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+((scanline>>1)&7)*2);
			}
			else
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+(scanline&7)*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+(scanline&7)*2);
			}
			break;

		case 0x02: // Cell Scroll
			if(megadrive_imode==3)
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+((scanline>>1)&~7)*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+((scanline>>1)&~7)*2);
			}
			else
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+(scanline&~7)*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+(scanline&~7)*2);
			}
			break;

		case 0x03: // Full Line Scroll
			if(megadrive_imode==3)
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+(scanline>>1)*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+(scanline>>1)*2);
			}
			else
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+scanline*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+scanline*2);
			}
			break;
	}

	/* Low Priority B Tiles */
	{
		int column;
		int vscroll;

		for (column=0;column<numcolumns/2;column++)
		{	/* 20x 16x1 blocks */
			int vcolumn;
			int dpos;

			/* Get V Scroll Value for this block */

			dpos = column*16;

			{
				/* hscroll is not divisible by 8, this segment will contain 3 tiles, 1 partial, 1 whole, 1 partial */
				int hscroll_part = 8-(hscroll_b%8);
				int hcolumn;
				int tile_base;
				int tile_dat;
				int tile_addr;
				int tile_xflip;
				int tile_yflip;
				int tile_colour;
				int tile_pri;
				int dat;

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					if (hscroll_b&0xf) vscroll = megadrive_vdp_vsram[((column-1)*2+1)&0x3f];
					else vscroll = megadrive_vdp_vsram[((column)*2+1)&0x3f];
				}
				else
				{
					vscroll = megadrive_vdp_vsram[1];
				}

				hcolumn = ((column*2-1)-(hscroll_b>>3))&(hsize-1);

				if(megadrive_imode==3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
					tile_base = (base_b>>1)+((vcolumn>>4)*hsize)+hcolumn;

				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
					tile_base = (base_b>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}


				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);

				if(megadrive_imode==3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;
					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}

				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=hscroll_part;shift<8;shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;  if (!tile_pri) { if(dat) video_renderline[dpos] = dat | (tile_colour<<4); }  else highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=hscroll_part;shift<8;shift++)
					{
						dat = (gfxdata>>(shift*4) )&0x000f;  if (!tile_pri) { if(dat) video_renderline[dpos] = dat | (tile_colour<<4); }  else highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					if (hscroll_b&0xf) vscroll = megadrive_vdp_vsram[((column-1)*2+1)&0x3f];
					else vscroll = megadrive_vdp_vsram[((column)*2+1)&0x3f];
				}
				else
				{
					vscroll = megadrive_vdp_vsram[1];
				}

				hcolumn = ((column*2)-(hscroll_b>>3))&(hsize-1);

				if(megadrive_imode==3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
					tile_base = (base_b>>1)+((vcolumn>>4)*hsize)+hcolumn;
				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
					tile_base = (base_b>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}

				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);

				if(megadrive_imode==3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;

					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}

				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=0;shift<8;shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;  if (!tile_pri) { if(dat) video_renderline[dpos] = dat | (tile_colour<<4); }  else highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=0;shift<8;shift++)
					{
						dat = (gfxdata>>(shift*4))&0x000f;  if (!tile_pri) { if(dat) video_renderline[dpos] = dat | (tile_colour<<4); }  else highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					vscroll = megadrive_vdp_vsram[((column)*2+1)&0x3f];
				}
				else
				{
					vscroll = megadrive_vdp_vsram[1];
				}

				hcolumn = ((column*2+1)-(hscroll_b>>3))&(hsize-1);

				if(megadrive_imode==3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
					tile_base = (base_b>>1)+((vcolumn>>4)*hsize)+hcolumn;
				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
					tile_base = (base_b>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}

				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);

				if(megadrive_imode==3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;
					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}


				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=0;shift<(hscroll_part);shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;  if (!tile_pri) { if(dat) video_renderline[dpos] = dat | (tile_colour<<4); }  else highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=0;shift<(hscroll_part);shift++)
					{
						dat = (gfxdata>>(shift*4) )&0x000f;  if (!tile_pri) { if(dat) video_renderline[dpos] = dat | (tile_colour<<4); }  else highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}
			}
		}
		/* END */
	}
	/* Low Priority A Tiles + Window(!) */

	{
		int column;
		int vscroll;

		for (column=window_firstcol/16;column<window_lastcol/16;column++)
		{
			int vcolumn;
			int dpos;

			int hcolumn;
			int tile_base;
			int tile_dat;
			int tile_addr;
			int tile_xflip;
			int tile_yflip;
			int tile_colour;
			int tile_pri;
			int dat;

			vcolumn = scanline&((window_vsize*8)-1);
			dpos = column*16;
			hcolumn = (column*2)&(window_hsize-1);

			if(megadrive_imode==3)
			{
				tile_base = (base_w>>1)+((vcolumn>>4)*window_hsize)+hcolumn;
			}
			else
			{
				tile_base = (base_w>>1)+((vcolumn>>3)*window_hsize)+hcolumn;
			}

			tile_base &=0x7fff;
			tile_dat = MEGADRIV_VDP_VRAM(tile_base);
			tile_xflip = (tile_dat&0x0800);
			tile_yflip = (tile_dat&0x1000);
			tile_colour =(tile_dat&0x6000)>>13;
			tile_pri = (tile_dat&0x8000)>>15;
			tile_addr = ((tile_dat&0x07ff)<<4);

			if(megadrive_imode==3)
			{
				tile_addr <<=1;
				tile_addr &=0x7fff;
			}

			if(megadrive_imode==3)
			{
				if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
				else tile_addr+=((0xf-vcolumn)&0xf)*2;
			}
			else
			{
				if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
				else tile_addr+=((7-vcolumn)&7)*2;
			}

			if (!tile_xflip)
			{
				/* 8 pixels */
				UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
				int shift;

				for (shift=0;shift<8;shift++)
				{
					dat = (gfxdata>>(28-(shift*4)))&0x000f;
					if (!tile_pri)
					{
						if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
					}
					else
					{
						if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
					}
					dpos++;
				}
			}
			else
			{
				UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
				int shift;
				for (shift=0;shift<8;shift++)
				{
					dat = (gfxdata>>(shift*4) )&0x000f;
					if (!tile_pri)
					{
						if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
					}
					else
					{
						if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
					}
					dpos++;

				}
			}


			hcolumn = (column*2+1)&(window_hsize-1);
			if(megadrive_imode==3)
			{
				tile_base = (base_w>>1)+((vcolumn>>4)*window_hsize)+hcolumn;
			}
			else
			{
				tile_base = (base_w>>1)+((vcolumn>>3)*window_hsize)+hcolumn;
			}
			tile_base &=0x7fff;
			tile_dat = MEGADRIV_VDP_VRAM(tile_base);
			tile_xflip = (tile_dat&0x0800);
			tile_yflip = (tile_dat&0x1000);
			tile_colour =(tile_dat&0x6000)>>13;
			tile_pri = (tile_dat&0x8000)>>15;
			tile_addr = ((tile_dat&0x07ff)<<4);

			if(megadrive_imode==3)
			{
				tile_addr <<=1;
				tile_addr &=0x7fff;
			}

			if(megadrive_imode==3)
			{
				if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
				else tile_addr+=((0xf-vcolumn)&0xf)*2;
			}
			else
			{
				if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
				else tile_addr+=((7-vcolumn)&7)*2;
			}

			if (!tile_xflip)
			{
				/* 8 pixels */
				UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
				int shift;

				for (shift=0;shift<8;shift++)
				{
					dat = (gfxdata>>(28-(shift*4)))&0x000f;
					if (!tile_pri)
					{
						if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
					}
					else
					{
						if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
					}
					dpos++;
				}
			}
			else
			{
				UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
				int shift;
				for (shift=0;shift<8;shift++)
				{
					dat = (gfxdata>>(shift*4) )&0x000f;
					if (!tile_pri)
					{
						if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
					}
					else
					{
						if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
					}
					dpos++;
				}
			}
		}

		/* Non Window Part */

		for (column=non_window_firstcol/16;column<non_window_lastcol/16;column++)
		{	/* 20x 16x1 blocks */
		//  int xx;
			int vcolumn;
			int dpos;

			dpos = column*16;

			{	/* hscroll is not divisible by 8, this segment will contain 3 tiles, 1 partial, 1 whole, 1 partial */
				int hscroll_part = 8-(hscroll_a%8);
				int hcolumn;
				int tile_base;
				int tile_dat;
				int tile_addr;
				int tile_xflip;
				int tile_yflip;
				int tile_colour;
				int tile_pri;
				int dat;

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					if (hscroll_a&0xf) vscroll = megadrive_vdp_vsram[((column-1)*2+0)&0x3f];
					else vscroll = megadrive_vdp_vsram[((column)*2+0)&0x3f];
				}
				else
				{
					vscroll = megadrive_vdp_vsram[0];
				}


				if ((!window_is_bugged) || ((hscroll_a&0xf)==0) || (column>non_window_firstcol/16)) hcolumn = ((column*2-1)-(hscroll_a>>3))&(hsize-1);
				else hcolumn = ((column*2+1)-(hscroll_a>>3))&(hsize-1);

				if(megadrive_imode==3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
				}

				if(megadrive_imode==3)
				{
					tile_base = (base_a>>1)+((vcolumn>>4)*hsize)+hcolumn;
				}
				else
				{
					tile_base = (base_a>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}


				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);

				if(megadrive_imode==3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;
					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}

				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=hscroll_part;shift<8;shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;
						if (!tile_pri)
						{
							if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=hscroll_part;shift<8;shift++)
					{
						dat = (gfxdata>>(shift*4) )&0x000f;
						if (!tile_pri)
						{
							if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					if (hscroll_a&0xf) vscroll = megadrive_vdp_vsram[((column-1)*2+0)&0x3f];
					else vscroll = megadrive_vdp_vsram[((column)*2+0)&0x3f];
				}
				else
				{
					vscroll = megadrive_vdp_vsram[0];
				}

				if ((!window_is_bugged) || ((hscroll_a&0xf)==0) || (column>non_window_firstcol/16)) hcolumn = ((column*2)-(hscroll_a>>3))&(hsize-1); // not affected by bug?
				else
				{
					if ((hscroll_a&0xf)<8) hcolumn = ((column*2)-(hscroll_a>>3))&(hsize-1);
					else hcolumn = ((column*2+2)-(hscroll_a>>3))&(hsize-1);
				}


				if(megadrive_imode==3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
					tile_base = (base_a>>1)+((vcolumn>>4)*hsize)+hcolumn;
				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
					tile_base = (base_a>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}

				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);


				if(megadrive_imode==3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;
					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}

				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=0;shift<8;shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;
						if (!tile_pri)
						{
							if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=0;shift<8;shift++)
					{
						dat = (gfxdata>>(shift*4) )&0x000f;
						if (!tile_pri)
						{
							if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					vscroll = megadrive_vdp_vsram[((column)*2+0)&0x3f];
				}
				else
				{
					vscroll = megadrive_vdp_vsram[0];
				}

				if ((!window_is_bugged) || ((hscroll_a&0xf)==0) || (column>non_window_firstcol/16)) hcolumn = ((column*2+1)-(hscroll_a>>3))&(hsize-1);
				else hcolumn = ((column*2+1)-(hscroll_a>>3))&(hsize-1);

				if(megadrive_imode==3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
					tile_base = (base_a>>1)+((vcolumn>>4)*hsize)+hcolumn;
				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
					tile_base = (base_a>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}
				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);

				if(megadrive_imode==3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;
				}

				if(megadrive_imode==3)
				{
					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}

				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=0;shift<(hscroll_part);shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;
						if (!tile_pri)
						{
							if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=0;shift<(hscroll_part);shift++)
					{
						dat = (gfxdata>>(shift*4) )&0x000f;
						if (!tile_pri)
						{
							if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}
			}
		}
	}
		/* END */

/* MEGADRIVE_REG0C_SHADOW_HIGLIGHT */
		/* Low Priority Sprites */
		for (x=0;x<320;x++)
		{
			if (!MEGADRIVE_REG0C_SHADOW_HIGLIGHT)
			{
				if (sprite_renderline[x+128] & 0x40)
				{
					video_renderline[x] = sprite_renderline[x+128]&0x3f;
					video_renderline[x] |= 0x10000; // mark as sprite pixel
				}
			}
			else
			{	/* Special Shadow / Highlight processing */

				if (sprite_renderline[x+128] & 0x40)
				{
					UINT8 spritedata;
					spritedata = sprite_renderline[x+128]&0x3f;

					if ((spritedata==0x0e) || (spritedata==0x1e) || (spritedata==0x2e))
					{
						/* BUG in sprite chip, these colours are always normal intensity */
						video_renderline[x] = spritedata | 0x4000;
						video_renderline[x] |= 0x10000; // mark as sprite pixel
					}
					else if (spritedata==0x3e)
					{
						/* Everything below this is half colour, mark with 0x8000 to mark highlight' */
						video_renderline[x] = video_renderline[x]|0x8000; // spiderwebs..
					}
					else if (spritedata==0x3f)
					{
						/* This is a Shadow operator, but everything below is already low pri, no effect */
						video_renderline[x] = video_renderline[x]|0x2000;

					}
					else
					{
						video_renderline[x] = spritedata;
						video_renderline[x] |= 0x10000; // mark as sprite pixel
					}

				}
			}
		}
		/* High Priority A+B Tiles */
		for (x=0;x<320;x++)
		{
			if (!MEGADRIVE_REG0C_SHADOW_HIGLIGHT)
			{
				/* Normal Processing */
				int dat;
				dat = highpri_renderline[x];

				if (dat&0x80)
				{
					 if (dat&0x0f) video_renderline[x] = highpri_renderline[x]&0x3f;
				}
			}
			else
			{
				/* Shadow / Highlight Mode */
				int dat;
				dat = highpri_renderline[x];

				if (dat&0x80)
				{
					 if (dat&0x0f) video_renderline[x] = (highpri_renderline[x]&0x3f) | 0x4000;
					 else video_renderline[x] = video_renderline[x] | 0x4000; // set 'normal'
				}
			}
		}

		/* High Priority Sprites */
		for (x=0;x<320;x++)
		{
			if (!MEGADRIVE_REG0C_SHADOW_HIGLIGHT)
			{
				/* Normal */
				if (sprite_renderline[x+128] & 0x80)
				{
					video_renderline[x] = sprite_renderline[x+128]&0x3f;
					video_renderline[x] |= 0x10000; // mark as sprite pixel
				}
			}
			else
			{
				if (sprite_renderline[x+128] & 0x80)
				{
					UINT8 spritedata;
					spritedata = sprite_renderline[x+128]&0x3f;

					if (spritedata==0x3e)
					{
						/* set flag 0x8000 to indicate highlight */
						video_renderline[x] = video_renderline[x]|0x8000;
					}
					else if (spritedata==0x3f)
					{
						/* This is a Shadow operator set shadow bit */
						video_renderline[x] = video_renderline[x]|0x2000;
					}
					else
					{
						video_renderline[x] = spritedata | 0x4000;
						video_renderline[x] |= 0x10000; // mark as sprite pixel
					}
				}
			}
		}
}

static UINT32 _32x_linerender[320+258]; // tmp buffer (bigger than it needs to be to simplify RLE decode)

/* This converts our render buffer to real screen colours */
static void genesis_render_videobuffer_to_screenbuffer(running_machine &machine, int scanline)
{
	UINT16*lineptr;
	int x;
	lineptr = &render_bitmap->pix16(scanline);

	/* render 32x output to a buffer */
	if (_32x_is_connected && (_32x_displaymode != 0))
	{
		if (_32x_displaymode==1)
		{

			UINT32 lineoffs;
			int start;

			lineoffs = _32x_display_dram[scanline];

			if (_32x_screenshift == 0) start=0;
			else start = -1;

			for (x=start;x<320;x++)
			{
				UINT16 coldata;
				coldata = _32x_display_dram[lineoffs];

				{
					if (x>=0)
					{
						_32x_linerender[x] = _32x_palette_lookup[(coldata & 0xff00)>>8];
					}

					x++;

					if (x>=0)
					{
						_32x_linerender[x] = _32x_palette_lookup[(coldata & 0x00ff)];
					}
				}

				lineoffs++;

			}
		}
		else if (_32x_displaymode==3) // mode 3 = RLE  (used by BRUTAL intro)
		{
			UINT32 lineoffs;
			int start;

			lineoffs = _32x_display_dram[scanline];

			if (_32x_screenshift == 0) start=0;
			else start = -1;

            x = start;
			while (x<320)
			{
				UINT16 coldata, length, l;
				coldata = _32x_display_dram[lineoffs];
				length = ((coldata & 0xff00)>>8)+1;
				coldata = (coldata & 0x00ff)>>0;
				for (l=0;l<length;l++)
				{
					if (x>=0)
					{
						_32x_linerender[x] = _32x_palette_lookup[(coldata)];
					}
					x++;
				}

				lineoffs++;

			}
		}
		else // MODE 2 - 15bpp mode, not used by any commercial games?
		{
			UINT32 lineoffs;
			int start;

			lineoffs = _32x_display_dram[scanline];

			if (_32x_screenshift == 0) start=0;
			else start = -1;

            x = start;
			while (x<320)
			{
				UINT16 coldata;
				coldata = _32x_display_dram[lineoffs&0xffff];

				// need to swap red and blue around for MAME
				{
					int r = ((coldata >> 0)  & 0x1f);
					int g = ((coldata >> 5)  & 0x1f);
					int b = ((coldata >> 10) & 0x1f);
					int p = ((coldata >> 15) & 0x01); // priority 'through' bit

					coldata = (r << 10) | (g << 5) | (b << 0) | (p << 15);

				}

				if (x>=0)
					_32x_linerender[x] = coldata;

				x++;
				lineoffs++;
			}
		}
	}

	int _32x_priority = _32x_videopriority;


	if (!MEGADRIVE_REG0C_SHADOW_HIGLIGHT)
	{

		for (x=0;x<320;x++)
		{
			UINT32 dat;
			dat = video_renderline[x];
			int drawn = 0;

			// low priority 32x - if it's the bg pen, we have a 32x, and it's display is enabled...
			if ((dat&0x20000) && (_32x_is_connected) && (_32x_displaymode != 0))
			{
				if (!_32x_priority)
				{
					if (!(_32x_linerender[x]&0x8000))
					{
						lineptr[x] = _32x_linerender[x]&0x7fff;
						drawn = 1;
					}
				}
				else
				{
					if ((_32x_linerender[x]&0x8000))
					{
						lineptr[x] = _32x_linerender[x]&0x7fff;
						drawn = 1;
					}
				}
			}


			if (drawn==0)
			{
				if (dat&0x10000)
					lineptr[x] = megadrive_vdp_palette_lookup_sprite[(dat&0x0f) | segac2_sp_pal_lookup[(dat&0x30)>>4]];
				else
					lineptr[x] = megadrive_vdp_palette_lookup[(dat&0x0f) | segac2_bg_pal_lookup[(dat&0x30)>>4]];
			}



		}
	}
	else
	{

		for (x=0;x<320;x++)
		{
			UINT32 dat;
			dat = video_renderline[x];

			int drawn = 0;

			// low priority 32x - if it's the bg pen, we have a 32x, and it's display is enabled...
			if ((dat&0x20000) && (_32x_is_connected) && (_32x_displaymode != 0))
			{
				if (!_32x_priority)
				{
					if (!(_32x_linerender[x]&0x8000))
					{
						lineptr[x] = _32x_linerender[x]&0x7fff;
						drawn = 1;
					}
				}
				else
				{
					if ((_32x_linerender[x]&0x8000))
					{
						lineptr[x] = _32x_linerender[x]&0x7fff;
						drawn = 1;
					}
				}
			}


			if (drawn==0)
			{
				/* Verify my handling.. I'm not sure all cases are correct */
				switch (dat&0x1e000)
				{
					case 0x00000: // low priority, no shadow sprite, no highlight = shadow
					case 0x02000: // low priority, shadow sprite, no highlight = shadow
					case 0x06000: // normal pri,   shadow sprite, no highlight = shadow?
					case 0x10000: // (sprite) low priority, no shadow sprite, no highlight = shadow
					case 0x12000: // (sprite) low priority, shadow sprite, no highlight = shadow
					case 0x16000: // (sprite) normal pri,   shadow sprite, no highlight = shadow?
						lineptr[x] = megadrive_vdp_palette_lookup_shadow[(dat&0x0f)  | segac2_bg_pal_lookup[(dat&0x30)>>4]];
						break;

					case 0x4000: // normal pri, no shadow sprite, no highlight = normal;
					case 0x8000: // low pri, highlight sprite = normal;
						lineptr[x] = megadrive_vdp_palette_lookup[(dat&0x0f)  | segac2_bg_pal_lookup[(dat&0x30)>>4]];
						break;

					case 0x14000: // (sprite) normal pri, no shadow sprite, no highlight = normal;
					case 0x18000: // (sprite) low pri, highlight sprite = normal;
						lineptr[x] = megadrive_vdp_palette_lookup_sprite[(dat&0x0f)  | segac2_sp_pal_lookup[(dat&0x30)>>4]];
						break;


					case 0x0c000: // normal pri, highlight set = highlight?
					case 0x1c000: // (sprite) normal pri, highlight set = highlight?
						lineptr[x] = megadrive_vdp_palette_lookup_highlight[(dat&0x0f) | segac2_bg_pal_lookup[(dat&0x30)>>4]];
						break;

					case 0x0a000: // shadow set, highlight set - not possible
					case 0x0e000: // shadow set, highlight set, normal set, not possible
					case 0x1a000: // (sprite)shadow set, highlight set - not possible
					case 0x1e000: // (sprite)shadow set, highlight set, normal set, not possible
					default:
						lineptr[x] = machine.rand()&0x3f;
					break;
				}
			}



		}

	}


	// high priority 32x
	if ((_32x_is_connected) && (_32x_displaymode != 0))
	{
		for (x=0;x<320;x++)
		{
			if (!_32x_priority)
			{
				if ((_32x_linerender[x]&0x8000))
					lineptr[x] = _32x_linerender[x]&0x7fff;
			}
			else
			{
				if (!(_32x_linerender[x]&0x8000))
					lineptr[x] = _32x_linerender[x]&0x7fff;
			}
		}
	}
}

static void genesis_render_scanline(running_machine &machine, int scanline)
{
	//if (MEGADRIVE_REG01_DMA_ENABLE==0) mame_printf_debug("off\n");
	genesis_render_spriteline_to_spritebuffer(genesis_scanline_counter);
	genesis_render_videoline_to_videobuffer(scanline);
	genesis_render_videobuffer_to_screenbuffer(machine, scanline);
}

INLINE UINT16 get_hposition(void)
{
//  static int lowest = 99999;
//  static int highest = -99999;

	attotime time_elapsed_since_scanline_timer;
	UINT16 value4;

	time_elapsed_since_scanline_timer = scanline_timer->time_elapsed();

	if (time_elapsed_since_scanline_timer.attoseconds<(ATTOSECONDS_PER_SECOND/megadriv_framerate /megadrive_total_scanlines))
	{
		value4 = (UINT16)(megadrive_max_hposition*((double)(time_elapsed_since_scanline_timer.attoseconds) / (double)(ATTOSECONDS_PER_SECOND/megadriv_framerate /megadrive_total_scanlines)));
	}
	else /* in some cases (probably due to rounding errors) we get some stupid results (the odd huge value where the time elapsed is much higher than the scanline time??!).. hopefully by clamping the result to the maximum we limit errors */
	{
		value4 = megadrive_max_hposition;
	}

//  if (value4>highest) highest = value4;
//  if (value4<lowest) lowest = value4;

	//mame_printf_debug("%d low %d high %d scancounter %d\n", value4, lowest, highest,genesis_scanline_counter);

	return value4;
}

/* This is what our timing values look like

--xxxxx-------- cycles 0,     00000000 00000000 counter 0
--xxxxx-------- cycles 487,   000039db 1c11d6a2 counter 1
--xxxxx-------- cycles 975,   000073b6 3823ad44 counter 2
--xxxxx-------- cycles 1463,  0000ad91 543583e6 counter 3
--xxxxx-------- cycles 1951,  0000e76c 70475a88 counter 4
--xxxxx-------- cycles 2439,  00012147 8c59312a counter 5
--xxxxx-------- cycles 2927,  00015b22 a86b07cc counter 6
--xxxxx-------- cycles 3415,  000194fd c47cde6e counter 7
--xxxxx-------- cycles 3903,  0001ced8 e08eb510 counter 8
--xxxxx-------- cycles 4391,  000208b3 fca08bb2 counter 9
--xxxxx-------- cycles 4879,  0002428f 18b26254 counter 10
--xxxxx-------- cycles 5367,  00027c6a 34c438f6 counter 11
--xxxxx-------- cycles 5855,  0002b645 50d60f98 counter 12
--xxxxx-------- cycles 6343,  0002f020 6ce7e63a counter 13
--xxxxx-------- cycles 6831,  000329fb 88f9bcdc counter 14
--xxxxx-------- cycles 7319,  000363d6 a50b937e counter 15
--xxxxx-------- cycles 7807,  00039db1 c11d6a20 counter 16
--xxxxx-------- cycles 8295,  0003d78c dd2f40c2 counter 17
--xxxxx-------- cycles 8782,  00041167 f9411764 counter 18
--xxxxx-------- cycles 9270,  00044b43 1552ee06 counter 19
--xxxxx-------- cycles 9758,  0004851e 3164c4a8 counter 20
--xxxxx-------- cycles 10246, 0004bef9 4d769b4a counter 21
--xxxxx-------- cycles 10734, 0004f8d4 698871ec counter 22
--xxxxx-------- cycles 11222, 000532af 859a488e counter 23
--xxxxx-------- cycles 11710, 00056c8a a1ac1f30 counter 24
--xxxxx-------- cycles 12198, 0005a665 bdbdf5d2 counter 25
--xxxxx-------- cycles 12686, 0005e040 d9cfcc74 counter 26
--xxxxx-------- cycles 13174, 00061a1b f5e1a316 counter 27
--xxxxx-------- cycles 13662, 000653f7 11f379b8 counter 28
--xxxxx-------- cycles 14150, 00068dd2 2e05505a counter 29
--xxxxx-------- cycles 14638, 0006c7ad 4a1726fc counter 30
--xxxxx-------- cycles 15126, 00070188 6628fd9e counter 31
--xxxxx-------- cycles 15614, 00073b63 823ad440 counter 32
--xxxxx-------- cycles 16102, 0007753e 9e4caae2 counter 33
--xxxxx-------- cycles 16590, 0007af19 ba5e8184 counter 34
--xxxxx-------- cycles 17077, 0007e8f4 d6705826 counter 35
--xxxxx-------- cycles 17565, 000822cf f2822ec8 counter 36
--xxxxx-------- cycles 18053, 00085cab 0e94056a counter 37
--xxxxx-------- cycles 18541, 00089686 2aa5dc0c counter 38
--xxxxx-------- cycles 19029, 0008d061 46b7b2ae counter 39
--xxxxx-------- cycles 19517, 00090a3c 62c98950 counter 40
--xxxxx-------- cycles 20005, 00094417 7edb5ff2 counter 41
--xxxxx-------- cycles 20493, 00097df2 9aed3694 counter 42
--xxxxx-------- cycles 20981, 0009b7cd b6ff0d36 counter 43
--xxxxx-------- cycles 21469, 0009f1a8 d310e3d8 counter 44
--xxxxx-------- cycles 21957, 000a2b83 ef22ba7a counter 45
--xxxxx-------- cycles 22445, 000a655f 0b34911c counter 46
--xxxxx-------- cycles 22933, 000a9f3a 274667be counter 47
--xxxxx-------- cycles 23421, 000ad915 43583e60 counter 48
--xxxxx-------- cycles 23909, 000b12f0 5f6a1502 counter 49
--xxxxx-------- cycles 24397, 000b4ccb 7b7beba4 counter 50
--xxxxx-------- cycles 24885, 000b86a6 978dc246 counter 51
--xxxxx-------- cycles 25372, 000bc081 b39f98e8 counter 52
--xxxxx-------- cycles 25860, 000bfa5c cfb16f8a counter 53
--xxxxx-------- cycles 26348, 000c3437 ebc3462c counter 54
--xxxxx-------- cycles 26836, 000c6e13 07d51cce counter 55
--xxxxx-------- cycles 27324, 000ca7ee 23e6f370 counter 56
--xxxxx-------- cycles 27812, 000ce1c9 3ff8ca12 counter 57
--xxxxx-------- cycles 28300, 000d1ba4 5c0aa0b4 counter 58
--xxxxx-------- cycles 28788, 000d557f 781c7756 counter 59
--xxxxx-------- cycles 29276, 000d8f5a 942e4df8 counter 60
--xxxxx-------- cycles 29764, 000dc935 b040249a counter 61
--xxxxx-------- cycles 30252, 000e0310 cc51fb3c counter 62
--xxxxx-------- cycles 30740, 000e3ceb e863d1de counter 63
--xxxxx-------- cycles 31228, 000e76c7 0475a880 counter 64
--xxxxx-------- cycles 31716, 000eb0a2 20877f22 counter 65
--xxxxx-------- cycles 32204, 000eea7d 3c9955c4 counter 66
--xxxxx-------- cycles 32692, 000f2458 58ab2c66 counter 67
--xxxxx-------- cycles 33180, 000f5e33 74bd0308 counter 68
--xxxxx-------- cycles 33667, 000f980e 90ced9aa counter 69
--xxxxx-------- cycles 34155, 000fd1e9 ace0b04c counter 70
--xxxxx-------- cycles 34643, 00100bc4 c8f286ee counter 71
--xxxxx-------- cycles 35131, 0010459f e5045d90 counter 72
--xxxxx-------- cycles 35619, 00107f7b 01163432 counter 73
--xxxxx-------- cycles 36107, 0010b956 1d280ad4 counter 74
--xxxxx-------- cycles 36595, 0010f331 3939e176 counter 75
--xxxxx-------- cycles 37083, 00112d0c 554bb818 counter 76
--xxxxx-------- cycles 37571, 001166e7 715d8eba counter 77
--xxxxx-------- cycles 38059, 0011a0c2 8d6f655c counter 78
--xxxxx-------- cycles 38547, 0011da9d a9813bfe counter 79
--xxxxx-------- cycles 39035, 00121478 c59312a0 counter 80
--xxxxx-------- cycles 39523, 00124e53 e1a4e942 counter 81
--xxxxx-------- cycles 40011, 0012882e fdb6bfe4 counter 82
--xxxxx-------- cycles 40499, 0012c20a 19c89686 counter 83
--xxxxx-------- cycles 40987, 0012fbe5 35da6d28 counter 84
--xxxxx-------- cycles 41475, 001335c0 51ec43ca counter 85
--xxxxx-------- cycles 41962, 00136f9b 6dfe1a6c counter 86
--xxxxx-------- cycles 42450, 0013a976 8a0ff10e counter 87
--xxxxx-------- cycles 42938, 0013e351 a621c7b0 counter 88
--xxxxx-------- cycles 43426, 00141d2c c2339e52 counter 89
--xxxxx-------- cycles 43914, 00145707 de4574f4 counter 90
--xxxxx-------- cycles 44402, 001490e2 fa574b96 counter 91
--xxxxx-------- cycles 44890, 0014cabe 16692238 counter 92
--xxxxx-------- cycles 45378, 00150499 327af8da counter 93
--xxxxx-------- cycles 45866, 00153e74 4e8ccf7c counter 94
--xxxxx-------- cycles 46354, 0015784f 6a9ea61e counter 95
--xxxxx-------- cycles 46842, 0015b22a 86b07cc0 counter 96
--xxxxx-------- cycles 47330, 0015ec05 a2c25362 counter 97
--xxxxx-------- cycles 47818, 001625e0 bed42a04 counter 98
--xxxxx-------- cycles 48306, 00165fbb dae600a6 counter 99
--xxxxx-------- cycles 48794, 00169996 f6f7d748 counter 100
--xxxxx-------- cycles 49282, 0016d372 1309adea counter 101
--xxxxx-------- cycles 49770, 00170d4d 2f1b848c counter 102
--xxxxx-------- cycles 50257, 00174728 4b2d5b2e counter 103
--xxxxx-------- cycles 50745, 00178103 673f31d0 counter 104
--xxxxx-------- cycles 51233, 0017bade 83510872 counter 105
--xxxxx-------- cycles 51721, 0017f4b9 9f62df14 counter 106
--xxxxx-------- cycles 52209, 00182e94 bb74b5b6 counter 107
--xxxxx-------- cycles 52697, 0018686f d7868c58 counter 108
--xxxxx-------- cycles 53185, 0018a24a f39862fa counter 109
--xxxxx-------- cycles 53673, 0018dc26 0faa399c counter 110
--xxxxx-------- cycles 54161, 00191601 2bbc103e counter 111
--xxxxx-------- cycles 54649, 00194fdc 47cde6e0 counter 112
--xxxxx-------- cycles 55137, 001989b7 63dfbd82 counter 113
--xxxxx-------- cycles 55625, 0019c392 7ff19424 counter 114
--xxxxx-------- cycles 56113, 0019fd6d 9c036ac6 counter 115
--xxxxx-------- cycles 56601, 001a3748 b8154168 counter 116
--xxxxx-------- cycles 57089, 001a7123 d427180a counter 117
--xxxxx-------- cycles 57577, 001aaafe f038eeac counter 118
--xxxxx-------- cycles 58065, 001ae4da 0c4ac54e counter 119
--xxxxx-------- cycles 58552, 001b1eb5 285c9bf0 counter 120
--xxxxx-------- cycles 59040, 001b5890 446e7292 counter 121
--xxxxx-------- cycles 59528, 001b926b 60804934 counter 122
--xxxxx-------- cycles 60016, 001bcc46 7c921fd6 counter 123
--xxxxx-------- cycles 60504, 001c0621 98a3f678 counter 124
--xxxxx-------- cycles 60992, 001c3ffc b4b5cd1a counter 125
--xxxxx-------- cycles 61480, 001c79d7 d0c7a3bc counter 126
--xxxxx-------- cycles 61968, 001cb3b2 ecd97a5e counter 127
--xxxxx-------- cycles 62456, 001ced8e 08eb5100 counter 128
--xxxxx-------- cycles 62944, 001d2769 24fd27a2 counter 129
--xxxxx-------- cycles 63432, 001d6144 410efe44 counter 130
--xxxxx-------- cycles 63920, 001d9b1f 5d20d4e6 counter 131
--xxxxx-------- cycles 64408, 001dd4fa 7932ab88 counter 132
--xxxxx-------- cycles 64896, 001e0ed5 9544822a counter 133
--xxxxx-------- cycles 65384, 001e48b0 b15658cc counter 134
--xxxxx-------- cycles 65872, 001e828b cd682f6e counter 135
--xxxxx-------- cycles 66360, 001ebc66 e97a0610 counter 136
--xxxxx-------- cycles 66847, 001ef642 058bdcb2 counter 137
--xxxxx-------- cycles 67335, 001f301d 219db354 counter 138
--xxxxx-------- cycles 67823, 001f69f8 3daf89f6 counter 139
--xxxxx-------- cycles 68311, 001fa3d3 59c16098 counter 140
--xxxxx-------- cycles 68799, 001fddae 75d3373a counter 141
--xxxxx-------- cycles 69287, 00201789 91e50ddc counter 142
--xxxxx-------- cycles 69775, 00205164 adf6e47e counter 143
--xxxxx-------- cycles 70263, 00208b3f ca08bb20 counter 144
--xxxxx-------- cycles 70751, 0020c51a e61a91c2 counter 145
--xxxxx-------- cycles 71239, 0020fef6 022c6864 counter 146
--xxxxx-------- cycles 71727, 002138d1 1e3e3f06 counter 147
--xxxxx-------- cycles 72215, 002172ac 3a5015a8 counter 148
--xxxxx-------- cycles 72703, 0021ac87 5661ec4a counter 149
--xxxxx-------- cycles 73191, 0021e662 7273c2ec counter 150
--xxxxx-------- cycles 73679, 0022203d 8e85998e counter 151
--xxxxx-------- cycles 74167, 00225a18 aa977030 counter 152
--xxxxx-------- cycles 74655, 002293f3 c6a946d2 counter 153
--xxxxx-------- cycles 75143, 0022cdce e2bb1d74 counter 154
--xxxxx-------- cycles 75630, 002307a9 feccf416 counter 155
--xxxxx-------- cycles 76118, 00234185 1adecab8 counter 156
--xxxxx-------- cycles 76606, 00237b60 36f0a15a counter 157
--xxxxx-------- cycles 77094, 0023b53b 530277fc counter 158
--xxxxx-------- cycles 77582, 0023ef16 6f144e9e counter 159
--xxxxx-------- cycles 78070, 002428f1 8b262540 counter 160
--xxxxx-------- cycles 78558, 002462cc a737fbe2 counter 161
--xxxxx-------- cycles 79046, 00249ca7 c349d284 counter 162
--xxxxx-------- cycles 79534, 0024d682 df5ba926 counter 163
--xxxxx-------- cycles 80022, 0025105d fb6d7fc8 counter 164
--xxxxx-------- cycles 80510, 00254a39 177f566a counter 165
--xxxxx-------- cycles 80998, 00258414 33912d0c counter 166
--xxxxx-------- cycles 81486, 0025bdef 4fa303ae counter 167
--xxxxx-------- cycles 81974, 0025f7ca 6bb4da50 counter 168
--xxxxx-------- cycles 82462, 002631a5 87c6b0f2 counter 169
--xxxxx-------- cycles 82950, 00266b80 a3d88794 counter 170
--xxxxx-------- cycles 83438, 0026a55b bfea5e36 counter 171
--xxxxx-------- cycles 83925, 0026df36 dbfc34d8 counter 172
--xxxxx-------- cycles 84413, 00271911 f80e0b7a counter 173
--xxxxx-------- cycles 84901, 002752ed 141fe21c counter 174
--xxxxx-------- cycles 85389, 00278cc8 3031b8be counter 175
--xxxxx-------- cycles 85877, 0027c6a3 4c438f60 counter 176
--xxxxx-------- cycles 86365, 0028007e 68556602 counter 177
--xxxxx-------- cycles 86853, 00283a59 84673ca4 counter 178
--xxxxx-------- cycles 87341, 00287434 a0791346 counter 179
--xxxxx-------- cycles 87829, 0028ae0f bc8ae9e8 counter 180
--xxxxx-------- cycles 88317, 0028e7ea d89cc08a counter 181
--xxxxx-------- cycles 88805, 002921c5 f4ae972c counter 182
--xxxxx-------- cycles 89293, 00295ba1 10c06dce counter 183
--xxxxx-------- cycles 89781, 0029957c 2cd24470 counter 184
--xxxxx-------- cycles 90269, 0029cf57 48e41b12 counter 185
--xxxxx-------- cycles 90757, 002a0932 64f5f1b4 counter 186
--xxxxx-------- cycles 91245, 002a430d 8107c856 counter 187
--xxxxx-------- cycles 91733, 002a7ce8 9d199ef8 counter 188
--xxxxx-------- cycles 92220, 002ab6c3 b92b759a counter 189
--xxxxx-------- cycles 92708, 002af09e d53d4c3c counter 190
--xxxxx-------- cycles 93196, 002b2a79 f14f22de counter 191
--xxxxx-------- cycles 93684, 002b6455 0d60f980 counter 192
--xxxxx-------- cycles 94172, 002b9e30 2972d022 counter 193
--xxxxx-------- cycles 94660, 002bd80b 4584a6c4 counter 194
--xxxxx-------- cycles 95148, 002c11e6 61967d66 counter 195
--xxxxx-------- cycles 95636, 002c4bc1 7da85408 counter 196
--xxxxx-------- cycles 96124, 002c859c 99ba2aaa counter 197
--xxxxx-------- cycles 96612, 002cbf77 b5cc014c counter 198
--xxxxx-------- cycles 97100, 002cf952 d1ddd7ee counter 199
--xxxxx-------- cycles 97588, 002d332d edefae90 counter 200
--xxxxx-------- cycles 98076, 002d6d09 0a018532 counter 201
--xxxxx-------- cycles 98564, 002da6e4 26135bd4 counter 202
--xxxxx-------- cycles 99052, 002de0bf 42253276 counter 203
--xxxxx-------- cycles 99540, 002e1a9a 5e370918 counter 204
--xxxxx-------- cycles 100028, 002e5475 7a48dfba counter 205
--xxxxx-------- cycles 100515, 002e8e50 965ab65c counter 206
--xxxxx-------- cycles 101003, 002ec82b b26c8cfe counter 207
--xxxxx-------- cycles 101491, 002f0206 ce7e63a0 counter 208
--xxxxx-------- cycles 101979, 002f3be1 ea903a42 counter 209
--xxxxx-------- cycles 102467, 002f75bd 06a210e4 counter 210
--xxxxx-------- cycles 102955, 002faf98 22b3e786 counter 211
--xxxxx-------- cycles 103443, 002fe973 3ec5be28 counter 212
--xxxxx-------- cycles 103931, 0030234e 5ad794ca counter 213
--xxxxx-------- cycles 104419, 00305d29 76e96b6c counter 214
--xxxxx-------- cycles 104907, 00309704 92fb420e counter 215
--xxxxx-------- cycles 105395, 0030d0df af0d18b0 counter 216
--xxxxx-------- cycles 105883, 00310aba cb1eef52 counter 217
--xxxxx-------- cycles 106371, 00314495 e730c5f4 counter 218
--xxxxx-------- cycles 106859, 00317e71 03429c96 counter 219
--xxxxx-------- cycles 107347, 0031b84c 1f547338 counter 220
--xxxxx-------- cycles 107835, 0031f227 3b6649da counter 221
--xxxxx-------- cycles 108323, 00322c02 5778207c counter 222
--xxxxx-------- cycles 108810, 003265dd 7389f71e counter 223
--xxxxx-------- cycles 109298, 00329fb8 8f9bcdc0 counter 224
--xxxxx-------- cycles 109786, 0032d993 abada462 counter 225
--xxxxx-------- cycles 110274, 0033136e c7bf7b04 counter 226
--xxxxx-------- cycles 110762, 00334d49 e3d151a6 counter 227
--xxxxx-------- cycles 111250, 00338724 ffe32848 counter 228
--xxxxx-------- cycles 111738, 0033c100 1bf4feea counter 229
--xxxxx-------- cycles 112226, 0033fadb 3806d58c counter 230
--xxxxx-------- cycles 112714, 003434b6 5418ac2e counter 231
--xxxxx-------- cycles 113202, 00346e91 702a82d0 counter 232
--xxxxx-------- cycles 113690, 0034a86c 8c3c5972 counter 233
--xxxxx-------- cycles 114178, 0034e247 a84e3014 counter 234
--xxxxx-------- cycles 114666, 00351c22 c46006b6 counter 235
--xxxxx-------- cycles 115154, 003555fd e071dd58 counter 236
--xxxxx-------- cycles 115642, 00358fd8 fc83b3fa counter 237
--xxxxx-------- cycles 116130, 0035c9b4 18958a9c counter 238
--xxxxx-------- cycles 116618, 0036038f 34a7613e counter 239
--xxxxx-------- cycles 117105, 00363d6a 50b937e0 counter 240
--xxxxx-------- cycles 117593, 00367745 6ccb0e82 counter 241
--xxxxx-------- cycles 118081, 0036b120 88dce524 counter 242
--xxxxx-------- cycles 118569, 0036eafb a4eebbc6 counter 243
--xxxxx-------- cycles 119057, 003724d6 c1009268 counter 244
--xxxxx-------- cycles 119545, 00375eb1 dd12690a counter 245
--xxxxx-------- cycles 120033, 0037988c f9243fac counter 246
--xxxxx-------- cycles 120521, 0037d268 1536164e counter 247
--xxxxx-------- cycles 121009, 00380c43 3147ecf0 counter 248
--xxxxx-------- cycles 121497, 0038461e 4d59c392 counter 249
--xxxxx-------- cycles 121985, 00387ff9 696b9a34 counter 250
--xxxxx-------- cycles 122473, 0038b9d4 857d70d6 counter 251
--xxxxx-------- cycles 122961, 0038f3af a18f4778 counter 252
--xxxxx-------- cycles 123449, 00392d8a bda11e1a counter 253
--xxxxx-------- cycles 123937, 00396765 d9b2f4bc counter 254
--xxxxx-------- cycles 124425, 0039a140 f5c4cb5e counter 255
--xxxxx-------- cycles 124913, 0039db1c 11d6a200 counter 256
--xxxxx-------- cycles 125400, 003a14f7 2de878a2 counter 257
--xxxxx-------- cycles 125888, 003a4ed2 49fa4f44 counter 258
--xxxxx-------- cycles 126376, 003a88ad 660c25e6 counter 259
--xxxxx-------- cycles 126864, 003ac288 821dfc88 counter 260
--xxxxx-------- cycles 127352, 003afc63 9e2fd32a counter 261
     ---------- cycles 127840, 003b363e ba41aaaa (End of frame / start of next)
*/

static int irq4counter;

static timer_device* render_timer;

static TIMER_DEVICE_CALLBACK( render_timer_callback )
{
	if (genesis_scanline_counter>=0 && genesis_scanline_counter<megadrive_visible_scanlines)
	{
		genesis_render_scanline(timer.machine(), genesis_scanline_counter);
	}
}

void _32x_check_irqs(running_machine& machine)
{

	if (sh2_master_vint_enable && sh2_master_vint_pending) device_set_input_line(_32x_master_cpu,SH2_VINT_IRQ_LEVEL,ASSERT_LINE);
	else device_set_input_line(_32x_master_cpu,SH2_VINT_IRQ_LEVEL,CLEAR_LINE);

	if (sh2_slave_vint_enable && sh2_slave_vint_pending) device_set_input_line(_32x_slave_cpu,SH2_VINT_IRQ_LEVEL,ASSERT_LINE);
	else device_set_input_line(_32x_slave_cpu,SH2_VINT_IRQ_LEVEL,CLEAR_LINE);
}



static TIMER_DEVICE_CALLBACK( scanline_timer_callback )
{
	/* This function is called at the very start of every scanline starting at the very
       top-left of the screen.  The first scanline is scanline 0 (we set scanline to -1 in
       VIDEO_EOF) */

	timer.machine().scheduler().synchronize();
	/* Compensate for some rounding errors

       When the counter reaches 261 we should have reached the end of the frame, however due
       to rounding errors in the timer calculation we're not quite there.  Let's assume we are
       still in the previous scanline for now.
    */

	if (genesis_scanline_counter!=(megadrive_total_scanlines-1))
	{
		genesis_scanline_counter++;
//      mame_printf_debug("scanline %d\n",genesis_scanline_counter);
		scanline_timer->adjust(attotime::from_hz(megadriv_framerate) / megadrive_total_scanlines);
		render_timer->adjust(attotime::from_usec(1));

		if (genesis_scanline_counter==megadrive_irq6_scanline )
		{
		//  mame_printf_debug("x %d",genesis_scanline_counter);
			irq6_on_timer->adjust(attotime::from_usec(6));
			megadrive_irq6_pending = 1;
			megadrive_vblank_flag = 1;

			// 32x interrupt!
			if (_32x_is_connected)
			{
				sh2_master_vint_pending = 1;
				sh2_slave_vint_pending = 1;
				_32x_check_irqs(timer.machine());
			}

		}



		_32x_check_framebuffer_swap();


	//  if (genesis_scanline_counter==0) irq4counter = MEGADRIVE_REG0A_HINT_VALUE;
		// irq4counter = MEGADRIVE_REG0A_HINT_VALUE;

		if (genesis_scanline_counter<=224)
		{
			irq4counter--;

			if (irq4counter==-1)
			{
				if (megadrive_imode==3) irq4counter = MEGADRIVE_REG0A_HINT_VALUE*2;
				else irq4counter=MEGADRIVE_REG0A_HINT_VALUE;

				megadrive_irq4_pending = 1;

				if (MEGADRIVE_REG0_IRQ4_ENABLE)
				{
					irq4_on_timer->adjust(attotime::from_usec(1));
					//mame_printf_debug("irq4 on scanline %d reload %d\n",genesis_scanline_counter,MEGADRIVE_REG0A_HINT_VALUE);
				}
			}
		}
		else
		{
			if (megadrive_imode==3) irq4counter = MEGADRIVE_REG0A_HINT_VALUE*2;
			else irq4counter=MEGADRIVE_REG0A_HINT_VALUE;
		}

		//if (genesis_scanline_counter==0) irq4_on_timer->adjust(attotime::from_usec(2));

		if(_32x_is_connected)
		{
			_32x_hcount_compare_val++;

			if(_32x_hcount_compare_val >= _32x_hcount_reg)
			{
				_32x_hcount_compare_val = -1;

				if(genesis_scanline_counter < 224 || sh2_hint_in_vbl)
				{
					if(sh2_master_hint_enable) { device_set_input_line(_32x_master_cpu,SH2_HINT_IRQ_LEVEL,ASSERT_LINE); }
					if(sh2_slave_hint_enable) { device_set_input_line(_32x_slave_cpu,SH2_HINT_IRQ_LEVEL,ASSERT_LINE); }
				}
			}
		}


		if (timer.machine().device("genesis_snd_z80") != NULL)
		{
			if (genesis_scanline_counter == megadrive_z80irq_scanline)
			{
				if ((genz80.z80_has_bus == 1) && (genz80.z80_is_reset == 0))
					cputag_set_input_line(timer.machine(), "genesis_snd_z80", 0, HOLD_LINE);
			}
			if (genesis_scanline_counter == megadrive_z80irq_scanline + 1)
			{
				cputag_set_input_line(timer.machine(), "genesis_snd_z80", 0, CLEAR_LINE);
			}
		}

	}
	else /* pretend we're still on the same scanline to compensate for rounding errors */
	{
		genesis_scanline_counter = megadrive_total_scanlines - 1;
	}

}

static TIMER_DEVICE_CALLBACK( irq6_on_callback )
{
	//mame_printf_debug("irq6 active on %d\n",genesis_scanline_counter);

	{
//      megadrive_irq6_pending = 1;
		if (MEGADRIVE_REG01_IRQ6_ENABLE || genesis_always_irq6)
			cputag_set_input_line(timer.machine(), "maincpu", 6, HOLD_LINE);
	}
}

static TIMER_DEVICE_CALLBACK( irq4_on_callback )
{
	//mame_printf_debug("irq4 active on %d\n",genesis_scanline_counter);
	cputag_set_input_line(timer.machine(), "maincpu", 4, HOLD_LINE);
}

/*****************************************************************************************/

static int hazemdchoice_megadrive_region_export;
static int hazemdchoice_megadrive_region_pal;
static int hazemdchoice_megadriv_framerate;

MACHINE_START( megadriv )
{
	if (megadrive_6buttons_pad)
		init_megadri6_io(machine);
}

MACHINE_RESET( megadriv )
{
	/* default state of z80 = reset, with bus */
	mame_printf_debug("Resetting Megadrive / Genesis\n");

	switch (input_port_read_safe(machine, "REGION", 0xff))
	{

		case 1: // US
		megadrive_region_export = 1;
		megadrive_region_pal = 0;
		megadriv_framerate = 60;
		mame_printf_debug("Using Region = US\n");
		break;

		case 2: // JAPAN
		megadrive_region_export = 0;
		megadrive_region_pal = 0;
		megadriv_framerate = 60;
		mame_printf_debug("Using Region = JAPAN\n");
		break;

		case 3: // EUROPE
		megadrive_region_export = 1;
		megadrive_region_pal = 1;
		megadriv_framerate = 50;
		mame_printf_debug("Using Region = EUROPE\n");
		break;

		case 0: // as chosen by driver
		megadrive_region_export = hazemdchoice_megadrive_region_export;
		megadrive_region_pal = hazemdchoice_megadrive_region_pal;
		megadriv_framerate = hazemdchoice_megadriv_framerate;
		mame_printf_debug("Using Region = DEFAULT\n");
		break;

		default:
		megadriv_framerate = hazemdchoice_megadriv_framerate;
		break;
	}

	if (machine.device("genesis_snd_z80") != NULL)
	{
		genz80.z80_is_reset = 1;
		genz80.z80_has_bus = 1;
		genz80.z80_bank_addr = 0;
		genesis_scanline_counter = -1;
		machine.scheduler().timer_set( attotime::zero, FUNC(megadriv_z80_run_state ));
	}

	megadrive_imode = 0;

	megadrive_reset_io(machine);

	frame_timer = machine.device<timer_device>("frame_timer");
	scanline_timer = machine.device<timer_device>("scanline_timer");
	render_timer = machine.device<timer_device>("render_timer");

	irq6_on_timer = machine.device<timer_device>("irq6_timer");
	irq4_on_timer = machine.device<timer_device>("irq4_timer");

	frame_timer->adjust(attotime::zero);
	scanline_timer->adjust(attotime::zero);

	if (genesis_other_hacks)
	{
	//  set_refresh_rate(megadriv_framerate);
	//  machine.device("maincpu")->set_clock_scale(0.9950f); /* Fatal Rewind is very fussy... (and doesn't work now anyway, so don't bother with this) */

		memset(megadrive_ram,0x00,0x10000);
	}

	irq4counter = -1;
	megadrive_total_scanlines = 262;
	megadrive_visible_scanlines = 224;
	megadrive_irq6_scanline = 224;
	megadrive_z80irq_scanline = 226;


	/* if any of these extra CPUs exist, pause them until we actually turn them on */
	if (_32x_master_cpu != NULL)
	{
		device_set_input_line(_32x_master_cpu, INPUT_LINE_RESET, ASSERT_LINE);
	}

	if (_32x_slave_cpu != NULL)
	{
		device_set_input_line(_32x_slave_cpu, INPUT_LINE_RESET, ASSERT_LINE);
	}

	if (_segacd_68k_cpu != NULL )
	{
		MACHINE_RESET_CALL( segacd );
	}


	if(_32x_is_connected)
	{
		current_fifo_block = fifo_block_a;
		current_fifo_readblock = fifo_block_b;
		current_fifo_write_pos = 0;
		current_fifo_read_pos = 0;
		fifo_block_a_full = 0;
		fifo_block_b_full = 0;

		_32x_hcount_compare_val = -1;
	}
}

void megadriv_stop_scanline_timer(void)
{
	scanline_timer->reset();
}

/*
 999999999999999960
1000000000000000000 subseconds = 1 second

/ 60

*/

/* VIDEO_EOF is used to resync the scanline counters */

SCREEN_VBLANK(megadriv)
{
	// rising edge
	if (vblank_on)
	{
		rectangle visarea;
		int scr_width = 320;

		megadrive_vblank_flag = 0;
		//megadrive_irq6_pending = 0; /* NO! (breaks warlock) */

		/* Set it to -1 here, so it becomes 0 when the first timer kicks in */
		genesis_scanline_counter = -1;
		megadrive_sprite_collision=0;//? when to reset this ..
		megadrive_imode = MEGADRIVE_REG0C_INTERLEAVE; // can't change mid-frame..
		megadrive_imode_odd_frame^=1;
//      cputag_set_input_line(machine, "genesis_snd_z80", 0, CLEAR_LINE); // if the z80 interrupt hasn't happened by now, clear it..

		if (input_port_read_safe(screen.machine(), "RESET", 0x00) & 0x01)
			cputag_set_input_line(screen.machine(), "maincpu", INPUT_LINE_RESET, PULSE_LINE);

/*
int megadrive_total_scanlines = 262;
int megadrive_visible_scanlines = 224;
int megadrive_irq6_scanline = 224;
int megadrive_irq6_hpos = 320;
int megadrive_z80irq_scanline = 224;
int megadrive_z80irq_hpos = 320;
*/


		if (MEGADRIVE_REG01_240_LINE)
		{
			if (!megadrive_region_pal)
			{
				/* this is invalid! */
				megadrive_visible_scanlines = 240;
				megadrive_total_scanlines = 262;
				megadrive_irq6_scanline = 240;
				megadrive_z80irq_scanline = 240;
			}
			else
			{
				megadrive_visible_scanlines = 240;
				megadrive_total_scanlines = 313;
				megadrive_irq6_scanline = 240;
				megadrive_z80irq_scanline = 240;
			}
		}
		else
		{
			if (!megadrive_region_pal)
			{
				megadrive_visible_scanlines = 224;
				megadrive_total_scanlines=262;
				megadrive_irq6_scanline = 224;
				megadrive_z80irq_scanline = 224;
			}
			else
			{
				megadrive_visible_scanlines = 224;
				megadrive_total_scanlines=313;
				megadrive_irq6_scanline = 224;
				megadrive_z80irq_scanline = 224;
			}
		}

		if (megadrive_imode==3)
		{
			megadrive_visible_scanlines<<=1;
			megadrive_total_scanlines<<=1;
			megadrive_irq6_scanline <<=1;
			megadrive_z80irq_scanline <<=1;
		}


		//get_hposition();
		switch (MEGADRIVE_REG0C_RS0 | (MEGADRIVE_REG0C_RS1 << 1))
		{
			 /* note, add 240 mode + init new timings! */
			case 0:scr_width = 256;break;// configure_screen(0, 256-1, megadrive_visible_scanlines-1,(double)megadriv_framerate); break;
			case 1:scr_width = 256;break;// configure_screen(0, 256-1, megadrive_visible_scanlines-1,(double)megadriv_framerate); mame_printf_debug("invalid screenmode!\n"); break;
			case 2:scr_width = 320;break;// configure_screen(0, 320-1, megadrive_visible_scanlines-1,(double)megadriv_framerate); break; /* technically invalid, but used in rare cases */
			case 3:scr_width = 320;break;// configure_screen(0, 320-1, megadrive_visible_scanlines-1,(double)megadriv_framerate); break;
		}
//      mame_printf_debug("my mode %02x", megadrive_vdp_register[0x0c]);

		visarea.set(0, scr_width-1, 0, megadrive_visible_scanlines-1);

		screen.machine().primary_screen->configure(scr_width, megadrive_visible_scanlines, visarea, HZ_TO_ATTOSECONDS(megadriv_framerate));

		if (0)
		{
			//int xxx;
//          UINT64 frametime;

		//  /* reference */
//          frametime = ATTOSECONDS_PER_SECOND/megadriv_framerate;

			//time_elapsed_since_crap = frame_timer->time_elapsed();
			//xxx = screen.machine().device<cpudevice>("maincpu")->attotime_to_cycles(time_elapsed_since_crap);
			//mame_printf_debug("---------- cycles %d, %08x %08x\n",xxx, (UINT32)(time_elapsed_since_crap.attoseconds>>32),(UINT32)(time_elapsed_since_crap.attoseconds&0xffffffff));
			//mame_printf_debug("---------- framet %d, %08x %08x\n",xxx, (UINT32)(frametime>>32),(UINT32)(frametime&0xffffffff));
			frame_timer->adjust(attotime::zero);
		}

		scanline_timer->adjust(attotime::zero);

		if(_32x_is_connected)
			_32x_hcount_compare_val = -1;
	}
}


UINT16* megadriv_backupram;
int megadriv_backupram_length;

static NVRAM_HANDLER( megadriv )
{
	if (megadriv_backupram!=NULL)
	{
		if (read_or_write)
			file->write(megadriv_backupram, megadriv_backupram_length);
		else
		{
			if (file)
			{
				file->read(megadriv_backupram, megadriv_backupram_length);
			}
			else
			{
				int x;
				for (x=0;x<megadriv_backupram_length/2;x++)
					megadriv_backupram[x]=0xffff;//machine.rand(); // dino dini's needs 0xff or game rules are broken
			}
		}
	}
}


MACHINE_CONFIG_FRAGMENT( megadriv_timers )
	MCFG_TIMER_ADD("frame_timer", frame_timer_callback)
	MCFG_TIMER_ADD("scanline_timer", scanline_timer_callback)
	MCFG_TIMER_ADD("render_timer", render_timer_callback)
	MCFG_TIMER_ADD("irq6_timer", irq6_on_callback)
	MCFG_TIMER_ADD("irq4_timer", irq4_on_callback)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( md_ntsc )
	MCFG_CPU_ADD("maincpu", M68000, MASTER_CLOCK_NTSC / 7) /* 7.67 MHz */
	MCFG_CPU_PROGRAM_MAP(megadriv_map)
	/* IRQs are handled via the timers */

	MCFG_CPU_ADD("genesis_snd_z80", Z80, MASTER_CLOCK_NTSC / 15) /* 3.58 MHz */
	MCFG_CPU_PROGRAM_MAP(megadriv_z80_map)
	MCFG_CPU_IO_MAP(megadriv_z80_io_map)
	/* IRQ handled via the timers */

	MCFG_MACHINE_START(megadriv)
	MCFG_MACHINE_RESET(megadriv)

	MCFG_FRAGMENT_ADD(megadriv_timers)

	MCFG_SCREEN_ADD("megadriv", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)) // Vblank handled manually.
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 0, 28*8-1)
	MCFG_SCREEN_UPDATE_STATIC(megadriv) /* Copies a bitmap */
	MCFG_SCREEN_VBLANK_STATIC(megadriv) /* Used to Sync the timing */

	MCFG_NVRAM_HANDLER(megadriv)

	MCFG_PALETTE_LENGTH(0x200)

	MCFG_VIDEO_START(megadriv)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2612, MASTER_CLOCK_NTSC/7) /* 7.67 MHz */
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	/* sound hardware */
	MCFG_SOUND_ADD("snsnd", SEGAPSG, MASTER_CLOCK_NTSC/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25) /* 3.58 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker",0.25) /* 3.58 MHz */
MACHINE_CONFIG_END

MACHINE_CONFIG_START( megadriv, md_cons_state )
	MCFG_FRAGMENT_ADD(md_ntsc)
MACHINE_CONFIG_END

/************ PAL hardware has a different master clock *************/

MACHINE_CONFIG_FRAGMENT( md_pal )
	MCFG_CPU_ADD("maincpu", M68000, MASTER_CLOCK_PAL / 7) /* 7.67 MHz */
	MCFG_CPU_PROGRAM_MAP(megadriv_map)
	/* IRQs are handled via the timers */

	MCFG_CPU_ADD("genesis_snd_z80", Z80, MASTER_CLOCK_PAL / 15) /* 3.58 MHz */
	MCFG_CPU_PROGRAM_MAP(megadriv_z80_map)
	MCFG_CPU_IO_MAP(megadriv_z80_io_map)
	/* IRQ handled via the timers */

	MCFG_MACHINE_START(megadriv)
	MCFG_MACHINE_RESET(megadriv)

	MCFG_FRAGMENT_ADD(megadriv_timers)

	MCFG_SCREEN_ADD("megadriv", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)) // Vblank handled manually.
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 0, 28*8-1)
	MCFG_SCREEN_UPDATE_STATIC(megadriv) /* Copies a bitmap */
	MCFG_SCREEN_VBLANK_STATIC(megadriv) /* Used to Sync the timing */

	MCFG_NVRAM_HANDLER(megadriv)

	MCFG_PALETTE_LENGTH(0x200)

	MCFG_VIDEO_START(megadriv)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2612, MASTER_CLOCK_PAL/7) /* 7.67 MHz */
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	/* sound hardware */
	MCFG_SOUND_ADD("snsnd", SEGAPSG, MASTER_CLOCK_PAL/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25) /* 3.58 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker",0.25) /* 3.58 MHz */
MACHINE_CONFIG_END

MACHINE_CONFIG_START( megadpal, driver_device )
	MCFG_FRAGMENT_ADD(md_pal)
MACHINE_CONFIG_END


static int _32x_fifo_available_callback(device_t *device, UINT32 src, UINT32 dst, UINT32 data, int size)
{
	if (src==0x4012)
	{
		if (current_fifo_readblock==fifo_block_a && fifo_block_a_full)
			return 1;

		if (current_fifo_readblock==fifo_block_b && fifo_block_b_full)
			return 1;

		return 0;
	}

	return 1;
}


static const sh2_cpu_core sh2_conf_master = { 0, NULL, _32x_fifo_available_callback };
static const sh2_cpu_core sh2_conf_slave  = { 1, NULL, _32x_fifo_available_callback };

MACHINE_CONFIG_DERIVED( genesis_32x, megadriv )

#ifndef _32X_SWAP_MASTER_SLAVE_HACK
	MCFG_CPU_ADD("32x_master_sh2", SH2, (MASTER_CLOCK_NTSC*3)/7 )
	MCFG_CPU_PROGRAM_MAP(sh2_main_map)
	MCFG_CPU_CONFIG(sh2_conf_master)
#endif

	MCFG_CPU_ADD("32x_slave_sh2", SH2, (MASTER_CLOCK_NTSC*3)/7 )
	MCFG_CPU_PROGRAM_MAP(sh2_slave_map)
	MCFG_CPU_CONFIG(sh2_conf_slave)

#ifdef _32X_SWAP_MASTER_SLAVE_HACK
	MCFG_CPU_ADD("32x_master_sh2", SH2, (MASTER_CLOCK_NTSC*3)/7 )
	MCFG_CPU_PROGRAM_MAP(sh2_main_map)
	MCFG_CPU_CONFIG(sh2_conf_master)
#endif

	// brutal needs at least 30000 or the backgrounds don't animate properly / lock up, and the game
	// freezes.  Some stage seem to need as high as 80000 ?   this *KILLS* performance
	//
	// boosting the interleave here actually makes Kolibri run incorrectly however, that
	// one works best just boosting the interleave on communications?!
	MCFG_QUANTUM_TIME(attotime::from_hz(1800000))

	// we need to remove and re-add the sound system because the balance is different
	// due to MAME / MESS having severe issues if the dac output is > 0.40? (sound is corrupted even if DAC is slient?!)
	MCFG_DEVICE_REMOVE("ymsnd")
	MCFG_DEVICE_REMOVE("snsnd")


	MCFG_SOUND_ADD("ymsnd", YM2612, MASTER_CLOCK_NTSC/7)
	MCFG_SOUND_ROUTE(0, "lspeaker", (0.50)/2)
	MCFG_SOUND_ROUTE(1, "rspeaker", (0.50)/2)

	/* sound hardware */
	MCFG_SOUND_ADD("snsnd", SEGAPSG, MASTER_CLOCK_NTSC/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", (0.25)/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", (0.25)/2)

	MCFG_SOUND_ADD("lch_pwm", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.40)

	MCFG_SOUND_ADD("rch_pwm", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.40)
MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( genesis_32x_pal, megadpal )

#ifndef _32X_SWAP_MASTER_SLAVE_HACK
	MCFG_CPU_ADD("32x_master_sh2", SH2, (MASTER_CLOCK_PAL*3)/7 )
	MCFG_CPU_PROGRAM_MAP(sh2_main_map)
	MCFG_CPU_CONFIG(sh2_conf_master)
#endif

	MCFG_CPU_ADD("32x_slave_sh2", SH2, (MASTER_CLOCK_PAL*3)/7 )
	MCFG_CPU_PROGRAM_MAP(sh2_slave_map)
	MCFG_CPU_CONFIG(sh2_conf_slave)

#ifdef _32X_SWAP_MASTER_SLAVE_HACK
	MCFG_CPU_ADD("32x_master_sh2", SH2, (MASTER_CLOCK_PAL*3)/7 )
	MCFG_CPU_PROGRAM_MAP(sh2_main_map)
	MCFG_CPU_CONFIG(sh2_conf_master)
#endif

	// brutal needs at least 30000 or the backgrounds don't animate properly / lock up, and the game
	// freezes.  Some stage seem to need as high as 80000 ?   this *KILLS* performance
	//
	// boosting the interleave here actually makes Kolibri run incorrectly however, that
	// one works best just boosting the interleave on communications?!
	MCFG_QUANTUM_TIME(attotime::from_hz(1800000))

	// we need to remove and re-add the sound system because the balance is different
	// due to MAME / MESS having severe issues if the dac output is > 0.40? (sound is corrupted even if DAC is slient?!)
	MCFG_DEVICE_REMOVE("ymsnd")
	MCFG_DEVICE_REMOVE("snsnd")


	MCFG_SOUND_ADD("ymsnd", YM2612, MASTER_CLOCK_PAL/7)
	MCFG_SOUND_ROUTE(0, "lspeaker", (0.50)/2)
	MCFG_SOUND_ROUTE(1, "rspeaker", (0.50)/2)

	/* sound hardware */
	MCFG_SOUND_ADD("snsnd", SEGAPSG, MASTER_CLOCK_PAL/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", (0.25)/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", (0.25)/2)

	MCFG_SOUND_ADD("lch_pwm", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.40)

	MCFG_SOUND_ADD("rch_pwm", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.40)
MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( genesis_scd, megadriv )
	MCFG_NVRAM_HANDLER_CLEAR()
	MCFG_CPU_ADD("segacd_68k", M68000, SEGACD_CLOCK ) /* 12.5 MHz */
	MCFG_CPU_PROGRAM_MAP(segacd_map)

	MCFG_TIMER_ADD("sw_timer", NULL) //stopwatch timer

	MCFG_NVRAM_ADD_0FILL("backupram")

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 0.50 ) // TODO: accurate volume balance
	MCFG_SOUND_ROUTE( 1, "rspeaker", 0.50 )

	MCFG_SOUND_ADD("rfsnd", RF5C68, SEGACD_CLOCK) // RF5C164!
	MCFG_SOUND_ROUTE( 0, "lspeaker", 0.50 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 0.50 )

	MCFG_TIMER_ADD("scd_dma_timer", scd_dma_timer_callback)


	MCFG_QUANTUM_PERFECT_CPU("segacd_68k") // perfect sync to the fastest cpu
MACHINE_CONFIG_END

struct cdrom_interface scd_cdrom =
{
	"scd_cdrom",
	NULL
};

/* Different Softlists for different regions (for now at least) */
MACHINE_CONFIG_DERIVED( genesis_scd_scd, genesis_scd )
	MCFG_CDROM_ADD( "cdrom",scd_cdrom )
	MCFG_SOFTWARE_LIST_ADD("cd_list","segacd")
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( genesis_scd_mcd, genesis_scd )
	MCFG_CDROM_ADD( "cdrom",scd_cdrom )
	MCFG_SOFTWARE_LIST_ADD("cd_list","megacd")
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( genesis_scd_mcdj, genesis_scd )
	MCFG_CDROM_ADD( "cdrom",scd_cdrom )
	MCFG_SOFTWARE_LIST_ADD("cd_list","megacdj")
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( genesis_32x_scd, genesis_32x )

	MCFG_CPU_ADD("segacd_68k", M68000, SEGACD_CLOCK ) /* 12.5 MHz */
	MCFG_CPU_PROGRAM_MAP(segacd_map)

	MCFG_TIMER_ADD("sw_timer", NULL) //stopwatch timer
	MCFG_NVRAM_ADD_0FILL("backupram")
	MCFG_TIMER_ADD("scd_dma_timer", scd_dma_timer_callback)

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 0.50 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 0.50 )

	MCFG_SOUND_ADD("rfsnd", RF5C68, SEGACD_CLOCK) // RF5C164
	MCFG_SOUND_ROUTE( 0, "lspeaker", 0.25 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 0.25 )

	MCFG_CDROM_ADD( "cdrom", scd_cdrom)
	MCFG_SOFTWARE_LIST_ADD("cd_list","segacd")

	MCFG_QUANTUM_PERFECT_CPU("32x_master_sh2")
MACHINE_CONFIG_END


/* Callback when the genesis enters interrupt code */
static IRQ_CALLBACK(genesis_int_callback)
{
	if (irqline==4)
	{
		megadrive_irq4_pending = 0;
	}

	if (irqline==6)
	{
		megadrive_irq6_pending = 0;
	//  mame_printf_debug("clear pending!\n");
	}

	return (0x60+irqline*4)/4; // vector address
}

static int megadriv_tas_callback(device_t *device)
{
	return 0; // writeback not allowed
}

static void megadriv_init_common(running_machine &machine)
{
	/* Look to see if this system has the standard Sound Z80 */
	_genesis_snd_z80_cpu = machine.device<cpu_device>("genesis_snd_z80");
	if (_genesis_snd_z80_cpu != NULL)
	{
		//printf("GENESIS Sound Z80 cpu found '%s'\n", _genesis_snd_z80_cpu->tag() );

		genz80.z80_prgram = auto_alloc_array(machine, UINT8, 0x2000);
		memory_set_bankptr(machine,  "bank1", genz80.z80_prgram );
	}

	/* Look to see if this system has the 32x Master SH2 */
	_32x_master_cpu = machine.device<cpu_device>("32x_master_sh2");
	if (_32x_master_cpu != NULL)
	{
		printf("32x MASTER SH2 cpu found '%s'\n", _32x_master_cpu->tag() );
	}

	/* Look to see if this system has the 32x Slave SH2 */
	_32x_slave_cpu = machine.device<cpu_device>("32x_slave_sh2");
	if (_32x_slave_cpu != NULL)
	{
		printf("32x SLAVE SH2 cpu found '%s'\n", _32x_slave_cpu->tag() );
	}

	if ((_32x_master_cpu != NULL) && (_32x_slave_cpu != NULL))
	{
		_32x_is_connected = 1;
	}
	else
	{
		_32x_is_connected = 0;
	}

	if(_32x_is_connected)
	{
		_32x_pwm_timer = machine.scheduler().timer_alloc(FUNC(_32x_pwm_callback));
		_32x_pwm_timer->adjust(attotime::never);
	}

	sega_cd_connected = 0;
	segacd_wordram_mapped = 0;
	_segacd_68k_cpu = machine.device<cpu_device>("segacd_68k");
	if (_segacd_68k_cpu != NULL)
	{
		printf("Sega CD secondary 68k cpu found '%s'\n", _segacd_68k_cpu->tag() );
		sega_cd_connected = 1;
		segacd_init_main_cpu(machine);
		scd_dma_timer = machine.device<timer_device>("scd_dma_timer");

	}

	_svp_cpu = machine.device<cpu_device>("svp");
	if (_svp_cpu != NULL)
	{
		printf("SVP (cpu) found '%s'\n", _svp_cpu->tag() );
	}

	device_set_irq_callback(machine.device("maincpu"), genesis_int_callback);
	megadriv_backupram = NULL;
	megadriv_backupram_length = 0;

	vdp_get_word_from_68k_mem = vdp_get_word_from_68k_mem_default;

	m68k_set_tas_callback(machine.device("maincpu"), megadriv_tas_callback);

	// the drivers which need 6 buttons pad set this to 1 in their init befare calling the megadrive init
	if (megadrive_6buttons_pad)
	{
		megadrive_io_read_data_port_ptr	= megadrive_io_read_data_port_6button;
		megadrive_io_write_data_port_ptr = megadrive_io_write_data_port_6button;
		mame_printf_debug("6 button game\n");
	}
	else
	{
		megadrive_io_read_data_port_ptr	= megadrive_io_read_data_port_3button;
		megadrive_io_write_data_port_ptr = megadrive_io_write_data_port_3button;
		mame_printf_debug("3 button game\n");
	}

	{
		/* only really useful on official games, ea games etc. don't bother
          some games specify a single address, (start 200001, end 200001)
          this usually means there is serial eeprom instead */
		int i;
		UINT16 *rom = (UINT16*)machine.region("maincpu")->base();

		mame_printf_debug("DEBUG:: Header: Backup RAM string (ignore for games without)\n");
		for (i=0;i<12;i++)
		{
			if (i==2) mame_printf_debug("\nstart: ");
			if (i==4) mame_printf_debug("\nend  : ");
			if (i==6) mame_printf_debug("\n");

			mame_printf_debug("%04x ",rom[(0x1b0/2)+i]);
		}
		mame_printf_debug("\n");
	}

	/* if we have an SVP cpu then do some extra initilization for it */
	if (_svp_cpu != NULL)
	{
		svp_init(machine);
	}


}

DRIVER_INIT( megadriv_c2 )
{
	genvdp_use_cram = 0;
	genesis_always_irq6 = 1;
	genesis_other_hacks = 0;

	megadriv_init_common(machine);
	hazemdchoice_megadrive_region_export = 1;
	hazemdchoice_megadrive_region_pal = 0;
	hazemdchoice_megadriv_framerate = 60;
}



DRIVER_INIT( megadriv )
{
	genvdp_use_cram = 1;
	genesis_always_irq6 = 0;
	genesis_other_hacks = 1;

	megadriv_init_common(machine);
	hazemdchoice_megadrive_region_export = 1;
	hazemdchoice_megadrive_region_pal = 0;
	hazemdchoice_megadriv_framerate = 60;
}

DRIVER_INIT( megadrij )
{
	genvdp_use_cram = 1;
	genesis_always_irq6 = 0;
	genesis_other_hacks = 1;

	megadriv_init_common(machine);
	hazemdchoice_megadrive_region_export = 0;
	hazemdchoice_megadrive_region_pal = 0;
	hazemdchoice_megadriv_framerate = 60;
}

DRIVER_INIT( megadrie )
{
	genvdp_use_cram = 1;
	genesis_always_irq6 = 0;
	genesis_other_hacks = 1;

	megadriv_init_common(machine);
	hazemdchoice_megadrive_region_export = 1;
	hazemdchoice_megadrive_region_pal = 1;
	hazemdchoice_megadriv_framerate = 50;
}

DRIVER_INIT( mpnew )
{
	DRIVER_INIT_CALL(megadrij);
	megadrive_io_read_data_port_ptr	= megadrive_io_read_data_port_3button;
	megadrive_io_write_data_port_ptr = megadrive_io_write_data_port_3button;
}

/* used by megatech */
static READ8_HANDLER( z80_unmapped_port_r )
{
//  printf("unmapped z80 port read %04x\n",offset);
	return 0;
}

static WRITE8_HANDLER( z80_unmapped_port_w )
{
//  printf("unmapped z80 port write %04x\n",offset);
}

static READ8_HANDLER( z80_unmapped_r )
{
	printf("unmapped z80 read %04x\n",offset);
	return 0;
}

static WRITE8_HANDLER( z80_unmapped_w )
{
	printf("unmapped z80 write %04x\n",offset);
}


/* sets the megadrive z80 to it's normal ports / map */
void megatech_set_megadrive_z80_as_megadrive_z80(running_machine &machine, const char* tag)
{
	device_t *ym = machine.device("ymsnd");

	/* INIT THE PORTS *********************************************************************************************/
	machine.device(tag)->memory().space(AS_IO)->install_legacy_readwrite_handler(0x0000, 0xffff, FUNC(z80_unmapped_port_r), FUNC(z80_unmapped_port_w));

	/* catch any addresses that don't get mapped */
	machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x0000, 0xffff, FUNC(z80_unmapped_r), FUNC(z80_unmapped_w));


	machine.device(tag)->memory().space(AS_PROGRAM)->install_readwrite_bank(0x0000, 0x1fff, "bank1");
	memory_set_bankptr(machine,  "bank1", genz80.z80_prgram );

	machine.device(tag)->memory().space(AS_PROGRAM)->install_ram(0x0000, 0x1fff, genz80.z80_prgram);


	// not allowed??
//  machine.device(tag)->memory().space(AS_PROGRAM)->install_readwrite_bank(0x2000, 0x3fff, "bank1");

	machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(*ym, 0x4000, 0x4003, FUNC(ym2612_r), FUNC(ym2612_w));
	machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_write_handler    (0x6000, 0x6000, FUNC(megadriv_z80_z80_bank_w));
	machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_write_handler    (0x6001, 0x6001, FUNC(megadriv_z80_z80_bank_w));
	machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_read_handler     (0x6100, 0x7eff, FUNC(megadriv_z80_unmapped_read));
	machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x7f00, 0x7fff, FUNC(megadriv_z80_vdp_read), FUNC(megadriv_z80_vdp_write));
	machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x8000, 0xffff, FUNC(z80_read_68k_banked_data), FUNC(z80_write_68k_banked_data));
}

// these are tests for 'special case' hardware to make sure I don't break anything while rearranging things
//



DRIVER_INIT( _32x )
{
	_32x_dram0 = auto_alloc_array(machine, UINT16, 0x40000/2);
	_32x_dram1 = auto_alloc_array(machine, UINT16, 0x40000/2);

	memset(_32x_dram0, 0x00, 0x40000);
	memset(_32x_dram1, 0x00, 0x40000);

	_32x_palette_lookup = auto_alloc_array(machine, UINT16, 0x200/2);
	_32x_palette = auto_alloc_array(machine, UINT16, 0x200/2);

	memset(_32x_palette_lookup, 0x00, 0x200);
	memset(_32x_palette, 0x00, 0x200);


	_32x_display_dram = _32x_dram0;
	_32x_access_dram = _32x_dram1;

	_32x_adapter_enabled = 0;

	if (_32x_adapter_enabled == 0)
	{
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_rom(0x0000000, 0x03fffff, machine.region("gamecart")->base());
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x000070, 0x000073, FUNC(_32x_68k_hint_vector_r), FUNC(_32x_68k_hint_vector_w)); // h interrupt vector
	};


	a15100_reg = 0x0000;
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15100, 0xa15101, FUNC(_32x_68k_a15100_r), FUNC(_32x_68k_a15100_w)); // framebuffer control regs
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15102, 0xa15103, FUNC(_32x_68k_a15102_r), FUNC(_32x_68k_a15102_w)); // send irq to sh2
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15104, 0xa15105, FUNC(_32x_68k_a15104_r), FUNC(_32x_68k_a15104_w)); // 68k BANK rom set
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15106, 0xa15107, FUNC(_32x_68k_a15106_r), FUNC(_32x_68k_a15106_w)); // dreq stuff
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15108, 0xa15113, FUNC(_32x_dreq_common_r), FUNC(_32x_dreq_common_w)); // dreq src / dst / length /fifo

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa1511a, 0xa1511b, FUNC(_32x_68k_a1511a_r), FUNC(_32x_68k_a1511a_w)); // SEGA TV

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15120, 0xa1512f, FUNC(_32x_68k_commsram_r), FUNC(_32x_68k_commsram_w)); // comms reg 0-7
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15130, 0xa1513f, FUNC(_32x_pwm_r), FUNC(_32x_68k_pwm_w));

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x0a130ec, 0x0a130ef, FUNC(_32x_68k_MARS_r)); // system ID


	/* Interrupts are masked / disabled at first */
	sh2_master_vint_enable = sh2_slave_vint_enable = 0;
	sh2_master_hint_enable = sh2_slave_hint_enable = 0;
	sh2_master_cmdint_enable = sh2_slave_cmdint_enable = 0;
	sh2_master_pwmint_enable = sh2_slave_pwmint_enable = 0;
	sh2_master_vint_pending = sh2_slave_vint_pending = 0;

	// start in a reset state
	sh2_are_running = 0;

	_32x_a1518a_reg = 0x00; // inital value
	_32x_68k_a15104_reg = 0x00;

	_32x_autofill_length = 0;
	_32x_autofill_address = 0;
	_32x_autofill_data = 0;
	_32x_screenshift = 0;
	_32x_videopriority = 0; // MD priority
	_32x_displaymode = 0;
	_32x_240mode = 0;

// checking if these help brutal, they don't.
	sh2drc_set_options(machine.device("32x_master_sh2"), SH2DRC_COMPATIBLE_OPTIONS);
	sh2drc_set_options(machine.device("32x_slave_sh2"), SH2DRC_COMPATIBLE_OPTIONS);

	DRIVER_INIT_CALL(megadriv);
}

