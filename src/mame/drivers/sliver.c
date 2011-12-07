/*
Sliver - Hollow Corp.1996
driver by Tomasz Slanina

Custom blitter + background framebuffer + oki for sound.

The background images on this hardware are in JPEG format, the Zoran chips are
hardware JPEG decompression chips.
Driver (temporary) uses fake rom with decompressed images (gfx.bin)

TODO:
- verify OKI rom banking  (bank num inverted or not)
- DIPS
- add jpeg decompression
- fix transparency problems in some stages


PCB Layout
----------

WS16-AJ-940820
|---------|-----------|------------------------------------|
|   TL084 |   i8031   |   KA-1                             |
|         |-----------||--------|                          |
|  AD-65    KA-2       |ACTEL   |   KA-6  KA-7  KA-8  KA-9 |
|                      |A1020B  |                          |
|           KA-3       |PL84C   |                          |
|               PAL    |        |                          |
|                      |--------|                          |
|             AT76C176                62256        62256   |
|                                                          |
|J             KM75C02                62256        62256   |
|A                                                         |
|M                 PAL                62256        62256   |
|M                                                         |
|A                                    62256        62256   |
|                                                  HY534256|
|   DSW1           PAL                 |-------|   HY534256|
|          PAL     PAL                 |ZORAN  |   HY534256|
|                            |-------| |ZR36011|   HY534256|
|   DSW2   62256    62256    |ZORAN  | |-------|           |
|          KA-4     KA-5     |ZR36050|                     |
|                            |-------|        |-------|    |
|        |------------------|16MHz            |FUJI   |    |
|        |                  |                 |MD0204 |    |
|        |    MC68000P10    |                 |-------|    |
|        |                  |           KA-12  KA-11  KA-10|
|24MHz   |------------------|                              |
|----------------------------------------------------------|
Notes:
      i8031    - Intel 8031 CPU, clock 8.000MHz (DIP40)
      68000    - Motorola MC68000 CPU, clock 12.000MHz (DIP64)
      AT76C176 - Atmel AT76C176 1-Channel 6-Bit AD/DA Convertor with Clamp Circuit (DIP28).
                 When removed, text and _some_ graphics turn black (palette related use)
                 This chip is compatible to Fujitsu MB40176
      A1020B   - Actel A1020B FPGA (PLCC84)
      ZR36050  - Zoran ZR36050PQC-21 DF4B9423G (QFP100)
      ZR36011  - Zoran ZR36011PQC JAPAN 079414 (QFP100)
      MD0204   - Fuji MD0204 JAPAN F39D110 (QFP128) - Memory controller
      62256    - 32K x8 SRAM (DIP28)
      HY534256 - Hyundai 256K x4 (1MBit) DRAM (DIP20)
      KM75C02  - Samsung KM75C02 FIFO RAM (DIP28)
      AD-65    - Clone OKI M6295 (QFP44), clock 1.000MHz, sample rate = 1000000Hz / 132
      VSync    - 60Hz
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "cpu/mcs51/mcs51.h"
#include "video/ramdac.h"

#define FIFO_SIZE 1024
#define IO_SIZE 	0x100
#define COMMAND_SIZE 8
#define x_offset 0x45
#define y_offset 0x0d

class sliver_state : public driver_device
{
public:
	sliver_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
		{ }

	UINT16 m_io_offset;
	UINT16 m_io_reg[IO_SIZE];
	UINT16 m_fifo[FIFO_SIZE];
	UINT16 m_fptr;

	int m_jpeg_addr;
	UINT16 m_jpeg1;
	UINT16 m_jpeg2;
	int m_jpeg_h;
	int m_jpeg_w;
	int m_jpeg_x;
	int m_jpeg_y;
	int m_tmp_counter;
	int m_clr_offset;

	UINT8 *m_colorram;
	bitmap_t *m_bitmap_fg;
	bitmap_t *m_bitmap_bg;

	UINT16 m_tempbuf[8];

	required_device<cpu_device> m_maincpu;
};

static const int gfxlookup[][4]=
{
   { 0x0000000, 0x0000000, 512, 256 },
   { 0x0060000, 0x0007fa5, 512, 256 },
   { 0x00c0000, 0x0008c04, 512, 512 },
   { 0x0180000, 0x001642b, 512, 512 },
   { 0x0240000, 0x00240fb, 512, 512 },
   { 0x0300000, 0x0033494, 512, 256 },
   { 0x0360000, 0x003ad9a, 512, 256 },
   { 0x03c0000, 0x003cab3, 512, 256 },
   { 0x0420000, 0x003e50c, 512, 256 },
   { 0x0480000, 0x003fd94, 512, 256 },
   { 0x04e0000, 0x004166e, 512, 256 },
   { 0x0540000, 0x004306a, 512, 256 },
   { 0x05a0000, 0x0044c65, 512, 256 },
   { 0x0600000, 0x0046aeb, 512, 256 },
   { 0x0660000, 0x0048a86, 512, 512 },
   { 0x0720000, 0x00527c7, 512, 512 },
   { 0x07e0000, 0x005a789, 512, 256 },
   { 0x0840000, 0x005f9b0, 512, 256 },
   { 0x08a0000, 0x0065621, 512, 512 },
   { 0x0960000, 0x006fbd6, 512, 512 },
   { 0x0a20000, 0x0077b98, 512, 512 },
   { 0x0ae0000, 0x007fa6e, 512, 512 },
   { 0x0ba0000, 0x0087a42, 512, 512 },
   { 0x0c60000, 0x008f996, 512, 256 },
   { 0x0cc0000, 0x0094b82, 512, 512 },
   { 0x0d80000, 0x009e27a, 512, 512 },
   { 0x0e40000, 0x00a6157, 512, 256 },
   { 0x0ea0000, 0x00ab417, 512, 512 },
   { 0x0f60000, 0x00b33c1, 512, 512 },
   { 0x1020000, 0x00bd7f1, 512, 256 },
   { 0x1080000, 0x00c1679, 512, 512 },
   { 0x1140000, 0x00cbc8a, 512, 256 },
   { 0x11a0000, 0x00d0ee2, 512, 256 },
   { 0x1200000, 0x00d4d76, 512, 256 },
   { 0x1260000, 0x00d8a4f, 512, 256 },
   { 0x12c0000, 0x00dc980, 512, 256 },
   { 0x1320000, 0x00e07ba, 512, 256 },
   { 0x1380000, 0x00e45a2, 512, 256 },
   { 0x13e0000, 0x00e842e, 512, 256 },
   { 0x1440000, 0x00ec0db, 512, 256 },
   { 0x14a0000, 0x00efdae, 512, 256 },
   { 0x1500000, 0x00f3c99, 512, 256 },
   { 0x1560000, 0x00f794f, 512, 256 },
   { 0x15c0000, 0x00fb5db, 512, 256 },
   { 0x1620000, 0x00ff2e6, 512, 256 },
   { 0x1680000, 0x0103156, 512, 256 },
   { 0x16e0000, 0x0106fcf, 512, 256 },
   { 0x1740000, 0x010ae0c, 512, 256 },
   { 0x17a0000, 0x010edaf, 512, 256 },
   { 0x1800000, 0x0112bac, 512, 256 },
   { 0x1860000, 0x01169c1, 128, 128 },
   { 0x186c000, 0x0119b95, 128, 128 },
   { 0x1878000, 0x011d0dc, 128, 128 },
   { 0x1884000, 0x01205ca, 128, 128 },
   { 0x1890000, 0x01236b2, 128, 128 },
   { 0x189c000, 0x01268ff, 128, 128 },
   { 0x18a8000, 0x0129bef, 128, 128 },
   { 0x18b4000, 0x012cf51, 128, 128 },
   { 0x18c0000, 0x0130331, 128, 128 },
   { 0x18cc000, 0x013370d, 128, 128 },
   { 0x18d8000, 0x0136a80, 128, 128 },
   { 0x18e4000, 0x0139d2a, 128, 128 },
   { 0x18f0000, 0x013ce39, 128, 128 },
   { 0x18fc000, 0x0140236, 128, 128 },
   { 0x1908000, 0x0143701, 128, 128 },
   { 0x1914000, 0x0146cb3, 128, 128 },
   { 0x1920000, 0x014a40d, 128, 128 },
   { 0x192c000, 0x014da18, 128, 128 },
   { 0x1938000, 0x0150ea4, 512, 256 },
   { 0x1998000, 0x0154e17, 512, 256 },
   { 0x19f8000, 0x0158ddf, 512, 256 },
   { 0x1a58000, 0x015cd80, 512, 256 },
   { 0x1ab8000, 0x0160d5b, 512, 256 },
   { 0x1b18000, 0x0164d2f, 512, 256 },
   { 0x1b78000, 0x017b5db, 512, 256 },
   { 0x1bd8000, 0x017f2e6, 512, 256 },
   { -1,-1,-1,-1}
};


static void plot_pixel_rgb(sliver_state *state, int x, int y, UINT32 r, UINT32 g, UINT32 b)
{
	UINT16 color;

	if (y < 0 || x < 0 || x > 383 || y > 255)
		return;

	if (state->m_bitmap_bg->bpp == 32)
	{
		*BITMAP_ADDR32(state->m_bitmap_bg, y, x) = r | (g<<8) | (b<<16);
	}
	else
	{
		r>>=3;
		g>>=3;
		b>>=3;
		color = r|(g<<5)|(b<<10);
		*BITMAP_ADDR16(state->m_bitmap_bg, y, x) = color;
	}
}

static void plot_pixel_pal(running_machine &machine, int x, int y, int addr)
{
	sliver_state *state = machine.driver_data<sliver_state>();
	UINT32 r,g,b;
	UINT16 color;

	if (y < 0 || x < 0 || x > 383 || y > 255)
		return;

	b=(state->m_colorram[addr] << 2) | (state->m_colorram[addr] & 0x3);
	g=(state->m_colorram[addr+0x100] << 2) | (state->m_colorram[addr+0x100] & 3);
	r=(state->m_colorram[addr+0x200] << 2) | (state->m_colorram[addr+0x200] & 3);

	if (state->m_bitmap_fg->bpp == 32)
	{
		*BITMAP_ADDR32(state->m_bitmap_fg, y, x) = r | (g<<8) | (b<<16);
	}
	else
	{
		r>>=3;
		g>>=3;
		b>>=3;
		color = r|(g<<5)|(b<<10);
		*BITMAP_ADDR16(state->m_bitmap_fg, y, x) = color;
	}
}

static WRITE16_HANDLER( fifo_data_w )
{
	sliver_state *state = space->machine().driver_data<sliver_state>();

	if (state->m_tmp_counter < 8)
	{
		COMBINE_DATA(&state->m_tempbuf[state->m_tmp_counter]);
		state->m_tmp_counter++;
		if (state->m_tmp_counter == 8) // copy 8 bytes to fifo,  every byte should be copied directly, but it's easier to copy whole commands
		{
			do
			{
				state->m_fifo[state->m_fptr++]=state->m_tempbuf[8-state->m_tmp_counter];
				if (state->m_fptr > (FIFO_SIZE - 1))
				{
					state->m_fptr=FIFO_SIZE-1;
				}
			}
			while (--state->m_tmp_counter > 0);
		}
	}
}

static void blit_gfx(running_machine &machine)
{
	sliver_state *state = machine.driver_data<sliver_state>();
	int tmpptr=0;
	const UINT8 *rom = machine.region("user1")->base();

	while (tmpptr < state->m_fptr)
	{
		int x,y,romdata;
		int w,h;
		int romoffs=state->m_fifo[tmpptr+0]+(state->m_fifo[tmpptr+1] << 8)+(state->m_fifo[tmpptr+2] << 16);

		w=state->m_fifo[tmpptr+3]+1;
		h=state->m_fifo[tmpptr+4]+1;

		if (state->m_fifo[tmpptr+7] == 0)
		{
			for (y=0; y < h; y++)
			{
				for (x=0; x < w; x++)
				{
					romdata = rom[romoffs&0x1fffff];
					if (romdata)
					{
						plot_pixel_pal(machine, state->m_fifo[tmpptr+5]+state->m_fifo[tmpptr+3]-x, state->m_fifo[tmpptr+6]+state->m_fifo[tmpptr+4]-y, romdata);
					}
					romoffs++;
				}
			}
		}
		tmpptr+=COMMAND_SIZE;
	}
}

static WRITE16_HANDLER( fifo_clear_w )
{
	sliver_state *state = space->machine().driver_data<sliver_state>();

	bitmap_fill(state->m_bitmap_fg, 0,0);
	state->m_fptr=0;
	state->m_tmp_counter=0;
}

static WRITE16_HANDLER( fifo_flush_w )
{
	blit_gfx(space->machine());
}


static WRITE16_HANDLER( jpeg1_w )
{
	sliver_state *state = space->machine().driver_data<sliver_state>();

	COMBINE_DATA(&state->m_jpeg1);
}

static void render_jpeg(running_machine &machine)
{
	sliver_state *state = machine.driver_data<sliver_state>();
	int x, y;
	int addr = state->m_jpeg_addr;
	UINT8 *rom;

	bitmap_fill(state->m_bitmap_bg, 0, 0);
	if (addr < 0)
	{
		return;
	}

	rom = machine.region("user3")->base();
	for (y = 0; y < state->m_jpeg_h; y++)
	{
		for (x = 0; x < state->m_jpeg_w; x++)
		{
			plot_pixel_rgb(state, x - x_offset + state->m_jpeg_x, state->m_jpeg_h - y - y_offset - state->m_jpeg_y, rom[addr], rom[addr + 1], rom[addr + 2]);
			addr+=3;
		}
	}
}

static int find_data(int offset)
{
	int idx = 0;
	while (gfxlookup[idx][0] >= 0)
	{
		if (offset == gfxlookup[idx][1])
		{
			return idx;
		}
		++idx;
	}
	return -1;
}

static WRITE16_HANDLER( jpeg2_w )
{
	sliver_state *state = space->machine().driver_data<sliver_state>();
	int idx;

	COMBINE_DATA(&state->m_jpeg2);

	idx = find_data((int)state->m_jpeg2 + (((int)state->m_jpeg1) << 16));
	if (idx >= 0)
	{
		state->m_jpeg_addr = gfxlookup[idx][0];
		state->m_jpeg_w = gfxlookup[idx][2];
		state->m_jpeg_h = gfxlookup[idx][3];
		render_jpeg(space->machine());
	}
	else
	{
		state->m_jpeg_addr = -1;
	}
}

static WRITE16_HANDLER(io_offset_w)
{
	sliver_state *state = space->machine().driver_data<sliver_state>();

	COMBINE_DATA(&state->m_io_offset);
}

static WRITE16_HANDLER(io_data_w)
{
	sliver_state *state = space->machine().driver_data<sliver_state>();

	if (state->m_io_offset < IO_SIZE)
	{
		int tmpx, tmpy;
		COMBINE_DATA(&state->m_io_reg[state->m_io_offset]);

		tmpy = state->m_io_reg[0x1a] + (state->m_io_reg[0x1b] << 8) - state->m_io_reg[0x20]; //0x20  ???
		tmpx = state->m_io_reg[0x1e] + (state->m_io_reg[0x1f] << 8);

		if (tmpy != state->m_jpeg_y || tmpx != state->m_jpeg_x)
		{
			state->m_jpeg_x = tmpx;
			state->m_jpeg_y = tmpy;
			render_jpeg(space->machine());
		}
	}
	else
	{
		logerror("I/O access out of range: %x\n", state->m_io_offset);
	}
}

static WRITE16_HANDLER(sound_w)
{
	soundlatch_w(space, 0, data & 0xff);
	cputag_set_input_line(space->machine(), "audiocpu", MCS51_INT0_LINE, HOLD_LINE);
}

static ADDRESS_MAP_START( sliver_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM

	AM_RANGE(0x100000, 0x100001) AM_DEVWRITE8_MODERN("ramdac", ramdac_device, index_w, 0x00ff)
	AM_RANGE(0x100002, 0x100003) AM_DEVWRITE8_MODERN("ramdac", ramdac_device, pal_w, 0x00ff)
	AM_RANGE(0x100004, 0x100005) AM_DEVWRITE8_MODERN("ramdac", ramdac_device, mask_w, 0x00ff)

	AM_RANGE(0x300002, 0x300003) AM_NOP // bit 0 tested, writes 0xe0 and 0xc0 - both r and w at the end of interrupt code

	AM_RANGE(0x300004, 0x300005) AM_WRITE(io_offset_w) //unknown i/o device
	AM_RANGE(0x300006, 0x300007) AM_WRITE(io_data_w)

	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("DSW")
	AM_RANGE(0x400006, 0x400007) AM_WRITE(fifo_data_w)
	AM_RANGE(0x400008, 0x400009) AM_WRITE(fifo_clear_w)
	AM_RANGE(0x40000a, 0x40000b) AM_WRITE(fifo_flush_w)
	AM_RANGE(0x40000c, 0x40000d) AM_WRITE(jpeg1_w)
	AM_RANGE(0x40000e, 0x40000f) AM_WRITE(jpeg2_w)

	AM_RANGE(0x400010, 0x400015) AM_WRITENOP //unknown
	AM_RANGE(0x400016, 0x400017) AM_WRITE(sound_w)
	AM_RANGE(0x400018, 0x400019) AM_WRITENOP //unknown

	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

// Sound CPU

static WRITE8_HANDLER(oki_setbank)
{
	UINT8 *sound = space->machine().region("oki")->base();
	int bank=(data^0xff)&3; //xor or not ?
	memcpy(sound+0x20000, sound+0x100000+0x20000*bank, 0x20000);
}

static ADDRESS_MAP_START( soundmem_prg, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( soundmem_io, AS_IO, 8 )
	AM_RANGE(0x0100, 0x0100) AM_DEVREADWRITE_MODERN("oki", okim6295_device, read, write)
	AM_RANGE(0x0101, 0x0101) AM_READ(soundlatch_r)
	/* ports */
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_WRITE( oki_setbank )
ADDRESS_MAP_END

