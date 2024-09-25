// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi, Xing Xing
#include "pgm.h"

class pgm_arm_type1_state : public pgm_state
{
public:
	pgm_arm_type1_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag)
		, m_arm7_shareram(*this, "arm7_shareram")
		, m_prot(*this, "prot")
	{
		m_curslots = 0;
		m_puzzli_54_trigger = 0;
	}

	void init_photoy2k();
	void init_kovsh();
	void init_kovshp();
	void init_kovshxas();
	void init_kovlsqh2();
	void init_kovqhsgs();
	void init_ddp3();
	void init_ket();
	void init_espgal();
	void init_puzzli2();
	void init_py2k2();
	void init_pgm3in1();
	void init_pstar();
	void init_kov();
	void init_kovboot();
	void init_oldsplus();

	void pgm_arm_type1_sim(machine_config &config);
	void pgm_arm_type1_cave(machine_config &config);
	void pgm_arm_type1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	/////////////// simulations
	u16 m_value0 = 0;
	u16 m_value1 = 0;
	u16 m_valuekey = 0;
	u16 m_ddp3lastcommand = 0;
	u32 m_valueresponse = 0;
	int m_curslots = 0;
	u32 m_slots[0x100]{};

	// pstars / oldsplus / kov
	u16 m_pstar_e7_value = 0;
	u16 m_pstar_b1_value = 0;
	u16 m_pstar_ce_value = 0;
	u16 m_kov_c0_value = 0;
	u16 m_kov_cb_value = 0;
	u16 m_kov_fe_value = 0;
	u16 m_extra_ram[0x100]{};
	// puzzli2
	s32 m_puzzli_54_trigger;

	typedef void (pgm_arm_type1_state::*pgm_arm_sim_command_handler)(int pc);

	pgm_arm_sim_command_handler arm_sim_handler{};

	/////////////// emulation
	u32 m_arm_type1_latch_arm_w = 0;
	u32 m_arm_type1_latch_68k_w = 0;
	u32 m_arm_type1_counter = 0;
	optional_shared_ptr<u32> m_arm7_shareram;

	optional_device<cpu_device> m_prot;
	DECLARE_MACHINE_START(pgm_arm_type1);

	u32 arm7_type1_protlatch_r();
	void arm7_type1_protlatch_w(offs_t offset, u32 data, u32 mem_mask);
	u16 arm7_type1_68k_protlatch_r(offs_t offset);
	void arm7_type1_68k_protlatch_w(offs_t offset, u16 data);
	u16 arm7_type1_ram_r(offs_t offset, u16 mem_mask = ~0);
	void arm7_type1_ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u32 arm7_type1_unk_r();
	u32 arm7_type1_exrom_r();
	u32 arm7_type1_shareram_r(offs_t offset, u32 mem_mask = ~0);
	void arm7_type1_shareram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void arm7_type1_latch_init();
	u16 kovsh_fake_region_r();
	void kovshp_asic27a_write_word(offs_t offset, u16 data);
	void pgm_decode_kovlsqh2_tiles();
	void pgm_decode_kovlsqh2_sprites(u8 *src );
	void pgm_decode_kovlsqh2_samples();
	void pgm_decode_kovqhsgs_program();
	void pgm_decode_kovqhsgs2_program();
	u16 arm7_type1_sim_r(offs_t offset);
	void command_handler_ddp3(int pc);
	void command_handler_puzzli2(int pc);
	void command_handler_py2k2(int pc);
	void command_handler_pstars(int pc);
	void command_handler_kov(int pc);
	void command_handler_oldsplus(int pc);
	void arm7_type1_sim_w(offs_t offset, u16 data);
	u16 arm7_type1_sim_protram_r(offs_t offset);
	u16 pstars_arm7_type1_sim_protram_r(offs_t offset);
	int m_simregion = 0;

	/* puzzli2 protection internal state stuff */
	int stage = 0;
	int tableoffs = 0;
	int tableoffs2 = 0;
	int entries_left = 0;
	int currentcolumn = 0;
	int currentrow = 0;
	int num_entries = 0;
	int full_entry = 0;
	int prev_tablloc = 0;
	int numbercolumns = 0;
	int depth = 0;
	u16 m_row_bitmask = 0;
	int hackcount = 0;
	int hackcount2 = 0;
	int hack_47_value = 0;
	int hack_31_table_offset = 0;
	int hack_31_table_offset2 = 0;
	int p2_31_retcounter = 0;

	u8 coverage[256]{}; // coverage is how much of the table we've managed to verify using known facts about the table structure

	int command_31_write_type = 0;

	// py2k2 protection related
	u16 m_py2k2_sprite_pos = 0;
	u16 m_py2k2_sprite_base = 0;
	u16 m_py2k2_prev_base = 0;

	// the maximum level size returned or read by the device appears to be this size
	u16 level_structure[8][10]{};


	int puzzli2_take_leveldata_value(u8 datvalue);
	void _55857E_arm7_map(address_map &map) ATTR_COLD;
	void cavepgm_mem(address_map &map) ATTR_COLD;
	void kov_map(address_map &map) ATTR_COLD;
	void kov_sim_map(address_map &map) ATTR_COLD;
};

INPUT_PORTS_EXTERN( sango );
INPUT_PORTS_EXTERN( sango_ch );
INPUT_PORTS_EXTERN( photoy2k );
INPUT_PORTS_EXTERN( photoy2kj );
INPUT_PORTS_EXTERN( oldsplus );
INPUT_PORTS_EXTERN( pstar );
INPUT_PORTS_EXTERN( py2k2 );
INPUT_PORTS_EXTERN( pgm3in1 );
INPUT_PORTS_EXTERN( puzzli2 );
INPUT_PORTS_EXTERN( kovsh );
INPUT_PORTS_EXTERN( ddp3 );
INPUT_PORTS_EXTERN( espgal );
