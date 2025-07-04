// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    HAR MadMax hardware

****************************************************************************

    Games supported:
        * Double Cheese
        * Lotto Fun 2
        * Fred Flintstones' Memory Match
        * ChuckECheese's Match Game

    Known bugs:
        * Test/tilt buttons seem to be swapped compared to test mode
        * Don't know what the opto switches do

    [dcheese]
    * you can not spin the wheel by using the inc/dec buttons unless you
      switch back and forth between them.  The game seems to check for a
      constant turn rate and constant acceleration/deceleration, then not
      allow the wheel to start spinning.  This is most likely to stop
      people from rigging the game.

    [fredmem / cecmatch]
    * Controls are set up as a 3 x 3 matrix of buttons that match the 9
      positions on the screen.

**************************************************************************/


#include "emu.h"
#include "dcheese.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m6809/m6809.h"
#include "machine/eepromser.h"
#include "machine/ticket.h"
#include "machine/watchdog.h"
#include "speaker.h"


#define MAIN_OSC    14318180
#define SOUND_OSC   24000000


/*************************************
 *
 *  Interrupts
 *
 *************************************/

void dcheese_state::update_irq_state()
{
	for (int i = 1; i < 5; i++)
		m_maincpu->set_input_line(i, m_irq_state[i] ? ASSERT_LINE : CLEAR_LINE);
}


u8 dcheese_state::iack_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		/* auto-ack the IRQ */
		m_irq_state[offset] = 0;
		update_irq_state();
	}

	/* vector is 0x40 + index */
	return 0x40 + offset;
}


TIMER_CALLBACK_MEMBER(dcheese_state::signal_irq)
{
	m_irq_state[param] = 1;
	update_irq_state();
}


void dcheese_state::vblank(int state)
{
	if (state)
	{
		logerror("---- VBLANK ----\n");
		signal_irq(4);
	}
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

void dcheese_state::machine_start()
{
	save_item(NAME(m_irq_state));
	save_item(NAME(m_sound_control));
	save_item(NAME(m_sound_msb_latch));
}



/*************************************
 *
 *  I/O ports
 *
 *************************************/

void dcheese_state::eeprom_control_w(offs_t offset, u16 data, u16 mem_mask)
{
	/* toggles bit $0100 very frequently while waiting for things */
	/* bits $0080-$0010 are probably lamps */
	if (ACCESSING_BITS_0_7)
	{
		m_eepromout_io->write(data, 0xff);
	}
}



/*************************************
 *
 *  Sound CPU handlers
 *
 *************************************/

u8 dcheese_state::sound_status_r()
{
	/* seems to be ready signal on BSMT or latching hardware */
	return m_bsmt->read_status() << 7;
}


void dcheese_state::sound_control_w(u8 data)
{
	u8 const diff = data ^ m_sound_control;
	m_sound_control = data;

	/* bit 0x20 = LED */
	/* bit 0x40 = BSMT2000 reset */
	if ((diff & 0x40) && (data & 0x40))
		m_bsmt->reset();
	if (data != 0x40 && data != 0x60)
		logerror("%04X:sound_control_w = %02X\n", m_audiocpu->pc(), data);
}


void dcheese_state::bsmt_data_w(offs_t offset, u8 data)
{
	/* writes come in pairs; even bytes latch, odd bytes write */
	if ((offset & 1) == 0)
	{
		m_bsmt->write_reg(offset >> 1);
		m_sound_msb_latch = data;
	}
	else
		m_bsmt->write_data((m_sound_msb_latch << 8) | data);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void dcheese_state::main_cpu_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x03ffff).rom().region("maincpu", 0);
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x200001).portr("200000").w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x220000, 0x220001).portr("220000").w(FUNC(dcheese_state::blitter_color_w));
	map(0x240000, 0x240001).portr("240000").w(FUNC(dcheese_state::eeprom_control_w));
	map(0x260000, 0x26001f).w(FUNC(dcheese_state::blitter_xparam_w));
	map(0x280000, 0x28001f).w(FUNC(dcheese_state::blitter_yparam_w));
	map(0x2a0000, 0x2a003f).rw(FUNC(dcheese_state::blitter_vidparam_r), FUNC(dcheese_state::blitter_vidparam_w));
	map(0x2e0001, 0x2e0001).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x300000, 0x300001).w(FUNC(dcheese_state::blitter_unknown_w));
}

void dcheese_state::main_fc7_map(address_map &map)
{
	map(0xfffff0, 0xfffff9).r(FUNC(dcheese_state::iack_r)).umask16(0x00ff);
}


/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

void dcheese_state::sound_cpu_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0000).mirror(0x07ff).rw(FUNC(dcheese_state::sound_status_r), FUNC(dcheese_state::sound_control_w));
	map(0x0800, 0x0800).mirror(0x07ff).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x1000, 0x10ff).mirror(0x0700).w(FUNC(dcheese_state::bsmt_data_w));
	map(0x1800, 0x1fff).ram();
	map(0x2000, 0xffff).rom().region("audiocpu", 0x2000);
}



/*************************************
 *
 *  Input port definitions
 *
 *************************************/

