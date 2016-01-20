// license:???
// copyright-holders:David Graves, Jarek Burczynski
// thanks-to:Richard Bush
/***************************************************************************

Darius    (c) Taito 1986
======

David Graves, Jarek Burczynski

Sound panning and other enhancements: Hiromitsu Shioya

Sources:        MAME Rastan driver
            Raine source - invaluable for this driver -
            many thanks to Richard Bush and the Raine Team.

                *****

Rom Versions
------------

Darius appears to be a revision of Dariusj (as well as being
for a different sales area). It has continue facilities, missing
in Dariusj, and extra sprites which are used when continuing.
It also has 2 extra program roms.


Use of PC080SN
--------------

This game uses 3 x PC080SN for generating tilemaps. They must be
mapped somehow to a single memory block and set of scroll registers.
There is an additional text tilemap on top of this, to allow for
both background planes scrolling. This need is presumably what led
to the TC0100SCN tilemap chip, debuted in Ninja Warriors (c)1987.
(The TC0100SCN includes a separate text layer.)


ADPCM Z80
---------

This writes the rom area whenever an interrupt occurs. It has no ram
therefore no stack to store registers or return addresses: so the
standard Z80 writes to the stack become irrelevant.


Dumpers Notes
=============

Darius (Old JPN Ver.)
(c)1986 Taito

-----------------------
Sound Board
K1100228A
CPU     :Z80 x2
Sound   :YM2203C x2
OSC     :8.000MHz
-----------------------
A96_56.18
A96_57.33

-----------------------
M4300067A
K1100226A
CPU     :MC68000P8 x2
OSC     :16000.00KHz
-----------------------
A96_28.152
A96_29.185
A96_30.154
A96_31.187

A96_32.157
A96_33.190
A96_34.158
A96_35.191

A96_36.175
A96_37.196
A96_38.176
A96_39.197
A96_40.177
A96_41.198
A96_42.178
A96_43.199
A96_44.179
A96_45.200
A96_46.180
A96_47.201

-----------------------
K1100227A
OSC     :26686.00KHz
Other   :PC080SN x3
-----------------------
A96_48.103
A96_48.24
A96_48.63
A96_49.104
A96_49.25
A96_49.64
A96_50.105
A96_50.26
A96_50.65
A96_51.131
A96_51.47
A96_51.86
A96_52.132
A96_52.48
A96_52.87
A96_53.133
A96_53.49
A96_53.88

A96_54.142
A96_55.143

A96-24.163
A96-25.164
A96-26.165


TODO
====

When you add a coin there is temporary volume distortion of other
sounds.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "audio/taitosnd.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"
#include "sound/flt_vol.h"
#include "includes/darius.h"
#include "darius.lh"


void darius_state::parse_control(  )   /* assumes Z80 sandwiched between 68Ks */
{
	/* bit 0 enables cpu B */
	/* however this fails when recovering from a save state
	   if cpu B is disabled !! */
	m_cpub->set_input_line(INPUT_LINE_RESET, (m_cpua_ctrl & 0x01) ? CLEAR_LINE : ASSERT_LINE);
}

WRITE16_MEMBER(darius_state::cpua_ctrl_w)
{
	if ((data & 0xff00) && ((data & 0xff) == 0))
		data = data >> 8;

	m_cpua_ctrl = data;

	parse_control();

	logerror("CPU #0 PC %06x: write %04x to cpu control\n", space.device().safe_pc(), data);
}

WRITE16_MEMBER(darius_state::darius_watchdog_w)
{
	watchdog_reset_w(space, 0, data);
}


/**********************************************************
                        GAME INPUTS
**********************************************************/

READ16_MEMBER(darius_state::darius_ioc_r)
{
	switch (offset)
	{
		case 0x01:
			return (m_tc0140syt->master_comm_r(space, 0) & 0xff);    /* sound interface read */

		case 0x04:
			return ioport("P1")->read();

		case 0x05:
			return ioport("P2")->read();

		case 0x06:
			return ioport("SYSTEM")->read();

		case 0x07:
			return m_coin_word; /* bits 3&4 coin lockouts, must return zero */

		case 0x08:
			return ioport("DSW")->read();
	}

logerror("CPU #0 PC %06x: warning - read unmapped ioc offset %06x\n",space.device().safe_pc(),offset);

	return 0xff;
}

WRITE16_MEMBER(darius_state::darius_ioc_w)
{
	switch (offset)
	{
		case 0x00:  /* sound interface write */

			m_tc0140syt->master_port_w(space, 0, data & 0xff);
			return;

		case 0x01:  /* sound interface write */

			m_tc0140syt->master_comm_w(space, 0, data & 0xff);
			return;

		case 0x28:  /* unknown, written by both cpus - always 0? */
//popmessage(" address %04x value %04x",offset,data);
			return;

		case 0x30:  /* coin control */
			/* bits 7,5,4,0 used on reset */
			/* bit 4 used whenever bg is blanked ? */
			machine().bookkeeping().coin_lockout_w(0, ~data & 0x02);
			machine().bookkeeping().coin_lockout_w(1, ~data & 0x04);
			machine().bookkeeping().coin_counter_w(0, data & 0x08);
			machine().bookkeeping().coin_counter_w(1, data & 0x40);
			m_coin_word = data & 0xffff;
//popmessage(" address %04x value %04x",offset,data);
			return;
	}

logerror("CPU #0 PC %06x: warning - write unmapped ioc offset %06x with %04x\n",space.device().safe_pc(),offset,data);
}


