/*************************************************************************************************************

                                                -= IGS Lord Of Gun =-

                                          driver by   Luca Elia (l.elia@tin.it)
                                 skeleton driver by   David Haywood
                                  code decrypted by   unknown


CPU     :   68000 + Z80
Custom  :   IGS005, IGS006, IGS007, IGS008
Sound   :   M6295 [+ M6295] + YM3812 or YMF278B
NVRAM   :   93C46

-----------------------------------------------------------------------------------
Year + Game           PCB    FM Sound  Chips                         Notes
-----------------------------------------------------------------------------------
1994  Lord Of Gun     T0076  YM3812    IGS005? IGS006 IGS007 IGS008  Lightguns
1994  Alien Challenge ?      YMF278B   ?                             Not encrypted
-----------------------------------------------------------------------------------

To do:

- Protection emulation instead of patching the roms
- lordgun: in the 3rd leg of the ship stage, sometimes part of a far jetboat is drawn above a nearer sub (both sprites).
  But this is correct considering both priorities and sprite list positions. Original game bug?
- lordgun: wrong colors for tilemap 0 in the 2nd leg of the last stage (where some sprite priority bugs happen too)
- lordgun: in the jungle level, final enemy, tilemap 0 does not scroll. It may have wrong priority, or may need to be
  disabled, even though it is used by enemies to hide, so it's probably just odd but right after all.
- aliencha: no info on the PCB (clocks, chips etc.)

Notes:

- aliencha: when booting into service mode, keep buttons 1, 2 and 3 pressed to show more options
- aliencha: original videos at http://www.youtube.com/watch?v=TRHb3WTGuvk, http://www.youtube.com/watch?v=_RaCpHwyS78.
  The latter shows English text and a 1995 copyright (instead of 1994) in a different font.
- aliencha: routine at A34 - english/chinese text, A38 - english/chinese names, A3C - Alien Challenge/Round House Rumble title

*************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"
#include "machine/eeprom.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "sound/ymf278b.h"
#include "includes/lordgun.h"

/***************************************************************************

    Code Decryption / Protection patches

***************************************************************************/

static DRIVER_INIT( lordgun )
{
	int i;
	UINT16 *rom = (UINT16 *)machine.region("maincpu")->base();

	// Decryption

	int rom_size = 0x100000;
	for(i = 0; i < rom_size/2; i++)
	{
		UINT16 x = rom[i];

		if((i & 0x0120) == 0x0100 || (i & 0x0a00) == 0x0800)
			x ^= 0x0010;

		rom[i] = x;
	}

	// Protection

	rom[0x14832/2]	=	0x6000;		// 014832: 6700 0006  beq     $1483a (protection)
	rom[0x1587e/2]	=	0x6010;		// 01587E: 6710       beq     $15890 (rom check)
}


// From XingXing:
static DRIVER_INIT( aliencha )
{
	UINT16 *rom = (UINT16 *)machine.region("maincpu")->base();

	// Protection

	rom[0x0A558/2]	=	0x6012;		// 0A558  beq.s   $A56C
	rom[0x0A8DC/2]	=	0x6012;		// 0A8DC  beq.s   $A8F0
	rom[0x0AC92/2]	=	0x6012;		// 0AC92  beq.s   $ACA6
	rom[0x124CC/2]	=	0x6012;		// 124CC  beq.s   $124E0
	rom[0x12850/2]	=	0x6012;		// 12850  beq.s   $12864
	rom[0x12C06/2]	=	0x6012;		// 12C06  beq.s   $12C1A
	rom[0x1862A/2]	=	0x6012;		// 1862A  beq.s   $1863E
	rom[0x189AE/2]	=	0x6012;		// 189AE  beq.s   $189C2
	rom[0x18D64/2]	=	0x6012;		// 18D64  beq.s   $18D78
	rom[0x230FC/2]	=	0x6012;		// 230FC  beq.s   $23110
	rom[0x23480/2]	=	0x6012;		// 23480  beq.s   $23494
	rom[0x23836/2]	=	0x6012;		// 23836  beq.s   $2384A
	rom[0x2BD0E/2]	=	0x6012;		// 2BD0E  beq.s   $2BD22
	rom[0x2C092/2]	=	0x6012;		// 2C092  beq.s   $2C0A6
	rom[0x2C448/2]	=	0x6012;		// 2C448  beq.s   $2C45C
}


static DRIVER_INIT( alienchac )
{
	UINT16 *rom = (UINT16 *)machine.region("maincpu")->base();

	// Protection

	rom[0x0A550/2]	=	0x6012;		// 0A558  beq.s   $A564
	rom[0x0A8D4/2]	=	0x6012;		// 0A8D4  beq.s   $A8E8
	rom[0x0AC8A/2]	=	0x6012;		// 0AC8A  beq.s   $AC9E
	rom[0x124B8/2]	=	0x6012;		// 124B8  beq.s   $124CC
	rom[0x1283C/2]	=	0x6012;		// 1283C  beq.s   $12850
	rom[0x12BF2/2]	=	0x6012;		// 12BF2  beq.s   $12C06
	rom[0x18616/2]	=	0x6012;		// 18616  beq.s   $1862A
	rom[0x1899A/2]	=	0x6012;		// 1899A  beq.s   $189AE
	rom[0x18D50/2]	=	0x6012;		// 18D50  beq.s   $18D64
	rom[0x230E8/2]	=	0x6012;		// 230E8  beq.s   $230FC
	rom[0x2346C/2]	=	0x6012;		// 2346C  beq.s   $23480
	rom[0x23822/2]	=	0x6012;		// 23822  beq.s   $23822
	rom[0x2BCFA/2]	=	0x6012;		// 2BCFA  beq.s   $2BD0E
	rom[0x2C07E/2]	=	0x6012;		// 2C07E  beq.s   $2C092
	rom[0x2C434/2]	=	0x6012;		// 2C434  beq.s   $2C448
}