static INPUT_PORTS_START( dcheese )
	PORT_START("200000")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x000c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE )      /* says tilt */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_TILT )         /* says test */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 )      /* bump left */
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 )      /* bump right */
	PORT_BIT( 0x1800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )      /* brake right */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 )      /* brake left */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("220000")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("240000")
	PORT_BIT( 0x001f, IP_ACTIVE_LOW, IPT_UNKNOWN )      /* low 5 bits read as a unit */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM )     /* sound->main buffer status (0=empty) */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundlatch", FUNC(generic_latch_8_device::pending_r))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("2a0002")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNKNOWN )  // read as a unit
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON7 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON8 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 )  // opto 1
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON6 )  // opto 2
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_CUSTOM )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("2a000e")
	PORT_BIT( 0x00ff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_REVERSE
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("ticket", FUNC(ticket_dispenser_device::motor_w))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
INPUT_PORTS_END


static INPUT_PORTS_START( lottof2 )
	PORT_START("200000")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x000c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x1f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 )      /* button */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 )      /* ticket */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("220000")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("240000")
	PORT_BIT( 0x001f, IP_ACTIVE_LOW, IPT_UNKNOWN )      /* low 5 bits read as a unit */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM )     /* sound->main buffer status (0=empty) */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundlatch", FUNC(generic_latch_8_device::pending_r))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("2a0002")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_CUSTOM )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("2a000e")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("ticket", FUNC(ticket_dispenser_device::motor_w))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
INPUT_PORTS_END


static INPUT_PORTS_START( fredmem )
	PORT_START("200000")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x000c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x1f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("220000")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("240000")
	PORT_BIT( 0x001f, IP_ACTIVE_LOW, IPT_UNKNOWN )      /* low 5 bits read as a unit */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM )     /* sound->main buffer status (0=empty) */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundlatch", FUNC(generic_latch_8_device::pending_r))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("2a0002")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_CUSTOM )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("2a000e")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("ticket", FUNC(ticket_dispenser_device::motor_w))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void dcheese_state::dcheese(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, MAIN_OSC);
	m_maincpu->set_addrmap(AS_PROGRAM, &dcheese_state::main_cpu_map);
	m_maincpu->set_addrmap(m68000_device::AS_CPU_SPACE, &dcheese_state::main_fc7_map);

	M6809(config, m_audiocpu, SOUND_OSC/16); // TODO : Unknown CPU type
	m_audiocpu->set_addrmap(AS_PROGRAM, &dcheese_state::sound_cpu_map);
	m_audiocpu->set_periodic_int(FUNC(dcheese_state::irq1_line_hold), attotime::from_hz(480));   /* accurate for fredmem */

	EEPROM_93C46_16BIT(config, "eeprom");

	TICKET_DISPENSER(config, "ticket", attotime::from_msec(200));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(360, 262);  /* guess, need to see what the games write to the vid registers */
	m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(dcheese_state::screen_update));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set(FUNC(dcheese_state::vblank));

	PALETTE(config, "palette", FUNC(dcheese_state::dcheese_palette), 65536);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	BSMT2000(config, m_bsmt, SOUND_OSC);
	m_bsmt->add_route(0, "speaker", 1.2, 0);
	m_bsmt->add_route(1, "speaker", 1.2, 1);
}


void dcheese_state::fredmem(machine_config &config)
{
	dcheese(config);

	m_screen->set_visarea(0, 359, 0, 239);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

/*
    Double Cheese (c) 1993 HAR

    CPU: 68000
    Sound: 6809, BSMt2000 D15505N
    RAM: 84256 (x2), 5116
    Other: TRW9312HH (x2), LSI L1A6017 (MAX1 EXIT)

    Notes: PCB labeled "Exit Entertainment MADMAX version 5". Title screen reports
    (c)1993 Midway Manufacturing. ROM labels (c) 1993 HAR
*/
ROM_START( dcheese )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "dchez.104", 0x00000, 0x20000, CRC(5b6233d8) SHA1(7fdb606b5780dd8f45db07d3ee50e14a27f39533) )
	ROM_LOAD16_BYTE( "dchez.103", 0x00001, 0x20000, CRC(599c73ff) SHA1(f33e617ab7e9489c52b2434cfc61a5e1696e9400) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* M6809 */
	ROM_LOAD( "dchez.102", 0x8000, 0x8000, CRC(5d110061) SHA1(10d852a408a75979b8e8843afc7b39737ca2c6c8) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "dchez.123", 0x00000, 0x40000, CRC(2293dd9a) SHA1(3f0550c2a6f59a233c5b1010cecdb19404170dc0) )
	ROM_LOAD( "dchez.127", 0x40000, 0x40000, CRC(372f9d67) SHA1(74f73f0344bfb890b5e457fcde3d82c9106e7edd) )
	ROM_LOAD( "dchez.125", 0x80000, 0x40000, CRC(ddf28bab) SHA1(0f3bc86d0db7afebf8c6094b8337e5f343a82f29) )

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "dchez.ar0", 0x000000, 0x40000, CRC(6a9e2b12) SHA1(f7cb4d6b4a459682a68f734b2b2e27e3639b9ed5) )
	ROM_RELOAD(            0x040000, 0x40000 )
	ROM_RELOAD(            0x080000, 0x40000 )
	ROM_RELOAD(            0x0c0000, 0x40000 )
	ROM_LOAD( "dchez.ar1", 0x100000, 0x40000, CRC(5f3a5f41) SHA1(30e0c7b2ab43a3224432204a9388d509a6a06a11) )
	ROM_RELOAD(            0x140000, 0x40000 )
	ROM_RELOAD(            0x180000, 0x40000 )
	ROM_RELOAD(            0x1c0000, 0x40000 )
	ROM_LOAD( "dchez.ar2", 0x200000, 0x20000, CRC(d79b0d41) SHA1(cc84ddf6635097ba0aad2f1838ad0606c5bb8166) )
	ROM_RELOAD(            0x220000, 0x20000 )
	ROM_RELOAD(            0x240000, 0x20000 )
	ROM_RELOAD(            0x260000, 0x20000 )
	ROM_RELOAD(            0x280000, 0x20000 )
	ROM_RELOAD(            0x2a0000, 0x20000 )
	ROM_RELOAD(            0x2c0000, 0x20000 )
	ROM_RELOAD(            0x2e0000, 0x20000 )
	ROM_LOAD( "dchez.ar3", 0x300000, 0x20000, CRC(2056c1fd) SHA1(4c44930fb87ea6ad71326cc29313f3b817919d08) )
	ROM_RELOAD(            0x320000, 0x20000 )
	ROM_RELOAD(            0x340000, 0x20000 )
	ROM_RELOAD(            0x360000, 0x20000 )
	ROM_RELOAD(            0x380000, 0x20000 )
	ROM_RELOAD(            0x3a0000, 0x20000 )
	ROM_RELOAD(            0x3c0000, 0x20000 )
	ROM_RELOAD(            0x3e0000, 0x20000 )

	ROM_REGION16_LE( 0x20000, "palrom", 0 )
	ROM_LOAD16_BYTE( "dchez.144", 0x00000, 0x10000, CRC(52c96252) SHA1(46de465c25e4602aa360336315b3c8e1a9a0b5f3) )
	ROM_LOAD16_BYTE( "dchez.145", 0x00001, 0x10000, CRC(a11b92d0) SHA1(265f93cb3657910aabca21ed8afbb55bdc86a964) )
