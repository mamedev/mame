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
#include "machine/i2cmem.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "machine/xavix_mtrk_wheel.h"
#include "machine/xavix_madfb_ball.h"

class xavix_state : public driver_device
{
public:
	xavix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_mainram(*this, "mainram"),
		m_fragment_sprite(*this, "fragment_sprite"),
		m_rom_dma_src(*this,"rom_dma_src"),
		m_rom_dma_dst(*this,"rom_dma_dst"),
		m_rom_dma_len(*this,"rom_dma_len"),
		m_palram_sh(*this, "palram_sh"),
		m_palram_l(*this, "palram_l"),
		m_bmp_palram_sh(*this, "bmp_palram_sh"),
		m_bmp_palram_l(*this, "bmp_palram_l"),
		m_bmp_base(*this, "bmp_base"),
		m_colmix_sh(*this, "colmix_sh"),
		m_colmix_l(*this, "colmix_l"),
		m_colmix_ctrl(*this, "colmix_ctrl"),
		m_posirq_x(*this, "posirq_x"),
		m_posirq_y(*this, "posirq_y"),
		m_segment_regs(*this, "segment_regs"),
		m_palette(*this, "palette"),
		m_region(*this, "REGION"),
		m_gfxdecode(*this, "gfxdecode"),
		m_lowbus(*this, "lowbus"),
		m_hack_timer_disable(false)
	{ }
	
	void xavix(machine_config &config);
	void xavixp(machine_config &config);
	void xavix2000(machine_config &config);

	void init_xavix();
	void init_bass();

	DECLARE_WRITE_LINE_MEMBER(ioevent_trg01);
	DECLARE_WRITE_LINE_MEMBER(ioevent_trg02);
	DECLARE_WRITE_LINE_MEMBER(ioevent_trg04);
	DECLARE_WRITE_LINE_MEMBER(ioevent_trg08);

protected:

	virtual uint8_t read_io0(uint8_t direction);
	virtual uint8_t read_io1(uint8_t direction);
	virtual void write_io0(uint8_t data, uint8_t direction);
	virtual void write_io1(uint8_t data, uint8_t direction);
	required_ioport m_in0;
	required_ioport m_in1;

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void xavix_map(address_map &map);

	void xavix_lowbus_map(address_map &map);
	void xavix_extbus_map(address_map &map);
	void superxavix_lowbus_map(address_map &map);

	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	DECLARE_READ8_MEMBER(opcodes_000000_r);
	DECLARE_READ8_MEMBER(opcodes_800000_r);

	DECLARE_READ8_MEMBER(extbus_r) { return m_rgn[(offset) & (m_rgnlen - 1)]; }
	DECLARE_WRITE8_MEMBER(extbus_w) { logerror("write to external bus %06x %02x\n", offset, data); }


	/* this is just a quick memory system bypass for video reads etc. because going through the
	   memory system is slow and also pollutes logs significantly with unmapped reads if the games
	   enable the video before actually setting up the source registers! 
   
	   this will need modifying if any games have RAM instead of ROM (which I think is possible
	   with SuperXaviX at least)
	*/
	inline uint8_t read_full_data_sp_lowbus_bypass(uint16_t adr)
	{
		adr &= 0x7fff;

		if (adr < 0x4000)
		{
			adr &= 0x3fff;
			return m_mainram[adr];
		}
		else if (adr < 0x5000)
		{
			adr &= 0xfff;
			return txarray_r(adr);
		}
		else if ((adr >= 0x6000) && (adr < 0x6800))
		{
			adr &= 0x7ff;
			return m_fragment_sprite[adr];
		}
		else if ((adr >= 0x6800) && (adr < 0x6900))
		{
			adr &= 0xff;
			return m_palram_sh[adr];
		}
		else if ((adr >= 0x6900) && (adr < 0x6a00))
		{
			adr &= 0xff;
			return m_palram_l[adr];
		}
		else if ((adr >= 0x6a00) && (adr < 0x6a20))
		{
			adr &= 0x1f;
			return m_segment_regs[adr];
		}
		// superxavix bitmap palette?

		return 0x00;
	}

	inline uint8_t read_full_data_sp_bypass(uint32_t adr)
	{
		uint8_t databank = adr >> 16;

		if (databank >= 0x80)
		{
			return m_rgn[adr & (m_rgnlen - 1)];
		}
		else
		{
			if ((adr&0xffff) >= 0x8000)
			{
				return m_rgn[adr & (m_rgnlen - 1)];
			}
			else
			{
				return read_full_data_sp_lowbus_bypass(adr);
			}
		}
	}

	DECLARE_WRITE8_MEMBER(extintrf_7900_w);
	DECLARE_WRITE8_MEMBER(extintrf_7901_w);
	DECLARE_WRITE8_MEMBER(extintrf_7902_w);

