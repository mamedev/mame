// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    Skeleton driver for Wyse WY-60 terminal.

***********************************************************************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
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
		, m_pvtc(*this, "pvtc")
		, m_sio(*this, "sio")
		, m_charram(*this, "charram")
		, m_attrram(*this, "attrram")
		, m_fontram(*this, "fontram", 0x2000, ENDIANNESS_LITTLE)
		, m_internal_view(*this, "8051_internal")
		, m_char_addr(0)
		, m_is_132(false)
	{
	}

	void wy60(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void driver_start() override;

private:
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);
	u8 mbc_char_r(offs_t offset);
	u8 mbc_attr_r(offs_t offset);

	u8 pvtc_r(offs_t offset);
	void pvtc_w(offs_t offset, u8 data);
	u8 sio_r(offs_t offset);
	void sio_w(offs_t offset, u8 data);

	void p1_w(u8 data);
	u8 p1_r();
	DECLARE_WRITE_LINE_MEMBER(ea_w);

	void prog_map(address_map &map);
	void ext_map(address_map &map);
	void row_buffer_map(address_map &map);

	required_device<i2cmem_device> m_eeprom;
	required_device<scn2672_device> m_pvtc;
	required_device<scn2661b_device> m_sio;
	required_shared_ptr<u8> m_charram;
	required_shared_ptr<u8> m_attrram;
	memory_share_creator<u8> m_fontram;
	memory_view m_internal_view;

	u16 m_char_addr;
	bool m_is_132;
};


void wy60_state::machine_start()
{
	save_item(NAME(m_char_addr));
	save_item(NAME(m_is_132));
}

SCN2672_DRAW_CHARACTER_MEMBER(wy60_state::draw_character)
{
	const int char_width = m_is_132 ? 9 : 10;

	// TODO: attributes
	const u16 char_addr = u16(m_charram[0x0001] & 0x60) << 6 | u16(charcode & 0x7f) << 4 | linecount;
	u8 dots = m_fontram[char_addr];
	for (int i = 0; i < char_width; i++)
	{
		bitmap.pix(y, x++) = BIT(dots, 7) ? rgb_t::white() : rgb_t::black();
		dots <<= 1;
	}
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
	if (BIT(data, 0) != m_is_132)
	{
		m_is_132 = BIT(data, 0);
		m_pvtc->set_character_width(m_is_132 ? 9 : 10);
		m_pvtc->set_unscaled_clock(m_is_132 ? 39.71_MHz_XTAL / 9 : 26.58_MHz_XTAL / 10);
	}

	// N.B. The current i2cmem emulation is a bit sensitive to the order of these writes.
	m_eeprom->write_scl(BIT(data, 2));
	m_eeprom->write_sda(BIT(data, 1));

	// TODO: P1.3 -> AUX DSR
	// TODO: P1.5 -> KEYBOARD CMD
}

u8 wy60_state::p1_r()
{
	// TODO: P1.4 <- AUX DTR
	// TODO: P1.6 <- KEYBOARD DATA
	return (m_eeprom->read_sda() << 1) | 0xfd;
}

WRITE_LINE_MEMBER(wy60_state::ea_w)
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
	maincpu.port_out_cb<3>().set(FUNC(wy60_state::ea_w)).bit(5);

	I2C_X2404P(config, m_eeprom);

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
	m_pvtc->mbc_char_callback().set(FUNC(wy60_state::mbc_char_r));
	m_pvtc->mbc_attr_callback().set(FUNC(wy60_state::mbc_attr_r));

	SCN2661B(config, m_sio, 4.9152_MHz_XTAL);
	m_sio->rxrdy_handler().set_inputline("maincpu", MCS51_INT1_LINE);
	m_sio->txd_handler().set("modem", FUNC(rs232_port_device::write_txd));
	m_sio->dtr_handler().set("modem", FUNC(rs232_port_device::write_dtr));
	m_sio->rts_handler().set("modem", FUNC(rs232_port_device::write_rts));

	rs232_port_device &modem(RS232_PORT(config, "modem", default_rs232_devices, nullptr));
	modem.rxd_handler().set(m_sio, FUNC(scn2661b_device::rxd_w));
	modem.cts_handler().set(m_sio, FUNC(scn2661b_device::cts_w));
	modem.dcd_handler().set(m_sio, FUNC(scn2661b_device::dcd_w));

	// TODO: AUX port connected to 8051
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

COMP(1986, wy60,  0,    0, wy60, wy60, wy60_state, empty_init, "Wyse Technology", "WY-60 (RBFNG2)", MACHINE_IS_SKELETON)
COMP(1986, wy60a, wy60, 0, wy60, wy60, wy60_state, empty_init, "Wyse Technology", "WY-60 (RBFNB0)", MACHINE_IS_SKELETON)