/***********************************************************
                     MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( darius_map, AS_PROGRAM, 16, darius_state )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x080000, 0x08ffff) AM_RAM                                             /* main RAM */
	AM_RANGE(0x0a0000, 0x0a0001) AM_WRITE(cpua_ctrl_w)
	AM_RANGE(0x0b0000, 0x0b0001) AM_WRITE(darius_watchdog_w)
	AM_RANGE(0xc00000, 0xc0007f) AM_READWRITE(darius_ioc_r, darius_ioc_w)           /* inputs, sound */
	AM_RANGE(0xd00000, 0xd0ffff) AM_DEVREADWRITE("pc080sn", pc080sn_device, word_r, word_w)  /* tilemaps */
	AM_RANGE(0xd20000, 0xd20003) AM_DEVWRITE("pc080sn", pc080sn_device, yscroll_word_w)
	AM_RANGE(0xd40000, 0xd40003) AM_DEVWRITE("pc080sn", pc080sn_device, xscroll_word_w)
	AM_RANGE(0xd50000, 0xd50003) AM_DEVWRITE("pc080sn", pc080sn_device, ctrl_word_w)
	AM_RANGE(0xd80000, 0xd80fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")/* palette */
	AM_RANGE(0xe00100, 0xe00fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe01000, 0xe02fff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xe08000, 0xe0ffff) AM_RAM_WRITE(darius_fg_layer_w) AM_SHARE("fg_ram")
	AM_RANGE(0xe10000, 0xe10fff) AM_RAM                                             /* ??? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( darius_cpub_map, AS_PROGRAM, 16, darius_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0x04ffff) AM_RAM             /* local RAM */
	AM_RANGE(0xc00000, 0xc0007f) AM_WRITE(darius_ioc_w) /* only writes $c00050 (?) */
	AM_RANGE(0xd80000, 0xd80fff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xe00100, 0xe00fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe01000, 0xe02fff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xe08000, 0xe0ffff) AM_RAM_WRITE(darius_fg_layer_w) AM_SHARE("fg_ram")
ADDRESS_MAP_END


/*****************************************************
                        SOUND
*****************************************************/

WRITE8_MEMBER(darius_state::sound_bankswitch_w)
{
	membank("bank1")->set_entry(data & 3);
}

WRITE8_MEMBER(darius_state::adpcm_command_w)
{
	m_adpcm_command = data;
	/* logerror("#ADPCM command write =%2x\n",data); */
}

#if 0
WRITE8_MEMBER(darius_state::display_value)
{
	popmessage("d800=%x", data);
}
#endif


/*****************************************************
               Sound mixer/pan control
*****************************************************/

void darius_state::update_fm0(  )
{
	int left  = (        m_pan[0]  * m_vol[6]) >> 8;
	int right = ((0xff - m_pan[0]) * m_vol[6]) >> 8;

	if (m_filter0_3l != nullptr)
		m_filter0_3l->flt_volume_set_volume(left / 100.0);
	if (m_filter0_3r != nullptr)
		m_filter0_3r->flt_volume_set_volume(right / 100.0); /* FM #0 */
}

void darius_state::update_fm1(  )
{
	int left  = (        m_pan[1]  * m_vol[7]) >> 8;
	int right = ((0xff - m_pan[1]) * m_vol[7]) >> 8;

	if (m_filter1_3l != nullptr)
		m_filter1_3l->flt_volume_set_volume(left / 100.0);
	if (m_filter1_3r != nullptr)
		m_filter1_3r->flt_volume_set_volume(right / 100.0); /* FM #1 */
}

void darius_state::update_psg0( int port )
{
	filter_volume_device *lvol = nullptr, *rvol = nullptr;
	int left, right;

	switch (port)
	{
		case 0: lvol = m_filter0_0l; rvol = m_filter0_0r; break;
		case 1: lvol = m_filter0_1l; rvol = m_filter0_1r; break;
		case 2: lvol = m_filter0_2l; rvol = m_filter0_2r; break;
		default: break;
	}

	left  = (        m_pan[2]  * m_vol[port]) >> 8;
	right = ((0xff - m_pan[2]) * m_vol[port]) >> 8;

	if (lvol != nullptr)
		lvol->flt_volume_set_volume(left / 100.0);
	if (rvol != nullptr)
		rvol->flt_volume_set_volume(right / 100.0);
}

void darius_state::update_psg1( int port )
{
	filter_volume_device *lvol = nullptr, *rvol = nullptr;
	int left, right;

	switch (port)
	{
		case 0: lvol = m_filter1_0l; rvol = m_filter1_0r; break;
		case 1: lvol = m_filter1_1l; rvol = m_filter1_1r; break;
		case 2: lvol = m_filter1_2l; rvol = m_filter1_2r; break;
		default: break;
	}

	left  = (        m_pan[3]  * m_vol[port + 3]) >> 8;
	right = ((0xff - m_pan[3]) * m_vol[port + 3]) >> 8;

	if (lvol != nullptr)
		lvol->flt_volume_set_volume(left / 100.0);
	if (rvol != nullptr)
		rvol->flt_volume_set_volume(right / 100.0);
}

void darius_state::update_da(  )
{
	int left  = m_def_vol[(m_pan[4] >> 4) & 0x0f];
	int right = m_def_vol[(m_pan[4] >> 0) & 0x0f];

	if (m_msm5205_l != nullptr)
		m_msm5205_l->flt_volume_set_volume(left / 100.0);
	if (m_msm5205_r != nullptr)
		m_msm5205_r->flt_volume_set_volume(right / 100.0);
}

WRITE8_MEMBER(darius_state::darius_fm0_pan)
{
	m_pan[0] = data & 0xff;  /* data 0x00:right 0xff:left */
	update_fm0();
}

WRITE8_MEMBER(darius_state::darius_fm1_pan)
{
	m_pan[1] = data & 0xff;
	update_fm1();
}

WRITE8_MEMBER(darius_state::darius_psg0_pan)
{
	m_pan[2] = data & 0xff;
	update_psg0(0);
	update_psg0(1);
	update_psg0(2);
}

WRITE8_MEMBER(darius_state::darius_psg1_pan)
{
	m_pan[3] = data & 0xff;
	update_psg1( 0);
	update_psg1( 1);
	update_psg1( 2);
}

WRITE8_MEMBER(darius_state::darius_da_pan)
{
	m_pan[4] = data & 0xff;
	update_da();
}

/**** Mixer Control ****/

WRITE8_MEMBER(darius_state::darius_write_portA0)
{
	// volume control FM #0 PSG #0 A
	//popmessage(" pan %02x %02x %02x %02x %02x", m_pan[0], m_pan[1], m_pan[2], m_pan[3], m_pan[4] );
	//popmessage(" A0 %02x A1 %02x B0 %02x B1 %02x", port[0], port[1], port[2], port[3] );

	m_vol[0] = m_def_vol[(data >> 4) & 0x0f];
	m_vol[6] = m_def_vol[(data >> 0) & 0x0f];
	update_fm0();
	update_psg0(0);
}

WRITE8_MEMBER(darius_state::darius_write_portA1)
{
	// volume control FM #1 PSG #1 A
	//popmessage(" pan %02x %02x %02x %02x %02x", m_pan[0], m_pan[1], m_pan[2], m_pan[3], m_pan[4] );

	m_vol[3] = m_def_vol[(data >> 4) & 0x0f];
	m_vol[7] = m_def_vol[(data >> 0) & 0x0f];
	update_fm1();
	update_psg1( 0);
}

WRITE8_MEMBER(darius_state::darius_write_portB0)
{
	// volume control PSG #0 B/C
	//popmessage(" pan %02x %02x %02x %02x %02x", m_pan[0], m_pan[1], m_pan[2], m_pan[3], m_pan[4] );

	m_vol[1] = m_def_vol[(data >> 4) & 0x0f];
	m_vol[2] = m_def_vol[(data >> 0) & 0x0f];
	update_psg0(1);
	update_psg0(2);
}

WRITE8_MEMBER(darius_state::darius_write_portB1)
{
	// volume control PSG #1 B/C
	//popmessage(" pan %02x %02x %02x %02x %02x", m_pan[0], m_pan[1], m_pan[2], m_pan[3], m_pan[4] );

	m_vol[4] = m_def_vol[(data >> 4) & 0x0f];
	m_vol[5] = m_def_vol[(data >> 0) & 0x0f];
	update_psg1( 1);
	update_psg1( 2);
}


/*****************************************************
           Sound memory structures / ADPCM
*****************************************************/

static ADDRESS_MAP_START( darius_sound_map, AS_PROGRAM, 8, darius_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
	AM_RANGE(0xa000, 0xa001) AM_DEVREADWRITE("ym2", ym2203_device, read, write)
	AM_RANGE(0xb000, 0xb000) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xb001, 0xb001) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(darius_fm0_pan)
	AM_RANGE(0xc400, 0xc400) AM_WRITE(darius_fm1_pan)
	AM_RANGE(0xc800, 0xc800) AM_WRITE(darius_psg0_pan)
	AM_RANGE(0xcc00, 0xcc00) AM_WRITE(darius_psg1_pan)
	AM_RANGE(0xd000, 0xd000) AM_WRITE(darius_da_pan)
	AM_RANGE(0xd400, 0xd400) AM_WRITE(adpcm_command_w)  /* ADPCM command for second Z80 to read from port 0x00 */
//  AM_RANGE(0xd800, 0xd800) AM_WRITE(display_value)    /* ??? */
	AM_RANGE(0xdc00, 0xdc00) AM_WRITE(sound_bankswitch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( darius_sound2_map, AS_PROGRAM, 8, darius_state )
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_WRITENOP
	/* yes, no RAM */
ADDRESS_MAP_END


WRITE_LINE_MEMBER(darius_state::darius_adpcm_int)
{
	if (m_nmi_enable)
		m_adpcm->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

READ8_MEMBER(darius_state::adpcm_command_read)
{
	/* logerror("read port 0: %02x  PC=%4x\n",adpcm_command, space.device().safe_pc() ); */
	return m_adpcm_command;
}

READ8_MEMBER(darius_state::readport2)
{
	return 0;
}

READ8_MEMBER(darius_state::readport3)
{
	return 0;
}

WRITE8_MEMBER(darius_state::adpcm_nmi_disable)
{
	m_nmi_enable = 0;
	/* logerror("write port 0: NMI DISABLE  PC=%4x\n", data, space.device().safe_pc() ); */
}

WRITE8_MEMBER(darius_state::adpcm_nmi_enable)
{
	m_nmi_enable = 1;
	/* logerror("write port 1: NMI ENABLE   PC=%4x\n", space.device().safe_pc() ); */
}

WRITE8_MEMBER(darius_state::adpcm_data_w)
{
	m_msm->data_w(data);
	m_msm->reset_w(!(data & 0x20));    /* my best guess, but it could be output enable as well */
}

static ADDRESS_MAP_START( darius_sound2_io_map, AS_IO, 8, darius_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(adpcm_command_read, adpcm_nmi_disable)
	AM_RANGE(0x01, 0x01) AM_WRITE(adpcm_nmi_enable)
	AM_RANGE(0x02, 0x02) AM_READ(readport2) AM_WRITE(adpcm_data_w)  /* readport2 ??? */
	AM_RANGE(0x03, 0x03) AM_READ(readport3) /* ??? */
ADDRESS_MAP_END


/***********************************************************
                      INPUT PORTS, DIPs
***********************************************************/


#define TAITO_COINAGE_JAPAN_16 \
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6") \
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) ) \
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8") \
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )


static INPUT_PORTS_START( darius )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSW")   /* DSW */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Autofire" )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Fast" )
	PORT_SERVICE_DIPLOC(  0x0004, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "every 600k" )
	PORT_DIPSETTING(      0x0c00, "600k only" )
	PORT_DIPSETTING(      0x0400, "800k only" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x2000, "4" )
	PORT_DIPSETTING(      0x1000, "5" )
	PORT_DIPSETTING(      0x0000, "6" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dariusu ) /* The US version uses the Japan coinage settings & Extra Version has continue */
	PORT_INCLUDE( darius )

	PORT_MODIFY("DSW")   /* DSW */
	TAITO_COINAGE_JAPAN_16
INPUT_PORTS_END

static INPUT_PORTS_START( dariusj )
	PORT_INCLUDE( darius )

	PORT_MODIFY("DSW")   /* DSW */
	TAITO_COINAGE_JAPAN_16
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8") /* No Continue for this version */
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/**************************************************************
                           GFX DECODING
**************************************************************/

static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
		{ 24, 8, 16, 0 },       /* pixel bits separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		0+ 32*8, 1+ 32*8, 2+ 32*8, 3+ 32*8, 4+ 32*8, 5+ 32*8, 6+ 32*8, 7+ 32*8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		64*8 + 0*32, 64*8 + 1*32, 64*8 + 2*32, 64*8 + 3*32,
		64*8 + 4*32, 64*8 + 5*32, 64*8 + 6*32, 64*8 + 7*32 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static const gfx_layout char2layout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),
	2,  /* 2 bits per pixel */
	{ 0, 8 },   /* pixel bits separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8    /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( darius )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,   0, 128 )  /* sprites */
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 128 )  /* scr tiles */
	GFXDECODE_ENTRY( "gfx3", 0, char2layout,  0, 128 )  /* top layer scr tiles */
