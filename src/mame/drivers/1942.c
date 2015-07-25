// license:???
// copyright-holders:Paul Leaman, Couriersud
/***************************************************************************

1942

driver by Paul Leaman


MAIN CPU:

0000-bfff ROM (8000-bfff banked)
cc00-cc7f Sprites
d000-d3ff Video RAM
d400-d7ff Color RAM
d800-dbff Background RAM (groups of 32 bytes, 16 code, 16 color/attribute)
e000-efff RAM

read:
c000      IN0
c001      IN1
c002      IN2
c003      DSW0
c004      DSW1

write:
c800      command for the audio CPU
c802-c803 background scroll
c804      bit 7: flip screen
          bit 4: cpu B reset
          bit 0: coin counter
c805      background palette bank selector
c806      bit 0-1 ROM bank selector 00=1-N5.BIN
                                    01=1-N6.BIN
                                    10=1-N7.BIN



SOUND CPU:

0000-3fff ROM
4000-47ff RAM
6000      command from the main CPU
8000      8910 #1 control
8001      8910 #1 write
c000      8910 #2 control
c001      8910 #2 write



Game runs in interrupt mode 0 (the devices supply the interrupt number).

Two interrupts must be triggered per refresh for the game to function
correctly.

0x10 is the video retrace. This controls the speed of the game and generally
     drives the code. This must be triggered for each video retrace.
0x08 is the sound card service interrupt. The game uses this to throw sounds
     at the sound CPU.


***************************************************************************/

#define MAIN_CPU_CLOCK      (XTAL_12MHz/3) /* 12MHz is the only OSC on the PCB */
#define SOUND_CPU_CLOCK     (XTAL_12MHz/4) /* 12MHz is the only OSC on the PCB */
#define AUDIO_CLOCK     (XTAL_12MHz/8) /* 12MHz is the only OSC on the PCB */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/netlist.h"
#include "netlist/devices/net_lib.h"

#include "includes/1942.h"

#define NLFILT(RA, R1, C1, R2) \
	NET_C(RA.1, V5)             \
	NET_C(RA.2, R1.1)           \
	NET_C(R1.2, GND)            \
	NET_C(R1.1, C1.1)           \
	NET_C(C1.2, R2.1)

static NETLIST_START(nl_1942)

	/* Standard stuff */

	SOLVER(Solver, 48000)
	ANALOG_INPUT(V5, 5)
	PARAM(Solver.ACCURACY, 1e-6)
	PARAM(Solver.GS_LOOPS, 6)
	PARAM(Solver.SOR_FACTOR, 1.0)
	//PARAM(Solver.DYNAMIC_TS, 1)
	//PARAM(Solver.LTE, 5e-8)

	/* AY 8910 internal resistors */

	RES(R_AY1_1, 1000);
	RES(R_AY1_2, 1000);
	RES(R_AY1_3, 1000);
	RES(R_AY2_1, 1000);
	RES(R_AY2_2, 1000);
	RES(R_AY2_3, 1000);

	RES(R2, 220000)
	RES(R3, 220000)
	RES(R4, 220000)
	RES(R5, 220000)
	RES(R6, 220000)
	RES(R7, 220000)

	RES(R11, 10000)
	RES(R12, 10000)
	RES(R13, 10000)
	RES(R14, 10000)
	RES(R15, 10000)
	RES(R16, 10000)

	CAP(CC7, 10e-6)
	CAP(CC8, 10e-6)
	CAP(CC9, 10e-6)
	CAP(CC10, 10e-6)
	CAP(CC11, 10e-6)
	CAP(CC12, 10e-6)

	NLFILT(R_AY2_3, R13, CC7, R2)
	NLFILT(R_AY2_2, R15, CC8, R3)
	NLFILT(R_AY2_1, R11, CC9, R4)

	NLFILT(R_AY1_3, R12, CC10, R5)
	NLFILT(R_AY1_2, R14, CC11, R6)
	NLFILT(R_AY1_1, R16, CC12, R7)

	POT(VR, 2000)
	NET_C(VR.3, GND)

	NET_C(R2.2, VR.1)
	NET_C(R3.2, VR.1)
	NET_C(R4.2, VR.1)
	NET_C(R5.2, VR.1)
	NET_C(R6.2, VR.1)
	NET_C(R7.2, VR.1)

	CAP(CC6, 10e-6)
	RES(R1, 100000)

	NET_C(CC6.1, VR.2)
	NET_C(CC6.2, R1.1)
	CAP(CC3, 220e-6)
	NET_C(R1.2, CC3.1)
	NET_C(CC3.2, GND)

NETLIST_END()

WRITE8_MEMBER(_1942_state::c1942_bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x03);
}

TIMER_DEVICE_CALLBACK_MEMBER(_1942_state::c1942_scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7);   /* RST 10h - vblank */

	if(scanline == 0) // unknown irq event, presumably vblank-in or a periodic one (writes to the soundlatch and drives freeze dip-switch)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xcf);   /* RST 08h */
}




