
/*

Megadrive / Genesis Rewrite, Take 65498465432356345250432.3  August 06

Thanks to:
Charles Macdonald for much useful information (cgfm2.emuviews.com)

Long Decription names mostly taken from the GoodGen database

ToDo:

The Code here is terrible for now, this is just for testing
Fix HV Counter & Raster Implementation (One line errors in some games, others not working eg. Dracula)
Fix Horizontal timings (currently a kludge, currently doesn't change with resolution changes)
Add Real DMA timings (using a timer)
Add All VDP etc. Mirror addresses (not done yet as I prefer to catch odd cases for now)
Investigate other Bugs (see list bloew)
Rewrite (again) using cleaner, more readable and better optimized code with knowledge gained
Add support for other peripherals (6 player pad, Teamplay Adapters, Lightguns, Sega Mouse etc.)
Sort out set info, making sure all games have right manufacturers, dates etc.
Make sure everything that needs backup RAM has it setup and working
Fix Reset glitches
Add 32X / SegaCD support
Add Megaplay, Megatech support (needs SMS emulation too)
Add other obscure features (palette flicker for mid-screen CRAM changes, undocumented register behavior)
Figure out how sprite masking *really* works
Add EEprom support in games that need it

Known Issues:
    Bass Masters Classic Pro Edition (U) [!] - Sega Logo is corrupt
    Bram Stoker's Dracula (U) [!] - Doesn't Work (HV Timing)
    Double Dragon 2 - The Revenge (J) [!] - Too Slow?
    International Superstar Soccer Deluxe (E) [!] - Single line Raster Glitch
    Lemmings (JU) (REV01) [!] - Rasters off by ~7-8 lines (strange case)
    Mercs - Sometimes sound doesn't work..

    Some beta games without proper sound programs seem to crash because the z80 crashes

Known Non-Issues (confirmed on Real Genesis)
    Castlevania - Bloodlines (U) [!] - Pause text is missing on upside down level
    Blood Shot (E) (M4) [!] - corrupt texture in level 1 is correct...
*/

#include "driver.h"
#include "deprecat.h"
#include "sound/sn76496.h"
#include "sound/2612intf.h"
#include "sound/upd7759.h"
#include "sound/fm.h"
#include "cpu/m68000/m68000.h"
#include "megadriv.h"

#define MEGADRIV_VDP_VRAM(address) megadrive_vdp_vram[(address)&0x7fff]

/* the same on all systems? */
#define MASTER_CLOCK		53693100
/* timing details */
static int megadriv_framerate = 60;
static int megadrive_total_scanlines = 262;
static int megadrive_visible_scanlines = 224;
static int megadrive_irq6_scanline = 224;
//int megadrive_irq6_hpos = 320;
static int megadrive_z80irq_scanline = 226;
//int megadrive_z80irq_hpos = 320;
static int megadrive_imode = 0;
static int megadrive_imode_odd_frame = 0;
static int megadrive_vblank_flag = 0;
static int megadrive_irq6_pending = 0;
static int megadrive_irq4_pending = 0;

INLINE UINT16 get_hposition(void);

static UINT8* sprite_renderline;
static UINT8* highpri_renderline;
static UINT16* video_renderline;
static UINT16* megadrive_vdp_palette_lookup;
static UINT16* megadrive_vdp_palette_lookup_shadow;
static UINT16* megadrive_vdp_palette_lookup_highlight;
static UINT8 oldscreenwidth = 3;
UINT16* megadrive_ram;
static UINT8 megadrive_vram_fill_pending = 0;
static UINT16 megadrive_vram_fill_length = 0;
static int genesis_scanline_counter = 0;
static int megadrive_sprite_collision = 0;
static int megadrive_region_export;
static int megadrive_region_pal;
static int megadrive_max_hposition;


static emu_timer* frame_timer;
static emu_timer* scanline_timer;
static emu_timer* irq6_on_timer;
static emu_timer* irq4_on_timer;
static bitmap_t* render_bitmap;
//emu_timer* vblankirq_off_timer;


#ifdef UNUSED_FUNCTION
/* taken from segaic16.c */
/* doesn't seem to meet my needs, not used */
static UINT16 read_next_instruction(void)
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
	result = program_read_word_16be(activecpu_get_pc());
	recurse = 0;
	return result;
}
#endif



static struct genesis_z80_vars
{
	int z80_cpunum;
	int z80_is_reset;
	int z80_has_bus;
	int z80_bank_pos;
	UINT32 z80_bank_partial;
	UINT32 z80_bank_addr;
	UINT8* z80_prgram;
} genz80;

static void megadriv_z80_bank_w(UINT16 data)
{
	genz80.z80_bank_partial |= (data & 0x01)<<23; // add new bit to partial address
	genz80.z80_bank_pos++;

	if (genz80.z80_bank_pos<9)
	{
		genz80.z80_bank_partial >>= 1;
	}
	else
	{
		genz80.z80_bank_pos = 0;
		genz80.z80_bank_addr = genz80.z80_bank_partial;
		genz80.z80_bank_partial = 0;
	//  logerror("z80 bank set to %08x\n",genz80.z80_bank_addr);

	}
}

static WRITE16_HANDLER( megadriv_68k_z80_bank_write )
{
	//logerror("%06x: 68k writing bit to bank register %01x\n", activecpu_get_pc(),data&0x01);
	megadriv_z80_bank_w(data&0x01);
}

static WRITE8_HANDLER(megadriv_z80_z80_bank_w)
{
	//logerror("%04x: z80 writing bit to bank register %01x\n", activecpu_get_pc(),data&0x01);
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

static void write_cram_value(int offset, int data)
{
	megadrive_vdp_cram[offset] = data;

	//logerror("Wrote to CRAM addr %04x data %04x\n",megadrive_vdp_address&0xfffe,megadrive_vdp_cram[megadrive_vdp_address>>1]);

	{
		int r,g,b;
	  	r = ((data >> 1)&0x07);
		g = ((data >> 5)&0x07);
		b = ((data >> 9)&0x07);
		palette_set_color_rgb(Machine,offset,pal3bit(r),pal3bit(g),pal3bit(b));
		megadrive_vdp_palette_lookup[offset] = (b<<2) | (g<<7) | (r<<12);
		megadrive_vdp_palette_lookup_shadow[offset] = (b<<1) | (g<<6) | (r<<11);
		megadrive_vdp_palette_lookup_highlight[offset] = ((b|0x08)<<1) | ((g|0x08)<<6) | ((r|0x08)<<11);

	}
}

static void vdp_cram_write(UINT16 data)
{
	int offset;
	offset = (megadrive_vdp_address&0x7e)>>1;

	write_cram_value(offset,data);

	megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;

	megadrive_vdp_address &=0xffff;
}


static void megadriv_vdp_data_port_w(int data)
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
				vdp_cram_write(data);
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



static void megadrive_vdp_set_register(int regnum, UINT8 value)
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
				cpunum_set_input_line(Machine, 0,4,HOLD_LINE);
			else
				cpunum_set_input_line(Machine, 0,4,CLEAR_LINE);
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
			if (MEGADRIVE_REG01_IRQ6_ENABLE)
				cpunum_set_input_line(Machine, 0,6,HOLD_LINE);
			else
				cpunum_set_input_line(Machine, 0,6,CLEAR_LINE);
		}

		/* ??? */
	//  megadrive_irq6_pending = 0;
	//  megadrive_irq4_pending = 0;

	}


//  if (regnum == 0x0a)
//      mame_printf_debug("Set HINT Reload Register to %d on scanline %d\n",value, genesis_scanline_counter);

//  mame_printf_debug("%06x Setting VDP Register #%02x to %02x\n",activecpu_get_pc(), regnum,value);
}

static void update_megadrive_vdp_code_and_address(void)
{
	megadrive_vdp_code = ((megadrive_vdp_command_part1 & 0xc000) >> 14) |
	                     ((megadrive_vdp_command_part2 & 0x00f0) >> 2);

	megadrive_vdp_address = ((megadrive_vdp_command_part1 & 0x3fff) >> 0) |
                            ((megadrive_vdp_command_part2 & 0x0003) << 14);
}

static UINT16 (*vdp_get_word_from_68k_mem)(UINT32 source);