ROM_END


ROM_START( lottof2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "u104.r20", 0x00000, 0x20000, CRC(0dfa710e) SHA1(b28676caf2074822e87bd213d76a892bcce07c1a) )
	ROM_LOAD16_BYTE( "u103.r20", 0x00001, 0x20000, CRC(1bcd7c77) SHA1(891f066cbcf558e7a725154758cf5a7a58a4400a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* M6809 */
	ROM_LOAD( "u102.r10", 0x8000, 0x8000, CRC(fcb34c81) SHA1(f80cef85d0f4218c88c01b238f10eff2c6241d33) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "u123.r10", 0x00000, 0x40000, CRC(dbcdb5aa) SHA1(7473c5e0fc1a40a39e148277b4094fe1338d988c) )
	ROM_LOAD( "u127.r10", 0x40000, 0x40000, CRC(029ffed9) SHA1(63ba56277745ebea7c2c2b3738790cd2f4ddbe00) )
	ROM_LOAD( "u125.r10", 0x80000, 0x40000, CRC(c70cf1c6) SHA1(eb5f0c5f7485d92ce569ad915b9f5c3c48338172) )
	ROM_LOAD( "u129.r10", 0xc0000, 0x40000, CRC(e9c9e4b0) SHA1(02a3bc279e2489fd53f9a08df5f1023f75fff4d1) )

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "arom0.r10", 0x000000, 0x40000, CRC(05e7581b) SHA1(e12be200abfbef269fc085d6c5efea106487e05f) )
	ROM_RELOAD(            0x040000, 0x40000 )
	ROM_RELOAD(            0x080000, 0x40000 )
	ROM_RELOAD(            0x0c0000, 0x40000 )
	ROM_LOAD( "arom1.r10", 0x100000, 0x20000, CRC(6c4ebbfd) SHA1(2b396d96ce8903e5e8d455ce019422b744f3c4d5) )
	ROM_RELOAD(            0x120000, 0x20000 )
	ROM_RELOAD(            0x140000, 0x20000 )
	ROM_RELOAD(            0x160000, 0x20000 )
	ROM_RELOAD(            0x180000, 0x20000 )
	ROM_RELOAD(            0x1a0000, 0x20000 )
	ROM_RELOAD(            0x1c0000, 0x20000 )
	ROM_RELOAD(            0x1e0000, 0x20000 )
	ROM_LOAD( "arom2.r10", 0x200000, 0x20000, CRC(fbe9fbbb) SHA1(457fc3c0d33cf430e5969f4fa11317f1f930351b) )
	ROM_RELOAD(            0x220000, 0x20000 )
	ROM_RELOAD(            0x240000, 0x20000 )
	ROM_RELOAD(            0x260000, 0x20000 )
	ROM_RELOAD(            0x280000, 0x20000 )
	ROM_RELOAD(            0x2a0000, 0x20000 )
	ROM_RELOAD(            0x2c0000, 0x20000 )
	ROM_RELOAD(            0x2e0000, 0x20000 )
	ROM_LOAD( "arom3.r10", 0x300000, 0x20000, CRC(ffb6e463) SHA1(1349455d2ce8eb141bc0fa5219f5e7c52ee969dc) )
	ROM_RELOAD(            0x320000, 0x20000 )
	ROM_RELOAD(            0x340000, 0x20000 )
	ROM_RELOAD(            0x360000, 0x20000 )
	ROM_RELOAD(            0x380000, 0x20000 )
	ROM_RELOAD(            0x3a0000, 0x20000 )
	ROM_RELOAD(            0x3c0000, 0x20000 )
	ROM_RELOAD(            0x3e0000, 0x20000 )

	ROM_REGION16_LE( 0x20000, "palrom", 0 )
	ROM_LOAD16_BYTE( "u144.r10", 0x00000, 0x10000, CRC(3b9d5d9e) SHA1(b3fbfeb41c62c689a825dfe9487917a927a71f58) )
	ROM_LOAD16_BYTE( "u145.r10", 0x00001, 0x10000, CRC(e5a022a4) SHA1(567a37d24b36ca01a2ac3c40a0392cf97b1eb948) )
ROM_END


ROM_START( fredmem )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "prog0.104", 0x00000, 0x20000, CRC(9e90ebc3) SHA1(ef86e5070ec64772b8e8b9b30910b88bbd46285b) ) /* Program 0 - V2.0 at U104 */
	ROM_LOAD16_BYTE( "prog1.103", 0x00001, 0x20000, CRC(79cadede) SHA1(bfc04edf6dc3beb942ffba442fe4203d1e1a3c0e) ) /* Program 1 - V2.0 at U103 */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* M6809 */
	ROM_LOAD( "prog.102", 0x00000, 0x10000, CRC(b1526a1a) SHA1(456c44a0a908b3cd054b7c6741d7a1033c9b12fb) ) /* Sound Program 6809 code at U102 */

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "art-rom.123", 0x000000, 0x80000, CRC(48133505) SHA1(60f69b053e67256928db57e0a5335bbd5a72ddfc) ) /* Graphics / Art at U123 */
	ROM_LOAD( "art-rom.125", 0x080000, 0x80000, CRC(8181e154) SHA1(4d16b84ad52d8e3d3bcad3fdf5f8da23df198d46) ) /* Graphics / Art at U125 */
	ROM_LOAD( "art-rom.127", 0x100000, 0x80000, CRC(93095f3b) SHA1(de746829e04bf153024e94e6ef0ceffb1eae2b14) ) /* Graphics / Art at U127 */
	ROM_LOAD( "art-rom.129", 0x180000, 0x80000, CRC(d5715a02) SHA1(b7d9d29f2fc5d74adff1fefce312e6472c0f7565) ) /* Graphics / Art at U129 */

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "arom0", 0x000000, 0x80000, CRC(3b85ea34) SHA1(0a68e7df20a2c36e230c7935415dd5068c338669) )
	ROM_RELOAD(        0x080000, 0x80000 )
	ROM_LOAD( "arom1", 0x100000, 0x80000, CRC(405df3d4) SHA1(190b928789a879408beadd1647136bd85b018c63) )
	ROM_RELOAD(        0x180000, 0x80000 )
	ROM_LOAD( "arom2", 0x200000, 0x80000, CRC(48ecd5c9) SHA1(6aad371db7b658454c5feed548ffd19b81a8fcf4) )
	ROM_RELOAD(        0x280000, 0x80000 )
	ROM_LOAD( "arom3", 0x300000, 0x80000, CRC(411900b0) SHA1(ddc5b387c89baab0fd5c654f3768c6e27972c06a) )
	ROM_RELOAD(        0x380000, 0x80000 )

	ROM_REGION16_LE( 0x20000, "palrom", 0 )
	ROM_LOAD16_BYTE( "0.144", 0x00000, 0x10000, CRC(793c4bda) SHA1(5a8a2981b48922f4d9e617a9bf9ef6a47ab702b7) ) /* Palette - 0 at U144 */
	ROM_LOAD16_BYTE( "1.145", 0x00001, 0x10000, CRC(fe2c3521) SHA1(896e53427c7831620ca565be9c0b76aabc36b9f4) ) /* Palette - 1 at U145 */

	ROM_REGION( 0x100, "user2", 0 )
	ROM_LOAD16_BYTE( "93c46.u158", 0x00000, 0x0080, CRC(a40a7b87) SHA1(3632b7538b3bf41ee0cbe7541a0f5951f70b4a9b) ) /* EEPROM data at U158 */
