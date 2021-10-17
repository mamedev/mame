// license:BSD-3-Clause
// copyright-holders:

/*
Speed Basketball by Sega (1992)
https://www.youtube.com/watch?v=efs2KNNncn8

Exciting Boat Race by Sega (1993) (only sound PCB is dumped)

Hardware notes:
Main PCB marked 'SEGA 1991 171-6202B' with '834-8700' and '921016.0639E' stickers.
Main components:
- D70008AC-8 main CPU
- D71051G serial control unit
- D71054G programmable timer / counter
- 2x D71055G parallel interface unit
- 2x D4701AC incremental encoder counter
- 315-5338A I/O custom
- 93C45 serial EEPROM
- 3773P power monitor / watchdog
- MB3780A battery power monitor + battery
- OSC 32.000 MHz
- 1 4-dip bank

Sound PCB marked 'SOUND SEGA 1991 MADE IN JAPAN' with '837-8724-01' sticker ('837-9653' on Exciting Boat Race')
Main components:
- Z0840008PSC audio CPU
- 315-5476A Sega PCM
- 315-5497 Sega QFP100 ASIC
- YM2151
- TMP82C51AP-8 (D71051C-10 on Exciting Boat Race)
- Oki M6253
- NEC B6391GF QFP80 gate array (next to unpopulated QFP80 silkscreened "5381")
- 2x Sanyo LC7883KM digital filter / DAC
- OSC 48.000 MHz
- OSC 16.9344 MHz
- 1 4-dip bank


Notes:
- Speed Basketball only has some LEDs and lamps, the rest is mechanical.
- The driver contains the ROMs for the sound PCB of Exciting Boat Race as a placeholder, since it's the same PCB as the one used by Speed Basketball.
  Exciting Boat Race is a main + satellites arrangement with video, so when the other PCBs are found it should be moved out of here.
*/

#include "emu.h"

#include "machine/315_5338a.h"

#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/msm6253.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
#include "machine/upd4701.h"
#include "sound/rf5c68.h"
#include "sound/ymopm.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class speedbsk_state : public driver_device
{
public:
	speedbsk_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi(*this, "ppi%u", 0U),
		m_lcd(*this, "lcd")
	{ }

	void speedbsk(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<i8255_device, 2> m_ppi;
	required_device<hd44780_device> m_lcd;

	void main_map(address_map &map);
	void audio_map(address_map &map);
	void audio_io_map(address_map &map);

	void lcd_palette(palette_device &palette) const;
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	void ppi1_portc_w(uint8_t data);
};


void speedbsk_state::machine_start()
{
}


void speedbsk_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x8003).r("upd4701_0", FUNC(upd4701_device::read_xy));
	map(0x8200, 0x8203).r("upd4701_1", FUNC(upd4701_device::read_xy));
//	map(0x8800, 0x8800)
	map(0x8a00, 0x8a0f).rw("io", FUNC(sega_315_5338a_device::read), FUNC(sega_315_5338a_device::write));
	map(0x8a80, 0x8a83).rw("d71054", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x8ac0, 0x8ac1).rw("d71051", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x8b00, 0x8b03).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x8b40, 0x8b43).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe000, 0xffff).ram().share("nvram");
}

void speedbsk_state::audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("audiocpu", 0);
	map(0xc000, 0xdfff).m("rfsnd", FUNC(rf5c68_device::map));
	map(0xe000, 0xffff).ram();
}

void speedbsk_state::audio_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).nopw();
	map(0x02, 0x03).rw("tmp82c51", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x10, 0x13).nopw(); // misc. outputs
	map(0x20, 0x20).nopr(); // or 6253 serial read?
	map(0x21, 0x23).nopw();
	map(0x40, 0x41).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x50, 0x50).rw("adc", FUNC(msm6253_device::d0_r), FUNC(msm6253_device::select_w));
}


static INPUT_PORTS_START( speedbsk )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") // on main PCB
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED ) // only 4 dips

	PORT_START("DSW2") // on audio PCB
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED ) // only 4 dips
INPUT_PORTS_END


//**************************************************************************
//  SERVICE LCD
//**************************************************************************

void speedbsk_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t( 92,  83,  88)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}