static UINT16 vdp_get_word_from_68k_mem_default(UINT32 source)
{
	if (( source >= 0x000000 ) && ( source <= 0x3fffff ))
	{
		UINT16 *rom = (UINT16*)memory_region(REGION_CPU1);
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
		mame_printf_debug("DMA Read unmapped %06x\n",source);
		return mame_rand(Machine);
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
static void megadrive_do_insta_68k_to_vram_dma(UINT32 source,int length)
{
	int count;

	if (length==0x00) length = 0xffff;

	/* This is a hack until real DMA timings are implemented */
	cpu_spinuntil_time(ATTOTIME_IN_NSEC(length*1000/3500));

	for (count = 0;count<(length>>1);count++)
	{
		vdp_vram_write(vdp_get_word_from_68k_mem(source));
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


static void megadrive_do_insta_68k_to_cram_dma(UINT32 source,UINT16 length)
{
	int count;

	if (length==0x00) length = 0xffff;

	for (count = 0;count<(length>>1);count++)
	{
		//if (megadrive_vdp_address>=0x80) return; // abandon

		write_cram_value((megadrive_vdp_address&0x7e)>>1, vdp_get_word_from_68k_mem(source));
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

static void megadrive_do_insta_68k_to_vsram_dma(UINT32 source,UINT16 length)
{
	int count;

	if (length==0x00) length = 0xffff;

	for (count = 0;count<(length>>1);count++)
	{
		if (megadrive_vdp_address>=0x80) return; // abandon

		megadrive_vdp_vsram[(megadrive_vdp_address&0x7e)>>1] = vdp_get_word_from_68k_mem(source);
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
static void handle_dma_bits(void)
{

	if (megadrive_vdp_code&0x20)
	{
		UINT32 source;
		UINT16 length;
		source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8) | ((MEGADRIVE_REG17_DMASOURCE3&0xff)<<16))<<1;
		length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8))<<1;
	//  mame_printf_debug("%06x 68k DMAtran set source %06x length %04x dest %04x enabled %01x code %02x %02x\n", activecpu_get_pc(), source, length, megadrive_vdp_address,MEGADRIVE_REG01_DMA_ENABLE, megadrive_vdp_code,MEGADRIVE_REG0F_AUTO_INC);

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
			if (MEGADRIVE_REG01_DMA_ENABLE) megadrive_do_insta_68k_to_vram_dma(source,length);

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
			if (MEGADRIVE_REG01_DMA_ENABLE) megadrive_do_insta_68k_to_cram_dma(source,length);
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
			if (MEGADRIVE_REG01_DMA_ENABLE) megadrive_do_insta_68k_to_vsram_dma(source,length);
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

static void megadriv_vdp_ctrl_port_w(int data)
{
//  logerror("write to vdp control port %04x\n",data);
	megadrive_vram_fill_pending = 0; // ??

	if (megadrive_vdp_command_pending)
	{
		/* 2nd part of 32-bit command */
		megadrive_vdp_command_pending = 0;
		megadrive_vdp_command_part2 = data;

		update_megadrive_vdp_code_and_address();
		handle_dma_bits();

		//logerror("VDP Write Part 2 setting Code %02x Address %04x\n",megadrive_vdp_code, megadrive_vdp_address);

	}
	else
	{
		if ((data & 0xc000) == 0x8000)
		{	/* Register Setting Command */
			int regnum = (data & 0x3f00) >> 8;
			int value  = (data & 0x00ff);

			if (regnum &0x20) mame_printf_debug("reg error\n");

			megadrive_vdp_set_register(regnum&0x1f,value);
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

static WRITE16_HANDLER( megadriv_vdp_w )
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
			megadriv_vdp_data_port_w(data);
			break;

		case 0x04:
		case 0x06:
			if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) mame_printf_debug("8-bit write VDP control port access, offset %04x data %04x mem_mask %04x\n",offset,data,mem_mask);
			megadriv_vdp_ctrl_port_w(data);
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
			if (ACCESSING_BITS_0_7) SN76496_0_w(machine, 0, data & 0xff);
			//if (ACCESSING_BITS_8_15) SN76496_0_w(machine, 0, (data >>8) & 0xff);
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

static UINT16 megadriv_vdp_data_port_r(void)
{
	UINT16 retdata=0;

	//return mame_rand(Machine);

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
			retdata = mame_rand(Machine);
			break;

		case 0x0003:
			logerror("Attempting to READ from DATA PORT in CRAM WRITE MODE\n");
			retdata = mame_rand(Machine);
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
			retdata = mame_rand(Machine);
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

static READ16_HANDLER( megadriv_vdp_r )
{
	UINT16 retvalue = 0;



	switch (offset<<1)
	{

		case 0x00:
		case 0x02:
			if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) mame_printf_debug("8-bit VDP read data port access, offset %04x mem_mask %04x\n",offset,mem_mask);
			retvalue = megadriv_vdp_data_port_r();
			break;

		case 0x04:
		case 0x06:
		//  if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) mame_printf_debug("8-bit VDP read control port access, offset %04x mem_mask %04x\n",offset,mem_mask);
			retvalue = megadriv_vdp_ctrl_port_r();
		//  retvalue = mame_rand(machine);
		//  mame_printf_debug("%06x: Read Control Port at scanline %d hpos %d (return %04x)\n",activecpu_get_pc(),genesis_scanline_counter, get_hposition(),retvalue);
			break;

		case 0x08:
		case 0x0a:
		case 0x0c:
		case 0x0e:
		//  if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) mame_printf_debug("8-bit VDP read HV counter port access, offset %04x mem_mask %04x\n",offset,mem_mask);
			retvalue = megadriv_read_hv_counters();
		//  retvalue = mame_rand(machine);
		//  mame_printf_debug("%06x: Read HV counters at scanline %d hpos %d (return %04x)\n",activecpu_get_pc(),genesis_scanline_counter, get_hposition(),retvalue);
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

static READ16_HANDLER( megadriv_68k_YM2612_read)
{
	//mame_printf_debug("megadriv_68k_YM2612_read %02x %04x\n",offset,mem_mask);
	if ( (genz80.z80_has_bus==0) && (genz80.z80_is_reset==0) )
	{
		switch (offset)
		{
			case 0:
				if (ACCESSING_BITS_8_15)	 return YM2612_status_port_0_A_r(machine, 0) << 8;
				else 				 return YM2612_status_port_0_A_r(machine, 0);
				break;
			case 1:
				if (ACCESSING_BITS_8_15)	return YM2612_status_port_0_A_r(machine, 0) << 8;
				else 				return YM2612_status_port_0_A_r(machine, 0);
				break;
		}
	}
	else
	{
		logerror("%06x: 68000 attempting to access YM2612 (read) without bus\n", activecpu_get_pc());
		return 0;
	}

	return -1;
}



static WRITE16_HANDLER( megadriv_68k_YM2612_write)
{
	//mame_printf_debug("megadriv_68k_YM2612_write %02x %04x %04x\n",offset,data,mem_mask);
	if ( (genz80.z80_has_bus==0) && (genz80.z80_is_reset==0) )
	{
		switch (offset)
		{
			case 0:
				if (ACCESSING_BITS_8_15)	YM2612_control_port_0_A_w	(machine, 0,	(data >> 8) & 0xff);
				else 				YM2612_data_port_0_A_w		(machine, 0,	(data >> 0) & 0xff);
				break;
			case 1:
				if (ACCESSING_BITS_8_15)	YM2612_control_port_0_B_w	(machine, 0,	(data >> 8) & 0xff);
				else 				YM2612_data_port_0_B_w		(machine, 0,	(data >> 0) & 0xff);
				break;
		}
	}
	else
	{
		logerror("%06x: 68000 attempting to access YM2612 (write) without bus\n", activecpu_get_pc());
	}
}

static READ8_HANDLER( megadriv_z80_YM2612_read )
{
	switch (offset)
	{
		case 0: return YM2612_status_port_0_A_r(machine,0);
		case 1: return YM2612_status_port_0_A_r(machine,0);
		case 2: return YM2612_status_port_0_A_r(machine,0);
		case 3: return YM2612_status_port_0_A_r(machine,0);

	}

	return 0x00;
}

static WRITE8_HANDLER( megadriv_z80_YM2612_write )
{
	//mame_printf_debug("megadriv_z80_YM2612_write %02x %02x\n",offset,data);
	switch (offset)
	{
		case 0: YM2612_control_port_0_A_w(machine, 0, data); break;
		case 1: YM2612_data_port_0_A_w(machine, 0, data); break;
		case 2: YM2612_control_port_0_B_w(machine, 0, data); break;
		case 3: YM2612_data_port_0_B_w(machine, 0, data); break;
	}
}

/* Megadrive / Genesis has 3 I/O ports */
static emu_timer *io_timeout[3];
static int io_stage[3];

static TIMER_CALLBACK( io_timeout0_timer_callback )
{
	io_stage[0] = -1;
}

static TIMER_CALLBACK( io_timeout1_timer_callback )
{
	io_stage[1] = -1;
}

static TIMER_CALLBACK( io_timeout2_timer_callback )
{
	io_stage[2] = -1;
}


static void init_megadri6_io(void)
{
	io_timeout[0] = timer_alloc(io_timeout0_timer_callback, NULL);
	io_stage[0] = -1;

	io_timeout[1] = timer_alloc(io_timeout1_timer_callback, NULL);
	io_stage[1] = -1;

	io_timeout[2] = timer_alloc(io_timeout2_timer_callback, NULL);
	io_stage[2] = -1;
}

/* pointers to our io data read/write functions */
static UINT8 (*megadrive_io_read_data_port_ptr)(int offset);
static void (*megadrive_io_write_data_port_ptr)(int offset, UINT16 data);
INPUT_PORTS_START( megadri6 )
	PORT_START /* Joypad 1 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 A") // a
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 B") // b
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 C") // c
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 START") // start
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 X") // x
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Y") // y
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("P1 Z") // z
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME("P1 MODE") // mode


	PORT_START /* Joypad 1 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A") // a
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B") // b
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 C") // c
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 START") // start
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 X") // x
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Y") // y
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("P2 Z") // z
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(2) PORT_NAME("P2 MODE") // mode

	PORT_START /* 3rd I/O port */

	PORT_START_TAG("RESET") /* Buttons on Genesis Console */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Reset Button") PORT_IMPULSE(1) // reset, resets 68k (and..?)

	PORT_START_TAG("REGION") /* Buttons on Genesis Console */
	/* Region setting for Console */
	PORT_DIPNAME( 0x000f, 0x0000, DEF_STR( Region ) )
	PORT_DIPSETTING(      0x0000, "Use HazeMD Default Choice" )
	PORT_DIPSETTING(      0x0001, "US (NTSC, 60fps)" )
	PORT_DIPSETTING(      0x0002, "JAPAN (NTSC, 60fps)" )
	PORT_DIPSETTING(      0x0003, "EUROPE (PAL, 50fps)" )
INPUT_PORTS_END




INPUT_PORTS_START( ssf2ghw )
	PORT_START /* Joypad 1 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )


	PORT_START /* Joypad 1 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START /* 3rd I/O port */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )

//  PORT_START_TAG("RESET") /* Buttons on Genesis Console */
//  PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Reset Button") PORT_IMPULSE(1) // reset, resets 68k (and..?)

//  PORT_START_TAG("REGION") /* Buttons on Genesis Console */
//  /* Region setting for Console */
//  PORT_DIPNAME( 0x000f, 0x0000, DEF_STR( Region ) )
//  PORT_DIPSETTING(      0x0000, "Use HazeMD Default Choice" )
//  PORT_DIPSETTING(      0x0001, "US (NTSC, 60fps)" )
//  PORT_DIPSETTING(      0x0002, "JAPAN (NTSC, 60fps)" )
//  PORT_DIPSETTING(      0x0003, "EUROPE (PAL, 50fps)" )

	PORT_START_TAG("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	PORT_START_TAG("DSWB")
	PORT_DIPNAME( 0x07, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "0 (Easiest)" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7 (Hardest)" )

	PORT_START_TAG("DSWC")
	PORT_DIPNAME( 0x0f, 0x0b, "Speed" )
	PORT_DIPSETTING(    0x0f, "0 (Slowest)" )
	PORT_DIPSETTING(    0x0e, "1" )
	PORT_DIPSETTING(    0x0d, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x0b, "4" )
	PORT_DIPSETTING(    0x0a, "5" )
	PORT_DIPSETTING(    0x09, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x07, "8" )
	PORT_DIPSETTING(    0x06, "9" )
	PORT_DIPSETTING(    0x05, "10 (Fastest)" )
//  PORT_DIPSETTING(    0x04, "10 (Fastest)" )
//  PORT_DIPSETTING(    0x03, "10 (Fastest)" )
//  PORT_DIPSETTING(    0x02, "10 (Fastest)" )
//  PORT_DIPSETTING(    0x01, "10 (Fastest)" )
//  PORT_DIPSETTING(    0x00, "10 (Fastest)" )
INPUT_PORTS_END





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

INPUT_PORTS_START( megadriv )
	PORT_START /* Joypad 1 (3 button + start) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 A") // a
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 B") // b
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 C") // c
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 START") // start

	PORT_START /* Joypad 2 (3 button + start) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A") // a
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B") // b
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 C") // c
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 START") // start

	PORT_START /* 3rd I/O port */

	PORT_START_TAG("RESET") /* Buttons on Genesis Console */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Reset Button") PORT_IMPULSE(1) // reset, resets 68k (and..?)

	PORT_START_TAG("REGION") /* Region setting for Console */
	/* Region setting for Console */
	PORT_DIPNAME( 0x000f, 0x0000, DEF_STR( Region ) )
	PORT_DIPSETTING(      0x0000, "Use HazeMD Default Choice" )
	PORT_DIPSETTING(      0x0001, "US (NTSC, 60fps)" )
	PORT_DIPSETTING(      0x0002, "JAPAN (NTSC, 60fps)" )
	PORT_DIPSETTING(      0x0003, "EUROPE (PAL, 50fps)" )
INPUT_PORTS_END


INPUT_PORTS_START( aladbl )
	PORT_START /* Joypad 1 (3 button + start) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Throw") // a
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Sword") // b
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Jump") // c
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 ) // start

	PORT_START /* Joypad 2 (3 button + start) NOT READ DIRECTLY - not used */
//  PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
//  PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
//  PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
//  PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
//  PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Throw") // a
//  PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Sword") // b
//  PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Jump") // c
//  PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 ) // start

	PORT_START /* 3rd I/O port */

//  PORT_START_TAG("RESET") /* Buttons on Genesis Console */
//  PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Reset Button") PORT_IMPULSE(1) // reset, resets 68k (and..?)

//  PORT_START_TAG("REGION") /* Region setting for Console */
//  /* Region setting for Console */
//  PORT_DIPNAME( 0x000f, 0x0000, DEF_STR( Region ) )
//  PORT_DIPSETTING(      0x0000, "Use HazeMD Default Choice" )
//  PORT_DIPSETTING(      0x0001, "US (NTSC, 60fps)" )
//  PORT_DIPSETTING(      0x0002, "JAPAN (NTSC, 60fps)" )
//  PORT_DIPSETTING(      0x0003, "EUROPE (PAL, 50fps)" )

    /* As I don't know how it is on real hardware, this is more a guess than anything */
	PORT_START_TAG("MCU")
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )          /* code at 0x1b2a50 - unsure if there are so many settings */
//  PORT_DIPSETTING(    0x00, "INVALID" )                   /* adds 0 credit */
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
    PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ) )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_SPECIAL )         /* to avoid it being changed and corrupting Coinage settings */
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )       /* code at 0x1b2680 */
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )             /* "PRACTICE" */
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )           /* "NORMAL" */
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )             /* "DIFFICULT" */
//  PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPUNUSED( 0x0040, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x0080, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)     /* needed to avoid credits getting mad */
INPUT_PORTS_END


#define MODE_BUTTON(player) ( (input_port_read_indexed(Machine, player) & 0x800) >> 11 )
#define Z_BUTTON(player) ( (input_port_read_indexed(Machine, player) & 0x400) >> 10 )
#define Y_BUTTON(player) ( (input_port_read_indexed(Machine, player) & 0x200) >> 9 )
#define X_BUTTON(player) ( (input_port_read_indexed(Machine, player) & 0x100) >> 8 )


#define START_BUTTON(player) ( (input_port_read_indexed(Machine, player) & 0x80) >> 7 )
#define C_BUTTON(player)     ( (input_port_read_indexed(Machine, player) & 0x40) >> 6 )
#define B_BUTTON(player)     ( (input_port_read_indexed(Machine, player) & 0x20) >> 5 )
#define A_BUTTON(player)     ( (input_port_read_indexed(Machine, player) & 0x10) >> 4 )
#define RIGHT_BUTTON(player) ( (input_port_read_indexed(Machine, player) & 0x08) >> 3 )
#define LEFT_BUTTON(player)  ( (input_port_read_indexed(Machine, player) & 0x04) >> 2 )
#define DOWN_BUTTON(player)  ( (input_port_read_indexed(Machine, player) & 0x02) >> 1 )
#define UP_BUTTON(player)    ( (input_port_read_indexed(Machine, player) & 0x01) >> 0 )
#define MD_RESET_BUTTON      ( (input_port_read_safe(Machine, "RESET",0x00)      & 0x01) >> 0 )

static UINT8 megadrive_io_data_regs[3];
static UINT8 megadrive_io_ctrl_regs[3];
static UINT8 megadrive_io_tx_regs[3];

static void megadrive_init_io(running_machine *machine)
{
	megadrive_io_data_regs[0] = 0x7f;
	megadrive_io_data_regs[1] = 0x7f;
	megadrive_io_data_regs[2] = 0x7f;
	megadrive_io_ctrl_regs[0] = 0x00;
	megadrive_io_ctrl_regs[1] = 0x00;
	megadrive_io_ctrl_regs[2] = 0x00;
	megadrive_io_tx_regs[0] = 0xff;
	megadrive_io_tx_regs[1] = 0xff;
	megadrive_io_tx_regs[2] = 0xff;

	if (machine->gamedrv->ipt==ipt_megadri6)
		init_megadri6_io();

	if (machine->gamedrv->ipt==ipt_ssf2ghw)
		init_megadri6_io();
}

/************* 6 button version **************************/
static UINT8 megadrive_io_read_data_port_6button(int portnum)
{
	UINT8 retdata;


	if (megadrive_io_data_regs[portnum]&0x40)
	{
		if (io_stage[portnum]==2)
		{
			retdata = ( megadrive_io_data_regs[portnum] & 0x80) |
			          (1 <<6) |
			          ((megadrive_io_ctrl_regs[portnum]&0x20)?(megadrive_io_data_regs[portnum]&0x20):(C_BUTTON(portnum)<<5)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x10)?(megadrive_io_data_regs[portnum]&0x10):(B_BUTTON(portnum)<<4)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x08)?(megadrive_io_data_regs[portnum]&0x08):(MODE_BUTTON(portnum)<<3)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x04)?(megadrive_io_data_regs[portnum]&0x04):(X_BUTTON(portnum)<<2)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x02)?(megadrive_io_data_regs[portnum]&0x02):(Y_BUTTON(portnum)<<1)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x01)?(megadrive_io_data_regs[portnum]&0x01):(Z_BUTTON(portnum)<<0));
		}
		else
		{
			retdata = ( megadrive_io_data_regs[portnum] & 0x80) |
			          (1 << 6) |
			          ((megadrive_io_ctrl_regs[portnum]&0x20)?(megadrive_io_data_regs[portnum]&0x20):(C_BUTTON(portnum)<<5)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x10)?(megadrive_io_data_regs[portnum]&0x10):(B_BUTTON(portnum)<<4)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x08)?(megadrive_io_data_regs[portnum]&0x08):(RIGHT_BUTTON(portnum)<<3)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x04)?(megadrive_io_data_regs[portnum]&0x04):(LEFT_BUTTON(portnum)<<2)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x02)?(megadrive_io_data_regs[portnum]&0x02):(DOWN_BUTTON(portnum)<<1)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x01)?(megadrive_io_data_regs[portnum]&0x01):(UP_BUTTON(portnum)<<0));
		}
	}
	else
	{
		if (io_stage[portnum]==1)
		{
			retdata = ( megadrive_io_data_regs[portnum] & 0x80) |
			          ( 0<<6 ) |
			          ((megadrive_io_ctrl_regs[portnum]&0x20)?(megadrive_io_data_regs[portnum]&0x20):(START_BUTTON(portnum)<<5)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x10)?(megadrive_io_data_regs[portnum]&0x10):(A_BUTTON(portnum)<<4)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x08)?(megadrive_io_data_regs[portnum]&0x08):(0<<3)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x04)?(megadrive_io_data_regs[portnum]&0x04):(0<<2)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x02)?(megadrive_io_data_regs[portnum]&0x02):(0<<1)) |
	                  ((megadrive_io_ctrl_regs[portnum]&0x01)?(megadrive_io_data_regs[portnum]&0x01):(0<<0));
		}
		else if (io_stage[portnum]==2)
		{
			retdata = ( megadrive_io_data_regs[portnum] & 0x80) |
			          ( 0<<6 ) |
			          ((megadrive_io_ctrl_regs[portnum]&0x20)?(megadrive_io_data_regs[portnum]&0x20):(START_BUTTON(portnum)<<5)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x10)?(megadrive_io_data_regs[portnum]&0x10):(A_BUTTON(portnum)<<4)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x08)?(megadrive_io_data_regs[portnum]&0x08):(1<<3)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x04)?(megadrive_io_data_regs[portnum]&0x04):(1<<2)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x02)?(megadrive_io_data_regs[portnum]&0x02):(1<<1)) |
	                  ((megadrive_io_ctrl_regs[portnum]&0x01)?(megadrive_io_data_regs[portnum]&0x01):(1<<0));
		}
		else
		{
			retdata = ( megadrive_io_data_regs[portnum] & 0x80) |
			          ( 0<<6 ) |
			          ((megadrive_io_ctrl_regs[portnum]&0x20)?(megadrive_io_data_regs[portnum]&0x20):(START_BUTTON(portnum)<<5)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x10)?(megadrive_io_data_regs[portnum]&0x10):(A_BUTTON(portnum)<<4)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x08)?(megadrive_io_data_regs[portnum]&0x08):(0<<3)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x04)?(megadrive_io_data_regs[portnum]&0x04):(0<<2)) |
			          ((megadrive_io_ctrl_regs[portnum]&0x02)?(megadrive_io_data_regs[portnum]&0x02):(DOWN_BUTTON(portnum)<<1)) |
	                  ((megadrive_io_ctrl_regs[portnum]&0x01)?(megadrive_io_data_regs[portnum]&0x01):(UP_BUTTON(portnum)<<0));
		}
	}