ROM_END


ROM_START( fredmemus )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "u104.us.hiscore", 0x00000, 0x20000, CRC(4460c690) SHA1(08fec2704baac4b83add8f1d5936f15336a67599) ) /* Program 0 - V2.0 at U104 */
	ROM_LOAD16_BYTE( "u103.us.hiscore", 0x00001, 0x20000, CRC(ff5bfdc3) SHA1(c38b856d6a74df68bfc6fb15b521180f78742d45) ) /* Program 1 - V2.0 at U103 */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* M6809 */
	ROM_LOAD( "prog.102", 0x00000, 0x10000, CRC(b1526a1a) SHA1(456c44a0a908b3cd054b7c6741d7a1033c9b12fb) ) /* Sound Program 6809 code at U102 */

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "art-rom.123", 0x000000, 0x80000, CRC(48133505) SHA1(60f69b053e67256928db57e0a5335bbd5a72ddfc) ) /* Graphics / Art at U123 */
	ROM_LOAD( "art-rom.125", 0x080000, 0x80000, CRC(8181e154) SHA1(4d16b84ad52d8e3d3bcad3fdf5f8da23df198d46) ) /* Graphics / Art at U125 */
	ROM_LOAD( "art-rom.127", 0x100000, 0x80000, CRC(93095f3b) SHA1(de746829e04bf153024e94e6ef0ceffb1eae2b14) ) /* Graphics / Art at U127 */
	ROM_LOAD( "art-rom.129", 0x180000, 0x80000, CRC(d5715a02) SHA1(b7d9d29f2fc5d74adff1fefce312e6472c0f7565) ) /* Graphics / Art at U129 */

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "arom0", 0x000000, 0x80000, CRC(3b85ea34) SHA1(0a68e7df20a2c36e230c7935415dd5068c338669) )
	ROM_RELOAD(        0x080000, 0x80000 )
	ROM_LOAD( "arom1", 0x100000, 0x80000, CRC(405df3d4) SHA1(190b928789a879408beadd1647136bd85b018c63) )
	ROM_RELOAD(        0x180000, 0x80000 )
	ROM_LOAD( "arom2", 0x200000, 0x80000, CRC(48ecd5c9) SHA1(6aad371db7b658454c5feed548ffd19b81a8fcf4) )
	ROM_RELOAD(        0x280000, 0x80000 )
	ROM_LOAD( "arom3", 0x300000, 0x80000, CRC(411900b0) SHA1(ddc5b387c89baab0fd5c654f3768c6e27972c06a) )
	ROM_RELOAD(        0x380000, 0x80000 )

	ROM_REGION16_LE( 0x20000, "palrom", 0 )
	ROM_LOAD16_BYTE( "0.144", 0x00000, 0x10000, CRC(793c4bda) SHA1(5a8a2981b48922f4d9e617a9bf9ef6a47ab702b7) ) /* Palette - 0 at U144 */
	ROM_LOAD16_BYTE( "1.145", 0x00001, 0x10000, CRC(fe2c3521) SHA1(896e53427c7831620ca565be9c0b76aabc36b9f4) ) /* Palette - 1 at U145 */

	ROM_REGION( 0x100, "user2", 0 )
	ROM_LOAD16_BYTE( "93c46.u158", 0x00000, 0x0080, CRC(a40a7b87) SHA1(3632b7538b3bf41ee0cbe7541a0f5951f70b4a9b) ) /* EEPROM data at U158 */