/***************************************************************************

    Memory Maps - Main

***************************************************************************/

static WRITE8_DEVICE_HANDLER(fake_w)
{
}
static WRITE8_DEVICE_HANDLER(fake2_w)
{
//  popmessage("%02x",data);
}

static WRITE8_DEVICE_HANDLER( lordgun_eeprom_w )
{
	lordgun_state *state = device->machine().driver_data<lordgun_state>();
	eeprom_device *eeprom = device->machine().device<eeprom_device>("eeprom");
	int i;

	if (data & ~0xfd)
	{
//      popmessage("EE: %02x", data);
		logerror("%s: Unknown EEPROM bit written %02X\n",device->machine().describe_context(),data);
	}

	coin_counter_w(device->machine(), 0, data & 0x01);

	// Update light guns positions
	for (i = 0; i < 2; i++)
		if ( (data & (0x04 << i)) && !(state->m_old & (0x04 << i)) )
			lordgun_update_gun(device->machine(), i);

	// latch the bit
	eeprom->write_bit(data & 0x40);

	// reset line asserted: reset.
	eeprom->set_cs_line((data & 0x10) ? CLEAR_LINE : ASSERT_LINE );

	// clock line asserted: write latch or select next bit to read
	eeprom->set_clock_line((data & 0x20) ? ASSERT_LINE : CLEAR_LINE );

	state->m_whitescreen = data & 0x80;

	state->m_old = data;
}

static WRITE8_DEVICE_HANDLER( aliencha_eeprom_w )
{
	lordgun_state *state = device->machine().driver_data<lordgun_state>();
	eeprom_device *eeprom = device->machine().device<eeprom_device>("eeprom");

	if (~data & ~0xf8)
	{
//      popmessage("EE: %02x", data);
		logerror("%s: Unknown EEPROM bit written %02X\n",device->machine().describe_context(),data);
	}

	// bit 1? cleared during screen transitions
	state->m_whitescreen = !(data & 0x02);

	coin_counter_w(device->machine(), 0, data & 0x08);
	coin_counter_w(device->machine(), 1, data & 0x10);

	// latch the bit
	eeprom->write_bit(data & 0x80);

	// reset line asserted: reset.
	eeprom->set_cs_line((data & 0x20) ? CLEAR_LINE : ASSERT_LINE );

	// clock line asserted: write latch or select next bit to read
	eeprom->set_clock_line((data & 0x40) ? ASSERT_LINE : CLEAR_LINE );
}


static READ8_DEVICE_HANDLER( aliencha_dip_r )
{
	lordgun_state *state = device->machine().driver_data<lordgun_state>();
	switch (state->m_aliencha_dip_sel & 0x70)
	{
		case 0x30:	return input_port_read(device->machine(), "DIP1");
		case 0x60:	return input_port_read(device->machine(), "DIP2");
		case 0x50:	return input_port_read(device->machine(), "DIP3");

		default:
			logerror("%s: dip_r with unknown dip_sel = %02X\n",device->machine().describe_context(),state->m_aliencha_dip_sel);
			return 0xff;
	}
}

static WRITE8_DEVICE_HANDLER( aliencha_dip_w )
{
	lordgun_state *state = device->machine().driver_data<lordgun_state>();
	state->m_aliencha_dip_sel = data;
}


// Unknown, always equal to 7 in lordgun, aliencha.
WRITE16_MEMBER(lordgun_state::lordgun_priority_w)
{
	COMBINE_DATA(&m_priority);
//  popmessage("PR: %04x", data);
}


READ16_MEMBER(lordgun_state::lordgun_gun_0_x_r)
{
	return m_gun[0].hw_x;
}

READ16_MEMBER(lordgun_state::lordgun_gun_0_y_r)
{
	return m_gun[0].hw_y;
}

READ16_MEMBER(lordgun_state::lordgun_gun_1_x_r)
{
	return m_gun[1].hw_x;
}

READ16_MEMBER(lordgun_state::lordgun_gun_1_y_r)
{
	return m_gun[1].hw_y;
}


WRITE16_MEMBER(lordgun_state::lordgun_soundlatch_w)
{
	if (ACCESSING_BITS_0_7)		soundlatch_w (space, 0, (data >> 0) & 0xff);
	if (ACCESSING_BITS_8_15)	soundlatch2_w(space, 0, (data >> 8) & 0xff);

	cputag_set_input_line(machine(), "soundcpu", INPUT_LINE_NMI, PULSE_LINE);
}