//  mame_printf_debug("read io data port stage %d port %d %02x\n",io_stage[portnum],portnum,retdata);


	return retdata|(retdata<<8);
}
/************* end 6 button version ********************************************/

static UINT8 megadrive_io_read_data_port_3button(int portnum)
{
	UINT8 retdata;


	if (megadrive_io_data_regs[portnum]&0x40)
	{
		retdata = ( megadrive_io_data_regs[portnum] & 0x80) |
		          ((megadrive_io_ctrl_regs[portnum]&0x40)?(megadrive_io_data_regs[portnum]&0x40):(0x40)) |
		          ((megadrive_io_ctrl_regs[portnum]&0x20)?(megadrive_io_data_regs[portnum]&0x20):(C_BUTTON(portnum)<<5)) |
		          ((megadrive_io_ctrl_regs[portnum]&0x10)?(megadrive_io_data_regs[portnum]&0x10):(B_BUTTON(portnum)<<4)) |
		          ((megadrive_io_ctrl_regs[portnum]&0x08)?(megadrive_io_data_regs[portnum]&0x08):(RIGHT_BUTTON(portnum)<<3)) |
		          ((megadrive_io_ctrl_regs[portnum]&0x04)?(megadrive_io_data_regs[portnum]&0x04):(LEFT_BUTTON(portnum)<<2)) |
		          ((megadrive_io_ctrl_regs[portnum]&0x02)?(megadrive_io_data_regs[portnum]&0x02):(DOWN_BUTTON(portnum)<<1)) |
		          ((megadrive_io_ctrl_regs[portnum]&0x01)?(megadrive_io_data_regs[portnum]&0x01):(UP_BUTTON(portnum)<<0));
	}
	else
	{
		retdata = ( megadrive_io_data_regs[portnum] & 0x80) |
		          ((megadrive_io_ctrl_regs[portnum]&0x40)?(megadrive_io_data_regs[portnum]&0x40):(0x40)) |
		          ((megadrive_io_ctrl_regs[portnum]&0x20)?(megadrive_io_data_regs[portnum]&0x20):(START_BUTTON(portnum)<<5)) |
		          ((megadrive_io_ctrl_regs[portnum]&0x10)?(megadrive_io_data_regs[portnum]&0x10):(A_BUTTON(portnum)<<4)) |
		          ((megadrive_io_ctrl_regs[portnum]&0x08)?(megadrive_io_data_regs[portnum]&0x08):(0<<3)) |
		          ((megadrive_io_ctrl_regs[portnum]&0x04)?(megadrive_io_data_regs[portnum]&0x04):(0<<2)) |
		          ((megadrive_io_ctrl_regs[portnum]&0x02)?(megadrive_io_data_regs[portnum]&0x02):(DOWN_BUTTON(portnum)<<1)) |
		          ((megadrive_io_ctrl_regs[portnum]&0x01)?(megadrive_io_data_regs[portnum]&0x01):(UP_BUTTON(portnum)<<0));
	}

	return retdata;
}

