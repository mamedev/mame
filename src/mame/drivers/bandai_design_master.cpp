// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************************************************************

 __________________________________IR___
 |                          ____  RX/TX |
 |   _____      ________    |   |    __ |       
 |   |    |     |       |   |   |    | ||
 |   |IC3 |     | IC2   |   |   |CN2->_||
 |   |    |     |       |   |   |       |
 |   |____|     |_______|   |   |       |
 |   ____                   |   |       |
 |   |   |       CART SLOT->|   |       |
 |   |IC5|                  |   |       |
 |   |___|      ________    |   |       |
 |   ____       |       |   |   |       |
 |   |   |      | IC1   |   |   |       |
 |   |IC4|      |       |   |   |       |
 |   |___|      |_______|   |___|       |
 |
 | SW2
 |______________________________________
 
 IC1 = Hitachi H8/328 (24K-byte internal ROM + 1k-byte RAM)
 IC2 = Hitachi HG62G010R21FBN Gate Array (low gatecount and low I/O-count packages)
 IC3 = Hitachi HM62256LFP-10T 256kbit CMOS SRAM
 IC4 = BA10324AF Ground Sense Operational Amplifier
 IC5 = Hitachi 74HC00 (5B2T HC00)


 TODO: cartridge pinouts / information


 NOTE: cartridge dumps contain a boot vector, is internal ROM even used? (maybe when no cartridge is inserted?)


******************************************************************************************************************************/

#include "emu.h"
#include "cpu/h8/h83002.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

//#include "sound/multipcm.h"
//#include "screen.h"
//#include "speaker.h"

class bdsm_state : public driver_device
{
public:
	bdsm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void bdesignm(machine_config &config);

private:
	void io_map(address_map &map);
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

void bdsm_state::mem_map(address_map &map)
{
	map(0x00000, 0x07fff).rom().region("roms", 0);
}

void bdsm_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START( bdesignm )
INPUT_PORTS_END

void bdsm_state::bdesignm(machine_config &config)
{
	/* basic machine hardware */
	H83002(config, m_maincpu, XTAL(20'000'000)); /* H8/328 (24kbytes internal ROM, 1kbyte internal ROM) */
	m_maincpu->set_addrmap(AS_PROGRAM, &bdsm_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &bdsm_state::io_map);

	GENERIC_CARTSLOT(config, "cartslot", generic_linear_slot, "bdesignm_cart");

	SOFTWARE_LIST(config, "cart_list_game").set_original("bdesignm_game_cart");
	SOFTWARE_LIST(config, "cart_list_media").set_original("bdesignm_media_cart");
}

ROM_START( bdesignm )
	ROM_REGION16_BE(0x88000, "roms", 0)
	ROM_LOAD( "h8_328.bin", 0x00000, 0x6000, NO_DUMP ) // internal rom
ROM_END


CONS( 200?, bdesignm,  0,      0,      bdesignm,   bdesignm, bdsm_state, empty_init, "Bandai", "Design Master",   MACHINE_IS_SKELETON )