ROM_END

ROM_START( fredmemuk )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "u104.uk", 0x00000, 0x20000, CRC(e810daab) SHA1(99be21eb5df49fd8b665935c774798be270e0f27) ) /* Program 0 - V2.0 at U104 */
	ROM_LOAD16_BYTE( "u103.uk", 0x00001, 0x20000, CRC(0f2e65fb) SHA1(533a45d2de0ee3c306197d2559355c3193f9ac6b) ) /* Program 1 - V2.0 at U103 */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* M6809 */
	ROM_LOAD( "prog.102", 0x00000, 0x10000, CRC(b1526a1a) SHA1(456c44a0a908b3cd054b7c6741d7a1033c9b12fb) ) /* Sound Program 6809 code at U102 */

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "art-rom.123", 0x000000, 0x80000, CRC(48133505) SHA1(60f69b053e67256928db57e0a5335bbd5a72ddfc) ) /* Graphics / Art at U123 */
	ROM_LOAD( "art-rom.125", 0x080000, 0x80000, CRC(8181e154) SHA1(4d16b84ad52d8e3d3bcad3fdf5f8da23df198d46) ) /* Graphics / Art at U125 */
	ROM_LOAD( "art-rom.127", 0x100000, 0x80000, CRC(93095f3b) SHA1(de746829e04bf153024e94e6ef0ceffb1eae2b14) ) /* Graphics / Art at U127 */
	ROM_LOAD( "art-rom.129", 0x180000, 0x80000, CRC(d5715a02) SHA1(b7d9d29f2fc5d74adff1fefce312e6472c0f7565) ) /* Graphics / Art at U129 */

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "arom0", 0x000000, 0x80000, CRC(3b85ea34) SHA1(0a68e7df20a2c36e230c7935415dd5068c338669) )
	ROM_RELOAD(        0x080000, 0x80000 )
	ROM_LOAD( "arom1", 0x100000, 0x80000, CRC(405df3d4) SHA1(190b928789a879408beadd1647136bd85b018c63) )
	ROM_RELOAD(        0x180000, 0x80000 )
	ROM_LOAD( "arom2", 0x200000, 0x80000, CRC(48ecd5c9) SHA1(6aad371db7b658454c5feed548ffd19b81a8fcf4) )
	ROM_RELOAD(        0x280000, 0x80000 )
	ROM_LOAD( "arom3", 0x300000, 0x80000, CRC(411900b0) SHA1(ddc5b387c89baab0fd5c654f3768c6e27972c06a) )
	ROM_RELOAD(        0x380000, 0x80000 )

	ROM_REGION16_LE( 0x20000, "palrom", 0 )
	ROM_LOAD16_BYTE( "0.144", 0x00000, 0x10000, CRC(793c4bda) SHA1(5a8a2981b48922f4d9e617a9bf9ef6a47ab702b7) ) /* Palette - 0 at U144 */
	ROM_LOAD16_BYTE( "1.145", 0x00001, 0x10000, CRC(fe2c3521) SHA1(896e53427c7831620ca565be9c0b76aabc36b9f4) ) /* Palette - 1 at U145 */

	ROM_REGION( 0x100, "user2", 0 )
	ROM_LOAD16_BYTE( "93c46.u158", 0x00000, 0x0080, CRC(a40a7b87) SHA1(3632b7538b3bf41ee0cbe7541a0f5951f70b4a9b) ) /* EEPROM data at U158 */
ROM_END

