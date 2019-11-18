// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Wyse WY-100 video terminal.

    The WY-100 was Wyse Technology's first product.

    Of the two 8276 CRTCs, one is used solely to keep track of which characters
    are protected, which is the only transparent attribute supported.

    Known emulation bugs:
    - Return key often gets corrupted when looped back
    - Frequent screen glitches when writing to the display
    - No dimming of protected characters

*******************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/bankdev.h"
#include "machine/input_merger.h"
#include "machine/mc2661.h"
#include "machine/wy50kb.h"
#include "sound/spkrdev.h"
#include "video/i8275.h"
#include "screen.h"
#include "speaker.h"

class wy100_state : public driver_device
{
public:
	wy100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rambank(*this, "rambank")
		, m_crtc(*this, "crtc%u", 1U)
		, m_pci(*this, "pci")
		, m_modem(*this, "modem")
		, m_printer(*this, "aux")
		, m_chargen(*this, "chargen")
	{
	}

	void wy100(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	I8275_DRAW_CHARACTER_MEMBER(draw_character);

	DECLARE_WRITE_LINE_MEMBER(brdy_w);
	DECLARE_WRITE_LINE_MEMBER(txd_w);
	void p2_w(u8 data);
	u8 memory_r(offs_t offset);
	void memory_w(offs_t offset, u8 data);

	void prg_map(address_map &map);
	void io_map(address_map &map);
	void bank_map(address_map &map);

	required_device<mcs48_cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_rambank;
	required_device_array<i8276_device, 2> m_crtc;
	required_device<mc2661_device> m_pci;
	required_device<rs232_port_device> m_modem;
	required_device<rs232_port_device> m_printer;

	required_region_ptr<u8> m_chargen;

	bool m_brdy;
	bool m_bs_enable;
	bool m_txd;
	bool m_printer_select;
};

void wy100_state::machine_start()
{
	m_brdy = false;
	m_bs_enable = false;
	m_txd = true;
	m_printer_select = false;

	save_item(NAME(m_brdy));
	save_item(NAME(m_bs_enable));
	save_item(NAME(m_txd));
	save_item(NAME(m_printer_select));
}

WRITE_LINE_MEMBER(wy100_state::brdy_w)
{
	m_brdy = state;
}

I8275_DRAW_CHARACTER_MEMBER(wy100_state::draw_character)
{
	// LTEN attribute output is not used (GPA1 generates underline instead)
	u8 dots = 0;
	if (!vsp)
	{
		if (BIT(gpa, 1) && (linecount & 0xb) == 0xa)
			dots = 0xff;
		else if (!BIT(gpa, 0))
			dots = m_chargen[(charcode << 4) | linecount];
	}
	if (rvv)
		dots ^= 0xff;

	// TODO: dim protected characters
	const rgb_t fg = rgb_t::white();
	const rgb_t bg = rgb_t::black();
	for (int i = 0; i < 10; i++)
		bitmap.pix32(y, x + i) = BIT(dots, i < 1 || i > 8 ? 7 : 8 - i) ? fg : bg;
}

WRITE_LINE_MEMBER(wy100_state::txd_w)
{
	m_txd = state;
	if (m_printer_select)
		m_printer->write_txd(state);
	else
		m_modem->write_txd(state);
}

void wy100_state::p2_w(u8 data)
{
	m_rambank->set_bank(data & 0x1f);
	if (!BIT(data, 6))
		m_bs_enable = false;
	if (BIT(data, 7) && !m_printer_select)
	{
		m_printer_select = true;
		m_printer->write_txd(m_txd);
		m_modem->write_txd(1);
	}
	else if (!BIT(data, 7) && m_printer_select)
	{
		m_printer_select = false;
		m_modem->write_txd(m_txd);
		m_printer->write_txd(1);
	}
}

u8 wy100_state::memory_r(offs_t offset)
{
	u8 p2 = m_maincpu->p2_r();
	u8 data = BIT(p2, 5) ? m_pci->read(p2 & 3) : m_rambank->read8(offset);
	if (BIT(p2, 5))
		logerror("%s: Reading %02X from PCI register %d\n", machine().describe_context(), data, p2 & 3);
	if (m_bs_enable && !machine().side_effects_disabled())
	{
		u8 chardata = (data & 0xe0) == 0x80 ? data : data & 0x7f;
		m_crtc[0]->dack_w(chardata);
		m_crtc[1]->dack_w((chardata & 0xfe) | (BIT(data, 7) ? 0x00 : 0x01));
	}
	return data;
}

void wy100_state::memory_w(offs_t offset, u8 data)
{
	u8 p2 = m_maincpu->p2_r();

	// CRTC access is write-only
	if (!BIT(p2, 6))
	{
		m_crtc[0]->write(p2 & 1, data);
		m_crtc[1]->write(p2 & 1, data);
	}
	else if (m_brdy)
		m_bs_enable = true;

	if (BIT(p2, 5))
	{
		logerror("%s: Writing %02X to PCI register %d\n", machine().describe_context(), data, p2 & 3);

		m_pci->write(p2 & 3, data);
	}

	m_rambank->write8(offset, data);
}

void wy100_state::prg_map(address_map &map)
{
	map(0x000, 0xfff).rom().region("maincpu", 0);
}

void wy100_state::io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(wy100_state::memory_r), FUNC(wy100_state::memory_w));
}

