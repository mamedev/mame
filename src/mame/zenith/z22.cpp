// license:BSD-3-Clause
// copyright-holders:AJR
/**************************************************************************

    Skeleton driver for Zenith Z-22 terminal.

    Despite featuring much of the same hardware as the MDT 60, with the
    CPU, CRTC and UART types and even dot clock frequency being identical,
    much else is different: a serial EEPROM replaces DIP switches, an 8254
    generates baud rates, fonts are uploaded to RAM, and the serial
    keyboard protocol is incompatible.

**************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "machine/eepromser.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "video/mc6845.h"
#include "screen.h"


namespace {

class z22_state : public driver_device
{
public:
	z22_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_uart(*this, "uart%u", 0U)
		, m_eeprom(*this, "eeprom")
		, m_dispram(*this, "dispram")
		, m_fontram(*this, "fontram")
		, m_eeprom_clk(false)
	{
	}

	void z22(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	MC6845_UPDATE_ROW(update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(update_cb);

	u8 irq_ack_r();
	u8 status_r();
	void control_w(u8 data);

	void eeprom_clock_w(int state);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<r6545_1_device> m_crtc;
	optional_device_array<i8251_device, 2> m_uart;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_shared_ptr<u8> m_dispram;
	required_shared_ptr<u8> m_fontram;

	bool m_eeprom_clk;
};

void z22_state::machine_start()
{
	save_item(NAME(m_eeprom_clk));
}

void z22_state::machine_reset()
{
	control_w(0);
}


MC6845_UPDATE_ROW(z22_state::update_row)
{
	u32 *pix = &bitmap.pix(y);

	for (unsigned x = 0; x < x_count; x++)
	{
		u8 cdata = m_dispram[(ma + x) & 0x7ff];
		u16 dots = m_fontram[(cdata & 0x7f) << 4 | ra] << 1;

		if (x == cursor_x)
			dots = ~dots;

		rgb_t fg = rgb_t::white();
		rgb_t bg = rgb_t::black();

		for (int n = 8; n >= 0; n--)
			*pix++ = BIT(dots, n) ? fg : bg;
	}
}

MC6845_ON_UPDATE_ADDR_CHANGED(z22_state::update_cb)
{
}


u8 z22_state::irq_ack_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(m6512_device::IRQ_LINE, CLEAR_LINE);
	return 0xff;
}

u8 z22_state::status_r()
{
	u8 result = 0;
	if (!m_eeprom->do_read())
		result |= 0x01;
	if (m_eeprom_clk)
		result |= 0x02;
	// TODO: bit 7 = received keyboard data
	return result;
}

void z22_state::control_w(u8 data)
{
	m_eeprom->di_write(!BIT(data, 7));
	m_eeprom->cs_write(BIT(data, 6));
	//m_keyboard->keyout_w(BIT(data, 0));
}

void z22_state::eeprom_clock_w(int state)
{
	m_eeprom_clk = state;
	if (state)
		m_maincpu->set_input_line(m6512_device::IRQ_LINE, ASSERT_LINE);
}

void z22_state::mem_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x27ff).ram().share("dispram");
	map(0x4000, 0x4001).rw(m_uart[1], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x6000, 0x67ff).ram().share("fontram");
	map(0x8000, 0x8001).rw(m_uart[0], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x8800, 0x8800).rw(m_crtc, FUNC(r6545_1_device::status_r), FUNC(r6545_1_device::address_w));
	map(0x8801, 0x8801).rw(m_crtc, FUNC(r6545_1_device::register_r), FUNC(r6545_1_device::register_w));
	map(0x9000, 0x9003).w("pit", FUNC(pit8254_device::write));
	map(0x9800, 0x9800).r(FUNC(z22_state::irq_ack_r));
	map(0xa000, 0xa000).rw(FUNC(z22_state::status_r), FUNC(z22_state::control_w));
	map(0xc000, 0xffff).rom().region("program", 0);
}


static INPUT_PORTS_START(z22)
INPUT_PORTS_END

// XTAL frequency is specified as 16.589 MHz on actual parts. As with the MDT 60, this
// has been assumed to be a lower-precision specification of the common 16.5888 MHz value.

void z22_state::z22(machine_config &config)
{
	M6512(config, m_maincpu, 16.5888_MHz_XTAL / 9); // R6512AP
	m_maincpu->set_addrmap(AS_PROGRAM, &z22_state::mem_map);

	I8251(config, m_uart[0], 16.5888_MHz_XTAL / 9); // NEC D8251AC (U33) + Intel P8251A (U35)
	m_uart[0]->rxrdy_handler().set_inputline(m_maincpu, m6512_device::NMI_LINE);
	m_uart[0]->txd_handler().set("comm", FUNC(rs232_port_device::write_txd));
	m_uart[0]->dtr_handler().set("comm", FUNC(rs232_port_device::write_dtr));
	m_uart[0]->rts_handler().set("comm", FUNC(rs232_port_device::write_rts));

	I8251(config, m_uart[1], 16.5888_MHz_XTAL / 9);
	m_uart[1]->txd_handler().set("printer", FUNC(rs232_port_device::write_txd));

	EEPROM_93C06_16BIT(config, m_eeprom); // NMC9306 (U27)

	pit8254_device &pit(PIT8254(config, "pit")); // Intel P8254
	pit.set_clk<0>(16.5888_MHz_XTAL / 9);
	pit.set_clk<1>(16.5888_MHz_XTAL / 9);
	pit.set_clk<2>(16.5888_MHz_XTAL / 9);
	pit.out_handler<0>().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::clk_write)); // weird synchronous clocking
	pit.out_handler<0>().append(FUNC(z22_state::eeprom_clock_w));
	pit.out_handler<1>().set(m_uart[0], FUNC(i8251_device::write_rxc));
	pit.out_handler<2>().set(m_uart[0], FUNC(i8251_device::write_txc)); // or the other way around?
	pit.out_handler<2>().append(m_uart[1], FUNC(i8251_device::write_txc));

	// TODO: Keyboard (asynchronous serial, like Z-49?)

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::amber());
	screen.set_raw(16.5888_MHz_XTAL, 873, 0, 720, 317, 0, 300);
	screen.set_screen_update(m_crtc, FUNC(r6545_1_device::screen_update));

	R6545_1(config, m_crtc, 16.5888_MHz_XTAL / 9); // R6545-1AP
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(9);
	m_crtc->set_update_row_callback(FUNC(z22_state::update_row));
	m_crtc->set_on_update_addr_change_callback(FUNC(z22_state::update_cb));

	// TODO: onboard buzzer

	rs232_port_device &comm(RS232_PORT(config, "comm", default_rs232_devices, nullptr));
	comm.rxd_handler().set(m_uart[0], FUNC(i8251_device::write_rxd));
	comm.dsr_handler().set(m_uart[0], FUNC(i8251_device::write_dsr));
	comm.cts_handler().set(m_uart[0], FUNC(i8251_device::write_cts));

	rs232_port_device &printer(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	printer.dsr_handler().set(m_uart[1], FUNC(i8251_device::write_dsr));
}


ROM_START(z22)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("u39.bin", 0x0000, 0x2000, CRC(2f62c1f8) SHA1(448581d987fee6b4303e481e78f46d3255baccbb)) // D2764A-3
	ROM_LOAD("u38.bin", 0x2000, 0x2000, CRC(f0bfe9b5) SHA1(8807841b28549d0ddf30275fc6035a66093f8768)) // D2764A-3
ROM_END

} // anonymous namespace


SYST(1984, z22, 0, 0, z22, z22, z22_state, empty_init, "Zenith Data Systems", "Z-22 Terminal", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
