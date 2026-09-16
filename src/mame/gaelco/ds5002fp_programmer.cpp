// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

Small device for programming / reprogramming the DS5002 internal SRAM
used by Gaelco (was it designed by Gaelco, or is this a standard device provided
with the DS5002?)

Two different PCB versions.

One labeled "WR-1". This one had the PAL protected (no dump):
     _____________________________________      _____________________________________
  __|    Green LED->(_)(_)(_)<-Yellow LED |    |                                    |__
 |                    Red LED             |    |     ___________________________       |
 | ___   Xtal    ºººººººººººººººººººº     |    |    |                          |       |
 ||  |  12 MHz                            |    |    | Motorola                 |       |
 ||__|<- Button                <-EPROM socket  |    | MC68000P12               |       |
 |               ºººººººººººººººººººº     |    |    |__________________________|       |
 |      ___________                       |    |                                       |
 |__   |PAL16R6ACN|                       |    |                                     __|
    |_____________________________________|    |____________________________________|

Another one labeled "WR-2". This one has a 32MHz xtal (instead of 12 MHz), an unknown
chip with its Surface scratched out in the middle of the EPROM socket (the chips sets
behind the EPROM once on the socket) and a MC68000P12F 16MHz instead of a MC68000P12.
This one had the PAL unprotected (dumped).
     _____________________________________      _____________________________________
  __|    Green LED->(_)(_)(_)<-Yellow LED |    |                                    |__
 |                    Red LED             |    |     ___________________________       |
 | ___   Xtal    ºººººººººººººººººººº     |    |    |                          |       |
 ||  |  32 MHz    _________               |    |    | Motorola                 |       |
 ||__|<- Button  |_unknown_|    <-EPROM socket |    | MC68000P12F 16MHz        |       |
 |               ºººººººººººººººººººº     |    |    |__________________________|       |
 |      ___________                       |    |                                       |
 |__   |PAL16R6ACN|                       |    |                                     __|
    |_____________________________________|    |____________________________________|

Both are silkcreened as "68K-DS5002"

*******************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "softlist_dev.h"

namespace {

class ds5002fp_programmer_state : public driver_device
{
public:
	ds5002fp_programmer_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
	{ }

	void ds5002fp_programmer(machine_config &config) ATTR_COLD;

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load) ATTR_COLD;

	void prg_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
};

void ds5002fp_programmer_state::machine_start()
{
}

void ds5002fp_programmer_state::machine_reset()
{
}

void ds5002fp_programmer_state::prg_map(address_map &map)
{
	map(0x000000, 0x01ffff).r(m_cart, FUNC(generic_slot_device::read16_rom));
}


DEVICE_IMAGE_LOAD_MEMBER(ds5002fp_programmer_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	if ((size > 0x020000) | (size & 0x000001))
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported ROM size (must be 16 bits wide and no larger than 128K)");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_BIG);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

static INPUT_PORTS_START( ds5002fp_programmer )
INPUT_PORTS_END

void ds5002fp_programmer_state::ds5002fp_programmer(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(32'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ds5002fp_programmer_state::prg_map);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "gaelcods_rom");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(ds5002fp_programmer_state::cart_load));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("gaelco_ds5002fp_rom");
}

ROM_START( gaelcods )
	ROM_REGION( 0x200000, "pals", 0 )
	ROM_LOAD( "wr-2_pal16r6a-2.bin", 0x000, 0x104, CRC(4f027013) SHA1(8261665259a52f05e2682f2841fe788ec0b9e4ae) )
ROM_END

} // anonymous namespace

SYST( 1992, gaelcods, 0, 0, ds5002fp_programmer, ds5002fp_programmer, ds5002fp_programmer_state, empty_init, "Gaelco", "Gaelco DS5002FP Programmer", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
