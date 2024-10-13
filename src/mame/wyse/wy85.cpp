// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Preliminary driver for Wyse WY-85 VT220-compatible terminal.

    Unlike most later Wyse terminals, the WY-85 lacks a video gate array. It
    also has non-embedded hidden attributes, though they are buffered with the
    characters into the same RAM using a doubled character clock.

*******************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "bus/wysekbd/wysekbd.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/er1400.h"
#include "machine/mc68681.h"
#include "sound/beep.h"
#include "video/scn2674.h"
#include "screen.h"
#include "speaker.h"


namespace {

class wy85_state : public driver_device
{
public:
	wy85_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_earom(*this, "earom")
		, m_kybd(*this, "kybd")
		, m_pvtc(*this, "pvtc")
		, m_duart(*this, "duart")
		, m_beeper(*this, "beeper")
		, m_comm(*this, "comm")
		, m_pr(*this, "pr")
		, m_chargen(*this, "chargen")
		, m_mainram(*this, "mainram")
		, m_clr_rb(false)
		, m_tru_inv(false)
		, m_lc(0)
	{
	}

	void wy85(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	u8 pvtc_r(offs_t offset);
	void pvtc_w(offs_t offset, u8 data);
	u8 duart_r(offs_t offset);
	void duart_w(offs_t offset, u8 data);
	void earom_w(u8 data);
	u8 font_r();
	void font_w(u8 data);
	u8 utility_r();
	void duart_op_w(u8 data);
	u8 p1_r();
	void p1_w(u8 data);
	u8 p3_r();
	void p3_w(u8 data);

	void prg_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	u8 pvtc_char_r(offs_t offset);
	u8 pvtc_attr_r(offs_t offset);
	u8 pvtc_charbuf_r(offs_t offset);
	void pvtc_charbuf_w(offs_t offset, u8 data);
	u8 pvtc_attrbuf_r(offs_t offset);
	void pvtc_attrbuf_w(offs_t offset, u8 data);

	void char_row_buffer_map(address_map &map) ATTR_COLD;
	void attr_row_buffer_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<er1400_device> m_earom;
	required_device<wyse_keyboard_port_device> m_kybd;
	required_device<scn2672_device> m_pvtc;
	required_device<scn2681_device> m_duart;
	required_device<beep_device> m_beeper;
	required_device<rs232_port_device> m_comm;
	required_device<rs232_port_device> m_pr;

	required_region_ptr<u8> m_chargen;
	required_shared_ptr<u8> m_mainram;

	std::unique_ptr<u8[]> m_rowbuf;
	std::unique_ptr<u8[]> m_fontram;

	bool m_clr_rb;
	bool m_tru_inv;
	u8 m_lc;
};

void wy85_state::machine_start()
{
	m_rowbuf = util::make_unique_clear<u8[]>(0x200);
	m_fontram = util::make_unique_clear<u8[]>(0x800);

	save_pointer(NAME(m_rowbuf), 0x200);
	save_pointer(NAME(m_fontram), 0x800);

	save_item(NAME(m_clr_rb));
	save_item(NAME(m_tru_inv));
	save_item(NAME(m_lc));
}

void wy85_state::machine_reset()
{
	earom_w(0);
}

SCN2672_DRAW_CHARACTER_MEMBER(wy85_state::draw_character)
{
	if (BIT(attrcode, 0, 2) == 0)
	{
		std::fill_n(&bitmap.pix(y, x), 10, rgb_t::black());
		return;
	}

	u8 lc = m_lc + (m_clr_rb ? 0 : linecount);
	u16 a = bitswap<2>(attrcode, 5, 6) << 11 | ((charcode & 0x7f) << 4) | (lc & 0xf);
	u16 c = BIT(attrcode, 3) && (lc & 9) == 9 ? 0x3ff : a >= 0x1800 ? m_fontram[a - 0x1800] << 2 : m_chargen[a] << 2;

	rgb_t fg = !BIT(attrcode, 1) || (BIT(attrcode, 4) && blink) ? rgb_t(0xc0, 0xc0, 0xc0) : rgb_t::white();
	bool inv = (BIT(attrcode, 2) != cursor) == m_tru_inv;
	if (inv && BIT(c, 2))
		c |= 3;
	if (BIT(attrcode, 0, 2) == 3)
		c |= c >> 1;
	if (inv)
		c ^= 0x3ff;

	for (int i = 0; i < 10; i++)
	{
		bitmap.pix(y, x++) = BIT(c, 9) ? fg : rgb_t::black();
		c <<= 1;
	}
}

u8 wy85_state::pvtc_r(offs_t offset)
{
	return m_pvtc->read(offset >> 8);
}

void wy85_state::pvtc_w(offs_t offset, u8 data)
{
	m_pvtc->write(offset >> 8, data);
}

u8 wy85_state::duart_r(offs_t offset)
{
	return m_duart->read(offset >> 8);
}

void wy85_state::duart_w(offs_t offset, u8 data)
{
	m_duart->write(offset >> 8, data);
}

void wy85_state::earom_w(u8 data)
{
	// SN74LS174N latch + 7406 inverter
	m_earom->clock_w(BIT(data, 1));
	m_earom->c3_w(BIT(data, 2));
	m_earom->c2_w(BIT(data, 3));
	m_earom->c1_w(BIT(data, 4));
	m_earom->data_w(BIT(data, 3) ? BIT(data, 0) : 1);
}

u8 wy85_state::font_r()
{
	if (!m_clr_rb)
	{
		if (!machine().side_effects_disabled())
			logerror("%s: Reading font ROM/RAM with RB enabled\n", machine().describe_context());
		return 0;
	}

	u8 attr = m_rowbuf[0x100];
	if ((attr & 0x60) == 0x60)
		return m_fontram[(m_rowbuf[0] & 0x7f) << 4 | m_lc];
	else
		return m_chargen[bitswap<2>(attr, 5, 6) << 11 | (m_rowbuf[0] & 0x7f) << 4 | m_lc];
}

void wy85_state::font_w(u8 data)
{
	if (!m_clr_rb)
	{
		if (!machine().side_effects_disabled())
			logerror("%s: Writing to font RAM with RB enabled\n", machine().describe_context());
		return;
	}

	if ((m_rowbuf[0x100] & 0x60) == 0x60)
		m_fontram[(m_rowbuf[0] & 0x7f) << 4 | m_lc] = data;

	//logerror("%s: font_w %02X (CLR RB = %d, LC = %d, char[0] = %02X, attr[0] = %02X)\n", machine().describe_context(), data, m_clr_rb, m_lc, m_rowbuf[0], m_rowbuf[0x100]);
}

u8 wy85_state::utility_r()
{
	// D0 = HSYNC
	// D1 = not connected?
	// D2 = NVD OUT
	// D3 = KEY DATA
	return (m_earom->data_r() << 2) | (!m_kybd->data_r() << 3);
}

void wy85_state::duart_op_w(u8 data)
{
	// OP0 = RTS
	// OP1 = DTR
	// OP2-OP5 = L0-L3 reload
	// OP6 = SPDS
	// OP7 = character width select

	m_comm->write_rts(BIT(data, 0));
	m_comm->write_dtr(BIT(data, 1));
	m_comm->write_spds(BIT(data, 6));

	m_lc = BIT(data, 2, 4);
}

u8 wy85_state::p1_r()
{
	// P1.4 = AUX DSR
	return m_pr->dsr_r() ? 0xff : 0xef;
}

void wy85_state::p1_w(u8 data)
{
	// P1.0 = AUX DTR
	// P1.1 = CLR RB
	// P1.2 = pass-through?
	// P1.3 = AUX RTS
	// P1.5 = BEEPER
	// P1.6 = TRU INV
	// P1.7 = 80/132 column select

	m_pr->write_dtr(BIT(data, 0));
	m_pr->write_rts(BIT(data, 3));

	m_clr_rb = !BIT(data, 1);
	m_tru_inv = BIT(data, 6);

	m_beeper->set_state(BIT(data, 5));
}

u8 wy85_state::p3_r()
{
	return 0xfe | m_pr->rxd_r();
}

void wy85_state::p3_w(u8 data)
{
	// P3.1 (TXD) = AUX TXD
	// P3.5 (T1) = KEY OUT

	m_pr->write_txd(BIT(data, 1));
	m_kybd->cmd_w(!BIT(data, 5));
}

void wy85_state::prg_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0xc000).rom().region("maincpu", 0);
}