/* used by megatech bios, the test mode accesses the joypad/stick inputs like this */
UINT8 megatech_bios_port_cc_dc_r(int offset, int ctrl)
{
	UINT8 retdata;

	if (ctrl==0x55)
	{
			retdata = (1<<0) |
					  (1<<1) |
					  (A_BUTTON(1)<<2) |
					  (1<<3) |
					  (A_BUTTON(0)<<4) |
					  (1<<5) |
					  (1<<6) |
					  (1<<7);
	}
	else
	{
		if (offset==0)
		{
			retdata = (UP_BUTTON(0)<<0) |
					  (DOWN_BUTTON(0)<<1) |
					  (LEFT_BUTTON(0)<<2) |
					  (RIGHT_BUTTON(0)<<3) |
					  (B_BUTTON(0)<<4) |
					  (C_BUTTON(0)<<5) |
					  (UP_BUTTON(1)<<6) |
					  (DOWN_BUTTON(1)<<7);
		}
		else
		{
			retdata = (LEFT_BUTTON(1)<<0) |
					  (RIGHT_BUTTON(1)<<1) |
					  (B_BUTTON(1)<<2) |
					  (C_BUTTON(1)<<3) |
					  (1<<4) |
					  (1<<5) |
					  (1<<6) |
					  (1<<7);
		}

	}

	return retdata;
}

/* the SMS inputs should be more complex, like the megadrive ones */
READ8_HANDLER (megatech_sms_ioport_dc_r)
{
	return (DOWN_BUTTON(1)  << 7) |
		   (UP_BUTTON(1)    << 6) |
		   (B_BUTTON(0)     << 5) | // TR-A
		   (A_BUTTON(0)     << 4) | // TL-A
		   (RIGHT_BUTTON(0) << 3) |
		   (LEFT_BUTTON(0)  << 2) |
		   (DOWN_BUTTON(0)  << 1) |
		   (UP_BUTTON(0)    << 0);
}

READ8_HANDLER (megatech_sms_ioport_dd_r)
{
	return (0               << 7) | // TH-B
		   (0               << 6) | // TH-A
		   (0               << 5) | // unused
		   (1               << 4) | // RESET button
		   (B_BUTTON(1)     << 3) | // TR-B
		   (A_BUTTON(1)     << 2) | // TL-B
		   (RIGHT_BUTTON(1) << 1) |
		   (LEFT_BUTTON(1)  << 0);
}

static UINT8 megadrive_io_read_ctrl_port(int portnum)
{
	UINT8 retdata;
	retdata = megadrive_io_ctrl_regs[portnum];
	//mame_printf_debug("read io ctrl port %d %02x\n",portnum,retdata);

	return retdata|(retdata<<8);
}

static UINT8 megadrive_io_read_tx_port(int portnum)
{
	UINT8 retdata;
	retdata = megadrive_io_tx_regs[portnum];
	return retdata|(retdata<<8);
}

static UINT8 megadrive_io_read_rx_port(int portnum)
{
	return 0x00;
}

static UINT8 megadrive_io_read_sctrl_port(int portnum)
{
	return 0x00;
}


static READ16_HANDLER( megadriv_68k_io_read )
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

	//return (mame_rand(machine)&0x0f0f)|0xf0f0;//0x0000;
	switch (offset)
	{
		case 0:
			logerror("%06x read version register\n", activecpu_get_pc());
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
			retdata = megadrive_io_read_data_port_ptr(offset-1);
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

	return retdata | (retdata<<8);
}


static void megadrive_io_write_data_port_3button(int portnum, UINT16 data)
{
	megadrive_io_data_regs[portnum] = data;
	//mame_printf_debug("Writing IO Data Register #%d data %04x\n",portnum,data);

}


/******************************6 button version*****************************/

static void megadrive_io_write_data_port_6button(int portnum, UINT16 data)
{
	if (megadrive_io_ctrl_regs[portnum]&0x40)
	{
		if (((megadrive_io_data_regs[portnum]&0x40)==0x00) && ((data&0x40) == 0x40))
		{
			io_stage[portnum]++;
			timer_adjust_oneshot(io_timeout[portnum], ATTOTIME_IN_CYCLES(8192,0), 0);
		}

	}

	megadrive_io_data_regs[portnum] = data;
	//mame_printf_debug("Writing IO Data Register #%d data %04x\n",portnum,data);

}

/***************************end 6 button version ****************************/

static void megadrive_io_write_ctrl_port(int portnum, UINT16 data)
{
	megadrive_io_ctrl_regs[portnum] = data;
//  mame_printf_debug("Setting IO Control Register #%d data %04x\n",portnum,data);
}

static void megadrive_io_write_tx_port(int portnum, UINT16 data)
{
	megadrive_io_tx_regs[portnum] = data;
}

static void megadrive_io_write_rx_port(int portnum, UINT16 data)
{

}

static void megadrive_io_write_sctrl_port(int portnum, UINT16 data)
{

}



static WRITE16_HANDLER( megadriv_68k_io_write )
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
			megadrive_io_write_data_port_ptr(offset-1,data);
			break;

		case 0x4:
		case 0x5:
		case 0x6:
			megadrive_io_write_ctrl_port(offset-4,data);
			break;

		/* Serial I/O Registers */

		case 0x7: megadrive_io_write_tx_port(0,data); break;
		case 0x8: megadrive_io_write_rx_port(0,data); break;
		case 0x9: megadrive_io_write_sctrl_port(0,data); break;


		case 0xa: megadrive_io_write_tx_port(1,data); break;
		case 0xb: megadrive_io_write_rx_port(1,data); break;
		case 0xc: megadrive_io_write_sctrl_port(1,data); break;
			break;

		case 0xd: megadrive_io_write_tx_port(2,data); break;
		case 0xe: megadrive_io_write_rx_port(2,data); break;
		case 0xf: megadrive_io_write_sctrl_port(2,data); break;
			break;
	}

}



static ADDRESS_MAP_START( megadriv_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000 , 0x3fffff) AM_READ(SMH_ROM)
	/*      (0x000000 - 0x3fffff) == GAME ROM (4Meg Max, Some games have special banking too) */

	AM_RANGE(0xa00000 , 0xa01fff) AM_READ(megadriv_68k_read_z80_ram)
	AM_RANGE(0xa04000 , 0xa04003) AM_READ(megadriv_68k_YM2612_read)

	AM_RANGE(0xa10000 , 0xa1001f) AM_READ(megadriv_68k_io_read)

	AM_RANGE(0xa11100 , 0xa11101) AM_READ(megadriv_68k_check_z80_bus)

	AM_RANGE(0xc00000 , 0xc0001f) AM_READ(megadriv_vdp_r)
	AM_RANGE(0xd00000 , 0xd0001f) AM_READ(megadriv_vdp_r) // the earth defend

	/* these are fake - remove allocs in VIDEO_START to use these to view ram instead */
