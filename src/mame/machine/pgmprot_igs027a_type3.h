// license:BSD-3-Clause
// copyright-holders:David Haywood, Xing Xing

class pgm_arm_type3_state : public pgm_state
{
public:
	pgm_arm_type3_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag),
			m_arm_ram(*this, "arm_ram"),
			m_arm_ram2(*this, "arm_ram2"),
			m_prot(*this, "prot") {
	}
	// svg
	int           m_svg_ram_sel;
	std::unique_ptr<UINT32[]>      m_svg_shareram[2];    //for 5585G MACHINE

	UINT32        m_svg_latchdata_68k_w;
	UINT32        m_svg_latchdata_arm_w;
	required_shared_ptr<UINT32> m_arm_ram;
	required_shared_ptr<UINT32> m_arm_ram2;

	UINT32* m_armrom;

	optional_device<cpu_device> m_prot;

	DECLARE_DRIVER_INIT(theglad);
	DECLARE_DRIVER_INIT(theglada);
	DECLARE_DRIVER_INIT(svg);
	DECLARE_DRIVER_INIT(svgpcb);
	DECLARE_DRIVER_INIT(killbldp);
	DECLARE_DRIVER_INIT(dmnfrnt);
	DECLARE_DRIVER_INIT(happy6);
	DECLARE_MACHINE_START(pgm_arm_type3);
	DECLARE_WRITE32_MEMBER( svg_arm7_ram_sel_w );
	DECLARE_READ32_MEMBER( svg_arm7_shareram_r );
	DECLARE_WRITE32_MEMBER( svg_arm7_shareram_w );
	DECLARE_READ16_MEMBER( svg_m68k_ram_r );
	DECLARE_WRITE16_MEMBER( svg_m68k_ram_w );
	DECLARE_READ16_MEMBER( svg_68k_nmi_r );
	DECLARE_WRITE16_MEMBER( svg_68k_nmi_w );
	DECLARE_WRITE16_MEMBER( svg_latch_68k_w );
	DECLARE_READ16_MEMBER( svg_latch_68k_r );
	DECLARE_READ32_MEMBER( svg_latch_arm_r );
	DECLARE_WRITE32_MEMBER( svg_latch_arm_w );
	void svg_basic_init();
	void pgm_create_dummy_internal_arm_region(int size);
	void pgm_patch_external_arm_rom_jumptable_theglada(int base);
	void pgm_create_dummy_internal_arm_region_theglad(int is_svg);
	void pgm_descramble_happy6(UINT8* src);
	void pgm_descramble_happy6_2(UINT8* src);
	void svg_latch_init();
	DECLARE_READ32_MEMBER( dmnfrnt_speedup_r );
	DECLARE_READ16_MEMBER( dmnfrnt_main_speedup_r );
	DECLARE_READ32_MEMBER( killbldp_speedup_r );
	DECLARE_READ32_MEMBER( theglad_speedup_r );
	DECLARE_READ32_MEMBER( happy6_speedup_r );
	DECLARE_READ32_MEMBER( svg_speedup_r );
	DECLARE_READ32_MEMBER( svgpcb_speedup_r );
	DECLARE_MACHINE_RESET(pgm_arm_type3_reset);
};

MACHINE_CONFIG_EXTERN( pgm_arm_type3 );
INPUT_PORTS_EXTERN(theglad);
INPUT_PORTS_EXTERN(happy6);
INPUT_PORTS_EXTERN(svg);
INPUT_PORTS_EXTERN(svgtw);
