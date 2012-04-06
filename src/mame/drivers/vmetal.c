/*

Varia Metal
Excellent System Ltd, 1995

PCB Layout
----------
This game runs on Metro hardware.

ES-9309B-B
|--------------------------------------------|
| TA7222   8.U9     DSW1(8)  DSW2(8)         |
|VOL    M6295  1.000MHz                      |
|                   |------------|           |
|          7.U12    |   68000    |           |
|  uPC3403          |------------|           |
|J 640kHz  ES-8712                           |
|A M6585           EPM7032    6B.U18  5B.U19 |
|M       MM1035                              |
|M        26.666MHz  16MHz    62256   62256  |
|A                                           |
|                 |--------|          1.U29  |
|         62256   |Imagetek|                 |
|         62256   |14220   |          2.U31  |
|                 |        |                 |
|                 |--------|          3.U28  |
|                                            |
|                  6264               4.U30  |
|--------------------------------------------|
Notes:
      68000   - clock 16.000MHz
      ES-8712 - ES-8712 Single Channel ADPCM Samples Player. Clock ?? seems to be 16kHz?
                This chip is branded 'EXCELLENT', may be (or not??) manufactured by Ensonic (SDIP48)
      M6295   - clock 1.000MHz. Sample rate = 1000000/132
      M6585   - Oki M6585 ADPCM Voice Synthesizer IC (DIP18). Clock 640kHz.
                Sample rate = 16kHz (selection - pin 1 LOW, pin 2 HIGH = 16kHz)
                This is a version-up to the previous M5205 with some additional
                capabilies and improvements.
      MM1035  - Mitsumi Monolithic IC MM1035 System Reset and Watchdog Timer (DIP8)
      uPC3403 - NEC uPC3403 High Performance Quad Operational Amplifier (DIP14)
      62256   - 32k x8 SRAM (DIP28)
      6264    - 8k x8 SRAM (DIP28)
      TA7222  - Toshiba TA7222 5.8 Watt Audio Power Amplifier (SIP10)
      EPM7032 - Altera EPM7032LC44-15T High Performance EEPROM-based Programmable Logic Device (PLCC44)
      Custom  - Imagetek 14220 Graphics Controller (QFP208)
      VSync   - 58.2328Hz
      HSync   - 15.32kHz
      ROMs    -
                6B & 5B are 27C040 EPROM (DIP32)
                8 is 4M MaskROM (DIP32)
                All other ROMs are 16M MaskROM (DIP42)




Varia Metal

Notes:

*****
i should fully merge video with metro.c, it uses the same imagetek chip (although with 16x16 tiles)
this should fix most of the remaining gfx glitches - looks very similar to 'taidoa' (stephh)
*****


It has Sega and Taito logos in the roms ?!

ES8712 sound may not be quite right. Samples are currently looped, but
whether they should and how, is unknown.

cleanup


*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/es8712.h"
#include "includes/metro.h"

class vmetal_state : public metro_state
{
public:
	vmetal_state(const machine_config &mconfig, device_type type, const char *tag)
		: metro_state(mconfig, type, tag) { }

	UINT16 *m_texttileram;
	UINT16 *m_mid1tileram;
	UINT16 *m_mid2tileram;
	UINT16 *m_tlookup;
	UINT16 *m_vmetal_videoregs;

	tilemap_t *m_texttilemap;
	tilemap_t *m_mid1tilemap;
	tilemap_t *m_mid2tilemap;
	DECLARE_READ16_MEMBER(varia_crom_read);
	DECLARE_WRITE16_MEMBER(vmetal_texttileram_w);
	DECLARE_WRITE16_MEMBER(vmetal_mid1tileram_w);
	DECLARE_WRITE16_MEMBER(vmetal_mid2tileram_w);
	DECLARE_READ16_MEMBER(varia_dips_bit8_r);
	DECLARE_READ16_MEMBER(varia_dips_bit7_r);
	DECLARE_READ16_MEMBER(varia_dips_bit6_r);
	DECLARE_READ16_MEMBER(varia_dips_bit5_r);
	DECLARE_READ16_MEMBER(varia_dips_bit4_r);
	DECLARE_READ16_MEMBER(varia_dips_bit3_r);
	DECLARE_READ16_MEMBER(varia_dips_bit2_r);
	DECLARE_READ16_MEMBER(varia_dips_bit1_r);
};


READ16_MEMBER(vmetal_state::varia_crom_read)
{
	/* game reads the cgrom, result is 7772, verified to be correct on the real board */


	UINT8 *cgrom = machine().region("gfx1")->base();
	UINT16 retdat;

	offset = offset << 1;
	offset |= (m_vmetal_videoregs[0x0ab / 2] & 0x7f) << 16;
	retdat = ((cgrom[offset] << 8) | (cgrom[offset + 1]));
	// popmessage("varia romread offset %06x data %04x", offset, retdat);

	return retdat;
}


