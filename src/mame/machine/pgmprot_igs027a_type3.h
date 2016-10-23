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
	std::unique_ptr<uint32_t[]>      m_svg_shareram[2];    //for 5585G MACHINE

	uint32_t        m_svg_latchdata_68k_w;
	uint32_t        m_svg_latchdata_arm_w;
	required_shared_ptr<uint32_t> m_arm_ram;
	required_shared_ptr<uint32_t> m_arm_ram2;

	uint32_t* m_armrom;

	optional_device<cpu_device> m_prot;

	void init_theglad();
	void init_theglada();
	void init_svg();
	void init_svgpcb();
	void init_killbldp();
	void init_dmnfrnt();
	void init_happy6();
	void machine_start_pgm_arm_type3();
	void svg_arm7_ram_sel_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t svg_arm7_shareram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void svg_arm7_shareram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t svg_m68k_ram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void svg_m68k_ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t svg_68k_nmi_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void svg_68k_nmi_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void svg_latch_68k_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t svg_latch_68k_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint32_t svg_latch_arm_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void svg_latch_arm_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void svg_basic_init();
	void pgm_create_dummy_internal_arm_region(int size);
	void pgm_patch_external_arm_rom_jumptable_theglada(int base);
	void pgm_create_dummy_internal_arm_region_theglad(int is_svg);
	void pgm_descramble_happy6(uint8_t* src);
	void pgm_descramble_happy6_2(uint8_t* src);
	void svg_latch_init();
	uint32_t dmnfrnt_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint16_t dmnfrnt_main_speedup_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint32_t killbldp_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t theglad_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t happy6_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t svg_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t svgpcb_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void machine_reset_pgm_arm_type3_reset();
};

MACHINE_CONFIG_EXTERN( pgm_arm_type3 );
INPUT_PORTS_EXTERN(theglad);
INPUT_PORTS_EXTERN(happy6);
INPUT_PORTS_EXTERN(svg);
INPUT_PORTS_EXTERN(svgtw);
