// license:BSD-3-Clause
// copyright-holders:David Haywood

// a generic looking s3c2410 SoC setup, with very little of note in the system, and a NAND ROM in the cartriges
// if you copy the NAND plumbing from ghosteo.cpp (and add the ID command) the software begins to boot, but
// the NAND should be included in the cartridge bus device, not the driver.

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/arm7/arm7.h"
#include "machine/s3c2410.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class digiblast_state : public driver_device
{
public:
	digiblast_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_s3c2410(*this, "s3c2410")
		, m_system_memory(*this, "systememory")
		, m_cart(*this, "cartslot")
	{
	}

	void digiblast(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<s3c2410_device> m_s3c2410;
	required_shared_ptr<uint32_t> m_system_memory;

	required_device<generic_slot_device> m_cart;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void digiblast_map(address_map &map) ATTR_COLD;
};

void digiblast_state::digiblast_map(address_map &map)
{
	map(0x30000000, 0x31ffffff).ram().share("systememory").mirror(0x02000000);
}

static INPUT_PORTS_START( digiblast )
INPUT_PORTS_END

void digiblast_state::machine_start()
{
}

void digiblast_state::machine_reset()
{
}

DEVICE_IMAGE_LOAD_MEMBER(digiblast_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}


void digiblast_state::digiblast(machine_config &config)
{
	/* basic machine hardware */
	ARM9(config, m_maincpu, 200000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &digiblast_state::digiblast_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(455, 262);
	screen.set_visarea(0, 320-1, 0, 256-1);
	screen.set_screen_update("s3c2410", FUNC(s3c2410_device::screen_update));

	PALETTE(config, "palette").set_entries(256);

	S3C2410(config, m_s3c2410, 12000000);
	m_s3c2410->set_palette_tag("palette");
	m_s3c2410->set_screen_tag("screen");
	//m_s3c2410->core_pin_r_callback().set(FUNC(digiblast_state::s3c2410_core_pin_r));
	//m_s3c2410->gpio_port_r_callback().set(FUNC(digiblast_state::s3c2410_gpio_port_r));
	//m_s3c2410->gpio_port_w_callback().set(FUNC(digiblast_state::s3c2410_gpio_port_w));
	//m_s3c2410->i2c_scl_w_callback().set(FUNC(digiblast_state::s3c2410_i2c_scl_w));
	//m_s3c2410->i2c_sda_r_callback().set(FUNC(digiblast_state::s3c2410_i2c_sda_r));
	//m_s3c2410->i2c_sda_w_callback().set(FUNC(digiblast_state::s3c2410_i2c_sda_w));
	//m_s3c2410->nand_command_w_callback().set(FUNC(digiblast_state::s3c2410_nand_command_w));
	//m_s3c2410->nand_address_w_callback().set(FUNC(digiblast_state::s3c2410_nand_address_w));
	//m_s3c2410->nand_data_r_callback().set(FUNC(digiblast_state::s3c2410_nand_data_r));
	//m_s3c2410->nand_data_w_callback().set(FUNC(digiblast_state::s3c2410_nand_data_w));

	/* sound hardware */
	SPEAKER(config, "front").front_center();

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "digiblast_cart");
	m_cart->set_width(GENERIC_ROM8_WIDTH);
	m_cart->set_device_load(FUNC(digiblast_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("digiblast_cart");

}

ROM_START( digiblst )
	//ROM_REGION( 0x4200000, "flash", ROMREGION_ERASEFF )
ROM_END

} // Anonymous namespace

CONS( 2005, digiblst,  0, 0, digiblast,  digiblast,  digiblast_state, empty_init, "Nikko Entertainment B.V. / Grey Innovation", "digiBLAST", MACHINE_IS_SKELETON )