static void get_vmetal_tlookup(running_machine &machine, UINT16 data, UINT16 *tileno, UINT16 *color)
{
	vmetal_state *state = machine.driver_data<vmetal_state>();
	int idx = ((data & 0x7fff) >> 4) * 2;
	UINT32 lookup = (state->m_tlookup[idx] << 16) | state->m_tlookup[idx + 1];

	*tileno = (data & 0xf) | ((lookup >> 2) & 0xfff0);
	*color = (lookup >> 20) & 0xff;
}


WRITE16_MEMBER(vmetal_state::vmetal_texttileram_w)
{

	COMBINE_DATA(&m_texttileram[offset]);
	m_texttilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(vmetal_state::vmetal_mid1tileram_w)
{

	COMBINE_DATA(&m_mid1tileram[offset]);
	m_mid1tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(vmetal_state::vmetal_mid2tileram_w)
{

	COMBINE_DATA(&m_mid2tileram[offset]);
	m_mid2tilemap->mark_tile_dirty(offset);
}


READ16_MEMBER(vmetal_state::varia_dips_bit8_r){ return ((input_port_read(machine(), "DSW2") & 0x80) << 0) | ((input_port_read(machine(), "DSW1") & 0x80) >> 1); }
READ16_MEMBER(vmetal_state::varia_dips_bit7_r){ return ((input_port_read(machine(), "DSW2") & 0x40) << 1) | ((input_port_read(machine(), "DSW1") & 0x40) >> 0); }
READ16_MEMBER(vmetal_state::varia_dips_bit6_r){ return ((input_port_read(machine(), "DSW2") & 0x20) << 2) | ((input_port_read(machine(), "DSW1") & 0x20) << 1); }
READ16_MEMBER(vmetal_state::varia_dips_bit5_r){ return ((input_port_read(machine(), "DSW2") & 0x10) << 3) | ((input_port_read(machine(), "DSW1") & 0x10) << 2); }
READ16_MEMBER(vmetal_state::varia_dips_bit4_r){ return ((input_port_read(machine(), "DSW2") & 0x08) << 4) | ((input_port_read(machine(), "DSW1") & 0x08) << 3); }
READ16_MEMBER(vmetal_state::varia_dips_bit3_r){ return ((input_port_read(machine(), "DSW2") & 0x04) << 5) | ((input_port_read(machine(), "DSW1") & 0x04) << 4); }
READ16_MEMBER(vmetal_state::varia_dips_bit2_r){ return ((input_port_read(machine(), "DSW2") & 0x02) << 6) | ((input_port_read(machine(), "DSW1") & 0x02) << 5); }
READ16_MEMBER(vmetal_state::varia_dips_bit1_r){ return ((input_port_read(machine(), "DSW2") & 0x01) << 7) | ((input_port_read(machine(), "DSW1") & 0x01) << 6); }

static WRITE8_DEVICE_HANDLER( vmetal_control_w )
{
	/* Lower nibble is the coin control bits shown in
       service mode, but in game mode they're different */
	coin_counter_w(device->machine(), 0, data & 0x04);
	coin_counter_w(device->machine(), 1, data & 0x08);	/* 2nd coin schute activates coin 0 counter in game mode?? */
//  coin_lockout_w(device->machine(), 0, data & 0x01);  /* always on in game mode?? */
	coin_lockout_w(device->machine(), 1, data & 0x02);	/* never activated in game mode?? */

	if ((data & 0x40) == 0)
		device->reset();
	else
		es8712_play(device);

	if (data & 0x10)
		es8712_set_bank_base(device, 0x100000);
	else
		es8712_set_bank_base(device, 0x000000);

	if (data & 0xa0)
		logerror("%s:Writing unknown bits %04x to $200000\n",device->machine().describe_context(),data);
}

static WRITE8_DEVICE_HANDLER( vmetal_es8712_w )
{
	/* Many samples in the ADPCM ROM are actually not used.

    Snd         Offset Writes                 Sample Range
         0000 0004 0002 0006 000a 0008 000c
    --   ----------------------------------   -------------
    00   006e 0001 00ab 003c 0002 003a 003a   01ab6e-023a3c
    01   003d 0002 003a 001d 0002 007e 007e   023a3d-027e1d
    02   00e2 0003 0005 002e 0003 00f3 00f3   0305e2-03f32e
    03   000a 0005 001e 00f6 0005 00ec 00ec   051e0a-05ecf6
    04   00f7 0005 00ec 008d 0006 0060 0060   05ecf7-06608d
    05   0016 0008 002e 0014 0009 0019 0019   082e16-091914
    06   0015 0009 0019 0094 000b 0015 0015   091915-0b1594
    07   0010 000d 0012 00bf 000d 0035 0035   0d1210-0d35bf
    08   00ce 000e 002f 0074 000f 0032 0032   0e2fce-0f3274
    09   0000 0000 0000 003a 0000 007d 007d   000000-007d3a
    0a   0077 0000 00fa 008d 0001 00b6 00b6   00fa77-01b68d
    0b   008e 0001 00b6 00b3 0002 0021 0021   01b68e-0221b3
    0c   0062 0002 00f7 0038 0003 00de 00de   02f762-03de38
    0d   00b9 0005 00ab 00ef 0006 0016 0016   05abb9-0616ef
    0e   00dd 0007 0058 00db 0008 001a 001a   0758dd-081adb
    0f   00dc 0008 001a 002e 0008 008a 008a   081adc-088a2e
    10   00db 0009 00d7 00ff 000a 0046 0046   09d7db-0a46ff
    11   0077 000c 0003 006d 000c 0080 0080   0c0377-0c806d
    12   006e 000c 0080 006c 000d 0002 0002   0c806e-0d026c
    13   006d 000d 0002 002b 000d 0041 0041   0d026d-0d412b
    14   002c 000d 0041 002a 000d 00be 00be   0d412c-0dbe2a
    15   002b 000d 00be 0029 000e 0083 0083   0dbe2b-0e8329
    16   002a 000e 0083 00ee 000f 0069 0069   0e832a-0f69ee
    */

	es8712_w(device, offset, data);
	logerror("%s:Writing %04x to ES8712 offset %02x\n", device->machine().describe_context(), data, offset);
}


static ADDRESS_MAP_START( varia_program_map, AS_PROGRAM, 16, vmetal_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x11ffff) AM_RAM_WRITE(vmetal_texttileram_w) AM_BASE(m_texttileram)
	AM_RANGE(0x120000, 0x13ffff) AM_RAM_WRITE(vmetal_mid1tileram_w) AM_BASE(m_mid1tileram)
	AM_RANGE(0x140000, 0x15ffff) AM_RAM_WRITE(vmetal_mid2tileram_w) AM_BASE(m_mid2tileram)

	AM_RANGE(0x160000, 0x16ffff) AM_READ(varia_crom_read) // cgrom read window ..

	AM_RANGE(0x170000, 0x173fff) AM_RAM_WRITE(paletteram16_GGGGGRRRRRBBBBBx_word_w) AM_SHARE("paletteram")	// Palette
	AM_RANGE(0x174000, 0x174fff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)
	AM_RANGE(0x175000, 0x177fff) AM_RAM
	AM_RANGE(0x178000, 0x1787ff) AM_RAM AM_BASE(m_tlookup)
	AM_RANGE(0x178800, 0x1796ff) AM_RAM AM_BASE(m_vmetal_videoregs)
	AM_RANGE(0x179700, 0x179713) AM_WRITEONLY AM_BASE(m_videoregs)	// Metro sprite chip Video Registers

	AM_RANGE(0x200000, 0x200001) AM_READ_PORT("P1_P2") AM_DEVWRITE8_LEGACY("essnd", vmetal_control_w, 0x00ff)
	AM_RANGE(0x200002, 0x200003) AM_READ_PORT("SYSTEM")

	/* same weird way to read Dip Switches as in many games in metro.c driver - use balcube_dsw_r read handler once the driver is merged */
	AM_RANGE(0x30fffe, 0x30ffff) AM_READNOP					// 0x40 = dip1-16 -> 0xff0086 (doesn't exist in this game : address is NEVER read back)
	AM_RANGE(0x317ffe, 0x317fff) AM_READNOP					// 0x40 = dip1-15 -> 0xff0086 (doesn't exist in this game : address is NEVER read back)
	AM_RANGE(0x31bffe, 0x31bfff) AM_READNOP					// 0x40 = dip1-14 -> 0xff0086 (doesn't exist in this game : address is NEVER read back)
	AM_RANGE(0x31dffe, 0x31dfff) AM_READNOP					// 0x40 = dip1-13 -> 0xff0086 (doesn't exist in this game : address is NEVER read back)
	AM_RANGE(0x31effe, 0x31efff) AM_READNOP					// 0x40 = dip1-12 -> 0xff0086 (doesn't exist in this game : address is NEVER read back)
	AM_RANGE(0x31f7fe, 0x31f7ff) AM_READNOP					// 0x40 = dip1-11 -> 0xff0086 (doesn't exist in this game : address is NEVER read back)
	AM_RANGE(0x31fbfe, 0x31fbff) AM_READNOP					// 0x40 = dip1-10 -> 0xff0086 (doesn't exist in this game : address is NEVER read back)
	AM_RANGE(0x31fdfe, 0x31fdff) AM_READNOP					// 0x40 = dip1-9  -> 0xff0086 (doesn't exist in this game : address is NEVER read back)
	AM_RANGE(0x31fefe, 0x31feff) AM_READ(varia_dips_bit8_r)	// 0x40 = dip1-8  -> 0xff0085 , 0x80 = dip2-8 -> 0xff0084
	AM_RANGE(0x31ff7e, 0x31ff7f) AM_READ(varia_dips_bit7_r)	// 0x40 = dip1-7  -> 0xff0085 , 0x80 = dip2-7 -> 0xff0084
	AM_RANGE(0x31ffbe, 0x31ffbf) AM_READ(varia_dips_bit6_r)	// 0x40 = dip1-6  -> 0xff0085 , 0x80 = dip2-6 -> 0xff0084
	AM_RANGE(0x31ffde, 0x31ffdf) AM_READ(varia_dips_bit5_r)	// 0x40 = dip1-5  -> 0xff0085 , 0x80 = dip2-5 -> 0xff0084
	AM_RANGE(0x31ffee, 0x31ffef) AM_READ(varia_dips_bit4_r)	// 0x40 = dip1-4  -> 0xff0085 , 0x80 = dip2-4 -> 0xff0084
	AM_RANGE(0x31fff6, 0x31fff7) AM_READ(varia_dips_bit3_r)	// 0x40 = dip1-3  -> 0xff0085 , 0x80 = dip2-3 -> 0xff0084
	AM_RANGE(0x31fffa, 0x31fffb) AM_READ(varia_dips_bit2_r)	// 0x40 = dip1-2  -> 0xff0085 , 0x80 = dip2-2 -> 0xff0084
	AM_RANGE(0x31fffc, 0x31fffd) AM_READ(varia_dips_bit1_r)	// 0x40 = dip1-1  -> 0xff0085 , 0x80 = dip2-1 -> 0xff0084

	AM_RANGE(0x400000, 0x400001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff )
	AM_RANGE(0x400002, 0x400003) AM_DEVWRITE8("oki", okim6295_device, write, 0x00ff)	// Volume/channel info
	AM_RANGE(0x500000, 0x50000d) AM_DEVWRITE8_LEGACY("essnd", vmetal_es8712_w, 0x00ff)

	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END


/* verified from M68000 code */
static INPUT_PORTS_START( varia )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_TILT )             /* 'Tilt' only in "test mode" - no effect ingame */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )         /* same coinage as COIN1 and COIN2 */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2 )         /* 'Test' only in "test mode" - no effect ingame */
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	/* stored to 0xff0085.b (cpl'ed) */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C )  )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C )  )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C )  )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_2C )  )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_3C )  )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_4C )  )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_5C )  )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C )  )
	PORT_DIPUNUSED( 0x0008, IP_ACTIVE_LOW )                 /* 0x01 (OFF) or 0x02 (ON) written to 0xff0112.b but NEVER read back - old credits for 2 players game ? */
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) )  /* 0x07c1 written to 0x1788ac.w (screen control ?) at first (code at 0x0001b8) */
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )          /* 0x07c1 written to 0xff0114.w (then 0x1788ac.w) during initialisation (code at 0x000436) */
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )           /* 0x07c0 written to 0xff0114.w (then 0x1788ac.w) during initialisation (code at 0x000436) */
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x0040, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x0080, IP_ACTIVE_LOW )

	/* stored to 0xff0084.b (cpl'ed) */
	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0008, "1"  )
	PORT_DIPSETTING(      0x0004, "2"  )
	PORT_DIPSETTING(      0x000c, "3"  )
	PORT_DIPSETTING(      0x0000, "4"  )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Bonus_Life ) )   /* code at 0x0004a4 */
	PORT_DIPSETTING(      0x0010, "Every 30000" )
	PORT_DIPSETTING(      0x0000, "Every 60000" )
	PORT_DIPUNUSED( 0x0020, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x0040, IP_ACTIVE_LOW )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )
