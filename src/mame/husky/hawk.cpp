// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Husky Hawk

    No schematics or manuals available.

    Known RAM configurations:
    Hawk - 352K

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/z180/z180.h"
#include "machine/msm6242.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class hawk_state : public driver_device
{
public:
	hawk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_speaker(*this, "speaker")
		, m_rs232(*this, "serial")
		, m_ram(*this, RAM_TAG)
		, m_nvram(*this, "nvram")
	{ }

	void hawk(machine_config &config);

	void init_hawk();

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	void hawk_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void hawk_mem(address_map &map) ATTR_COLD;
	void hawk_io(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<speaker_sound_device> m_speaker;
	required_device<rs232_port_device> m_rs232;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
};

void hawk_state::hawk_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0010, 0x0010).mirror(0xff00).nopw();
	map(0x0018, 0x0018).nopw();
	map(0x003f, 0x003f).nopw();
	map(0x0080, 0x00bf).noprw(); // Z180 internal registers
	map(0x00c0, 0x00cf).mirror(0xff00).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write));
}

void hawk_state::hawk_mem(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x7ffff).ram().share(RAM_TAG);
}

static INPUT_PORTS_START( hawk )
INPUT_PORTS_END

void hawk_state::machine_reset()
{
}

void hawk_state::init_hawk()
{
	m_nvram->set_base(m_ram->pointer(), m_ram->size());
}

uint32_t hawk_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// video RAM at 78000 ??
	return 0;
}

void hawk_state::hawk_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}


void hawk_state::hawk(machine_config &config)
{
	/* basic machine hardware */
	Z80180(config, m_maincpu, 12.288_MHz_XTAL); /* HD64B180R0F */
	m_maincpu->set_addrmap(AS_PROGRAM, &hawk_state::hawk_mem);
	m_maincpu->set_addrmap(AS_IO, &hawk_state::hawk_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(hawk_state::screen_update));
	screen.set_size(240, 64);
	screen.set_visarea(0, 239, 0, 63);
	PALETTE(config, m_palette, FUNC(hawk_state::hawk_palette), 2);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* rtc */
	MSM6242(config, "rtc", 32.768_kHz_XTAL); // M6242B

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);

	/* internal ram */
	RAM(config, m_ram).set_default_size("352K");
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}


ROM_START(hawk)
	ROM_REGION(0x20000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "120", "DEMOS 2.21 V1.20")
	ROMX_LOAD("hawk-v1.20.rom",   0x00000, 0x20000, CRC(186b1974) SHA1(eb46041715266cc263d71aad4f09cdf0c913cd16), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "110", "DEMOS 2.21 V1.10")
	ROMX_LOAD("hawk-v1.10-0.rom", 0x00000, 0x08000, CRC(c933ed5c) SHA1(f7b8375099ade54e3573467bf8e7f62624958e4c), ROM_BIOS(1))
	ROMX_LOAD("hawk-v1.10-1.rom", 0x08000, 0x08000, CRC(121b5ce0) SHA1(baf06bc0d16501b50cbf97b686612b00098c73ab), ROM_BIOS(1))
	ROMX_LOAD("hawk-v1.10-2.rom", 0x10000, 0x08000, CRC(cd5d94c7) SHA1(44c996b4bf00185ccb303cf9ef9bbd705018390c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "110a", "DEMOS 2.21 V1.10 (alt)")
	ROMX_LOAD("v1.10_0.rom",      0x00000, 0x08000, CRC(7ad48dcf) SHA1(0e2b82dec0a082f4a8032b7a64cc9afdbf421521), ROM_BIOS(2))
	ROMX_LOAD("v1.10_1.rom",      0x08000, 0x08000, CRC(121b5ce0) SHA1(baf06bc0d16501b50cbf97b686612b00098c73ab), ROM_BIOS(2))
	ROMX_LOAD("v1.10_2.rom",      0x10000, 0x08000, CRC(cd5d94c7) SHA1(44c996b4bf00185ccb303cf9ef9bbd705018390c), ROM_BIOS(2))
	ROMX_LOAD("56189.rom",        0x18000, 0x08000, CRC(1b2db82b) SHA1(2185d27816bde263c62db1a2441d8a6d2cb6d193), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "101", "DEMOS 2.21 V1.01")
	ROMX_LOAD("hawk-v1.01-0.rom", 0x00000, 0x08000, CRC(96404435) SHA1(3b108d6906ecc7d7a36c36b993e79ec7480668fa), ROM_BIOS(3))
	ROMX_LOAD("hawk-v1.01-1.rom", 0x08000, 0x08000, CRC(4f99c76c) SHA1(45ff638277cff7b1fb2e21c4c348dad2b2e779b7), ROM_BIOS(3))
	ROMX_LOAD("hawk-v1.01-2.rom", 0x10000, 0x08000, CRC(982ed053) SHA1(ab0a860f1204f36f490fdfadfefe2ee4a82ed3be), ROM_BIOS(3))
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT  CLASS       INIT       COMPANY                FULLNAME      FLAGS
COMP( 1987, hawk,    0,       0,      hawk,    hawk,  hawk_state, init_hawk, "Husky Computers Ltd", "Husky Hawk", MACHINE_IS_SKELETON )
