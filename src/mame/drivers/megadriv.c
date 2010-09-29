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
     - World Series Baseball 95 (and others) are odd, they write data to the normal DRAM framebuffer
       expecting it to act like the 'overwrite area' (where 00 bytes ignored)  this can't be right..



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

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"
#include "sound/sn76496.h"
#include "sound/2612intf.h"
#include "sound/upd7759.h"
#include "sound/fm.h"
#include "cpu/m68000/m68000.h"
#include "includes/megadriv.h"
#include "cpu/sh2/sh2.h"

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

#define SH2_VRES_IRQ_LEVEL 14
#define SH2_VINT_IRQ_LEVEL 12
#define SH2_HINT_IRQ_LEVEL 10
#define SH2_CINT_IRQ_LEVEL 8
#define SH2_PINT_IRQ_LEVEL 6


static UINT16* _32x_dram0;
static UINT16* _32x_dram1;
static UINT16 *_32x_display_dram, *_32x_access_dram;
static UINT16* _32x_palette;
static UINT16* _32x_palette_lookup;
/* SegaCD! */
static cpu_device *_segacd_68k_cpu;
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
static int megadrive_region_export;
static int megadrive_region_pal;
static int megadrive_max_hposition;


static timer_device* frame_timer;
static timer_device* scanline_timer;
static timer_device* irq6_on_timer;
static timer_device* irq4_on_timer;
static bitmap_t* render_bitmap;
//emu_timer* vblankirq_off_timer;


#ifdef UNUSED_FUNCTION
/* taken from segaic16.c */
/* doesn't seem to meet my needs, not used */
static UINT16 read_next_instruction(address_space *space)
{
	static UINT8 recurse = 0;
	UINT16 result;

	/* Unmapped memory returns the last word on the data bus, which is almost always the opcode */
	/* of the next instruction due to prefetch; however, since we may be encrypted, we actually */
	/* need to return the encrypted opcode, not the last decrypted data. */

	/* Believe it or not, this is actually important for Cotton, which has the following evil */
	/* code: btst #0,$7038f7, which tests the low bit of an unmapped address, which thus should */
	/* return the prefetched value. */

	/* prevent recursion */
	if (recurse)
		return 0xffff;

	/* read original encrypted memory at that address */
	recurse = 1;
	result = space->read_word(cpu_get_pc(space->cpu));
	recurse = 0;
	return result;
}
#endif



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
	//logerror("%06x: 68k writing bit to bank register %01x\n", cpu_get_pc(space->cpu),data&0x01);
	megadriv_z80_bank_w(data&0x01);
}

static WRITE8_HANDLER(megadriv_z80_z80_bank_w)
{
	//logerror("%04x: z80 writing bit to bank register %01x\n", cpu_get_pc(space->cpu),data&0x01);
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

static void write_cram_value(running_machine *machine, int offset, int data)
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

static void vdp_cram_write(running_machine *machine, UINT16 data)
{
	int offset;
	offset = (megadrive_vdp_address&0x7e)>>1;

	write_cram_value(machine, offset,data);

	megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;

	megadrive_vdp_address &=0xffff;
}


static void megadriv_vdp_data_port_w(running_machine *machine, int data)
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



static void megadrive_vdp_set_register(running_machine *machine, int regnum, UINT8 value)
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

//  mame_printf_debug("%s: Setting VDP Register #%02x to %02x\n",cpuexec_describe_context(machine), regnum,value);
}

static void update_megadrive_vdp_code_and_address(void)
{
	megadrive_vdp_code = ((megadrive_vdp_command_part1 & 0xc000) >> 14) |
	                     ((megadrive_vdp_command_part2 & 0x00f0) >> 2);

	megadrive_vdp_address = ((megadrive_vdp_command_part1 & 0x3fff) >> 0) |
                            ((megadrive_vdp_command_part2 & 0x0003) << 14);
}

static UINT16 (*vdp_get_word_from_68k_mem)(running_machine *machine, UINT32 source);

static UINT16 vdp_get_word_from_68k_mem_default(running_machine *machine, UINT32 source)
{
	// should we limit the valid areas here?
	// how does this behave with the segacd etc?
	// with the 32x it seems to see the raw cart? - using 68k space causes the character to vanish
	// svp uses it's own function elsewhere...

	if (( source >= 0x000000 ) && ( source <= 0x3fffff ))
	{
		UINT16 *rom = (UINT16*)memory_region(machine, "maincpu");
		return rom[(source&0x3fffff)>>1];
	}
	else if (( source >= 0xe00000 ) && ( source <= 0xffffff ))
	{
//      mame_printf_debug("dma\n");
	//  return ((megadrive_ram[(source&0xffff)>>1]&0xff00)>>8)|((megadrive_ram[(source&0xffff)>>1]&0x00ff)<<8);
		return megadrive_ram[(source&0xffff)>>1];
	}
	else
	{
		printf("DMA Read unmapped %06x\n",source);
		return mame_rand(machine);
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
static void megadrive_do_insta_68k_to_vram_dma(running_machine *machine, UINT32 source,int length)
{
	int count;

	if (length==0x00) length = 0xffff;

	/* This is a hack until real DMA timings are implemented */
	cpu_spinuntil_time(machine->device("maincpu"), ATTOTIME_IN_NSEC(length * 1000 / 3500));

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


static void megadrive_do_insta_68k_to_cram_dma(running_machine *machine,UINT32 source,UINT16 length)
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

static void megadrive_do_insta_68k_to_vsram_dma(running_machine *machine,UINT32 source,UINT16 length)
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
static void handle_dma_bits(running_machine *machine)
{

	if (megadrive_vdp_code&0x20)
	{
		UINT32 source;
		UINT16 length;
		source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8) | ((MEGADRIVE_REG17_DMASOURCE3&0xff)<<16))<<1;
		length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8))<<1;
	//  mame_printf_debug("%s 68k DMAtran set source %06x length %04x dest %04x enabled %01x code %02x %02x\n", cpuexec_describe_context(machine), source, length, megadrive_vdp_address,MEGADRIVE_REG01_DMA_ENABLE, megadrive_vdp_code,MEGADRIVE_REG0F_AUTO_INC);

	}

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

static void megadriv_vdp_ctrl_port_w(running_machine *machine, int data)
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
			megadriv_vdp_data_port_w(space->machine, data);
			break;

		case 0x04:
		case 0x06:
			if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) mame_printf_debug("8-bit write VDP control port access, offset %04x data %04x mem_mask %04x\n",offset,data,mem_mask);
			megadriv_vdp_ctrl_port_w(space->machine, data);
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
			if (ACCESSING_BITS_0_7) sn76496_w(space->machine->device("snsnd"), 0, data & 0xff);
			//if (ACCESSING_BITS_8_15) sn76496_w(space->machine->device("snsnd"), 0, (data >>8) & 0xff);
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

static UINT16 megadriv_vdp_data_port_r(running_machine *machine)
{
	UINT16 retdata=0;

	//return mame_rand(machine);

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
			retdata = mame_rand(machine);
			break;

		case 0x0003:
			logerror("Attempting to READ from DATA PORT in CRAM WRITE MODE\n");
			retdata = mame_rand(machine);
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
			retdata = mame_rand(machine);
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
			retvalue = megadriv_vdp_data_port_r(space->machine);
			break;

		case 0x04:
		case 0x06:
		//  if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) mame_printf_debug("8-bit VDP read control port access, offset %04x mem_mask %04x\n",offset,mem_mask);
			retvalue = megadriv_vdp_ctrl_port_r();
		//  retvalue = mame_rand(space->machine);
		//  mame_printf_debug("%06x: Read Control Port at scanline %d hpos %d (return %04x)\n",cpu_get_pc(space->cpu),genesis_scanline_counter, get_hposition(),retvalue);
			break;

		case 0x08:
		case 0x0a:
		case 0x0c:
		case 0x0e:
		//  if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) mame_printf_debug("8-bit VDP read HV counter port access, offset %04x mem_mask %04x\n",offset,mem_mask);
			retvalue = megadriv_read_hv_counters();
		//  retvalue = mame_rand(space->machine);
		//  mame_printf_debug("%06x: Read HV counters at scanline %d hpos %d (return %04x)\n",cpu_get_pc(space->cpu),genesis_scanline_counter, get_hposition(),retvalue);
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
		logerror("%s: 68000 attempting to access YM2612 (read) without bus\n", cpuexec_describe_context(device->machine));
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
		logerror("%s: 68000 attempting to access YM2612 (write) without bus\n", cpuexec_describe_context(device->machine));
	}
}

/* Megadrive / Genesis has 3 I/O ports */
static emu_timer *io_timeout[3];
static int io_stage[3];

static TIMER_CALLBACK( io_timeout_timer_callback )
{
	io_stage[(int)(FPTR)ptr] = -1;
}

static void init_megadri6_io(running_machine *machine)
{
	int i;

	for (i=0; i<3; i++)
	{
		io_timeout[i] = timer_alloc(machine, io_timeout_timer_callback, (void*)(FPTR)i);
	}
}

/* pointers to our io data read/write functions */
UINT8 (*megadrive_io_read_data_port_ptr)(running_machine *machine, int offset);
void (*megadrive_io_write_data_port_ptr)(running_machine *machine, int offset, UINT16 data);

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

static void megadrive_init_io(running_machine *machine)
{
	const input_port_token *ipt = machine->gamedrv->ipt;

	if (ipt == INPUT_PORTS_NAME(megadri6))
		init_megadri6_io(machine);

	if (ipt == INPUT_PORTS_NAME(ssf2mdb))
		init_megadri6_io(machine);

	if (ipt == INPUT_PORTS_NAME(mk3mdb))
		init_megadri6_io(machine);
}

static void megadrive_reset_io(running_machine *machine)
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
static UINT8 megadrive_io_read_data_port_6button(running_machine *machine, int portnum)
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
static UINT8 megadrive_io_read_data_port_3button(running_machine *machine, int portnum)
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
UINT8 megatech_bios_port_cc_dc_r(running_machine *machine, int offset, int ctrl)
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

	//return (mame_rand(space->machine)&0x0f0f)|0xf0f0;//0x0000;
	switch (offset)
	{
		case 0:
			logerror("%06x read version register\n", cpu_get_pc(space->cpu));
			retdata = megadrive_region_export<<7 | // Export
			          megadrive_region_pal<<6 | // NTSC
			          0x20 | // No Sega CD
			          0x00 | // Unused (Always 0)
			          0x01 | // Bit 3 of Version Number
			          0x01 | // Bit 2 of Version Number
			          0x01 | // Bit 1 of Version Number
			          0x01 ; // Bit 0 of Version Number
			break;

		/* Joystick Port Registers */

		case 0x1:
		case 0x2:
		case 0x3:
//          retdata = megadrive_io_read_data_port(offset-1);
			retdata = megadrive_io_read_data_port_ptr(space->machine, offset-1);
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


static void megadrive_io_write_data_port_3button(running_machine *machine, int portnum, UINT16 data)
{
	megadrive_io_data_regs[portnum] = data;
	//mame_printf_debug("Writing IO Data Register #%d data %04x\n",portnum,data);

}


/****************************** 6 buttons version*****************************/

static void megadrive_io_write_data_port_6button(running_machine *machine, int portnum, UINT16 data)
{
	if (megadrive_io_ctrl_regs[portnum]&0x40)
	{
		if (((megadrive_io_data_regs[portnum]&0x40)==0x00) && ((data&0x40) == 0x40))
		{
			io_stage[portnum]++;
			timer_adjust_oneshot(io_timeout[portnum], machine->device<cpu_device>("maincpu")->cycles_to_attotime(8192), 0);
		}

	}

	megadrive_io_data_regs[portnum] = data;
	//mame_printf_debug("Writing IO Data Register #%d data %04x\n",portnum,data);

}


/*************************** 3 buttons version ****************************/

static void megadrive_io_write_ctrl_port(running_machine *machine, int portnum, UINT16 data)
{
	megadrive_io_ctrl_regs[portnum] = data;
//  mame_printf_debug("Setting IO Control Register #%d data %04x\n",portnum,data);
}

static void megadrive_io_write_tx_port(running_machine *machine, int portnum, UINT16 data)
{
	megadrive_io_tx_regs[portnum] = data;
}

static void megadrive_io_write_rx_port(running_machine *machine, int portnum, UINT16 data)
{

}

static void megadrive_io_write_sctrl_port(running_machine *machine, int portnum, UINT16 data)
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
			megadrive_io_write_data_port_ptr(space->machine, offset-1,data);
			break;

		case 0x4:
		case 0x5:
		case 0x6:
			megadrive_io_write_ctrl_port(space->machine,offset-4,data);
			break;

		/* Serial I/O Registers */

		case 0x7: megadrive_io_write_tx_port(space->machine,0,data); break;
		case 0x8: megadrive_io_write_rx_port(space->machine,0,data); break;
		case 0x9: megadrive_io_write_sctrl_port(space->machine,0,data); break;


		case 0xa: megadrive_io_write_tx_port(space->machine,1,data); break;
		case 0xb: megadrive_io_write_rx_port(space->machine,1,data); break;
		case 0xc: megadrive_io_write_sctrl_port(space->machine,1,data); break;
			break;

		case 0xd: megadrive_io_write_tx_port(space->machine,2,data); break;
		case 0xe: megadrive_io_write_rx_port(space->machine,2,data); break;
		case 0xf: megadrive_io_write_sctrl_port(space->machine,2,data); break;
			break;
	}

}



