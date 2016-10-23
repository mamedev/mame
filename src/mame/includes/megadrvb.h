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
	DECLARE_WRITE16_MEMBER(bl_710000_w);
	DECLARE_READ16_MEMBER(bl_710000_r);
	DECLARE_WRITE16_MEMBER(aladmdb_w);
	DECLARE_READ16_MEMBER(aladmdb_r);
	DECLARE_READ16_MEMBER(mk3mdb_dsw_r);
	DECLARE_READ16_MEMBER(ssf2mdb_dsw_r);
	DECLARE_READ16_MEMBER(srmdb_dsw_r);
	DECLARE_READ16_MEMBER(topshoot_200051_r);
	DECLARE_READ16_MEMBER(puckpkmna_70001c_r);
	DECLARE_READ16_MEMBER(puckpkmna_4b2476_r);

	void machine_start_md_bootleg() { machine_start_megadriv(); m_vdp->stop_timers(); }
	void machine_start_md_6button();
};
