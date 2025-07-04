// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*************************************************************************************

    Yamaha MU-128 : 64-part, 128-note polyphonic/multitimbral General MIDI/XG
                    tone module
    Driver by O. Galibert

    Uses a dual SWP30 and a SH7043 as cpu.

**************************************************************************************/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/sh/sh7042.h"
#include "machine/i8251.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "sound/swp30.h"

#include "mulcd.h"
#include "speaker.h"

#include "mu128.lh"

namespace {

static INPUT_PORTS_START( mu128 )
INPUT_PORTS_END

class mu128_state : public driver_device
{
public:
	mu128_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "ram")
		, m_swp30m(*this, "swp30m")
		, m_swp30s(*this, "swp30s")
		, m_lcd(*this, "lcd")
		, m_sci(*this, "sci")
		, m_ram(*this, "ram")
	{ }

	void mu128(machine_config &config);

private:
	required_device<sh7043_device> m_maincpu;
	required_device<nvram_device> m_nvram;
	required_device<swp30_device> m_swp30m, m_swp30s;
	required_device<mulcd_device> m_lcd;
	required_device<i8251_device> m_sci;
	required_shared_ptr<u32> m_ram;

	u32 m_pa;
	u16 m_pe;

	void map(address_map &map) ATTR_COLD;
	void swp30_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u32 pa_r();
	void pa_w(u32 data);
	u16 pe_r();
	void pe_w(u16 data);
};

void mu128_state::machine_start()
{
	save_item(NAME(m_pa));
	save_item(NAME(m_pe));
	m_pa = 0;
	m_pe = 0;
}

void mu128_state::machine_reset()
{
}

// Port A:
//   8: sw5
//   9: sw6
//   f: rer
//  10: reb
//  11: rea
//  12: swd
//  14: led1
//  15: ry/by

// Port B:
//   2: sw1
//   3: sw2
//   4: sw3
//   5: sw4

// Port E:
//   0: lcd-r/w
//   1: nc
//   2: lcd-rs
//   3: lcread (secondary functions on lcread edge)
//   4: lcd-e
//   5: 1mclk
//   6: siclk
//   7: sws
//   8-a: db 0-2, lcdc a-c
//   b-c: db 3-4, mic/line
//   d:   db 5, swp reset
//   e:   db 6, sireset
//   f:   db 7, breset

u32 mu128_state::pa_r()
{
	return 0;
}

void mu128_state::pa_w(u32 data)
{
	m_pa = data;
}

u16 mu128_state::pe_r()
{
	if(BIT(m_pe, 4)) {
		if(BIT(m_pe, 0)) {
			if(BIT(m_pe, 2))
				return m_lcd->data_read() << 8;
			else
				return m_lcd->control_read() << 8;
		} else
			return 0x0000;
	}

	return 0;
}

void mu128_state::pe_w(u16 data)
{
	if(BIT(m_pe, 4) && !BIT(data, 4)) {
		if(!BIT(data, 0)) {
			if(BIT(data, 2))
				m_lcd->data_write(data >> 8);
			else
				m_lcd->control_write(data >> 8);
		}
	}

	m_pe = data;
}

void mu128_state::map(address_map &map)
{
	// 000000-3fffff: cs0 space, 32 bits, 2 wait states
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);

	// 400000-7fffff: cs1 space, 16 bits, 2 wait states
	map(0x400000, 0x43ffff).ram().share(m_ram);

	// 800000-bfffff: cs2 space, 16 bits, cs assert extension, 3 wait states
	map(0x800000, 0x801fff).m(m_swp30m, FUNC(swp30_device::map));
	map(0x802000, 0x803fff).m(m_swp30s, FUNC(swp30_device::map));

	// c00000-ffffff: cs3 space, 8 bits, 2 wait states
	map(0xc00000, 0xc00000).rw(m_sci, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w)).mirror(0x3ffffe);
	map(0xc00001, 0xc00001).rw(m_sci, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w)).mirror(0x3ffffe);
}

void mu128_state::swp30_map(address_map &map)
{
	map(0x000000, 0x5fffff).rom().region("swp30", 0);
}