INPUT_PORTS_END



static const gfx_layout char16x16layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24, 36, 32, 44, 40, 52, 48, 60, 56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	16*64
};

static const gfx_layout char8x8layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24 },
	{ 0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32 },
	8*32
};

static GFXDECODE_START( vmetal )
	GFXDECODE_ENTRY( "gfx1", 0, char16x16layout,   0x1000, 512  ) /* bg tiles */
	GFXDECODE_ENTRY( "gfx1", 0, char8x8layout,   0x1000, 512  ) /* bg tiles */
GFXDECODE_END


static TILE_GET_INFO( get_vmetal_texttilemap_tile_info )
{
	vmetal_state *state = machine.driver_data<vmetal_state>();
	UINT32 tile;
	UINT16 color, data = state->m_texttileram[tile_index];
	int idx = ((data & 0x7fff) >> 4) * 2;
	UINT32 lookup = (state->m_tlookup[idx] << 16) | state->m_tlookup[idx + 1];

	tile = (data & 0xf) | (lookup & 0x7fff0);
	color = ((lookup >> 20) & 0x1f) + 0xe0;

	if (data & 0x8000)
		tile = 0;

	SET_TILE_INFO(1, tile, color, TILE_FLIPYX(0x0));
}


static TILE_GET_INFO( get_vmetal_mid1tilemap_tile_info )
{
	vmetal_state *state = machine.driver_data<vmetal_state>();
	UINT16 tile, color, data = state->m_mid1tileram[tile_index];

	get_vmetal_tlookup(machine, data, &tile, &color);

	if (data & 0x8000)
		tile = 0;

	SET_TILE_INFO(0, tile, color, TILE_FLIPYX(0x0));
}

