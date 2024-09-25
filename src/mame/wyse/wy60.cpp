// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    Preliminary driver for Wyse WY-60 terminal.

    If the terminal starts up without valid EEPROM data, it will just display the error code "E" on a mostly blank screen. At this
    point, holding down the Set Up or Select key will cause the EEPROM to be initialized.

***********************************************************************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "bus/wysekbd/wysekbd.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/i2cmem.h"
#include "machine/scn_pci.h"
#include "video/scn2674.h"
#include "screen.h"

namespace {

class wy60_state : public driver_device
{
public:
	wy60_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_eeprom(*this, "eeprom")
		, m_keyboard(*this, "keyboard")
		, m_pvtc(*this, "pvtc")
		, m_sio(*this, "sio")
		, m_aux(*this, "aux")
		, m_charram(*this, "charram")
		, m_attrram(*this, "attrram")
		, m_fontram(*this, "fontram", 0x2000, ENDIANNESS_LITTLE)
		, m_internal_view(*this, "8051_internal")
		, m_char_addr(0)
		, m_is_132(false)
		, m_cur_attr(0)
		, m_last_row_attr(0)
	{
	}

	void wy60(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void driver_start() override;

private:
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);
	void mbc_attr_clock_w(int state);
	u8 mbc_char_r(offs_t offset);
	u8 mbc_attr_r(offs_t offset);

	u8 pvtc_r(offs_t offset);
	void pvtc_w(offs_t offset, u8 data);
	u8 sio_r(offs_t offset);
	void sio_w(offs_t offset, u8 data);

	void p1_w(u8 data);
	u8 p1_r();
	u8 p3_r();
	void ea_w(int state);

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;
	void row_buffer_map(address_map &map) ATTR_COLD;

	required_device<i2cmem_device> m_eeprom;
	required_device<wyse_keyboard_port_device> m_keyboard;
	required_device<scn2672_device> m_pvtc;
	required_device<scn2661b_device> m_sio;
	required_device<rs232_port_device> m_aux;
	required_shared_ptr<u8> m_charram;
	required_shared_ptr<u8> m_attrram;
	memory_share_creator<u8> m_fontram;
	memory_view m_internal_view;

