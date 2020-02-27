// license:BSD-3-Clause
// copyright-holders:Sterophonick
/**********************************************************************

	Linus Akesson Bit Banger Skeleton Driver
	
	The Atmel ATtiny15 MCU has 32 bytes of RAM, but it is actually
	the CPU registers. They are mapped as RAM until I or someone
	implements an ATtiny15 device.

**********************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "screen.h"

class bitbangr_state : public driver_device
{
public:
	bitbangr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void bitbangr(machine_config &config);
	
private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;

protected:
	void prg_map(address_map &map);
	void data_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

void bitbangr_state::prg_map(address_map &map)
{
	map(0x0000, 0x03FF).rom().region("maincpu", 0);
	map(0x0400, 0xFFFF).ram();
}

void bitbangr_state::machine_start()
{
}

void bitbangr_state::machine_reset()
{
}

uint32_t bitbangr_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( bitbangr )
INPUT_PORTS_END

void bitbangr_state::bitbangr(machine_config &config)
{
	M6502(config, m_maincpu, 1600000); //WRONG - Is ATtiny15
	m_maincpu->set_addrmap(AS_PROGRAM, &bitbangr_state::prg_map);
	
	//Screen hardware, uses standard VGA.
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(bitbangr_state::screen_update));
	
	//Sound hardware
	//SPEAKER(config, "mono").front_center();
}

ROM_START( bitbangr )
	ROM_REGION( 0x0400, "maincpu", 0 ) /* Main program store */
	ROM_LOAD( "bitbanger.bin", 0x0000, 0x0400, CRC(5ecf9cdf) SHA1(486b9c6bdae06afeef415421aa7e06c92bb7fdcd) )
ROM_END

SYST( 2011, bitbangr,   0,      0,      bitbangr,   bitbangr,  bitbangr_state,  empty_init, "Linus Akesson", "Bit Banger", MACHINE_IS_SKELETON )