	DECLARE_READ8_MEMBER(ioevent_enable_r);
	DECLARE_WRITE8_MEMBER(ioevent_enable_w);
	DECLARE_READ8_MEMBER(ioevent_irqstate_r);
	DECLARE_WRITE8_MEMBER(ioevent_irqack_w);
	uint8_t m_ioevent_enable;
	uint8_t m_ioevent_active;
	void process_ioevent(uint8_t bits);

	DECLARE_WRITE8_MEMBER(adc_7b00_w);
	DECLARE_READ8_MEMBER(adc_7b80_r);
	DECLARE_WRITE8_MEMBER(adc_7b80_w);
	DECLARE_READ8_MEMBER(adc_7b81_r);
	DECLARE_WRITE8_MEMBER(adc_7b81_w);

	DECLARE_WRITE8_MEMBER(slotreg_7810_w);

	DECLARE_WRITE8_MEMBER(rom_dmatrg_w);

	DECLARE_WRITE8_MEMBER(rom_dmasrc_w);

	DECLARE_WRITE8_MEMBER(rom_dmadst_w);
	DECLARE_WRITE8_MEMBER(rom_dmalen_w);
	DECLARE_READ8_MEMBER(rom_dmastat_r);

	DECLARE_WRITE8_MEMBER(spritefragment_dma_params_1_w);
	DECLARE_WRITE8_MEMBER(spritefragment_dma_params_2_w);
	DECLARE_WRITE8_MEMBER(spritefragment_dma_trg_w);
	DECLARE_READ8_MEMBER(spritefragment_dma_status_r);

	DECLARE_READ8_MEMBER(io0_data_r);
	DECLARE_READ8_MEMBER(io1_data_r);
	DECLARE_WRITE8_MEMBER(io0_data_w);
	DECLARE_WRITE8_MEMBER(io1_data_w);

	DECLARE_READ8_MEMBER(io0_direction_r);
	DECLARE_READ8_MEMBER(io1_direction_r);
	DECLARE_WRITE8_MEMBER(io0_direction_w);
	DECLARE_WRITE8_MEMBER(io1_direction_w);

	uint8_t m_io0_data;
	uint8_t m_io1_data;
	uint8_t m_io0_direction;
	uint8_t m_io1_direction;

	DECLARE_WRITE8_MEMBER(vector_enable_w);
	DECLARE_WRITE8_MEMBER(nmi_vector_lo_w);
	DECLARE_WRITE8_MEMBER(nmi_vector_hi_w);
	DECLARE_WRITE8_MEMBER(irq_vector_lo_w);
	DECLARE_WRITE8_MEMBER(irq_vector_hi_w);

	DECLARE_READ8_MEMBER(irq_source_r);
	DECLARE_WRITE8_MEMBER(irq_source_w);

	DECLARE_READ8_MEMBER(arena_start_r);
	DECLARE_WRITE8_MEMBER(arena_start_w);
	DECLARE_READ8_MEMBER(arena_end_r);
	DECLARE_WRITE8_MEMBER(arena_end_w);
	DECLARE_READ8_MEMBER(arena_control_r);
	DECLARE_WRITE8_MEMBER(arena_control_w);

	DECLARE_READ8_MEMBER(colmix_6ff0_r);
	DECLARE_WRITE8_MEMBER(colmix_6ff0_w);

	DECLARE_WRITE8_MEMBER(colmix_6ff1_w);
	DECLARE_WRITE8_MEMBER(colmix_6ff2_w);

	DECLARE_READ8_MEMBER(dispctrl_6ff8_r);
	DECLARE_WRITE8_MEMBER(dispctrl_6ff8_w);

	DECLARE_READ8_MEMBER(sound_reg16_0_r);
	DECLARE_WRITE8_MEMBER(sound_reg16_0_w);
	DECLARE_READ8_MEMBER(sound_reg16_1_r);
	DECLARE_WRITE8_MEMBER(sound_reg16_1_w);

	DECLARE_READ8_MEMBER(sound_sta16_r);
	DECLARE_READ8_MEMBER(sound_75f5_r);
	DECLARE_READ8_MEMBER(sound_volume_r);
	DECLARE_WRITE8_MEMBER(sound_volume_w);

	DECLARE_WRITE8_MEMBER(sound_regbase_w);

	DECLARE_READ8_MEMBER(sound_75f8_r);
	DECLARE_WRITE8_MEMBER(sound_75f8_w);

	DECLARE_READ8_MEMBER(sound_75f9_r);
	DECLARE_WRITE8_MEMBER(sound_75f9_w);