	u16 m_char_addr;
	bool m_is_132;
	u8 m_cur_attr;
	u8 m_last_row_attr;
};


void wy60_state::machine_start()
{
	save_item(NAME(m_char_addr));
	save_item(NAME(m_is_132));
	save_item(NAME(m_cur_attr));
	save_item(NAME(m_last_row_attr));
}

SCN2672_DRAW_CHARACTER_MEMBER(wy60_state::draw_character)
{
	// TODO: line attributes
	const u8 screen_attr = m_charram[0x0002];
	if (!BIT(screen_attr, 4))
	{
		if ((charcode & 0xe0) == 0x80)
		{
			// Set nonhidden display attribute and blank the character
			// (90 = blink, 88 = reverse, 84 = underline, 82 = dim, 81 = blank, 80 = normal)
			m_cur_attr = charcode & 0x1f;
			attrcode = 0x01;
		}
		else
		{
			if (x == 0)
				m_cur_attr = m_last_row_attr;
			attrcode = (m_cur_attr & 0x1c) << 1 | (m_cur_attr & 0x03);
		}
	}

	u8 dots = 0;
	if (!BIT(attrcode, 5) || !blink)
	{
		if (BIT(attrcode, 3) && ul)
			dots = 0xff;
		else if (!BIT(attrcode, 0))
		{
			const u16 char_addr = u16(attrcode & 0xc0) << 5 | u16(charcode & 0x7f) << 4 | linecount;
			dots = m_fontram[char_addr];
		}
		if (BIT(attrcode, 4))
			dots = ~dots;
	}

	const bool cur = cursor && (!BIT(screen_attr, 5) || blink) && (!BIT(screen_attr, 7) || ul);
	if (cur)
		dots = ~dots;

	const rgb_t fg = (BIT(attrcode, 1) && !cur) ? rgb_t(0xc0, 0xc0, 0xc0) : rgb_t::white();
	for (int i = 0; i < 7; i++)
	{
		bitmap.pix(y, x++) = BIT(dots, 7) ? fg : rgb_t::black();
		dots <<= 1;
	}
	std::fill_n(&bitmap.pix(y, x), m_is_132 ? 2 : 3, BIT(dots, 7) ? fg : rgb_t::black());
}

void wy60_state::mbc_attr_clock_w(int state)
{
	if (state)
		m_last_row_attr = m_cur_attr;
}

u8 wy60_state::mbc_char_r(offs_t offset)
{
	u8 data = m_charram[offset & 0x1fff];

	// HACK
	if (offset >= 0x0010 && offset < 0x0050 && BIT(m_charram[0x0002], 6))
	{
		if (BIT(offset, 0))
			m_fontram[m_char_addr] = data;
		else
			m_char_addr = u16(m_charram[0x0001] & 0x60) << 6 | u16(data & 0x7f) << 4 | ((offset - 0x0010) & 0x001e) >> 1;
	}

	return data;
}

u8 wy60_state::mbc_attr_r(offs_t offset)
{
	return m_attrram[offset & 0x1fff];
}

u8 wy60_state::pvtc_r(offs_t offset)
{
	return m_pvtc->read(offset >> 8);
}

void wy60_state::pvtc_w(offs_t offset, u8 data)
{
	m_pvtc->write(offset >> 8, data);
}

u8 wy60_state::sio_r(offs_t offset)
{
	return m_sio->read(offset >> 8);
}

void wy60_state::sio_w(offs_t offset, u8 data)
{
	m_sio->write(offset >> 8, data);
}

void wy60_state::p1_w(u8 data)
{
	// P1.0 -> _80/132
	if (BIT(data, 0) != m_is_132)
	{
		m_is_132 = BIT(data, 0);
		m_pvtc->set_character_width(m_is_132 ? 9 : 10);
		m_pvtc->set_unscaled_clock(m_is_132 ? 39.71_MHz_XTAL / 9 : 26.58_MHz_XTAL / 10);
	}

	// P1.1 -> EEPROM SDA
	// P1.2 -> EEPROM SCL
	// N.B. The current i2cmem emulation is a bit sensitive to the order of these writes.
	m_eeprom->write_scl(BIT(data, 2));
	m_eeprom->write_sda(BIT(data, 1));

	// P1.3 -> AUX DSR
	m_aux->write_dtr(BIT(data, 3));

	// P1.5 -> KBD CMD (transmitted through 74LS368)
	m_keyboard->cmd_w(!BIT(data, 5));
}

u8 wy60_state::p1_r()
{
	// P1.1 <- EEPROM SDA
	// P1.4 <- AUX DTR
	// P1.6 <- KBD DATA (received through 74LS368)
	return (m_eeprom->read_sda() << 1) | (m_aux->dsr_r() << 4) | (m_keyboard->data_r() ? 0 : 0x40) | 0xad;
}

u8 wy60_state::p3_r()
{
	return m_aux->rxd_r() | 0xfe;
}

void wy60_state::ea_w(int state)
{
	if (state)
		m_internal_view.select(0);
	else
		m_internal_view.disable();
}

void wy60_state::prog_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("coderom", 0);
	map(0x0000, 0x0fff).view(m_internal_view); // FIXME: this view should be internal to the 8051 CPU device
	m_internal_view[0](0x0000, 0x0fff).rom().region("maincpu", 0);
}

void wy60_state::ext_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("charram");
	map(0x2000, 0x3fff).ram().share("attrram");
	map(0xa000, 0xa7ff).mirror(0x1800).rw(FUNC(wy60_state::pvtc_r), FUNC(wy60_state::pvtc_w));
	map(0xc000, 0xc3ff).mirror(0xc00).r(FUNC(wy60_state::sio_r));
	map(0xd000, 0xd3ff).mirror(0xc00).w(FUNC(wy60_state::sio_w));
}

void wy60_state::row_buffer_map(address_map &map)
{
	map.global_mask(0x0ff);
	map(0x000, 0x0ff).ram();
}


static INPUT_PORTS_START(wy60)
INPUT_PORTS_END

