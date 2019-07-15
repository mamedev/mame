// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_INCLUDES_MEGADRIV_RAD_H
#define MAME_INCLUDES_MEGADRIV_RAD_H

class megadriv_radica_state : public md_base_state
{
public:
	megadriv_radica_state(const machine_config &mconfig, device_type type, const char *tag)
		: md_base_state(mconfig, type, tag),
		m_bank(0),
		m_rom(*this, "maincpu")
	{}

	void init_megadriv_radica_6button_pal();

	DECLARE_MACHINE_START(megadriv_radica_6button);
	DECLARE_MACHINE_START(megadriv_radica_3button);
	DECLARE_MACHINE_RESET(megadriv_radica);

	DECLARE_READ16_MEMBER(read);
	DECLARE_READ16_MEMBER(read_a13);

	void megadriv_radica_6button_pal(machine_config &config);
	void megadriv_radica_3button_ntsc(machine_config &config);
	void megadriv_radica_3button_pal(machine_config &config);

	void megadriv_radica_map(address_map &map);
private:
	int m_bank;
	required_region_ptr<uint16_t> m_rom;
};

#endif // MAME_INCLUDES_MEGADRIV_RAD_H