/* Japan version, has a High Score table instead of tickets */
ROM_START( fredmemj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "prog0_japan.104", 0x00000, 0x20000, CRC(4f5e947e) SHA1(14c19832f98a14293a66e64d2d86e8c5cc8a9324) ) /* Program 0 - V2.0 at U104 */
	ROM_LOAD16_BYTE( "prog1_japan.103", 0x00001, 0x20000, CRC(2df6affb) SHA1(d1d28090a857cb0b0464986c446b189e7911d3d3) ) /* Program 1 - V2.0 at U103 */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* M6809 */
	ROM_LOAD( "prog.102", 0x00000, 0x10000, CRC(b1526a1a) SHA1(456c44a0a908b3cd054b7c6741d7a1033c9b12fb) ) /* Sound Program 6809 code at U102 */

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "art-rom.123", 0x000000, 0x80000, CRC(48133505) SHA1(60f69b053e67256928db57e0a5335bbd5a72ddfc) ) /* Graphics / Art at U123 */
	ROM_LOAD( "art-rom_japan.125", 0x080000, 0x80000, CRC(7bfd9b92) SHA1(306f276cf4574587fb4421c2b214522ee2b53774) ) /* Graphics / Art at U125 */
	ROM_LOAD( "art-rom.127", 0x100000, 0x80000, CRC(93095f3b) SHA1(de746829e04bf153024e94e6ef0ceffb1eae2b14) ) /* Graphics / Art at U127 */
	ROM_LOAD( "art-rom_japan.129", 0x180000, 0x80000, CRC(aaaddc7b) SHA1(27e4d31a904a451249affda2226c6556e24bfaf6) ) /* Graphics / Art at U129 */

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "arom0", 0x000000, 0x80000, CRC(3b85ea34) SHA1(0a68e7df20a2c36e230c7935415dd5068c338669) )
	ROM_RELOAD(        0x080000, 0x80000 )
	ROM_LOAD( "arom1", 0x100000, 0x80000, CRC(405df3d4) SHA1(190b928789a879408beadd1647136bd85b018c63) )
	ROM_RELOAD(        0x180000, 0x80000 )
	ROM_LOAD( "arom2", 0x200000, 0x80000, CRC(48ecd5c9) SHA1(6aad371db7b658454c5feed548ffd19b81a8fcf4) )
	ROM_RELOAD(        0x280000, 0x80000 )
	ROM_LOAD( "arom3", 0x300000, 0x80000, CRC(411900b0) SHA1(ddc5b387c89baab0fd5c654f3768c6e27972c06a) )
	ROM_RELOAD(        0x380000, 0x80000 )

	ROM_REGION16_LE( 0x20000, "palrom", 0 )
	ROM_LOAD16_BYTE( "0.144", 0x00000, 0x10000, CRC(793c4bda) SHA1(5a8a2981b48922f4d9e617a9bf9ef6a47ab702b7) ) /* Palette - 0 at U144 */
	ROM_LOAD16_BYTE( "1.145", 0x00001, 0x10000, CRC(fe2c3521) SHA1(896e53427c7831620ca565be9c0b76aabc36b9f4) ) /* Palette - 1 at U145 */

	ROM_REGION( 0x100, "user2", 0 )
	ROM_LOAD16_BYTE( "93c46.u158", 0x00000, 0x0080, CRC(a40a7b87) SHA1(3632b7538b3bf41ee0cbe7541a0f5951f70b4a9b) ) /* EEPROM data at U158 */
ROM_END

ROM_START( fredmemc )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "u104.mandarin", 0x00000, 0x20000, CRC(f46e4af6) SHA1(3bc5a7e7db7bcf86e4e8ab5df0c8bff89398d8c5) ) /* Program 0 - V2.0 at U104 */
	ROM_LOAD16_BYTE( "u103.mandarin", 0x00001, 0x20000, CRC(160a7f47) SHA1(14704d1618320b2155c6387d03ac006b3b64fc58) ) /* Program 1 - V2.0 at U103 */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* M6809 */
	ROM_LOAD( "prog.102", 0x00000, 0x10000, CRC(b1526a1a) SHA1(456c44a0a908b3cd054b7c6741d7a1033c9b12fb) ) /* Sound Program 6809 code at U102 */

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "art-rom.123", 0x000000, 0x80000, CRC(48133505) SHA1(60f69b053e67256928db57e0a5335bbd5a72ddfc) ) /* Graphics / Art at U123 */
	ROM_LOAD( "art-rom.125.mandarin", 0x080000, 0x80000, CRC(780c06fa) SHA1(34aa420fb8a628b8cb92b0975e602d8c676c608a) ) /* Graphics / Art at U125 */
	ROM_LOAD( "art-rom.127", 0x100000, 0x80000, CRC(93095f3b) SHA1(de746829e04bf153024e94e6ef0ceffb1eae2b14) ) /* Graphics / Art at U127 */
	ROM_LOAD( "art-rom.129.mandarin", 0x180000, 0x80000, CRC(31444b3f) SHA1(dd3930fd784e685a05b7fc8039e6542710861ae5) ) /* Graphics / Art at U129 */

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "arom0", 0x000000, 0x80000, CRC(3b85ea34) SHA1(0a68e7df20a2c36e230c7935415dd5068c338669) )
	ROM_RELOAD(        0x080000, 0x80000 )
	ROM_LOAD( "arom1", 0x100000, 0x80000, CRC(405df3d4) SHA1(190b928789a879408beadd1647136bd85b018c63) )
	ROM_RELOAD(        0x180000, 0x80000 )
	ROM_LOAD( "arom2", 0x200000, 0x80000, CRC(48ecd5c9) SHA1(6aad371db7b658454c5feed548ffd19b81a8fcf4) )
	ROM_RELOAD(        0x280000, 0x80000 )
	ROM_LOAD( "arom3", 0x300000, 0x80000, CRC(411900b0) SHA1(ddc5b387c89baab0fd5c654f3768c6e27972c06a) )
	ROM_RELOAD(        0x380000, 0x80000 )

	ROM_REGION16_LE( 0x20000, "palrom", 0 )
	ROM_LOAD16_BYTE( "0.144", 0x00000, 0x10000, CRC(793c4bda) SHA1(5a8a2981b48922f4d9e617a9bf9ef6a47ab702b7) ) /* Palette - 0 at U144 */
	ROM_LOAD16_BYTE( "1.145", 0x00001, 0x10000, CRC(fe2c3521) SHA1(896e53427c7831620ca565be9c0b76aabc36b9f4) ) /* Palette - 1 at U145 */

	ROM_REGION( 0x100, "user2", 0 )
	ROM_LOAD16_BYTE( "93c46.u158", 0x00000, 0x0080, CRC(a40a7b87) SHA1(3632b7538b3bf41ee0cbe7541a0f5951f70b4a9b) ) /* EEPROM data at U158 */