static ADDRESS_MAP_START( lordgun_map, AS_PROGRAM, 16, lordgun_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0x210000, 0x21ffff) AM_RAM AM_BASE(m_priority_ram)						// PRIORITY
	AM_RANGE(0x300000, 0x30ffff) AM_RAM_WRITE(lordgun_vram_0_w) AM_BASE(m_vram[0])	// DISPLAY
	AM_RANGE(0x310000, 0x313fff) AM_RAM_WRITE(lordgun_vram_1_w) AM_BASE(m_vram[1])	// DISPLAY
	AM_RANGE(0x314000, 0x314fff) AM_RAM_WRITE(lordgun_vram_2_w) AM_BASE(m_vram[2])	// DISPLAY
	AM_RANGE(0x315000, 0x317fff) AM_RAM														//
	AM_RANGE(0x318000, 0x319fff) AM_RAM_WRITE(lordgun_vram_3_w) AM_BASE(m_vram[3])	// DISPLAY
	AM_RANGE(0x31c000, 0x31c7ff) AM_RAM AM_BASE(m_scrollram)							// LINE
	AM_RANGE(0x400000, 0x4007ff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)						// ANIMATOR
	AM_RANGE(0x500000, 0x500fff) AM_RAM_WRITE(lordgun_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0x502000, 0x502001) AM_WRITEONLY AM_BASE(m_scroll_x[0])
	AM_RANGE(0x502200, 0x502201) AM_WRITEONLY AM_BASE(m_scroll_x[1])
	AM_RANGE(0x502400, 0x502401) AM_WRITEONLY AM_BASE(m_scroll_x[2])
	AM_RANGE(0x502600, 0x502601) AM_WRITEONLY AM_BASE(m_scroll_x[3])
	AM_RANGE(0x502800, 0x502801) AM_WRITEONLY AM_BASE(m_scroll_y[0])
	AM_RANGE(0x502a00, 0x502a01) AM_WRITEONLY AM_BASE(m_scroll_y[1])
	AM_RANGE(0x502c00, 0x502c01) AM_WRITEONLY AM_BASE(m_scroll_y[2])
	AM_RANGE(0x502e00, 0x502e01) AM_WRITEONLY AM_BASE(m_scroll_y[3])
	AM_RANGE(0x503000, 0x503001) AM_WRITE(lordgun_priority_w)
	AM_RANGE(0x503800, 0x503801) AM_READ(lordgun_gun_0_x_r)
	AM_RANGE(0x503a00, 0x503a01) AM_READ(lordgun_gun_1_x_r)
	AM_RANGE(0x503c00, 0x503c01) AM_READ(lordgun_gun_0_y_r)
	AM_RANGE(0x503e00, 0x503e01) AM_READ(lordgun_gun_1_y_r)
	AM_RANGE(0x504000, 0x504001) AM_WRITE(lordgun_soundlatch_w)
	AM_RANGE(0x506000, 0x506007) AM_DEVREADWRITE8_LEGACY("ppi8255_0", ppi8255_r, ppi8255_w, 0x00ff)
	AM_RANGE(0x508000, 0x508007) AM_DEVREADWRITE8_LEGACY("ppi8255_1", ppi8255_r, ppi8255_w, 0x00ff)
	AM_RANGE(0x50a900, 0x50a9ff) AM_RAM	// protection
ADDRESS_MAP_END


static ADDRESS_MAP_START( aliencha_map, AS_PROGRAM, 16, lordgun_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0x210000, 0x21ffff) AM_RAM AM_BASE(m_priority_ram)						// PRIORITY
	AM_RANGE(0x300000, 0x30ffff) AM_RAM_WRITE(lordgun_vram_0_w) AM_BASE(m_vram[0])	// BACKGROUND 1
	AM_RANGE(0x310000, 0x313fff) AM_RAM_WRITE(lordgun_vram_1_w) AM_BASE(m_vram[1])	// BACKGROUND 2
	AM_RANGE(0x314000, 0x314fff) AM_RAM_WRITE(lordgun_vram_2_w) AM_BASE(m_vram[2])	// BACKGROUND 3
	AM_RANGE(0x315000, 0x317fff) AM_RAM														//
	AM_RANGE(0x318000, 0x319fff) AM_RAM_WRITE(lordgun_vram_3_w) AM_BASE(m_vram[3])	// TEXT
	AM_RANGE(0x31c000, 0x31c7ff) AM_RAM AM_BASE(m_scrollram)							// LINE OFFSET
	AM_RANGE(0x400000, 0x4007ff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)						// ANIMATE
	AM_RANGE(0x500000, 0x500fff) AM_RAM_WRITE(lordgun_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0x502000, 0x502001) AM_WRITEONLY AM_BASE(m_scroll_x[0])
	AM_RANGE(0x502200, 0x502201) AM_WRITEONLY AM_BASE(m_scroll_x[1])
	AM_RANGE(0x502400, 0x502401) AM_WRITEONLY AM_BASE(m_scroll_x[2])
	AM_RANGE(0x502600, 0x502601) AM_WRITEONLY AM_BASE(m_scroll_x[3])
	AM_RANGE(0x502800, 0x502801) AM_WRITEONLY AM_BASE(m_scroll_y[0])
	AM_RANGE(0x502a00, 0x502a01) AM_WRITEONLY AM_BASE(m_scroll_y[1])
	AM_RANGE(0x502c00, 0x502c01) AM_WRITEONLY AM_BASE(m_scroll_y[2])
	AM_RANGE(0x502e00, 0x502e01) AM_WRITEONLY AM_BASE(m_scroll_y[3])
	AM_RANGE(0x503000, 0x503001) AM_WRITE(lordgun_priority_w)
	AM_RANGE(0x504000, 0x504001) AM_WRITE(lordgun_soundlatch_w)
	AM_RANGE(0x506000, 0x506007) AM_DEVREADWRITE8_LEGACY("ppi8255_0", ppi8255_r, ppi8255_w, 0x00ff)
	AM_RANGE(0x508000, 0x508007) AM_DEVREADWRITE8_LEGACY("ppi8255_1", ppi8255_r, ppi8255_w, 0x00ff)
	AM_RANGE(0x50b900, 0x50b9ff) AM_RAM	// protection
ADDRESS_MAP_END


/***************************************************************************

    Memory Maps - Sound

***************************************************************************/