static ADDRESS_MAP_START( c1942_map, AS_PROGRAM, 8, _1942_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xc001, 0xc001) AM_READ_PORT("P1")
	AM_RANGE(0xc002, 0xc002) AM_READ_PORT("P2")
	AM_RANGE(0xc003, 0xc003) AM_READ_PORT("DSWA")
	AM_RANGE(0xc004, 0xc004) AM_READ_PORT("DSWB")
	AM_RANGE(0xc800, 0xc800) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xc802, 0xc803) AM_WRITE(c1942_scroll_w)
	AM_RANGE(0xc804, 0xc804) AM_WRITE(c1942_c804_w)
	AM_RANGE(0xc805, 0xc805) AM_WRITE(c1942_palette_bank_w)
	AM_RANGE(0xc806, 0xc806) AM_WRITE(c1942_bankswitch_w)
	AM_RANGE(0xcc00, 0xcc7f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(c1942_fgvideoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xd800, 0xdbff) AM_RAM_WRITE(c1942_bgvideoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

WRITE8_MEMBER(_1942_state::c1942p_f600_w)
{
//  printf("c1942p_f600_w %02x\n", data);
}

WRITE8_MEMBER(_1942_state::c1942p_palette_w)
{
	m_protopal[offset] = data;

	int r = (data & 0x07) >> 0;
	int g = (data & 0x38) >> 3;
	int b = (data & 0xc0) >> 6;

	m_palette->set_indirect_color(offset, rgb_t(r<<5,g<<5,b<<6));
}

WRITE8_MEMBER(_1942_state::c1942p_soundlatch_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static ADDRESS_MAP_START( c1942p_map, AS_PROGRAM, 8, _1942_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")

	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(c1942_fgvideoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xd800, 0xdbff) AM_RAM_WRITE(c1942_bgvideoram_w) AM_SHARE("bg_videoram")

	AM_RANGE(0xe000, 0xefff) AM_RAM

	AM_RANGE(0xce00, 0xcfff) AM_RAM AM_SHARE("spriteram")

	AM_RANGE(0xdc02, 0xdc03) AM_WRITE(c1942_scroll_w)
	AM_RANGE(0xc804, 0xc804) AM_WRITE(c1942_c804_w)
	AM_RANGE(0xc805, 0xc805) AM_WRITE(c1942_palette_bank_w)

	AM_RANGE(0xf000, 0xf3ff) AM_RAM AM_WRITE(c1942p_palette_w)  AM_SHARE("protopal")

	AM_RANGE(0xf400, 0xf400) AM_WRITE(c1942_bankswitch_w)
	AM_RANGE(0xf500, 0xf500) AM_WRITE(c1942p_soundlatch_w)
	AM_RANGE(0xf600, 0xf600) AM_WRITE(c1942p_f600_w)

	AM_RANGE(0xf700, 0xf700) AM_READ_PORT("DSWA")
	AM_RANGE(0xf701, 0xf701) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xf702, 0xf702) AM_READ_PORT("DSWB")
	AM_RANGE(0xf703, 0xf703) AM_READ_PORT("P1")
	AM_RANGE(0xf704, 0xf704) AM_READ_PORT("P2")
ADDRESS_MAP_END


static ADDRESS_MAP_START(c1942p_sound_map, AS_PROGRAM, 8, _1942_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM

	AM_RANGE(0xc000, 0xc000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( c1942p_sound_io, AS_IO, 8, _1942_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0000, 0x0000) AM_WRITENOP
	AM_RANGE(0x0014, 0x0015) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x0018, 0x0019) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
ADDRESS_MAP_END



static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, _1942_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x8000, 0x8001) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0xc000, 0xc001) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( 1942 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:8,7,6")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWA:4,3")
	PORT_DIPSETTING(    0x30, "20K 80K 80K+" )
	PORT_DIPSETTING(    0x20, "20K 100K 100K+" )
	PORT_DIPSETTING(    0x10, "30K 80K 80K+" )
	PORT_DIPSETTING(    0x00, "30K 100K 100K+" )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:2,1")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_SERVICE_DIPLOC(0x08, IP_ACTIVE_LOW, "SWB:5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x80, "Screen Stop" )           PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( 1942p )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )


	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_DIPNAME( 0x0001, 0x0001, "B" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:8,7,6")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWA:4,3")
	PORT_DIPSETTING(    0x30, "20K 80K 80K+" )
	PORT_DIPSETTING(    0x20, "20K 100K 100K+" )
	PORT_DIPSETTING(    0x10, "30K 80K 80K+" )
	PORT_DIPSETTING(    0x00, "30K 100K 100K+" )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:2,1")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_SERVICE_DIPLOC(0x08, IP_ACTIVE_LOW, "SWB:5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x80, "Screen Stop" )           PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16*16+0, 16*16+1, 16*16+2, 16*16+3, 16*16+8+0, 16*16+8+1, 16*16+8+2, 16*16+8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static GFXDECODE_START( 1942 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,             0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,          64*4, 4*32 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 64*4+4*32*8, 16 )
GFXDECODE_END


static const gfx_layout charlayout_p =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tilelayout_p =
{
	16,16,
	RGN_FRAC(1,12),
	3,
	{ RGN_FRAC(1,3), RGN_FRAC(2,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			0x2000*8, 0x2000*8 +1, 0x2000*8 +2, 0x2000*8 +3, 0x2000*8 +4, 0x2000*8 +5, 0x2000*8 + 6, 0x2000*8 +7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,    0x1000*8 + 0*8, 0x1000*8 + 1*8, 0x1000*8 + 2*8,  0x1000*8 + 3*8,  0x1000*8 + 4*8,  0x1000*8 + 5*8,  0x1000*8 + 6*8,  0x1000*8 + 7*8     },
	8*8
};

static const gfx_layout spritelayout_p =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*16+0, 8*16+1, 8*16+2, 8*16+3, 8*16+4, 8*16+5, 8*16+6, 8*16+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8 , 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static GFXDECODE_START( 1942p )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_p,             0x000, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout_p,          0x300, 32 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout_p, 0x400, 16 )
GFXDECODE_END



void _1942_state::machine_start()
{
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_scroll));
}

void _1942_state::machine_reset()
{
	m_palette_bank = 0;
	m_scroll[0] = 0;
	m_scroll[1] = 0;
}