static ADDRESS_MAP_START( megadriv_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	/*      (0x000000 - 0x3fffff) == GAME ROM (4Meg Max, Some games have special banking too) */

	AM_RANGE(0xa00000, 0xa01fff) AM_READWRITE(megadriv_68k_read_z80_ram,megadriv_68k_write_z80_ram)
	AM_RANGE(0xa02000, 0xa03fff) AM_WRITE(megadriv_68k_write_z80_ram)
	AM_RANGE(0xa04000, 0xa04003) AM_DEVREADWRITE8("ymsnd", megadriv_68k_YM2612_read,megadriv_68k_YM2612_write, 0xffff)

	AM_RANGE(0xa06000, 0xa06001) AM_WRITE(megadriv_68k_z80_bank_write)

	AM_RANGE(0xa10000, 0xa1001f) AM_READWRITE(megadriv_68k_io_read,megadriv_68k_io_write)

	AM_RANGE(0xa11100, 0xa11101) AM_READWRITE(megadriv_68k_check_z80_bus,megadriv_68k_req_z80_bus)
	AM_RANGE(0xa11200, 0xa11201) AM_WRITE(megadriv_68k_req_z80_reset)

	/* these are fake - remove allocs in VIDEO_START to use these to view ram instead */
//  AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_BASE(&megadrive_vdp_vram)
//  AM_RANGE(0xb10000, 0xb1007f) AM_RAM AM_BASE(&megadrive_vdp_vsram)
//  AM_RANGE(0xb10100, 0xb1017f) AM_RAM AM_BASE(&megadrive_vdp_cram)

	AM_RANGE(0xc00000, 0xc0001f) AM_READWRITE(megadriv_vdp_r,megadriv_vdp_w)
	AM_RANGE(0xd00000, 0xd0001f) AM_READWRITE(megadriv_vdp_r,megadriv_vdp_w) // the earth defend

	AM_RANGE(0xe00000, 0xe0ffff) AM_RAM AM_MIRROR(0x1f0000) AM_BASE(&megadrive_ram)
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
		logerror("%06x: 68000 attempting to access Z80 (read) address space without bus\n", cpu_get_pc(space->cpu));
		return mame_rand(space->machine);
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
		logerror("%06x: 68000 attempting to access Z80 (write) address space without bus\n", cpu_get_pc(space->cpu));
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
	UINT16 nextvalue = mame_rand(space->machine);//read_next_instruction(space)&0xff00;


	/* Check if the 68k has the z80 bus */
	if (!ACCESSING_BITS_0_7) // byte (MSB) access
	{
		if (genz80.z80_has_bus || genz80.z80_is_reset) retvalue = nextvalue | 0x0100;
		else retvalue = (nextvalue & 0xfeff);

		//logerror("%06x: 68000 check z80 Bus (byte MSB access) returning %04x mask %04x\n", cpu_get_pc(space->cpu),retvalue, mem_mask);
		return retvalue;

	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		//logerror("%06x: 68000 check z80 Bus (byte LSB access) %04x\n", cpu_get_pc(space->cpu),mem_mask);
		if (genz80.z80_has_bus || genz80.z80_is_reset) retvalue = 0x0001;
		else retvalue = 0x0000;

		return retvalue;
	}
	else
	{
		//logerror("%06x: 68000 check z80 Bus (word access) %04x\n", cpu_get_pc(space->cpu),mem_mask);
		if (genz80.z80_has_bus || genz80.z80_is_reset) retvalue = nextvalue | 0x0100;
		else retvalue = (nextvalue & 0xfeff);

	//  mame_printf_debug("%06x: 68000 check z80 Bus (word access) %04x %04x\n", cpu_get_pc(space->cpu),mem_mask, retvalue);
		return retvalue;
	}
}


static TIMER_CALLBACK( megadriv_z80_run_state )
{
	/* Is the z80 RESET line pulled? */
	if ( genz80.z80_is_reset )
	{
		devtag_reset( machine, "genesis_snd_z80" );
		machine->device<cpu_device>( "genesis_snd_z80" )->suspend(SUSPEND_REASON_HALT, 1 );
		devtag_reset( machine, "ymsnd" );
	}
	else
	{
		/* Check if z80 has the bus */
		if ( genz80.z80_has_bus )
		{
			machine->device<cpu_device>( "genesis_snd_z80" )->resume(SUSPEND_REASON_HALT );
		}
		else
		{
			machine->device<cpu_device>( "genesis_snd_z80" )->suspend(SUSPEND_REASON_HALT, 1 );
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
			//logerror("%06x: 68000 request z80 Bus (byte MSB access) %04x %04x\n", cpu_get_pc(space->cpu),data,mem_mask);
			genz80.z80_has_bus = 0;
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte MSB access) %04x %04x\n", cpu_get_pc(space->cpu),data,mem_mask);
			genz80.z80_has_bus = 1;
		}
	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		if (data & 0x0001)
		{
			//logerror("%06x: 68000 request z80 Bus (byte LSB access) %04x %04x\n", cpu_get_pc(space->cpu),data,mem_mask);
			genz80.z80_has_bus = 0;
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte LSB access) %04x %04x\n", cpu_get_pc(space->cpu),data,mem_mask);
			genz80.z80_has_bus = 1;
		}
	}
	else // word access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 request z80 Bus (word access) %04x %04x\n", cpu_get_pc(space->cpu),data,mem_mask);
			genz80.z80_has_bus = 0;
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte LSB access) %04x %04x\n", cpu_get_pc(space->cpu),data,mem_mask);
			genz80.z80_has_bus = 1;
		}
	}

	/* If the z80 is running, sync the z80 execution state */
	if ( ! genz80.z80_is_reset )
		timer_set( space->machine, attotime_zero, NULL, 0, megadriv_z80_run_state );
}

static WRITE16_HANDLER ( megadriv_68k_req_z80_reset )
{
	if (!ACCESSING_BITS_0_7) // byte access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 clear z80 reset (byte MSB access) %04x %04x\n", cpu_get_pc(space->cpu),data,mem_mask);
			genz80.z80_is_reset = 0;
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte MSB access) %04x %04x\n", cpu_get_pc(space->cpu),data,mem_mask);
			genz80.z80_is_reset = 1;
		}
	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		if (data & 0x0001)
		{
			//logerror("%06x: 68000 clear z80 reset (byte LSB access) %04x %04x\n", cpu_get_pc(space->cpu),data,mem_mask);
			genz80.z80_is_reset = 0;
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte LSB access) %04x %04x\n", cpu_get_pc(space->cpu),data,mem_mask);
			genz80.z80_is_reset = 1;
		}
	}
	else // word access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 clear z80 reset (word access) %04x %04x\n", cpu_get_pc(space->cpu),data,mem_mask);
			genz80.z80_is_reset = 0;
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte LSB access) %04x %04x\n", cpu_get_pc(space->cpu),data,mem_mask);
			genz80.z80_is_reset = 1;
		}
	}
	timer_set( space->machine, attotime_zero, NULL, 0, megadriv_z80_run_state );
}


// just directly access the 68k space, this makes it easier to deal with
// add-on hardware which changes the cpu mapping like the 32x and SegaCD.
// - we might need to add exceptions for example, z80 reading / writing the
//   z80 area of the 68k if games misbehave
static READ8_HANDLER( z80_read_68k_banked_data )
{
	address_space *space68k = space->machine->device<legacy_cpu_device>("maincpu")->space();
	UINT8 ret = space68k->read_byte(genz80.z80_bank_addr+offset);
	return ret;
}

static WRITE8_HANDLER( z80_write_68k_banked_data )
{
	address_space *space68k = space->machine->device<legacy_cpu_device>("maincpu")->space();
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
			sn76496_w(space->machine->device("snsnd"), 0, data);
			break;

		default:
			mame_printf_debug("unhandled z80 vdp write %02x %02x\n",offset,data);
	}

}



static READ8_HANDLER( megadriv_z80_vdp_read )
{
	mame_printf_debug("megadriv_z80_vdp_read %02x\n",offset);
	return mame_rand(space->machine);
}

static READ8_HANDLER( megadriv_z80_unmapped_read )
{
	return 0xff;
}

static ADDRESS_MAP_START( megadriv_z80_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAMBANK("bank1") AM_MIRROR(0x2000) // RAM can be accessed by the 68k
	AM_RANGE(0x4000, 0x4003) AM_DEVREADWRITE("ymsnd", ym2612_r,ym2612_w)

	AM_RANGE(0x6000, 0x6000) AM_WRITE(megadriv_z80_z80_bank_w)
	AM_RANGE(0x6001, 0x6001) AM_WRITE(megadriv_z80_z80_bank_w) // wacky races uses this address

	AM_RANGE(0x6100, 0x7eff) AM_READ(megadriv_z80_unmapped_read)

	AM_RANGE(0x7f00, 0x7fff) AM_READWRITE(megadriv_z80_vdp_read,megadriv_z80_vdp_write)

	AM_RANGE(0x8000, 0xffff) AM_READWRITE(z80_read_68k_banked_data,z80_write_68k_banked_data) // The Z80 can read the 68k address space this way
ADDRESS_MAP_END

static ADDRESS_MAP_START( megadriv_z80_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0000, 0xff) AM_NOP
ADDRESS_MAP_END


/************************************ Megadrive Bootlegs *************************************/

// smaller ROM region because some bootlegs check for RAM there
static ADDRESS_MAP_START( md_bootleg_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM	/* Cartridge Program Rom */
	AM_RANGE(0x200000, 0x2023ff) AM_RAM // tested

	AM_RANGE(0xa00000, 0xa01fff) AM_READWRITE(megadriv_68k_read_z80_ram, megadriv_68k_write_z80_ram)
	AM_RANGE(0xa02000, 0xa03fff) AM_WRITE(megadriv_68k_write_z80_ram)
	AM_RANGE(0xa04000, 0xa04003) AM_DEVREADWRITE8("ymsnd", megadriv_68k_YM2612_read, megadriv_68k_YM2612_write, 0xffff)
	AM_RANGE(0xa06000, 0xa06001) AM_WRITE(megadriv_68k_z80_bank_write)

	AM_RANGE(0xa10000, 0xa1001f) AM_READWRITE(megadriv_68k_io_read, megadriv_68k_io_write)
	AM_RANGE(0xa11100, 0xa11101) AM_READWRITE(megadriv_68k_check_z80_bus, megadriv_68k_req_z80_bus)
	AM_RANGE(0xa11200, 0xa11201) AM_WRITE(megadriv_68k_req_z80_reset)

	AM_RANGE(0xc00000, 0xc0001f) AM_READWRITE(megadriv_vdp_r, megadriv_vdp_w)
	AM_RANGE(0xd00000, 0xd0001f) AM_READWRITE(megadriv_vdp_r, megadriv_vdp_w) // the earth defend

	AM_RANGE(0xe00000, 0xe0ffff) AM_RAM AM_MIRROR(0x1f0000) AM_BASE(&megadrive_ram)
ADDRESS_MAP_END

MACHINE_CONFIG_DERIVED( md_bootleg, megadriv )

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(md_bootleg_map)
MACHINE_CONFIG_END


/****************************************** 32X related ******************************************/

/**********************************************************************************************/
// Function Prototypes
/**********************************************************************************************/


static READ16_HANDLER( _32x_68k_a15180_r );
static READ16_HANDLER( _32x_68k_a15182_r );
static READ16_HANDLER( _32x_68k_a15184_r );
static READ16_HANDLER( _32x_68k_a15186_r );
static READ16_HANDLER( _32x_68k_a15188_r );
static READ16_HANDLER( _32x_68k_a1518a_r );

static WRITE16_HANDLER( _32x_68k_a15180_w );
static WRITE16_HANDLER( _32x_68k_a15182_w );
static WRITE16_HANDLER( _32x_68k_a15184_w );
static WRITE16_HANDLER( _32x_68k_a15186_w );
static WRITE16_HANDLER( _32x_68k_a15188_w );
static WRITE16_HANDLER( _32x_68k_a1518a_w );

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

	palette_set_color_rgb(space->machine,offset+0x40,pal5bit(r),pal5bit(g),pal5bit(b));

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
	return _32x_access_dram[offset+0x10000];
}

static WRITE16_HANDLER( _32x_68k_dram_overwrite_w )
{
	COMBINE_DATA(&_32x_access_dram[offset+0x10000]);

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

static READ16_HANDLER( _32x_68k_a15112_r)
{
	printf("read write-only FIFO register\n");
	return 0;
}

static WRITE16_HANDLER( _32x_68k_a15112_w )
{
	//printf("write to FIFO %04x!\n", data);

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
			}

			current_fifo_write_pos = 0;
		}
	}
}



/*

15106 DREQ

 ---- ---- F--- -K0R

 F = Fifo FULL
 K = 68k CPU Write mode (0 = no, 1 = CPU write)
 0 = always 0? no, marsch test wants it to be latched or 1
 R = RV (0 = no operation, 1 = DMA Start allowed)

*/

static UINT16 a15106_reg;


static READ16_HANDLER( _32x_68k_a15106_r)
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
			memory_install_rom(space, 0x0000100, 0x03fffff, 0, 0, memory_region(space->machine, "gamecart") + 0x100);

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

static READ16_HANDLER( _32x_68k_a15108_r )
{
	return dreq_src_addr[offset];
}

