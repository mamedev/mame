// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Sharp FontWriter series

    Skeleton driver by R. Belmont

    Main CPU: ROMless Mitsubishi M37720
    FDC: NEC 72068 (entire PC controller on a chip)
    512k RAM
    Custom gate array
    AT28C16 parallel EPROM
    640x400 dot-matrix LCD

    Things to check
    - Hook up 37720 DMAC, it's used before this dies
    - Check if "stack in bank FF" bit is used
    - Verify timer implementation

****************************************************************************/

#include "emu.h"
#include "cpu/m37710/m37710.h"
#include "machine/nvram.h"
#include "machine/at28c16.h"
#include "screen.h"
#include "speaker.h"


namespace {

class fontwriter_state : public driver_device
{
public:
	fontwriter_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void fontwriter(machine_config &config);
	void fw600(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t vbl_r()
	{
		m_vbl ^= 0xff;
		return m_vbl;
	}
	uint8_t vbl2_r()
	{
		m_vbl2 ^= 0x88;
		return m_vbl;
	}
	void main_map(address_map &map) ATTR_COLD;
	void fw600_map(address_map &map) ATTR_COLD;

	// devices
	required_device<m37720s1_device> m_maincpu;

	// driver_device overrides
	virtual void video_start() override ATTR_COLD;
	uint8_t m_vbl = 0, m_vbl2 = 0;
};

void fontwriter_state::machine_reset()
{
	m_vbl = 0;
}

void fontwriter_state::machine_start()
{
}

void fontwriter_state::video_start()
{
}

uint32_t fontwriter_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void fontwriter_state::main_map(address_map &map)
{
	map(0x002000, 0x007fff).ram();
	map(0x008000, 0x00ffff).rom().region("maincpu", 0x0000);
	map(0x020000, 0x04ffff).ram();
	map(0x100000, 0x1007ff).rw("at28c16", FUNC(at28c16_device::read), FUNC(at28c16_device::write));
	map(0x200000, 0x3fffff).rom().region("maincpu", 0x0000);
}

void fontwriter_state::fw600_map(address_map &map)
{
	map(0x000280, 0x0002ff).ram();
	map(0x000800, 0x000fff).rw("at28c16", FUNC(at28c16_device::read), FUNC(at28c16_device::write));
	map(0x002000, 0x007fff).ram();
	map(0x008000, 0x00ffff).rom().region("maincpu", 0x1f8000);
	map(0x020000, 0x04ffff).ram();
	map(0x200000, 0x3fffff).rom().region("maincpu", 0x0000);
}

static INPUT_PORTS_START( fontwriter )
INPUT_PORTS_END

void fontwriter_state::fontwriter(machine_config &config)
{
	M37720S1(config, m_maincpu, XTAL(16'000'000)); /* M37720S1 @ 16MHz - main CPU */
	m_maincpu->set_addrmap(AS_PROGRAM, &fontwriter_state::main_map);
	m_maincpu->p6_in_cb().set(FUNC(fontwriter_state::vbl_r));
	m_maincpu->p7_in_cb().set(FUNC(fontwriter_state::vbl2_r));

	AT28C16(config, "at28c16", 0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_screen_update(FUNC(fontwriter_state::screen_update));
	screen.set_size(640, 400);
	screen.set_visarea_full();
}

void fontwriter_state::fw600(machine_config &config)
{
	M37720S1(config, m_maincpu, XTAL(16'000'000)); /* M37720S1 @ 16MHz - main CPU */
	m_maincpu->set_addrmap(AS_PROGRAM, &fontwriter_state::fw600_map);
	m_maincpu->p6_in_cb().set(FUNC(fontwriter_state::vbl_r));
	m_maincpu->p7_in_cb().set(FUNC(fontwriter_state::vbl2_r));

	AT28C16(config, "at28c16", 0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_screen_update(FUNC(fontwriter_state::screen_update));
	screen.set_size(640, 400);
	screen.set_visarea_full();
}

ROM_START(fw600)
	ROM_REGION(0x200000, "maincpu", 0)      /* M37720 program ROM */
	ROM_LOAD( "lh5388n5.bin", 0x000000, 0x100000, CRC(3bcc5c19) SHA1(510e3795faf18e10f2fef69110f96183e7cfee35) )
	ROM_LOAD( "lh5388n9.bin", 0x100000, 0x100000, CRC(be2198df) SHA1(9e42f3a933c6f247c452910af3a2e9196291574a) )

	ROM_REGION(0x800, "at28c16", 0)         /* AT28C16 parallel EPROM */
	ROM_LOAD( "at28c16.bin",  0x000000, 0x000800, CRC(a84eafd9) SHA1(12503a71e98f80819959d41643b1d2773739b923) )
ROM_END

ROM_START(fw700ger)
	ROM_REGION(0x200000, "maincpu", 0)       /* M37720 program ROM */
	ROM_LOAD( "lh5370pd.ic7", 0x000000, 0x200000, CRC(29083e13) SHA1(7e1605f91b53580e75f638f9e6b0917305c35f84) )

	ROM_REGION(0x800, "at28c16", ROMREGION_ERASE00)         /* AT28C16 parallel EPROM */
ROM_END

} // anonymous namespace


SYST( 1994, fw600,    0, 0, fw600, fontwriter, fontwriter_state, empty_init, "Sharp", "FontWriter FW-600", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
SYST( 1994, fw700ger, 0, 0, fontwriter, fontwriter, fontwriter_state, empty_init, "Sharp", "FontWriter FW-700 (German)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