void wy100_state::bank_map(address_map &map)
{
	map(0x0000, 0x0bff).nopw();
	map(0x0c00, 0x0fff).ram(); // buffer RAM (P2114A-6 at 4-5A)
	map(0x1000, 0x1fff).ram(); // display RAM (P2114A-6 at 6-9A, optionally also at 10-13A)
}


static INPUT_PORTS_START(wy100)
INPUT_PORTS_END


void wy100_state::wy100(machine_config &config)
{
	I8039(config, m_maincpu, 10.1376_MHz_XTAL); // INS8039N-11
	m_maincpu->set_addrmap(AS_PROGRAM, &wy100_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &wy100_state::io_map);
	m_maincpu->p1_out_cb().set("keyboard", FUNC(wy100_keyboard_device::scan_w)).mask(0x7f).invert();
	m_maincpu->p1_out_cb().append("spkrgate", FUNC(input_merger_device::in_w<0>)).bit(7);
	m_maincpu->p2_out_cb().set(FUNC(wy100_state::p2_w));
	m_maincpu->t0_in_cb().set("keyboard", FUNC(wy100_keyboard_device::sense_r)).invert();
	m_maincpu->t1_in_cb().set(m_pci, FUNC(mc2661_device::rxrdy_r)).invert();

	WY100_KEYBOARD(config, "keyboard");

	ADDRESS_MAP_BANK(config, m_rambank);
	m_rambank->set_addrmap(0, &wy100_state::bank_map);
	m_rambank->set_data_width(8);
	m_rambank->set_addr_width(13);
	m_rambank->set_stride(0x100);

	MC2661(config, m_pci, 10.1376_MHz_XTAL / 2); // INS2651N
	m_pci->rts_handler().set(m_modem, FUNC(rs232_port_device::write_rts));
	m_pci->dtr_handler().set(m_modem, FUNC(rs232_port_device::write_dtr));
	m_pci->txd_handler().set(FUNC(wy100_state::txd_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(18.48_MHz_XTAL, 1000, 0, 800, 308, 0, 286);
	//screen.set_raw(18.48_MHz_XTAL, 1100, 0, 800, 336, 0, 312);
	screen.set_color(rgb_t::green());
	screen.set_screen_update("crtc1", FUNC(i8276_device::screen_update));

	for (auto &crtc : m_crtc)
	{
		I8276(config, crtc, 18.48_MHz_XTAL / 10);
		crtc->set_screen("screen");
		crtc->set_character_width(10);
	}
	m_crtc[0]->set_display_callback(FUNC(wy100_state::draw_character));
	m_crtc[0]->drq_wr_callback().set_inputline(m_maincpu, MCS48_INPUT_IRQ);
	m_crtc[0]->drq_wr_callback().append(FUNC(wy100_state::brdy_w));
	m_crtc[0]->lc_wr_callback().set("spkrgate", FUNC(input_merger_device::in_w<1>)).bit(3);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.5);
	input_merger_device &spkrgate(INPUT_MERGER_ALL_HIGH(config, "spkrgate"));
	spkrgate.output_handler().set("speaker", FUNC(speaker_sound_device::level_w));

	RS232_PORT(config, m_modem, default_rs232_devices, "loopback");
	m_modem->dcd_handler().set(m_pci, FUNC(mc2661_device::dcd_w));
	m_modem->cts_handler().set(m_pci, FUNC(mc2661_device::cts_w));
	m_modem->rxd_handler().set(m_pci, FUNC(mc2661_device::rx_w));

	RS232_PORT(config, m_printer, default_rs232_devices, nullptr);
	m_printer->dsr_handler().set(m_pci, FUNC(mc2661_device::dsr_w));
}


ROM_START(wy100)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("wy100_00401f.bin", 0x0000, 0x1000, CRC(1f71de8f) SHA1(2bd9f712aba8b44823ce0b3e111da7b472a1ab38))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("wy100_23-002-01c.bin", 0x0000, 0x0800, CRC(93c31537) SHA1(085e5ad110a76bee83e819a718a7d4cbfb8e07e7))
ROM_END


COMP(1981, wy100, 0, 0, wy100, wy100, wy100_state, empty_init, "Wyse Technology", "WY-100", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS)
