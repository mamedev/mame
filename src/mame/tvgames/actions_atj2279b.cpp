// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    System is fairly barebones
    Main PCB has the SoC, RAM, ROM
    connects to other PCBs with various types of slots (USB, SD etc.)
    (ADD FULL DETAILS)

    Specifications (incomplete and unconfirmed):
    CPU: 450 MHz ATJ227X with intergrated GPU
    SDRAM: 256MB
    (source: http://wecmuseum.org/index.php?title=Retro-Bit_Generations)

    TODO: everything - emulate the SoC

    Presumably has an internal bootstrap (at least) to boot from the NAND

    --

    reviews for this device were less than favourable with many comments
    about how the SNES and Arcade games often did not run at full speed
    and how the overall quality of emulation was unremarkable

*******************************************************************************/

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "speaker.h"
#include "screen.h"


namespace {

class actions_atj2279b_state : public driver_device
{
public:
	actions_atj2279b_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "arm7")
	{ }

	void actions_atj2279b(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);


	void atj2279b_map(address_map &map) ATTR_COLD;

	required_device<arm7_cpu_device> m_maincpu;
};

void actions_atj2279b_state::atj2279b_map(address_map &map)
{
}

void actions_atj2279b_state::machine_start()
{
}

void actions_atj2279b_state::machine_reset()
{
}

static INPUT_PORTS_START( actions_atj2279b )
INPUT_PORTS_END


uint32_t actions_atj2279b_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void actions_atj2279b_state::actions_atj2279b(machine_config &config)
{
	ARM7_BE(config, m_maincpu, 450'000'000); // Probably ATJ227X 450MHz, but this needs to be checked more closely
	m_maincpu->set_addrmap(AS_PROGRAM, &actions_atj2279b_state::atj2279b_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(1280, 720);
	screen.set_visarea(0, 1280-1, 0, 720-1); // resolution unconfirmed (possibly 1080p as well, but this is unlikely)
	screen.set_screen_update(FUNC(actions_atj2279b_state::screen_update));

	SPEAKER(config, "speaker", 2).front();
}

ROM_START( rbitgen )
	ROM_REGION32_BE( 0x21000000, "nand", 0 ) // NAND dump
	//TOSHIBA
	// TC58NVG2S3?A00  (there is a yellow stripe on the label before A00 obscuring the text)
	ROM_LOAD16_WORD_SWAP( "nand.bin", 0x000000, 0x21000000, CRC(92576add) SHA1(1fe61ef1d2dd24e5b5d48c477846ef0c83ec6568) )
ROM_END

} // anonymous namespace


//    year, name,         parent,  compat, machine,      input,        class,              init,       company,  fullname,                             flags
CONS( 2016, rbitgen,      0,       0,      actions_atj2279b, actions_atj2279b, actions_atj2279b_state, empty_init, "Retro-Bit", "Generations (Retro-Bit)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