GFXDECODE_END


/**************************************************************
                        YM2203 (SOUND)
**************************************************************/

/* handler called by the YM2203 emulator when the internal timers cause an IRQ */
WRITE_LINE_MEMBER(darius_state::irqhandler) /* assumes Z80 sandwiched between 68Ks */
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

/***********************************************************
                       MACHINE DRIVERS
***********************************************************/

void darius_state::darius_postload()
{
	parse_control();
}

void darius_state::machine_start()
{
	membank("bank1")->configure_entries(0, 4, memregion("audiocpu")->base(), 0x4000);

	save_item(NAME(m_cpua_ctrl));
	save_item(NAME(m_coin_word));

	save_item(NAME(m_adpcm_command));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_vol));
	save_item(NAME(m_pan));
	machine().save().register_postload(save_prepost_delegate(FUNC(darius_state::darius_postload), this));
}


void darius_state::machine_reset()
{
	membank("bank1")->set_entry(0);

	m_cpua_ctrl = 0xff;
	m_coin_word = 0;
	m_adpcm_command = 0;
	m_nmi_enable = 0;

	machine().sound().system_enable(true);  /* mixer enabled */

	for (auto & elem : m_vol)
		elem = 0x00;    /* min volume */

	for (auto & elem : m_pan)
		elem = 0x80;    /* center */

	for (int i = 0; i < 0x10; i++)
	{
		//logerror( "calc %d = %d\n", i, (int)(100.0f / (float)pow(10.0f, (32.0f - (i * (32.0f / (float)(0xf)))) / 20.0f)) );
		m_def_vol[i] = (int)(100.0f / (float)pow(10.0f, (32.0f - (i * (32.0f / (float)(0xf)))) / 20.0f));
	}
}