static ADDRESS_MAP_START( lordgun_soundmem_map, AS_PROGRAM, 8, lordgun_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static WRITE8_DEVICE_HANDLER( lordgun_okibank_w )
{
	downcast<okim6295_device *>(device)->set_bank_base((data & 2) ? 0x40000 : 0);
	if (data & ~3)	logerror("%s: unknown okibank bits %02x\n", device->machine().describe_context(), data);
//  popmessage("OKI %x", data);
}

static ADDRESS_MAP_START( lordgun_soundio_map, AS_IO, 8, lordgun_state )
	AM_RANGE(0x1000, 0x1001) AM_DEVWRITE_LEGACY("ymsnd", ym3812_w )
	AM_RANGE(0x2000, 0x2000) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x3000, 0x3000) AM_READ(soundlatch2_r )
	AM_RANGE(0x4000, 0x4000) AM_READ(soundlatch_r )
	AM_RANGE(0x5000, 0x5000) AM_READNOP
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE_LEGACY("oki", lordgun_okibank_w )
ADDRESS_MAP_END


static ADDRESS_MAP_START( aliencha_soundio_map, AS_IO, 8, lordgun_state )
	AM_RANGE(0x3000, 0x3000) AM_READ(soundlatch2_r )
	AM_RANGE(0x4000, 0x4000) AM_READ(soundlatch_r )
	AM_RANGE(0x5000, 0x5000) AM_WRITENOP	// writes 03 then 07 at end of NMI
	AM_RANGE(0x7000, 0x7000) AM_DEVREAD_LEGACY("ymf", ymf278b_r)
	AM_RANGE(0x7000, 0x7005) AM_DEVWRITE_LEGACY("ymf", ymf278b_w)
	AM_RANGE(0x7400, 0x7400) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x7800, 0x7800) AM_DEVREADWRITE("oki2", okim6295_device, read, write)
ADDRESS_MAP_END


/***************************************************************************

    Graphics Layout

***************************************************************************/

static const gfx_layout lordgun_8x8x6_layout =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{	RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0,
		RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0,
		RGN_FRAC(0,3)+8, RGN_FRAC(0,3)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	8*8*2
};

static const gfx_layout lordgun_16x16x6_layout =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{	RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0,
		RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0,
		RGN_FRAC(0,3)+8, RGN_FRAC(0,3)+0 },
	{ STEP8(0,1),STEP8(8*16*2,1) },
	{ STEP16(0,8*2) },
	16*16*2
};

static const gfx_layout lordgun_32x32x6_layout =
{
	32,32,
	RGN_FRAC(1,3),
	6,
	{	RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0,
		RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0,
		RGN_FRAC(0,3)+8, RGN_FRAC(0,3)+0 },
	{ STEP8(0,1),STEP8(8*32*2,1),STEP8(8*32*2*2,1),STEP8(8*32*2*3,1) },
	{ STEP16(0,8*2),STEP16(16*8*2,8*2) },
	32*32*2
};

static GFXDECODE_START( lordgun )
	GFXDECODE_ENTRY( "tiles0",  0, lordgun_8x8x6_layout,    0x000, 0x800/0x40*8  )	// [0] Tilemap 0
	GFXDECODE_ENTRY( "tiles1",  0, lordgun_16x16x6_layout,  0x000, 0x800/0x40*8  )	// [1] Tilemap 1
	GFXDECODE_ENTRY( "tiles1",  0, lordgun_32x32x6_layout,  0x000, 0x800/0x40*8  )	// [2] Tilemap 2
	GFXDECODE_ENTRY( "tiles0",  0, lordgun_8x8x6_layout,    0x000, 0x800/0x40*8  )	// [3] Tilemap 3
	GFXDECODE_ENTRY( "sprites", 0, lordgun_16x16x6_layout,  0x000, 0x800/0x40*8  )	// [4] Sprites
GFXDECODE_END


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( lordgun )
	PORT_START("DIP")
	PORT_DIPNAME( 0x01, 0x01, "Stage Select" )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Guns" )		PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, "IGS" )     // x table offset  = 0x25
	PORT_DIPSETTING(    0x00, "Konami" )  // "" = 0x2c
	PORT_DIPNAME( 0x04, 0x04, "Ranking Music" )	PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "Exciting" )
	PORT_DIPSETTING(    0x00, "Tender" )
	PORT_DIPNAME( 0x08, 0x08, "Coin Slots" )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_device, read_bit)

	PORT_START("START1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1   )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE2 ) // game cheat: skip stage

	PORT_START("START2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(5)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2   ) PORT_IMPULSE(5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("LIGHT0_X")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x3c,0x1d8) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x3c,0x1d8) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0,224) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0,224) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( aliencha )
	PORT_START("DIP1")
	PORT_DIPNAME( 0x01, 0x01, "Credits To Start" )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )		PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("DIP2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPNAME( 0x08, 0x08, "Round Time" )		PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, "32 s" )  // 40 s (measured) AKA "Short"
	PORT_DIPSETTING(    0x08, "40 s" )  // 50 s (measured) AKA "Normal"
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) )	PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Allow Join" )		PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DIP3")
	PORT_DIPNAME( 0x03, 0x03, "Buttons" )			PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "6" )
//  PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, "Vs. Rounds" )		PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW3:4" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW3:5" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW3:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW3:7" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW3:8" ) /* Listed as "Unused" */

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_device, read_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1         )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2         )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(2)

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1          )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2          )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5        ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6        ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5        ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6        ) PORT_PLAYER(2)
INPUT_PORTS_END


/***************************************************************************

    Machine Drivers

***************************************************************************/