static VIDEO_START(sliver)
{
	sliver_state *state = machine.driver_data<sliver_state>();

	state->m_bitmap_bg = machine.primary_screen->alloc_compatible_bitmap();
	state->m_bitmap_fg = machine.primary_screen->alloc_compatible_bitmap();
}

static SCREEN_UPDATE(sliver)
{
	sliver_state *state = screen->machine().driver_data<sliver_state>();

	copybitmap      (bitmap, state->m_bitmap_bg, 0, 0, 0, 0, cliprect);
	copybitmap_trans(bitmap, state->m_bitmap_fg, 0, 0, 0, 0, cliprect, 0);
	return 0;
}

static INPUT_PORTS_START( sliver )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN ) //jpeg ready flag
	PORT_BIT( 0xffa4, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(	0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(	0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(	0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0030, 0x0020, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0030, "2" )
	PORT_DIPSETTING(    0x0020, "3" )
	PORT_DIPSETTING(    0x0010, "4" )
	PORT_DIPSETTING(    0x0000, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Demo_Sounds" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unknown
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static ADDRESS_MAP_START( ramdac_map, AS_0, 8 )
	AM_RANGE(0x000, 0x3ff) AM_RAM AM_BASE_MEMBER(sliver_state,m_colorram)
ADDRESS_MAP_END

static RAMDAC_INTERFACE( ramdac_intf )
{
	0
};

static TIMER_DEVICE_CALLBACK ( obj_irq_cb )
{
	sliver_state *state = timer.machine().driver_data<sliver_state>();

	device_set_input_line(state->m_maincpu, 3, HOLD_LINE);
}

static MACHINE_CONFIG_START( sliver, sliver_state )
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(sliver_map)
	MCFG_CPU_VBLANK_INT("screen",irq4_line_hold)
	MCFG_TIMER_ADD_PERIODIC("obj_actel", obj_irq_cb, attotime::from_hz(60)) /* unknown clock, causes "obj actel ready error" without this */
	// irq 2 valid but not used?

	MCFG_CPU_ADD("audiocpu", I8051, 8000000)
	MCFG_CPU_PROGRAM_MAP(soundmem_prg)
	MCFG_CPU_IO_MAP(soundmem_io)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 384-1-16, 0*8, 240-1)
	MCFG_SCREEN_UPDATE(sliver)

	MCFG_RAMDAC_ADD("ramdac", ramdac_intf, ramdac_map)

	MCFG_VIDEO_START(sliver)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.6)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.6)
MACHINE_CONFIG_END

ROM_START( sliver )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ka-4.bin", 0x00001, 0x20000, CRC(4906367f) SHA1(cc030930ffe7018ba6c362cab136798d027db7d8) )
	ROM_LOAD16_BYTE( "ka-5.bin", 0x00000, 0x20000, CRC(f260dabc) SHA1(3727cb8aa652809386075b39a1d85d5b20973702) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 8031 */
	ROM_LOAD( "ka-1.bin", 0x000000, 0x10000, CRC(56e616a2) SHA1(f8952aba62ae0410e300d99e95dc8b752543af1e) )

	ROM_REGION( 0x180000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ka-2.bin", 0x000000, 0x20000, CRC(3df96eb0) SHA1(ec3dfc29da08f6525a1c708839f83094a6784f72) )
	ROM_LOAD( "ka-3.bin", 0x100000, 0x80000, CRC(33ee929c) SHA1(a652ad68c547248ef5fa1ed8006b7ac7aef76383) )

	ROM_REGION( 0x200000, "user1", 0 ) /* Graphics (not tiles) */
	ROM_LOAD16_BYTE( "ka-8.bin", 0x000000, 0x80000, CRC(dbfd7489) SHA1(4a7b07d041dce04a8d8d6688698164f988baefc9) )
	ROM_LOAD16_BYTE( "ka-6.bin", 0x000001, 0x80000, CRC(bd182316) SHA1(a22db9f73a2865f59630183c14201aeede821642) )
	ROM_LOAD16_BYTE( "ka-9.bin", 0x100000, 0x40000, CRC(71f044ba) SHA1(bd88bfaa0249de9fd8eb8bd25eae0126744a9046) )
	ROM_LOAD16_BYTE( "ka-7.bin", 0x100001, 0x40000, CRC(1c5d6fb9) SHA1(372533264eb41a5f57b2a59eb039adb6334f36c5) )

	ROM_REGION( 0x180000, "user2", 0 ) /* JPEG(!) compressed GFX */
	ROM_LOAD( "ka-10.bin", 0x000000, 0x80000, CRC(a6824271) SHA1(2eefa4e61491f7b72ccde744fa6f88a1a3c60c92) )
	ROM_LOAD( "ka-11.bin", 0x080000, 0x80000, CRC(4ae121ff) SHA1(ece7cc07483801a0d436def977d72dc7b1a07c8f) )
	ROM_LOAD( "ka-12.bin", 0x100000, 0x80000, CRC(0901e142) SHA1(68ebd38beeedf53414a831c01813881feee33446) )

	ROM_REGION( 0x2000000, "user3", 0 ) /* decompressed GFX  - temporary!*/
	ROM_LOAD( "gfx.bin", 0x000000, 0x2000000, BAD_DUMP CRC(706f264e) SHA1(dbcc8dbf30bd65d86bcde7d6db1b08af4242a253) )
ROM_END

static DRIVER_INIT(sliver)
{
	sliver_state *state = machine.driver_data<sliver_state>();

	state->m_jpeg_addr = -1;
}

GAME( 1996, sliver, 0,        sliver, sliver, sliver, ROT0,  "Hollow Corp", "Sliver", GAME_IMPERFECT_GRAPHICS )