void wy60_state::wy60(machine_config &config)
{
	i8031_device &maincpu(I8031(config, "maincpu", 11_MHz_XTAL)); // AMD P8051AH-40196
	// FIXME: correct device type once 8051 core implements EA pin control
	maincpu.set_addrmap(AS_PROGRAM, &wy60_state::prog_map);
	maincpu.set_addrmap(AS_IO, &wy60_state::ext_map);
	maincpu.port_out_cb<1>().set(FUNC(wy60_state::p1_w));
	maincpu.port_in_cb<1>().set(FUNC(wy60_state::p1_r));
	maincpu.port_out_cb<3>().set(m_aux, FUNC(rs232_port_device::write_txd)).bit(1);
	maincpu.port_out_cb<3>().append(FUNC(wy60_state::ea_w)).bit(5);
	maincpu.port_in_cb<3>().set(FUNC(wy60_state::p3_r));

	I2C_X2404P(config, m_eeprom);

	WYSE_KEYBOARD(config, m_keyboard, wy60_keyboards, "ascii");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(26.58_MHz_XTAL, 1000, 0, 800 + 20, 443, 0, 416 + 16); // 26.580 kHz horizontal
	//screen.set_raw(39.71_MHz_XTAL, 1494, 0, 1188 + 18, 443, 0, 416 + 16);
	screen.set_screen_update("pvtc", FUNC(scn2672_device::screen_update));

	SCN2672(config, m_pvtc, 26.58_MHz_XTAL / 10); // custom-marked as Motorola SC67336P (205001-02)
	m_pvtc->set_screen("screen");
	m_pvtc->set_character_width(10); // 9 in 132-column mode
	m_pvtc->set_addrmap(0, &wy60_state::row_buffer_map);
	m_pvtc->set_addrmap(1, &wy60_state::row_buffer_map);
	m_pvtc->set_display_callback(FUNC(wy60_state::draw_character));
	m_pvtc->intr_callback().set_inputline("maincpu", MCS51_T0_LINE);
	m_pvtc->breq_callback().set_inputline("maincpu", MCS51_INT0_LINE);
	m_pvtc->mbc_callback().set(FUNC(wy60_state::mbc_attr_clock_w));
	m_pvtc->mbc_char_callback().set(FUNC(wy60_state::mbc_char_r));
	m_pvtc->mbc_attr_callback().set(FUNC(wy60_state::mbc_attr_r));

	SCN2661B(config, m_sio, 4.9152_MHz_XTAL);
	m_sio->rxrdy_handler().set_inputline("maincpu", MCS51_INT1_LINE);
	m_sio->txd_handler().set("modem", FUNC(rs232_port_device::write_txd));
	m_sio->dtr_handler().set("modem", FUNC(rs232_port_device::write_dtr));
	m_sio->rts_handler().set("modem", FUNC(rs232_port_device::write_rts));

	rs232_port_device &modem(RS232_PORT(config, "modem", default_rs232_devices, "loopback"));
	modem.rxd_handler().set(m_sio, FUNC(scn2661b_device::rxd_w));
	modem.cts_handler().set(m_sio, FUNC(scn2661b_device::cts_w));
	modem.dcd_handler().set(m_sio, FUNC(scn2661b_device::dcd_w));

	RS232_PORT(config, m_aux, default_rs232_devices, "loopback");
}

// CPU:   8051(202008-03)
// EPROM: 27512(193003-01)
// Video: 211003-02/205001-02
// RAM:   2064 (2064/2016/2016/2064)
// NVRAM: X2404
// UART:  2661
// XTALs: 39.710, 26.580, 11.000, 4.9152

ROM_START(wy60)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("202008-03_p8051ah-40196.bin", 0x0000, 0x1000, CRC(33a076cb) SHA1(ceb9ae74634b19b0192ed098802c5f551e0ed07f))

	ROM_REGION(0x10000, "coderom", 0)
	ROM_LOAD("193003-01.u9", 0x00000, 0x10000, CRC(26de0ea4) SHA1(91409f98a3990b514fbcb7de2eb45944bf5b95bc))
ROM_END

ROM_START(wy60a)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("202008-03_p8051ah-40196.bin", 0x0000, 0x1000, CRC(33a076cb) SHA1(ceb9ae74634b19b0192ed098802c5f551e0ed07f))

	ROM_REGION(0x10000, "coderom", 0)
	ROM_LOAD("wy-60_4k.u9", 0x00000, 0x10000, CRC(6daf2824) SHA1(23cd039ec7ae71b0742e8eebf75be8cd5992e3fd))
ROM_END

void wy60_state::driver_start()
{
	uint8_t *rom = memregion("coderom")->base();
	for (offs_t base = 0x00000; base < 0x10000; base += 0x2000)
	{
		std::vector<uint8_t> orig(&rom[base], &rom[base + 0x2000]);

		for (offs_t offset = 0; offset < 0x2000; offset++)
			rom[base | offset] = bitswap<8>(orig[bitswap<13>(offset, 0, 6, 9, 4, 2, 1, 3, 5, 7, 8, 10, 11, 12)], 6, 0, 5, 1, 4, 2, 3, 7);
	}
}

} // anonymous namespace

COMP(1986, wy60,  0,    0, wy60, wy60, wy60_state, empty_init, "Wyse Technology", "WY-60 (RBFNG2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
COMP(1986, wy60a, wy60, 0, wy60, wy60, wy60_state, empty_init, "Wyse Technology", "WY-60 (RBFNB0)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