ROM_END

ROM_START( fredmesp )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "u104.spanish", 0x00000, 0x20000, CRC(ba150de6) SHA1(57aedc2c96309d6b5b67090e24e1e672404d34bf) ) /* Program 0 - V2.0 at U104 */
	ROM_LOAD16_BYTE( "u103.spanish", 0x00001, 0x20000, CRC(4af72eb0) SHA1(c0addfc2900fb41c24ecf9b052ef1854206a4cba) ) /* Program 1 - V2.0 at U103 */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* M6809 */
	ROM_LOAD( "prog.102", 0x00000, 0x10000, CRC(b1526a1a) SHA1(456c44a0a908b3cd054b7c6741d7a1033c9b12fb) ) /* Sound Program 6809 code at U102 */

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "art-rom.123", 0x000000, 0x80000, CRC(48133505) SHA1(60f69b053e67256928db57e0a5335bbd5a72ddfc) ) /* Graphics / Art at U123 */
	ROM_LOAD( "art-rom.125.spanish", 0x080000, 0x80000, CRC(3ee88ec8) SHA1(24f3d548fe47b92d68904e1cd6233f75b109772c) ) /* Graphics / Art at U125 */
	ROM_LOAD( "art-rom.127", 0x100000, 0x80000, CRC(93095f3b) SHA1(de746829e04bf153024e94e6ef0ceffb1eae2b14) ) /* Graphics / Art at U127 */
	ROM_LOAD( "art-rom.129.spanish", 0x180000, 0x80000, CRC(8f0fa246) SHA1(10eef16f41c82224d369fd6b7c2fa9212e22fb42) ) /* Graphics / Art at U129 */

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "arom0", 0x000000, 0x80000, CRC(3b85ea34) SHA1(0a68e7df20a2c36e230c7935415dd5068c338669) )
	ROM_RELOAD(        0x080000, 0x80000 )
	ROM_LOAD( "arom1", 0x100000, 0x80000, CRC(405df3d4) SHA1(190b928789a879408beadd1647136bd85b018c63) )
	ROM_RELOAD(        0x180000, 0x80000 )
	ROM_LOAD( "arom2", 0x200000, 0x80000, CRC(48ecd5c9) SHA1(6aad371db7b658454c5feed548ffd19b81a8fcf4) )
	ROM_RELOAD(        0x280000, 0x80000 )
	ROM_LOAD( "arom3", 0x300000, 0x80000, CRC(411900b0) SHA1(ddc5b387c89baab0fd5c654f3768c6e27972c06a) )
	ROM_RELOAD(        0x380000, 0x80000 )

	ROM_REGION16_LE( 0x20000, "palrom", 0 )
	ROM_LOAD16_BYTE( "0.144", 0x00000, 0x10000, CRC(793c4bda) SHA1(5a8a2981b48922f4d9e617a9bf9ef6a47ab702b7) ) /* Palette - 0 at U144 */
	ROM_LOAD16_BYTE( "1.145", 0x00001, 0x10000, CRC(fe2c3521) SHA1(896e53427c7831620ca565be9c0b76aabc36b9f4) ) /* Palette - 1 at U145 */

	ROM_REGION( 0x100, "user2", 0 )
	ROM_LOAD16_BYTE( "93c46.u158", 0x00000, 0x0080, CRC(a40a7b87) SHA1(3632b7538b3bf41ee0cbe7541a0f5951f70b4a9b) ) /* EEPROM data at U158 */
ROM_END


