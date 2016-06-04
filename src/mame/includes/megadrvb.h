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

	DECLARE_DRIVER_INIT(aladmdb);
	DECLARE_DRIVER_INIT(mk3mdb);
	DECLARE_DRIVER_INIT(ssf2mdb);
	DECLARE_DRIVER_INIT(srmdb);
	DECLARE_DRIVER_INIT(topshoot);
	DECLARE_DRIVER_INIT(puckpkmn);
	DECLARE_DRIVER_INIT(hshavoc);
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

	DECLARE_MACHINE_START(md_bootleg) { MACHINE_START_CALL_MEMBER(megadriv); m_vdp->stop_timers(); }
	DECLARE_MACHINE_START(md_6button);
};


