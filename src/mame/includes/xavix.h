// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_XAVIX_H
#define MAME_INCLUDES_XAVIX_H

#include "cpu/m6502/xavix.h"
#include "cpu/m6502/xavix2000.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "machine/bankdev.h"


class xavix_state : public driver_device
{
public:
	xavix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_spr_attr0(*this, "spr_attr0"),
		m_spr_attr1(*this, "spr_attr1"),
		m_spr_ypos(*this, "spr_ypos"),
		m_spr_xpos(*this, "spr_xpos"),
		m_spr_addr_lo(*this, "spr_addr_lo"),
		m_spr_addr_md(*this, "spr_addr_md"),
		m_spr_addr_hi(*this, "spr_addr_hi"),
		m_palram1(*this, "palram1"),
		m_palram2(*this, "palram2"),
		m_spr_attra(*this, "spr_attra"),
		m_palette(*this, "palette"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_region(*this, "REGION"),
		m_gfxdecode(*this, "gfxdecode"),
		m_alt_addressing(0),
		m_lowbus(*this, "lowbus")
	{ }

	void xavix(machine_config &config);
	void xavixp(machine_config &config);
	void xavix2000(machine_config &config);

	void init_xavix();
	void init_taitons1();
	void init_rad_box();

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void xavix_map(address_map &map);
	void xavix_lowbus_map(address_map &map);

	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	DECLARE_READ8_MEMBER(main_r);
	DECLARE_WRITE8_MEMBER(main_w);
	DECLARE_READ8_MEMBER(main2_r);
	DECLARE_WRITE8_MEMBER(main2_w);

	DECLARE_WRITE8_MEMBER(extintrf_7900_w);
	DECLARE_WRITE8_MEMBER(extintrf_7901_w);
	DECLARE_WRITE8_MEMBER(extintrf_7902_w);

	DECLARE_WRITE8_MEMBER(xavix_7a80_w);
	DECLARE_WRITE8_MEMBER(adc_7b00_w);
	DECLARE_READ8_MEMBER(adc_7b80_r);
	DECLARE_WRITE8_MEMBER(adc_7b80_w);
	DECLARE_WRITE8_MEMBER(adc_7b81_w);

	DECLARE_WRITE8_MEMBER(slotreg_7810_w);

	DECLARE_WRITE8_MEMBER(rom_dmatrg_w);
	DECLARE_WRITE8_MEMBER(rom_dmasrc_lo_w);
	DECLARE_WRITE8_MEMBER(rom_dmasrc_md_w);
	DECLARE_WRITE8_MEMBER(rom_dmasrc_hi_w);
	DECLARE_WRITE8_MEMBER(rom_dmadst_lo_w);
	DECLARE_WRITE8_MEMBER(rom_dmadst_hi_w);
	DECLARE_WRITE8_MEMBER(rom_dmalen_lo_w);
	DECLARE_WRITE8_MEMBER(rom_dmalen_hi_w);
	DECLARE_READ8_MEMBER(rom_dmatrg_r);

	DECLARE_WRITE8_MEMBER(vid_dma_params_1_w);
	DECLARE_WRITE8_MEMBER(vid_dma_params_2_w);
	DECLARE_WRITE8_MEMBER(vid_rom_dmatrg_w);
	DECLARE_READ8_MEMBER(vid_rom_dmatrg_r);

	DECLARE_READ8_MEMBER(io_0_r);
	DECLARE_READ8_MEMBER(io_1_r);
	DECLARE_READ8_MEMBER(io_2_r);
	DECLARE_READ8_MEMBER(io_3_r);

	DECLARE_WRITE8_MEMBER(io_0_w);
	DECLARE_WRITE8_MEMBER(io_1_w);
	DECLARE_WRITE8_MEMBER(io_2_w);
	DECLARE_WRITE8_MEMBER(io_3_w);

	DECLARE_WRITE8_MEMBER(vector_enable_w);
	DECLARE_WRITE8_MEMBER(irq_vector0_lo_w);
	DECLARE_WRITE8_MEMBER(irq_vector0_hi_w);
	DECLARE_WRITE8_MEMBER(irq_vector1_lo_w);
	DECLARE_WRITE8_MEMBER(irq_vector1_hi_w);

	DECLARE_READ8_MEMBER(irq_source_r);
	DECLARE_WRITE8_MEMBER(irq_source_w);

	DECLARE_READ8_MEMBER(arena_6fe8_r);
	DECLARE_WRITE8_MEMBER(arena_6fe8_w);
	DECLARE_READ8_MEMBER(arena_6fe9_r);
	DECLARE_WRITE8_MEMBER(arena_6fe9_w);
	DECLARE_WRITE8_MEMBER(arena_6fea_w);

	DECLARE_READ8_MEMBER(colmix_6ff0_r);
	DECLARE_WRITE8_MEMBER(colmix_6ff0_w);

	DECLARE_WRITE8_MEMBER(colmix_6ff1_w);
	DECLARE_WRITE8_MEMBER(colmix_6ff2_w);

	DECLARE_READ8_MEMBER(dispctrl_6ff8_r);
	DECLARE_WRITE8_MEMBER(dispctrl_6ff8_w);

	DECLARE_WRITE8_MEMBER(dispctrl_6ffa_w);
	DECLARE_WRITE8_MEMBER(dispctrl_6ffb_w);

	DECLARE_READ8_MEMBER(sound_75f0_r);
	DECLARE_WRITE8_MEMBER(sound_75f0_w);

	DECLARE_READ8_MEMBER(sound_75f1_r);
	DECLARE_WRITE8_MEMBER(sound_75f1_w);

	DECLARE_READ8_MEMBER(sound_75f4_r);
	DECLARE_READ8_MEMBER(sound_75f5_r);
	DECLARE_READ8_MEMBER(sound_75f6_r);
	DECLARE_WRITE8_MEMBER(sound_75f6_w);

	DECLARE_WRITE8_MEMBER(sound_75f7_w);

	DECLARE_READ8_MEMBER(sound_75f8_r);
	DECLARE_WRITE8_MEMBER(sound_75f8_w);

	DECLARE_READ8_MEMBER(sound_75f9_r);
	DECLARE_WRITE8_MEMBER(sound_75f9_w);

	DECLARE_READ8_MEMBER(sound_75fa_r);
	DECLARE_WRITE8_MEMBER(sound_75fa_w);
	DECLARE_READ8_MEMBER(sound_75fb_r);
	DECLARE_WRITE8_MEMBER(sound_75fb_w);
	DECLARE_READ8_MEMBER(sound_75fc_r);
	DECLARE_WRITE8_MEMBER(sound_75fc_w);
	DECLARE_READ8_MEMBER(sound_75fd_r);
	DECLARE_WRITE8_MEMBER(sound_75fd_w);

	DECLARE_WRITE8_MEMBER(sound_75fe_w);
	DECLARE_WRITE8_MEMBER(sound_75ff_w);

	DECLARE_WRITE8_MEMBER(timer_7c00_w);
	DECLARE_READ8_MEMBER(timer_7c01_r);
	DECLARE_WRITE8_MEMBER(timer_7c01_w);
	DECLARE_WRITE8_MEMBER(timer_7c02_w);

	//DECLARE_WRITE8_MEMBER(xavix_6fc0_w);
	DECLARE_WRITE8_MEMBER(tmap1_regs_w);
	DECLARE_WRITE8_MEMBER(spriteregs_w);
	DECLARE_WRITE8_MEMBER(tmap2_regs_w);
	DECLARE_READ8_MEMBER(tmap2_regs_r);

	DECLARE_READ8_MEMBER(pal_ntsc_r);

	DECLARE_READ8_MEMBER(xavix_4000_r);

	DECLARE_READ8_MEMBER(mult_r);
	DECLARE_WRITE8_MEMBER(mult_w);
	DECLARE_WRITE8_MEMBER(mult_param_w);

	required_device<cpu_device> m_maincpu;

	uint8_t m_rom_dmasrc_lo_data;
	uint8_t m_rom_dmasrc_md_data;
	uint8_t m_rom_dmasrc_hi_data;

	uint8_t m_rom_dmadst_lo_data;
	uint8_t m_rom_dmadst_hi_data;

	uint8_t m_rom_dmalen_lo_data;
	uint8_t m_rom_dmalen_hi_data;

	uint8_t m_vectorenable;
	uint8_t m_irq_vector0_lo_data;
	uint8_t m_irq_vector0_hi_data;
	uint8_t m_irq_vector1_lo_data;
	uint8_t m_irq_vector1_hi_data;

	uint8_t m_multparams[3];
	uint8_t m_multresults[2];

	uint8_t m_vid_dma_param1[2];
	uint8_t m_vid_dma_param2[2];

	uint8_t m_tmap1_regs[8];
	uint8_t m_tmap2_regs[8];

	uint8_t m_6fe8;
	uint8_t m_6fe9;

	uint8_t m_6ff0;
	uint8_t m_6ff8;

	uint8_t m_75fx[0x10];

	uint8_t m_7c01;

	int16_t get_vectors(int which, int half);

	required_shared_ptr<uint8_t> m_mainram;

	required_shared_ptr<uint8_t> m_spr_attr0;
	required_shared_ptr<uint8_t> m_spr_attr1;
	required_shared_ptr<uint8_t> m_spr_ypos;
	required_shared_ptr<uint8_t> m_spr_xpos;

	required_shared_ptr<uint8_t> m_spr_addr_lo;
	required_shared_ptr<uint8_t> m_spr_addr_md;
	required_shared_ptr<uint8_t> m_spr_addr_hi;

	required_shared_ptr<uint8_t> m_palram1;
	required_shared_ptr<uint8_t> m_palram2;

	required_shared_ptr<uint8_t> m_spr_attra;

	required_device<palette_device> m_palette;

	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport m_region;

	required_device<gfxdecode_device> m_gfxdecode;

	void handle_palette(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	double hue2rgb(double p, double q, double t);
	void draw_tile(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int tile, int bpp, int xpos, int ypos, int drawheight, int drawwidth, int flipx, int flipy, int pal, int opaque);
	void draw_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	int m_rgnlen;
	uint8_t* m_rgn;

	// variables used by rendering
	int m_tmp_dataaddress;
	int m_tmp_databit;
	void set_data_address(int address, int bit);
	uint8_t get_next_bit();
	uint8_t get_next_byte();

	int get_current_address_byte();

	int m_alt_addressing;
	required_device<address_map_bank_device> m_lowbus;
};

#endif // MAME_INCLUDES_XAVIX_H
