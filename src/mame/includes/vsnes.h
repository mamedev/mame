// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
#include "video/ppu2c0x.h"

class vsnes_state : public driver_device
{
public:
	vsnes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "sub")
		, m_ppu1(*this, "ppu1")
		, m_ppu2(*this, "ppu2")
		, m_work_ram(*this, "work_ram")
		, m_work_ram_1(*this, "work_ram_1")
		, m_palette(*this, "palette")
		, m_gfx1_rom(*this, "gfx1")
	{
	}

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<ppu2c0x_device> m_ppu1;
	optional_device<ppu2c0x_device> m_ppu2;

	required_shared_ptr<uint8_t> m_work_ram;
	optional_shared_ptr<uint8_t> m_work_ram_1;
	required_device<palette_device> m_palette;

	optional_memory_region m_gfx1_rom;

	int m_coin;
	int m_do_vrom_bank;
	int m_input_latch[4];
	int m_sound_fix;
	uint8_t m_last_bank;
	std::unique_ptr<uint8_t[]> m_vram;
	uint8_t* m_vrom[2];
	std::unique_ptr<uint8_t[]> m_nt_ram[2];
	uint8_t* m_nt_page[2][4];
	uint32_t m_vrom_size[2];
	int m_vrom_banks;
	int m_zapstore;
	int m_old_bank;
	int m_drmario_shiftreg;
	int m_drmario_shiftcount;
	int m_size16k;
	int m_switchlow;
	int m_vrom4k;
	int m_MMC3_cmd;
	int m_MMC3_prg_bank[4];
	int m_MMC3_chr_bank[6];
	int m_MMC3_prg_mask;
	int m_IRQ_enable;
	int m_IRQ_count;
	int m_IRQ_count_latch;
	int m_VSindex;
	int m_supxevs_prot_index;
	int m_security_counter;
	int m_ret;
	void sprite_dma_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sprite_dma_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vsnes_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t vsnes_coin_counter_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void vsnes_coin_counter_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vsnes_in0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t vsnes_in0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t vsnes_in1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void vsnes_in0_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t vsnes_in0_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t vsnes_in1_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t gun_in0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void vsnes_nt0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vsnes_nt1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t vsnes_nt0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t vsnes_nt1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void vsnormal_vrom_banking(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gun_in0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vskonami_rom_banking(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vsgshoe_gun_in0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void drmario_rom_banking(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vsvram_rom_banking(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mapper4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rbi_hack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t supxevs_read_prot_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t supxevs_read_prot_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t supxevs_read_prot_3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t supxevs_read_prot_4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tko_security_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mapper68_rom_banking(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void set_bnglngby_irq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t set_bnglngby_irq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void vsdual_vrom_banking_main(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vsdual_vrom_banking_sub(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void v_set_mirroring(int ppu, int mirroring);

	void init_vskonami();
	void init_vsvram();
	void init_bnglngby();
	void init_drmario();
	void init_MMC3();
	void init_vsfdf();
	void init_tkoboxng();
	void init_vsgun();
	void init_supxevs();
	void init_vsgshoe();
	void init_vsnormal();
	void init_platoon();
	void init_rbibb();
	void init_vsdual();
	void machine_start_vsnes();
	void machine_reset_vsnes();
	void video_start_vsnes();
	void palette_init_vsnes(palette_device &palette);
	void machine_start_vsdual();
	void machine_reset_vsdual();
	void video_start_vsdual();
	void palette_init_vsdual(palette_device &palette);
	uint32_t screen_update_vsnes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_vsnes_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void v_set_videorom_bank(  int start, int count, int vrom_start_bank );
	void mapper4_set_prg(  );
	void mapper4_set_chr(  );
	void mapper4_irq( int scanline, int vblank, int blanked );
	void ppu_irq_1(int *ppu_regs);
	void ppu_irq_2(int *ppu_regs);

	uint8_t vsnes_bootleg_z80_latch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bootleg_sound_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t vsnes_bootleg_z80_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t vsnes_bootleg_z80_address_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t m_bootleg_sound_offset;
	uint8_t m_bootleg_sound_data;

};
