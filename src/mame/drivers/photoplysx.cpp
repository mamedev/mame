// license:BSD-3-Clause
// copyright-holders:
/*
Funworld Photo Play Spirit Xtreme

CPU: Intel CELERON 2 GHz / 128 / 400 (SL6VR)
RAM: 256MB DDR 333
PCB: Intel 865G + Intel Extreme Graphics 2 + Intel FW82801FR
I/O: Cypress AN2131SC (12.000 MHz xtal) + Winbond W83627HF + Realtek RTL8101L
BIOS: 03/11/2009-I865G-6A79AD4EC-00 (Pm49FL004T-33JC)
Dongle: USB
Sound: C-Media CMI9761A
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "screen.h"

namespace {

class photoplaysx_state : public driver_device
{
public:
	photoplaysx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void photoplaysx(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void photoplaysx_map(address_map &map);
};

void photoplaysx_state::video_start()
{
}

uint32_t photoplaysx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void photoplaysx_state::photoplaysx_map(address_map &map)
{
}

static INPUT_PORTS_START( photoplaysx )
INPUT_PORTS_END


void photoplaysx_state::machine_start()
{
}

void photoplaysx_state::machine_reset()
{
}

void photoplaysx_state::photoplaysx(machine_config &config)
{
	// Basic machine hardware
	PENTIUM4(config, m_maincpu, 100000000); // Actually an Intel CELERON 2 GHz / 128 / 400 (SL6VR)
	m_maincpu->set_addrmap(AS_PROGRAM, &photoplaysx_state::photoplaysx_map);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(photoplaysx_state::screen_update));
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( photopsxsp )
	ROM_REGION(0x80000, "bios", 0)
	ROM_LOAD("photoplay_bios_pm49fl004t.bin", 0x00000, 0x80000, CRC(50bf84fe) SHA1(d0afe83b57f822d4fdb96dc1e0c6eedeccbfce7b) ) // 03/11/2009-I865G-6A79AD4EC-00

	DISK_REGION( "ide:0:hdd:image" ) // ExcelStor Callisto 40GB J840
	DISK_IMAGE( "photoply_sprt_xt_2004_sp", 0, BAD_DUMP SHA1(00f600872bf0c84b045f55cc4903d694593dfcb3) ) // May contain operator data / configuration
ROM_END

ROM_START( photopsxusp )
	ROM_REGION(0x80000, "bios", 0)
	ROM_LOAD("photoplay_bios_pm49fl004t.bin", 0x00000, 0x80000, CRC(50bf84fe) SHA1(d0afe83b57f822d4fdb96dc1e0c6eedeccbfce7b) ) // 03/11/2009-I865G-6A79AD4EC-00

	DISK_REGION( "ide:0:hdd:image" ) // Seagate ST3160815A 160GB
	DISK_IMAGE( "photoply_sprt_xt_2004_sp_u", 0, BAD_DUMP SHA1(dac230cae6efaa24362260101d39b421d5fccd45) ) // May contain operator data / configuration
ROM_END

} // Anonymous namespace


GAME( 2004,  photopsxsp,  0,          photoplaysx, photoplaysx, photoplaysx_state, empty_init, ROT0, "Funworld", "Photo Play Spirit Xtreme (Spanish)",         MACHINE_IS_SKELETON )
GAME( 2004?, photopsxusp, photopsxsp, photoplaysx, photoplaysx, photoplaysx_state, empty_init, ROT0, "Funworld", "Photo Play Spirit Xtreme (update, Spanish)", MACHINE_IS_SKELETON )