static MACHINE_CONFIG_START( 1942, _1942_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MAIN_CPU_CLOCK)    /* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(c1942_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", _1942_state, c1942_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, SOUND_CPU_CLOCK)  /* 3 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(_1942_state, irq0_line_hold, 4*60)


	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 1942)

	MCFG_PALETTE_ADD("palette", 64*4+4*32*8+16*16)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(_1942_state, 1942)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(_1942_state, screen_update_1942)
	MCFG_SCREEN_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, AUDIO_CLOCK)  /* 1.5 MHz */
	MCFG_AY8910_OUTPUT_TYPE(AY8910_RESISTOR_OUTPUT)
	MCFG_AY8910_RES_LOADS(10000.0, 10000.0, 10000.0)

	MCFG_SOUND_ROUTE_EX(0, "snd_nl", 1.0, 0)
	MCFG_SOUND_ROUTE_EX(1, "snd_nl", 1.0, 1)
	MCFG_SOUND_ROUTE_EX(2, "snd_nl", 1.0, 2)

	MCFG_SOUND_ADD("ay2", AY8910, AUDIO_CLOCK)  /* 1.5 MHz */
	MCFG_AY8910_OUTPUT_TYPE(AY8910_RESISTOR_OUTPUT)
	MCFG_AY8910_RES_LOADS(10000.0, 10000.0, 10000.0)

	MCFG_SOUND_ROUTE_EX(0, "snd_nl", 1.0, 3)
	MCFG_SOUND_ROUTE_EX(1, "snd_nl", 1.0, 4)
	MCFG_SOUND_ROUTE_EX(2, "snd_nl", 1.0, 5)

	/* NETLIST configuration using internal AY8910 resistor values */

	/* Minimize resampling between ay8910 and netlist */
	MCFG_SOUND_ADD("snd_nl", NETLIST_SOUND, AUDIO_CLOCK / 8 / 2)
	MCFG_NETLIST_SETUP(nl_1942)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 5.0)
	MCFG_NETLIST_STREAM_INPUT("snd_nl", 0, "R_AY1_1.R")
	MCFG_NETLIST_STREAM_INPUT("snd_nl", 1, "R_AY1_2.R")
	MCFG_NETLIST_STREAM_INPUT("snd_nl", 2, "R_AY1_3.R")
	MCFG_NETLIST_STREAM_INPUT("snd_nl", 3, "R_AY2_1.R")
	MCFG_NETLIST_STREAM_INPUT("snd_nl", 4, "R_AY2_2.R")
	MCFG_NETLIST_STREAM_INPUT("snd_nl", 5, "R_AY2_3.R")

	MCFG_NETLIST_STREAM_OUTPUT("snd_nl", 0, "R1.1")
	//MCFG_NETLIST_STREAM_OUTPUT("snd_nl", 0, "VR.2")
	MCFG_NETLIST_ANALOG_MULT_OFFSET(70000.0, 0.0)

MACHINE_CONFIG_END