static TILE_GET_INFO( get_vmetal_mid2tilemap_tile_info )
{
	vmetal_state *state = machine.driver_data<vmetal_state>();
	UINT16 tile, color, data = state->m_mid2tileram[tile_index];

	get_vmetal_tlookup(machine, data, &tile, &color);

	if (data & 0x8000)
		tile = 0;

	SET_TILE_INFO(0, tile, color, TILE_FLIPYX(0x0));
}

static void expand_gfx1(running_machine &machine)
{
	metro_state *state = machine.driver_data<metro_state>();
	UINT8 *base_gfx = machine.region("gfx1")->base();
	UINT32 length = 2 * machine.region("gfx1")->bytes();
	state->m_expanded_gfx1 = auto_alloc_array(machine, UINT8, length);
	for (int i = 0; i < length; i += 2)
	{
		UINT8 src = base_gfx[i / 2];
		state->m_expanded_gfx1[i+0] = src & 15;
		state->m_expanded_gfx1[i+1] = src >> 4;
	}
}

static VIDEO_START(varia)
{
	vmetal_state *state = machine.driver_data<vmetal_state>();

	state->m_texttilemap = tilemap_create(machine, get_vmetal_texttilemap_tile_info, tilemap_scan_rows,  8,  8, 256, 256);
	state->m_mid1tilemap = tilemap_create(machine, get_vmetal_mid1tilemap_tile_info, tilemap_scan_rows, 16, 16, 256, 256);
	state->m_mid2tilemap = tilemap_create(machine, get_vmetal_mid2tilemap_tile_info, tilemap_scan_rows, 16, 16, 256, 256);

	state->m_texttilemap->set_transparent_pen(15);
	state->m_mid1tilemap->set_transparent_pen(15);
	state->m_mid2tilemap->set_transparent_pen(15);

	expand_gfx1(machine);
}