void wy85_state::io_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("mainram"); // 4x HM6116P-3 (with 2 more for the row buffer and font RAM)
	map(0x2000, 0x2000).mirror(0x1fff).w(FUNC(wy85_state::earom_w));
	map(0x4000, 0x4000).select(0x700).mirror(0x18ff).rw(FUNC(wy85_state::pvtc_r), FUNC(wy85_state::pvtc_w));
	map(0x6000, 0x6000).select(0xf00).mirror(0x10ff).rw(FUNC(wy85_state::duart_r), FUNC(wy85_state::duart_w));
	map(0x8000, 0x8000).mirror(0x1fff).rw(FUNC(wy85_state::font_r), FUNC(wy85_state::font_w));
	map(0xa000, 0xa000).mirror(0x1fff).r(FUNC(wy85_state::utility_r));
}

u8 wy85_state::pvtc_char_r(offs_t offset)
{
	return m_mainram[offset & 0xfff];
}

u8 wy85_state::pvtc_attr_r(offs_t offset)
{
	return m_mainram[0x1000 + (offset & 0xfff)];
}

u8 wy85_state::pvtc_charbuf_r(offs_t offset)
{
	return m_rowbuf[m_clr_rb ? 0 : (offset + 1) & 0xff];
}

void wy85_state::pvtc_charbuf_w(offs_t offset, u8 data)
{
	m_rowbuf[m_clr_rb ? 0 : (offset + 1) & 0xff] = data;
}

u8 wy85_state::pvtc_attrbuf_r(offs_t offset)
{
	return m_rowbuf[0x100 + (m_clr_rb ? 0 : (offset + 1) & 0xff)];
}