static MACHINE_CONFIG_START( 1942p, _1942_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MAIN_CPU_CLOCK)    /* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(c1942p_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", _1942_state,  irq0_line_hold) // note, powerups won't move down the screen with the original '1942' logic.

	MCFG_CPU_ADD("audiocpu", Z80, SOUND_CPU_CLOCK)  /* 3 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(c1942p_sound_map)
	MCFG_CPU_IO_MAP(c1942p_sound_io)
	MCFG_CPU_PERIODIC_INT_DRIVER(_1942_state, irq0_line_hold, 4*60)


	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 1942p)
	MCFG_PALETTE_ADD("palette", 0x500)
	MCFG_PALETTE_INDIRECT_ENTRIES(0x400)
	MCFG_PALETTE_INIT_OWNER(_1942_state, 1942p)

	MCFG_VIDEO_START_OVERRIDE(_1942_state,c1942p)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(_1942_state, screen_update_1942p)
	MCFG_SCREEN_PALETTE("palette")


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, AUDIO_CLOCK)  /* 1.5 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("ay2", AY8910, AUDIO_CLOCK)  /* 1.5 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( 1942 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "srb-03.m3", 0x00000, 0x4000, CRC(d9dafcc3) SHA1(a089a9bc55fb7d6d0ac53f91b258396d5d62677a) )
	ROM_LOAD( "srb-04.m4", 0x04000, 0x4000, CRC(da0cf924) SHA1(856fbb302c9a4ec7850a26ab23dab8467f79bba4) )
	ROM_LOAD( "srb-05.m5", 0x10000, 0x4000, CRC(d102911c) SHA1(35ba1d82bd901940f61d8619273463d02fc0a952) )
	ROM_LOAD( "srb-06.m6", 0x14000, 0x2000, CRC(466f8248) SHA1(2ccc8fc59962d3001fbc10e8d2f20a254a74f251) )
	ROM_LOAD( "srb-07.m7", 0x18000, 0x4000, CRC(0d31038c) SHA1(b588eaf6fddd66ecb2d9832dc197f286f1ccd846) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sr-01.c11", 0x0000, 0x4000, CRC(bd87f06b) SHA1(821f85cf157f81117eeaba0c3cf0337eac357e58) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sr-02.f2", 0x0000, 0x2000, CRC(6ebca191) SHA1(0dbddadde54a0ab66994c4a8726be05c6ca88a0e) )    /* characters */

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "sr-08.a1", 0x0000, 0x2000, CRC(3884d9eb) SHA1(5cbd9215fa5ba5a61208b383700adc4428521aed) )    /* tiles */
	ROM_LOAD( "sr-09.a2", 0x2000, 0x2000, CRC(999cf6e0) SHA1(5b8b685038ec98b781908b92eb7fb9506db68544) )
	ROM_LOAD( "sr-10.a3", 0x4000, 0x2000, CRC(8edb273a) SHA1(85fdd4c690ed31e6396e3c16aa02140ee7ea2d61) )
	ROM_LOAD( "sr-11.a4", 0x6000, 0x2000, CRC(3a2726c3) SHA1(187c92ef591febdcbd1d42ab850e0cbb62c00873) )
	ROM_LOAD( "sr-12.a5", 0x8000, 0x2000, CRC(1bd3d8bb) SHA1(ef4dce605eb4dc8035985a415315ec61c21419c6) )
	ROM_LOAD( "sr-13.a6", 0xa000, 0x2000, CRC(658f02c4) SHA1(f087d69e49e38cf3107350cde18fcf85a8fa04f0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "sr-14.l1", 0x00000, 0x4000, CRC(2528bec6) SHA1(29f7719f18faad6bd1ec6735cc24e69168361470) )   /* sprites */
	ROM_LOAD( "sr-15.l2", 0x04000, 0x4000, CRC(f89287aa) SHA1(136fff6d2a4f48a488fc7c620213761459c3ada0) )
	ROM_LOAD( "sr-16.n1", 0x08000, 0x4000, CRC(024418f8) SHA1(145b8d5d6c8654cd090955a98f6dd8c8dbafe7c1) )
	ROM_LOAD( "sr-17.n2", 0x0c000, 0x4000, CRC(e2c7e489) SHA1(d4b5d575c021f58f6966df189df94e08c5b3621c) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "sb-5.e8",  0x0000, 0x0100, CRC(93ab8153) SHA1(a792f24e5c0c3c4a6b436102e7a98199f878ece1) )    /* red component */
	ROM_LOAD( "sb-6.e9",  0x0100, 0x0100, CRC(8ab44f7d) SHA1(f74680a6a987d74b3acb32e6396f20e127874149) )    /* green component */
	ROM_LOAD( "sb-7.e10", 0x0200, 0x0100, CRC(f4ade9a4) SHA1(62ad31d31d183cce213b03168daa035083b2f28e) )    /* blue component */
	ROM_LOAD( "sb-0.f1",  0x0300, 0x0100, CRC(6047d91b) SHA1(1ce025f9524c1033e48c5294ee7d360f8bfebe8d) )    /* char lookup table */
	ROM_LOAD( "sb-4.d6",  0x0400, 0x0100, CRC(4858968d) SHA1(20b5dbcaa1a4081b3139e7e2332d8fe3c9e55ed6) )    /* tile lookup table */
	ROM_LOAD( "sb-8.k3",  0x0500, 0x0100, CRC(f6fad943) SHA1(b0a24ea7805272e8ebf72a99b08907bc00d5f82f) )    /* sprite lookup table */
	ROM_LOAD( "sb-2.d1",  0x0600, 0x0100, CRC(8bb8b3df) SHA1(49de2819c4c92057fedcb20425282515d85829aa) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-3.d2",  0x0700, 0x0100, CRC(3b0c99af) SHA1(38f30ac1e48632634e409f328ee3051b987de7ad) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-1.k6",  0x0800, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    /* interrupt timing (not used) */
	ROM_LOAD( "sb-9.m11", 0x0900, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END

ROM_START( 1942a )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "sra-03.m3", 0x00000, 0x4000, CRC(40201bab) SHA1(4886c07a4602223c21419118e10aadce9c99fa5a) )
	ROM_LOAD( "sr-04.m4",  0x04000, 0x4000, CRC(a60ac644) SHA1(f37862db3cf5e6cc9ab3276f3bc45fd629fd70dd) )
	ROM_LOAD( "sr-05.m5",  0x10000, 0x4000, CRC(835f7b24) SHA1(24b66827f08c43fbf5b9517d638acdfc38e1b1e7) )
	ROM_LOAD( "sr-06.m6",  0x14000, 0x2000, CRC(821c6481) SHA1(06becb6bf8b4bde3a458098498eecad566a87711) )
	ROM_LOAD( "sr-07.m7",  0x18000, 0x4000, CRC(5df525e1) SHA1(70cd2910e2945db76bd6ebfa0ff09a5efadc2d0b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sr-01.c11", 0x0000, 0x4000, CRC(bd87f06b) SHA1(821f85cf157f81117eeaba0c3cf0337eac357e58) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sr-02.f2", 0x0000, 0x2000, CRC(6ebca191) SHA1(0dbddadde54a0ab66994c4a8726be05c6ca88a0e) )    /* characters */

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "sr-08.a1", 0x0000, 0x2000, CRC(3884d9eb) SHA1(5cbd9215fa5ba5a61208b383700adc4428521aed) )    /* tiles */
	ROM_LOAD( "sr-09.a2", 0x2000, 0x2000, CRC(999cf6e0) SHA1(5b8b685038ec98b781908b92eb7fb9506db68544) )
	ROM_LOAD( "sr-10.a3", 0x4000, 0x2000, CRC(8edb273a) SHA1(85fdd4c690ed31e6396e3c16aa02140ee7ea2d61) )
	ROM_LOAD( "sr-11.a4", 0x6000, 0x2000, CRC(3a2726c3) SHA1(187c92ef591febdcbd1d42ab850e0cbb62c00873) )
	ROM_LOAD( "sr-12.a5", 0x8000, 0x2000, CRC(1bd3d8bb) SHA1(ef4dce605eb4dc8035985a415315ec61c21419c6) )
	ROM_LOAD( "sr-13.a6", 0xa000, 0x2000, CRC(658f02c4) SHA1(f087d69e49e38cf3107350cde18fcf85a8fa04f0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "sr-14.l1", 0x00000, 0x4000, CRC(2528bec6) SHA1(29f7719f18faad6bd1ec6735cc24e69168361470) )   /* sprites */
	ROM_LOAD( "sr-15.l2", 0x04000, 0x4000, CRC(f89287aa) SHA1(136fff6d2a4f48a488fc7c620213761459c3ada0) )
	ROM_LOAD( "sr-16.n1", 0x08000, 0x4000, CRC(024418f8) SHA1(145b8d5d6c8654cd090955a98f6dd8c8dbafe7c1) )
	ROM_LOAD( "sr-17.n2", 0x0c000, 0x4000, CRC(e2c7e489) SHA1(d4b5d575c021f58f6966df189df94e08c5b3621c) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "sb-5.e8",  0x0000, 0x0100, CRC(93ab8153) SHA1(a792f24e5c0c3c4a6b436102e7a98199f878ece1) )    /* red component */
	ROM_LOAD( "sb-6.e9",  0x0100, 0x0100, CRC(8ab44f7d) SHA1(f74680a6a987d74b3acb32e6396f20e127874149) )    /* green component */
	ROM_LOAD( "sb-7.e10", 0x0200, 0x0100, CRC(f4ade9a4) SHA1(62ad31d31d183cce213b03168daa035083b2f28e) )    /* blue component */
	ROM_LOAD( "sb-0.f1",  0x0300, 0x0100, CRC(6047d91b) SHA1(1ce025f9524c1033e48c5294ee7d360f8bfebe8d) )    /* char lookup table */
	ROM_LOAD( "sb-4.d6",  0x0400, 0x0100, CRC(4858968d) SHA1(20b5dbcaa1a4081b3139e7e2332d8fe3c9e55ed6) )    /* tile lookup table */
	ROM_LOAD( "sb-8.k3",  0x0500, 0x0100, CRC(f6fad943) SHA1(b0a24ea7805272e8ebf72a99b08907bc00d5f82f) )    /* sprite lookup table */
	ROM_LOAD( "sb-2.d1",  0x0600, 0x0100, CRC(8bb8b3df) SHA1(49de2819c4c92057fedcb20425282515d85829aa) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-3.d2",  0x0700, 0x0100, CRC(3b0c99af) SHA1(38f30ac1e48632634e409f328ee3051b987de7ad) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-1.k6",  0x0800, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    /* interrupt timing (not used) */
	ROM_LOAD( "sb-9.m11", 0x0900, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END

/* this is the same as the 1942a set, but with a different rom arrangement (larger roms), it appears to be a common bootleg */
ROM_START( 1942abl )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "3.bin", 0x00000, 0x8000, CRC(f3184f5a) SHA1(a566c344ee1f63580d41aca95ece9ad1f7a135d2) )
	ROM_LOAD( "5.bin", 0x10000, 0x4000, CRC(835f7b24) SHA1(24b66827f08c43fbf5b9517d638acdfc38e1b1e7) )
	ROM_LOAD( "7.bin", 0x14000, 0x8000, CRC(2f456c6e) SHA1(b728c72f97ccdb57a4aac53ef7ca3f4516fc2ecb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1.bin", 0x0000, 0x4000, CRC(bd87f06b) SHA1(821f85cf157f81117eeaba0c3cf0337eac357e58) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x2000, CRC(6ebca191) SHA1(0dbddadde54a0ab66994c4a8726be05c6ca88a0e) )   /* characters */

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "9.bin",  0x0000, 0x4000, CRC(60329fa4) SHA1(8f66c283c992a6bc676f5f0f739b7e9d07bbf9ee) )  /* tiles */
	ROM_LOAD( "11.bin", 0x4000, 0x4000, CRC(66bac116) SHA1(ce21a693ad8d7592d21e05d0cb9eabb36e7e8fef) )
	ROM_LOAD( "13.bin", 0x8000, 0x4000, CRC(623fcec1) SHA1(b3eea37d705e3871dc94e4cf6f2aacc6fbd09216) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "14.bin", 0x04000, 0x4000, CRC(df2345ef) SHA1(3776edebda7bc9c72117f4b764f3bdaec0a632b4) ) /* sprites */
	ROM_CONTINUE(0x0000,0x4000)
	ROM_LOAD( "16.bin", 0x0c000, 0x4000, CRC(c106b1ed) SHA1(a16520752fb02e403c93975ecf12b75854d58d69) )
	ROM_CONTINUE(0x8000,0x4000)

	// proms not in the set, assumed to be the same
	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "sb-5.e8",  0x0000, 0x0100, CRC(93ab8153) SHA1(a792f24e5c0c3c4a6b436102e7a98199f878ece1) )    /* red component */
	ROM_LOAD( "sb-6.e9",  0x0100, 0x0100, CRC(8ab44f7d) SHA1(f74680a6a987d74b3acb32e6396f20e127874149) )    /* green component */
	ROM_LOAD( "sb-7.e10", 0x0200, 0x0100, CRC(f4ade9a4) SHA1(62ad31d31d183cce213b03168daa035083b2f28e) )    /* blue component */
	ROM_LOAD( "sb-0.f1",  0x0300, 0x0100, CRC(6047d91b) SHA1(1ce025f9524c1033e48c5294ee7d360f8bfebe8d) )    /* char lookup table */
	ROM_LOAD( "sb-4.d6",  0x0400, 0x0100, CRC(4858968d) SHA1(20b5dbcaa1a4081b3139e7e2332d8fe3c9e55ed6) )    /* tile lookup table */
	ROM_LOAD( "sb-8.k3",  0x0500, 0x0100, CRC(f6fad943) SHA1(b0a24ea7805272e8ebf72a99b08907bc00d5f82f) )    /* sprite lookup table */
	ROM_LOAD( "sb-2.d1",  0x0600, 0x0100, CRC(8bb8b3df) SHA1(49de2819c4c92057fedcb20425282515d85829aa) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-3.d2",  0x0700, 0x0100, CRC(3b0c99af) SHA1(38f30ac1e48632634e409f328ee3051b987de7ad) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-1.k6",  0x0800, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    /* interrupt timing (not used) */
	ROM_LOAD( "sb-9.m11", 0x0900, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END

/* set contained only three program ROMs, other ROMs should be checked against a real PCB */
ROM_START( 1942h )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "42-3.bin",  0x00000, 0x4000, CRC(ec70785f) SHA1(2010a945e1d5c984a14cf7f47a883d04bd71567d) )
	ROM_LOAD( "42-4.bin",  0x04000, 0x4000, CRC(cc11355f) SHA1(44fceb449f406f657494eeee4e6b43bf063f2013) )
	ROM_LOAD( "42-5.bin",  0x10000, 0x4000, CRC(42746d75) SHA1(ede6919b84653b94fddeb40b3004e44336880ba2) )
	ROM_LOAD( "srb-06.m6", 0x14000, 0x2000, CRC(466f8248) SHA1(2ccc8fc59962d3001fbc10e8d2f20a254a74f251) )
	ROM_LOAD( "srb-07.m7", 0x18000, 0x4000, CRC(0d31038c) SHA1(b588eaf6fddd66ecb2d9832dc197f286f1ccd846) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sr-01.c11", 0x0000, 0x4000, CRC(bd87f06b) SHA1(821f85cf157f81117eeaba0c3cf0337eac357e58) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sr-02.f2", 0x0000, 0x2000, CRC(6ebca191) SHA1(0dbddadde54a0ab66994c4a8726be05c6ca88a0e) )    /* characters */

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "sr-08.a1", 0x0000, 0x2000, CRC(3884d9eb) SHA1(5cbd9215fa5ba5a61208b383700adc4428521aed) )    /* tiles */
	ROM_LOAD( "sr-09.a2", 0x2000, 0x2000, CRC(999cf6e0) SHA1(5b8b685038ec98b781908b92eb7fb9506db68544) )
	ROM_LOAD( "sr-10.a3", 0x4000, 0x2000, CRC(8edb273a) SHA1(85fdd4c690ed31e6396e3c16aa02140ee7ea2d61) )
	ROM_LOAD( "sr-11.a4", 0x6000, 0x2000, CRC(3a2726c3) SHA1(187c92ef591febdcbd1d42ab850e0cbb62c00873) )
	ROM_LOAD( "sr-12.a5", 0x8000, 0x2000, CRC(1bd3d8bb) SHA1(ef4dce605eb4dc8035985a415315ec61c21419c6) )
	ROM_LOAD( "sr-13.a6", 0xa000, 0x2000, CRC(658f02c4) SHA1(f087d69e49e38cf3107350cde18fcf85a8fa04f0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "sr-14.l1", 0x00000, 0x4000, CRC(2528bec6) SHA1(29f7719f18faad6bd1ec6735cc24e69168361470) )   /* sprites */
	ROM_LOAD( "sr-15.l2", 0x04000, 0x4000, CRC(f89287aa) SHA1(136fff6d2a4f48a488fc7c620213761459c3ada0) )
	ROM_LOAD( "sr-16.n1", 0x08000, 0x4000, CRC(024418f8) SHA1(145b8d5d6c8654cd090955a98f6dd8c8dbafe7c1) )
	ROM_LOAD( "sr-17.n2", 0x0c000, 0x4000, CRC(e2c7e489) SHA1(d4b5d575c021f58f6966df189df94e08c5b3621c) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "sb-5.e8",  0x0000, 0x0100, CRC(93ab8153) SHA1(a792f24e5c0c3c4a6b436102e7a98199f878ece1) )    /* red component */
	ROM_LOAD( "sb-6.e9",  0x0100, 0x0100, CRC(8ab44f7d) SHA1(f74680a6a987d74b3acb32e6396f20e127874149) )    /* green component */
	ROM_LOAD( "sb-7.e10", 0x0200, 0x0100, CRC(f4ade9a4) SHA1(62ad31d31d183cce213b03168daa035083b2f28e) )    /* blue component */
	ROM_LOAD( "sb-0.f1",  0x0300, 0x0100, CRC(6047d91b) SHA1(1ce025f9524c1033e48c5294ee7d360f8bfebe8d) )    /* char lookup table */
	ROM_LOAD( "sb-4.d6",  0x0400, 0x0100, CRC(4858968d) SHA1(20b5dbcaa1a4081b3139e7e2332d8fe3c9e55ed6) )    /* tile lookup table */
	ROM_LOAD( "sb-8.k3",  0x0500, 0x0100, CRC(f6fad943) SHA1(b0a24ea7805272e8ebf72a99b08907bc00d5f82f) )    /* sprite lookup table */
	ROM_LOAD( "sb-2.d1",  0x0600, 0x0100, CRC(8bb8b3df) SHA1(49de2819c4c92057fedcb20425282515d85829aa) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-3.d2",  0x0700, 0x0100, CRC(3b0c99af) SHA1(38f30ac1e48632634e409f328ee3051b987de7ad) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-1.k6",  0x0800, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    /* interrupt timing (not used) */
	ROM_LOAD( "sb-9.m11", 0x0900, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END

ROM_START( 1942b )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "sr-03.m3", 0x00000, 0x4000, CRC(612975f2) SHA1(f3744335862dd4c53925cc32792badd4a378c837) )
	ROM_LOAD( "sr-04.m4", 0x04000, 0x4000, CRC(a60ac644) SHA1(f37862db3cf5e6cc9ab3276f3bc45fd629fd70dd) )
	ROM_LOAD( "sr-05.m5", 0x10000, 0x4000, CRC(835f7b24) SHA1(24b66827f08c43fbf5b9517d638acdfc38e1b1e7) )
	ROM_LOAD( "sr-06.m6", 0x14000, 0x2000, CRC(821c6481) SHA1(06becb6bf8b4bde3a458098498eecad566a87711) )
	ROM_LOAD( "sr-07.m7", 0x18000, 0x4000, CRC(5df525e1) SHA1(70cd2910e2945db76bd6ebfa0ff09a5efadc2d0b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sr-01.c11", 0x0000, 0x4000, CRC(bd87f06b) SHA1(821f85cf157f81117eeaba0c3cf0337eac357e58) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sr-02.f2", 0x0000, 0x2000, CRC(6ebca191) SHA1(0dbddadde54a0ab66994c4a8726be05c6ca88a0e) )    /* characters */

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "sr-08.a1", 0x0000, 0x2000, CRC(3884d9eb) SHA1(5cbd9215fa5ba5a61208b383700adc4428521aed) )    /* tiles */
	ROM_LOAD( "sr-09.a2", 0x2000, 0x2000, CRC(999cf6e0) SHA1(5b8b685038ec98b781908b92eb7fb9506db68544) )
	ROM_LOAD( "sr-10.a3", 0x4000, 0x2000, CRC(8edb273a) SHA1(85fdd4c690ed31e6396e3c16aa02140ee7ea2d61) )
	ROM_LOAD( "sr-11.a4", 0x6000, 0x2000, CRC(3a2726c3) SHA1(187c92ef591febdcbd1d42ab850e0cbb62c00873) )
	ROM_LOAD( "sr-12.a5", 0x8000, 0x2000, CRC(1bd3d8bb) SHA1(ef4dce605eb4dc8035985a415315ec61c21419c6) )
	ROM_LOAD( "sr-13.a6", 0xa000, 0x2000, CRC(658f02c4) SHA1(f087d69e49e38cf3107350cde18fcf85a8fa04f0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "sr-14.l1", 0x00000, 0x4000, CRC(2528bec6) SHA1(29f7719f18faad6bd1ec6735cc24e69168361470) )   /* sprites */
	ROM_LOAD( "sr-15.l2", 0x04000, 0x4000, CRC(f89287aa) SHA1(136fff6d2a4f48a488fc7c620213761459c3ada0) )
	ROM_LOAD( "sr-16.n1", 0x08000, 0x4000, CRC(024418f8) SHA1(145b8d5d6c8654cd090955a98f6dd8c8dbafe7c1) )
	ROM_LOAD( "sr-17.n2", 0x0c000, 0x4000, CRC(e2c7e489) SHA1(d4b5d575c021f58f6966df189df94e08c5b3621c) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "sb-5.e8",  0x0000, 0x0100, CRC(93ab8153) SHA1(a792f24e5c0c3c4a6b436102e7a98199f878ece1) )    /* red component */
	ROM_LOAD( "sb-6.e9",  0x0100, 0x0100, CRC(8ab44f7d) SHA1(f74680a6a987d74b3acb32e6396f20e127874149) )    /* green component */
	ROM_LOAD( "sb-7.e10", 0x0200, 0x0100, CRC(f4ade9a4) SHA1(62ad31d31d183cce213b03168daa035083b2f28e) )    /* blue component */
	ROM_LOAD( "sb-0.f1",  0x0300, 0x0100, CRC(6047d91b) SHA1(1ce025f9524c1033e48c5294ee7d360f8bfebe8d) )    /* char lookup table */
	ROM_LOAD( "sb-4.d6",  0x0400, 0x0100, CRC(4858968d) SHA1(20b5dbcaa1a4081b3139e7e2332d8fe3c9e55ed6) )    /* tile lookup table */
	ROM_LOAD( "sb-8.k3",  0x0500, 0x0100, CRC(f6fad943) SHA1(b0a24ea7805272e8ebf72a99b08907bc00d5f82f) )    /* sprite lookup table */
	ROM_LOAD( "sb-2.d1",  0x0600, 0x0100, CRC(8bb8b3df) SHA1(49de2819c4c92057fedcb20425282515d85829aa) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-3.d2",  0x0700, 0x0100, CRC(3b0c99af) SHA1(38f30ac1e48632634e409f328ee3051b987de7ad) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-1.k6",  0x0800, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    /* interrupt timing (not used) */
	ROM_LOAD( "sb-9.m11", 0x0900, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END

ROM_START( 1942w )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "sw-03.m3", 0x00000, 0x4000, CRC(afd79770) SHA1(74c7a887fe3d4abfce1dcfec4c75b21ab81adc8c) )
	ROM_LOAD( "sw-04.m4", 0x04000, 0x4000, CRC(933d9910) SHA1(9c73ef880f56e30a865be959f8bbdbe79c7ef8e2) )
	ROM_LOAD( "sw-05.m5", 0x10000, 0x4000, CRC(e9a71bb6) SHA1(1f0d52c9282d15f9e4898b3b144ece25d345b71f) )
	ROM_LOAD( "sw-06.m6", 0x14000, 0x2000, CRC(466f8248) SHA1(2ccc8fc59962d3001fbc10e8d2f20a254a74f251) )   /* matches srb-06.m6 from 1942 (Revision B) */
	ROM_LOAD( "sw-07.m7", 0x18000, 0x4000, CRC(ec41655e) SHA1(dbe4bb11f2e88574cb43ba5cd216354c3b7f69a6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sr-01.c11", 0x0000, 0x4000, CRC(bd87f06b) SHA1(821f85cf157f81117eeaba0c3cf0337eac357e58) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sw-02.f2", 0x0000, 0x2000, CRC(f8e9ada2) SHA1(028f554e70425c53faa30a6fe1c45cc16724560a) )    /* characters */

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "sr-08.a1", 0x0000, 0x2000, CRC(3884d9eb) SHA1(5cbd9215fa5ba5a61208b383700adc4428521aed) )    /* tiles */
	ROM_LOAD( "sr-09.a2", 0x2000, 0x2000, CRC(999cf6e0) SHA1(5b8b685038ec98b781908b92eb7fb9506db68544) )
	ROM_LOAD( "sr-10.a3", 0x4000, 0x2000, CRC(8edb273a) SHA1(85fdd4c690ed31e6396e3c16aa02140ee7ea2d61) )
	ROM_LOAD( "sr-11.a4", 0x6000, 0x2000, CRC(3a2726c3) SHA1(187c92ef591febdcbd1d42ab850e0cbb62c00873) )
	ROM_LOAD( "sr-12.a5", 0x8000, 0x2000, CRC(1bd3d8bb) SHA1(ef4dce605eb4dc8035985a415315ec61c21419c6) )
	ROM_LOAD( "sr-13.a6", 0xa000, 0x2000, CRC(658f02c4) SHA1(f087d69e49e38cf3107350cde18fcf85a8fa04f0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "sr-14.l1", 0x00000, 0x4000, CRC(2528bec6) SHA1(29f7719f18faad6bd1ec6735cc24e69168361470) )   /* sprites */
	ROM_LOAD( "sr-15.l2", 0x04000, 0x4000, CRC(f89287aa) SHA1(136fff6d2a4f48a488fc7c620213761459c3ada0) )
	ROM_LOAD( "sr-16.n1", 0x08000, 0x4000, CRC(024418f8) SHA1(145b8d5d6c8654cd090955a98f6dd8c8dbafe7c1) )
	ROM_LOAD( "sr-17.n2", 0x0c000, 0x4000, CRC(e2c7e489) SHA1(d4b5d575c021f58f6966df189df94e08c5b3621c) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "sb-5.e8",  0x0000, 0x0100, CRC(93ab8153) SHA1(a792f24e5c0c3c4a6b436102e7a98199f878ece1) )    /* red component */
	ROM_LOAD( "sb-6.e9",  0x0100, 0x0100, CRC(8ab44f7d) SHA1(f74680a6a987d74b3acb32e6396f20e127874149) )    /* green component */
	ROM_LOAD( "sb-7.e10", 0x0200, 0x0100, CRC(f4ade9a4) SHA1(62ad31d31d183cce213b03168daa035083b2f28e) )    /* blue component */
	ROM_LOAD( "sb-0.f1",  0x0300, 0x0100, CRC(6047d91b) SHA1(1ce025f9524c1033e48c5294ee7d360f8bfebe8d) )    /* char lookup table */
	ROM_LOAD( "sb-4.d6",  0x0400, 0x0100, CRC(4858968d) SHA1(20b5dbcaa1a4081b3139e7e2332d8fe3c9e55ed6) )    /* tile lookup table */
	ROM_LOAD( "sb-8.k3",  0x0500, 0x0100, CRC(f6fad943) SHA1(b0a24ea7805272e8ebf72a99b08907bc00d5f82f) )    /* sprite lookup table */
	ROM_LOAD( "sb-2.d1",  0x0600, 0x0100, CRC(8bb8b3df) SHA1(49de2819c4c92057fedcb20425282515d85829aa) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-3.d2",  0x0700, 0x0100, CRC(3b0c99af) SHA1(38f30ac1e48632634e409f328ee3051b987de7ad) )    /* tile palette selector? (not used) */
	ROM_LOAD( "sb-1.k6",  0x0800, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    /* interrupt timing (not used) */
	ROM_LOAD( "sb-9.m11", 0x0900, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) )    /* video timing? (not used) */
ROM_END



ROM_START( 1942p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF ) /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "1.bin",    0x00000, 0x8000, CRC(d8506aee) SHA1(aebdce3203e7743d70a8465a5e5766f9f47cb33f) ) // sldh
	ROM_LOAD( "2.bin",    0x10000, 0x4000, CRC(793a8fbc) SHA1(57f27a2b59cbc7e82e41683ddfd58055350f80bc) ) // sldh
	ROM_CONTINUE(         0x18000, 0x4000)
	ROM_LOAD( "3.bin",    0x14000, 0x4000, CRC(108fda63) SHA1(6ffdf57a04bcfae9fdb2343f30cff50926188cbf) ) // sldh

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "04.bin",   0x0000, 0x4000,  CRC(b4efd1af) SHA1(015b687b1714f892c3b2528bceb2df8ca48b6b8e) )

	ROM_REGION( 0x2000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "8.bin",    0x0000, 0x2000, CRC(6ebca191) SHA1(0dbddadde54a0ab66994c4a8726be05c6ca88a0e) ) /* characters */ // sldh

	ROM_REGION( 0xc000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "5.bin",    0x0000, 0x4000, CRC(1081b88c) SHA1(f3026e72206c96573fd6ba28d15e865b51735004) ) /* tiles */ // sldh
	ROM_LOAD( "6.bin",    0x4000, 0x4000, CRC(2d6acd8c) SHA1(914bb971c8f1364d0c44bd11f5f7e8da1f4953bb) ) // sldh
	ROM_LOAD( "7.bin",    0x8000, 0x4000, CRC(30f13e78) SHA1(51b9c0dfc53db705b75dd7ce643cec807533af5a) ) // sldh

	ROM_REGION( 0x10000, "gfx3", ROMREGION_INVERT )
	ROM_LOAD( "9.bin",    0x0000, 0x4000, CRC(755a4762) SHA1(b8747e02854a2dd8fa1251e206dbf0a0fc017b38) ) /* tiles */ // sldh
	ROM_LOAD( "10.bin",   0x4000, 0x4000, CRC(4a5a9084) SHA1(dcf9834e58324f9c94206728a055083e335bc862) ) // sldh
	ROM_LOAD( "11.bin",   0x8000, 0x4000, CRC(d2ce3eb6) SHA1(ebe71bd413b169ff2cea6973faf48527a8283eef) ) // sldh
	ROM_LOAD( "12.bin",   0xc000, 0x4000, CRC(aaa86493) SHA1(b0f6c59b5369b565bf863544a26cde2105aa35be) ) // sldh

	ROM_REGION( 0x0100, "proms", 0 ) // only one prom was in the dump - uses paletteram instead of proms
	ROM_LOAD( "ic22.bin", 0x0000, 0x0100, CRC(f6fad943) SHA1(b0a24ea7805272e8ebf72a99b08907bc00d5f82f) ) /* sprite lookup table */ // sldh
ROM_END


DRIVER_INIT_MEMBER(_1942_state,1942)
{
	UINT8 *ROM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 4, &ROM[0x10000], 0x4000);
}


GAME( 1984, 1942,     0,    1942, 1942, _1942_state, 1942, ROT270, "Capcom", "1942 (Revision B)", GAME_SUPPORTS_SAVE )
GAME( 1984, 1942a,    1942, 1942, 1942, _1942_state, 1942, ROT270, "Capcom", "1942 (Revision A)", GAME_SUPPORTS_SAVE )
GAME( 1984, 1942abl,  1942, 1942, 1942, _1942_state, 1942, ROT270, "bootleg", "1942 (Revision A, bootleg)", GAME_SUPPORTS_SAVE ) // data is the same as 1942a set, different rom format
GAME( 198?, 1942h,    1942, 1942, 1942, _1942_state, 1942, ROT270, "hack (Two Bit Score?)", "42", GAME_SUPPORTS_SAVE )
GAME( 1984, 1942b,    1942, 1942, 1942, _1942_state, 1942, ROT270, "Capcom", "1942 (First Version)", GAME_SUPPORTS_SAVE )
GAME( 1985, 1942w,    1942, 1942, 1942, _1942_state, 1942, ROT270, "Capcom (Williams Electronics license)", "1942 (Williams Electronics license)", GAME_SUPPORTS_SAVE ) /* Based on 1942 (Revision B) */
GAME( 1984, 1942p,    1942, 1942p,1942p,_1942_state, 1942, ROT270, "bootleg", "1942 (Tecfri PCB, bootleg?)", GAME_SUPPORTS_SAVE )
