// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for HP-2620 series display terminals.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
#include "video/dp8350.h"
#include "screen.h"

class hp2620_state : public driver_device
{
public:
	hp2620_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_p_chargen(*this, "chargen")
		, m_nvram(*this, "nvram")
	{ }

	void hp2622(machine_config &config);

private:
	DECLARE_READ8_MEMBER(nvram_r);
	DECLARE_WRITE8_MEMBER(nvram_w);
	DECLARE_READ8_MEMBER(keystat_r);
	DECLARE_WRITE8_MEMBER(keydisp_w);
	DECLARE_READ8_MEMBER(sysstat_r);
	DECLARE_WRITE8_MEMBER(modem_w);
	void crtc_w(offs_t offset, u8 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<dp8367_device> m_crtc;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_nvram;
};


u32 hp2620_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

READ8_MEMBER(hp2620_state::nvram_r)
{
	return 0xf0 | m_nvram[offset];
}

WRITE8_MEMBER(hp2620_state::nvram_w)
{
	m_nvram[offset] = data & 0x0f;
}

READ8_MEMBER(hp2620_state::keystat_r)
{
	return 0xff;
}

WRITE8_MEMBER(hp2620_state::keydisp_w)
{
}

READ8_MEMBER(hp2620_state::sysstat_r)
{
	return 0xff;
}

WRITE8_MEMBER(hp2620_state::modem_w)
{
}

void hp2620_state::crtc_w(offs_t offset, u8 data)
{
	m_crtc->register_load(data & 3, offset & 0xfff);
}

void hp2620_state::mem_map(address_map &map)
{
	map(0x0000, 0xbfff).rom().region("maincpu", 0);
	map(0x8000, 0xbfff).w(FUNC(hp2620_state::crtc_w));
	map(0xc000, 0xffff).ram();
}

void hp2620_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).rw(FUNC(hp2620_state::nvram_r), FUNC(hp2620_state::nvram_w)).share("nvram");
	map(0x80, 0x80).r(FUNC(hp2620_state::keystat_r));
	map(0x90, 0x90).r(FUNC(hp2620_state::sysstat_r));
	map(0xa0, 0xa3).w("acia", FUNC(mos6551_device::write));
	map(0xa4, 0xa7).r("acia", FUNC(mos6551_device::read));
	map(0xa8, 0xa8).w(FUNC(hp2620_state::modem_w));
	map(0xb8, 0xb8).w(FUNC(hp2620_state::keydisp_w));
}

static INPUT_PORTS_START( hp2622 )
INPUT_PORTS_END

void hp2620_state::hp2622(machine_config &config)
{
	Z80(config, m_maincpu, 25.7715_MHz_XTAL / 7); // 3.68 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &hp2620_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &hp2620_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 5101 (A7 tied to GND) + battery (+ wait states)

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(hp2620_state::screen_update));

	DP8367(config, m_crtc, 25.7715_MHz_XTAL).set_screen("screen");
	m_crtc->set_half_shift(true);

	mos6551_device &acia(MOS6551(config, "acia", 0)); // SY6551
	acia.set_xtal(25.7715_MHz_XTAL / 14); // 1.84 MHz
	acia.irq_handler().set_inputline("maincpu", INPUT_LINE_IRQ0);
}

/**************************************************************************************************************

Hewlett-Packard HP-2622A.
Chips: National 8367 CRTC (labeled B8250), SY6551 (labeled 8251), Z8400A (Z80)
Crystal: 25.7715

***************************************************************************************************************/

ROM_START( hp2622a )
	ROM_REGION(0xc000, "maincpu", 0)
	ROM_LOAD( "1818-1685.xu63", 0x0000, 0x2000, CRC(a57ffe5e) SHA1(4d7844320deba916d9ec289927af987fea025b02) )
	ROM_LOAD( "1818-1686.xu64", 0x2000, 0x2000, CRC(bee9274c) SHA1(20796c559031a91cb2666776fcf7ffdb52a0a318) )
	ROM_LOAD( "1818-1687.xu65", 0x4000, 0x2000, CRC(e9ecd489) SHA1(9b249b8d066d256069ccdb8809bb808c414f106a) )
	// XU66-XU68 are empty

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD( "1818-1489.xu311", 0x0000, 0x2000, CRC(9879b153) SHA1(fc1705d6de38eb6d3a67f1ae439e359e5124d028) )
ROM_END

COMP( 1982, hp2622a, 0, 0, hp2622, hp2622, hp2620_state, empty_init, "HP", "HP-2622A", MACHINE_IS_SKELETON )
