// license:BSD-3-Clause
// copyright-holders:David Shah

/*

Monkey King SoCs (currently only 3B is supported)

Presumably-custom ARM-based system-on-chips by Digital Media Cartridge (DMC).
Intended to run NES and Genesis emulators, primarily for ATgames systems.

Sometimes abbreviated MK. It is a successor of the Titan SoC used in previous
emulation based ATgames systems.

Monkey King and Monkey 2: Presumed custom. Used in some ATgames/Blaze
Genesis systems and the Atari Flashback Portable.

Monkey King 3 and Monkey King 3B: Presumed custom. Used in the ATgames
BLAST system and the RS-70 648-in-1 "PS1 form factor" clone. Supports
HDMI output.

Monkey King 3.6: not a custom part but a rebranded RK3036, usually
running a cut-down Android based OS. Used in newer ATgames systems.

The typical configuration of the Monkey King SoCs (other than the
3.6) is with 8/16MB of SDRAM, NOR flash for the firmware and
built-in games, and a SD card for additional games.

The RS-70 is notable for having a debug UART on the USB port 
(serial TX on D+, 115200). It prints the following messages on boot:

	EXEC: Executing 'boot' with 0 args (ZLib ON)...
	EXEC: Loading 'boot' at 0x18000000...
	EXEC: Loaded 372272 bytes of 2097152 available.	

There are other strings in the ROM that imply there may be more serial
debug possibilities.

TODO:
	implement everything
	add dumps of more Monkey King systems
*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "emupal.h"
#include "screen.h"

class mk3b_soc_state : public driver_device
{
public:
	mk3b_soc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_iram3(*this, "iram3"),
		m_iram7(*this, "iram7"),
		m_sdram(*this, "sdram"),
		m_maincpu(*this, "maincpu")
	{ }

	void mk3b_soc(machine_config &config);

	void init_rs70();

private:
	required_shared_ptr<uint32_t> m_iram3, m_iram7;
	required_shared_ptr<uint32_t> m_sdram;
	required_device<cpu_device> m_maincpu;

	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_mk3b_soc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void map(address_map &map);
};


void mk3b_soc_state::map(address_map &map)
{
	// 64MB external NOR flash
	// We assume that the lower 32MB is mirrored to 0x00000000
	// solely so the vectors end up in the right place. This may
	// well not be correct.
	map(0x08000000, 0x0BFFFFFF).rom().share("norflash");
	map(0x00000000, 0x01FFFFFF).rom().share("norflash");
	// unknown amount and configuration of internal RAM
	map(0x03000000, 0x0300FFFF).ram().share("iram3");
	map(0x07000000, 0x0700FFFF).ram().share("iram7");
	// 16MB of external SDRAM
	map(0x18000000, 0x18FFFFFF).ram().share("sdram");
}

static INPUT_PORTS_START( mk3b_soc )

INPUT_PORTS_END

void mk3b_soc_state::video_start()
{
}

void mk3b_soc_state::machine_reset()
{
}

uint32_t mk3b_soc_state::screen_update_mk3b_soc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void mk3b_soc_state::mk3b_soc(machine_config &config)
{
	/* basic machine hardware */
	ARM920T(config, m_maincpu, 200000000); // type + clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &mk3b_soc_state::map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(1280, 720);
	screen.set_visarea(0, 1280-1, 0, 720-1);
	screen.set_screen_update(FUNC(mk3b_soc_state::screen_update_mk3b_soc));

}

void mk3b_soc_state::init_rs70()
{
	// Uppermost address bit seems to be inverted
	uint8_t *ROM = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	for (int i = 0; i < (size / 2); i++)
	{
		std::swap(ROM[i], ROM[i + (size / 2)]);
	}
}


ROM_START( rs70_748in1 )
	ROM_REGION(0x04000000, "maincpu", 0)
	ROM_LOAD("s29gl512p.bin", 0x000000, 0x04000000, CRC(cb452bd7) SHA1(0b19a13a3d0b829725c10d64d7ff852ff5202ed0) )
ROM_END

CONS( 2019, rs70_748in1,  0,        0, mk3b_soc, mk3b_soc, mk3b_soc_state, init_rs70, "<unknown>", "RS-70 748-in-1",      MACHINE_IS_SKELETON )
