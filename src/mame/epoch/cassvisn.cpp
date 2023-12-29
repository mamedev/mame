// license:BSD-3-Clause
// copyright-holders:

/*

for architecture details see
https://www.oguchi-rd.com/LSI%20products.php
the linked 'design note' contains a large amount of useful information
https://www.oguchi-rd.com/777/777%20Design%20Note.pdf

*/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/upd777/upd777.h"
#include "cpu/upd777/upd777dasm.h"

#include "softlist_dev.h"


namespace {

class cassvisn_state : public driver_device
{
public:
	cassvisn_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot")
	{ }

	void cassvisn(machine_config &config);

	void init_cass();
protected:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
private:
	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;

	u16 m_table[0xfe0/2];
};

static INPUT_PORTS_START( cassvisn )
INPUT_PORTS_END

DEVICE_IMAGE_LOAD_MEMBER(cassvisn_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("prg");
	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "prg");
	memcpy(memregion("maincpu")->base(), m_cart->get_rom_base(), size);

	upd777_disassembler::populate_addr_table(m_table);

	if (size == 0xfe0)
	{
		u8 *ROM = memregion("maincpu")->base();

		u8 m_newrom[0x1000];
		std::fill(std::begin(m_newrom), std::end(m_newrom), 0xff);

		for (int realadd = 0; realadd < 0x800; realadd++)
		{
			int found = -1;
			for (int checkaddr = 0; checkaddr < 0xfe0/2; checkaddr++)
			{
				if (m_table[checkaddr] == realadd)
					found = checkaddr;
			}

			if (found != -1)
			{
				m_newrom[(realadd * 2) + 1] = ROM[(found * 2) + 0];
				m_newrom[(realadd * 2) + 0] = ROM[(found * 2) + 1];
			}
		}

		FILE *fp;
		fp=fopen("rearranged_rom.bin", "w+b");
		if (fp)
		{
			fwrite(m_newrom, 0x1000, 1, fp);
			fclose(fp);
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}

void cassvisn_state::cassvisn(machine_config &config)
{
	UPD777(config, m_maincpu, 1000000); // frequency? UPD774 / UPD778 in some carts?

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "cassvisn_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(cassvisn_state::cart_load));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("cassvisn_cart");
}

ROM_START( cassvisn )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
ROM_END

void cassvisn_state::init_cass()
{
}

} // anonymous namespace

CONS( 1981, cassvisn, 0, 0, cassvisn,  cassvisn, cassvisn_state, init_cass, "Epoch", "Cassette Vision", MACHINE_IS_SKELETON )
