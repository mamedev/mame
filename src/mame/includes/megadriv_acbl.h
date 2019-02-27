// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_INCLUDES_MEGADRIV_ACBL_H
#define MAME_INCLUDES_MEGADRIV_ACBL_H

class md_boot_state : public md_base_state
{
public:
	md_boot_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag) { m_protcount = 0;}

	void megadrvb(machine_config &config);
	void megadrvb_6b(machine_config &config);
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
	void init_hshavoc();
	void init_barek3();
	void init_sonic2mb();
	void init_twinktmb();

private:
	DECLARE_WRITE16_MEMBER(bl_710000_w);
	DECLARE_READ16_MEMBER(bl_710000_r);
	DECLARE_WRITE16_MEMBER(aladmdb_w);
	DECLARE_READ16_MEMBER(aladmdb_r);
	DECLARE_READ16_MEMBER(twinktmb_r);
	DECLARE_READ16_MEMBER(mk3mdb_dsw_r);
	DECLARE_READ16_MEMBER(ssf2mdb_dsw_r);
	DECLARE_READ16_MEMBER(srmdb_dsw_r);
	DECLARE_READ16_MEMBER(topshoot_200051_r);
	DECLARE_READ16_MEMBER(puckpkmna_70001c_r);
	DECLARE_READ16_MEMBER(puckpkmna_4b2476_r);

	DECLARE_MACHINE_START(md_bootleg) { MACHINE_START_CALL_MEMBER(megadriv); m_vdp->stop_timers(); }
	DECLARE_MACHINE_START(md_6button);

	void jzth_map(address_map &map);
	void md_bootleg_map(address_map &map);
	void puckpkmn_map(address_map &map);
	void puckpkmna_map(address_map &map);

	// bootleg specific
	int m_aladmdb_mcu_port;

	int m_protcount;
};

#endif // MAME_INCLUDES_MEGADRIV_ACBL_H