void wy85_state::pvtc_attrbuf_w(offs_t offset, u8 data)
{
	m_rowbuf[0x100 + (m_clr_rb ? 0 : (offset + 1) & 0xff)] = data;
}

void wy85_state::char_row_buffer_map(address_map &map)
{
	map.global_mask(0x0ff);
	map(0x000, 0x0ff).rw(FUNC(wy85_state::pvtc_charbuf_r), FUNC(wy85_state::pvtc_charbuf_w));
}

void wy85_state::attr_row_buffer_map(address_map &map)
{
	map.global_mask(0x0ff);
	map(0x000, 0x0ff).rw(FUNC(wy85_state::pvtc_attrbuf_r), FUNC(wy85_state::pvtc_attrbuf_w));
}

static INPUT_PORTS_START(wy85)
INPUT_PORTS_END

void wy85_state::wy85(machine_config &config)
{
	I8032(config, m_maincpu, 11_MHz_XTAL); // SCN8032H
	m_maincpu->set_addrmap(AS_PROGRAM, &wy85_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &wy85_state::io_map);
	m_maincpu->port_in_cb<1>().set(FUNC(wy85_state::p1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(wy85_state::p1_w));
	m_maincpu->port_in_cb<3>().set(FUNC(wy85_state::p3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(wy85_state::p3_w));

	ER1400(config, m_earom); // M5G1400

	WYSE_KEYBOARD(config, m_kybd, wy85_keyboards, "wy85");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(48.5568_MHz_XTAL / 3, 96 * 10, 0, 80 * 10, 281, 0, 260);
	//screen.set_raw(48.5568_MHz_XTAL / 2, 160 * 9, 0, 132 * 9, 281, 0, 260);
	screen.set_screen_update("pvtc", FUNC(scn2672_device::screen_update));

	SCN2672(config, m_pvtc, 48.5568_MHz_XTAL / 30); // SCN2672B
	m_pvtc->set_screen("screen");
	m_pvtc->set_character_width(10); // 9 in 132-column mode
	m_pvtc->set_addrmap(0, &wy85_state::char_row_buffer_map);
	m_pvtc->set_addrmap(1, &wy85_state::attr_row_buffer_map);
	m_pvtc->set_display_callback(FUNC(wy85_state::draw_character));
	m_pvtc->intr_callback().set_inputline(m_maincpu, MCS51_T0_LINE);
	m_pvtc->breq_callback().set_inputline(m_maincpu, MCS51_INT0_LINE);
	m_pvtc->mbc_char_callback().set(FUNC(wy85_state::pvtc_char_r));
	m_pvtc->mbc_attr_callback().set(FUNC(wy85_state::pvtc_attr_r));

	SCN2681(config, m_duart, 3.6864_MHz_XTAL); // SCN2681A (+ 4x µA9636ATC drivers and UA9639CP receivers)
	m_duart->a_tx_cb().set(m_comm, FUNC(rs232_port_device::write_txd));
	m_duart->b_tx_cb().set("20ma", FUNC(rs232_port_device::write_txd));
	m_duart->outport_cb().set(FUNC(wy85_state::duart_op_w));
	m_duart->irq_cb().set_inputline(m_maincpu, MCS51_INT1_LINE);

	SPEAKER(config, "speaker").front_center();
	BEEP(config, m_beeper, 1000).add_route(ALL_OUTPUTS, "speaker", 0.10); // FIXME: not accurate; actually uses same circuit as WY-50 with 74LS14 Schmitt triggers and discrete components

	RS232_PORT(config, m_comm, default_rs232_devices, nullptr); // RS423 port, also RS232 compatible
	m_comm->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));
	m_comm->cts_handler().set(m_duart, FUNC(scn2681_device::ip0_w));
	m_comm->dsr_handler().set(m_duart, FUNC(scn2681_device::ip1_w));
	m_comm->dcd_handler().set(m_duart, FUNC(scn2681_device::ip2_w));
	m_comm->si_handler().set(m_duart, FUNC(scn2681_device::ip4_w));

	RS232_PORT(config, m_pr, default_rs232_devices, nullptr); // RS423 port, also RS232 compatible

	RS232_PORT(config, "20ma", default_rs232_devices, nullptr) // 20mA current loop, not actually RS232 compatible
				.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_b_w));
}

ROM_START(wy85)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "revd", "Rev. D")
	ROMX_LOAD("23-121-r7d.5e", 0x0000, 0x4000, CRC(76203960) SHA1(ea58c7337435edb06d2d5434886a67045e96bf1f), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "reva", "Rev. A")
	ROMX_LOAD("250151-04_reva.5e", 0x0000, 0x4000, CRC(8fcb9f43) SHA1(6c7e1d27fa6014870c29ab2b8b856ae412bfc411), ROM_BIOS(1)) // 27128

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("am9265.1h", 0x0000, 0x2000, CRC(5ee65b55) SHA1(a0b38a38838f262aaea22d212351e7441e4b07e8)) // AM9265EPC
ROM_END

} // anonymous namespace


COMP(1985, wy85, 0, 0, wy85, wy85, wy85_state, empty_init, "Wyse Technology", "WY-85", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND)
