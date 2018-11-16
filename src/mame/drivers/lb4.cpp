// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/****************************************************************************

    Liberty Electronics LB-4 serial terminal

    Known hardware:
    - Motorola 6800 CPU
    - Hitachi HD46505 (Motorola 6845-compatible) CRTC
    - Hitachi HD46850 (Motorola 6850-compatible) ACIA
    - Zilog Z8430 CTC
    - 16.6698MHz Crystal

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6850acia.h"
#include "machine/z80ctc.h"
#include "video/mc6845.h"
#include "screen.h"
#include "emupal.h"

#define MASTER_CLOCK	16.6698_MHz_XTAL

class lb4_state : public driver_device
{
public:
	lb4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_acia(*this, "acia")
		, m_ctc(*this, "ctc")
		, m_crtc(*this, "crtc")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void lb4(machine_config &config);

private:
	void mem_map(address_map &map);

	required_device<m6800_cpu_device> m_maincpu;
	required_device<acia6850_device> m_acia;
	required_device<z80ctc_device> m_ctc;
	required_device<h46505_device> m_crtc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

void lb4_state::mem_map(address_map &map)
{
	// There are what look like RAM reads/writes from 0000-03ff, but also accesses which look like attempts to talk to a device.
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x8000).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x8001, 0x8001).w(m_crtc, FUNC(mc6845_device::register_w));
	map(0xe000, 0xffff).rom().region("maincpu", 0);
}

static const gfx_layout char_layout =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chars", 0, char_layout, 0, 1)
GFXDECODE_END

void lb4_state::lb4(machine_config &config)
{
	// All dividers unknown/guessed
	M6800(config, m_maincpu, MASTER_CLOCK / 16);
	m_maincpu->set_addrmap(AS_PROGRAM, &lb4_state::mem_map);

	ACIA6850(config, m_acia, MASTER_CLOCK / 128);

	Z80CTC(config, m_ctc, MASTER_CLOCK / 4);

    H46505(config, m_crtc, MASTER_CLOCK);

	GFXDECODE(config, m_gfxdecode, m_palette, chars);

	PALETTE(config, m_palette, 2).set_init("palette", FUNC(palette_device::palette_init_monochrome));
}

static INPUT_PORTS_START( lb4 )
INPUT_PORTS_END

ROM_START( lb4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u8.bin", 0x0000, 0x2000, CRC(2e375abc) SHA1(12ad1e49c5773c36c3a8d65845c9a50f9dec141f) )

	ROM_REGION( 0x1000, "chars", 0 )
	ROM_LOAD( "u32.bin", 0x0000, 0x1000, CRC(f6d86e87) SHA1(c0885e4a35095a730d760bf91a1cf4e8edd6a2bb) )
ROM_END

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT    CLASS      INIT        COMPANY                FULLNAME  FLAGS
COMP( 197?, lb4,  0,      0,      lb4,     lb4,     lb4_state, empty_init, "Liberty Electronics", "LB-4",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