static WRITE16_HANDLER( _32x_68k_a15108_w )
{
	dreq_src_addr[offset] = (offset == 0) ? (data & 0xff) : (data & 0xfffe);

	if((dreq_src_addr[0]<<16)|dreq_src_addr[1])
		printf("DREQ set SRC = %08x\n",(dreq_src_addr[0]<<16)|dreq_src_addr[1]);
}

static READ16_HANDLER( _32x_68k_a1510c_r )
{
	return dreq_dst_addr[offset];
}

static WRITE16_HANDLER( _32x_68k_a1510c_w )
{
	dreq_dst_addr[offset] = (offset == 0) ? (data & 0xff) : (data & 0xffff);

	if((dreq_dst_addr[0]<<16)|dreq_dst_addr[1])
		printf("DREQ set DST = %08x\n",(dreq_dst_addr[0]<<16)|dreq_dst_addr[1]);
}

static READ16_HANDLER( _32x_68k_a15110_r )
{
	return dreq_size;
}

static WRITE16_HANDLER( _32x_68k_a15110_w )
{
	dreq_size = data & 0xfffc;

//	if(dreq_size)
//		printf("DREQ set SIZE = %04x\n",dreq_size);
}

/*
a1511a SEGA TV register
---- ---x Cartridge Mode (0) ROM (1) DRAM

Sega disallows the use of this register

*/

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
			cpu_set_input_line(_32x_master_cpu, INPUT_LINE_RESET, CLEAR_LINE);
			cpu_set_input_line(_32x_slave_cpu, INPUT_LINE_RESET, CLEAR_LINE);
		}

		if (data & 0x01)
		{
			_32x_adapter_enabled = 1;
			memory_install_rom(space, 0x0880000, 0x08fffff, 0, 0, memory_region(space->machine, "gamecart")); // 'fixed' 512kb rom bank

			memory_install_read_bank(space, 0x0900000, 0x09fffff, 0, 0, "bank12"); // 'bankable' 1024kb rom bank
			memory_set_bankptr(space->machine,  "bank12", memory_region(space->machine, "gamecart") );

			memory_install_rom(space, 0x0000000, 0x03fffff, 0, 0, memory_region(space->machine, "32x_68k_bios"));

			memory_install_readwrite16_handler(space, 0x0a15180, 0x0a15181, 0, 0, _32x_68k_a15180_r, _32x_68k_a15180_w); // mode control regs
			memory_install_readwrite16_handler(space, 0x0a15182, 0x0a15183, 0, 0, _32x_68k_a15182_r, _32x_68k_a15182_w); // screen shift
			memory_install_readwrite16_handler(space, 0x0a15184, 0x0a15185, 0, 0, _32x_68k_a15184_r, _32x_68k_a15184_w); // autofill length reg
			memory_install_readwrite16_handler(space, 0x0a15186, 0x0a15187, 0, 0, _32x_68k_a15186_r, _32x_68k_a15186_w); // autofill address reg
			memory_install_readwrite16_handler(space, 0x0a15188, 0x0a15189, 0, 0, _32x_68k_a15188_r, _32x_68k_a15188_w); // autofill data reg / start fill
			memory_install_readwrite16_handler(space, 0x0a1518a, 0x0a1518b, 0, 0, _32x_68k_a1518a_r, _32x_68k_a1518a_w); // framebuffer control regs

			memory_install_readwrite16_handler(space, 0x0a15200, 0x0a153ff, 0, 0, _32x_68k_palette_r, _32x_68k_palette_w); // access to 'palette' xRRRRRGGGGGBBBBB

			memory_install_readwrite16_handler(space, 0x0840000, 0x085ffff, 0, 0, _32x_68k_dram_r, _32x_68k_dram_w); // access to 'display ram' (framebuffer)
			memory_install_readwrite16_handler(space, 0x0860000, 0x087ffff, 0, 0, _32x_68k_dram_overwrite_r, _32x_68k_dram_overwrite_w); // access to 'display ram' (framebuffer)

			memory_install_readwrite16_handler(cputag_get_address_space(space->machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x000070, 0x000073, 0, 0, _32x_68k_hint_vector_r, _32x_68k_hint_vector_w); // h interrupt vector
		}
		else
		{
			_32x_adapter_enabled = 0;

			memory_install_rom(space, 0x0000000, 0x03fffff, 0, 0, memory_region(space->machine, "gamecart"));
			memory_install_readwrite16_handler(cputag_get_address_space(space->machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x000070, 0x000073, 0, 0, _32x_68k_hint_vector_r, _32x_68k_hint_vector_w); // h interrupt vector
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
			if (sh2_master_cmdint_enable) cpu_set_input_line(_32x_master_cpu,SH2_CINT_IRQ_LEVEL,ASSERT_LINE);
		}

		if (data&0x2)
		{
			if (sh2_slave_cmdint_enable) cpu_set_input_line(_32x_slave_cpu,SH2_CINT_IRQ_LEVEL,ASSERT_LINE);
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

	memory_set_bankptr(space->machine,  "bank12", memory_region(space->machine, "gamecart")+((_32x_68k_a15104_reg&0x3)*0x100000) );
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
	if (_32X_COMMS_PORT_SYNC) timer_call_after_resynch(space->machine, NULL, 0, NULL);
	return commsram[offset];
}

// writes
static WRITE16_HANDLER( _32x_68k_commsram_w )
{
	COMBINE_DATA(&commsram[offset]);
	if (_32X_COMMS_PORT_SYNC) timer_call_after_resynch(space->machine, NULL, 0, NULL);
}

/**********************************************************************************************/
// 68k side a15130 - a1513f
// PWM registers
// access from the SH2 via 4030 - 403f
/**********************************************************************************************/

static UINT16 pwm_ctrl,pwm_cycle,pwm_tm_reg;
static UINT16 pwm_cycle_reg; //used for latching
static emu_timer *_32x_pwm_timer;

static void calculate_pwm_timer(void)
{
	if(pwm_tm_reg == 0) { pwm_tm_reg = 16; } // zero gives max range
	if(pwm_cycle == 0) { pwm_cycle = 4095; } // zero gives max range

	/* if both RMD and LMD are set to OFF or pwm cycle register is one, then PWM timer ticks doesn't occur */
	if(pwm_cycle == 1 || ((pwm_ctrl & 0xf) == 0))
		timer_adjust_oneshot(_32x_pwm_timer, attotime_never, 0);
	else
		timer_adjust_oneshot(_32x_pwm_timer, ATTOTIME_IN_HZ(((MASTER_CLOCK_NTSC*3 / 7) / (pwm_cycle - 1)) * pwm_tm_reg), 0);
}

static TIMER_CALLBACK( _32x_pwm_callback )
{
	// ...

	/* disabled, we need PWM FIFO support first! */
//	if(sh2_master_pwmint_enable) { cpu_set_input_line(_32x_master_cpu, SH2_PINT_IRQ_LEVEL,ASSERT_LINE); }

//	if(sh2_slave_pwmint_enable) { cpu_set_input_line(_32x_slave_cpu, SH2_PINT_IRQ_LEVEL,ASSERT_LINE); }

	timer_adjust_oneshot(_32x_pwm_timer, ATTOTIME_IN_HZ(((MASTER_CLOCK_NTSC*3 / 7) / (pwm_cycle - 1)) * pwm_tm_reg), 0);
}

static READ16_HANDLER( _32x_68k_pwm_control_reg_r )
{
	return pwm_ctrl;
}

static WRITE16_HANDLER( _32x_68k_pwm_control_reg_w )
{
	pwm_ctrl = (data & 0x7f) | (pwm_ctrl & 0xff80);

	calculate_pwm_timer();
}

static WRITE16_HANDLER( _32x_sh2_pwm_control_reg_w )
{
	pwm_ctrl = data & 0xffff;

	pwm_tm_reg = (pwm_ctrl & 0xf00) >> 8;

	calculate_pwm_timer();
}

static READ16_HANDLER( _32x_68k_pwm_cycle_reg_r )
{
	return pwm_cycle_reg;
}

static WRITE16_HANDLER( _32x_68k_pwm_cycle_reg_w )
{
	pwm_cycle = pwm_cycle_reg = data & 0xfff;

	calculate_pwm_timer();
}

/**********************************************************************************************/
// 68k side a15180
// framebuffer control
// also accessed from the SH2 @ 4100
/**********************************************************************************************/

static READ16_HANDLER( _32x_68k_a15180_r )
{
	// the flag is inverted compared to the megadrive
	int ntsc;
	if (megadrive_region_pal) ntsc = 0;
	else ntsc = 1;

	return (ntsc << 15) |
	       (_32x_videopriority << 7 ) |
	       ( _32x_240mode << 6 ) |
	       ( _32x_displaymode << 0 );

}

static WRITE16_HANDLER( _32x_68k_a15180_w )
{
//  printf("_32x_68k_a15180_w (a15180) %04x %04x\n",data,mem_mask);
	if (ACCESSING_BITS_0_7)
	{
		_32x_videopriority = (data & 0x80) >> 7;
		_32x_240mode   = (data & 0x40) >> 6;
		_32x_displaymode   = (data & 0x03) >> 0;
	}

	if (ACCESSING_BITS_8_15)
	{
		// nothing?  (pal flag is read only)
	}
}

/**********************************************************************************************/
// 68k side a15182
// screenshift register
// also accessed from the SH2 @ 4102
// used to shift 32x framebuffer by 1 pixel
/**********************************************************************************************/

static READ16_HANDLER( _32x_68k_a15182_r )
{
	return _32x_screenshift;
}

static WRITE16_HANDLER( _32x_68k_a15182_w )
{
	if (ACCESSING_BITS_0_7)
	{
		_32x_screenshift = data & 1; // allows 1 pixel shifting
	}
	if (ACCESSING_BITS_8_15)
	{

	}
}

/**********************************************************************************************/
// 68k side a15184
// autofill length
// also accessed from the SH2 @ 4104
/**********************************************************************************************/


static READ16_HANDLER( _32x_68k_a15184_r )
{
	return _32x_autofill_length;
}

static WRITE16_HANDLER( _32x_68k_a15184_w )
{
	if (ACCESSING_BITS_0_7)
	{
		_32x_autofill_length = data & 0xff;
	}

	if (ACCESSING_BITS_8_15)
	{

	}
}

/**********************************************************************************************/
// 68k side a15186
// auto fill addres
// also accessed from the SH2 @ 4106
/**********************************************************************************************/


static READ16_HANDLER( _32x_68k_a15186_r )
{
	return _32x_autofill_address;
}

static WRITE16_HANDLER( _32x_68k_a15186_w )
{
	if (ACCESSING_BITS_0_7)
	{
		_32x_autofill_address = (_32x_autofill_address & 0xff00) | (data & 0x00ff);
	}

	if (ACCESSING_BITS_8_15)
	{
		_32x_autofill_address = (_32x_autofill_address & 0x00ff) | (data & 0xff00);
	}
}

/**********************************************************************************************/
// 68k side a15188
// auto fill data (start command)
// also accessed from the SH2 @ 4108
/**********************************************************************************************/


static READ16_HANDLER( _32x_68k_a15188_r )
{
	return _32x_autofill_data;
}

static WRITE16_HANDLER( _32x_68k_a15188_w )
{
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
}


/**********************************************************************************************/
// 68k side a1518a
// framebuffer status / control
// also accessed from the SH2 @ 410A

/*
vhp- ---- ---- --fb

v = 1=vblank   r/o
h = 1=hblank   r/o
p = 0=palette access approval   r/o
- = unused
f = 0=MD framebuffer access, 1 = SH2   r/o
b = 0=DRAM0 accessed by VDP, 1=DRAM1   r/w

*/

/**********************************************************************************************/

static UINT16 _32x_a1518a_reg;
static READ16_HANDLER( _32x_68k_a1518a_r )
{
	UINT16 retdata = _32x_a1518a_reg;
	UINT16 hpos = get_hposition();
	int megadrive_hblank_flag = 0;

	if (megadrive_vblank_flag) retdata |= 0x8000;

	if (hpos>400) megadrive_hblank_flag = 1;
	if (hpos>460) megadrive_hblank_flag = 0;

	if (megadrive_hblank_flag) retdata |= 0x4000;

	if (megadrive_vblank_flag && _32x_access_auth) { retdata |= 2; } // framebuffer approval (TODO: condition is unknown at current time)

	if (megadrive_hblank_flag && megadrive_vblank_flag) { retdata |= 0x2000; } // palette approval (TODO: active high or low?)

	return retdata;
}

static WRITE16_HANDLER( _32x_68k_a1518a_w )
{
	// bit 0 is the framebuffer select;
	_32x_a1518a_reg = (_32x_a1518a_reg & 0xfffe) | (data & 1);

	if (_32x_a1518a_reg & 1)
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

		if (sh2_master_hint_enable) printf("sh2_master_hint_enable enable!\n");
		//if (sh2_master_pwmint_enable) printf("sh2_master_pwn_enable enable!\n");
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

		if (sh2_slave_hint_enable) printf("sh2_slave_hint_enable enable!\n");
		//if (sh2_slave_pwmint_enable) printf("sh2_slave_pwm_enable enable!\n");

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
static UINT16 hcount;

static READ16_HANDLER( _32x_sh2_common_4004_r )
{
	return hcount;
}

static WRITE16_HANDLER( _32x_sh2_common_4004_w )
{
	hcount = data & 0xff;
}


/**********************************************************************************************/
// SH2 side 4006
// DReq Control Register
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_common_4006_r )
{
	printf("DREQ read!\n");
	return 0;
}

static WRITE16_HANDLER( _32x_sh2_common_4006_w )
{
	printf("DREQ write!\n");
}

/**********************************************************************************************/
// SH2 side 4008
// 68k To SH2 DReq Source Address Register ( High Bits )
/**********************************************************************************************/

/**********************************************************************************************/
// SH2 side 400A
// 68k To SH2 DReq Source Address Register ( Low Bits )
/**********************************************************************************************/

/**********************************************************************************************/
// SH2 side 400C
// 68k To SH2 DReq Destination Address Register ( High Bits )
/**********************************************************************************************/

/**********************************************************************************************/
// SH2 side 400E
// 68k To SH2 DReq Destination Address Register ( Low Bits )
/**********************************************************************************************/

/**********************************************************************************************/
// SH2 side 4010
// 68k To SH2 DReq Length Register
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_common_4010_r )
{
//	printf("reading DReq Length!\n");
	return 0x0000;
}

/**********************************************************************************************/
// SH2 side 4012
// FIFO Register (read)
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_common_4012_r )
{
	UINT16 retdat = current_fifo_readblock[current_fifo_read_pos];

	current_fifo_read_pos++;

//	printf("reading FIFO!\n");

	if (current_fifo_readblock == fifo_block_a && !fifo_block_a_full)
		printf("Fifo block a isn't filled!\n");

	if (current_fifo_readblock == fifo_block_b && !fifo_block_b_full)
		printf("Fifo block b isn't filled!\n");


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



/**********************************************************************************************/
// SH2 side 4014
// VRES (md reset button interrupt) clear
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_4014_w ){cpu_set_input_line(_32x_master_cpu,SH2_VRES_IRQ_LEVEL,CLEAR_LINE);}
static WRITE16_HANDLER( _32x_sh2_slave_4014_w ) { cpu_set_input_line(_32x_slave_cpu, SH2_VRES_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 4016
// VINT (vertical interrupt) clear
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_4016_w ){cpu_set_input_line(_32x_master_cpu,SH2_VINT_IRQ_LEVEL,CLEAR_LINE);}
static WRITE16_HANDLER( _32x_sh2_slave_4016_w ) { cpu_set_input_line(_32x_slave_cpu, SH2_VINT_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 4018
// HINT (horizontal interrupt) clear
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_4018_w ){ cpu_set_input_line(_32x_master_cpu,SH2_HINT_IRQ_LEVEL,CLEAR_LINE);}
static WRITE16_HANDLER( _32x_sh2_slave_4018_w ) { cpu_set_input_line(_32x_slave_cpu, SH2_HINT_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 401A
// HINT (control register interrupt) clear
// Note: flag cleared here is a guess, according to After Burner behaviour
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_401a_w ){ _32x_68k_a15102_reg &= ~1; cpu_set_input_line(_32x_master_cpu,SH2_CINT_IRQ_LEVEL,CLEAR_LINE);}
static WRITE16_HANDLER( _32x_sh2_slave_401a_w ) { _32x_68k_a15102_reg &= ~2; cpu_set_input_line(_32x_slave_cpu, SH2_CINT_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 401C
// PINT (PWM timer interrupt) clear
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_401c_w ){ cpu_set_input_line(_32x_master_cpu,SH2_PINT_IRQ_LEVEL,CLEAR_LINE);}
static WRITE16_HANDLER( _32x_sh2_slave_401c_w ) { cpu_set_input_line(_32x_slave_cpu, SH2_PINT_IRQ_LEVEL,CLEAR_LINE);}

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

static READ16_HANDLER( _32x_sh2_pwm_control_reg_r ) { return _32x_68k_pwm_control_reg_r(space, offset, mem_mask); }
//static WRITE16_HANDLER( _32x_sh2_pwm_control_reg_w ) { _32x_sh2_pwm_control_reg_w(space, offset, data, mem_mask); }

/**********************************************************************************************/
// SH2 side 4032
// Cycle Register
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_pwm_cycle_reg_r ) { return _32x_68k_pwm_cycle_reg_r(space, offset, mem_mask); }
static WRITE16_HANDLER( _32x_sh2_pwm_cycle_reg_w ) { _32x_68k_pwm_cycle_reg_w(space, offset, data, mem_mask); }


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

/**********************************************************************************************/
// SH2 side 4100
// Access to Framebuffer control
// maps through to 68k at a15180
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_common_4100_r ) { return _32x_68k_a15180_r(space,offset,mem_mask); }
static WRITE16_HANDLER( _32x_sh2_common_4100_w ) { _32x_68k_a15180_w(space,offset,data,mem_mask); }

/**********************************************************************************************/
// SH2 side 4102
// Screenshift register
// maps through to 68k at a15182
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_common_4102_r ) { return _32x_68k_a15182_r(space,offset,mem_mask); }
static WRITE16_HANDLER( _32x_sh2_common_4102_w ) { _32x_68k_a15182_w(space,offset,data,mem_mask); }

/**********************************************************************************************/
// SH2 side 4104
// autofill length
// maps through to 68k at a15184
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_common_4104_r ) { return _32x_68k_a15184_r(space,offset,mem_mask); }
static WRITE16_HANDLER( _32x_sh2_common_4104_w ) { _32x_68k_a15184_w(space,offset,data,mem_mask); }