void mu128_state::mu128(machine_config &config)
{
	SH7043(config, m_maincpu, 7_MHz_XTAL * 4); // md=9, no on-chip rom, 32-bit space, pll 4x
	m_maincpu->set_addrmap(AS_PROGRAM, &mu128_state::map);
	m_maincpu->read_porta().set(FUNC(mu128_state::pa_r));
	m_maincpu->write_porta().set(FUNC(mu128_state::pa_w));
	m_maincpu->read_porte().set(FUNC(mu128_state::pe_r));
	m_maincpu->write_porte().set(FUNC(mu128_state::pe_w));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_NONE);

	MULCD(config, m_lcd);

	SPEAKER(config, "speaker", 2).front();

	SWP30(config, m_swp30m);
	m_swp30m->set_addrmap(AS_DATA, &mu128_state::swp30_map);
	m_swp30m->add_route(0, "speaker", 1.0, 0);
	m_swp30m->add_route(1, "speaker", 1.0, 1);

	SWP30(config, m_swp30s);
	m_swp30s->set_addrmap(AS_DATA, &mu128_state::swp30_map);
	m_swp30s->add_route(0, "speaker", 1.0, 0);
	m_swp30s->add_route(1, "speaker", 1.0, 1);

	INPUT_MERGER_ANY_HIGH(config, "irq0").output_handler().set_inputline(m_maincpu, 0);
	I8251(config, m_sci, 10_MHz_XTAL); // uPD71051GU-10
	m_sci->rxrdy_handler().set("irq0", FUNC(input_merger_device::in_w<0>));
	m_sci->txrdy_handler().set("irq0", FUNC(input_merger_device::in_w<1>));

	auto &mdin_a(MIDI_PORT(config, "mdin_a"));
	midiin_slot(mdin_a);
	mdin_a.rxd_handler().set(m_maincpu, FUNC(sh7043_device::sci_rx_w<0>));

	auto &mdin_b(MIDI_PORT(config, "mdin_b"));
	midiin_slot(mdin_b);
	mdin_b.rxd_handler().set(m_maincpu, FUNC(sh7043_device::sci_rx_w<1>));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_maincpu->write_sci_tx<0>().set(mdout, FUNC(midi_port_device::write_txd));

	config.set_default_layout(layout_mu128);
}

#define ROM_LOAD32_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
	ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(bios))

ROM_START( mu128 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("v200")
	ROM_SYSTEM_BIOS( 0, "v200", "Upgrade package (Ver2.00 99-MAY-21)" )
	ROM_LOAD32_WORD_SWAP_BIOS( 0, "mu128-v2.00-h.bin", 0x000000, 0x100000, CRC(2891487b) SHA1(8ed4a6929c66fcb5248e16288dfaf56a3286aaf8) )
	ROM_LOAD32_WORD_SWAP_BIOS( 0, "mu128-v2.00-l.bin", 0x000002, 0x100000, CRC(cc236dc4) SHA1(e1ff3387968e89f5bc5df3e15cd0d6039104acd0) )
	ROM_SYSTEM_BIOS( 1, "v106", "c0 (Ver1.06 98-OCT-01)" )
	ROM_LOAD32_WORD_SWAP_BIOS( 1, "xv217c0.ic27", 0x000000, 0x100000, CRC(f4ba61f1) SHA1(381e1d146c4e693a21f6e6e4ea2a8b9f6e3033ef) )
	ROM_LOAD32_WORD_SWAP_BIOS( 1, "xv224c0.ic25", 0x000002, 0x100000, CRC(079bfcf0) SHA1(56d69f3214899fa25ef5e9ea6c2bbf0c3d378123) )

	ROM_REGION32_LE( 0x1800000, "swp30", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD( "xv364a0.ic53", 0x0000000, 0x800000, CRC(cda1afd6) SHA1(e7098246b33c3cf22ed8cc15ed6383f8a06d17e9) )
	ROM_LOAD32_WORD( "xv365a0.ic54", 0x0000002, 0x800000, CRC(10985ed0) SHA1(d45a2e85859e05046f3ede8317a9bb0b88898116) )
	ROM_LOAD32_WORD( "xv366a0.ic57", 0x1000000, 0x400000, CRC(781dfac6) SHA1(e6b8b7cf95e4e9552001450570fcea87282db1e8) )
	ROM_LOAD32_WORD( "xv376a0.ic58", 0x1000002, 0x400000, CRC(91a7533b) SHA1(c30888603fd5db367d14553763f0a3f392c5427c) )
ROM_END

} // anonymous namespace


CONS( 1998, mu128, 0, 0, mu128,  mu128, mu128_state, empty_init, "Yamaha", "MU128", MACHINE_NOT_WORKING )