static SCREEN_UPDATE_IND16(varia)
{
	vmetal_state *state = screen.machine().driver_data<vmetal_state>();

	bitmap.fill(get_black_pen(screen.machine()), cliprect);
	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_mid2tilemap->set_scrollx(0, state->m_vmetal_videoregs[0x06a/2]-64 /*+ state->m_vmetal_videoregs[0x066/2]*/);
	state->m_mid1tilemap->set_scrollx(0, state->m_vmetal_videoregs[0x07a/2]-64 /*+ state->m_vmetal_videoregs[0x076/2]*/);
	state->m_texttilemap->set_scrollx(0, -64 /*+ state->m_vmetal_videoregs[0x076/2]*/);

	state->m_mid2tilemap->set_scrolly(0, -64);
	state->m_mid1tilemap->set_scrolly(0, -64);
	state->m_texttilemap->set_scrolly(0, -64);

	state->m_mid1tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_mid2tilemap->draw(bitmap, cliprect, 0, 0);
	metro_draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_texttilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}


static MACHINE_CONFIG_START( varia, vmetal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 16000000)
	MCFG_CPU_PROGRAM_MAP(varia_program_map)
	MCFG_CPU_VBLANK_INT("screen", irq1_line_hold) // also level 3

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(2048, 2048)
	MCFG_SCREEN_VISIBLE_AREA(0+64, 319+64, 0+64, 223+64)
	MCFG_SCREEN_UPDATE_STATIC(varia)

	MCFG_GFXDECODE(vmetal)
	MCFG_PALETTE_LENGTH(0x4000)

	MCFG_VIDEO_START(varia)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", 1320000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.75)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.75)

	MCFG_SOUND_ADD("essnd", ES8712, 12000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END


ROM_START( vmetal )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "5b.u19", 0x00001, 0x80000, CRC(4933ac6c) SHA1(1a3303e32fcb08854d4d6e13f36ca99d92aed4cc) )
	ROM_LOAD16_BYTE( "6b.u18", 0x00000, 0x80000, CRC(4eb939d5) SHA1(741ab05043fc3bd886162d878630e45da9359718) )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROMX_LOAD( "1.u29", 0x000004, 0x200000, CRC(b470c168) SHA1(c30462dc134da1e71a94b36ef96ecd65c325b07e) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "2.u31", 0x000000, 0x200000, CRC(b36f8d60) SHA1(1676859d0fee4eb9897ce1601a2c9fd9a6dc4a43) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "3.u28", 0x000006, 0x200000, CRC(00fca765) SHA1(ca9010bd7f59367e483868018db9a9abf871386e) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "4.u30", 0x000002, 0x200000, CRC(5a25a49c) SHA1(c30781202ec882e1ec6adfb560b0a1075b3cce55) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x080000, "oki", 0 ) /* OKI6295 Samples */
	/* Second half is junk */
	ROM_LOAD( "8.u9", 0x00000, 0x80000, CRC(c14c001c) SHA1(bad96b5cd40d1c34ef8b702262168ecab8192fb6) )

	ROM_REGION( 0x200000, "essnd", 0 ) /* Samples */
	ROM_LOAD( "7.u12", 0x00000, 0x200000, CRC(a88c52f1) SHA1(d74a5a11f84ba6b1042b33a2c156a1071b6fbfe1) )