/**********************************************************************************************/
// SH2 side 4106
// autofill address
// maps through to 68k at a15186
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_common_4106_r ) { return _32x_68k_a15186_r(space,offset,mem_mask); }
static WRITE16_HANDLER( _32x_sh2_common_4106_w ) { _32x_68k_a15186_w(space,offset,data,mem_mask); }

/**********************************************************************************************/
// SH2 side 4108
// autofill start
// maps through to 68k at a15188
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_common_4108_r ) { return _32x_68k_a15188_r(space,offset,mem_mask); }
static WRITE16_HANDLER( _32x_sh2_common_4108_w ) { _32x_68k_a15188_w(space,offset,data,mem_mask); }

/**********************************************************************************************/
// SH2 side 410a
// framebuffer status / control
// maps through to 68k at a1518a
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_common_410a_r ) { return _32x_68k_a1518a_r(space,offset,mem_mask); }
static WRITE16_HANDLER( _32x_sh2_common_410a_w ) { _32x_68k_a1518a_w(space,offset,data,mem_mask); }

/**********************************************************************************************/
// SH2 side 4200 - 43ff
// framebuffer status / control
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
   to 2x 16-bit handlers here */

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

_32X_MAP_READHANDLERS(pwm_control_reg,pwm_cycle_reg)
_32X_MAP_WRITEHANDLERS(pwm_control_reg,pwm_cycle_reg)

_32X_MAP_READHANDLERS(common_4010,common_4012)

_32X_MAP_READHANDLERS(common_4100,common_4102) // _32x_sh2_common_4100_common_4102_r
_32X_MAP_WRITEHANDLERS(common_4100,common_4102) // _32x_sh2_common_4100_common_4102_w


_32X_MAP_READHANDLERS(common_4104,common_4106) // _32x_sh2_common_4104_common_4106_r
_32X_MAP_WRITEHANDLERS(common_4104,common_4106) // _32x_sh2_common_4104_common_4106_w

_32X_MAP_READHANDLERS(common_4108,common_410a) // _32x_sh2_common_4108_common_410a_r
_32X_MAP_WRITEHANDLERS(common_4108,common_410a) // _32x_sh2_common_4108_common_410a_w


_32X_MAP_RAM_READHANDLERS(framebuffer_dram) // _32x_sh2_framebuffer_dram_r
_32X_MAP_RAM_WRITEHANDLERS(framebuffer_dram) // _32x_sh2_framebuffer_dram_w

_32X_MAP_RAM_READHANDLERS(framebuffer_overwrite_dram) // _32x_sh2_framebuffer_overwrite_dram_r
_32X_MAP_RAM_WRITEHANDLERS(framebuffer_overwrite_dram) // _32x_sh2_framebuffer_overwrite_dram_w

_32X_MAP_RAM_READHANDLERS(paletteram) // _32x_sh2_paletteram_r
_32X_MAP_RAM_WRITEHANDLERS(paletteram) // _32x_sh2_paletteram_w


/**********************************************************************************************/
// SH2 memory maps
/**********************************************************************************************/