HD44780_PIXEL_UPDATE( speedbsk_state::lcd_pixel_update )
{
	// char size is 5x8
	if (x > 4 || y > 7)
		return;

	if (line < 2 && pos < 20)
		bitmap.pix(1 + y + line*8 + line, 1 + pos*6 + x) = state ? 1 : 2;
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void speedbsk_state::ppi1_portc_w(uint8_t data)
{
	// 76543---  unknown
	// -----2--  lcd e
	// ------1-  lcd rw
	// -------0  lcd rs

	m_lcd->rs_w(BIT(data, 0));
	m_lcd->rw_w(BIT(data, 1));
	m_lcd->e_w(BIT(data, 2));
}


void speedbsk_state::speedbsk(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 32_MHz_XTAL / 4); // actually D70008AC-8, divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &speedbsk_state::main_map);

	// placeholder
	clock_device &irqclock(CLOCK(config, "irqclock", 60));
	irqclock.signal_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80_device &audiocpu(Z80(config, "audiocpu", 48_MHz_XTAL / 12)); // divider guessed
	audiocpu.set_addrmap(AS_PROGRAM, &speedbsk_state::audio_map);
	audiocpu.set_addrmap(AS_IO, &speedbsk_state::audio_io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // MB8464A-10LL-SK + MB8730A + battery

	I8251(config, "d71051", 0);

	PIT8254(config, "d71054", 0);

	I8255(config, m_ppi[0]);
	// port a: output
	// port b: output
	// port c: input

	I8255(config, m_ppi[1]);
	// port a: output
	m_ppi[1]->out_pb_callback().set(m_lcd, FUNC(hd44780_device::db_w));
	m_ppi[1]->tri_pc_callback().set_constant(0);
	m_ppi[1]->out_pc_callback().set(FUNC(speedbsk_state::ppi1_portc_w));

	MSM6253(config, "adc", 0);

	UPD4701A(config, "upd4701_0");

	UPD4701A(config, "upd4701_1");

	SEGA_315_5338A(config, "io", 0);

	EEPROM_93C46_8BIT(config, "eeprom"); // Actually 93c45

	// service lcd
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(6*20+1, 19);
	screen.set_visarea(0, 6*20, 0, 19-1);
	screen.set_screen_update(m_lcd, FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(speedbsk_state::lcd_palette), 3);

	HD44780(config, m_lcd, 0);
	m_lcd->set_lcd_size(2, 20);
	m_lcd->set_pixel_update_cb(FUNC(speedbsk_state::lcd_pixel_update));

	// TODO: LED screen

	// sound hardware, on sound PCB
	i8251_device &tmp82c51(I8251(config, "tmp82c51", 0));
	tmp82c51.rxrdy_handler().set_inputline("audiocpu", INPUT_LINE_IRQ0);

	SPEAKER(config, "mono").front_center(); // TODO: verify if stereo

	YM2151(config, "ymsnd", 16.9344_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.75); // divider guessed, may use 48_MHz XTAL instead

	RF5C68(config, "rfsnd", 16.9344_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.75); // actually Sega 315-5476A
}


ROM_START( speedbsk )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "epr-14666.ic25", 0x0000, 0x8000, CRC(9f6d896a) SHA1(d7133f7bc8225bca14249d354f35ed2e9290567a) )

	ROM_REGION( 0x200000, "audiocpu", 0 )
	ROM_LOAD( "epr-14711.ic13",   0x000000, 0x20000, CRC(9b4322c5) SHA1(f68238a85c10528f87d90d383eed64bef9feacde) ) // 11xxxxxxxxxxxxxxx = 0xFF, almost empty
	ROM_LOAD( "mpr-14712-t.ic14", 0x080000, 0x80000, CRC(4b05f4ba) SHA1(5915d735a6bdf55e651a24342f1ded9c54f5c495) )
	ROM_LOAD( "mpr-14713-t.ic15", 0x100000, 0x80000, BAD_DUMP CRC(16950cda) SHA1(e32413f9585d31624d05d0e594e70bbc7273eb06) ) // was marked as bad by the dumper
	// ic16 empty

	ROM_REGION( 0x400, "mainpcb_plds", 0 )
	ROM_LOAD( "315-5558_peel16v8.ic23", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "315-5559_peel16v8.ic24", 0x200, 0x117, NO_DUMP )

	ROM_REGION( 0x600, "soundpcb_plds", 0 )
	ROM_LOAD( "315-4499_gal16v8.ic6", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "315-5500_gal16v8.ic7", 0x200, 0x117, NO_DUMP )
	ROM_LOAD( "315-5501_gal16v8.ic8", 0x400, 0x117, NO_DUMP )
ROM_END

// TODO: move this out of this driver when rest of the PCBs are found and dumped
ROM_START( boatrace )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x200000, "audiocpu", 0 )
	ROM_LOAD( "epr-15892.ic13", 0x000000, 0x20000, CRC(3a178efc) SHA1(21b0fa0f962bb0b882d8a9649863fdfb4ecb63db) ) // 11xxxxxxxxxxxxxxx = 0xFF, almost empty
	ROM_LOAD( "epr-15893.ic14", 0x080000, 0x80000, CRC(4cf30c7b) SHA1(20e9432ec0e55451bce1b193c030707216141f96) )
	ROM_LOAD( "epr-15894.ic15", 0x100000, 0x80000, CRC(60adad47) SHA1(7a8f373c2d225794c85ee0e7354ace35e757ffa2) )
	ROM_LOAD( "epr-15895.ic16", 0x180000, 0x80000, CRC(a6bf6eff) SHA1(1a3280464b0b449521d64df665d889c35d6f1fc5) )

	ROM_REGION( 0x600, "soundpcb_plds", 0 )
	ROM_LOAD( "315-4499_gal16v8.ic6", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "315-5500_gal16v8.ic7", 0x200, 0x117, NO_DUMP )
	ROM_LOAD( "315-5501_gal16v8.ic8", 0x400, 0x117, NO_DUMP )
ROM_END

} // Anonymous namespace


GAME( 1992, speedbsk, 0, speedbsk, speedbsk, speedbsk_state, empty_init, ROT0, "Sega", "Speed Basketball", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1993, boatrace, 0, speedbsk, speedbsk, speedbsk_state, empty_init, ROT0, "Sega", "Exciting Boat Race", MACHINE_IS_SKELETON )