static const ppi8255_interface lordgun_ppi8255_intf[2] =
{
	{
		DEVCB_INPUT_PORT("DIP"),		// Port A read
		DEVCB_NULL,						// Port B read
		DEVCB_INPUT_PORT("SERVICE"),	// Port C read
		DEVCB_HANDLER(fake_w),			// Port A write
		DEVCB_HANDLER(lordgun_eeprom_w),// Port B write
		DEVCB_HANDLER(fake2_w)			// Port C write
	},
	{
		DEVCB_INPUT_PORT("START1"),		// Port A read
		DEVCB_INPUT_PORT("START2"),		// Port B read
		DEVCB_INPUT_PORT("COIN"),		// Port C read
		DEVCB_HANDLER(fake_w),			// Port A write
		DEVCB_HANDLER(fake_w),			// Port B write
		DEVCB_HANDLER(fake_w)			// Port C write
	}
};


static const ppi8255_interface aliencha_ppi8255_intf[2] =
{
	{
		DEVCB_HANDLER(aliencha_dip_r),		// Port A read
		DEVCB_NULL,							// Port B read
		DEVCB_INPUT_PORT("SERVICE"),		// Port C read
		DEVCB_HANDLER(fake2_w),				// Port A write
		DEVCB_HANDLER(aliencha_eeprom_w),	// Port B write
		DEVCB_HANDLER(aliencha_dip_w)		// Port C write
	},
	{
		DEVCB_INPUT_PORT("P1"),				// Port A read
		DEVCB_INPUT_PORT("P2"),				// Port B read
		DEVCB_INPUT_PORT("COIN"),			// Port C read
		DEVCB_HANDLER(fake_w),				// Port A write
		DEVCB_HANDLER(fake_w),				// Port B write
		DEVCB_HANDLER(fake_w)				// Port C write
	}
};

static void soundirq(device_t *device, int state)
{
	cputag_set_input_line(device->machine(), "soundcpu", INPUT_LINE_IRQ0, state);
}

static const ym3812_interface lordgun_ym3812_interface =
{
	soundirq
};

static MACHINE_CONFIG_START( lordgun, lordgun_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_20MHz / 2)
	MCFG_CPU_PROGRAM_MAP(lordgun_map)
	MCFG_CPU_VBLANK_INT("screen", irq4_line_hold)

	MCFG_CPU_ADD("soundcpu", Z80, XTAL_20MHz / 4)
	MCFG_CPU_PROGRAM_MAP(lordgun_soundmem_map)
	MCFG_CPU_IO_MAP(lordgun_soundio_map)

	MCFG_PPI8255_ADD( "ppi8255_0", lordgun_ppi8255_intf[0] )
	MCFG_PPI8255_ADD( "ppi8255_1", lordgun_ppi8255_intf[1] )

	MCFG_EEPROM_93C46_ADD("eeprom")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(0x200, 0x100)
	MCFG_SCREEN_VISIBLE_AREA(0,0x1c0-1, 0,0xe0-1)
	MCFG_SCREEN_UPDATE_STATIC(lordgun)

	MCFG_GFXDECODE(lordgun)
	MCFG_PALETTE_LENGTH(0x800 * 8)	// 0x800 real colors, repeated per priority level

	MCFG_VIDEO_START(lordgun)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_3_579545MHz)
	MCFG_SOUND_CONFIG(lordgun_ym3812_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_20MHz / 20, OKIM6295_PIN7_HIGH)	// ? 5MHz can't be right!
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static const ymf278b_interface ymf278b_config =
{
	soundirq
};