static ADDRESS_MAP_START( sh2_main_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM

	AM_RANGE(0x00004000, 0x00004003) AM_READWRITE( _32x_sh2_master_4000_common_4002_r, _32x_sh2_master_4000_common_4002_w )
	AM_RANGE(0x00004004, 0x00004007) AM_READWRITE( _32x_sh2_common_4004_common_4006_r, _32x_sh2_common_4004_common_4006_w)

	AM_RANGE(0x00004010, 0x00004013) AM_READ( _32x_sh2_common_4010_common_4012_r )

	AM_RANGE(0x00004014, 0x00004017) AM_READNOP AM_WRITE( _32x_sh2_master_4014_master_4016_w ) // IRQ clear
	AM_RANGE(0x00004018, 0x0000401b) AM_READNOP AM_WRITE( _32x_sh2_master_4018_master_401a_w ) // IRQ clear
	AM_RANGE(0x0000401c, 0x0000401f) AM_READNOP AM_WRITE( _32x_sh2_master_401c_master_401e_w ) // IRQ clear

	AM_RANGE(0x00004020, 0x0000402f) AM_READWRITE( _32x_sh2_commsram_r, _32x_sh2_commsram_w )
	AM_RANGE(0x00004030, 0x00004033) AM_READWRITE( _32x_sh2_pwm_control_reg_pwm_cycle_reg_r, _32x_sh2_pwm_control_reg_pwm_cycle_reg_w )
	AM_RANGE(0x00004034, 0x0000403f) AM_NOP

	AM_RANGE(0x00004100, 0x00004103) AM_READWRITE( _32x_sh2_common_4100_common_4102_r, _32x_sh2_common_4100_common_4102_w )
	AM_RANGE(0x00004104, 0x00004107) AM_READWRITE( _32x_sh2_common_4104_common_4106_r, _32x_sh2_common_4104_common_4106_w )
	AM_RANGE(0x00004108, 0x0000410b) AM_READWRITE( _32x_sh2_common_4108_common_410a_r, _32x_sh2_common_4108_common_410a_w )
	AM_RANGE(0x00004200, 0x000043ff) AM_READWRITE( _32x_sh2_paletteram_r, _32x_sh2_paletteram_w)

	AM_RANGE(0x04000000, 0x0401ffff) AM_READWRITE(_32x_sh2_framebuffer_dram_r, _32x_sh2_framebuffer_dram_w)
	AM_RANGE(0x04020000, 0x0403ffff) AM_READWRITE(_32x_sh2_framebuffer_overwrite_dram_r, _32x_sh2_framebuffer_overwrite_dram_w)

	AM_RANGE(0x06000000, 0x0603ffff) AM_RAM AM_SHARE("share10")
	AM_RANGE(0x02000000, 0x023fffff) AM_ROM AM_REGION("gamecart_sh2", 0)

	AM_RANGE(0x22000000, 0x223fffff) AM_ROM AM_REGION("gamecart_sh2", 0) // cart mirror (fifa96)

	AM_RANGE(0xc0000000, 0xc0000fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sh2_slave_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM

	AM_RANGE(0x00004000, 0x00004003) AM_READWRITE( _32x_sh2_slave_4000_common_4002_r, _32x_sh2_slave_4000_common_4002_w )
	AM_RANGE(0x00004004, 0x00004007) AM_READWRITE( _32x_sh2_common_4004_common_4006_r, _32x_sh2_common_4004_common_4006_w)

	AM_RANGE(0x00004010, 0x00004013) AM_READ( _32x_sh2_common_4010_common_4012_r )

	AM_RANGE(0x00004014, 0x00004017) AM_READNOP AM_WRITE( _32x_sh2_slave_4014_slave_4016_w ) // IRQ clear
	AM_RANGE(0x00004018, 0x0000401b) AM_READNOP AM_WRITE( _32x_sh2_slave_4018_slave_401a_w ) // IRQ clear
	AM_RANGE(0x0000401c, 0x0000401f) AM_READNOP AM_WRITE( _32x_sh2_slave_401c_slave_401e_w ) // IRQ clear

	AM_RANGE(0x00004020, 0x0000402f) AM_READWRITE( _32x_sh2_commsram_r, _32x_sh2_commsram_w )
	AM_RANGE(0x00004030, 0x00004033) AM_READWRITE( _32x_sh2_pwm_control_reg_pwm_cycle_reg_r, _32x_sh2_pwm_control_reg_pwm_cycle_reg_w )
	AM_RANGE(0x00004034, 0x0000403f) AM_NOP

	AM_RANGE(0x00004100, 0x00004103) AM_READWRITE( _32x_sh2_common_4100_common_4102_r, _32x_sh2_common_4100_common_4102_w )
	AM_RANGE(0x00004104, 0x00004107) AM_READWRITE( _32x_sh2_common_4104_common_4106_r, _32x_sh2_common_4104_common_4106_w )
	AM_RANGE(0x00004108, 0x0000410b) AM_READWRITE( _32x_sh2_common_4108_common_410a_r, _32x_sh2_common_4108_common_410a_w )
	AM_RANGE(0x00004200, 0x000043ff) AM_READWRITE(_32x_sh2_paletteram_r, _32x_sh2_paletteram_w)

	AM_RANGE(0x04000000, 0x0401ffff) AM_READWRITE(_32x_sh2_framebuffer_dram_r, _32x_sh2_framebuffer_dram_w)
	AM_RANGE(0x04020000, 0x0403ffff) AM_READWRITE(_32x_sh2_framebuffer_overwrite_dram_r, _32x_sh2_framebuffer_overwrite_dram_w)

	AM_RANGE(0x06000000, 0x0603ffff) AM_RAM AM_SHARE("share10")
	AM_RANGE(0x02000000, 0x023fffff) AM_ROM AM_REGION("gamecart_sh2", 0)

	AM_RANGE(0x22000000, 0x223fffff) AM_ROM AM_REGION("gamecart_sh2", 0) // cart mirror (fifa96)

	AM_RANGE(0xc0000000, 0xc0000fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( segacd_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x0000000, 0x0003fff) AM_RAM
ADDRESS_MAP_END


/****************************************** END 32X related *************************************/

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
 * acts as PM register (independend on GPO bits).
 */

#include "cpu/ssp1601/ssp1601.h"

static struct svp_vars
{
	UINT8 *iram; // IRAM (0-0x7ff)
	UINT8 *dram; // [0x20000];
	UINT32 pmac_read[6];	// read modes/addrs for PM0-PM5
	UINT32 pmac_write[6];	// write ...
	PAIR pmc;
	#define SSP_PMC_HAVE_ADDR  1  // address written to PMAC, waiting for mode
	#define SSP_PMC_SET        2  // PMAC is set, PMx can be programmed
	UINT32 emu_status;
	UINT16 XST;		// external status, mapped at a15000 and a15002 on 68k side.
	UINT16 XST2;		// status of XST (bit1 set when 68k writes to XST)
} svp;

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
	if (svp.emu_status & SSP_PMC_SET)
	{
		svp.pmac_read[write ? reg + 6 : reg] = svp.pmc.d;
		svp.emu_status &= ~SSP_PMC_SET;
		return 0;
	}

	// just in case
	if (svp.emu_status & SSP_PMC_HAVE_ADDR) {
		svp.emu_status &= ~SSP_PMC_HAVE_ADDR;
	}

	if (reg == 4 || (cpu_get_reg(space->cpu, SSP_ST) & 0x60))
	{
		#define CADDR ((((mode<<16)&0x7f0000)|addr)<<1)
		UINT16 *dram = (UINT16 *)svp.dram;
		if (write)
		{
			int mode = svp.pmac_write[reg]>>16;
			int addr = svp.pmac_write[reg]&0xffff;
			if      ((mode & 0x43ff) == 0x0018) // DRAM
			{
				int inc = get_inc(mode);
				if (mode & 0x0400) {
				       overwrite_write(&dram[addr], d);
				} else dram[addr] = d;
				svp.pmac_write[reg] += inc;
			}
			else if ((mode & 0xfbff) == 0x4018) // DRAM, cell inc
			{
				if (mode & 0x0400) {
				       overwrite_write(&dram[addr], d);
				} else dram[addr] = d;
				svp.pmac_write[reg] += (addr&1) ? 31 : 1;
			}
			else if ((mode & 0x47ff) == 0x001c) // IRAM
			{
				int inc = get_inc(mode);
				((UINT16 *)svp.iram)[addr&0x3ff] = d;
				svp.pmac_write[reg] += inc;
			}
			else
			{
				logerror("ssp FIXME: PM%i unhandled write mode %04x, [%06x] %04x\n",
						reg, mode, CADDR, d);
			}
		}
		else
		{
			int mode = svp.pmac_read[reg]>>16;
			int addr = svp.pmac_read[reg]&0xffff;
			if      ((mode & 0xfff0) == 0x0800) // ROM, inc 1, verified to be correct
			{
				UINT16 *ROM = (UINT16 *) memory_region(space->machine, "maincpu");
				svp.pmac_read[reg] += 1;
				d = ROM[addr|((mode&0xf)<<16)];
			}
			else if ((mode & 0x47ff) == 0x0018) // DRAM
			{
				int inc = get_inc(mode);
				d = dram[addr];
				svp.pmac_read[reg] += inc;
			}
			else
			{
				logerror("ssp FIXME: PM%i unhandled read  mode %04x, [%06x]\n",
						reg, mode, CADDR);
				d = 0;
			}
		}

		// PMC value corresponds to last PMR accessed (not sure).
		svp.pmc.d = svp.pmac_read[write ? reg + 6 : reg];

		return d;
	}

	return (UINT32)-1;
}

static READ16_HANDLER( read_PM0 )
{
	UINT32 d = pm_io(space, 0, 0, 0);
	if (d != (UINT32)-1) return d;
	d = svp.XST2;
	svp.XST2 &= ~2; // ?
	return d;
}

static WRITE16_HANDLER( write_PM0 )
{
	UINT32 r = pm_io(space, 0, 1, data);
	if (r != (UINT32)-1) return;
	svp.XST2 = data; // ?
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
	UINT32 d = pm_io(space, 3, 0, 0);
	if (d != (UINT32)-1) return d;

	return svp.XST;
}

static WRITE16_HANDLER( write_XST )
{
	UINT32 r = pm_io(space, 3, 1, data);
	if (r != (UINT32)-1) return;

	svp.XST2 |= 1;
	svp.XST = data;
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
	if (svp.emu_status & SSP_PMC_HAVE_ADDR) {
		svp.emu_status |= SSP_PMC_SET;
		svp.emu_status &= ~SSP_PMC_HAVE_ADDR;
		return ((svp.pmc.w.l << 4) & 0xfff0) | ((svp.pmc.w.l >> 4) & 0xf);
	} else {
		svp.emu_status |= SSP_PMC_HAVE_ADDR;
		return svp.pmc.w.l;
	}
}

static WRITE16_HANDLER( write_PMC )
{
	if (svp.emu_status & SSP_PMC_HAVE_ADDR) {
		svp.emu_status |= SSP_PMC_SET;
		svp.emu_status &= ~SSP_PMC_HAVE_ADDR;
		svp.pmc.w.h = data;
	} else {
		svp.emu_status |= SSP_PMC_HAVE_ADDR;
		svp.pmc.w.l = data;
	}
}

static READ16_HANDLER( read_AL )
{
	svp.emu_status &= ~(SSP_PMC_SET|SSP_PMC_HAVE_ADDR);
	return 0;
}

static WRITE16_HANDLER( write_AL )
{
}



static READ16_HANDLER( svp_68k_io_r )
{
	UINT32 d;
	switch (offset)
	{
		// 0xa15000, 0xa15002
		case 0:
		case 1:  return svp.XST;
		// 0xa15004
		case 2:  d = svp.XST2; svp.XST2 &= ~1; return d;
		default: logerror("unhandled SVP reg read @ %x\n", offset<<1);
	}
	return 0;
}

static WRITE16_HANDLER( svp_68k_io_w )
{
	switch (offset)
	{
		// 0xa15000, 0xa15002
		case 0:
		case 1:  svp.XST = data; svp.XST2 |= 2; break;
		// 0xa15006
		case 3:  break; // possibly halts SSP1601
		default: logerror("unhandled SVP reg write %04x @ %x\n", data, offset<<1);
	}
}

static READ16_HANDLER( svp_68k_cell1_r )
{
	// this is rewritten 68k test code
	UINT32 a1 = offset;
	a1 = (a1 & 0x7001) | ((a1 & 0x3e) << 6) | ((a1 & 0xfc0) >> 5);
	return ((UINT16 *)svp.dram)[a1];
}

static READ16_HANDLER( svp_68k_cell2_r )
{
	// this is rewritten 68k test code
	UINT32 a1 = offset;
	a1 = (a1 & 0x7801) | ((a1 & 0x1e) << 6) | ((a1 & 0x7e0) >> 4);
	return ((UINT16 *)svp.dram)[a1];
}

static ADDRESS_MAP_START( svp_ssp_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x0000, 0x03ff) AM_ROMBANK("bank3")
	AM_RANGE(0x0400, 0xffff) AM_ROMBANK("bank4")
ADDRESS_MAP_END

static ADDRESS_MAP_START( svp_ext_map, ADDRESS_SPACE_IO, 16 )
	ADDRESS_MAP_GLOBAL_MASK(0xf)
	AM_RANGE(0*2, 0*2+1) AM_READWRITE(read_PM0, write_PM0)
	AM_RANGE(1*2, 1*2+1) AM_READWRITE(read_PM1, write_PM1)
	AM_RANGE(2*2, 2*2+1) AM_READWRITE(read_PM2, write_PM2)
	AM_RANGE(3*2, 3*2+1) AM_READWRITE(read_XST, write_XST)
	AM_RANGE(4*2, 4*2+1) AM_READWRITE(read_PM4, write_PM4)
	AM_RANGE(6*2, 6*2+1) AM_READWRITE(read_PMC, write_PMC)
	AM_RANGE(7*2, 7*2+1) AM_READWRITE(read_AL, write_AL)
ADDRESS_MAP_END


/* DMA read function for SVP */
static UINT16 vdp_get_word_from_68k_mem_svp(running_machine *machine, UINT32 source)
{
	if ((source & 0xe00000) == 0x000000)
	{
		UINT16 *rom = (UINT16*)memory_region(machine, "maincpu");
		source -= 2; // DMA latency
		return rom[source >> 1];
	}
	else if ((source & 0xfe0000) == 0x300000)
	{
		UINT16 *dram = (UINT16*)svp.dram;
		source &= 0x1fffe;
		source -= 2;
		return dram[source >> 1];
	}
	else if ((source & 0xe00000) == 0xe00000)
	{
		return megadrive_ram[(source&0xffff)>>1];
	}
	else
	{
		mame_printf_debug("DMA Read unmapped %06x\n",source);
		return mame_rand(machine);
	}
}

/* emulate testmode plug */
static UINT8 megadrive_io_read_data_port_svp(running_machine *machine, int portnum)
{
	if (portnum == 0 && input_port_read_safe(machine, "MEMORY_TEST", 0x00))
	{
		return (megadrive_io_data_regs[0] & 0xc0);
	}
	return megadrive_io_read_data_port_3button(machine, portnum);
}


static READ16_HANDLER( svp_speedup_r )
{
	 cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(100));
	return 0x0425;
}


static void svp_init(running_machine *machine)
{
	UINT8 *ROM;

	memset(&svp, 0, sizeof(svp));

	/* SVP stuff */
	svp.dram = auto_alloc_array(machine, UINT8, 0x20000);
	memory_install_ram(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x300000, 0x31ffff, 0, 0, svp.dram);
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa15000, 0xa150ff, 0, 0, svp_68k_io_r, svp_68k_io_w);
	// "cell arrange" 1 and 2
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x390000, 0x39ffff, 0, 0, svp_68k_cell1_r);
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x3a0000, 0x3affff, 0, 0, svp_68k_cell2_r);

	memory_install_read16_handler(cputag_get_address_space(machine, "svp", ADDRESS_SPACE_PROGRAM), 0x438, 0x438, 0, 0, svp_speedup_r);

	svp.iram = auto_alloc_array(machine, UINT8, 0x800);
	memory_set_bankptr(machine,  "bank3", svp.iram );
	/* SVP ROM just shares m68k region.. */
	ROM = memory_region(machine, "maincpu");
	memory_set_bankptr(machine,  "bank4", ROM + 0x800 );

	vdp_get_word_from_68k_mem = vdp_get_word_from_68k_mem_svp;
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

MACHINE_CONFIG_DERIVED( megdsvp, megadriv )

	MDRV_CPU_ADD("svp", SSP1601, MASTER_CLOCK_NTSC / 7 * 3) /* ~23 MHz (guessed) */
	MDRV_CPU_PROGRAM_MAP(svp_ssp_map)
	MDRV_CPU_IO_MAP(svp_ext_map)
	/* IRQs are not used by this CPU */
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( megdsvppal, megadpal )

	MDRV_CPU_ADD("svp", SSP1601, MASTER_CLOCK_PAL / 7 * 3) /* ~23 MHz (guessed) */
	MDRV_CPU_PROGRAM_MAP(svp_ssp_map)
	MDRV_CPU_IO_MAP(svp_ext_map)
	/* IRQs are not used by this CPU */
MACHINE_CONFIG_END

/****************************************** END SVP related *************************************/


//static attotime time_elapsed_since_crap;


