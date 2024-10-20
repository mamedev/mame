// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Powertran Cortex

2012-04-20 Skeleton driver.

ftp://ftp.whtech.com/Powertran Cortex/
http://www.powertrancortex.com/index.html

Uses Texas Instruments parts and similar to other TI computers.
It was designed by TI engineers, so it may perhaps be a clone
of another TI or the Geneve.

Chips:
TMS9995   - CPU
TMS9929   - Video
TMS9911   - DMA to floppy (unemulated device)
TMS9909   - Floppy Disk Controller (unemulated device)
TMS9902   - UART (x2) (device not usable with rs232.h)
AY-5-2376 - Keyboard controller

All input to be in uppercase. Note that "lowercase" is just smaller uppercase,
and is not acceptable as input.

There's no option in BASIC to produce sound. It will beep if an invalid key
(usually a control key) is pressed.

To clear the screen press Ctrl L.

ToDo:
- Unemulated devices
- Cassette
- Keyboard to use AY device
- Memory mapping unit (74LS610)
- Various CRU I/O

****************************************************************************/

#include "emu.h"

#include "cpu/tms9900/tms9995.h"
#include "machine/74259.h"
#include "machine/keyboard.h"
//#include "machine/tms9902.h"
#include "video/tms9928a.h"
#include "sound/beep.h"

#include "speaker.h"


namespace {

class cortex_state : public driver_device
{
public:
	cortex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_beep(*this, "beeper")
		, m_io_dsw(*this, "DSW")
	{ }

	void cortex(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void kbd_put(u8 data);
	void keyboard_ack_w(int state);
	void romsw_w(int state);
	void vdp_int_w(int state);
	u8 pio_r(offs_t offset);
	u8 keyboard_r(offs_t offset);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	bool m_kbd_ack = 0;
	bool m_vdp_int = 0;
	u8 m_term_data = 0U;
	required_device<tms9995_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_memory_bank    m_bank1;
	required_device<beep_device>    m_beep;
	required_ioport m_io_dsw;
};

void cortex_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share(m_ram).bankr(m_bank1);
	map(0x8000, 0xefff).ram();
	map(0xf100, 0xf11f).ram(); // memory mapping unit
	map(0xf120, 0xf121).rw("crtc", FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));
	//map(0xf140, 0xf147) // fdc tms9909
}

void cortex_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x000f).mirror(0x30).w("control", FUNC(ls259_device::write_d0));
	map(0x0000, 0x000f).r(FUNC(cortex_state::pio_r));
	map(0x0010, 0x001f).r(FUNC(cortex_state::keyboard_r));
	//map(0x0080, 0x00bf).rw("uart1", FUNC(tms9902_device::cruread), FUNC(tms9902_device::cruwrite)); // RS232 (r12 = 80-bf)
	//map(0x0180, 0x01bf).rw("uart2", FUNC(tms9902_device::cruread), FUNC(tms9902_device::cruwrite)); // Cassette (r12 = 180-1bf)
	//map(0x01c0, 0x01ff).rw("dma", FUNC(tms9911_device::read), FUNC(tms9911_device::write)); // r12 = 1c0-1fe
	//map(0x0800, 0x080f).w(cortex_state::cent_data_w)); // r12 = 800-80e
	//map(0x0810, 0x0811).w(FUNC(cortex_state::cent_strobe_w)); // r12 = 810
	//map(0x0812, 0x0813).r(FUNC(cortex_state::cent_stat_r)); // CRU 409 (r12 = 812)
}

/* Input ports */
static INPUT_PORTS_START( cortex )
	PORT_START("DSW")
	PORT_DIPNAME( 0x04, 0x00, "DISK SIZE")
	PORT_DIPSETTING(    0x04, "20cm")
	PORT_DIPSETTING(    0x00, "13cm")
	PORT_DIPNAME( 0x08, 0x08, "DISK DENSITY")
	PORT_DIPSETTING(    0x08, "Double")
	PORT_DIPSETTING(    0x00, "Single")
INPUT_PORTS_END

u8 cortex_state::pio_r(offs_t offset)
{
	switch (offset)
	{
	case 5:
		return m_kbd_ack;

	case 6:
		return m_vdp_int;

	case 2:
	case 3:
		return BIT(m_io_dsw->read(), offset);

	default:
		return 1;
	}
}