//  AM_RANGE(0xb00000 , 0xb0ffff) AM_READ(SMH_RAM) AM_BASE(&megadrive_vdp_vram)
//  AM_RANGE(0xb10000 , 0xb1007f) AM_READ(SMH_RAM) AM_BASE(&megadrive_vdp_vsram)
//  AM_RANGE(0xb10100 , 0xb1017f) AM_READ(SMH_RAM) AM_BASE(&megadrive_vdp_cram)



	AM_RANGE(0xe00000 , 0xe0ffff) AM_READ(SMH_RAM) AM_MIRROR(0x1f0000)
//  AM_RANGE(0xff0000 , 0xffffff) AM_READ(SMH_RAM)
	/*       0xe00000 - 0xffffff) == MAIN RAM (64kb, Mirrored, most games use ff0000 - ffffff) */

ADDRESS_MAP_END

static ADDRESS_MAP_START( megadriv_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000 , 0x3fffff) AM_WRITE(SMH_ROM)

	AM_RANGE(0xa00000 , 0xa01fff) AM_WRITE(megadriv_68k_write_z80_ram)
	AM_RANGE(0xa02000 , 0xa03fff) AM_WRITE(megadriv_68k_write_z80_ram)
	AM_RANGE(0xa04000 , 0xa04003) AM_WRITE(megadriv_68k_YM2612_write)

	AM_RANGE(0xa06000 , 0xa06001) AM_WRITE(megadriv_68k_z80_bank_write)

	AM_RANGE(0xa10000 , 0xa1001f) AM_WRITE(megadriv_68k_io_write)

	AM_RANGE(0xa11100 , 0xa11101) AM_WRITE(megadriv_68k_req_z80_bus)
	AM_RANGE(0xa11200 , 0xa11201) AM_WRITE(megadriv_68k_req_z80_reset)

	AM_RANGE(0xc00000 , 0xc0001f) AM_WRITE(megadriv_vdp_w)
	AM_RANGE(0xd00000 , 0xd0001f) AM_WRITE(megadriv_vdp_w) // the earth defend

	AM_RANGE(0xe00000 , 0xe0ffff) AM_WRITE(SMH_RAM) AM_MIRROR(0x1f0000) AM_BASE(&megadrive_ram)
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
		logerror("%06x: 68000 attempting to access Z80 (read) address space without bus\n", activecpu_get_pc());
		return mame_rand(machine);
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
		logerror("%06x: 68000 attempting to access Z80 (write) address space without bus\n", activecpu_get_pc());
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
	UINT16 nextvalue = mame_rand(machine);//read_next_instruction()&0xff00;


	/* Check if the 68k has the z80 bus */
	if (!ACCESSING_BITS_0_7) // byte (MSB) access
	{
		if (genz80.z80_has_bus || genz80.z80_is_reset) retvalue = nextvalue | 0x0100;
		else retvalue = (nextvalue & 0xfeff);

		//logerror("%06x: 68000 check z80 Bus (byte MSB access) returning %04x mask %04x\n", activecpu_get_pc(),retvalue, mem_mask);
		return retvalue;

	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		//logerror("%06x: 68000 check z80 Bus (byte LSB access) %04x\n", activecpu_get_pc(),mem_mask);
		if (genz80.z80_has_bus || genz80.z80_is_reset) retvalue = 0x0001;
		else retvalue = 0x0000;

		return retvalue;
	}
	else
	{
		//logerror("%06x: 68000 check z80 Bus (word access) %04x\n", activecpu_get_pc(),mem_mask);
		if (genz80.z80_has_bus || genz80.z80_is_reset) retvalue = nextvalue | 0x0100;
		else retvalue = (nextvalue & 0xfeff);

	//  mame_printf_debug("%06x: 68000 check z80 Bus (word access) %04x %04x\n", activecpu_get_pc(),mem_mask, retvalue);
		return retvalue;
	}
}


static WRITE16_HANDLER( megadriv_68k_req_z80_bus )
{
	/* Request the Z80 bus, allows 68k to read/write Z80 address space */
	if (!ACCESSING_BITS_0_7) // byte access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 request z80 Bus (byte MSB access) %04x %04x\n", activecpu_get_pc(),data,mem_mask);
			genz80.z80_has_bus=0;
			cpunum_set_input_line(machine, genz80.z80_cpunum, INPUT_LINE_HALT, ASSERT_LINE);
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte MSB access) %04x %04x\n", activecpu_get_pc(),data,mem_mask);
			genz80.z80_has_bus=1;
			cpunum_set_input_line(machine, genz80.z80_cpunum, INPUT_LINE_HALT, CLEAR_LINE);

		}
	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		if (data & 0x0001)
		{
			//logerror("%06x: 68000 request z80 Bus (byte LSB access) %04x %04x\n", activecpu_get_pc(),data,mem_mask);
			genz80.z80_has_bus=0;
			cpunum_set_input_line(machine, genz80.z80_cpunum, INPUT_LINE_HALT, ASSERT_LINE);
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte LSB access) %04x %04x\n", activecpu_get_pc(),data,mem_mask);
			genz80.z80_has_bus=1;
			cpunum_set_input_line(machine, genz80.z80_cpunum, INPUT_LINE_HALT, CLEAR_LINE);
		}
	}
	else // word access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 request z80 Bus (word access) %04x %04x\n", activecpu_get_pc(),data,mem_mask);
			genz80.z80_has_bus=0;
			cpunum_set_input_line(machine, genz80.z80_cpunum, INPUT_LINE_HALT, ASSERT_LINE);
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte LSB access) %04x %04x\n", activecpu_get_pc(),data,mem_mask);
			genz80.z80_has_bus=1;
			cpunum_set_input_line(machine, genz80.z80_cpunum, INPUT_LINE_HALT, CLEAR_LINE);
		}
	}
}

static WRITE16_HANDLER ( megadriv_68k_req_z80_reset )
{
	if (!ACCESSING_BITS_0_7) // byte access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 clear z80 reset (byte MSB access) %04x %04x\n", activecpu_get_pc(),data,mem_mask);
			genz80.z80_is_reset=0;
			cpunum_set_input_line(machine, genz80.z80_cpunum, INPUT_LINE_RESET, CLEAR_LINE);
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte MSB access) %04x %04x\n", activecpu_get_pc(),data,mem_mask);
			genz80.z80_is_reset=1;
			cpunum_set_input_line(machine, genz80.z80_cpunum, INPUT_LINE_RESET, ASSERT_LINE);
			sndti_reset(SOUND_YM2612, 0);
		}
	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		if (data & 0x0001)
		{
			//logerror("%06x: 68000 clear z80 reset (byte LSB access) %04x %04x\n", activecpu_get_pc(),data,mem_mask);
			genz80.z80_is_reset=0;
			cpunum_set_input_line(machine, genz80.z80_cpunum, INPUT_LINE_RESET, CLEAR_LINE);
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte LSB access) %04x %04x\n", activecpu_get_pc(),data,mem_mask);
			genz80.z80_is_reset=1;
			cpunum_set_input_line(machine, genz80.z80_cpunum, INPUT_LINE_RESET, ASSERT_LINE);
			sndti_reset(SOUND_YM2612, 0);

		}
	}
	else // word access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 clear z80 reset (word access) %04x %04x\n", activecpu_get_pc(),data,mem_mask);
			genz80.z80_is_reset=0;
			cpunum_set_input_line(machine, genz80.z80_cpunum, INPUT_LINE_RESET, CLEAR_LINE );
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte LSB access) %04x %04x\n", activecpu_get_pc(),data,mem_mask);
			genz80.z80_is_reset=1;
			cpunum_set_input_line(machine, genz80.z80_cpunum, INPUT_LINE_RESET, ASSERT_LINE);
			sndti_reset(SOUND_YM2612, 0);
		}
	}
}

static READ8_HANDLER( z80_read_68k_banked_data )
{
	// genz80.z80_bank_addr contains the address to read

	if ((genz80.z80_bank_addr >= 0x000000) && (genz80.z80_bank_addr <= 0x3fffff)) // ROM Addresses
	{
		UINT32 fulladdress;
		fulladdress = genz80.z80_bank_addr + offset;

		return memory_region(REGION_CPU1)[fulladdress^1]; // ^1? better..


	}
	else
	{
		//mame_printf_debug("unhandled z80 bank read, gen.z80_bank_addr %08x\n",genz80.z80_bank_addr);
		return 0x0000;
	}

}


static WRITE8_HANDLER( megadriv_z80_vdp_write )
{
	switch (offset)
	{
		case 0x11:
		case 0x13:
		case 0x15:
		case 0x17:
			SN76496_0_w(machine, 0, data);
			break;

		default:
			mame_printf_debug("unhandled z80 vdp write %02x %02x\n",offset,data);
	}

}

static WRITE8_HANDLER( z80_write_68k_banked_data )
{
	UINT32 fulladdress;
	fulladdress = genz80.z80_bank_addr + offset;


//  mame_printf_debug("z80_write_68k_banked_data %04x %02x\n",offset,data);
	if ((fulladdress >= 0x000000) && (fulladdress <= 0x3fffff)) // ROM Addresses
	{

		//mame_printf_debug("z80 write to 68k rom??\n");
	}
	else if ((fulladdress >= 0xe00000) && (fulladdress <= 0xffffff)) // ROM Addresses
	{
		fulladdress &=0xffff;
		/* Cheese Cat-Astrophe Starring Speedy Gonzales (E) (M4) [!] has no sound without this */
		if (fulladdress&1) megadrive_ram[fulladdress>>1] = (megadrive_ram[fulladdress>>1]&0xff00) | (data);
		else  megadrive_ram[fulladdress>>1] = (megadrive_ram[fulladdress>>1]&0x00ff) | (data <<8);
	}
	else if (fulladdress == 0xc00011)
	{
		/* quite a few early games write here, most of the later ones don't */
		SN76496_0_w(machine, 0, data);
	}
	else
	{

		//mame_printf_debug("z80 write to 68k address %06x\n",fulladdress);
	}

}

static READ8_HANDLER( megadriv_z80_vdp_read )
{
	mame_printf_debug("megadriv_z80_vdp_read %02x\n",offset);
	return mame_rand(machine);
}

static READ8_HANDLER( megadriv_z80_unmapped_read )
{
	return 0xff;
}