static MACHINE_CONFIG_START( aliencha, lordgun_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_20MHz / 2)
	MCFG_CPU_PROGRAM_MAP(aliencha_map)
	MCFG_CPU_VBLANK_INT("screen", irq4_line_hold)

	MCFG_CPU_ADD("soundcpu", Z80, XTAL_20MHz / 4)
	MCFG_CPU_PROGRAM_MAP(lordgun_soundmem_map)
	MCFG_CPU_IO_MAP(aliencha_soundio_map)

	MCFG_PPI8255_ADD( "ppi8255_0", aliencha_ppi8255_intf[0] )
	MCFG_PPI8255_ADD( "ppi8255_1", aliencha_ppi8255_intf[1] )

	MCFG_EEPROM_93C46_ADD("eeprom")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(0x200, 0x100)
	MCFG_SCREEN_VISIBLE_AREA(0,0x1c0-1, 0,0xe0-1)
	MCFG_SCREEN_UPDATE_STATIC(lordgun)

	MCFG_GFXDECODE(lordgun)
	MCFG_PALETTE_LENGTH(0x800 * 8)	// 0x800 real colors, repeated per priority level

	MCFG_VIDEO_START(lordgun)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymf", YMF278B, 26000000)			// ? 26MHz matches video (decrease for faster music tempo)
	MCFG_SOUND_CONFIG(ymf278b_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_OKIM6295_ADD("oki", XTAL_20MHz / 20, OKIM6295_PIN7_HIGH)	// ? 5MHz can't be right
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki2", XTAL_20MHz / 20, OKIM6295_PIN7_HIGH)	// ? 5MHz can't be right
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/***************************************************************************

    ROMs Loading

***************************************************************************/

/***************************************************************************

Lord of Gun
IGS, 1994

PCB Layout
----------

IGSPCB NO. T0076
--------------------------------------------------------
| YM3014           62256      IGS008  IGS006   IGST003 |
| YM3812      6295 62256                       IGST002 |
|       3.57945MHz 62256                       IGST001 |
|                  62256                               |
|6116 LORDGUN.100                              IGSB003 |
|     Z80               62256                  IGSB002 |
|LORDGUN.90                                    IGSB001 |
|J    PAL              6116                            |
|A    PAL              6116                       6116 |
|M                          IGS003                6116 |
|M   68000P10 PAL                                 6116 |
|A                          PAL     PAL           6116 |
|                           PAL     6116               |
|                           PAL     6116        IGS007 |
|                           PAL     6116         20MHz |
|       DSW1(4)                     6116 PAL           |
|             62256    62256          IGSA001 IGSA004  |
|      8255          LORDGUN.10       IGSA002 IGSA005  |
|93C46 8255          LORDGUN.4        IGSA003 IGSA006  |
--------------------------------------------------------

HW Notes:
      68k clock: 10.000MHz
      Z80 clock: 5.000MHz
          VSync: 60Hz
          HSync: 15.15kHz
   YM3812 clock: 3.57945MHz
 OKI 6295 clock: 5.000MHz
  OKI 6295 pin7: HI

  All frequencies are checked with my frequency counter (i.e. they are not guessed)

  IGST* are 8M devices
  IGSA* and IGSB* are 16M devices
  LORDGUN.90 is 27C512
  LORDGUN.100 \
  LORDGUN.10  | 27C040
  LORDGUN.4   /

-----

Lord of Gun (c) 1994 IGS

PCB: IGSPCB NO.T0076

  Main: MC68000P10 10MHz
   Sub: Zilog Z0840006PCS (Z80 6MHz)
 Sound: OKI M6295, Yamaha YM3812-F + Y3014B-F
   OSC: 20.000 MHz, Unmarked OSC for sound chips
EEPROM: NMC 9346N

1 Push Button - Test/Setup Mode

Custom chips:
IGS 005 (144 Pin PQFP)
IGS 006 (144 Pin PQFP)
IGS 007 (144 Pin PQFP)
IGS 008 (160 Pin PQFP)

lg_u122.m3 - Labelled as "LORD GUN U122-M3" MX 27C4000
lg_u144.m3 - Labelled as "LORD GUN U144-M3" MX 27C4000

lordgun.u90  - Labelled as "LORD GUN U90"  27C512
lordgunu.100 - Labelled as "LORD GUN U100" MX 27C4000

Surface mounted ROMs (42 pin DIP)

2 Unmarked ROM(?) chips

IGS A001
IGS A002
IGS A003
IGS A004
IGS A005
IGS A006

IGS B001
IGS B002
IGS B003

IGS T001
IGS T002
IGS T003

DIP Switch-1 (4 Position DIP)
--------------------------------------------------
    DipSwitch Title   | Function | 1 | 2 | 3 | 4 |
--------------------------------------------------
       Game Mode      |  Arcade  |off|           |
                      |  Street  |on |           |
--------------------------------------------------
       Selection      |   IGS    |   |off|       |
        of Guns       |  Konami  |   |on |       |
--------------------------------------------------
       Ranking        | Exciting |       |off|   |
      Background      |  Tender  |       |on |   |
--------------------------------------------------
      Coin Slots      | Separate |           |off|
                      |  Common  |           |on |
--------------------------------------------------
     Settings Upon Shipping      |off|off|off|off|
--------------------------------------------------

Game modes explained:
 In "Arcade Mode" players could play this game by entering each scene in a
  pre-defined order.
 In "Street Mode" this game now presents 10 selectable scenes for players,
  not 4 any more.  After all scenes are passed (except training courses),
  players can enter the last scene; the Headquarters

                       Lord of Gun JAMMA Pinout

                        Main Jamma Connector
          Solder Side            |             Parts Side
------------------------------------------------------------------
             GND             | A | 1 |             GND
             GND             | B | 2 |             GND
             +5              | C | 3 |             +5
             +5              | D | 4 |             +5
                             | E | 5 |
             +12             | F | 6 |             +12
------------ KEY ------------| H | 7 |------------ KEY -----------
                             | J | 8 |       Coin Counter
                             | K | 9 |
        Speaker (-)          | L | 10|        Speaker (+)
                             | M | 11|
        Video Green          | N | 12|        Video Red
        Video Sync           | P | 13|        Video Blue
                             | R | 14|        Video GND
                             | S | 15|        Test Switch
        Coin Switch 2        | T | 16|        Coin Switch 1
        Start Player 2       | U | 17|        Start Player 1
                             | V | 18|
                             | W | 19|
                             | X | 20|
                             | Y | 21|
                             | Z | 22|
                             | a | 23|
                             | b | 24|
                             | c | 25|
                             | d | 26|
             GND             | e | 27|             GND
             GND             | f | 28|             GND


NOTE: Speakers should be connected serially to Speaker (+) and Speaker (-).
      You must avoid connecting speakers parallelly or connecting speakers
      to Speaker (+) and GND, to keep the amplifier from being damaged or
      from malfunctioning.

 JP1: Player 1 Gun Connector Pinout

   1| +5 Volts - RED Wire    (Manual says "VCC")
   2| Trigger  - White Wire
   3| Ground   - Black Wire
   4| Gun OPTO - Blue Wire   (Manual says "HIT")

 JP2: Player 2 Gun Connector Pinout

   1| +5 Volts - RED Wire    (Manual says "VCC")
   2| Trigger  - White Wire
   3| Ground   - Black Wire
   4| Gun OPTO - Blue Wire   (Manual says "HIT")

***************************************************************************/

ROM_START( lordgun )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "lordgun.10", 0x00000, 0x80000, CRC(acda77ef) SHA1(7cd8580419e2f62a3b5a1e4a6020a3ef978ff1e8) )
	ROM_LOAD16_BYTE( "lordgun.4",  0x00001, 0x80000, CRC(a1a61254) SHA1(b0c5aa656024cfb9be28a11061656159e7b72d00) )

	ROM_REGION( 0x010000, "soundcpu", 0 ) // Z80
	ROM_LOAD( "lordgun.90", 0x00000, 0x10000, CRC(d59b5e28) SHA1(36696058684d69306f463ed543c8b0195bafa21e) )	// 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x300000, "tiles0", 0 )	// Tilemaps 0 & 3
	ROM_LOAD( "igst001.108", 0x000000, 0x100000, CRC(36dd96f3) SHA1(4e70eb807160e7ed1b19d7f38df3a38021f42d9b) )
	ROM_LOAD( "igst002.114", 0x100000, 0x100000, CRC(816a7665) SHA1(f2f2624ab262c957f84c657cfc432d14c61b19e8) )
	ROM_LOAD( "igst003.119", 0x200000, 0x100000, CRC(cbfee543) SHA1(6fad8ef8d683f709f6ff2b16319447516c372fc8) )

	ROM_REGION( 0x600000, "tiles1", 0 )	// Tilemaps 1 & 2
	ROM_LOAD( "igsb001.82", 0x000000, 0x200000, CRC(3096de1c) SHA1(d010990d21cfda9cb8ab5b4bc0e329c23b7719f5) )
	ROM_LOAD( "igsb002.91", 0x200000, 0x200000, CRC(2234531e) SHA1(58a82e31a1c0c1a4dd026576319f4e7ecffd140e) )
	ROM_LOAD( "igsb003.97", 0x400000, 0x200000, CRC(6cbf21ac) SHA1(ad25090a00f291aa48929ffa01347cc53e0051f8) )

	ROM_REGION( 0xc00000, "sprites", 0 )	// Sprites
	ROM_LOAD( "igsa001.14", 0x000000, 0x200000, CRC(400abe33) SHA1(20de1eb626424ea41bd55eb3cecd6b50be744ee0) )
	ROM_LOAD( "igsa004.13", 0x200000, 0x200000, CRC(52687264) SHA1(28444cf6b5662054e283992857e0827a2ca15b83) )
	ROM_LOAD( "igsa002.9",  0x400000, 0x200000, CRC(a4810e38) SHA1(c31fe641feab2c93795fc35bf71d4f37af1056d4) )
	ROM_LOAD( "igsa005.8",  0x600000, 0x200000, CRC(e32e79e3) SHA1(419f9b501e5a37d763ece9322271e61035b50217) )
	ROM_LOAD( "igsa003.3",  0x800000, 0x200000, CRC(649e48d9) SHA1(ce346154024cf13f3e40000ceeb4c2003cd35894) )
	ROM_LOAD( "igsa006.2",  0xa00000, 0x200000, CRC(39288eb6) SHA1(54d157f0e151f6665f4288b4d09bd65571005132) )

	ROM_REGION( 0x080000, "oki", 0 ) // Samples
	ROM_LOAD( "lordgun.100", 0x00000, 0x80000, CRC(b4e0fa07) SHA1(f5f33fe3f3a124f4737751fda3ea409fceeec0be) )
