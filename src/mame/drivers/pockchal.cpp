// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************************************************

Similar to https://www.youtube.com/watch?v=FmyR-kL-QWo

base unit contains

1x Toshiba TMP90C845AF

1x SANYO LC21003 BLA5

3x SEC C941A KS0108B

1x Toshiba T9842B

(system has no bios ROM)

Cart sizes: 1MB, 2MB, 4MB

********************************************************************/

#include "emu.h"
#include "cpu/tlcs90/tlcs90.h"
#include "softlist.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


class pockchalv1_state : public driver_device
{
public:
	pockchalv1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
		{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_pockchalv1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	UINT32  m_rom_size;
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(pockchalv1_cart);
};


DEVICE_IMAGE_LOAD_MEMBER( pockchalv1_state, pockchalv1_cart )
{
	m_rom_size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(m_rom_size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), m_rom_size, "rom");

	return IMAGE_INIT_PASS;
}
void pockchalv1_state::video_start()
{
}

UINT32 pockchalv1_state::screen_update_pockchalv1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static ADDRESS_MAP_START( pockchalv1_map, AS_PROGRAM, 8, pockchalv1_state )
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( pockchalv1 )
INPUT_PORTS_END



void pockchalv1_state::machine_start()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	if (m_cart->exists())
		space.install_read_handler(0x0000, 0x7fff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_cart));
}

void pockchalv1_state::machine_reset()
{
}


static MACHINE_CONFIG_START( pockchalv1, pockchalv1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMP90845,8000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(pockchalv1_map)
//  MCFG_CPU_VBLANK_INT_DRIVER("screen", pockchalv1_state,  irq0_line_hold)

	// wrong, it's a b&w / greyscale thing
	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(pockchalv1_state, screen_update_pockchalv1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "pockchalw_cart")
	MCFG_GENERIC_EXTENSIONS("bin")
	MCFG_GENERIC_LOAD(pockchalv1_state, pockchalv1_cart)
	MCFG_GENERIC_MANDATORY

	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("pc1_list","pockchalw")

MACHINE_CONFIG_END



ROM_START( pockchal )
ROM_END

/*     YEAR  NAME     PARENT  COMPAT  MACHINE     INPUT       CLASS          INIT        COMPANY               FULLNAME*/
CONS( 199?, pockchal,  0,     0,      pockchalv1, pockchalv1, driver_device,  0, "Benesse Corporation", "Pocket Challenge W (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
