// license:BSD-3-Clause
// copyright-holders:AJR
/**************************************************************************

    MDT 60 was designed by Zenith for use with Morrow's Micro-Decision
    computers as a low-cost terminal with limited features. It appears
    to be a stripped-down version of some previously designed terminal,
    with the auxiliary port omitted from the Morrow version even though
    the schematics published in its service manual include it. These
    schematics also include the same keyboard used by the Z-29, though
    the actual keyboard found with one unit was different.

**************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "bus/z29_kbd/keyboard.h"
#include "cpu/m6502/m6502.h"
#include "machine/i8251.h"
#include "video/mc6845.h"
#include "screen.h"


namespace {

class mdt60_state : public driver_device
{
public:
	mdt60_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_keyboard(*this, "keyboard")
		, m_crtc(*this, "crtc")
		, m_uart(*this, "uart%u", 0U)
		, m_charram(*this, "charram")
		, m_charrom(*this, "charrom")
		, m_attrram(*this, "attrram")
		, m_dip0(*this, "DIP0")
		, m_baud_timer(nullptr)
		, m_keyin(false)
		, m_output_reg(0x3f)
		, m_timer_output(false)
	{
	}

	void mdt60(machine_config &mconfig);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	MC6845_UPDATE_ROW(update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(update_cb);
	TIMER_CALLBACK_MEMBER(baud_timer);

	void keyin_w(int state);
	u8 dip0_r(offs_t offset);
	void reg_w(u8 data);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<z29_keyboard_port_device> m_keyboard;
	required_device<r6545_1_device> m_crtc;
	optional_device_array<i8251_device, 2> m_uart;
	required_shared_ptr<u8> m_charram;
	required_region_ptr<u8> m_charrom;
	required_shared_ptr<u8> m_attrram;
	required_ioport m_dip0;

	emu_timer *m_baud_timer;

	bool m_keyin;
	u8 m_output_reg;
	bool m_timer_output;
};

void mdt60_state::machine_start()
{
	m_baud_timer = timer_alloc(FUNC(mdt60_state::baud_timer), this);
	m_baud_timer->adjust(attotime::zero);

	save_item(NAME(m_keyin));
	save_item(NAME(m_output_reg));
	save_item(NAME(m_timer_output));
}

void mdt60_state::machine_reset()
{
	reg_w(0);
}


TIMER_CALLBACK_MEMBER(mdt60_state::baud_timer)
{
	m_timer_output = !m_timer_output;

	// LS352 multiplexer has inverting outputs
	m_uart[0]->write_rxc(!m_timer_output);
	m_uart[0]->write_txc(!m_timer_output);

	constexpr auto time_base = 16.5888_MHz_XTAL / 9;
	if ((m_output_reg & 0x06) == 0x00)
		m_baud_timer->adjust(attotime::from_ticks(m_timer_output ? 2 : 4, time_base));
	else if ((m_output_reg & 0x06) == 0x06)
		m_baud_timer->adjust(attotime::from_ticks(48, time_base));
	else
		m_baud_timer->adjust(attotime::from_ticks(3 * (m_output_reg & 0x06), time_base));
}

MC6845_UPDATE_ROW(mdt60_state::update_row)
{
	u32 *pix = &bitmap.pix(y);

	for (unsigned x = 0; x < x_count; x++)
	{
		u8 cdata = m_charram[(ma + x) & 0x7ff];
		u8 adata = m_attrram[(ma + x) & 0x3ff] >> (BIT(ma + x, 10) ? 4 : 0);
		u16 dots = m_charrom[cdata << 4 | ra] << 1 | BIT(adata, 3);

		if (x == cursor_x)
			dots = ~dots;
		if (BIT(adata, 2))
			dots = ~dots;
		if (BIT(adata, 0) && (ra & 0xb) == 0xa)
			dots = ~dots;

		rgb_t fg = BIT(adata, 1) ? rgb_t::white() : rgb_t(0xc0, 0xc0, 0xc0);
		rgb_t bg = rgb_t::black();
		if (BIT(m_output_reg, 5))
			std::swap(fg, bg);

		for (int n = 8; n >= 0; n--)
			*pix++ = BIT(dots, n) ? fg : bg;
	}
}

MC6845_ON_UPDATE_ADDR_CHANGED(mdt60_state::update_cb)
{
}


void mdt60_state::keyin_w(int state)
{
	m_keyin = state;
	m_maincpu->set_input_line(m6502_device::IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

u8 mdt60_state::dip0_r(offs_t offset)
{
	u8 buffer = 0xff;
	if (!BIT(offset, 0))
		buffer &= m_dip0->read() >> 1 | 0x80;
	if (!BIT(offset, 1))
		buffer &= m_dip0->read() | 0xfe;
	if (!m_keyin)
		buffer &= 0x7f;
	return buffer;
}

void mdt60_state::reg_w(u8 data)
{
	if (BIT(data, 0) != BIT(m_output_reg, 0))
		m_keyboard->keyout_w(BIT(data, 0));

	// Bits 1–2 & 3–4 select 1 of 4 baud rates for each UART; bit 5 selects inverse video
	m_output_reg = data & 0x3f;
}

void mdt60_state::mem_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x1800).ram();
	map(0x0400, 0x07ff).mirror(0x1800).ram().share("attrram");
	map(0x2000, 0x27ff).mirror(0x1800).ram().share("charram");
	map(0x4000, 0x4000).mirror(0x1ffe).rw(m_crtc, FUNC(r6545_1_device::status_r), FUNC(r6545_1_device::address_w));
	map(0x4001, 0x4001).mirror(0x1ffe).rw(m_crtc, FUNC(r6545_1_device::register_r), FUNC(r6545_1_device::register_w));
	map(0x6000, 0x6001).mirror(0x1ffe).rw(m_uart[0], FUNC(i8251_device::read), FUNC(i8251_device::write));
	//map(0x8000, 0x8001).mirror(0x1ffe).rw(m_uart[1], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xa000, 0xa003).mirror(0x1ffc).r(FUNC(mdt60_state::dip0_r));
	map(0xa000, 0xa000).mirror(0x1fff).w(FUNC(mdt60_state::reg_w));
	//map(0xc000, 0xc000).mirror(0x1fff).portr("DIP1");
	map(0xe000, 0xefff).mirror(0x1000).rom().region("coderom", 0);
}


static INPUT_PORTS_START(mdt60)
	PORT_START("DIP0")
	PORT_DIPNAME(0x03, 0x01, "Baud Rate") PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(0x03, "300")
	PORT_DIPSETTING(0x02, "1200")
	PORT_DIPSETTING(0x01, "9600")
	PORT_DIPSETTING(0x00, "19200")
	PORT_DIPNAME(0x0c, 0x00, "Parity") PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(0x04, "Odd")
	PORT_DIPSETTING(0x0c, "Even")
	PORT_DIPSETTING(0x00, "Mark")
	PORT_DIPSETTING(0x08, "Space")
	PORT_DIPNAME(0x70, 0x00, "Character Set") PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(0x00, "U.S.")
	PORT_DIPSETTING(0x10, "U.K.")
	PORT_DIPSETTING(0x20, "French")
	PORT_DIPSETTING(0x30, "German")
	PORT_DIPSETTING(0x40, "Spanish")
	PORT_DIPSETTING(0x50, "Danish/Norwegian")
	PORT_DIPSETTING(0x60, "Swedish/Finnish")
	PORT_DIPSETTING(0x70, "Italian")
	PORT_DIPNAME(0x80, 0x80, "Function Key Sequence Type") PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(0x80, "Morrow (FS)")
	PORT_DIPSETTING(0x00, "TeleVideo (SOH...CR)")

	PORT_START("DIP1")
	PORT_BIT(0xff, 0xff, IPT_UNUSED) // SW2 is unpopulated (as is LS244 at U20)
INPUT_PORTS_END

// XTAL frequency is specified as 16.589 MHz on actual parts as well as in MDT 60 schematics.
// This has been assumed to be a lower-precision specification of the common 16.5888 MHz value.

void mdt60_state::mdt60(machine_config &config)
{
	M6512(config, m_maincpu, 16.5888_MHz_XTAL / 9); // R6512AP
	m_maincpu->set_addrmap(AS_PROGRAM, &mdt60_state::mem_map);

	I8251(config, m_uart[0], 16.5888_MHz_XTAL / 9); // TMP8251AP (U19)
	m_uart[0]->rxrdy_handler().set_inputline(m_maincpu, m6512_device::NMI_LINE);
	m_uart[0]->txd_handler().set("comm", FUNC(rs232_port_device::write_txd));
	m_uart[0]->dtr_handler().set("comm", FUNC(rs232_port_device::write_dtr));
	m_uart[0]->rts_handler().set("comm", FUNC(rs232_port_device::write_rts));

	// UART 1 (at U28) is unpopulated and not used by code

	Z29_KEYBOARD(config, m_keyboard, z29_keyboards, "md");
	m_keyboard->keyin_callback().set(FUNC(mdt60_state::keyin_w)).invert();
	// Reset output (pin 2) is not connected

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::amber());
	screen.set_raw(16.5888_MHz_XTAL, 918, 0, 720, 301, 0, 288);
	screen.set_screen_update(m_crtc, FUNC(r6545_1_device::screen_update));

	R6545_1(config, m_crtc, 16.5888_MHz_XTAL / 9); // R6545-1AP
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(9);
	m_crtc->set_update_row_callback(FUNC(mdt60_state::update_row));
	m_crtc->set_on_update_addr_change_callback(FUNC(mdt60_state::update_cb));
	// LPEN is tied to GND (code uses this to determine CRTC type)

	rs232_port_device &comm(RS232_PORT(config, "comm", default_rs232_devices, "loopback"));
	// Service Manual suggests shorting pins with a bent paper clip if no DB-25 loopback connector is available ;-)
	comm.rxd_handler().set(m_uart[0], FUNC(i8251_device::write_rxd));
	comm.dsr_handler().set(m_uart[0], FUNC(i8251_device::write_dsr));
	comm.cts_handler().set(m_uart[0], FUNC(i8251_device::write_cts));
}


ROM_START(mdt60)
	ROM_REGION(0x1000, "coderom", 0)
	// "ROM REV. 1.7 -- Copyright (C) 1983 by Morrow Designs, Inc."
	ROM_LOAD("vb-rom_1.7_bcfd.u9", 0x0000, 0x1000, CRC(3902f7b3) SHA1(ee0ce7f68efe20efe1ab8a44888e276f08fe2d07))

	ROM_REGION(0x1000, "charrom", 0)
	ROM_LOAD("char_gen_b207.u6", 0x0000, 0x1000, CRC(b19432da) SHA1(fa73641c08b778f19a17676a7f4074f69b7b55dd))
ROM_END

} // anonymous namespace


SYST(1983, mdt60, 0, 0, mdt60, mdt60, mdt60_state, empty_init, "Morrow Designs", "MDT 60 Video Display Terminal", MACHINE_SUPPORTS_SAVE)