ROM_END


/***************************************************************************

  Alien Challenge (World)
  (C) 1994 IGS
  01/16/95 21:51:28 in test mode

***************************************************************************/

ROM_START( aliencha )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000
	ROM_LOAD( "igsc0102.u81", 0x00000, 0x200000, CRC(e3432be3) SHA1(d3597c885571d4a996afaaf29c78da123798371e) )

	ROM_REGION( 0x010000, "soundcpu", 0 ) // Z80
	ROM_LOAD( "hfh_s.u86", 0x00000, 0x10000, CRC(5728a9ed) SHA1(e5a9e4a1a2cc6c848b08608bc8727bc739270873) )

	ROM_REGION( 0x300000, "tiles0", 0 )	// Tilemaps 0 & 3
	ROM_LOAD( "igst0101.u9",  0x000000, 0x100000, CRC(2ce12d7b) SHA1(aa93a82e5f4015c46bb705efb2051b62cd5d7e04) )
	ROM_LOAD( "igst0102.u10", 0x100000, 0x100000, CRC(542a76a0) SHA1(6947b50a024d0053c1eaf9da8c90652bab875142) )
	ROM_LOAD( "igst0103.u11", 0x200000, 0x100000, CRC(adf5698a) SHA1(4b798f8acc5d7581c7e0989260863ae0ca654acd) )

	ROM_REGION( 0x600000, "tiles1", 0 )	// Tilemaps 1 & 2
	ROM_LOAD( "igsb0101.u8", 0x000000, 0x200000, CRC(5c995f7e) SHA1(4f08cf13e313c6802c924b914c73cab4b450da61) )
	ROM_LOAD( "igsb0102.u7", 0x200000, 0x200000, CRC(a2ae9baf) SHA1(338ee260c33448568f138ca00e1d4edda4da018f) )
	ROM_LOAD( "igsb0103.u6", 0x400000, 0x200000, CRC(11b927af) SHA1(2f15e5cea1b86cde3b679bdd0f3d79672d0ddd3e) )

	ROM_REGION( 0xc00000, "sprites", 0 ) // Sprites
	ROM_LOAD( "igsa0101.u3", 0x000000, 0x400000, CRC(374d07c4) SHA1(87e9bfe32cbfe9964ba7253847fbd14aa3c8ed20) )
	ROM_LOAD( "igsa0102.u2", 0x400000, 0x400000, CRC(dbeee7ac) SHA1(e0eb0d73d9230aa6f69f5ac25d44fa19affebe88) )
	ROM_LOAD( "igsa0103.u1", 0x800000, 0x400000, CRC(e5f19041) SHA1(c92a29bbbcb9a1f63364c665e3e0f9679add4389) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "hfh_g.u65", 0x00000, 0x40000, CRC(ec469b57) SHA1(ba1668078987ad51f47bcd3e61c51a0cf2545350) )

	ROM_REGION( 0x40000, "oki2", 0 ) // Samples
	ROM_LOAD( "hfh_g.u66", 0x00000, 0x40000, CRC(7cfcd98e) SHA1(3b03123160adfd3404a9e0c4c68420930e80ae48) )

	ROM_REGION( 0x200000, "ymf", 0 ) // Samples (Standard Yamaha YRW801 2MB samples ROM)
	ROM_LOAD( "yrw801-m", 0x000000, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca) ) /* Not dumped from PCB, but is a standard samples rom */
