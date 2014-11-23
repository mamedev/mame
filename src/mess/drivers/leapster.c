/* 
    LeapFrog - Leapster

	educational system from 2003, software is all developed in MXFlash

	hwspecs

	
    CPU:
	  Custom ASIC (ARCTangent 5.1 CPU @ 96MHz)

    Memory: 
	  Leapster: 2MB onboard RAM, 256 bytes NVRAM.
	  Leapster2: 16MB RAM, 128kbytes NVRAM

    Media type:
	  Cartridges of 4-16MB with between 2 and 512kb NVRAM
    
	Graphics:
	  4Mb ATI chip.

    Audio:
	  Custom

    Screen:
	  160x160 CSTN with touchscreen.

    
	The Leapster 2 also has 
		USB 1.1 (client only) + full-sized SD slot.

*/

#include "emu.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


class leapster_state : public driver_device
{
public:
	leapster_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cart(*this, "cartslot")
		{ }


	virtual void machine_start();
	virtual void machine_reset();

	UINT32 screen_update_leapster(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(leapster_cart);

protected:
	required_device<generic_slot_device> m_cart;

	memory_region *m_cart_rom;
};




static INPUT_PORTS_START( leapster )
INPUT_PORTS_END



UINT32 leapster_state::screen_update_leapster(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

DEVICE_IMAGE_LOAD_MEMBER( leapster_state, leapster_cart )
{
	UINT32 size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}

void leapster_state::machine_start()
{
	astring region_tag;
	m_cart_rom = memregion(region_tag.cpy(m_cart->tag()).cat(GENERIC_ROM_REGION_TAG));
}

void leapster_state::machine_reset()
{
}



static MACHINE_CONFIG_START( leapster, leapster_state )
	/* basic machine hardware */
	// CPU is ArcTangent A5

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(160, 160)
	MCFG_SCREEN_VISIBLE_AREA(0, 160-1, 0, 160-1)
	MCFG_SCREEN_UPDATE_DRIVER(leapster_state, screen_update_leapster)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "leapster_cart")
	MCFG_GENERIC_EXTENSIONS("bin")
	MCFG_GENERIC_LOAD(leapster_state, leapster_cart)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "leapster")
MACHINE_CONFIG_END

ROM_START(leapster)
	ROM_REGION(0x200000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "155-10072-a.bin", 0x00000, 0x200000, CRC(af05e5a0) SHA1(d4468d060543ba7e44785041093bc98bcd9afa07) )
ROM_END

ROM_START(leapstertv)
	ROM_REGION(0x200000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "am29pl160cb-90sf.bin", 0x00000, 0x200000, CRC(dc281f1f) SHA1(17588de54ab3bb82801bd5062f3e6aa687412178) )
ROM_END


CONS(2003,  leapster,    0,         0,  leapster,    leapster, driver_device, 0,    "LeapFrog",   "Leapster (Germany)",    GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IS_SKELETON )
CONS(2005,  leapstertv,  leapster,  0,  leapster,    leapster, driver_device, 0,    "LeapFrog",   "Leapster TV (Germany)", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IS_SKELETON )
