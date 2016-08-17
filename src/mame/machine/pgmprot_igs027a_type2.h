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
	UINT32        m_kov2_latchdata_68k_w;
	UINT32        m_kov2_latchdata_arm_w;

	required_shared_ptr<UINT32> m_arm_ram;
	required_shared_ptr<UINT32> m_arm7_shareram;

	optional_device<cpu_device> m_prot;

	DECLARE_DRIVER_INIT(kov2);
	DECLARE_DRIVER_INIT(kov2p);
	DECLARE_DRIVER_INIT(martmast);
	DECLARE_DRIVER_INIT(ddp2);
	DECLARE_DRIVER_INIT(dw2001);
	DECLARE_DRIVER_INIT(dwpc);
	DECLARE_MACHINE_START(pgm_arm_type2);
	DECLARE_READ32_MEMBER( arm7_latch_arm_r );
	DECLARE_WRITE32_MEMBER( arm7_latch_arm_w );
	DECLARE_READ32_MEMBER( arm7_shareram_r );
	DECLARE_WRITE32_MEMBER( arm7_shareram_w );
	DECLARE_READ16_MEMBER( arm7_latch_68k_r );
	DECLARE_WRITE16_MEMBER( arm7_latch_68k_w );
	DECLARE_READ16_MEMBER( arm7_ram_r );
	DECLARE_WRITE16_MEMBER( arm7_ram_w );
	void kov2_latch_init();
	DECLARE_WRITE32_MEMBER( martmast_arm_region_w );
	DECLARE_WRITE32_MEMBER( kov2_arm_region_w );
	DECLARE_WRITE32_MEMBER( kov2p_arm_region_w );
	DECLARE_READ32_MEMBER( ddp2_speedup_r );
	DECLARE_READ16_MEMBER( ddp2_main_speedup_r );
};

/* emulations */
MACHINE_CONFIG_EXTERN( pgm_arm_type2 );

/* simulations (or missing) */
INPUT_PORTS_EXTERN( ddp2 );
INPUT_PORTS_EXTERN( kov2 );
INPUT_PORTS_EXTERN( martmast );
INPUT_PORTS_EXTERN( dw2001 );