VIDEO_START(megadriv)
{
	int x;

	render_bitmap = machine->primary_screen->alloc_compatible_bitmap();

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

VIDEO_UPDATE(megadriv)
{
	/* Copy our screen buffer here */
	copybitmap(bitmap, render_bitmap, 0, 0, 0, 0, cliprect);

//  int xxx;
	/* reference */

//  time_elapsed_since_crap = frame_timer->time_elapsed();
//  xxx = screen->machine->device<device>("maincpu")->attotime_to_cycles(time_elapsed_since_crap);
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
	UINT16 window_hpos;
	UINT16 window_down;
	UINT16 window_vpos;
	UINT16 hscroll_base;
	UINT8  vscroll_mode;
	UINT8  hscroll_mode;
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
	window_hpos = MEGADRIVE_REG11_WINDOW_HPOS;
	window_down = MEGADRIVE_REG12_WINDOW_DOWN;
	window_vpos = MEGADRIVE_REG12_WINDOW_VPOS;

	screenwidth = MEGADRIVE_REG0C_RS0 | (MEGADRIVE_REG0C_RS1 << 1);

	switch (screenwidth)
	{
		case 0: numcolumns = 32; window_hsize = 32; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1f) << 11; break;
		case 1: numcolumns = 32; window_hsize = 32; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1f) << 11; break;
		case 2: numcolumns = 40; window_hsize = 64; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1e) << 11; break;
		case 3: numcolumns = 40; window_hsize = 64; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1e) << 11; break; // talespin cares about base mask, used for status bar
	}

	//mame_printf_debug("screenwidth %d\n",screenwidth);

	//base_w = mame_rand(Machine)&0xff;

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


    vscroll_mode = MEGADRIVE_REG0B_VSCROLL_MODE;
    hscroll_mode = MEGADRIVE_REG0B_HSCROLL_MODE;
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
static void genesis_render_videobuffer_to_screenbuffer(running_machine *machine, int scanline)
{
	UINT16*lineptr;
	int x;
	lineptr = BITMAP_ADDR16(render_bitmap, scanline, 0);

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


	if (!MEGADRIVE_REG0C_SHADOW_HIGLIGHT)
	{

		for (x=0;x<320;x++)
		{
			UINT32 dat;
			dat = video_renderline[x];
			if ((dat&0x20000) && (_32x_is_connected) && (_32x_displaymode != 0))
			{
				if (_32x_linerender[x]&0x8000)
				{
					if (_32x_videopriority)
					{
						lineptr[x] = _32x_linerender[x]&0x7fff;
					}
					else
					{
						// display md bg?
					}
				}
				else
				{
					if (_32x_videopriority)
					{
						// display md bg?
					}
					else
					{
						lineptr[x] = _32x_linerender[x]&0x7fff;
					}
				}
			}
			else
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

			if ((dat&0x20000) && (_32x_is_connected) && (_32x_displaymode != 0))
			{
				if (_32x_linerender[x]&0x8000)
				{
					if (_32x_videopriority)
					{
						lineptr[x] = _32x_linerender[x]&0x7fff;
					}
					else
					{
						// display md bg?
					}
				}
				else
				{
					if (_32x_videopriority)
					{
						// display md bg?
					}
					else
					{
						lineptr[x] = _32x_linerender[x]&0x7fff;
					}
				}

			}
			else
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
						lineptr[x] = mame_rand(machine)&0x3f;
					break;
				}
			}
		}

	}


	if (_32x_is_connected && ( _32x_displaymode != 0))
	{
		for (x=0;x<320;x++)
		{
			if (_32x_linerender[x]&0x8000)
			{
				if (_32x_videopriority)
				{
					// display md screen?

				}
				else
				{
					lineptr[x] = _32x_linerender[x]&0x7fff;
				}
			}
			else
			{
				if (_32x_videopriority)
				{
					lineptr[x] = _32x_linerender[x]&0x7fff;
				}
				else
				{
					// display md screen?
				}
			}
		}
	}
}

static void genesis_render_scanline(running_machine *machine, int scanline)
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
		genesis_render_scanline(timer.machine, genesis_scanline_counter);
	}
}


static TIMER_DEVICE_CALLBACK( scanline_timer_callback )
{
	/* This function is called at the very start of every scanline starting at the very
       top-left of the screen.  The first scanline is scanline 0 (we set scanline to -1 in
       VIDEO_EOF) */

	timer_call_after_resynch(timer.machine, NULL, 0, 0);
	/* Compensate for some rounding errors

       When the counter reaches 261 we should have reached the end of the frame, however due
       to rounding errors in the timer calculation we're not quite there.  Let's assume we are
       still in the previous scanline for now.
    */

	if (genesis_scanline_counter!=(megadrive_total_scanlines-1))
	{
		genesis_scanline_counter++;
//      mame_printf_debug("scanline %d\n",genesis_scanline_counter);
		scanline_timer->adjust(attotime_div(ATTOTIME_IN_HZ(megadriv_framerate), megadrive_total_scanlines));
		render_timer->adjust(ATTOTIME_IN_USEC(1));

		if (genesis_scanline_counter==megadrive_irq6_scanline )
		{
		//  mame_printf_debug("x %d",genesis_scanline_counter);
			irq6_on_timer->adjust(ATTOTIME_IN_USEC(6));
			megadrive_irq6_pending = 1;
			megadrive_vblank_flag = 1;

			// 32x interrupt!
			if (_32x_is_connected)
			{
				if (sh2_master_vint_enable) cpu_set_input_line(_32x_master_cpu,SH2_VINT_IRQ_LEVEL,ASSERT_LINE);
				if (sh2_slave_vint_enable) cpu_set_input_line(_32x_slave_cpu,SH2_VINT_IRQ_LEVEL,ASSERT_LINE);
			}

		}

		if (megadrive_vblank_flag>=224)
			megadrive_vblank_flag = 1;

		if (megadrive_vblank_flag>=236)
			megadrive_vblank_flag = 0;

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
					irq4_on_timer->adjust(ATTOTIME_IN_USEC(1));
					//mame_printf_debug("irq4 on scanline %d reload %d\n",genesis_scanline_counter,MEGADRIVE_REG0A_HINT_VALUE);
				}
			}
		}
		else
		{
			if (megadrive_imode==3) irq4counter = MEGADRIVE_REG0A_HINT_VALUE*2;
			else irq4counter=MEGADRIVE_REG0A_HINT_VALUE;
		}

		//if (genesis_scanline_counter==0) irq4_on_timer->adjust(ATTOTIME_IN_USEC(2));




		if (timer.machine->device("genesis_snd_z80") != NULL)
		{
			if (genesis_scanline_counter == megadrive_z80irq_scanline)
			{
				if ((genz80.z80_has_bus == 1) && (genz80.z80_is_reset == 0))
					cputag_set_input_line(timer.machine, "genesis_snd_z80", 0, HOLD_LINE);
			}
			if (genesis_scanline_counter == megadrive_z80irq_scanline + 1)
			{
				cputag_set_input_line(timer.machine, "genesis_snd_z80", 0, CLEAR_LINE);
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
			cputag_set_input_line(timer.machine, "maincpu", 6, HOLD_LINE);
	}
}

static TIMER_DEVICE_CALLBACK( irq4_on_callback )
{
	//mame_printf_debug("irq4 active on %d\n",genesis_scanline_counter);
	cputag_set_input_line(timer.machine, "maincpu", 4, HOLD_LINE);
}

/*****************************************************************************************/

static int hazemdchoice_megadrive_region_export;
static int hazemdchoice_megadrive_region_pal;
static int hazemdchoice_megadriv_framerate;

MACHINE_START( megadriv )
{
	megadrive_init_io(machine);
}

MACHINE_RESET( megadriv )
{
	/* default state of z80 = reset, with bus */
	mame_printf_debug("Resetting Megadrive / Genesis\n");



	switch (input_port_read_safe(machine, "REGION", 0x00))
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

		default: // as chosen by driver
		megadrive_region_export = hazemdchoice_megadrive_region_export;
		megadrive_region_pal = hazemdchoice_megadrive_region_pal;
		megadriv_framerate = hazemdchoice_megadriv_framerate;
		mame_printf_debug("Using Region = DEFAULT\n");
		break;
	}

	if (machine->device("genesis_snd_z80") != NULL)
	{
		genz80.z80_is_reset = 1;
		genz80.z80_has_bus = 1;
		genz80.z80_bank_addr = 0;
		genesis_scanline_counter = -1;
		timer_set( machine, attotime_zero, NULL, 0, megadriv_z80_run_state );
	}

	megadrive_imode = 0;

	megadrive_reset_io(machine);

	frame_timer = machine->device<timer_device>("frame_timer");
	scanline_timer = machine->device<timer_device>("scanline_timer");
	render_timer = machine->device<timer_device>("render_timer");

	irq6_on_timer = machine->device<timer_device>("irq6_timer");
	irq4_on_timer = machine->device<timer_device>("irq4_timer");

	frame_timer->adjust(attotime_zero);
	scanline_timer->adjust(attotime_zero);

	if (genesis_other_hacks)
	{
	//  set_refresh_rate(megadriv_framerate);
		machine->device("maincpu")->set_clock_scale(0.9950f); /* Fatal Rewind is very fussy... */
	//  machine->device("maincpu")->set_clock_scale(0.3800f); /* Fatal Rewind is very fussy... */

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
		cpu_set_input_line(_32x_master_cpu, INPUT_LINE_RESET, ASSERT_LINE);
	}

	if (_32x_slave_cpu != NULL)
	{
		cpu_set_input_line(_32x_slave_cpu, INPUT_LINE_RESET, ASSERT_LINE);
	}

	if (_segacd_68k_cpu != NULL )
	{
		cpu_set_input_line(_segacd_68k_cpu, INPUT_LINE_RESET, ASSERT_LINE);
		cpu_set_input_line(_segacd_68k_cpu, INPUT_LINE_HALT, ASSERT_LINE);
	}

	current_fifo_block = fifo_block_a;
	current_fifo_readblock = fifo_block_b;
	current_fifo_write_pos = 0;
	current_fifo_read_pos = 0;
	fifo_block_a_full = 0;
	fifo_block_b_full = 0;

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

VIDEO_EOF(megadriv)
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
//  cputag_set_input_line(machine, "genesis_snd_z80", 0, CLEAR_LINE); // if the z80 interrupt hasn't happened by now, clear it..

	if (input_port_read_safe(machine, "RESET", 0x00) & 0x01)
		cputag_set_input_line(machine, "maincpu", INPUT_LINE_RESET, PULSE_LINE);

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
//  mame_printf_debug("my mode %02x", megadrive_vdp_register[0x0c]);

	visarea.min_x = 0;
	visarea.max_x = scr_width-1;
	visarea.min_y = 0;
	visarea.max_y = megadrive_visible_scanlines-1;

	machine->primary_screen->configure(scr_width, megadrive_visible_scanlines, visarea, HZ_TO_ATTOSECONDS(megadriv_framerate));

	if (0)
	{
		//int xxx;
		UINT64 frametime;

	//  /* reference */
		frametime = ATTOSECONDS_PER_SECOND/megadriv_framerate;

		//time_elapsed_since_crap = frame_timer->time_elapsed();
		//xxx = machine->device<cpudevice>("maincpu")->attotime_to_cycles(time_elapsed_since_crap);
		//mame_printf_debug("---------- cycles %d, %08x %08x\n",xxx, (UINT32)(time_elapsed_since_crap.attoseconds>>32),(UINT32)(time_elapsed_since_crap.attoseconds&0xffffffff));
		//mame_printf_debug("---------- framet %d, %08x %08x\n",xxx, (UINT32)(frametime>>32),(UINT32)(frametime&0xffffffff));
		frame_timer->adjust(attotime_zero);
	}

	scanline_timer->adjust(attotime_zero);

}


UINT16* megadriv_backupram;
int megadriv_backupram_length;

#ifndef MESS
static NVRAM_HANDLER( megadriv )
{
	if (megadriv_backupram!=NULL)
	{
		if (read_or_write)
			mame_fwrite(file, megadriv_backupram, megadriv_backupram_length);
		else
		{
			if (file)
			{
				mame_fread(file, megadriv_backupram, megadriv_backupram_length);
			}
			else
			{
				int x;
				for (x=0;x<megadriv_backupram_length/2;x++)
					megadriv_backupram[x]=0xffff;//mame_rand(machine); // dino dini's needs 0xff or game rules are broken
			}
		}
	}
}
#endif


MACHINE_CONFIG_FRAGMENT( megadriv_timers )
	MDRV_TIMER_ADD("frame_timer", frame_timer_callback)
	MDRV_TIMER_ADD("scanline_timer", scanline_timer_callback)
	MDRV_TIMER_ADD("render_timer", render_timer_callback)
	MDRV_TIMER_ADD("irq6_timer", irq6_on_callback)
	MDRV_TIMER_ADD("irq4_timer", irq4_on_callback)
MACHINE_CONFIG_END


MACHINE_CONFIG_START( megadriv, driver_device )
	MDRV_CPU_ADD("maincpu", M68000, MASTER_CLOCK_NTSC / 7) /* 7.67 MHz */
	MDRV_CPU_PROGRAM_MAP(megadriv_map)
	/* IRQs are handled via the timers */

	MDRV_CPU_ADD("genesis_snd_z80", Z80, MASTER_CLOCK_NTSC / 15) /* 3.58 MHz */
	MDRV_CPU_PROGRAM_MAP(megadriv_z80_map)
	MDRV_CPU_IO_MAP(megadriv_z80_io_map)
	/* IRQ handled via the timers */

	MDRV_MACHINE_START(megadriv)
	MDRV_MACHINE_RESET(megadriv)

	MDRV_FRAGMENT_ADD(megadriv_timers)

	MDRV_SCREEN_ADD("megadriv", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB15)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)) // Vblank handled manually.
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 32*8-1, 0, 28*8-1)

#ifndef MESS
	MDRV_NVRAM_HANDLER(megadriv)
#endif

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(megadriv)
	MDRV_VIDEO_UPDATE(megadriv) /* Copies a bitmap */
	MDRV_VIDEO_EOF(megadriv) /* Used to Sync the timing */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2612, MASTER_CLOCK_NTSC/7) /* 7.67 MHz */
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.50)

	/* sound hardware */
	MDRV_SOUND_ADD("snsnd", SMSIII, MASTER_CLOCK_NTSC/15)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25) /* 3.58 MHz */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker",0.25) /* 3.58 MHz */