	DECLARE_READ8_MEMBER(sound_timer0_r);
	DECLARE_WRITE8_MEMBER(sound_timer0_w);
	DECLARE_READ8_MEMBER(sound_timer1_r);
	DECLARE_WRITE8_MEMBER(sound_timer1_w);
	DECLARE_READ8_MEMBER(sound_timer2_r);
	DECLARE_WRITE8_MEMBER(sound_timer2_w);
	DECLARE_READ8_MEMBER(sound_timer3_r);
	DECLARE_WRITE8_MEMBER(sound_timer3_w);

	DECLARE_READ8_MEMBER(sound_irqstatus_r);
	DECLARE_WRITE8_MEMBER(sound_irqstatus_w);
	DECLARE_WRITE8_MEMBER(sound_75ff_w);
	uint8_t m_sound_irqstatus;
	uint8_t m_soundreg16_0[2];
	uint8_t m_soundreg16_1[2];
	uint8_t m_sound_regbase;

	DECLARE_READ8_MEMBER(timer_status_r);
	DECLARE_WRITE8_MEMBER(timer_control_w);
	DECLARE_READ8_MEMBER(timer_baseval_r);
	DECLARE_WRITE8_MEMBER(timer_baseval_w);
	DECLARE_READ8_MEMBER(timer_freq_r);
	DECLARE_WRITE8_MEMBER(timer_freq_w);
	DECLARE_READ8_MEMBER(timer_curval_r);
	uint8_t m_timer_control;
	uint8_t m_timer_freq;
	TIMER_CALLBACK_MEMBER(freq_timer_done);
	emu_timer *m_freq_timer;

	DECLARE_WRITE8_MEMBER(palram_sh_w);
	DECLARE_WRITE8_MEMBER(palram_l_w);
	DECLARE_WRITE8_MEMBER(colmix_sh_w);
	DECLARE_WRITE8_MEMBER(colmix_l_w);
	DECLARE_WRITE8_MEMBER(bmp_palram_sh_w);
	DECLARE_WRITE8_MEMBER(bmp_palram_l_w);


	DECLARE_WRITE8_MEMBER(tmap1_regs_w);
	DECLARE_WRITE8_MEMBER(tmap2_regs_w);
	DECLARE_READ8_MEMBER(tmap1_regs_r);
	DECLARE_READ8_MEMBER(tmap2_regs_r);

	DECLARE_WRITE8_MEMBER(spriteregs_w);

	DECLARE_READ8_MEMBER(pal_ntsc_r);

	DECLARE_READ8_MEMBER(xavix_memoryemu_txarray_r);
	DECLARE_WRITE8_MEMBER(xavix_memoryemu_txarray_w);
	uint8_t m_txarray[3];

	inline uint8_t txarray_r(uint16_t offset)
	{
		if (offset < 0x100)
		{
			offset &= 0xff;
			return ((offset >> 4) | (offset << 4));
		}
		else if (offset < 0x200)
		{
			offset &= 0xff;
			return ((offset >> 4) | (~offset << 4));
		}
		else if (offset < 0x300)
		{
			offset &= 0xff;
			return ((~offset >> 4) | (offset << 4));
		}
		else if (offset < 0x400)
		{
			offset &= 0xff;
			return ((~offset >> 4) | (~offset << 4));
		}
		else if (offset < 0x800)
		{
			return m_txarray[0];
		}
		else if (offset < 0xc00)
		{
			return m_txarray[1];
		}
		else if (offset < 0x1000)
		{
			return m_txarray[2];
		}

		return 0xff;
	}

	DECLARE_READ8_MEMBER(mult_r);
	DECLARE_WRITE8_MEMBER(mult_w);
	DECLARE_READ8_MEMBER(mult_param_r);
	DECLARE_WRITE8_MEMBER(mult_param_w);

	required_device<xavix_device> m_maincpu;
	required_device<screen_device> m_screen;
	
	void update_irqs();
	uint8_t m_irqsource;

	uint8_t m_vectorenable;
	uint8_t m_nmi_vector_lo_data;
	uint8_t m_nmi_vector_hi_data;
	uint8_t m_irq_vector_lo_data;
	uint8_t m_irq_vector_hi_data;

	uint8_t m_multparams[3];
	uint8_t m_multresults[2];

	uint8_t m_spritefragment_dmaparam1[2];
	uint8_t m_spritefragment_dmaparam2[2];

	uint8_t m_tmap1_regs[8];
	uint8_t m_tmap2_regs[8];

	int m_arena_start;
	int m_arena_end;
	uint8_t m_arena_control;

	uint8_t m_6ff0;
	uint8_t m_video_ctrl;

	uint8_t m_soundregs[0x10];

	uint8_t m_timer_baseval;

