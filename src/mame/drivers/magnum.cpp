// license:BSD-3-Clause
// copyright-holders:Carl

// Dulmont Magnum
// Additional info https://www.youtube.com/watch?v=st7H_vqSaQc and
// http://www.eevblog.com/forum/blog/eevblog-949-vintage-australian-made-laptop-teardown/msg1080508/#msg1080508

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/cdp1879.h"
#include "video/hd61830.h"
#include "sound/beep.h"
#include "rendlay.h"
#include "screen.h"
#include "speaker.h"

class magnum_state : public driver_device
{
public:
	magnum_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_beep(*this, "beep")
	{
	}
	void magnum(machine_config &config);
protected:
	virtual void machine_reset() override;
	virtual void machine_start() override;

private:
	DECLARE_WRITE8_MEMBER(beep_w);

	void magnum_io(address_map &map);
	void magnum_map(address_map &map);
	void magnum_lcdc(address_map &map);

	required_device<beep_device> m_beep;
};

void magnum_state::machine_start()
{
}

void magnum_state::machine_reset()
{
}

WRITE8_MEMBER(magnum_state::beep_w)
{
	if (data & ~1) printf("beep_w unmapped bits %02x\n", data);
	m_beep->set_state(BIT(data, 0));
}

void magnum_state::magnum_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram(); // fixed 256k for now
	map(0xe0000, 0xfffff).rom().region("bios", 0);
}

void magnum_state::magnum_io(address_map &map)
{
	map.unmap_value_high();
	//AM_RANGE(0x000a, 0x000b) cdp1854 1
	//AM_RANGE(0x000e, 0x000f) cpd1854 2
	map(0x0018, 0x0019).rw("lcdc2", FUNC(hd61830_device::data_r), FUNC(hd61830_device::data_w)).umask16(0x00ff);
	map(0x001a, 0x001b).rw("lcdc2", FUNC(hd61830_device::status_r), FUNC(hd61830_device::control_w)).umask16(0x00ff);
	map(0x001c, 0x001d).rw("lcdc1", FUNC(hd61830_device::data_r), FUNC(hd61830_device::data_w)).umask16(0x00ff);
	map(0x001e, 0x001f).rw("lcdc1", FUNC(hd61830_device::status_r), FUNC(hd61830_device::control_w)).umask16(0x00ff);
	map(0x0056, 0x0056).w(this, FUNC(magnum_state::beep_w));
	map(0x0080, 0x008f).rw("rtc", FUNC(cdp1879_device::read), FUNC(cdp1879_device::write)).umask16(0x00ff);
}

void magnum_state::magnum_lcdc(address_map &map)
{
	map(0x0000, 0x027f).ram();
}

MACHINE_CONFIG_START(magnum_state::magnum)
	MCFG_CPU_ADD("maincpu", I80186, XTAL(12'000'000) / 2)
	MCFG_CPU_PROGRAM_MAP(magnum_map)
	MCFG_CPU_IO_MAP(magnum_io)

	MCFG_DEVICE_ADD("rtc", CDP1879, XTAL(32'768))

	MCFG_SCREEN_ADD("screen1", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_DEVICE("lcdc1", hd61830_device, screen_update)
	MCFG_SCREEN_SIZE(6*40, 9*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*40-1, 0, 9*16-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_SCREEN_ADD("screen2", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_DEVICE("lcdc2", hd61830_device, screen_update)
	MCFG_SCREEN_SIZE(6*40, 9*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*40-1, 0, 9*16-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("lcdc1", HD61830, 1000000) // unknown clock
	MCFG_DEVICE_ADDRESS_MAP(0, magnum_lcdc)
	MCFG_VIDEO_SET_SCREEN("screen1")

	MCFG_DEVICE_ADD("lcdc2", HD61830, 1000000) // unknown clock
	MCFG_DEVICE_ADDRESS_MAP(0, magnum_lcdc)
	MCFG_VIDEO_SET_SCREEN("screen2")

	MCFG_PALETTE_ADD_MONOCHROME_INVERTED("palette")

	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("beep", BEEP, 500) /// frequency is guessed
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.50)
MACHINE_CONFIG_END

ROM_START( magnum )
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD16_BYTE("a1.7.88.bin", 0x00000, 0x4000, CRC(57882427) SHA1(97637b65ca43eb9d3bba546fb8ca701ba25ade8d))
	ROM_LOAD16_BYTE("a1.7.81.bin", 0x00001, 0x4000, CRC(949f53a8) SHA1(b339f1495d9af7dfff0c3a2c24789631f9d1265b))
	ROM_LOAD16_BYTE("a1.7.87.bin", 0x08000, 0x4000, CRC(25036dda) SHA1(20bc3782a66855b20cb0abe1051fa2eb50c7a860))
	ROM_LOAD16_BYTE("a1.7.82.bin", 0x08001, 0x4000, CRC(ecf387d8) SHA1(8b42f6ab030afb51f21f4a56c62e5acf7d074066))
	ROM_LOAD16_BYTE("a1.7.86.bin", 0x10000, 0x4000, CRC(c80b3a6b) SHA1(0f0d2cb653bbeff8f3bab6d20dc30c220a67a315))
	ROM_LOAD16_BYTE("a1.7.83.bin", 0x10001, 0x4000, CRC(51f56d78) SHA1(df717eada5e6439b1c01d91bd0ea009cd0f8ddfa))
	ROM_LOAD16_BYTE("a1.7.85.bin", 0x18000, 0x4000, CRC(f5dd5407) SHA1(af2edf7a658bcf648acb8be9f13849f838d96214))
	ROM_LOAD16_BYTE("a1.7.84.bin", 0x18001, 0x4000, CRC(b3434bb0) SHA1(8000a7aca8fc505b136a618d9eb210c50393eff1))

	ROM_REGION(0x1000, "char", 0)
	ROM_LOAD("dulmontcharrom.bin", 0x0000, 0x1000, CRC(9dff89bf) SHA1(d359aeba7f0b0c81accf3bca25e7da636c033721))
ROM_END

COMP( 1983, magnum, 0, 0, magnum, 0, magnum_state, 0, "Dulmont", "Magnum", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
