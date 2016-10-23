// license:BSD-3-Clause
// copyright-holders:David Haywood

class md_boot_state : public md_base_state
{
public:
	md_boot_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag) { m_protcount = 0;}

	// bootleg specific
	int m_aladmdb_mcu_port;

	int m_protcount;

	void init_aladmdb();
	void init_mk3mdb();
	void init_ssf2mdb();
	void init_srmdb();
	void init_topshoot();
	void init_puckpkmn();
	void init_hshavoc();
	void bl_710000_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t bl_710000_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void aladmdb_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t aladmdb_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t mk3mdb_dsw_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t ssf2mdb_dsw_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t srmdb_dsw_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t topshoot_200051_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t puckpkmna_70001c_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t puckpkmna_4b2476_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void machine_start_md_bootleg() { machine_start_megadriv(); m_vdp->stop_timers(); }
	void machine_start_md_6button();
};