static ADDRESS_MAP_START( z80_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0000 , 0xff) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80_readmem, ADDRESS_SPACE_PROGRAM, 8 )

	AM_RANGE(0x0000 , 0x1fff) AM_READ(SMH_BANK1) AM_MIRROR(0x2000) // RAM can be accessed by the 68k
	AM_RANGE(0x4000 , 0x4003) AM_READ(megadriv_z80_YM2612_read)

	AM_RANGE(0x6100 , 0x7eff) AM_READ(megadriv_z80_unmapped_read)

	AM_RANGE(0x7f00 , 0x7fff) AM_READ(megadriv_z80_vdp_read)

	AM_RANGE(0x8000 , 0xffff) AM_READ(z80_read_68k_banked_data) // The Z80 can read the 68k address space this way
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000 , 0x1fff) AM_WRITE(SMH_BANK1) AM_MIRROR(0x2000)
	AM_RANGE(0x4000 , 0x4003) AM_WRITE(megadriv_z80_YM2612_write)

	AM_RANGE(0x7f00 , 0x7fff) AM_WRITE(megadriv_z80_vdp_write)

	AM_RANGE(0x6000 , 0x6000) AM_WRITE(megadriv_z80_z80_bank_w)
	AM_RANGE(0x6001 , 0x6001) AM_WRITE(megadriv_z80_z80_bank_w) // wacky races uses this address

	AM_RANGE(0x8000 , 0xffff) AM_WRITE(z80_write_68k_banked_data)
ADDRESS_MAP_END

/****************************************** 32X related ******************************************/

static READ16_HANDLER( _32x_reg_r )
{
	return mame_rand(machine);
}

static UINT16 _32x_68k_comms[0x8];
static UINT16 _32x_palette[0x400/2];

static READ16_HANDLER( _32x_68k_comms_r )
{
	return _32x_68k_comms[offset];
}

static WRITE16_HANDLER(_32x_68k_comms_w)
{
	_32x_68k_comms[offset] = data;
}

static READ16_HANDLER( _32x_68k_palette_r )
{
	return _32x_palette[offset];
}

static WRITE16_HANDLER( _32x_68k_palette_w )
{
	_32x_palette[offset] = data;
}

static ADDRESS_MAP_START( _32x_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000 , 0x3fffff) AM_READ(SMH_ROM)

	AM_RANGE(0x880000 , 0x8fffff) AM_READ(SMH_BANK2)
	AM_RANGE(0x900000 , 0x9fffff) AM_READ(SMH_BANK3)

	AM_RANGE(0xa00000 , 0xa01fff) AM_READ(megadriv_68k_read_z80_ram)
	AM_RANGE(0xa04000 , 0xa04003) AM_READ(megadriv_68k_YM2612_read)
	AM_RANGE(0xa10000 , 0xa1001f) AM_READ(megadriv_68k_io_read)
	AM_RANGE(0xa11100 , 0xa11101) AM_READ(megadriv_68k_check_z80_bus)


	AM_RANGE(0xa15120 , 0xa1512f) AM_READ(_32x_68k_comms_r )
	AM_RANGE(0xa1518a , 0xa1518b) AM_READ(_32x_reg_r)
	AM_RANGE(0xa15200 , 0xa153ff) AM_READ(_32x_68k_palette_r )

	AM_RANGE(0xc00000 , 0xc0001f) AM_READ(megadriv_vdp_r)
	AM_RANGE(0xd00000 , 0xd0001f) AM_READ(megadriv_vdp_r) // the earth defend
	AM_RANGE(0xe00000 , 0xe0ffff) AM_READ(SMH_RAM) AM_MIRROR(0x1f0000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( _32x_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000 , 0x3fffff) AM_WRITE(SMH_ROM)

	AM_RANGE(0x880000, 0x8fffff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x900000, 0x9fffff) AM_WRITE(SMH_ROM)


	AM_RANGE(0xa00000 , 0xa01fff) AM_WRITE(megadriv_68k_write_z80_ram)
	AM_RANGE(0xa02000 , 0xa03fff) AM_WRITE(megadriv_68k_write_z80_ram)
	AM_RANGE(0xa04000 , 0xa04003) AM_WRITE(megadriv_68k_YM2612_write)
	AM_RANGE(0xa06000 , 0xa06001) AM_WRITE(megadriv_68k_z80_bank_write)
	AM_RANGE(0xa10000 , 0xa1001f) AM_WRITE(megadriv_68k_io_write)
	AM_RANGE(0xa11100 , 0xa11101) AM_WRITE(megadriv_68k_req_z80_bus)
	AM_RANGE(0xa11200 , 0xa11201) AM_WRITE(megadriv_68k_req_z80_reset)

	AM_RANGE(0xa15120 , 0xa1512f) AM_WRITE(_32x_68k_comms_w )
	AM_RANGE(0xa15200 , 0xa153ff) AM_WRITE(_32x_68k_palette_w )


	AM_RANGE(0xc00000 , 0xc0001f) AM_WRITE(megadriv_vdp_w)
	AM_RANGE(0xd00000 , 0xd0001f) AM_WRITE(megadriv_vdp_w)
	AM_RANGE(0xe00000 , 0xe0ffff) AM_WRITE(SMH_RAM) AM_MIRROR(0x1f0000) AM_BASE(&megadrive_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sh2main_readmem, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x0000000 , 0x0003fff) AM_READ(SMH_ROM)
	AM_RANGE(0x2000000 , 0x23fffff) AM_READ(SMH_BANK4)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sh2main_writemem, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x0000000 , 0x0003fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x2000000 , 0x23fffff) AM_WRITE(SMH_ROM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sh2slave_readmem, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x0000000 , 0x0003fff) AM_READ(SMH_ROM)
	AM_RANGE(0x2000000 , 0x23fffff) AM_READ(SMH_BANK4)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sh2slave_writemem, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x0000000 , 0x0003fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x2000000 , 0x23fffff) AM_WRITE(SMH_ROM)
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

static UINT32 pm_io(int reg, int write, UINT32 d)
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

	if (reg == 4 || (activecpu_get_reg(SSP_ST) & 0x60))
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
				UINT16 *ROM = (UINT16 *) memory_region(REGION_CPU1);
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
	UINT32 d = pm_io(0, 0, 0);
	if (d != (UINT32)-1) return d;
	d = svp.XST2;
	svp.XST2 &= ~2; // ?
	return d;
}

static WRITE16_HANDLER( write_PM0 )
{
	UINT32 r = pm_io(0, 1, data);
	if (r != (UINT32)-1) return;
	svp.XST2 = data; // ?
}

static READ16_HANDLER( read_PM1 )
{
	UINT32 r = pm_io(1, 0, 0);
	if (r != (UINT32)-1) return r;
	logerror("svp: PM1 acces in non PM mode?\n");
	return 0;
}

static WRITE16_HANDLER( write_PM1 )
{
	UINT32 r = pm_io(1, 1, data);
	if (r != (UINT32)-1) return;
	logerror("svp: PM1 acces in non PM mode?\n");
}

static READ16_HANDLER( read_PM2 )
{
	UINT32 r = pm_io(2, 0, 0);
	if (r != (UINT32)-1) return r;
	logerror("svp: PM2 acces in non PM mode?\n");
	return 0;
}

static WRITE16_HANDLER( write_PM2 )
{
	UINT32 r = pm_io(2, 1, data);
	if (r != (UINT32)-1) return;
	logerror("svp: PM2 acces in non PM mode?\n");
}

static READ16_HANDLER( read_XST )
{
	UINT32 d = pm_io(3, 0, 0);
	if (d != (UINT32)-1) return d;

	return svp.XST;
}

static WRITE16_HANDLER( write_XST )
{
	UINT32 r = pm_io(3, 1, data);
	if (r != (UINT32)-1) return;

	svp.XST2 |= 1;
	svp.XST = data;
}

static READ16_HANDLER( read_PM4 )
{
	return pm_io(4, 0, 0);
}

static WRITE16_HANDLER( write_PM4 )
{
	pm_io(4, 1, data);
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
	AM_RANGE(0x0000 , 0x03ff) AM_READ(SMH_BANK3)
	AM_RANGE(0x0400 , 0xffff) AM_READ(SMH_BANK4)
ADDRESS_MAP_END

static ADDRESS_MAP_START( svp_ext_map, ADDRESS_SPACE_IO, 16 )
	ADDRESS_MAP_GLOBAL_MASK(0xf)
	AM_RANGE(0*2 , 0*2+1) AM_READWRITE(read_PM0, write_PM0)
	AM_RANGE(1*2 , 1*2+1) AM_READWRITE(read_PM1, write_PM1)
	AM_RANGE(2*2 , 2*2+1) AM_READWRITE(read_PM2, write_PM2)
	AM_RANGE(3*2 , 3*2+1) AM_READWRITE(read_XST, write_XST)
	AM_RANGE(4*2 , 4*2+1) AM_READWRITE(read_PM4, write_PM4)
	AM_RANGE(6*2 , 6*2+1) AM_READWRITE(read_PMC, write_PMC)
	AM_RANGE(7*2 , 7*2+1) AM_READWRITE(read_AL, write_AL)
ADDRESS_MAP_END


/* DMA read function for SVP */
static UINT16 vdp_get_word_from_68k_mem_svp(UINT32 source)
{
	if ((source & 0xe00000) == 0x000000)
	{
		UINT16 *rom = (UINT16*)memory_region(REGION_CPU1);
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
		return mame_rand(Machine);
	}
}

/* emulate testmode plug */
static UINT8 megadrive_io_read_data_port_svp(int portnum)
{
	if (portnum == 0 && input_port_read_safe(Machine, "MEMORY_TEST", 0x00))
	{
		return (megadrive_io_data_regs[0] & 0xc0);
	}
	return megadrive_io_read_data_port_3button(portnum);
}

static void svp_init(running_machine *machine)
{
	UINT8 *ROM;

	memset(&svp, 0, sizeof(svp));

	/* SVP stuff */
	svp.dram = auto_malloc(0x20000);
	memory_install_readwrite16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x300000, 0x31ffff, 0, 0, SMH_BANK2, SMH_BANK2);
	memory_set_bankptr( 2, svp.dram );
	memory_install_readwrite16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xa15000, 0xa150ff, 0, 0, svp_68k_io_r, svp_68k_io_w);
	// "cell arrange" 1 and 2
	memory_install_read16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x390000, 0x39ffff, 0, 0, svp_68k_cell1_r);
	memory_install_read16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x3a0000, 0x3affff, 0, 0, svp_68k_cell2_r);

	svp.iram = auto_malloc(0x800);
	memory_set_bankptr( 3, svp.iram );
	/* SVP ROM just shares m68k region.. */
	ROM = memory_region(REGION_CPU1);
	memory_set_bankptr( 4, ROM + 0x800 );

	vdp_get_word_from_68k_mem = vdp_get_word_from_68k_mem_svp;
	megadrive_io_read_data_port_ptr	= megadrive_io_read_data_port_svp;
}


