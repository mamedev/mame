// license:BSD-3-Clause
// copyright-holders:Dan Boris, Olivier Galibert, Aaron Giles, Ryan Holtz
/***************************************************************************

    Star Fire/Fire One system

    driver by Daniel Boris, Olivier Galibert, Aaron Giles

    netlist audio by Ryan Holtz

****************************************************************************

    Memory map

****************************************************************************

    ========================================================================
    MAIN CPU
    ========================================================================
    0000-7FFF   R     xxxxxxxx   Program ROM
    8000-9FFF   R/W   xxxxxxxx   Scratch RAM, actually mapped into low VRAM
    9000          W   xxxxxxxx   VRAM write control register
                  W   xxx-----      (VRAM shift amount 1)
                  W   ---x----      (VRAM write mirror 1)
                  W   ----xxx-      (VRAM shift amount 2)
                  W   -------x      (VRAM write mirror 2)
    9001          W   xxxxxxxx   Video control register
                  W   x-------      (Color RAM source select)
                  W   -x------      (Palette RAM write enable)
                  W   --x-----      (Video RAM write enable)
                  W   ---x----      (Right side mask select)
                  W   ----xxxx      (Video RAM ALU operation)
    9800-9807   R     xxxxxxxx   Input ports
    A000-BFFF   R/W   xxxxxxxx   Color RAM
    C000-DFFF   R/W   xxxxxxxx   Video RAM, using shift/mirror 1 and color
    E000-FFFF   R/W   xxxxxxxx   Video RAM, using shift/mirror 2
    ========================================================================
    Interrupts:
       NMI generated once/frame
    ========================================================================

***************************************************************************

Notes:

starfirea has one less ROM in total than starfire, but everything passes as
OK in the ROM test, so it's probably just an earlier revision.

a Star Fire set with labels r1436-11 to r1445-11 is known to exist.

***************************************************************************/

#include "emu.h"
#include "starfire.h"

#include "cpu/z80/z80.h"
#include "speaker.h"

/*************************************
 *
 *  Scratch RAM, mapped into video RAM
 *
 *************************************/

void starfire_base_state::scratch_w(offs_t offset, uint8_t data)
{
	/* A12 and A3 select video control registers, only the low 4 addresses have an effect */
	if ((offset & 0x1008) == 0x1000)
	{
		switch (offset & 7)
		{
			case 0: m_vidctrl = data; break;
			case 1: m_vidctrl1 = data; break;
			case 2:
			case 3: sound_w(offset & 1, data); break;
			case 7: music_w((offset >> 10) & 3, data); break;
			default: break;
		}
	}

	/* convert to a videoram offset */
	offset = (offset & 0x31f) | ((offset & 0xe0) << 5);
	m_videoram[offset] = data;
}


uint8_t starfire_base_state::scratch_r(offs_t offset)
{
	/* A11 selects input ports */
	if (offset & 0x800)
		return input_r(offset);

	/* convert to a videoram offset */
	offset = (offset & 0x31f) | ((offset & 0xe0) << 5);
	return m_videoram[offset];
}



/*************************************
 *
 *  Game-specific I/O handlers
 *
 *************************************/

void starfire_state::sound_w(offs_t offset, uint8_t data)
{
	m_sound_size->write(BIT(data, 0));
	m_sound_explosion->write(BIT(data, 1));
	m_sound_tie->write(BIT(data, 2));
	m_sound_laser->write(BIT(data, 3));
	m_sound_track->write(BIT(data, 4));
	m_sound_lock->write(BIT(data, 5));
	m_sound_scanner->write(BIT(data, 6));
	m_sound_overheat->write(BIT(data, 7));
}

