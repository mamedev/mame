// license:BSD-3-Clause
// copyright-holders:David Haywood, Xing Xing

class pgm_arm_type3_state : public pgm_state
{
public:
	pgm_arm_type3_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag)
		, m_arm_ram(*this, "arm_ram")
		, m_arm_ram2(*this, "arm_ram2")
		, m_armrom(*this, "prot")
		, m_prot(*this, "prot") { }

	void init_theglad();
	void init_theglada();
	void init_svg();
	void init_svgpcb();
	void init_killbldp();
	void init_dmnfrnt();
	void init_happy6();

	void pgm_arm_type3(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// svg
	u8           m_ram_sel;
	std::unique_ptr<u32[]>      m_shareram[2];    //for 5585G MACHINE

	u32        m_latchdata_68k_w;
	u32        m_latchdata_arm_w;
	required_shared_ptr<u32> m_arm_ram;
	required_shared_ptr<u32> m_arm_ram2;
	required_region_ptr<u32> m_armrom;

	optional_device<cpu_device> m_prot;

	DECLARE_WRITE32_MEMBER(arm7_ram_sel_w);
	DECLARE_READ32_MEMBER(arm7_shareram_r);
	DECLARE_WRITE32_MEMBER(arm7_shareram_w);
	DECLARE_READ16_MEMBER(m68k_ram_r);
	DECLARE_WRITE16_MEMBER(m68k_ram_w);
	DECLARE_READ16_MEMBER(m68k_nmi_r);
	DECLARE_WRITE16_MEMBER(m68k_nmi_w);
	DECLARE_WRITE16_MEMBER(latch_68k_w);
	DECLARE_READ16_MEMBER(latch_68k_r);
	DECLARE_READ32_MEMBER(latch_arm_r);
	DECLARE_WRITE32_MEMBER(latch_arm_w);
	void create_dummy_internal_arm_region(int size);
	void patch_external_arm_rom_jumptable_theglada(int base);
	void create_dummy_internal_arm_region_theglad(int is_svg);
	void descramble_happy6(u8* src);
	void descramble_happy6_2(u8* src);
	DECLARE_READ32_MEMBER(dmnfrnt_speedup_r);
	DECLARE_READ16_MEMBER(dmnfrnt_main_speedup_r);
	DECLARE_READ32_MEMBER(killbldp_speedup_r);
	DECLARE_READ32_MEMBER(theglad_speedup_r);
	DECLARE_READ32_MEMBER(happy6_speedup_r);
	DECLARE_READ32_MEMBER(svg_speedup_r);
	DECLARE_READ32_MEMBER(svgpcb_speedup_r);
	void _55857G_arm7_map(address_map &map);
	void svg_68k_mem(address_map &map);
};

INPUT_PORTS_EXTERN(theglad);
INPUT_PORTS_EXTERN(happy6);
INPUT_PORTS_EXTERN(svg);
INPUT_PORTS_EXTERN(svgtw);
INPUT_PORTS_EXTERN(svgpcb);
