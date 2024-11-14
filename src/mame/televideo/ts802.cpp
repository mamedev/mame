// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

    Skeleton driver for Televideo TS802

    2012-11-06 Skeleton
    2014-02-07 Started adding devices

    Status:
    - TS802:  After 5 seconds, Slowly prints dots
    - TS802H: After 5 seconds, type in any 5 characters, then you get a prompt.

    TODO:
    - Almost everything

    Technical manual at:
    http://bitsavers.org/pdf/televideo/TS800A_TS802_TS802H_Maintenance_Manual_1982.pdf

    includes in-depth discussion of the inner workings of the WD1000 HDD controller.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/z80daisy.h"
#include "machine/terminal.h"
#include "machine/z80dma.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/wd_fdc.h"


namespace {

class ts802_state : public driver_device
{
public:
	ts802_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
	{ }

	void ts802(machine_config &config);

	void init_ts802();

private:
	virtual void machine_reset() override ATTR_COLD;
	uint8_t port00_r() { return 0x80; };
	uint8_t port0c_r() { return 1; };
	uint8_t port0e_r() { return 0; };
	uint8_t port0f_r() { return (m_term_data) ? 5 : 4; };
	uint8_t port0d_r();
	void port04_w(uint8_t data);
	void port18_w(uint8_t data);
	void port80_w(uint8_t data);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	uint8_t io_read_byte(offs_t offset);
	void io_write_byte(offs_t offset, uint8_t data);
	void kbd_put(u8 data);
	void ts802_io(address_map &map) ATTR_COLD;
	void ts802_mem(address_map &map) ATTR_COLD;

	uint8_t m_term_data = 0;
	address_space *m_mem = nullptr;
	address_space *m_io = nullptr;
	required_device<z80_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

void ts802_state::ts802_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).bankr("bankr0").bankw("bankw0");
	map(0x1000, 0xffff).ram();
}