ROM_END

ROM_START( vmetaln )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "vm5.bin", 0x00001, 0x80000, CRC(43ef844e) SHA1(c673f34fcc9e406282c9008795b52d01a240099a) )
	ROM_LOAD16_BYTE( "vm6.bin", 0x00000, 0x80000, CRC(cb292ab1) SHA1(41fdfe67e6cb848542fd5aa0dfde3b1936bb3a28) )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROMX_LOAD( "1.u29", 0x000004, 0x200000, CRC(b470c168) SHA1(c30462dc134da1e71a94b36ef96ecd65c325b07e) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "2.u31", 0x000000, 0x200000, CRC(b36f8d60) SHA1(1676859d0fee4eb9897ce1601a2c9fd9a6dc4a43) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "3.u28", 0x000006, 0x200000, CRC(00fca765) SHA1(ca9010bd7f59367e483868018db9a9abf871386e) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "4.u30", 0x000002, 0x200000, CRC(5a25a49c) SHA1(c30781202ec882e1ec6adfb560b0a1075b3cce55) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x080000, "oki", 0 ) /* OKI6295 Samples */
	/* Second half is junk */
	ROM_LOAD( "8.u9", 0x00000, 0x80000, CRC(c14c001c) SHA1(bad96b5cd40d1c34ef8b702262168ecab8192fb6) )

	ROM_REGION( 0x200000, "essnd", 0 ) /* Samples */
	ROM_LOAD( "7.u12", 0x00000, 0x200000, CRC(a88c52f1) SHA1(d74a5a11f84ba6b1042b33a2c156a1071b6fbfe1) )
ROM_END


GAME( 1995, vmetal,  0,      varia, varia, 0, ROT270, "Excellent System",                                "Varia Metal",                        GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1995, vmetaln, vmetal, varia, varia, 0, ROT270, "Excellent System (New Ways Trading Co. license)", "Varia Metal (New Ways Trading Co.)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