	int16_t get_vectors(int which, int half);

	// raster IRQ
	TIMER_CALLBACK_MEMBER(interrupt_gen);
	emu_timer *m_interrupt_timer;
	DECLARE_WRITE8_MEMBER(dispctrl_posirq_x_w);
	DECLARE_WRITE8_MEMBER(dispctrl_posirq_y_w);

	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_fragment_sprite;
	required_shared_ptr<uint8_t> m_rom_dma_src;
	required_shared_ptr<uint8_t> m_rom_dma_dst;
	required_shared_ptr<uint8_t> m_rom_dma_len;

	required_shared_ptr<uint8_t> m_palram_sh;
	required_shared_ptr<uint8_t> m_palram_l;

	optional_shared_ptr<uint8_t> m_bmp_palram_sh;
	optional_shared_ptr<uint8_t> m_bmp_palram_l;
	optional_shared_ptr<uint8_t> m_bmp_base;

	required_shared_ptr<uint8_t> m_colmix_sh;
	required_shared_ptr<uint8_t> m_colmix_l;
	required_shared_ptr<uint8_t> m_colmix_ctrl;

	required_shared_ptr<uint8_t> m_posirq_x;
	required_shared_ptr<uint8_t> m_posirq_y;

	required_shared_ptr<uint8_t> m_segment_regs;

	required_device<palette_device> m_palette;

	required_ioport m_region;

	required_device<gfxdecode_device> m_gfxdecode;

	void update_pen(int pen, uint8_t shval, uint8_t lval);
	double hue2rgb(double p, double q, double t);
	void draw_tile_line(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int tile, int bpp, int xpos, int ypos, int drawheight, int drawwidth, int flipx, int flipy, int pal, int zval, int line);
	void draw_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which);
	void draw_tilemap_line(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int line);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites_line(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int line);

	bitmap_ind16 m_zbuffer;

	uint8_t m_spritereg;

	int m_rgnlen;
	uint8_t* m_rgn;

	// variables used by rendering
	int m_tmp_dataaddress;
	int m_tmp_databit;
	uint8_t m_bit;

	void set_data_address(int address, int bit);
	uint8_t get_next_bit();
	uint8_t get_next_byte();

	int get_current_address_byte();
	required_device<address_map_bank_device> m_lowbus;

	bool m_hack_timer_disable;
};

class xavix_i2c_state : public xavix_state
{
public:
	xavix_i2c_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag),
		m_i2cmem(*this, "i2cmem")
	{ }

	void xavix_i2c_24lc04(machine_config &config);
	void xavix_i2c_24c08(machine_config &config);

	void xavix2000_i2c_24c04(machine_config &config);
	void xavix2000_i2c_24c02(machine_config &config);

protected:
	virtual uint8_t read_io1(uint8_t direction) override;
	virtual void write_io1(uint8_t data, uint8_t direction) override;

	required_device<i2cmem_device> m_i2cmem;
};


class xavix_mtrk_state : public xavix_state
{
public:
	xavix_mtrk_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag),
		m_wheel(*this, "wheel")
	{ }

	void xavix_mtrk(machine_config &config);
	void xavix_mtrkp(machine_config &config);

	CUSTOM_INPUT_MEMBER( mtrk_wheel_r );

protected:
	required_device<xavix_mtrk_wheel_device> m_wheel;
};

class xavix_madfb_state : public xavix_state
{
public:
	xavix_madfb_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag),
		m_ball(*this, "ball")
	{ }

	void xavix_madfb(machine_config &config);

protected:
	required_device<xavix_madfb_ball_device> m_ball;
};


class xavix_ekara_state : public xavix_state
{
public:
	xavix_ekara_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag),
		m_cart(*this, "cartslot"),
		m_extra0(*this, "EXTRA0"),
		m_extra1(*this, "EXTRA1"),
		m_extraioselect(0),
		m_extraiowrite(0),
		m_extrainlatch0(0),
		m_extrainlatch1(0)
	{ }

	void xavix_ekara(machine_config &config);

protected:
	required_device<generic_slot_device> m_cart;
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(ekara_cart);
	//READ8_MEMBER(cart_r) { return m_cart->read_rom(space, offset); }

	required_ioport m_extra0;
	required_ioport m_extra1;

	virtual uint8_t read_io1(uint8_t direction) override;
	virtual void write_io0(uint8_t data, uint8_t direction) override;
	virtual void write_io1(uint8_t data, uint8_t direction) override;

	uint8_t m_extraioselect;
	uint8_t m_extraiowrite;

	uint8_t m_extrainlatch0;
	uint8_t m_extrainlatch1;

};

#endif // MAME_INCLUDES_XAVIX_H
