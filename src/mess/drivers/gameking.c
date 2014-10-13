/* TimeTop - GameKing */

// these are meant to have a 3-in-1 internal ROM, not dumped

#include "emu.h"
#include "cpu/m6502/m65c02.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


class gameking_state : public driver_device
{
public:
	gameking_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_palette(*this, "palette")
		{ }


	DECLARE_DRIVER_INIT(gameking);
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_PALETTE_INIT(gameking);

	UINT32 screen_update_gameking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(gameking_cart);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_device<palette_device> m_palette;

	memory_region *m_cart_rom;
	memory_bank *m_mainbank;
};

static ADDRESS_MAP_START( gameking_mem , AS_PROGRAM, 8, gameking_state )
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("mainbank")
ADDRESS_MAP_END


static INPUT_PORTS_START( gameking )
INPUT_PORTS_END

static const unsigned char gameking_palette[] =
{
	0, 0, 0,
	63, 63, 63,
	127, 127, 127,
	255, 255, 255
};

PALETTE_INIT_MEMBER(gameking_state, gameking)
{
	for (int i = 0; i < sizeof(gameking_palette) / 3; i++)
		palette.set_pen_color(i, gameking_palette[i*3], gameking_palette[i*3+1], gameking_palette[i*3+2]);
}


UINT32 gameking_state::screen_update_gameking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


DRIVER_INIT_MEMBER(gameking_state, gameking)
{
}


DEVICE_IMAGE_LOAD_MEMBER( gameking_state, gameking_cart )
{
	UINT32 size = m_cart->common_get_size("rom");
	
	if (size > 0x20000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return IMAGE_INIT_FAIL;
	}
	
	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");			
	
	return IMAGE_INIT_PASS;
}

void gameking_state::machine_start()
{
	astring region_tag;
	m_cart_rom = memregion(region_tag.cpy(m_cart->tag()).cat(GENERIC_ROM_REGION_TAG));

	if (!m_cart_rom) printf("No Rom\n");

	m_mainbank = membank("mainbank");
	m_mainbank->set_base(m_cart_rom->base());
}

void gameking_state::machine_reset()
{
}



static MACHINE_CONFIG_START( gameking, gameking_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M65C02, 6000000)
	MCFG_CPU_PROGRAM_MAP(gameking_mem)
	//MCFG_CPU_VBLANK_INT_DRIVER("screen", gameking_state,  gameking_frame_int)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(48, 32)
	MCFG_SCREEN_VISIBLE_AREA(0, 48-1, 0, 32-1)
	MCFG_SCREEN_UPDATE_DRIVER(gameking_state, screen_update_gameking)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", ARRAY_LENGTH(gameking_palette) * 3)
	MCFG_PALETTE_INIT_OWNER(gameking_state, gameking )



	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "gameking_cart")
	MCFG_GENERIC_EXTENSIONS("bin")
	MCFG_GENERIC_MANDATORY
	MCFG_GENERIC_LOAD(gameking_state, gameking_cart)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "gameking")
MACHINE_CONFIG_END

ROM_START(gameking)
ROM_END



CONS(2003,  gameking,    0,  0,  gameking,    gameking, gameking_state, gameking,    "TimeTop",   "GameKing GM-218", GAME_NOT_WORKING | GAME_NO_SOUND )
// the GameKing 2 (GM-219) is probably identical HW