ROM_START( cecmatch )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "prog0.104", 0x00000, 0x20000, CRC(b13585e2) SHA1(dbf6db79e319157b5ac540471348682b45508c1f) ) /* Program 0 - V2.0 at U104 */
	ROM_LOAD16_BYTE( "prog1.103", 0x00001, 0x20000, CRC(5baf4f50) SHA1(e7529a4cffa292a491093a74f9ea49f59e41617f) ) /* Program 1 - V2.0 at U103 */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* M6809 */
	ROM_LOAD( "prog.102", 0x00000, 0x8000, CRC(d452ccf4) SHA1(7de9a4f39bf0ba448fe4ebeb459e98a1910a66be) ) /* Sound Program 6809 code at U102 */
	ROM_RELOAD(0x8000,0x8000)

	ROM_REGION( 0x100000, "gfx", ROMREGION_ERASEFF )
	ROM_LOAD( "art-rom.123", 0x000000, 0x40000, CRC(1bab1a52) SHA1(f713ba1bc755c41d38d9846444d753c9c7fb1f9d) ) /* Graphics / Art at U123 */
	ROM_LOAD( "art-rom.127", 0x040000, 0x40000, CRC(dc9be2ca) SHA1(d5059a49a3aad309e242c9c4791d10aa5ecd5d1a) ) /* Graphics / Art at U127 */
	ROM_LOAD( "art-rom.125", 0x080000, 0x40000, CRC(7abe18d9) SHA1(c5a582ded7c1b0a02847b342111c64ac0ccb70c2) ) /* Graphics / Art at U125 */

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD( "arom0", 0x000000, 0x40000, CRC(82129830) SHA1(2fa3a32ac4f81dd9c2ab11f34257df4074447f3a))
	ROM_RELOAD(        0x040000, 0x40000 )
	ROM_RELOAD(        0x080000, 0x40000 )
	ROM_RELOAD(        0x0c0000, 0x40000 )
	ROM_LOAD( "arom1", 0x100000, 0x40000, CRC(0d07ac31) SHA1(f1721c34d076a1695f01b90e99428736471cae49) )
	ROM_RELOAD(        0x140000, 0x40000 )
	ROM_RELOAD(        0x180000, 0x40000 )
	ROM_RELOAD(        0x1c0000, 0x40000 )
	ROM_LOAD( "arom2", 0x200000, 0x40000, CRC(625f3bf5) SHA1(e6a2ca8e105327d8f6a8245483dab29fe1b513fa) )
	ROM_RELOAD(        0x240000, 0x40000 )
	ROM_RELOAD(        0x280000, 0x40000 )
	ROM_RELOAD(        0x2c0000, 0x40000 )
	ROM_LOAD( "arom3", 0x300000, 0x40000, CRC(f3c9d554) SHA1(55daf85cec511010832d1d65bc734ec0c19b7c3f) )
	ROM_RELOAD(        0x340000, 0x40000 )
	ROM_RELOAD(        0x380000, 0x40000 )
	ROM_RELOAD(        0x3c0000, 0x40000 )

	ROM_REGION16_LE( 0x20000, "palrom", 0 )
	ROM_LOAD16_BYTE( "0.144", 0x00000, 0x10000, CRC(69b3cc85) SHA1(05f7204ac961274b5d2f42cc6c0d06e5fa146aef) ) /* Palette - 0 at U144 */
	ROM_LOAD16_BYTE( "1.145", 0x00001, 0x10000, CRC(e64a8511) SHA1(0e3a1fe936c841b8acfb150bf63e564b1dec2363) ) /* Palette - 1 at U145 */
ROM_END



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1993, dcheese,   0,       dcheese, dcheese, dcheese_state, empty_init, ROT90, "HAR Management",     "Double Cheese",                                                       MACHINE_SUPPORTS_SAVE )
GAME( 1993, lottof2,   0,       dcheese, lottof2, dcheese_state, empty_init, ROT0,  "HAR Management",     "Lotto Fun 2",                                                         MACHINE_SUPPORTS_SAVE )
GAME( 1993, cecmatch,  0,       fredmem, fredmem, dcheese_state, empty_init, ROT0,  "Coastal Amusements", "ChuckECheese's Match Game",                                           MACHINE_SUPPORTS_SAVE )
GAME( 1994, fredmem,   0,       fredmem, fredmem, dcheese_state, empty_init, ROT0,  "Coastal Amusements", "Fred Flintstone's Memory Match (World?, Ticket version, 3/17/95)",    MACHINE_SUPPORTS_SAVE )
GAME( 1994, fredmemus, fredmem, fredmem, fredmem, dcheese_state, empty_init, ROT0,  "Coastal Amusements", "Fred Flintstone's Memory Match (US, High Score version, 3/10/95)",    MACHINE_SUPPORTS_SAVE )
GAME( 1994, fredmemuk, fredmem, fredmem, fredmem, dcheese_state, empty_init, ROT0,  "Coastal Amusements", "Fred Flintstone's Memory Match (UK, 3/17/95)",                        MACHINE_SUPPORTS_SAVE )
GAME( 1994, fredmemj,  fredmem, fredmem, fredmem, dcheese_state, empty_init, ROT0,  "Coastal Amusements", "Fred Flintstone's Memory Match (Japan, High Score version, 3/20/95)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, fredmemc,  fredmem, fredmem, fredmem, dcheese_state, empty_init, ROT0,  "Coastal Amusements", "Fred Flintstone's Memory Match (Mandarin Chinese, 3/17/95)",          MACHINE_SUPPORTS_SAVE )
GAME( 1994, fredmesp,  fredmem, fredmem, fredmem, dcheese_state, empty_init, ROT0,  "Coastal Amusements", "Fred Flintstone's Memory Match (Spanish, 3/17/95)",                   MACHINE_SUPPORTS_SAVE )
