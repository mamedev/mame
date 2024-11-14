// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for HP-2620 series display terminals.

************************************************************************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/input_merger.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
#include "machine/ripple_counter.h"
#include "sound/spkrdev.h"
#include "video/dp8350.h"
#include "screen.h"
#include "speaker.h"


namespace {

class hp2620_state : public driver_device
{
public:
	hp2620_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nmigate(*this, "nmigate")
		, m_crtc(*this, "crtc")
		, m_datacomm(*this, "datacomm")
		, m_bell(*this, "bell")
		, m_bellctr(*this, "bellctr")
		, m_p_chargen(*this, "chargen")
		, m_nvram(*this, "nvram")
	{ }

	void hp2622(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 nvram_r(offs_t offset);
	void nvram_w(offs_t offset, u8 data);
	u8 keystat_r();
	void keydisp_w(u8 data);
	u8 sysstat_r();
	void modem_w(u8 data);
	void crtc_w(offs_t offset, u8 data);
	void ennmi_w(offs_t offset, u8 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void nlrc_w(int state);
	void bell_w(int state);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_nmigate;
	required_device<dp8367_device> m_crtc;
	required_device<rs232_port_device> m_datacomm;
	required_device<speaker_sound_device> m_bell;
	required_device<ripple_counter_device> m_bellctr;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_nvram;

	u16 m_display_page = 0;
	u8 m_key_status = 0;
};


u32 hp2620_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

u8 hp2620_state::nvram_r(offs_t offset)
{
	// TODO: wait states
	return 0xf0 | m_nvram[offset];
}

void hp2620_state::nvram_w(offs_t offset, u8 data)
{
	// TODO: wait states
	m_nvram[offset] = data & 0x0f;
}

u8 hp2620_state::keystat_r()
{
	// LS299 at U25
	return m_key_status;
}

void hp2620_state::keydisp_w(u8 data)
{
	// LS374 at U26

	// TODO: D0-D3 = KEY3-KEY6
	m_bellctr->reset_w(BIT(data, 4));
	// TODO: D6 = BLINK RATE
	// TODO: D7 = ENHOFF
}

u8 hp2620_state::sysstat_r()
{
	// LS244 at U36
	u8 status = 0xe0;

	status |= m_crtc->vblank_r() << 0;
	// TODO: D1 = PINT
	status |= m_datacomm->dsr_r() << 2; // DM (J6-12)
	status |= m_datacomm->cts_r() << 3; // CS (J6-11)
	status |= m_datacomm->ri_r() << 4; // OCR1 (J6-18)

	return status;
}

void hp2620_state::modem_w(u8 data)
{
	// LS174 at U35 (TODO: D0 = DISPOFF)
	m_datacomm->write_spds(BIT(data, 1)); // OCD1 (J6-7)
	m_crtc->refresh_control(BIT(data, 5));
}

void hp2620_state::crtc_w(offs_t offset, u8 data)
{
	m_crtc->register_load(data & 3, offset & 0xfff);
	m_display_page = offset & 0x3000;
}

void hp2620_state::ennmi_w(offs_t offset, u8 data)
{
	m_nmigate->in_w<0>(BIT(offset, 0));
}

void hp2620_state::nlrc_w(int state)
{
	// clock input for LS175 at U59
	if (state)
		m_nmigate->in_w<1>((m_crtc->lc_r() & 7) == 3);

	// TODO: shift keyboard response into m_key_status
}

void hp2620_state::bell_w(int state)
{
	m_bell->level_w(state);
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
	map(0x88, 0x89).w(FUNC(hp2620_state::ennmi_w));
	map(0x90, 0x90).r(FUNC(hp2620_state::sysstat_r));
	map(0xa0, 0xa3).w("acia", FUNC(mos6551_device::write));
	map(0xa4, 0xa7).r("acia", FUNC(mos6551_device::read));
	map(0xa8, 0xa8).w(FUNC(hp2620_state::modem_w));
	map(0xb8, 0xb8).w(FUNC(hp2620_state::keydisp_w));
	map(0xe1, 0xe1).nopw(); // program bug, undocumented expansion or ???
}

void hp2620_state::machine_start()
{
	save_item(NAME(m_display_page));
	save_item(NAME(m_key_status));
}

void hp2620_state::machine_reset()
{
	m_nmigate->in_w<1>(0);
	m_display_page = 0;
	m_key_status = 0;
}

static INPUT_PORTS_START( hp2622 )
INPUT_PORTS_END

void hp2620_state::hp2622(machine_config &config)
{
	Z80(config, m_maincpu, 25.7715_MHz_XTAL / 7); // 3.68 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &hp2620_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &hp2620_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 5101 (A7 tied to GND) + battery (+ wait states)

	INPUT_MERGER_ALL_HIGH(config, m_nmigate).output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(hp2620_state::screen_update));

	DP8367(config, m_crtc, 25.7715_MHz_XTAL).set_screen("screen");
	m_crtc->set_half_shift(true);
	m_crtc->hsync_callback().set(m_bellctr, FUNC(ripple_counter_device::clock_w)).invert();
	m_crtc->lrc_callback().set(FUNC(hp2620_state::nlrc_w)).invert();

	mos6551_device &acia(MOS6551(config, "acia", 0)); // SY6551
	acia.set_xtal(25.7715_MHz_XTAL / 14); // 1.84 MHz
	acia.irq_handler().set_inputline("maincpu", INPUT_LINE_IRQ0);
	acia.rts_handler().set(m_datacomm, FUNC(rs232_port_device::write_rts)); // RS (J6-22)
	acia.dtr_handler().set(m_datacomm, FUNC(rs232_port_device::write_dtr)); // TR (J6-23)
	acia.txd_handler().set(m_datacomm, FUNC(rs232_port_device::write_txd)); // SD (J6-21)

	RS232_PORT(config, m_datacomm, default_rs232_devices, nullptr);
	m_datacomm->rxd_handler().set("acia", FUNC(mos6551_device::write_rxd)); // RD (J6-9)

	RIPPLE_COUNTER(config, m_bellctr); // LS393 at U114
	m_bellctr->set_stages(8);
	m_bellctr->count_out_cb().set(FUNC(hp2620_state::bell_w)).bit(4);

	SPEAKER(config, "mono").front_center(); // on keyboard
	SPEAKER_SOUND(config, m_bell).add_route(ALL_OUTPUTS, "mono", 0.50);
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
	// XU66-XU68 are empty sockets

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD( "1818-1489.xu311", 0x0000, 0x2000, CRC(9879b153) SHA1(fc1705d6de38eb6d3a67f1ae439e359e5124d028) )
ROM_END

} // anonymous namespace


COMP(1982, hp2622a, 0, 0, hp2622, hp2622, hp2620_state, empty_init, "HP", "HP-2622A", MACHINE_NOT_WORKING)