INPUT_PORTS_START( megdsvp )
	PORT_INCLUDE( megadriv )

	PORT_START_TAG("MEMORY_TEST") /* special memtest mode */
	/* Region setting for Console */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Test ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x01, DEF_STR( On ) )
INPUT_PORTS_END

MACHINE_DRIVER_START( megdsvp )
	MDRV_IMPORT_FROM(megadriv)

	MDRV_CPU_ADD(SSP1601, MASTER_CLOCK / 7 * 3) /* ~23 MHz (guessed) */
	MDRV_CPU_PROGRAM_MAP(svp_ssp_map, 0)
	MDRV_CPU_IO_MAP(svp_ext_map, 0)
	/* IRQs are not used by this CPU */
MACHINE_DRIVER_END


/****************************************** END SVP related *************************************/


static attotime time_elapsed_since_crap;


VIDEO_START(megadriv)
{
	int x;

	render_bitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);

	megadrive_vdp_vram  = auto_malloc(0x10000);
	megadrive_vdp_cram  = auto_malloc(0x80);
	megadrive_vdp_vsram = auto_malloc(0x80);
	megadrive_vdp_internal_sprite_attribute_table = auto_malloc(0x400);

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

	sprite_renderline = auto_malloc(1024);
	highpri_renderline = auto_malloc(320);
	video_renderline = auto_malloc(320*2);

	oldscreenwidth = -1; // 40 cell mode

	megadrive_vdp_palette_lookup = auto_malloc(0x40*2);
	megadrive_vdp_palette_lookup_shadow = auto_malloc(0x40*2);
	megadrive_vdp_palette_lookup_highlight = auto_malloc(0x40*2);

	memset(megadrive_vdp_palette_lookup,0x00,0x40*2);
	memset(megadrive_vdp_palette_lookup_shadow,0x00,0x40*2);
	memset(megadrive_vdp_palette_lookup_highlight,0x00,0x40*2);
}

VIDEO_UPDATE(megadriv)
{
	/* Copy our screen buffer here */
	copybitmap(bitmap, render_bitmap, 0, 0, 0, 0, cliprect);

//  int xxx;
	/* reference */

//  time_elapsed_since_crap = timer_timeelapsed(frame_timer);
//  xxx = ATTOTIME_TO_CYCLES(0,time_elapsed_since_crap);
//  mame_printf_debug("update cycles %d, %08x %08x\n",xxx, (UINT32)(time_elapsed_since_crap.attoseconds>>32),(UINT32)(time_elapsed_since_crap.attoseconds&0xffffffff));

	return 0;
}



static TIMER_CALLBACK( frame_timer_callback )
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
		int drawwidth,drawheight;
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

				drawwidth = (width+1)*8;
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
		video_renderline[x]=MEGADRIVE_REG07_BGCOLOUR;
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
				if (sprite_renderline[x+128] & 0x40) video_renderline[x] = sprite_renderline[x+128]&0x3f;
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
				if (sprite_renderline[x+128] & 0x80) video_renderline[x] = sprite_renderline[x+128]&0x3f;
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
						video_renderline[x] = video_renderline[x]|=0x2000;
					}
					else
					{
						video_renderline[x] = spritedata | 0x4000;
					}
				}
			}

		}
}

/* This converts our render buffer to real screen colours */
static void genesis_render_videobuffer_to_screenbuffer(running_machine *machine, int scanline)
{
	UINT16*lineptr;
	int x;
	lineptr = BITMAP_ADDR16(render_bitmap, scanline, 0);

	if (!MEGADRIVE_REG0C_SHADOW_HIGLIGHT)
	{

		for (x=0;x<320;x++)
		{
			UINT16 dat;
			dat = video_renderline[x];
			lineptr[x] = megadrive_vdp_palette_lookup[dat&0x3f];
		}
	}
	else
	{
		for (x=0;x<320;x++)
		{
			UINT16 dat;
			dat = video_renderline[x];

			/* Verify my handling.. I'm not sure all cases are correct */

			switch (dat&0xe000)
			{
				case 0x0000: // low priority, no shadow sprite, no highlight = shadow
				case 0x2000: // low priority, shadow sprite, no highlight = shadow
				case 0x6000: // normal pri,   shadow sprite, no highlight = shadow?
					lineptr[x] = megadrive_vdp_palette_lookup_shadow[dat&0x3f];
					break;

				case 0x4000: // normal pri, no shadow sprite, no highlight = normal;
				case 0x8000: // low pri, highlight sprite = normal;
					lineptr[x] = megadrive_vdp_palette_lookup[dat&0x3f];
					break;

				case 0xc000: // normal pri, highlight set = highlight?
					lineptr[x] = megadrive_vdp_palette_lookup_highlight[dat&0x3f];
					break;

				case 0xa000: // shadow set, highlight set - not possible
				case 0xe000: // shadow set, highlight set, normal set, not possible
					lineptr[x] = megadrive_vdp_palette_lookup_highlight[mame_rand(Machine)&0x3f];
					break;


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

	time_elapsed_since_scanline_timer = timer_timeelapsed(scanline_timer);

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

static int irq4counter = -1;

static emu_timer* render_timer;

static TIMER_CALLBACK( render_timer_callback )
{
	if (genesis_scanline_counter>=0 && genesis_scanline_counter<megadrive_visible_scanlines)
	{
		genesis_render_scanline(machine, genesis_scanline_counter);
	}
}


static TIMER_CALLBACK( scanline_timer_callback )
{
	/* This function is called at the very start of every scanline starting at the very
       top-left of the screen.  The first scanline is scanline 0 (we set scanline to -1 in
       VIDEO_EOF) */

	timer_call_after_resynch(NULL, 0, 0);
	/* Compensate for some rounding errors

       When the counter reaches 261 we should have reached the end of the frame, however due
       to rounding errors in the timer calculation we're not quite there.  Let's assume we are
       still in the previous scanline for now.
    */

	if (genesis_scanline_counter!=(megadrive_total_scanlines-1))
	{
		genesis_scanline_counter++;
//      mame_printf_debug("scanline %d\n",genesis_scanline_counter);
		timer_adjust_oneshot(scanline_timer, attotime_div(ATTOTIME_IN_HZ(megadriv_framerate), megadrive_total_scanlines), 0);
		timer_adjust_oneshot(render_timer, ATTOTIME_IN_USEC(1), 0);

		if (genesis_scanline_counter==megadrive_irq6_scanline )
		{
		//  mame_printf_debug("x %d",genesis_scanline_counter);
			timer_adjust_oneshot(irq6_on_timer,  ATTOTIME_IN_USEC(6), 0);
			megadrive_irq6_pending = 1;
			megadrive_vblank_flag = 1;
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
					timer_adjust_oneshot(irq4_on_timer,  ATTOTIME_IN_USEC(1), 0);
					//mame_printf_debug("irq4 on scanline %d reload %d\n",genesis_scanline_counter,MEGADRIVE_REG0A_HINT_VALUE);
				}
			}
		}
		else
		{
			if (megadrive_imode==3) irq4counter = MEGADRIVE_REG0A_HINT_VALUE*2;
			else irq4counter=MEGADRIVE_REG0A_HINT_VALUE;
		}

		//if (genesis_scanline_counter==0) timer_adjust_oneshot(irq4_on_timer,  ATTOTIME_IN_USEC(2), 0);





		if (genesis_scanline_counter==megadrive_z80irq_scanline)
		{
			if ((genz80.z80_has_bus==1) && (genz80.z80_is_reset==0)) cpunum_set_input_line(machine, 1,0,HOLD_LINE);
		}
		if (genesis_scanline_counter==megadrive_z80irq_scanline+1)
		{
			cpunum_set_input_line(machine, 1,0,CLEAR_LINE);
		}


	}
	else /* pretend we're still on the same scanline to compensate for rounding errors */
	{
		genesis_scanline_counter = megadrive_total_scanlines-1;
	}

}

static TIMER_CALLBACK( irq6_on_callback )
{
	//mame_printf_debug("irq6 active on %d\n",genesis_scanline_counter);

	{
//      megadrive_irq6_pending = 1;
		if (MEGADRIVE_REG01_IRQ6_ENABLE) cpunum_set_input_line(machine, 0,6,HOLD_LINE);
	}
}

static TIMER_CALLBACK( irq4_on_callback )
{
	//mame_printf_debug("irq4 active on %d\n",genesis_scanline_counter);
	cpunum_set_input_line(machine, 0,4,HOLD_LINE);
}

/*****************************************************************************************/

static int hazemdchoice_megadrive_region_export;
static int hazemdchoice_megadrive_region_pal;
static int hazemdchoice_megadriv_framerate;

MACHINE_RESET( megadriv )
{
	/* default state of z80 = reset, with bus */
	mame_printf_debug("Resetting Megadrive / Genesis\n");

	switch (input_port_read_safe(machine, "REGION",0x00))
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


	genz80.z80_is_reset = 1;
	cpunum_set_input_line(machine, genz80.z80_cpunum, INPUT_LINE_RESET, ASSERT_LINE);
	genz80.z80_has_bus = 1;
	cpunum_set_input_line(machine, genz80.z80_cpunum, INPUT_LINE_HALT, CLEAR_LINE);
	genz80.z80_bank_pos = 0;
	genz80.z80_bank_addr = 0;
	genesis_scanline_counter = -1;

	megadrive_imode = 0;

	megadrive_init_io(machine);

	frame_timer = timer_alloc(frame_timer_callback, NULL);
	scanline_timer = timer_alloc(scanline_timer_callback, NULL);
	render_timer = timer_alloc(render_timer_callback, NULL);

	irq6_on_timer = timer_alloc(irq6_on_callback, NULL);
	irq4_on_timer = timer_alloc(irq4_on_callback, NULL);

	timer_adjust_oneshot(frame_timer, attotime_zero, 0);
	timer_adjust_oneshot(scanline_timer,  attotime_zero, 0);

//  set_refresh_rate(megadriv_framerate);
	cpunum_set_clockscale(machine, 0, 0.9950f); /* Fatal Rewind is very fussy... */
//  cpunum_set_clockscale(machine, 0, 0.3800f); /* Fatal Rewind is very fussy... */

	memset(megadrive_ram,0x00,0x10000);


}

void megadriv_stop_scanline_timer(void)
{
	timer_adjust_oneshot(scanline_timer,  attotime_never, 0);
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
//  cpunum_set_input_line(machine, 1,0,CLEAR_LINE); // if the z80 interrupt hasn't happened by now, clear it..

	if (MD_RESET_BUTTON)  cpunum_set_input_line(machine, 0, INPUT_LINE_RESET, PULSE_LINE);

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

	video_screen_configure(machine->primary_screen, scr_width, megadrive_visible_scanlines, &visarea, HZ_TO_ATTOSECONDS(megadriv_framerate));

#if 0
{
	UINT16 count = 0;
	int y,x,yy,xx;
//  mame_printf_debug("bb\n");

	for (y=0;y<64;y++)
	{
		for (x=0;x<64;x++)
		{
			for (yy=0;yy<8;yy++)
			{
				for (xx=0;xx<2;xx++)
				{
					UINT16 dat;
					UINT16*lineptr;

					dat = MEGADRIV_VDP_VRAM(count);
					count++;
					count &=(0xffff>>1);

					lineptr = BITMAP_ADDR16(render_bitmap, y*8+yy, 0);

					//lineptr[x*8+xx*2]   = (dat & 0xf0)>>4;
					//lineptr[x*8+xx*2+1] = (dat & 0x0f)>>0;

					lineptr[x*8+xx*4+0] = megadrive_vdp_palette_lookup[(dat & 0xf000)>>12];
					lineptr[x*8+xx*4+1] = megadrive_vdp_palette_lookup[(dat & 0x0f00)>>8];
					lineptr[x*8+xx*4+2] = megadrive_vdp_palette_lookup[(dat & 0x00f0)>>4];
					lineptr[x*8+xx*4+3] = megadrive_vdp_palette_lookup[(dat & 0x000f)>>0];
					//lineptr[x*8+xx*2] = megadrive_vdp_palette_lookup[x&0x3f];
					//lineptr[x*8+xx*2+1] = megadrive_vdp_palette_lookup[x&0x3f];


				}
			}
		}
	}
}
#endif

	if (1)
	{
		int xxx;
		UINT64 frametime;

	//  /* reference */
		frametime = ATTOSECONDS_PER_SECOND/megadriv_framerate;

		time_elapsed_since_crap = timer_timeelapsed(frame_timer);
		xxx = ATTOTIME_TO_CYCLES(0,time_elapsed_since_crap);
		//mame_printf_debug("---------- cycles %d, %08x %08x\n",xxx, (UINT32)(time_elapsed_since_crap.attoseconds>>32),(UINT32)(time_elapsed_since_crap.attoseconds&0xffffffff));
		//mame_printf_debug("---------- framet %d, %08x %08x\n",xxx, (UINT32)(frametime>>32),(UINT32)(frametime&0xffffffff));
	}

	timer_adjust_oneshot(frame_timer,  attotime_zero, 0);
	timer_adjust_oneshot(scanline_timer,  attotime_zero, 0);

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


MACHINE_DRIVER_START( megadriv )
	MDRV_CPU_ADD_TAG("main", M68000, MASTER_CLOCK / 7) /* 7.67 MHz */
	MDRV_CPU_PROGRAM_MAP(megadriv_readmem,megadriv_writemem)
	/* IRQs are handled via the timers */

	MDRV_CPU_ADD_TAG("sound", Z80, MASTER_CLOCK / 15) /* 3.58 MHz */
	MDRV_CPU_PROGRAM_MAP(z80_readmem,z80_writemem)
	MDRV_CPU_IO_MAP(z80_portmap,0)
	/* IRQ handled via the timers */

	MDRV_MACHINE_RESET(megadriv)


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
#if 0
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM2612, MASTER_CLOCK/7) /* 7.67 MHz */
	MDRV_SOUND_ROUTE(0, "mono", 0.50)
	MDRV_SOUND_ROUTE(1, "mono", 0.50)

	/* sound hardware */
	MDRV_SOUND_ADD(SN76496, MASTER_CLOCK/15)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50) /* 3.58 MHz */
#else
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YM2612, MASTER_CLOCK/7) /* 7.67 MHz */
	MDRV_SOUND_ROUTE(0, "left", 0.50)
	MDRV_SOUND_ROUTE(1, "right", 0.50)

	/* sound hardware */
	MDRV_SOUND_ADD(SN76496, MASTER_CLOCK/15)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.25) /* 3.58 MHz */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right",0.25) /* 3.58 MHz */