void fireone_state::sound_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		m_sound_left_torpedo->write(BIT(data, 0));
		m_sound_left_partial_hit->write(BIT(data, 1));
		m_sound_left_boom->write(BIT(data, 2));
		m_player_select = BIT(data, 3);
		m_pit->write_gate0(BIT(data, 4));
		m_pit->write_gate1(BIT(data, 5));
		m_pit->write_gate2(BIT(data, 6));
		m_sound_off_left->write(BIT(data, 7));  // HACK: There's only one SOUND_OFF signal, but it splits into the left and right mixer stages.
		m_sound_off_right->write(BIT(data, 7)); // Having the left and right halves split provides a better opportunity for frontier isolation.
	}
	else
	{
		m_sound_right_torpedo->write(BIT(data, 0));
		m_sound_right_partial_hit->write(BIT(data, 1));
		m_sound_right_boom->write(BIT(data, 2));
		m_sound_torpedo_collision->write(BIT(data, 3));
		m_sound_submarine_engine->write(BIT(data, 4));
		m_sound_alert->write(BIT(data, 5));
		m_sound_sonar_sync->write(BIT(data, 6));
		m_sound_sonar_enable->write(BIT(~data, 7));
	}
}

void fireone_state::music_w(offs_t offset, uint8_t data)
{
	m_pit->write(offset, data);
}

void fireone_state::music_a_out_cb(int state)
{
	m_music_a->write(state);
}

void fireone_state::music_b_out_cb(int state)
{
	m_music_b->write(state);
}

void fireone_state::music_c_out_cb(int state)
{
	m_music_c->write(state);
}

uint8_t starfire_state::input_r(offs_t offset)
{
	switch (offset & 15)
	{
		case 0: return m_dsw->read();
		case 1: return (m_system->read() & 0xe7) | m_sound_tie_on | m_sound_laser_on;
		case 5: return m_stickz->read();
		case 6: return m_stickx->read();
		case 7: return m_sticky->read();
		default: return 0xff;
	}
}

NETDEV_ANALOG_CALLBACK_MEMBER(starfire_state::tieon1_cb)
{
	m_sound_tie_on = (data > 2.5) ? 0x00 : 0x08;
}

NETDEV_ANALOG_CALLBACK_MEMBER(starfire_state::laseron1_cb)
{
	m_sound_laser_on = (data > 2.5) ? 0x00 : 0x10;
}

NETDEV_ANALOG_CALLBACK_MEMBER(starfire_state::sound_out_cb)
{
	m_dac->write(std::round(8192.0 * data));
}

