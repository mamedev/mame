// license:BSD-3-Clause
// copyright-holders:

/*
Gamebox, 2010
Hardware info by Guru
---------------------

This is a Chinese multi-game emulation box from around 2010 probably running MAME.
At bootup it shows a menu where games can be selected.
This is the first 'Gamebox' version released.
It contains a bunch of CPS1 games (IIRC), one bad quality video and some other random games (maybe Gameboy?)
Main components are....

- PCB: GAMEBOX 20100720_V1.03
- Ingenic JZ4755 SOC (many documents available)
- 1Gbit (128MB x8-bit) SDRAM (4x Samsung 256Mbit K4S560832E-TC75)
- 2GByte NAND ROM (Hynix H27UAG8T2ATR)
- 4x USB connector for controllers, PS1 style (these are said to be custom, apparently normal USB controllers won't work)
- MicroSD slot for expansion (file type / game type unknown)
- 32 pin single-sided cart slot for expansion, labelled 'GAME CARD'. Actual cart type might be Gameboy which also has 32 pins.
- Mini USB connector, connected to SOC pin 67 (USB DATA-) and pin 66 (USB DATA+)
- RCA Jacks for Composite Video and Left/Right Audio
- 3 Crystals (to be measured later when unit re-assembled)
- Small SOIC8 chip with surface scratched. Pin 5 connected to SOC pin 39 (I2C_SDA) then 2.2k pullup resistor, pin 6 connected
  to SOC pin 38 (I2C_SCK) then 2.2k pulldown resistor. It's probably an I2C EEPROM.

**********************************************************
*/


#include "emu.h"

#include "cpu/mips/mips3.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class gamebox_state : public driver_device
{
public:
	gamebox_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void gamebox(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t gamebox_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void gamebox_state::video_start()
{
}


void gamebox_state::program_map(address_map &map)
{
}


static INPUT_PORTS_START( gamebox )
INPUT_PORTS_END


static GFXDECODE_START( gfx_gamebox )
	// TODO
GFXDECODE_END


void gamebox_state::gamebox(machine_config &config)
{
	R4600BE(config, m_maincpu, 24_MHz_XTAL); // wrong, Ingenic JZ4755 (MIPS32 based, no CPU core available)
	m_maincpu->set_addrmap(AS_PROGRAM, &gamebox_state::program_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(gamebox_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_gamebox);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();
}


ROM_START( gamebox )
	ROM_REGION( 0x8e000000, "maincpu", 0 )
	ROM_LOAD( "h27uag8t2atr.u1", 0x00000000, 0x8e000000, CRC(bcab3fe4) SHA1(6a80099a64822e2495463e2efd70a60f8878a918) )
ROM_END

} // anonymous namespace


GAME( 2010, gamebox, 0, gamebox, gamebox, gamebox_state, empty_init, ROT0, "<unknown>", "Gamebox", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