static MACHINE_CONFIG_START( darius, darius_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz/2)  /* 8 MHz */
	MCFG_CPU_PROGRAM_MAP(darius_map)
	MCFG_CPU_VBLANK_INT_DRIVER("lscreen", darius_state,  irq4_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2) /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(darius_sound_map)

	MCFG_CPU_ADD("cpub", M68000, XTAL_16MHz/2) /* 8 MHz */
	MCFG_CPU_PROGRAM_MAP(darius_cpub_map)
	MCFG_CPU_VBLANK_INT_DRIVER("lscreen", darius_state,  irq4_line_hold)

	MCFG_CPU_ADD("adpcm", Z80, XTAL_8MHz/2) /* 4 MHz */  /* ADPCM player using MSM5205 */
	MCFG_CPU_PROGRAM_MAP(darius_sound2_map)
	MCFG_CPU_IO_MAP(darius_sound2_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))   /* 10 CPU slices per frame ? */


	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", darius)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)
	MCFG_DEFAULT_LAYOUT(layout_darius)

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(36*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 1*8, 29*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(darius_state, screen_update_darius_left)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_SCREEN_ADD("mscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(36*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 1*8, 29*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(darius_state, screen_update_darius_middle)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(36*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 1*8, 29*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(darius_state, screen_update_darius_right)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("pc080sn", PC080SN, 0)
	MCFG_PC080SN_GFX_REGION(1)
	MCFG_PC080SN_OFFSETS(-16, 8)
	MCFG_PC080SN_YINVERT(0)
	MCFG_PC080SN_DBLWIDTH(1)
	MCFG_PC080SN_GFXDECODE("gfxdecode")
	MCFG_PC080SN_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ym1", YM2203, XTAL_8MHz/2) /* 4 MHz */
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(darius_state, irqhandler))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(darius_state, darius_write_portA0))  /* portA write */
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(darius_state, darius_write_portB0))  /* portB write */
	MCFG_SOUND_ROUTE(0, "filter0.0l", 0.08)
	MCFG_SOUND_ROUTE(0, "filter0.0r", 0.08)
	MCFG_SOUND_ROUTE(1, "filter0.1l", 0.08)
	MCFG_SOUND_ROUTE(1, "filter0.1r", 0.08)
	MCFG_SOUND_ROUTE(2, "filter0.2l", 0.08)
	MCFG_SOUND_ROUTE(2, "filter0.2r", 0.08)
	MCFG_SOUND_ROUTE(3, "filter0.3l", 0.60)
	MCFG_SOUND_ROUTE(3, "filter0.3r", 0.60)

	MCFG_SOUND_ADD("ym2", YM2203, XTAL_8MHz/2) /* 4 MHz */
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(darius_state, darius_write_portA1))  /* portA write */
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(darius_state, darius_write_portB1))  /* portB write */
	MCFG_SOUND_ROUTE(0, "filter1.0l", 0.08)
	MCFG_SOUND_ROUTE(0, "filter1.0r", 0.08)
	MCFG_SOUND_ROUTE(1, "filter1.1l", 0.08)
	MCFG_SOUND_ROUTE(1, "filter1.1r", 0.08)
	MCFG_SOUND_ROUTE(2, "filter1.2l", 0.08)
	MCFG_SOUND_ROUTE(2, "filter1.2r", 0.08)
	MCFG_SOUND_ROUTE(3, "filter1.3l", 0.60)
	MCFG_SOUND_ROUTE(3, "filter1.3r", 0.60)

	MCFG_SOUND_ADD("msm", MSM5205, XTAL_384kHz)
	MCFG_MSM5205_VCLK_CB(WRITELINE(darius_state, darius_adpcm_int))   /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8KHz   */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "msm5205.l", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "msm5205.r", 1.0)

	MCFG_FILTER_VOLUME_ADD("filter0.0l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("filter0.0r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("filter0.1l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("filter0.1r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("filter0.2l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("filter0.2r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("filter0.3l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("filter0.3r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_FILTER_VOLUME_ADD("filter1.0l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("filter1.0r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("filter1.1l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("filter1.1r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("filter1.2l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("filter1.2r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("filter1.3l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("filter1.3r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_FILTER_VOLUME_ADD("msm5205.l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("msm5205.r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END


/***************************************************************************
                                  DRIVERS
***************************************************************************/

ROM_START( darius )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_FILL( 0x00000, 0x60000, 0xff )
	ROM_LOAD16_BYTE( "a96_59-1.186", 0x00000, 0x10000, CRC(11aab4eb) SHA1(92f795e96a940e8d94abbf429ba4ac119992b991) )
	ROM_LOAD16_BYTE( "a96_58-1.152", 0x00001, 0x10000, CRC(5f71e697) SHA1(bf959cf82e8e8ba950ab40d9c008ad5de01385aa) )
	ROM_LOAD16_BYTE( "a96_61-2.187", 0x20000, 0x10000, CRC(4736aa9b) SHA1(05e549d96a053e6b3bc34359267adcd73f98dd4a) )
	ROM_LOAD16_BYTE( "a96_66-2.153", 0x20001, 0x10000, CRC(4ede5f56) SHA1(88c06aef4b0a3e29fa30c24a57f2d3a05fc9f021) )
	ROM_LOAD16_BYTE( "a96_31.188",   0x40000, 0x10000, CRC(e9bb5d89) SHA1(a5d08129c32b97e2cce84496945766fd32b6506e) )    /* 2 data roms */
	ROM_LOAD16_BYTE( "a96_30.154",   0x40001, 0x10000, CRC(9eb5e127) SHA1(50e2fe5ec7f79ecf1fb5107298da13ef5ab37162) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "a96_57.33", 0x00000, 0x10000, CRC(33ceb730) SHA1(05070ea503ac57ff8445145d6f97115f7aad90a5) )

	ROM_REGION( 0x80000, "cpub", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "a96_33-1.190", 0x00000, 0x10000, CRC(ff186048) SHA1(becb00d2cc69a6d4e839086bd3d902f4e6a99aa6) )
	ROM_LOAD16_BYTE( "a96_32-1.157", 0x00001, 0x10000, CRC(d9719de8) SHA1(9e907cfb5cbe6abebccfbd065d02e7a71c5aa494) )
	ROM_LOAD16_BYTE( "a96_35-1.191", 0x20000, 0x10000, CRC(b3280193) SHA1(f4bad066c16682f9267752c50a31ef64b312f11e) )
	ROM_LOAD16_BYTE( "a96_34-1.158", 0x20001, 0x10000, CRC(ca3b2573) SHA1(4da0d8536e546ea46b2374318e25c30305f4c977) )

	ROM_REGION( 0x10000, "adpcm", 0 )   /* second Z80 driving the ADPCM chip */
	ROM_LOAD( "a96_56.18", 0x00000, 0x10000, CRC(292ef55c) SHA1(67bfe3693e43daece06d4795645d54cd66419e5b) )        /* Z80 prog + ADPCM samples */

	ROM_REGION( 0x60000, "gfx1", 0 )
	/* There are THREE of each SCR gfx rom on the actual board, making a complete set for every PC080SN tilemap chip */
	ROM_LOAD16_BYTE( "a96_48.24",  0x00000, 0x10000, CRC(39c9b3aa) SHA1(43a91d916c5a09207dfa37413feb5025636f37ae) )   /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_49.25",  0x20000, 0x10000, CRC(37a7d88a) SHA1(cede0d810d74ec460dcc4b391bb1acd5a669a7b4) )
	ROM_LOAD16_BYTE( "a96_50.26",  0x40000, 0x10000, CRC(75d738e4) SHA1(634606da46136ab605f5477af5639a20e39b44c4) )
	ROM_LOAD16_BYTE( "a96_51.47",  0x00001, 0x10000, CRC(1bf8f0d3) SHA1(7f36e69336260958282eb663fe71b56410f0ee42) )
	ROM_LOAD16_BYTE( "a96_52.48",  0x20001, 0x10000, CRC(2d9b2128) SHA1(9b72936fbd9dca6ef8302ac6c40a1cec019cebb5) )
	ROM_LOAD16_BYTE( "a96_53.49",  0x40001, 0x10000, CRC(0173484c) SHA1(41d70039bda0965afe89251696ceaec7b7f40c24) )

	ROM_REGION( 0xc0000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "a96_44.179", 0x00000, 0x10000, CRC(bbc18878) SHA1(7732ab2a3002f8b500615377dab42ac75451cb3b) )   /* 16x16 sprites */
	ROM_LOAD32_BYTE( "a96_45.200", 0x00001, 0x10000, CRC(616cdd8b) SHA1(74e0c483a68d984a689ea1381ed3a9da2f8a410a) )
	ROM_LOAD32_BYTE( "a96_46.180", 0x00002, 0x10000, CRC(fec35418) SHA1(f0f401c3634e91b81cb8484b7b03f350d382e889) )
	ROM_LOAD32_BYTE( "a96_47.201", 0x00003, 0x10000, CRC(8df9286a) SHA1(4a197e4c38d1750cc316b8710f4a0fef4316be14) )

	ROM_LOAD32_BYTE( "a96_40.177", 0x40000, 0x10000, CRC(b699a51e) SHA1(5fd751dd44618743dc8a3df04cf0a987753a868b) )
	ROM_LOAD32_BYTE( "a96_41.198", 0x40001, 0x10000, CRC(97128a3a) SHA1(257ddd1ba71e6beeaf18e0c5d7006d1d2b6a5edf) )
	ROM_LOAD32_BYTE( "a96_42.178", 0x40002, 0x10000, CRC(7f55ee0f) SHA1(d9ba7b8fbf59308a08613d67e92da6829f6b6db3) )
	ROM_LOAD32_BYTE( "a96_43.199", 0x40003, 0x10000, CRC(c7cad469) SHA1(dbd37aa10f12e4950f8ec6bcd7d150fa55e64742) )

	ROM_LOAD32_BYTE( "a96_62.175", 0x80000, 0x10000, CRC(9179862c) SHA1(be94c7d213a34baf82f974ee1092aba44b072623) )
	ROM_LOAD32_BYTE( "a96_63.196", 0x80001, 0x10000, CRC(fa19cfff) SHA1(58a3ae3270ebe5a162cd62df06da7199843707cf) )
	ROM_LOAD32_BYTE( "a96_64.176", 0x80002, 0x10000, CRC(814c676f) SHA1(a6a64e65a3c163ecfede14b48ea70c20050248c3) )
	ROM_LOAD32_BYTE( "a96_65.197", 0x80003, 0x10000, CRC(14eee326) SHA1(41760fada2a5e34ee6c9250af927baf650d9cfc4) )

	ROM_REGION( 0x8000, "gfx3", 0 )         /* 8x8 SCR tiles */
	/* There's only one of each of these on a real board */
	ROM_LOAD16_BYTE( "a96_54.143", 0x0000, 0x4000, CRC(51c02ae2) SHA1(27d2a6c649d047da1f22758569cb36531e3bf8bc) )
	ROM_LOAD16_BYTE( "a96_55.144", 0x0001, 0x4000, CRC(771e4d98) SHA1(0e8ce5d569775883f4bc777b9bd49eb23ba7b42e) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD16_BYTE( "a96-24.163", 0x0000, 0x0400, CRC(0fa8be7f) SHA1(079686b5d65b4b966591090d8c0e13e66dc5beca) ) /* proms, currently unused */
	ROM_LOAD16_BYTE( "a96-25.164", 0x0400, 0x0400, CRC(265508a6) SHA1(f8ee1c658b33ae76d8a457a4042d9b4b58247823) )
	ROM_LOAD16_BYTE( "a96-26.165", 0x0800, 0x0400, CRC(4891b9c0) SHA1(1f550a9a4ad3ca379f88f5865ed1b281c7b87f31) )
ROM_END


ROM_START( dariusu )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_FILL( 0x00000, 0x60000, 0xff )
	ROM_LOAD16_BYTE( "a96_59-1.186", 0x00000, 0x10000, CRC(11aab4eb) SHA1(92f795e96a940e8d94abbf429ba4ac119992b991) )
	ROM_LOAD16_BYTE( "a96_58-1.152", 0x00001, 0x10000, CRC(5f71e697) SHA1(bf959cf82e8e8ba950ab40d9c008ad5de01385aa) )
	ROM_LOAD16_BYTE( "a96_61-2.187", 0x20000, 0x10000, CRC(4736aa9b) SHA1(05e549d96a053e6b3bc34359267adcd73f98dd4a) )
	ROM_LOAD16_BYTE( "a96_60-2.153", 0x20001, 0x10000, CRC(9bf58617) SHA1(09b52b6aa522a61813b2e581b7e039a1fb6d6411) )
	ROM_LOAD16_BYTE( "a96_31.188",   0x40000, 0x10000, CRC(e9bb5d89) SHA1(a5d08129c32b97e2cce84496945766fd32b6506e) )    /* 2 data roms */
	ROM_LOAD16_BYTE( "a96_30.154",   0x40001, 0x10000, CRC(9eb5e127) SHA1(50e2fe5ec7f79ecf1fb5107298da13ef5ab37162) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "a96_57.33", 0x00000, 0x10000, CRC(33ceb730) SHA1(05070ea503ac57ff8445145d6f97115f7aad90a5) )

	ROM_REGION( 0x80000, "cpub", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "a96_33-1.190", 0x00000, 0x10000, CRC(ff186048) SHA1(becb00d2cc69a6d4e839086bd3d902f4e6a99aa6) )
	ROM_LOAD16_BYTE( "a96_32-1.157", 0x00001, 0x10000, CRC(d9719de8) SHA1(9e907cfb5cbe6abebccfbd065d02e7a71c5aa494) )
	ROM_LOAD16_BYTE( "a96_35-1.191", 0x20000, 0x10000, CRC(b3280193) SHA1(f4bad066c16682f9267752c50a31ef64b312f11e) )
	ROM_LOAD16_BYTE( "a96_34-1.158", 0x20001, 0x10000, CRC(ca3b2573) SHA1(4da0d8536e546ea46b2374318e25c30305f4c977) )

	ROM_REGION( 0x10000, "adpcm", 0 )   /* second Z80 driving the ADPCM chip */
	ROM_LOAD( "a96_56.18", 0x00000, 0x10000, CRC(292ef55c) SHA1(67bfe3693e43daece06d4795645d54cd66419e5b) )        /* Z80 prog + ADPCM samples */

	ROM_REGION( 0x60000, "gfx1", 0 )
	/* There are THREE of each SCR gfx rom on the actual board, making a complete set for every PC080SN tilemap chip */
	ROM_LOAD16_BYTE( "a96_48.24",  0x00000, 0x10000, CRC(39c9b3aa) SHA1(43a91d916c5a09207dfa37413feb5025636f37ae) )   /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_49.25",  0x20000, 0x10000, CRC(37a7d88a) SHA1(cede0d810d74ec460dcc4b391bb1acd5a669a7b4) )
	ROM_LOAD16_BYTE( "a96_50.26",  0x40000, 0x10000, CRC(75d738e4) SHA1(634606da46136ab605f5477af5639a20e39b44c4) )
	ROM_LOAD16_BYTE( "a96_51.47",  0x00001, 0x10000, CRC(1bf8f0d3) SHA1(7f36e69336260958282eb663fe71b56410f0ee42) )
	ROM_LOAD16_BYTE( "a96_52.48",  0x20001, 0x10000, CRC(2d9b2128) SHA1(9b72936fbd9dca6ef8302ac6c40a1cec019cebb5) )
	ROM_LOAD16_BYTE( "a96_53.49",  0x40001, 0x10000, CRC(0173484c) SHA1(41d70039bda0965afe89251696ceaec7b7f40c24) )

	ROM_REGION( 0xc0000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "a96_44.179", 0x00000, 0x10000, CRC(bbc18878) SHA1(7732ab2a3002f8b500615377dab42ac75451cb3b) )   /* 16x16 sprites */
	ROM_LOAD32_BYTE( "a96_45.200", 0x00001, 0x10000, CRC(616cdd8b) SHA1(74e0c483a68d984a689ea1381ed3a9da2f8a410a) )
	ROM_LOAD32_BYTE( "a96_46.180", 0x00002, 0x10000, CRC(fec35418) SHA1(f0f401c3634e91b81cb8484b7b03f350d382e889) )
	ROM_LOAD32_BYTE( "a96_47.201", 0x00003, 0x10000, CRC(8df9286a) SHA1(4a197e4c38d1750cc316b8710f4a0fef4316be14) )

	ROM_LOAD32_BYTE( "a96_40.177", 0x40000, 0x10000, CRC(b699a51e) SHA1(5fd751dd44618743dc8a3df04cf0a987753a868b) )
	ROM_LOAD32_BYTE( "a96_41.198", 0x40001, 0x10000, CRC(97128a3a) SHA1(257ddd1ba71e6beeaf18e0c5d7006d1d2b6a5edf) )
	ROM_LOAD32_BYTE( "a96_42.178", 0x40002, 0x10000, CRC(7f55ee0f) SHA1(d9ba7b8fbf59308a08613d67e92da6829f6b6db3) )
	ROM_LOAD32_BYTE( "a96_43.199", 0x40003, 0x10000, CRC(c7cad469) SHA1(dbd37aa10f12e4950f8ec6bcd7d150fa55e64742) )

	ROM_LOAD32_BYTE( "a96_62.175", 0x80000, 0x10000, CRC(9179862c) SHA1(be94c7d213a34baf82f974ee1092aba44b072623) )
	ROM_LOAD32_BYTE( "a96_63.196", 0x80001, 0x10000, CRC(fa19cfff) SHA1(58a3ae3270ebe5a162cd62df06da7199843707cf) )
	ROM_LOAD32_BYTE( "a96_64.176", 0x80002, 0x10000, CRC(814c676f) SHA1(a6a64e65a3c163ecfede14b48ea70c20050248c3) )
	ROM_LOAD32_BYTE( "a96_65.197", 0x80003, 0x10000, CRC(14eee326) SHA1(41760fada2a5e34ee6c9250af927baf650d9cfc4) )

	ROM_REGION( 0x8000, "gfx3", 0 )         /* 8x8 SCR tiles */
	/* There's only one of each of these on a real board */
	ROM_LOAD16_BYTE( "a96_54.143", 0x0000, 0x4000, CRC(51c02ae2) SHA1(27d2a6c649d047da1f22758569cb36531e3bf8bc) )
	ROM_LOAD16_BYTE( "a96_55.144", 0x0001, 0x4000, CRC(771e4d98) SHA1(0e8ce5d569775883f4bc777b9bd49eb23ba7b42e) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD16_BYTE( "a96-24.163", 0x0000, 0x0400, CRC(0fa8be7f) SHA1(079686b5d65b4b966591090d8c0e13e66dc5beca) ) /* proms, currently unused */
	ROM_LOAD16_BYTE( "a96-25.164", 0x0400, 0x0400, CRC(265508a6) SHA1(f8ee1c658b33ae76d8a457a4042d9b4b58247823) )
	ROM_LOAD16_BYTE( "a96-26.165", 0x0800, 0x0400, CRC(4891b9c0) SHA1(1f550a9a4ad3ca379f88f5865ed1b281c7b87f31) )
ROM_END


ROM_START( dariusj )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_FILL( 0x00000, 0x60000, 0xff )
	ROM_LOAD16_BYTE( "a96_29-1.185", 0x00000, 0x10000, CRC(75486f62) SHA1(818b095f2c6cc5764161c3e14ba70fe1c4b2f724) )
	ROM_LOAD16_BYTE( "a96_28-1.152", 0x00001, 0x10000, CRC(fb34d400) SHA1(b14517384f5eadca8b73833bcd81374614b928d4) )
	/* middle area is empty */
	ROM_LOAD16_BYTE( "a96_31.187",   0x40000, 0x10000, CRC(e9bb5d89) SHA1(a5d08129c32b97e2cce84496945766fd32b6506e) )   /* 2 data roms */
	ROM_LOAD16_BYTE( "a96_30.154",   0x40001, 0x10000, CRC(9eb5e127) SHA1(50e2fe5ec7f79ecf1fb5107298da13ef5ab37162) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "a96_57.33", 0x00000, 0x10000, CRC(33ceb730) SHA1(05070ea503ac57ff8445145d6f97115f7aad90a5) )

	ROM_REGION( 0x80000, "cpub", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "a96_33-1.190", 0x00000, 0x10000, CRC(ff186048) SHA1(becb00d2cc69a6d4e839086bd3d902f4e6a99aa6) )
	ROM_LOAD16_BYTE( "a96_32-1.157", 0x00001, 0x10000, CRC(d9719de8) SHA1(9e907cfb5cbe6abebccfbd065d02e7a71c5aa494) )
	ROM_LOAD16_BYTE( "a96_35-1.191", 0x20000, 0x10000, CRC(b3280193) SHA1(f4bad066c16682f9267752c50a31ef64b312f11e) )
	ROM_LOAD16_BYTE( "a96_34-1.158", 0x20001, 0x10000, CRC(ca3b2573) SHA1(4da0d8536e546ea46b2374318e25c30305f4c977) )

	ROM_REGION( 0x10000, "adpcm", 0 )   /* second Z80 driving the ADPCM chip */
	ROM_LOAD( "a96_56.18", 0x00000, 0x10000, CRC(292ef55c) SHA1(67bfe3693e43daece06d4795645d54cd66419e5b) )        /* Z80 prog + ADPCM samples */

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "a96_48.24",  0x00000, 0x10000, CRC(39c9b3aa) SHA1(43a91d916c5a09207dfa37413feb5025636f37ae) )   /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_49.25",  0x20000, 0x10000, CRC(37a7d88a) SHA1(cede0d810d74ec460dcc4b391bb1acd5a669a7b4) )
	ROM_LOAD16_BYTE( "a96_50.26",  0x40000, 0x10000, CRC(75d738e4) SHA1(634606da46136ab605f5477af5639a20e39b44c4) )
	ROM_LOAD16_BYTE( "a96_51.47",  0x00001, 0x10000, CRC(1bf8f0d3) SHA1(7f36e69336260958282eb663fe71b56410f0ee42) )
	ROM_LOAD16_BYTE( "a96_52.48",  0x20001, 0x10000, CRC(2d9b2128) SHA1(9b72936fbd9dca6ef8302ac6c40a1cec019cebb5) )
	ROM_LOAD16_BYTE( "a96_53.49",  0x40001, 0x10000, CRC(0173484c) SHA1(41d70039bda0965afe89251696ceaec7b7f40c24) )

	ROM_REGION( 0xc0000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "a96_44.179", 0x00000, 0x10000, CRC(bbc18878) SHA1(7732ab2a3002f8b500615377dab42ac75451cb3b) )   /* 16x16 sprites */
	ROM_LOAD32_BYTE( "a96_45.200", 0x00001, 0x10000, CRC(616cdd8b) SHA1(74e0c483a68d984a689ea1381ed3a9da2f8a410a) )
	ROM_LOAD32_BYTE( "a96_46.180", 0x00002, 0x10000, CRC(fec35418) SHA1(f0f401c3634e91b81cb8484b7b03f350d382e889) )
	ROM_LOAD32_BYTE( "a96_47.201", 0x00003, 0x10000, CRC(8df9286a) SHA1(4a197e4c38d1750cc316b8710f4a0fef4316be14) )

	ROM_LOAD32_BYTE( "a96_40.177", 0x40000, 0x10000, CRC(b699a51e) SHA1(5fd751dd44618743dc8a3df04cf0a987753a868b) )
	ROM_LOAD32_BYTE( "a96_41.198", 0x40001, 0x10000, CRC(97128a3a) SHA1(257ddd1ba71e6beeaf18e0c5d7006d1d2b6a5edf) )
	ROM_LOAD32_BYTE( "a96_42.178", 0x40002, 0x10000, CRC(7f55ee0f) SHA1(d9ba7b8fbf59308a08613d67e92da6829f6b6db3) )
	ROM_LOAD32_BYTE( "a96_43.199", 0x40003, 0x10000, CRC(c7cad469) SHA1(dbd37aa10f12e4950f8ec6bcd7d150fa55e64742) )

	ROM_LOAD32_BYTE( "a96_36.175", 0x80000, 0x10000, CRC(af598141) SHA1(f3b888bcbd4560cca48187055cbe4107e2b392a6) )
	ROM_LOAD32_BYTE( "a96_37.196", 0x80001, 0x10000, CRC(b48137c8) SHA1(03e98a93f4fa19dfe77da244c002abc84b936a22) )
	ROM_LOAD32_BYTE( "a96_38.176", 0x80002, 0x10000, CRC(e4f3e3a7) SHA1(0baa8a672516bcc4f17f40f429ac3d227de16625) )
	ROM_LOAD32_BYTE( "a96_39.197", 0x80003, 0x10000, CRC(ea30920f) SHA1(91d47b10886d6c243bc676435e300cb3b5fcca33) )

	ROM_REGION( 0x8000, "gfx3", 0 )         /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_54.143", 0x0000, 0x4000, CRC(51c02ae2) SHA1(27d2a6c649d047da1f22758569cb36531e3bf8bc) )
	ROM_LOAD16_BYTE( "a96_55.144", 0x0001, 0x4000, CRC(771e4d98) SHA1(0e8ce5d569775883f4bc777b9bd49eb23ba7b42e) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD16_BYTE( "a96-24.163", 0x0000, 0x0400, CRC(0fa8be7f) SHA1(079686b5d65b4b966591090d8c0e13e66dc5beca) ) /* proms, currently unused */
	ROM_LOAD16_BYTE( "a96-25.164", 0x0400, 0x0400, CRC(265508a6) SHA1(f8ee1c658b33ae76d8a457a4042d9b4b58247823) )
	ROM_LOAD16_BYTE( "a96-26.165", 0x0800, 0x0400, CRC(4891b9c0) SHA1(1f550a9a4ad3ca379f88f5865ed1b281c7b87f31) )
ROM_END

ROM_START( dariuso )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_FILL( 0x00000, 0x60000, 0xff )
	ROM_LOAD16_BYTE( "a96_29.185", 0x00000, 0x10000, CRC(f775162b) SHA1(a17e570c2ba4daf0a3526b45c324c822faac0c8d) )
	ROM_LOAD16_BYTE( "a96_28.152", 0x00001, 0x10000, CRC(4721d667) SHA1(fa9a109054a818f836452215204ce91f2b166ddb) )
	/* middle area is empty */
	ROM_LOAD16_BYTE( "a96_31.187", 0x40000, 0x10000, CRC(e9bb5d89) SHA1(a5d08129c32b97e2cce84496945766fd32b6506e) )   /* 2 data roms */
	ROM_LOAD16_BYTE( "a96_30.154", 0x40001, 0x10000, CRC(9eb5e127) SHA1(50e2fe5ec7f79ecf1fb5107298da13ef5ab37162) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "a96_57.33", 0x00000, 0x10000, CRC(33ceb730) SHA1(05070ea503ac57ff8445145d6f97115f7aad90a5) )

	ROM_REGION( 0x80000, "cpub", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "a96_33.190", 0x00000, 0x10000, CRC(d2f340d2) SHA1(d9175bf4dda5707afb3c57d3b6affe0305084c71) )
	ROM_LOAD16_BYTE( "a96_32.157", 0x00001, 0x10000, CRC(044c9848) SHA1(5293e9e83fd38d0d14e4f3b3a342d88e27ee44d6) )
	ROM_LOAD16_BYTE( "a96_35.191", 0x20000, 0x10000, CRC(b8ed718b) SHA1(8951f9c3c971c5621ec98b63fb27d44f30304c70) )
	ROM_LOAD16_BYTE( "a96_34.158", 0x20001, 0x10000, CRC(7556a660) SHA1(eaa82f3e1f827616ff25e22673d6d2ee54f0ad4c) )

	ROM_REGION( 0x10000, "adpcm", 0 )   /* second Z80 driving the ADPCM chip */
	ROM_LOAD( "a96_56.18", 0x00000, 0x10000, CRC(292ef55c) SHA1(67bfe3693e43daece06d4795645d54cd66419e5b) )        /* Z80 prog + ADPCM samples */

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "a96_48.24",  0x00000, 0x10000, CRC(39c9b3aa) SHA1(43a91d916c5a09207dfa37413feb5025636f37ae) )   /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_49.25",  0x20000, 0x10000, CRC(37a7d88a) SHA1(cede0d810d74ec460dcc4b391bb1acd5a669a7b4) )
	ROM_LOAD16_BYTE( "a96_50.26",  0x40000, 0x10000, CRC(75d738e4) SHA1(634606da46136ab605f5477af5639a20e39b44c4) )
	ROM_LOAD16_BYTE( "a96_51.47",  0x00001, 0x10000, CRC(1bf8f0d3) SHA1(7f36e69336260958282eb663fe71b56410f0ee42) )
	ROM_LOAD16_BYTE( "a96_52.48",  0x20001, 0x10000, CRC(2d9b2128) SHA1(9b72936fbd9dca6ef8302ac6c40a1cec019cebb5) )
	ROM_LOAD16_BYTE( "a96_53.49",  0x40001, 0x10000, CRC(0173484c) SHA1(41d70039bda0965afe89251696ceaec7b7f40c24) )

	ROM_REGION( 0xc0000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "a96_44.179", 0x00000, 0x10000, CRC(bbc18878) SHA1(7732ab2a3002f8b500615377dab42ac75451cb3b) )   /* 16x16 sprites */
	ROM_LOAD32_BYTE( "a96_45.200", 0x00001, 0x10000, CRC(616cdd8b) SHA1(74e0c483a68d984a689ea1381ed3a9da2f8a410a) )
	ROM_LOAD32_BYTE( "a96_46.180", 0x00002, 0x10000, CRC(fec35418) SHA1(f0f401c3634e91b81cb8484b7b03f350d382e889) )
	ROM_LOAD32_BYTE( "a96_47.201", 0x00003, 0x10000, CRC(8df9286a) SHA1(4a197e4c38d1750cc316b8710f4a0fef4316be14) )

	ROM_LOAD32_BYTE( "a96_40.177", 0x40000, 0x10000, CRC(b699a51e) SHA1(5fd751dd44618743dc8a3df04cf0a987753a868b) )
	ROM_LOAD32_BYTE( "a96_41.198", 0x40001, 0x10000, CRC(97128a3a) SHA1(257ddd1ba71e6beeaf18e0c5d7006d1d2b6a5edf) )
	ROM_LOAD32_BYTE( "a96_42.178", 0x40002, 0x10000, CRC(7f55ee0f) SHA1(d9ba7b8fbf59308a08613d67e92da6829f6b6db3) )
	ROM_LOAD32_BYTE( "a96_43.199", 0x40003, 0x10000, CRC(c7cad469) SHA1(dbd37aa10f12e4950f8ec6bcd7d150fa55e64742) )

	ROM_LOAD32_BYTE( "a96_36.175", 0x80000, 0x10000, CRC(af598141) SHA1(f3b888bcbd4560cca48187055cbe4107e2b392a6) )
	ROM_LOAD32_BYTE( "a96_37.196", 0x80001, 0x10000, CRC(b48137c8) SHA1(03e98a93f4fa19dfe77da244c002abc84b936a22) )
	ROM_LOAD32_BYTE( "a96_38.176", 0x80002, 0x10000, CRC(e4f3e3a7) SHA1(0baa8a672516bcc4f17f40f429ac3d227de16625) )
	ROM_LOAD32_BYTE( "a96_39.197", 0x80003, 0x10000, CRC(ea30920f) SHA1(91d47b10886d6c243bc676435e300cb3b5fcca33) )

	ROM_REGION( 0x8000, "gfx3", 0 )         /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_54.143", 0x0000, 0x4000, CRC(51c02ae2) SHA1(27d2a6c649d047da1f22758569cb36531e3bf8bc) )
	ROM_LOAD16_BYTE( "a96_55.144", 0x0001, 0x4000, CRC(771e4d98) SHA1(0e8ce5d569775883f4bc777b9bd49eb23ba7b42e) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD16_BYTE( "a96-24.163", 0x0000, 0x0400, CRC(0fa8be7f) SHA1(079686b5d65b4b966591090d8c0e13e66dc5beca) ) /* proms, currently unused */
	ROM_LOAD16_BYTE( "a96-25.164", 0x0400, 0x0400, CRC(265508a6) SHA1(f8ee1c658b33ae76d8a457a4042d9b4b58247823) )
	ROM_LOAD16_BYTE( "a96-26.165", 0x0800, 0x0400, CRC(4891b9c0) SHA1(1f550a9a4ad3ca379f88f5865ed1b281c7b87f31) )
ROM_END

ROM_START( dariuse )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_FILL( 0x00000, 0x60000, 0xff )
	ROM_LOAD16_BYTE( "a96_68.185", 0x00000, 0x10000, CRC(ed721127) SHA1(8127f4a9b26b5fb83a381235eef0577d60d1cfd7) )
	ROM_LOAD16_BYTE( "a96_67.152", 0x00001, 0x10000, CRC(b99aea8c) SHA1(859ada7c472ab2ac308faa775066e79ed1f4ad71) )
	/* middle area is empty */
	ROM_LOAD16_BYTE( "a96_70.187", 0x40000, 0x10000, CRC(54590b31) SHA1(2b89846f14a5cb19b58ab4999bc5ae11671bbb5a) )   /* 2 data roms */
	ROM_LOAD16_BYTE( "a96_69.154", 0x40001, 0x10000, CRC(9eb5e127) SHA1(50e2fe5ec7f79ecf1fb5107298da13ef5ab37162) )   // == a96_30.154

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "a96_57.33", 0x00000, 0x10000, CRC(33ceb730) SHA1(05070ea503ac57ff8445145d6f97115f7aad90a5) )

	ROM_REGION( 0x80000, "cpub", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "a96_72.190", 0x00000, 0x10000, CRC(248ca2cc) SHA1(43b29146d8e2c62dd1fb7dc842fd441a360f2453) )
	ROM_LOAD16_BYTE( "a96_71.157", 0x00001, 0x10000, CRC(65dd0403) SHA1(8036c35ce5df0727cccb9ece3bfac9577160d4fd) )
	ROM_LOAD16_BYTE( "a96_74.191", 0x20000, 0x10000, CRC(0ea31f60) SHA1(c9e7eaf8bf3abbef944b7de407d5d5ddaac93e31) )
	ROM_LOAD16_BYTE( "a96_73.158", 0x20001, 0x10000, CRC(27036a4d) SHA1(426dccb8f559d39460c97bfd4354c74a59af172e) )

	ROM_REGION( 0x10000, "adpcm", 0 )   /* second Z80 driving the ADPCM chip */
	ROM_LOAD( "a96_56.18", 0x00000, 0x10000, CRC(292ef55c) SHA1(67bfe3693e43daece06d4795645d54cd66419e5b) )        /* Z80 prog + ADPCM samples */

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "a96_48.24",  0x00000, 0x10000, CRC(39c9b3aa) SHA1(43a91d916c5a09207dfa37413feb5025636f37ae) )   /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_49.25",  0x20000, 0x10000, CRC(37a7d88a) SHA1(cede0d810d74ec460dcc4b391bb1acd5a669a7b4) )
	ROM_LOAD16_BYTE( "a96_50.26",  0x40000, 0x10000, CRC(75d738e4) SHA1(634606da46136ab605f5477af5639a20e39b44c4) )
	ROM_LOAD16_BYTE( "a96_51.47",  0x00001, 0x10000, CRC(1bf8f0d3) SHA1(7f36e69336260958282eb663fe71b56410f0ee42) )
	ROM_LOAD16_BYTE( "a96_52.48",  0x20001, 0x10000, CRC(2d9b2128) SHA1(9b72936fbd9dca6ef8302ac6c40a1cec019cebb5) )
	ROM_LOAD16_BYTE( "a96_53.49",  0x40001, 0x10000, CRC(0173484c) SHA1(41d70039bda0965afe89251696ceaec7b7f40c24) )

	ROM_REGION( 0xc0000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "a96_44.179", 0x00000, 0x10000, CRC(bbc18878) SHA1(7732ab2a3002f8b500615377dab42ac75451cb3b) )   /* 16x16 sprites */
	ROM_LOAD32_BYTE( "a96_45.200", 0x00001, 0x10000, CRC(616cdd8b) SHA1(74e0c483a68d984a689ea1381ed3a9da2f8a410a) )
	ROM_LOAD32_BYTE( "a96_46.180", 0x00002, 0x10000, CRC(fec35418) SHA1(f0f401c3634e91b81cb8484b7b03f350d382e889) )
	ROM_LOAD32_BYTE( "a96_47.201", 0x00003, 0x10000, CRC(8df9286a) SHA1(4a197e4c38d1750cc316b8710f4a0fef4316be14) )

	ROM_LOAD32_BYTE( "a96_40.177", 0x40000, 0x10000, CRC(b699a51e) SHA1(5fd751dd44618743dc8a3df04cf0a987753a868b) )
	ROM_LOAD32_BYTE( "a96_41.198", 0x40001, 0x10000, CRC(97128a3a) SHA1(257ddd1ba71e6beeaf18e0c5d7006d1d2b6a5edf) )
	ROM_LOAD32_BYTE( "a96_42.178", 0x40002, 0x10000, CRC(7f55ee0f) SHA1(d9ba7b8fbf59308a08613d67e92da6829f6b6db3) )
	ROM_LOAD32_BYTE( "a96_43.199", 0x40003, 0x10000, CRC(c7cad469) SHA1(dbd37aa10f12e4950f8ec6bcd7d150fa55e64742) )

	ROM_LOAD32_BYTE( "a96_36.175", 0x80000, 0x10000, CRC(af598141) SHA1(f3b888bcbd4560cca48187055cbe4107e2b392a6) )
	ROM_LOAD32_BYTE( "a96_37.196", 0x80001, 0x10000, CRC(b48137c8) SHA1(03e98a93f4fa19dfe77da244c002abc84b936a22) )
	ROM_LOAD32_BYTE( "a96_38.176", 0x80002, 0x10000, CRC(e4f3e3a7) SHA1(0baa8a672516bcc4f17f40f429ac3d227de16625) )
	ROM_LOAD32_BYTE( "a96_39.197", 0x80003, 0x10000, CRC(ea30920f) SHA1(91d47b10886d6c243bc676435e300cb3b5fcca33) )

	ROM_REGION( 0x8000, "gfx3", 0 )         /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_54.143", 0x0000, 0x4000, CRC(51c02ae2) SHA1(27d2a6c649d047da1f22758569cb36531e3bf8bc) )
	ROM_LOAD16_BYTE( "a96_55.144", 0x0001, 0x4000, CRC(771e4d98) SHA1(0e8ce5d569775883f4bc777b9bd49eb23ba7b42e) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD16_BYTE( "a96-24.163", 0x0000, 0x0400, CRC(0fa8be7f) SHA1(079686b5d65b4b966591090d8c0e13e66dc5beca) ) /* proms, currently unused */
	ROM_LOAD16_BYTE( "a96-25.164", 0x0400, 0x0400, CRC(265508a6) SHA1(f8ee1c658b33ae76d8a457a4042d9b4b58247823) )
	ROM_LOAD16_BYTE( "a96-26.165", 0x0800, 0x0400, CRC(4891b9c0) SHA1(1f550a9a4ad3ca379f88f5865ed1b281c7b87f31) )
ROM_END


GAME( 1986, darius,   0,        darius,   darius,  driver_device, 0, ROT0, "Taito Corporation Japan",   "Darius (World, rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, dariusu,  darius,   darius,   dariusu, driver_device, 0, ROT0, "Taito America Corporation", "Darius (US, rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, dariusj,  darius,   darius,   dariusj, driver_device, 0, ROT0, "Taito Corporation",         "Darius (Japan, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, dariuso,  darius,   darius,   dariusj, driver_device, 0, ROT0, "Taito Corporation",         "Darius (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, dariuse,  darius,   darius,   dariusu, driver_device, 0, ROT0, "Taito Corporation",         "Darius Extra Version (Japan)", MACHINE_SUPPORTS_SAVE )