uint8_t fireone_state::input_r(offs_t offset)
{
	switch (offset & 15)
	{
		case 0:
			return m_dsw->read();
		case 1:
			return m_system->read();
		case 2:
		{
			const uint8_t input = m_controls[m_player_select]->read();
			return input ^ BIT(input, 1, 5); // paddle portion is a 6-bit Gray code
		}
		default: return 0xff;
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void starfire_base_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).rw(FUNC(starfire_base_state::scratch_r), FUNC(starfire_base_state::scratch_w));
	map(0xa000, 0xbfff).rw(FUNC(starfire_base_state::colorram_r), FUNC(starfire_base_state::colorram_w)).share("colorram");
	map(0xc000, 0xffff).rw(FUNC(starfire_base_state::videoram_r), FUNC(starfire_base_state::videoram_w)).share("videoram");
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( starfire )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, "Time" )              PORT_DIPLOCATION("3A:1,2")
	PORT_DIPSETTING(    0x00, "90 Sec" )
	PORT_DIPSETTING(    0x01, "80 Sec" )
	PORT_DIPSETTING(    0x02, "70 Sec" )
	PORT_DIPSETTING(    0x03, "60 Sec" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("3A:3")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x08, 0x00, "Fuel per Coin" )     PORT_DIPLOCATION("3A:4")
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPSETTING(    0x08, "600" )
	PORT_DIPNAME( 0x30, 0x00, "Bonus" )             PORT_DIPLOCATION("3A:5,6")
	PORT_DIPSETTING(    0x00, "300 points" )
	PORT_DIPSETTING(    0x10, "500 points" )
	PORT_DIPSETTING(    0x20, "700 points" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, "Score Table Hold" )  PORT_DIPLOCATION("3A:7")
	PORT_DIPSETTING(    0x00, "fixed length" )
	PORT_DIPSETTING(    0x40, "fixed length+fire" )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_HIGH, "3A:8" )

	PORT_START("SYSTEM")    /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) // (audio) TIE ON, see input_r
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) // (audio) LASER ON, see input_r
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT ) // SLAM/STATIC
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("STICKX")    /* IN2 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("STICKY")    /* IN3 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("STICKZ")    /* IN4 */ /* throttle */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE

	PORT_START("NMI")
	PORT_CONFNAME( 0x01, 0x01, "Jumper J6/4G: Enable NMI" )
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
	PORT_CONFSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("POT_TRACK")
	PORT_ADJUSTER( 50, "Tracking Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "trackvol")

	PORT_START("POT_LASER")
	PORT_ADJUSTER( 50, "Laser Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "laservol")

	PORT_START("POT_TIE")
	PORT_ADJUSTER( 50, "Enemy Shot Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "enemyvol")

	PORT_START("POT_SIZE")
	PORT_ADJUSTER( 50, "'Size' Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "sizevol")

	PORT_START("POT_EXPLO")
	PORT_ADJUSTER( 50, "Explosion Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "explovol")

	PORT_START("POT_LOH")
	PORT_ADJUSTER( 50, "Lock/Scan/Overheat Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "lohvol")

	PORT_START("POT_MAIN")
	PORT_ADJUSTER( 50, "Main Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "mainvol")
INPUT_PORTS_END


static INPUT_PORTS_START( fireone )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW:!1,!2")
	PORT_DIPSETTING(    0x03, "2 Coins/1 Player" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 or 2 Players" )
	PORT_DIPSETTING(    0x00, "1 Coin/1 Player" )
	PORT_DIPSETTING(    0x01, "1 Coin/1 or 2 Players" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW:!3,!4")
	PORT_DIPSETTING(    0x00, "75 Sec" )
	PORT_DIPSETTING(    0x04, "90 Sec" )
	PORT_DIPSETTING(    0x08, "105 Sec" )
	PORT_DIPSETTING(    0x0c, "120 Sec" )
	PORT_DIPNAME( 0x30, 0x00, "Bonus difficulty" )      PORT_DIPLOCATION("SW:!5,!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_HIGH, "SW:!8" )

	PORT_START("SYSTEM")    /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1")    /* IN2 */
	PORT_BIT( 0x3f, 0x20, IPT_PADDLE ) PORT_MINMAX(0,63) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("P2")    /* IN3 */
	PORT_BIT( 0x3f, 0x20, IPT_PADDLE ) PORT_MINMAX(0,63) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("POT_L")
	PORT_ADJUSTER( 50, "Mixer Volume (L)" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "volume_l")

	PORT_START("POT_R")
	PORT_ADJUSTER( 50, "Mixer Volume (R)" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "volume_r")
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void starfire_state::machine_start()
{
	save_item(NAME(m_sound_tie_on));
	save_item(NAME(m_sound_laser_on));
}

void starfire_state::machine_reset()
{
	m_sound_tie_on = 0x08;
	m_sound_laser_on = 0x10;
}

void fireone_state::machine_start()
{
	save_item(NAME(m_player_select));

	m_pit->set_clockin(0, STARFIRE_CPU_CLOCK.dvalue());
	m_pit->set_clockin(1, STARFIRE_CPU_CLOCK.dvalue());
	m_pit->set_clockin(2, STARFIRE_CPU_CLOCK.dvalue());
}

void fireone_state::machine_reset()
{
	m_player_select = 0;
}

INTERRUPT_GEN_MEMBER(starfire_state::vblank_int)
{
	// starfire has a jumper for disabling NMI, used to do a complete RAM test
	if (m_nmi->read())
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INTERRUPT_GEN_MEMBER(fireone_state::vblank_int)
{
	device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void starfire_base_state::base_config(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, STARFIRE_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &starfire_base_state::main_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(STARFIRE_PIXEL_CLOCK, STARFIRE_HTOTAL, STARFIRE_HBEND, STARFIRE_HBSTART, STARFIRE_VTOTAL, STARFIRE_VBEND, STARFIRE_VBSTART);
	m_screen->set_screen_update(FUNC(starfire_base_state::screen_update));
}

void fireone_state::fireone(machine_config &config)
{
	base_config(config);
	m_maincpu->set_vblank_int("screen", FUNC(fireone_state::vblank_int));

	PIT8253(config, m_pit);
	m_pit->out_handler<0>().set(FUNC(fireone_state::music_a_out_cb));
	m_pit->out_handler<1>().set(FUNC(fireone_state::music_b_out_cb));
	m_pit->out_handler<2>().set(FUNC(fireone_state::music_c_out_cb));

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(fireone))
		.add_route(0, "speaker", 1.0, 0)
		.add_route(1, "speaker", 1.0, 1);

	NETLIST_LOGIC_INPUT(config, "sound_nl:ltorp", "LTORP.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:lshpht", "LSHPHT.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:lboom", "LBOOM.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:lsound_off", "SOUND_OFF_L.IN", 0); // HACK: Split the SOUND OFF into left/right halves for better netlist isolation.
	NETLIST_LOGIC_INPUT(config, "sound_nl:rsound_off", "SOUND_OFF_R.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:rtorp", "RTORP.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:rshpht", "RSHPHT.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:rboom", "RBOOM.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:torpcoll", "TORPCOLL.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:subeng", "SUBENG.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:alert", "ALERT.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:sonar_enable", "SONAR_ENABLE.POS", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:sonar_sync", "SONAR_SYNC.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:music_a", "MUSIC_A.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:music_b", "MUSIC_B.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:music_c", "MUSIC_C.IN", 0);
	NETLIST_ANALOG_INPUT(config, "sound_nl:volume_l", "R64.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:volume_r", "R65.DIAL");

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUT_L").set_mult_offset(100000.0 / 32768.0, 0.0);
	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout1", 1, "OUT_R").set_mult_offset(100000.0 / 32768.0, 0.0);
}

void starfire_state::starfire(machine_config &config)
{
	base_config(config);
	m_maincpu->set_vblank_int("screen", FUNC(starfire_state::vblank_int));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	NETLIST_CPU(config, "sound_nl", netlist::config::DEFAULT_CLOCK()).set_source(NETLIST_NAME(starfire));

	NETLIST_LOGIC_INPUT(config, "sound_nl:size", "SIZE.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:sexplo", "SEXPLO.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:stie", "STIE.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:slaser", "SLASER.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:track", "TRACK.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:lock", "LOCK.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:scanner", "SCANNER.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:oheat", "OHEAT.IN", 0);

	NETLIST_ANALOG_INPUT(config, "sound_nl:trackvol", "R16.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:laservol", "R18.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:enemyvol", "R17.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:sizevol", "R19.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:explovol", "R20.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:lohvol", "R15.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:mainvol", "R21.DIAL");

	NETLIST_ANALOG_OUTPUT(config, "sound_nl:tieon1", 0).set_params("TIEON1", FUNC(starfire_state::tieon1_cb));
	NETLIST_ANALOG_OUTPUT(config, "sound_nl:laseron1", 0).set_params("LASERON1", FUNC(starfire_state::laseron1_cb));
	NETLIST_ANALOG_OUTPUT(config, "sound_nl:output", 0).set_params("OUTPUT", FUNC(starfire_state::sound_out_cb));

	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.5); // Not actually a DAC, just here to receive output.
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( starfire )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r1412-11.1a",     0x0000, 0x0800, CRC(9990af64) SHA1(05eccf1084ace55be9d6cf0fccddcaa18fa5487a) )
	ROM_LOAD( "r1413-11.2a",     0x0800, 0x0800, CRC(6e17ba33) SHA1(59433696f56018a7b253491b1db3ff45546dcd46) )
	ROM_LOAD( "r1414-11.1b",     0x1000, 0x0800, CRC(946175d0) SHA1(6a55d9f6031b96e9e05d61d59a23d4fc6df724bf) )
	ROM_LOAD( "r1415-11.2b",     0x1800, 0x0800, CRC(67be4275) SHA1(dd6232e034030e0c2b4d866fda36cbe22d8518f7) )
	ROM_LOAD( "r1416-11.1c",     0x2000, 0x0800, CRC(c56b4e07) SHA1(e55ae84c484a78372180783df37750cdad8b04a2) )
	ROM_LOAD( "r1417-11.2c",     0x2800, 0x0800, CRC(b4b9d3a7) SHA1(8f3e0d67d1e94f6b1c41a78e59ac81f021aa827a) )
	ROM_LOAD( "r1418-11.1d",     0x3000, 0x0800, CRC(fd52ffb5) SHA1(c1ba2ffb7de0301a962cca2e693bfbbd9838b852) )
	ROM_LOAD( "r1419-11.2d",     0x3800, 0x0800, CRC(51c69fe3) SHA1(33159cb3ea5029d395fc20916899aa05139c2d51) )
	ROM_LOAD( "r1420-11.1e",     0x4000, 0x0800, CRC(01994ec8) SHA1(db694f922a98bb0fc585cad83bee8a88d72fca8f) )
	ROM_LOAD( "r1421-11.2e",     0x4800, 0x0800, CRC(ef3d1b71) SHA1(ca427209194f519b1ac5b94d29c2789445303dc1) )
	ROM_LOAD( "r1422-11.1f",     0x5000, 0x0800, CRC(af31dc39) SHA1(0dfeff6973fd03e85b08e70c77d212f0bb60121d) )

	ROM_REGION( 0x0040, "proms", 0 ) /* DRAM addressing */
	ROM_LOAD( "prom-1.7a",    0x0000, 0x0020, CRC(ae1f4acd) SHA1(1d502b61db73cf6a4ac3d235455a5c464f12652a) ) /* BPROM type is N82S123 */
	ROM_LOAD( "prom-2.8a",    0x0020, 0x0020, CRC(9b713924) SHA1(943ad55d232f7bb99886a9a273dd14a1e1533491) ) /* BPROM type is N82S123 */
ROM_END

ROM_START( starfirea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "starfire.1a",  0x0000, 0x0800, CRC(6adcd7e7) SHA1(a931fb80e48db3050ce3bc39f455961c0c7c56ce) )
	ROM_LOAD( "starfire.2a",  0x0800, 0x0800, CRC(835c70ea) SHA1(36828735aa48de5e3e973ca1f42ef08537e1c6ce) )
	ROM_LOAD( "starfire.1b",  0x1000, 0x0800, CRC(377afbef) SHA1(97cb5a20aeb8c70670d6db8f41b2abcb181755c6) )
	ROM_LOAD( "starfire.2b",  0x1800, 0x0800, CRC(f3a833cb) SHA1(d2e01806ead71b0946347fd9668fd3f24524734e) )
	ROM_LOAD( "starfire.1c",  0x2000, 0x0800, CRC(db625c1d) SHA1(5d0307258a73b4b82fbe7b10634076412f4ab3c7) )
	ROM_LOAD( "starfire.2c",  0x2800, 0x0800, CRC(68fa2ce6) SHA1(2b32df960bc4ec38f50f0d23ab96becb68bc4034) )
	ROM_LOAD( "starfire.1d",  0x3000, 0x0800, CRC(c6b5f1d1) SHA1(85a3f7ce7a51597609c762c9a809b84922f8a6e5) )
	ROM_LOAD( "starfire.2d",  0x3800, 0x0800, CRC(ab2a36a5) SHA1(debd9503246b4d27c8136bfb60cdffd9107ad95e) )
	ROM_LOAD( "starfire.1e",  0x4000, 0x0800, CRC(1ac8ba8c) SHA1(90c5a8a943edad74141b15e1f145598abce8cb75) )
	ROM_LOAD( "starfire.2e",  0x4800, 0x0800, CRC(ba8434c5) SHA1(1831b291dfe3e4b081e66caa909b8c727bfffa7b) )

	ROM_REGION( 0x0040, "proms", 0 ) /* DRAM addressing */
	ROM_LOAD( "prom-1.7a",    0x0000, 0x0020, CRC(ae1f4acd) SHA1(1d502b61db73cf6a4ac3d235455a5c464f12652a) ) /* BPROM type is N82S123 */
	ROM_LOAD( "prom-2.8a",    0x0020, 0x0020, CRC(9b713924) SHA1(943ad55d232f7bb99886a9a273dd14a1e1533491) ) /* BPROM type is N82S123 */
ROM_END

ROM_START( fireone )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fo-ic13.7b",   0x0000, 0x0800, CRC(f927f086) SHA1(509db84d781dd2d5aaefd561539738f0db7c4ca5) )
	ROM_LOAD( "fo-ic24.7c",   0x0800, 0x0800, CRC(0d2d8723) SHA1(e9bb2092ce7786016f15e42916ad48ef12735e9c) )
	ROM_LOAD( "fo-ic12.6b",   0x1000, 0x0800, CRC(ac7783d9) SHA1(8bcfcc5d3126382f4ec8904e0435de0931abc41e) )
	ROM_LOAD( "fo-ic23.6c",   0x1800, 0x0800, CRC(15c74ee7) SHA1(0adb87c2471ecbbd18d10579043765ce877dbde7) )
	ROM_LOAD( "fo-ic11.5b",   0x2000, 0x0800, CRC(721930a1) SHA1(826245ffbd399056a74ccd14cd2bd4acd2fb2d24) )
	ROM_LOAD( "fo-ic22.5c",   0x2800, 0x0800, CRC(f0c965b4) SHA1(ffe96e636720325d9a40b729128730446b74435b) )
	ROM_LOAD( "fo-ic10.4b",   0x3000, 0x0800, CRC(27a7b2c0) SHA1(7a8c70e565bdcb6e085e4d283f41c92758640055) )
	ROM_LOAD( "fo-ic21.4c",   0x3800, 0x0800, CRC(b142c857) SHA1(609fbd0c0b5833807fd606284c26ad7cb7e4d742) )
	ROM_LOAD( "fo-ic09.3b",   0x4000, 0x0800, CRC(1c076b1b) SHA1(874c09c81e90e1be869902057b7359e71f77db52) )
	ROM_LOAD( "fo-ic20.3c",   0x4800, 0x0800, CRC(b4ac6e71) SHA1(4731dd6865929b8c9c33cbe4cf1dde23046d6914) )
	ROM_LOAD( "fo-ic08.2b",   0x5000, 0x0800, CRC(5839e2ff) SHA1(9d8a17c5b64cdf5bf222f4dbca48f0210b18e403) )
	ROM_LOAD( "fo-ic19.2c",   0x5800, 0x0800, CRC(9fd85e11) SHA1(f8264357a63f757bc58f3703e60e219d67d0d081) )
	ROM_LOAD( "fo-ic07.1b",   0x6000, 0x0800, CRC(b90baae1) SHA1(c7dedf38e5a1977234f1f745a7aa443f6bf7db52) )
	ROM_LOAD( "fo-ic18.1c",   0x6800, 0x0800, CRC(771ee5ba) SHA1(6577e219386de594dbde8a54d5f5f9657419061a) )

	ROM_REGION( 0x0040, "proms", 0 ) /* DRAM addressing */
	ROM_LOAD( "prom-1.7a",    0x0000, 0x0020, CRC(ae1f4acd) SHA1(1d502b61db73cf6a4ac3d235455a5c464f12652a) ) /* BPROM type is N82S123 */
	ROM_LOAD( "prom-2.8a",    0x0020, 0x0020, CRC(9b713924) SHA1(943ad55d232f7bb99886a9a273dd14a1e1533491) ) /* BPROM type is N82S123 */
ROM_END

ROM_START( starfir2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sfire2.01",    0x0000, 0x0800, CRC(f75be2f4) SHA1(b15511c345363f45eee0c019aa336a9aa16e63ea) )
	ROM_LOAD( "sfire2.02",    0x0800, 0x0800, CRC(ccf98c6a) SHA1(3e7792aa47750ee19baf1e74016038fe80c92381) )
	ROM_LOAD( "sfire2.03",    0x1000, 0x0800, CRC(604b2d50) SHA1(39d402135aaaa44c1ad05e1665eb6668280fae28) )
	ROM_LOAD( "sfire2.04",    0x1800, 0x0800, CRC(f8a9658f) SHA1(aea97387001183a797375971c7325b4a838ea1d5) )
	ROM_LOAD( "sfire2.05",    0x2000, 0x0800, CRC(acbaf827) SHA1(a546340f8533557a86b589f5011e5af0439e0d4d) )
	ROM_LOAD( "sfire2.06",    0x2800, 0x0800, CRC(3525bb22) SHA1(1a1ca8b5ef1a5584d28644bdc751635aac3fad02) )
	ROM_LOAD( "sfire2.07",    0x3000, 0x0800, CRC(7fce0e54) SHA1(17355fe98cf1511c32e90434960ced7b3f3ecac7) )
	ROM_LOAD( "sfire2.08",    0x3800, 0x0800, CRC(98054c14) SHA1(4a561a9d87be9c5d4283ee78c4cf05c10c979a2f) )
	ROM_LOAD( "sfire2.09",    0x4000, 0x0800, CRC(abaa4144) SHA1(045ebcd38d6a3f75c6d819a42aa1fb92ac84755c) )
	ROM_LOAD( "sfire2.10",    0x4800, 0x0800, CRC(a0b3dadb) SHA1(d86683b528b5fbafad0cdd054940bc04b056b850) )
	ROM_LOAD( "sfire2.11",    0x5000, 0x0800, CRC(a61ebbd2) SHA1(9fdf6558306aebbf5e9e106e4f4f6f7a3e703696) )
	ROM_LOAD( "sfire2.12",    0x5800, 0x0800, CRC(a35ba06d) SHA1(122f1dbc235977367fdd06b7517c356a3147dfd1) )

	ROM_REGION( 0x0040, "proms", 0 ) /* DRAM addressing */
	ROM_LOAD( "prom-1.7a",    0x0000, 0x0020, CRC(ae1f4acd) SHA1(1d502b61db73cf6a4ac3d235455a5c464f12652a) ) /* BPROM type is N82S123 */
	ROM_LOAD( "prom-2.8a",    0x0020, 0x0020, CRC(9b713924) SHA1(943ad55d232f7bb99886a9a273dd14a1e1533491) ) /* BPROM type is N82S123 */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, starfire,  0,        starfire, starfire, starfire_state, empty_init, ROT0, "Exidy", "Star Fire (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, starfirea, starfire, starfire, starfire, starfire_state, empty_init, ROT0, "Exidy", "Star Fire (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, fireone,   0,        fireone,  fireone,  fireone_state,  empty_init, ROT0, "Exidy", "Fire One", MACHINE_SUPPORTS_SAVE )
GAME( 1979, starfir2,  0,        starfire, starfire, starfire_state, empty_init, ROT0, "Exidy", "Star Fire 2", MACHINE_SUPPORTS_SAVE )
