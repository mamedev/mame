// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_INCLUDES_MEGADRIV_ACBL_H
#define MAME_INCLUDES_MEGADRIV_ACBL_H

class md_boot_state : public md_base_state
{
public:
	md_boot_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag) { m_protcount = 0; }

	void megadrvb(machine_config &config);
	void md_bootleg(machine_config &config);
	void puckpkmn(machine_config &config);
	void jzth(machine_config &config);
	void puckpkmna(machine_config &config);

	void init_aladmdb();
	void init_mk3mdb();
	void init_ssf2mdb();
	void init_srmdb();
	void init_topshoot();
	void init_puckpkmn();
	void init_barek2();
	void init_barek2ch();
	void init_barek3();
	void init_barekch();
	void init_bk3ssrmb();
	void init_sonic2mb();
	void init_twinktmb();
	void init_jparkmb();
	void init_sbubsm();

private:
	void bl_710000_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t bl_710000_r();
	void aladmdb_w(uint16_t data);
	uint16_t aladmdb_r();
	uint16_t barek2mb_r();
	uint16_t jparkmb_r();
	uint16_t twinktmb_r();
	uint16_t dsw_r(offs_t offset);
	uint16_t topshoot_200051_r();
	uint16_t sbubsm_400000_r();
	uint16_t sbubsm_400002_r();
	uint16_t puckpkmna_70001c_r();
	uint16_t puckpkmna_4b2476_r();

	void jzth_map(address_map &map);
	void md_bootleg_map(address_map &map);
	void puckpkmn_map(address_map &map);
	void puckpkmna_map(address_map &map);

	// bootleg specific
	int m_aladmdb_mcu_port = 0;

	int m_protcount;
};

class md_sonic3bl_state : public md_boot_state
{
public:
	md_sonic3bl_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_boot_state(mconfig, type, tag)
	, m_in_coin(*this, "COIN")
	, m_in_mcu(*this, "MCU")
	{ }

	void init_sonic3mb();

private:
	required_ioport m_in_coin;
	required_ioport m_in_mcu;

	void prot_w(u8 data);
	uint16_t prot_r();

	u8 m_prot_cmd = 0;
};

class md_boot_6button_state : public md_boot_state
{
public:
	md_boot_6button_state(const machine_config& mconfig, device_type type, const char* tag)
		: md_boot_state(mconfig, type, tag)
	{
	}

	void megadrvb_6b(machine_config &config);

protected:
	virtual void machine_start() override;

};

#endif // MAME_INCLUDES_MEGADRIV_ACBL_H