#endif

MACHINE_DRIVER_END

MACHINE_DRIVER_START( megadpal )
	MDRV_IMPORT_FROM(megadriv)

	MDRV_SCREEN_MODIFY("main")
	MDRV_SCREEN_REFRESH_RATE(50)
MACHINE_DRIVER_END

MACHINE_DRIVER_START( _32x )
	MDRV_IMPORT_FROM(megadriv)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(_32x_readmem,_32x_writemem)

	MDRV_CPU_ADD_TAG("SH2main", SH2, 10000000 )
	MDRV_CPU_PROGRAM_MAP(sh2main_readmem,sh2main_writemem)

	MDRV_CPU_ADD_TAG("SH2slave", SH2, 10000000 )
	MDRV_CPU_PROGRAM_MAP(sh2slave_readmem,sh2slave_writemem)
MACHINE_DRIVER_END


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

static int megadriv_tas_callback(void)
{
	return 0; // writeback not allowed
}

static void megadriv_init_common(running_machine *machine)
{
	genz80.z80_cpunum = 1;
	genz80.z80_prgram = auto_malloc(0x2000);
	memory_set_bankptr( 1, genz80.z80_prgram );

	cpunum_set_irq_callback(0, genesis_int_callback);
	megadriv_backupram = NULL;
	megadriv_backupram_length = 0;

	vdp_get_word_from_68k_mem = vdp_get_word_from_68k_mem_default;

	cpunum_set_info_fct(0, CPUINFO_PTR_M68K_TAS_CALLBACK, (void *)megadriv_tas_callback);

	if ((machine->gamedrv->ipt==ipt_megadri6) || (machine->gamedrv->ipt==ipt_ssf2ghw))
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
		mame_printf_debug("DEBUG:: Header: Backup RAM string (ignore for games without)\n");
		for (i=0;i<12;i++)
		{
			UINT16 *rom = (UINT16*)memory_region(REGION_CPU1);
			if (i==2) mame_printf_debug("\nstart: ");
			if (i==4) mame_printf_debug("\nend  : ");
			if (i==6) mame_printf_debug("\n");

			mame_printf_debug("%04x ",rom[(0x1b0/2)+i]);
		}
		mame_printf_debug("\n");
	}

}


DRIVER_INIT( megadriv )
{
	megadriv_init_common(machine);
	hazemdchoice_megadrive_region_export = 1;
	hazemdchoice_megadrive_region_pal = 0;
	hazemdchoice_megadriv_framerate = 60;
}

DRIVER_INIT( megadrij )
{
	megadriv_init_common(machine);
	hazemdchoice_megadrive_region_export = 0;
	hazemdchoice_megadrive_region_pal = 0;
	hazemdchoice_megadriv_framerate = 60;
}

DRIVER_INIT( megadrie )
{
	megadriv_init_common(machine);
	hazemdchoice_megadrive_region_export = 1;
	hazemdchoice_megadrive_region_pal = 1;
	hazemdchoice_megadriv_framerate = 50;
}

DRIVER_INIT( megadsvp )
{
	megadriv_init_common(machine);
	svp_init(machine);
	hazemdchoice_megadrive_region_export = 1;
	hazemdchoice_megadrive_region_pal = 0;
	hazemdchoice_megadriv_framerate = 60;
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
void megatech_set_megadrive_z80_as_megadrive_z80(running_machine *machine)
{
	/* INIT THE PORTS *********************************************************************************************/
	memory_install_readwrite8_handler(machine, 1, ADDRESS_SPACE_IO, 0x0000, 0xffff, 0, 0, z80_unmapped_port_r, z80_unmapped_port_w);

	/* catch any addresses that don't get mapped */
	memory_install_readwrite8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x0000, 0xffff, 0, 0, z80_unmapped_r, z80_unmapped_w);


	memory_install_readwrite8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x0000, 0x1fff, 0, 0, SMH_BANK1, SMH_BANK1);
	memory_set_bankptr( 1, genz80.z80_prgram );

	memory_install_readwrite8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x0000, 0x1fff, 0, 0, SMH_BANK6, SMH_BANK6);
	memory_set_bankptr( 6, genz80.z80_prgram );


	// not allowed??
//  memory_install_readwrite8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x2000, 0x3fff, 0, 0, SMH_BANK1, SMH_BANK1);

	memory_install_readwrite8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x4000, 0x4003, 0, 0, megadriv_z80_YM2612_read, megadriv_z80_YM2612_write);
	memory_install_write8_handler    (machine, 1, ADDRESS_SPACE_PROGRAM, 0x6000, 0x6000, 0, 0, megadriv_z80_z80_bank_w);
	memory_install_write8_handler    (machine, 1, ADDRESS_SPACE_PROGRAM, 0x6001, 0x6001, 0, 0, megadriv_z80_z80_bank_w);
	memory_install_read8_handler     (machine, 1, ADDRESS_SPACE_PROGRAM, 0x6100, 0x7eff, 0, 0, megadriv_z80_unmapped_read);
	memory_install_readwrite8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x7f00, 0x7fff, 0, 0, megadriv_z80_vdp_read, megadriv_z80_vdp_write);
	memory_install_readwrite8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, z80_read_68k_banked_data, z80_write_68k_banked_data);
}