u8 cortex_state::keyboard_r(offs_t offset)
{
	return BIT(m_term_data, offset);
}

void cortex_state::keyboard_ack_w(int state)
{
	if (!state)
	{
		m_maincpu->set_input_line(INT_9995_INT4, CLEAR_LINE);
		m_kbd_ack = 1;
	}
}

void cortex_state::romsw_w(int state)
{
	m_bank1->set_entry(state ? 0 : 1);
}

void cortex_state::vdp_int_w(int state)
{
	m_vdp_int = state ? 0 : 1;  // change polarity to match mame
}

void cortex_state::kbd_put(u8 data)
{
	m_term_data = data;
	m_kbd_ack = 0;
	m_maincpu->set_input_line(INT_9995_INT4, ASSERT_LINE);
}

void cortex_state::machine_start()
{
	m_bank1->configure_entry(0, m_ram);
	m_bank1->configure_entry(1, m_rom);
	save_item(NAME(m_kbd_ack));
	save_item(NAME(m_vdp_int));
	save_item(NAME(m_term_data));
}

void cortex_state::machine_reset()
{
	m_kbd_ack = 1;
	m_vdp_int = 0;
	m_beep->set_state(0);
	m_bank1->set_entry(1);
	m_maincpu->ready_line(ASSERT_LINE);
	m_maincpu->reset_line(ASSERT_LINE);
}

void cortex_state::cortex(machine_config &config)
{
	/* basic machine hardware */
	/* TMS9995 CPU @ 12.0 MHz */
	// Standard variant, no overflow int
	// No lines connected yet
	TMS9995(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &cortex_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &cortex_state::io_map);

	ls259_device &control(LS259(config, "control")); // IC64
	//control.q_out_cb<0>().set(FUNC(cortex_state::basic_led_w));
	control.q_out_cb<1>().set(FUNC(cortex_state::keyboard_ack_w));
	//control.q_out_cb<2>().set(FUNC(cortex_state::ebus_int_ack_w));
	//control.q_out_cb<3>().set(FUNC(cortex_state::ebus_to_en_w));
	//control.q_out_cb<4>().set(FUNC(cortex_state::disk_size_w));
	control.q_out_cb<5>().set(FUNC(cortex_state::romsw_w));
	control.q_out_cb<6>().set("beeper", FUNC(beep_device::set_state));

	/* video hardware */
	tms9929a_device &crtc(TMS9929A(config, "crtc", XTAL(10'738'635)));
	crtc.set_screen("screen");
	crtc.int_callback().set_inputline(m_maincpu, INT_9995_INT1);
	crtc.int_callback().append(FUNC(cortex_state::vdp_int_w));
	crtc.set_vram_size(0x4000);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(cortex_state::kbd_put));

	//TMS9902(config, "uart1", XTAL(12'000'000) / 4);
	//TMS9902(config, "uart2", XTAL(12'000'000) / 4);

	/* Sound */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 950); // guess
	m_beep->add_route(ALL_OUTPUTS, "mono", 0.05);
}

/* ROM definition */
ROM_START( cortex )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "basic", "Cortex BIOS")
	ROMX_LOAD( "cortex.ic47", 0x0000, 0x2000, CRC(bdb8c7bd) SHA1(340829dcb7a65f2e830fd5aff82a312e3ed7918f), ROM_BIOS(0))
	ROMX_LOAD( "cortex.ic46", 0x2000, 0x2000, CRC(4de459ea) SHA1(00a42fe556d4ffe1f85b2ce369f544b07fbd06d9), ROM_BIOS(0))
	ROMX_LOAD( "cortex.ic45", 0x4000, 0x2000, CRC(b0c9b6e8) SHA1(4e20c3f0b7546b803da4805cd3b8616f96c3d923), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "forth", "FIG-Forth")
	ROMX_LOAD( "forth.ic47",  0x0000, 0x2000, CRC(999034be) SHA1(0dcc7404c38aa0ae913101eb0aa98da82104b5d4), ROM_BIOS(1))
	ROMX_LOAD( "forth.ic46",  0x2000, 0x2000, CRC(8eca54cc) SHA1(0f1680e941ef60bb9bde9a4b843b78f30dff3202), ROM_BIOS(1))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY                  FULLNAME  FLAGS
COMP( 1982, cortex, 0,      0,      cortex,  cortex, cortex_state, empty_init, "Powertran Cybernetics", "Cortex", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