MACHINE_CONFIG_END

/************ PAL hardware has a different master clock *************/

MACHINE_CONFIG_START( megadpal, driver_device )
	MDRV_CPU_ADD("maincpu", M68000, MASTER_CLOCK_PAL / 7) /* 7.67 MHz */
	MDRV_CPU_PROGRAM_MAP(megadriv_map)
	/* IRQs are handled via the timers */

	MDRV_CPU_ADD("genesis_snd_z80", Z80, MASTER_CLOCK_PAL / 15) /* 3.58 MHz */
	MDRV_CPU_PROGRAM_MAP(megadriv_z80_map)
	MDRV_CPU_IO_MAP(megadriv_z80_io_map)
	/* IRQ handled via the timers */

	MDRV_MACHINE_START(megadriv)
	MDRV_MACHINE_RESET(megadriv)

	MDRV_FRAGMENT_ADD(megadriv_timers)

	MDRV_SCREEN_ADD("megadriv", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB15)
	MDRV_SCREEN_REFRESH_RATE(50)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)) // Vblank handled manually.
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 32*8-1, 0, 28*8-1)

#ifndef MESS
	MDRV_NVRAM_HANDLER(megadriv)
#endif

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(megadriv)
	MDRV_VIDEO_UPDATE(megadriv) /* Copies a bitmap */
	MDRV_VIDEO_EOF(megadriv) /* Used to Sync the timing */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2612, MASTER_CLOCK_PAL/7) /* 7.67 MHz */
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.50)

	/* sound hardware */
	MDRV_SOUND_ADD("snsnd", SMSIII, MASTER_CLOCK_PAL/15)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25) /* 3.58 MHz */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker",0.25) /* 3.58 MHz */
MACHINE_CONFIG_END



static int _32x_fifo_available_callback(UINT32 src, UINT32 dst, UINT32 data, int size)
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

	MDRV_CPU_ADD("32x_master_sh2", SH2, (MASTER_CLOCK_NTSC*3)/7 )
	MDRV_CPU_PROGRAM_MAP(sh2_main_map)
	MDRV_CPU_CONFIG(sh2_conf_master)

	MDRV_CPU_ADD("32x_slave_sh2", SH2, (MASTER_CLOCK_NTSC*3)/7 )
	MDRV_CPU_PROGRAM_MAP(sh2_slave_map)
	MDRV_CPU_CONFIG(sh2_conf_slave)

	// brutal needs at least 30000 or the backgrounds don't animate properly / lock up, and the game
	// freezes.  Some stage seem to need as high as 80000 ?   this *KILLS* performance
	//
	// boosting the interleave here actually makes Kolibri run incorrectly however, that
	// one works best just boosting the interleave on communications?!
	MDRV_QUANTUM_TIME(HZ(1800000))
MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( genesis_32x_pal, megadpal )

	MDRV_CPU_ADD("32x_master_sh2", SH2, (MASTER_CLOCK_PAL*3)/7 )
	MDRV_CPU_PROGRAM_MAP(sh2_main_map)
	MDRV_CPU_CONFIG(sh2_conf_master)

	MDRV_CPU_ADD("32x_slave_sh2", SH2, (MASTER_CLOCK_PAL*3)/7 )
	MDRV_CPU_PROGRAM_MAP(sh2_slave_map)
	MDRV_CPU_CONFIG(sh2_conf_slave)

MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( genesis_scd, megadriv )

	MDRV_CPU_ADD("segacd_68k", M68000, SEGACD_CLOCK ) /* 12.5 MHz */
	MDRV_CPU_PROGRAM_MAP(segacd_map)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( genesis_32x_scd, genesis_32x )

	MDRV_CPU_ADD("segacd_68k", M68000, SEGACD_CLOCK ) /* 12.5 MHz */
	MDRV_CPU_PROGRAM_MAP(segacd_map)
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

static int megadriv_tas_callback(running_device *device)
{
	return 0; // writeback not allowed
}

static void megadriv_init_common(running_machine *machine)
{
	const input_port_token *ipt = machine->gamedrv->ipt;

	/* Look to see if this system has the standard Sound Z80 */
	_genesis_snd_z80_cpu = machine->device<cpu_device>("genesis_snd_z80");
	if (_genesis_snd_z80_cpu != NULL)
	{
		//printf("GENESIS Sound Z80 cpu found '%s'\n", _genesis_snd_z80_cpu->tag() );

		genz80.z80_prgram = auto_alloc_array(machine, UINT8, 0x2000);
		memory_set_bankptr(machine,  "bank1", genz80.z80_prgram );
	}

	/* Look to see if this system has the 32x Master SH2 */
	_32x_master_cpu = machine->device<cpu_device>("32x_master_sh2");
	if (_32x_master_cpu != NULL)
	{
		printf("32x MASTER SH2 cpu found '%s'\n", _32x_master_cpu->tag() );
	}

	/* Look to see if this system has the 32x Slave SH2 */
	_32x_slave_cpu = machine->device<cpu_device>("32x_slave_sh2");
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
		_32x_pwm_timer = timer_alloc(machine, _32x_pwm_callback, 0);
		timer_adjust_oneshot(_32x_pwm_timer, attotime_never, 0);
	}

	_segacd_68k_cpu = machine->device<cpu_device>("segacd_68k");
	if (_segacd_68k_cpu != NULL)
	{
		printf("Sega CD secondary 68k cpu found '%s'\n", _segacd_68k_cpu->tag() );
	}

	_svp_cpu = machine->device<cpu_device>("svp");
	if (_svp_cpu != NULL)
	{
		printf("SVP (cpu) found '%s'\n", _svp_cpu->tag() );
	}


	cpu_set_irq_callback(machine->device("maincpu"), genesis_int_callback);
	megadriv_backupram = NULL;
	megadriv_backupram_length = 0;

	vdp_get_word_from_68k_mem = vdp_get_word_from_68k_mem_default;

	m68k_set_tas_callback(machine->device("maincpu"), megadriv_tas_callback);

	if ((ipt == INPUT_PORTS_NAME(megadri6)) || (ipt == INPUT_PORTS_NAME(ssf2mdb)) || (ipt == INPUT_PORTS_NAME(mk3mdb)))
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
		UINT16 *rom = (UINT16*)memory_region(machine, "maincpu");

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
void megatech_set_megadrive_z80_as_megadrive_z80(running_machine *machine, const char* tag)
{
	running_device *ym = machine->device("ymsnd");

	/* INIT THE PORTS *********************************************************************************************/
	memory_install_readwrite8_handler(cputag_get_address_space(machine, tag, ADDRESS_SPACE_IO), 0x0000, 0xffff, 0, 0, z80_unmapped_port_r, z80_unmapped_port_w);

	/* catch any addresses that don't get mapped */
	memory_install_readwrite8_handler(cputag_get_address_space(machine, tag, ADDRESS_SPACE_PROGRAM), 0x0000, 0xffff, 0, 0, z80_unmapped_r, z80_unmapped_w);


	memory_install_readwrite_bank(cputag_get_address_space(machine, tag, ADDRESS_SPACE_PROGRAM), 0x0000, 0x1fff, 0, 0, "bank1");
	memory_set_bankptr(machine,  "bank1", genz80.z80_prgram );

	memory_install_ram(cputag_get_address_space(machine, tag, ADDRESS_SPACE_PROGRAM), 0x0000, 0x1fff, 0, 0, genz80.z80_prgram);


	// not allowed??
//  memory_install_readwrite_bank(cputag_get_address_space(machine, tag, ADDRESS_SPACE_PROGRAM), 0x2000, 0x3fff, 0, 0, "bank1");

	memory_install_readwrite8_device_handler(cputag_get_address_space(machine, tag, ADDRESS_SPACE_PROGRAM), ym, 0x4000, 0x4003, 0, 0, ym2612_r, ym2612_w);
	memory_install_write8_handler    (cputag_get_address_space(machine, tag, ADDRESS_SPACE_PROGRAM), 0x6000, 0x6000, 0, 0, megadriv_z80_z80_bank_w);
	memory_install_write8_handler    (cputag_get_address_space(machine, tag, ADDRESS_SPACE_PROGRAM), 0x6001, 0x6001, 0, 0, megadriv_z80_z80_bank_w);
	memory_install_read8_handler     (cputag_get_address_space(machine, tag, ADDRESS_SPACE_PROGRAM), 0x6100, 0x7eff, 0, 0, megadriv_z80_unmapped_read);
	memory_install_readwrite8_handler(cputag_get_address_space(machine, tag, ADDRESS_SPACE_PROGRAM), 0x7f00, 0x7fff, 0, 0, megadriv_z80_vdp_read, megadriv_z80_vdp_write);
	memory_install_readwrite8_handler(cputag_get_address_space(machine, tag, ADDRESS_SPACE_PROGRAM), 0x8000, 0xffff, 0, 0, z80_read_68k_banked_data, z80_write_68k_banked_data);
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
		memory_install_rom(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0000000, 0x03fffff, 0, 0, memory_region(machine, "gamecart"));
		memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x000070, 0x000073, 0, 0, _32x_68k_hint_vector_r, _32x_68k_hint_vector_w); // h interrupt vector
	};


	a15100_reg = 0x0000;
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa15100, 0xa15101, 0, 0, _32x_68k_a15100_r, _32x_68k_a15100_w); // framebuffer control regs
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa15102, 0xa15103, 0, 0, _32x_68k_a15102_r, _32x_68k_a15102_w); // send irq to sh2
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa15104, 0xa15105, 0, 0, _32x_68k_a15104_r, _32x_68k_a15104_w); // 68k BANK rom set
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa15106, 0xa15107, 0, 0, _32x_68k_a15106_r, _32x_68k_a15106_w); // dreq stuff
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa15108, 0xa1510b, 0, 0, _32x_68k_a15108_r, _32x_68k_a15108_w); // dreq src address
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa1510c, 0xa1510f, 0, 0, _32x_68k_a1510c_r, _32x_68k_a1510c_w); // dreq dst address
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa15110, 0xa15111, 0, 0, _32x_68k_a15110_r, _32x_68k_a15110_w); // dreq length
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa15112, 0xa15113, 0, 0, _32x_68k_a15112_r, _32x_68k_a15112_w); // fifo

	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa1511a, 0xa1511b, 0, 0, _32x_68k_a1511a_r, _32x_68k_a1511a_w); // SEGA TV

	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa15120, 0xa1512f, 0, 0, _32x_68k_commsram_r, _32x_68k_commsram_w); // comms reg 0-7
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa15130, 0xa15131, 0, 0, _32x_68k_pwm_control_reg_r, _32x_68k_pwm_control_reg_w);
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa15132, 0xa15133, 0, 0, _32x_68k_pwm_cycle_reg_r, _32x_68k_pwm_cycle_reg_w);

	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0a130ec, 0x0a130ef, 0, 0, _32x_68k_MARS_r); // system ID


	/* Interrupts are masked / disabled at first */
	sh2_master_vint_enable = sh2_slave_vint_enable = 0;
	sh2_master_hint_enable = sh2_slave_hint_enable = 0;
	sh2_master_cmdint_enable = sh2_slave_cmdint_enable = 0;
	sh2_master_pwmint_enable = sh2_slave_pwmint_enable = 0;

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
	sh2drc_set_options(machine->device("32x_master_sh2"), SH2DRC_COMPATIBLE_OPTIONS);
	sh2drc_set_options(machine->device("32x_slave_sh2"), SH2DRC_COMPATIBLE_OPTIONS);

	DRIVER_INIT_CALL(megadriv);
}

#if 1

ROM_START( 32x_bios )

	ROM_REGION16_BE( 0x400000, "gamecart", 0 ) /* 68000 Code */
	// test sets
//  ROM_LOAD( "32xquin.rom",  0x000000,  0x05d124, CRC(93d4b0a3) SHA1(128bd0b6e048c749da1a2f4c3abd6a867539a293))
//  ROM_LOAD( "32x_babe.rom", 0x000000,  0x014f80, CRC(816b0cb4) SHA1(dc16d3170d5809b57192e03864b7136935eada64) )
//  ROM_LOAD( "32x_hot.rom",  0x000000,  0x01235c, CRC(da9c93c9) SHA1(a62652eb8ad8c62b36f6b1ffb96922d045c4e3ac))
//  ROM_LOAD( "32x_rot.bin",  0x000000,  0x001638, CRC(98c25033) SHA1(8d9ab3084bd29e60b8cdf4b9f1cb755eb4c88d29) )
//  ROM_LOAD( "32x_3d.bin",   0x000000,  0x006568, CRC(0171743e) SHA1(bbe6fec182baae5e4d47d263fae6b419db5366ae) )
//  ROM_LOAD( "32x_h15.bin",  0x000000,  0x024564, CRC(938f4e1d) SHA1(ab7270121be53c6c82c4cb45f8f41dd24eb3a2a5) ) // test demo for 15bpp mode
//  ROM_LOAD( "32x_spin.bin", 0x000000,  0x012c28, CRC(3d1d1191) SHA1(221a74408653e18cef8ce2f9b4d33ed93e4218b7) )
//  ROM_LOAD( "32x_ecco.bin", 0x000000,  0x300000, CRC(b06178df) SHA1(10409f2245b058e8a32cba51e1ea391ca4480108) ) // fails after sega logo

	// actual games, for testing
