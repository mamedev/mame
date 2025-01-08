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
	void init_theglad();
	void init_theglada();
	void init_svg();
	void init_svgpcb();
	void init_killbldp();
	void init_dmnfrnt();
	void init_happy6();

	void pgm_arm_type3(machine_config &config);
	void pgm_arm_type3_22m(machine_config &config);
	void pgm_arm_type3_24m(machine_config &config);
	void pgm_arm_type3_33m(machine_config &config);
	void pgm_arm_type3_33_8688m(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	// svg
	int m_svg_ram_sel = 0;
	std::unique_ptr<u32[]> m_svg_shareram[2];    //for 5585G MACHINE

	u32 m_svg_latchdata_68k_w = 0;
	u32 m_svg_latchdata_arm_w = 0;
	required_shared_ptr<u32> m_arm_ram;
	required_shared_ptr<u32> m_arm_ram2;

	u32* m_armrom = nullptr;

	optional_device<cpu_device> m_prot;

	void svg_arm7_ram_sel_w(u32 data);
	u32 svg_arm7_shareram_r(offs_t offset);
	void svg_arm7_shareram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u16 svg_m68k_ram_r(offs_t offset);
	void svg_m68k_ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 svg_68k_nmi_r();
	void svg_68k_nmi_w(u16 data);
	void svg_latch_68k_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 svg_latch_68k_r(offs_t offset, u16 mem_mask = ~0);
	u32 svg_latch_arm_r(offs_t offset, u32 mem_mask = ~0);
	void svg_latch_arm_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void svg_basic_init();
	void pgm_create_dummy_internal_arm_region(int size);
	void pgm_patch_external_arm_rom_jumptable_theglada(int base);
	void pgm_create_dummy_internal_arm_region_theglad(int is_svg);
	void pgm_descramble_happy6(u8* src);
	void pgm_descramble_happy6_2(u8* src);
	void svg_latch_init();
	u32 dmnfrnt_speedup_r();
	u16 dmnfrnt_main_speedup_r();
	u32 killbldp_speedup_r();
	u32 theglad_speedup_r();
	u32 happy6_speedup_r();
	u32 svg_speedup_r();
	u32 svgpcb_speedup_r();
	void _55857G_arm7_map(address_map &map) ATTR_COLD;
	void svg_68k_mem(address_map &map) ATTR_COLD;
};

INPUT_PORTS_EXTERN(theglad);
INPUT_PORTS_EXTERN(happy6);
INPUT_PORTS_EXTERN(happy6hk);
INPUT_PORTS_EXTERN(svg);
INPUT_PORTS_EXTERN(svghk);
INPUT_PORTS_EXTERN(svgtw);
INPUT_PORTS_EXTERN(svgpcb);