void ts802_state::ts802_io(address_map &map)
{
	//map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).r(FUNC(ts802_state::port00_r));  // DIP switches
	// 04 - written once after OS boot to bank in RAM from 0000-3FFF instead of ROM.  4000-FFFF is always RAM.
	map(0x04, 0x07).w(FUNC(ts802_state::port04_w));
	// 08-0B: Z80 CTC
	map(0x08, 0x0b).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	// 0C-0F: Z80 SIO #1
	//map(0x0c, 0x0f).rw("dart1", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x0c, 0x0c).r(FUNC(ts802_state::port0c_r));
	map(0x0d, 0x0d).r(FUNC(ts802_state::port0d_r)).w(m_terminal, FUNC(generic_terminal_device::write));
	map(0x0e, 0x0e).r(FUNC(ts802_state::port0e_r));
	map(0x0f, 0x0f).r(FUNC(ts802_state::port0f_r));
	// 10: Z80 DMA
	map(0x10, 0x13).rw("dma", FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	// 14-17: WD 1793
	map(0x14, 0x17).rw("fdc", FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	// 18: floppy misc.
	map(0x18, 0x1c).w(FUNC(ts802_state::port18_w));
	// 20-23: Z80 SIO #2
	map(0x20, 0x23).rw("dart2", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	// 48-4F: WD1000 harddisk controller
	// 80: LEDs
	map(0x80, 0x80).w(FUNC(ts802_state::port80_w));
}


/* Input ports */
static INPUT_PORTS_START( ts802 )
INPUT_PORTS_END

void ts802_state::port04_w(uint8_t data)
{
	membank("bankr0")->set_entry(1);
}

void ts802_state::port18_w(uint8_t data)
{
}

void ts802_state::port80_w(uint8_t data)
{
}

uint8_t ts802_state::memory_read_byte(offs_t offset)
{
	return m_mem->read_byte(offset);
}

void ts802_state::memory_write_byte(offs_t offset, uint8_t data)
{
	m_mem->write_byte(offset, data);
}

uint8_t ts802_state::io_read_byte(offs_t offset)
{
	return m_io->read_byte(offset);
}

void ts802_state::io_write_byte(offs_t offset, uint8_t data)
{
	m_io->write_byte(offset, data);
}

static void ts802_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void ts802_state::machine_reset()
{
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
}

uint8_t ts802_state::port0d_r()
{
	uint8_t ret = m_term_data;
	m_term_data = 0;
	return ret;
}

void ts802_state::kbd_put(u8 data)
{
	m_term_data = data;
}

#if 0
// not correct
static const z80_daisy_config daisy_chain_intf[] =
{
	{ "dart1" },
	{ "dart2" },
	{ "dma" },
	{ "ctc" },
	{ nullptr }
};
#endif

void ts802_state::init_ts802()
{
	m_mem = &m_maincpu->space(AS_PROGRAM);
	m_io = &m_maincpu->space(AS_IO);

	uint8_t *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
}

void ts802_state::ts802(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &ts802_state::ts802_mem);
	m_maincpu->set_addrmap(AS_IO, &ts802_state::ts802_io);
	//m_maincpu->set_daisy_config(daisy_chain_intf); // causes problems
	m_maincpu->busack_cb().set("dma", FUNC(z80dma_device::bai_w));

	/* Devices */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(ts802_state::kbd_put));

	z80dma_device& dma(Z80DMA(config, "dma", 16_MHz_XTAL / 4));
	dma.out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	dma.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	dma.in_mreq_callback().set(FUNC(ts802_state::memory_read_byte));
	dma.out_mreq_callback().set(FUNC(ts802_state::memory_write_byte));
	dma.in_iorq_callback().set(FUNC(ts802_state::io_read_byte));
	dma.out_iorq_callback().set(FUNC(ts802_state::io_write_byte));

	z80dart_device& dart1(Z80DART(config, "dart1", 16_MHz_XTAL / 4));
	dart1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80dart_device& dart2(Z80DART(config, "dart2", 16_MHz_XTAL / 4));
	dart2.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80ctc_device& ctc(Z80CTC(config, "ctc", 16_MHz_XTAL / 4));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	FD1793(config, "fdc", 4'000'000 / 2);                  // unknown clock
	FLOPPY_CONNECTOR(config, "fdc:0", ts802_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

/* ROM definition */
ROM_START( ts802 )
	ROM_REGION(0x11000, "maincpu", 0)
	ROM_LOAD( "ts802.rom", 0x10000, 0x1000, CRC(60bd086a) SHA1(82c5b60223e0d895683d3592a56684ef2dabfba6) )
ROM_END

ROM_START( ts802h )
	ROM_REGION(0x11000, "maincpu", 0)
	ROM_LOAD( "8000050 050 2732", 0x10000, 0x1000, CRC(7054f384) SHA1(cf0a01a32283272532ed4890c3a3c2082f1618bf) )

	ROM_REGION(0x2000, "roms", 0) // not Z80 code
	ROM_LOAD( "i800000 047d.a53", 0x0000, 0x1000, CRC(94bfcbc1) SHA1(87c5f8898b0041d012e142ee7f559cb8a90f4dc1) )
	ROM_LOAD( "a64",              0x1000, 0x1000, CRC(41b5feda) SHA1(c9435a97c032ffe457bdb84d5dde8ecf3677b56c) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "800000-003a.a68",  0x0000, 0x0800, CRC(24eeb74d) SHA1(77900937f1492b4c5a70ba3aac55da322d403fbd) )
	ROM_LOAD( "800000-002a.a67",  0x0800, 0x0800, CRC(4b6c6e29) SHA1(c236e4625bc16062154cbebc4dbc8d62183ef9ab) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  STATE        INIT        COMPANY      FULLNAME  FLAGS
COMP( 1982, ts802,  0,      0,      ts802,   ts802, ts802_state, init_ts802, "Televideo", "TS802",  MACHINE_IS_SKELETON )
COMP( 1982, ts802h, ts802,  0,      ts802,   ts802, ts802_state, init_ts802, "Televideo", "TS802H", MACHINE_IS_SKELETON )