//  ROM_LOAD( "32x_knux.rom", 0x000000,  0x300000, CRC(d0b0b842) SHA1(0c2fff7bc79ed26507c08ac47464c3af19f7ced7) )
//  ROM_LOAD( "32x_doom.bin", 0x000000,  0x300000, CRC(208332fd) SHA1(b68e9c7af81853b8f05b8696033dfe4c80327e38) ) // works!
//  ROM_LOAD( "32x_koli.bin", 0x000000,  0x300000, CRC(20ca53ef) SHA1(191ae0b525ecf32664086d8d748e0b35f776ddfe) ) // works but needs sync ONLY on command writes / reads or game stutters?!
//  ROM_LOAD( "32x_head.bin", 0x000000,  0x300000, CRC(ef5553ff) SHA1(4e872fbb44ecb2bd730abd8cc8f32f96b10582c0) ) // doesn't boot
//  ROM_LOAD( "32x_pit.bin",  0x000000,  0x300000, CRC(f9126f15) SHA1(ee864d1677c6d976d0846eb5f8d8edb839acfb76) ) // ok, needs vram fill on intro screens tho?
//  ROM_LOAD( "32x_spid.bin", 0x000000,  0x300000, CRC(29dce257) SHA1(7cc2ea1e10f110338ad880bd3e7ff3bce72e7e9e) ) // needs cmdint status reads, overwrite image support wrong? priority handling wrong??
//  ROM_LOAD( "32x_carn.bin", 0x000000,  0x300000, CRC(7c7be6a2) SHA1(9a563ed821b483148339561ebd2b876efa58847b) ) // ?? doesn't boot
//  ROM_LOAD( "32x_raw.bin",  0x000000,  0x400000, CRC(8eb7cd2c) SHA1(94b974f2f69f0c10bc18b349fa4ff95ca56fa47b) ) // needs cmdint status reads
//  ROM_LOAD( "32x_darx.bin", 0x000000,  0x200000, CRC(22d7c906) SHA1(108b4ffed8643abdefa921cfb58389b119b47f3d) ) // ?? probably abuses the hardware, euro only ;D
//  ROM_LOAD( "32x_prim.bin", 0x000000,  0x400000, CRC(e78a4d28) SHA1(5084dcca51d76173c383ab7d04cbc661673545f7) ) // needs tight sync or fails after sega logo - works with tight sync, but VERY slow
//  ROM_LOAD( "32x_brut.bin", 0x000000,  0x300000, CRC(7a72c939) SHA1(40aa2c787f37772cdbd7280b8be06b15421fabae) ) // needs *very* heavy sync to work..
//  ROM_LOAD( "32x_temp.bin", 0x000000,  0x300000, CRC(14e5c575) SHA1(6673ba83570b4f2c1b4a22415a56594c3cc6c6a9) ) // works (heavy slowdowns) RV emulation - really should hide 68k rom when transfer is off
//  ROM_LOAD( "32x_vr.bin",   0x000000,  0x300000, CRC(7896b62e) SHA1(18dfdeb50780c2623e60a6587d7ed701a1cf81f1) ) // doesn't work
//  ROM_LOAD( "32x_vf.bin",   0x000000,  0x400000, CRC(b5de9626) SHA1(f35754f4bfe3a53722d7a799f88face0fd13c424) ) // locks up when starting game
//  ROM_LOAD( "32x_zaxx.bin", 0x000000,  0x200000, CRC(447d44be) SHA1(60c390f76c394bdd221936c21aecbf98aec49a3d) ) // nothing
//  ROM_LOAD( "32x_trek.bin", 0x000000,  0x200000, CRC(dd9708b9) SHA1(e5248328b64a1ec4f1079c88ee53ef8d48e99e58) ) // boots, seems to run.. enables hints tho
//  ROM_LOAD( "32x_sw.bin",   0x000000,  0x280000, CRC(2f16b44a) SHA1(f4ffaaf1d8330ea971643021be3f3203e1ea065d) ) // gets stuck in impossible (buggy?) 68k loop
//  ROM_LOAD( "32x_wwfa.bin", 0x000000,  0x400000, CRC(61833503) SHA1(551eedc963cba0e1410b3d229b332ef9ea061469) ) // 32x game gfx missing, doesn't progress properly into game
//  ROM_LOAD( "32x_shar.bin", 0x000000,  0x200000, CRC(86e7f989) SHA1(f32a52a7082761982024e40291dbd962a835b231) ) // doesn't boot
//  ROM_LOAD( "32x_golf.bin", 0x000000,  0x300000, CRC(d3d0a2fe) SHA1(dc77b1e5c888c2c4284766915a5020bb14ee681d) ) // works
//  ROM_LOAD( "32x_moto.bin", 0x000000,  0x200000, CRC(a21c5761) SHA1(5f1a107991aaf9eff0b3ce864b2e3151f56abe7b) ) // works (with sound!)
//  ROM_LOAD( "32x_tmek.bin", 0x000000,  0x300000, CRC(66d2c48f) SHA1(173c8425921d83db3e8d181158e7599364f4c0f6) ) // works?
//  ROM_LOAD( "32x_bcr.bin",  0x000000,  0x300000, CRC(936c3d27) SHA1(9b5fd499eaa442d48a2c97fceb1d505dc8e8ddff) ) // overwrite image problems, locks going ingame
//  ROM_LOAD( "32x_blak.bin", 0x000000,  0x300000, CRC(d1a60a47) SHA1(4bf120cf056fe1417ca5b02fa0372ef33cb8ec11) ) // works?
//  ROM_LOAD( "32x_shad.bin", 0x000000,  0x200000, CRC(60c49e4d) SHA1(561c8c63dbcabc0b1b6f31673ca75a0bde7abc72) ) // works (nasty sound)
//  ROM_LOAD( "32x_abur.bin", 0x000000,  0x200000, CRC(204044c4) SHA1(9cf575feb036e2f26e78350154d5eb2fd3825325) ) // doesn't boot
//  ROM_LOAD( "32x_darx.bin", 0x000000,  0x200000, CRC(22d7c906) SHA1(108b4ffed8643abdefa921cfb58389b119b47f3d) ) // doesn't boot (PAL only too)
//  ROM_LOAD( "32x_fifa.bin", 0x000000,  0x300000, CRC(fb14a7c8) SHA1(131ebb717dee4dd1d8f5ab2b9393c23785d3a359) ) // crash
//  ROM_LOAD( "32x_tman.bin", 0x000000,  0x400000, CRC(14eac7a6) SHA1(7588b0b8f4e93d5fdc920d3ab7e464154e423da9) ) // ok, some bad gfx
//  ROM_LOAD( "32x_nba.bin",  0x000000,  0x400000, CRC(6b7994aa) SHA1(c8af3e74c49514669ba6652ec0c81bccf77873b6) ) // crash
//  ROM_LOAD( "32x_nfl.bin",  0x000000,  0x300000, CRC(0bc7018d) SHA1(a0dc24f2f3a7fc5bfd12791cf25af7f7888843cf) ) // doesn't boot
//  ROM_LOAD( "32x_rbi.bin",  0x000000,  0x200000, CRC(ff795fdc) SHA1(4f90433a4403fd74cafeea49272689046de4ae43) ) // doesn't boot
	ROM_LOAD( "32x_wsb.bin",  0x000000,  0x300000, CRC(6de1bc75) SHA1(ab3026eae46a775adb7eaebc13702699557ddc41) ) // working - overwrite problems
//  ROM_LOAD( "32x_mk2.bin",  0x000000,  0x400000, CRC(211085ce) SHA1(f75698de887d0ef980f73e35fc4615887a9ad58f) ) // working
//  ROM_LOAD( "32x_sang.bin", 0x000000,  0x400000, CRC(e4de7625) SHA1(74a3ba27c55cff12409bf6c9324ece6247abbad1) ) // hangs after sega logo

//  ROM_LOAD( "32x_mars.bin", 0x000000,  0x400000, CRC(8f7260fb) SHA1(7654c6d3cf2883c30df51cf38d723ab7902280c4) ) // official hw test program? reports lots of errors seems to get stuck on test 39?

	ROM_REGION32_BE( 0x400000, "gamecart_sh2", 0 ) /* Copy for the SH2 */
	ROM_COPY( "gamecart", 0x0, 0x0, 0x400000)

	ROM_REGION16_BE( 0x400000, "32x_68k_bios", 0 ) /* 68000 Code */
//  ROM_COPY( "gamecart", 0x0, 0x0, 0x400000)
	ROM_LOAD( "32x_g_bios.bin", 0x000000,  0x000100, CRC(5c12eae8) SHA1(dbebd76a448447cb6e524ac3cb0fd19fc065d944) )

	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	// temp, rom should only be visible here when one of the regs is set, tempo needs it
	ROM_COPY( "gamecart", 0x0, 0x0, 0x400000)
	ROM_COPY( "32x_68k_bios", 0x0, 0x0, 0x100)

	ROM_REGION( 0x400000, "32x_master_sh2", 0 ) /* SH2 Code */
	ROM_LOAD( "32x_m_bios.bin", 0x000000,  0x000800, CRC(dd9c46b8) SHA1(1e5b0b2441a4979b6966d942b20cc76c413b8c5e) )

	ROM_REGION( 0x400000, "32x_slave_sh2", 0 ) /* SH2 Code */
	ROM_LOAD( "32x_s_bios.bin", 0x000000,  0x000400, CRC(bfda1fe5) SHA1(4103668c1bbd66c5e24558e73d4f3f92061a109a) )
ROM_END

ROM_START( segacd )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "segacd_model2_bios_2_11_u.bin", 0x000000,  0x020000, CRC(2e49d72c) SHA1(328a3228c29fba244b9db2055adc1ec4f7a87e6b) )
ROM_END

/* some games use the 32x and SegaCD together to give better quality FMV */
ROM_START( 32x_scd )
	ROM_REGION16_BE( 0x400000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION16_BE( 0x400000, "gamecart", 0 ) /* 68000 Code */
	ROM_LOAD( "segacd_model2_bios_2_11_u.bin", 0x000000,  0x020000, CRC(2e49d72c) SHA1(328a3228c29fba244b9db2055adc1ec4f7a87e6b) )

	ROM_REGION16_BE( 0x400000, "32x_68k_bios", 0 ) /* 68000 Code */
	ROM_LOAD( "32x_g_bios.bin", 0x000000,  0x000100, CRC(5c12eae8) SHA1(dbebd76a448447cb6e524ac3cb0fd19fc065d944) )

	ROM_REGION( 0x400000, "32x_master_sh2", 0 ) /* SH2 Code */
	ROM_LOAD( "32x_m_bios.bin", 0x000000,  0x000800, CRC(dd9c46b8) SHA1(1e5b0b2441a4979b6966d942b20cc76c413b8c5e) )

	ROM_REGION( 0x400000, "32x_slave_sh2", 0 ) /* SH2 Code */
	ROM_LOAD( "32x_s_bios.bin", 0x000000,  0x000400, CRC(bfda1fe5) SHA1(4103668c1bbd66c5e24558e73d4f3f92061a109a) )
ROM_END

ROM_START( g_virr )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "g_virr.bin", 0x000000, 0x200000, CRC(7e1a324a) SHA1(ff969ae53120cc4e7cb1a8a7e47458f2eb8a2165) )
ROM_END
ROM_START( g_virrj )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "g_virrj.bin", 0x000000, 0x200000, CRC(53a293b5) SHA1(0ad38a3ab1cc99edac72184f8ae420e13df5cac6) )
ROM_END
ROM_START( g_virre )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "g_virre.bin", 0x000000, 0x200000, CRC(9624d4ef) SHA1(2c3812f8a010571e51269a33a989598787d27c2d) )
ROM_END
ROM_START( g_virrea )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "g_virrea.bin", 0x000000, 0x200000, CRC(5a943df9) SHA1(2c08ea556c79d48e88ff5202944c161ae1b41c63) )
ROM_END

#ifndef MESS
GAME( 1994, 32x_bios,    0,        genesis_32x,        megadriv,    _32x,    ROT0,   "Sega", "32X Bios", GAME_NOT_WORKING )
GAME( 1994, segacd,      0,        genesis_scd,        megadriv,    megadriv,ROT0,   "Sega", "Sega-CD Model 2 BIOS V2.11 (U)", GAME_NOT_WORKING )
GAME( 1994, 32x_scd,     0,        genesis_32x_scd,    megadriv,    _32x,    ROT0,   "Sega", "Sega-CD Model 2 BIOS V2.11 (U) (with 32X)", GAME_NOT_WORKING )
GAME( 1994, g_virr,      0,        megdsvp,            megadriv,   megadriv, ROT0,   "Sega", "Virtua Racing (U) [!]", 0 )
GAME( 1994, g_virrj,     g_virr,   megdsvp,            megadriv,   megadrij, ROT0,   "Sega", "Virtua Racing (J) [!]", 0 )
GAME( 1994, g_virre,     g_virr,   megdsvppal,         megadriv,   megadrie, ROT0,   "Sega", "Virtua Racing (E) [!]", 0 )
GAME( 1994, g_virrea,    g_virr,   megdsvppal,         megadriv,   megadrie, ROT0,   "Sega", "Virtua Racing (E) [a1]", 0 )
#endif

#endif