ROM_END

/***************************************************************************

  Alien Challenge (China)
  (C) 1994 IGS
  12/13/94 13:55:47 in test mode

***************************************************************************/

ROM_START( alienchac )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000
	// load the world version code in the top half, or it does not work. Are these program roms half size?
	ROM_LOAD( "igsc0102.u81",     0x00000, 0x200000, BAD_DUMP CRC(e3432be3) SHA1(d3597c885571d4a996afaaf29c78da123798371e) )
	ROM_LOAD16_BYTE( "hfh_p.u80", 0x00000, 0x080000, BAD_DUMP CRC(5175ebdc) SHA1(4a0bdda0f8291f895f888bfd45328b2b124b9051) )
	ROM_LOAD16_BYTE( "hfh_p.u79", 0x00001, 0x080000, BAD_DUMP CRC(42ad978c) SHA1(eccb96e7170902b37989c8f207e1a821f29b2475) )

	ROM_REGION( 0x010000, "soundcpu", 0 ) // Z80
	ROM_LOAD( "hfh_s.u86", 0x00000, 0x10000, CRC(5728a9ed) SHA1(e5a9e4a1a2cc6c848b08608bc8727bc739270873) )

	ROM_REGION( 0x300000, "tiles0", 0 )	// Tilemaps 0 & 3
	ROM_LOAD( "igst0101.u9",  0x000000, 0x100000, BAD_DUMP CRC(2ce12d7b) SHA1(aa93a82e5f4015c46bb705efb2051b62cd5d7e04) ) /* Graphics ROMs not confirmed to be the same */
	ROM_LOAD( "igst0102.u10", 0x100000, 0x100000, BAD_DUMP CRC(542a76a0) SHA1(6947b50a024d0053c1eaf9da8c90652bab875142) ) /* Use these until roms are dumped / verified */
	ROM_LOAD( "igst0103.u11", 0x200000, 0x100000, BAD_DUMP CRC(adf5698a) SHA1(4b798f8acc5d7581c7e0989260863ae0ca654acd) )

	ROM_REGION( 0x600000, "tiles1", 0 )	// Tilemaps 1 & 2
	ROM_LOAD( "igsb0101.u8", 0x000000, 0x200000, BAD_DUMP CRC(5c995f7e) SHA1(4f08cf13e313c6802c924b914c73cab4b450da61) ) /* Graphics ROMs not confirmed to be the same */
	ROM_LOAD( "igsb0102.u7", 0x200000, 0x200000, BAD_DUMP CRC(a2ae9baf) SHA1(338ee260c33448568f138ca00e1d4edda4da018f) ) /* Use these until roms are dumped / verified */
	ROM_LOAD( "igsb0103.u6", 0x400000, 0x200000, BAD_DUMP CRC(11b927af) SHA1(2f15e5cea1b86cde3b679bdd0f3d79672d0ddd3e) )

	ROM_REGION( 0xc00000, "sprites", 0 ) // Sprites
	ROM_LOAD( "igsa0101.u3", 0x000000, 0x400000, BAD_DUMP CRC(374d07c4) SHA1(87e9bfe32cbfe9964ba7253847fbd14aa3c8ed20) ) /* Graphics ROMs not confirmed to be the same */
	ROM_LOAD( "igsa0102.u2", 0x400000, 0x400000, BAD_DUMP CRC(dbeee7ac) SHA1(e0eb0d73d9230aa6f69f5ac25d44fa19affebe88) ) /* Use these until roms are dumped / verified */
	ROM_LOAD( "igsa0103.u1", 0x800000, 0x400000, BAD_DUMP CRC(e5f19041) SHA1(c92a29bbbcb9a1f63364c665e3e0f9679add4389) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "hfh_g.u65", 0x00000, 0x40000, CRC(ec469b57) SHA1(ba1668078987ad51f47bcd3e61c51a0cf2545350) )

	ROM_REGION( 0x40000, "oki2", 0 ) // Samples
	ROM_LOAD( "hfh_g.u66", 0x00000, 0x40000, CRC(7cfcd98e) SHA1(3b03123160adfd3404a9e0c4c68420930e80ae48) )

	ROM_REGION( 0x200000, "ymf", 0 ) // Samples (Standard Yamaha YRW801 2MB samples ROM)
	ROM_LOAD( "yrw801-m", 0x000000, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca) ) /* Not dumped from PCB, but is a standard samples rom */
ROM_END


/***************************************************************************

    Game Drivers

***************************************************************************/

GAME( 1994, lordgun,   0,        lordgun,  lordgun,  lordgun,   ROT0, "IGS", "Lord of Gun (USA)",       GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_GRAPHICS )
GAME( 1994, aliencha,  0,        aliencha, aliencha, aliencha,  ROT0, "IGS", "Alien Challenge (World)", GAME_UNEMULATED_PROTECTION )
GAME( 1994, alienchac, aliencha, aliencha, aliencha, alienchac, ROT0, "IGS", "Alien Challenge (China)", GAME_UNEMULATED_PROTECTION )
