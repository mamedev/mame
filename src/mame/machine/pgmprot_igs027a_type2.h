// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi, Xing Xing

class pgm_arm_type2_state : public pgm_state
{
public:
	pgm_arm_type2_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag),
			m_arm_ram(*this, "arm_ram"),
			m_arm7_shareram(*this, "arm7_shareram"),
			m_prot(*this, "prot") {
	}

	void init_kov2();
	void init_kov2p();
	void init_martmast();
	void init_ddp2();
	void init_dw2001();
	void init_dwpc();

	void pgm_arm_type2(machine_config &config);
private:
	// kov2
	u32        m_latchdata_68k_w;
	u32        m_latchdata_arm_w;

	required_shared_ptr<u32> m_arm_ram;
	required_shared_ptr<u32> m_arm7_shareram;

	optional_device<cpu_device> m_prot;

	virtual void machine_start() override;
	DECLARE_READ32_MEMBER(latch_arm_r);
	DECLARE_WRITE32_MEMBER(latch_arm_w);
	DECLARE_READ32_MEMBER(shareram_r);
	DECLARE_WRITE32_MEMBER(shareram_w);
	DECLARE_READ16_MEMBER(latch_68k_r);
	DECLARE_WRITE16_MEMBER(latch_68k_w);
	DECLARE_READ16_MEMBER(ram_r);
	DECLARE_WRITE16_MEMBER(ram_w);
	DECLARE_WRITE32_MEMBER(martmast_arm_region_w);
	DECLARE_WRITE32_MEMBER(kov2_arm_region_w);
	DECLARE_WRITE32_MEMBER(kov2p_arm_region_w);
	DECLARE_READ32_MEMBER(ddp2_speedup_r);
	DECLARE_READ16_MEMBER(ddp2_main_speedup_r);
	void _55857F_arm7_map(address_map &map);
	void kov2_mem(address_map &map);
};

/* simulations (or missing) */
INPUT_PORTS_EXTERN( ddp2 );
INPUT_PORTS_EXTERN( kov2 );
INPUT_PORTS_EXTERN( martmast );
INPUT_PORTS_EXTERN( dw2001 );
