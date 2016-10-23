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
	// kov2
	uint32_t        m_kov2_latchdata_68k_w;
	uint32_t        m_kov2_latchdata_arm_w;

	required_shared_ptr<uint32_t> m_arm_ram;
	required_shared_ptr<uint32_t> m_arm7_shareram;

	optional_device<cpu_device> m_prot;

	void init_kov2();
	void init_kov2p();
	void init_martmast();
	void init_ddp2();
	void init_dw2001();
	void init_dwpc();
	void machine_start_pgm_arm_type2();
	uint32_t arm7_latch_arm_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void arm7_latch_arm_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t arm7_shareram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void arm7_shareram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t arm7_latch_68k_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void arm7_latch_68k_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t arm7_ram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void arm7_ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void kov2_latch_init();
	void martmast_arm_region_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void kov2_arm_region_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void kov2p_arm_region_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t ddp2_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint16_t ddp2_main_speedup_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
};

/* emulations */
MACHINE_CONFIG_EXTERN( pgm_arm_type2 );

/* simulations (or missing) */
INPUT_PORTS_EXTERN( ddp2 );
INPUT_PORTS_EXTERN( kov2 );
INPUT_PORTS_EXTERN( martmast );
INPUT_PORTS_EXTERN( dw2001 );
