// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
#include "video/ppu2c0x.h"

class vsnes_state : public driver_device
{
public:
	vsnes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_ppu1(*this, "ppu1"),
		m_ppu2(*this, "ppu2"),
		m_work_ram(*this, "work_ram"),
		m_work_ram_1(*this, "work_ram_1")
		{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<ppu2c0x_device> m_ppu1;
	optional_device<ppu2c0x_device> m_ppu2;

	required_shared_ptr<UINT8> m_work_ram;
	optional_shared_ptr<UINT8> m_work_ram_1;
	int m_coin;
	int m_do_vrom_bank;
	int m_input_latch[4];
	int m_sound_fix;
	UINT8 m_last_bank;
	UINT8* m_vram;
	UINT8* m_vrom[2];
	UINT8* m_nt_ram[2];
	UINT8* m_nt_page[2][4];
	UINT32 m_vrom_size[2];
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
	DECLARE_WRITE8_MEMBER(sprite_dma_0_w);
	DECLARE_WRITE8_MEMBER(sprite_dma_1_w);
	DECLARE_WRITE8_MEMBER(vsnes_coin_counter_w);
	DECLARE_READ8_MEMBER(vsnes_coin_counter_r);
	DECLARE_WRITE8_MEMBER(vsnes_coin_counter_1_w);
	DECLARE_WRITE8_MEMBER(vsnes_in0_w);
	DECLARE_READ8_MEMBER(vsnes_in0_r);
	DECLARE_READ8_MEMBER(vsnes_in1_r);
	DECLARE_WRITE8_MEMBER(vsnes_in0_1_w);
	DECLARE_READ8_MEMBER(vsnes_in0_1_r);
	DECLARE_READ8_MEMBER(vsnes_in1_1_r);
	DECLARE_READ8_MEMBER(gun_in0_r);
	DECLARE_WRITE8_MEMBER(vsnes_nt0_w);
	DECLARE_WRITE8_MEMBER(vsnes_nt1_w);
	DECLARE_READ8_MEMBER(vsnes_nt0_r);
	DECLARE_READ8_MEMBER(vsnes_nt1_r);
	DECLARE_WRITE8_MEMBER(vsnormal_vrom_banking);
	DECLARE_WRITE8_MEMBER(gun_in0_w);
	DECLARE_WRITE8_MEMBER(vskonami_rom_banking);
	DECLARE_WRITE8_MEMBER(vsgshoe_gun_in0_w);
	DECLARE_WRITE8_MEMBER(drmario_rom_banking);
	DECLARE_WRITE8_MEMBER(vsvram_rom_banking);
	DECLARE_WRITE8_MEMBER(mapper4_w);
	DECLARE_READ8_MEMBER(rbi_hack_r);
	DECLARE_READ8_MEMBER(supxevs_read_prot_1_r);
	DECLARE_READ8_MEMBER(supxevs_read_prot_2_r);
	DECLARE_READ8_MEMBER(supxevs_read_prot_3_r);
	DECLARE_READ8_MEMBER(supxevs_read_prot_4_r);
	DECLARE_READ8_MEMBER(tko_security_r);
	DECLARE_WRITE8_MEMBER(mapper68_rom_banking);
	DECLARE_WRITE8_MEMBER(set_bnglngby_irq_w);
	DECLARE_READ8_MEMBER(set_bnglngby_irq_r);
	DECLARE_WRITE8_MEMBER(vsdual_vrom_banking_main);
	DECLARE_WRITE8_MEMBER(vsdual_vrom_banking_sub);
	void v_set_mirroring(int ppu, int mirroring);

	DECLARE_DRIVER_INIT(vskonami);
	DECLARE_DRIVER_INIT(vsvram);
	DECLARE_DRIVER_INIT(bnglngby);
	DECLARE_DRIVER_INIT(drmario);
	DECLARE_DRIVER_INIT(MMC3);
	DECLARE_DRIVER_INIT(vsfdf);
	DECLARE_DRIVER_INIT(tkoboxng);
	DECLARE_DRIVER_INIT(vsgun);
	DECLARE_DRIVER_INIT(supxevs);
	DECLARE_DRIVER_INIT(vsgshoe);
	DECLARE_DRIVER_INIT(vsnormal);
	DECLARE_DRIVER_INIT(platoon);
	DECLARE_DRIVER_INIT(rbibb);
	DECLARE_DRIVER_INIT(vsdual);
	DECLARE_MACHINE_START(vsnes);
	DECLARE_MACHINE_RESET(vsnes);
	DECLARE_VIDEO_START(vsnes);
	DECLARE_PALETTE_INIT(vsnes);
	DECLARE_MACHINE_START(vsdual);
	DECLARE_MACHINE_RESET(vsdual);
	DECLARE_VIDEO_START(vsdual);
	DECLARE_PALETTE_INIT(vsdual);
	UINT32 screen_update_vsnes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_vsnes_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void v_set_videorom_bank(  int start, int count, int vrom_start_bank );
	void mapper4_set_prg(  );
	void mapper4_set_chr(  );
	void mapper4_irq( int scanline, int vblank, int blanked );
	void ppu_irq_1(int *ppu_regs);
	void ppu_irq_2(int *ppu_regs);

	DECLARE_READ8_MEMBER( vsnes_bootleg_z80_latch_r );
	DECLARE_WRITE8_MEMBER(bootleg_sound_write);
	DECLARE_READ8_MEMBER(vsnes_bootleg_z80_data_r);
	DECLARE_READ8_MEMBER(vsnes_bootleg_z80_address_r);
	UINT8 m_bootleg_sound_offset;
	UINT8 m_bootleg_sound_data;

};
