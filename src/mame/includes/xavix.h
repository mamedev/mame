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
#include "bus/ekara/slot.h"
#include "machine/nvram.h"

#include "machine/xavix_mtrk_wheel.h"
#include "machine/xavix_madfb_ball.h"
#include "machine/xavix2002_io.h"
#include "machine/xavix_io.h"

class xavix_sound_device : public device_t, public device_sound_interface
{
public:
	xavix_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_regs_callback() { return m_readregs_cb.bind(); }
	auto read_samples_callback() { return m_readsamples_cb.bind(); }

	void enable_voice(int voice, bool update_only);
	void disable_voice(int voice);
	bool is_voice_enabled(int voice);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	sound_stream *m_stream;

	struct xavix_voice {
		bool enabled[2];
		uint32_t position[2];
		uint32_t startposition[2];
		uint8_t bank; // no samples appear to cross a bank boundary, so likely wraps
		int type;
		int rate;
		int vol;
	};

	devcb_read8 m_readregs_cb;

	devcb_read8 m_readsamples_cb;

	xavix_voice m_voice[16];
};

DECLARE_DEVICE_TYPE(XAVIX_SOUND, xavix_sound_device)


class xavix_state : public driver_device
{
public:
	xavix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_an_in(*this, "AN%u", 0U),
		m_mouse0x(*this, "MOUSE0X"),
		m_mouse0y(*this, "MOUSE0Y"),
		m_mouse1x(*this, "MOUSE1X"),
		m_mouse1y(*this, "MOUSE1Y"),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram"),
		m_screen(*this, "screen"),
		m_lowbus(*this, "lowbus"),
		m_sprite_xhigh_ignore_hack(true),
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
		m_sound(*this, "xavix_sound"),
		m_xavix2002io(*this, "xavix2002io")
	{ }

	void xavix(machine_config &config);
	void xavix_nv(machine_config &config);

	void xavixp(machine_config &config);
	void xavixp_nv(machine_config &config);

	void xavix2000(machine_config &config);
	void xavix2000_nv(machine_config &config);

	void xavix2002(machine_config &config);

	void init_xavix();

	DECLARE_WRITE_LINE_MEMBER(ioevent_trg01);
	DECLARE_WRITE_LINE_MEMBER(ioevent_trg02);
	DECLARE_WRITE_LINE_MEMBER(ioevent_trg04);
	DECLARE_WRITE_LINE_MEMBER(ioevent_trg08);

	int m_rgnlen;
	uint8_t* m_rgn;

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

protected:

	virtual uint8_t read_io0(uint8_t direction);
	virtual uint8_t read_io1(uint8_t direction);
	virtual void write_io0(uint8_t data, uint8_t direction);
	virtual void write_io1(uint8_t data, uint8_t direction);
	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport_array<8> m_an_in;
	optional_ioport m_mouse0x;
	optional_ioport m_mouse0y;
	optional_ioport m_mouse1x;
	optional_ioport m_mouse1y;
	required_device<xavix_device> m_maincpu;
	optional_device<nvram_device> m_nvram;
	required_device<screen_device> m_screen;
	required_device<address_map_bank_device> m_lowbus;
	address_space* m_cpuspace;

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

	DECLARE_WRITE8_MEMBER(debug_mem_w)
	{
		m_mainram[offset] = data;
	};

	virtual uint8_t opcodes_000000_r(offs_t offset)
	{
		if (offset & 0x8000)
		{
			return m_rgn[(offset) & (m_rgnlen - 1)];
		}
		else
		{
			return m_lowbus->read8(offset & 0x7fff);
		}
	}

	virtual uint8_t opcodes_800000_r(offs_t offset)
	{
		// rad_fb, rad_madf confirm that for >0x800000 the CPU only sees ROM when executing opcodes
		return m_rgn[(offset) & (m_rgnlen - 1)];
	}

	virtual uint8_t extbus_r(offs_t offset) { return m_rgn[(offset) & (m_rgnlen - 1)]; }
	virtual void extbus_w(offs_t offset, uint8_t data)
	{
		logerror("%s: write to external bus %06x %02x\n", machine().describe_context(), offset, data);
	}


	uint8_t sample_read(offs_t offset)
	{
		return read_full_data_sp_bypass(offset);
	};

	virtual inline uint8_t read_full_data_sp_bypass(uint32_t adr)
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


	DECLARE_READ8_MEMBER(ioevent_enable_r);
	DECLARE_WRITE8_MEMBER(ioevent_enable_w);
	DECLARE_READ8_MEMBER(ioevent_irqstate_r);
	DECLARE_WRITE8_MEMBER(ioevent_irqack_w);
	uint8_t m_ioevent_enable;
	uint8_t m_ioevent_active;
	void process_ioevent(uint8_t bits);

	DECLARE_READ8_MEMBER(mouse_7b00_r);
	DECLARE_READ8_MEMBER(mouse_7b01_r);
	DECLARE_READ8_MEMBER(mouse_7b10_r);
	DECLARE_READ8_MEMBER(mouse_7b11_r);

	DECLARE_WRITE8_MEMBER(mouse_7b00_w);
	DECLARE_WRITE8_MEMBER(mouse_7b01_w);
	DECLARE_WRITE8_MEMBER(mouse_7b10_w);
	DECLARE_WRITE8_MEMBER(mouse_7b11_w);

	DECLARE_READ8_MEMBER(adc_7b80_r);
	DECLARE_WRITE8_MEMBER(adc_7b80_w);
	DECLARE_READ8_MEMBER(adc_7b81_r);
	DECLARE_WRITE8_MEMBER(adc_7b81_w);
	TIMER_CALLBACK_MEMBER(adc_timer_done);
	emu_timer *m_adc_timer;
	uint8_t m_adc_control;

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

	uint8_t m_adc_inlatch;

	DECLARE_READ8_MEMBER(nmi_vector_lo_r);
	DECLARE_READ8_MEMBER(nmi_vector_hi_r);
	DECLARE_READ8_MEMBER(irq_vector_lo_r);
	DECLARE_READ8_MEMBER(irq_vector_hi_r);

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

	DECLARE_READ8_MEMBER(sound_startstop_r);
	DECLARE_WRITE8_MEMBER(sound_startstop_w);
	DECLARE_READ8_MEMBER(sound_updateenv_r);
	DECLARE_WRITE8_MEMBER(sound_updateenv_w);

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

	TIMER_CALLBACK_MEMBER(sound_timer_done);
	emu_timer *m_sound_timer[4];


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
	DECLARE_WRITE8_MEMBER(spriteram_w);
	bool m_sprite_xhigh_ignore_hack;

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

	uint8_t m_barrel_params[2];

	DECLARE_READ8_MEMBER(barrel_r);
	DECLARE_WRITE8_MEMBER(barrel_w);


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

	uint8_t m_mastervol;
	uint8_t m_unk_snd75f8;
	uint8_t m_unk_snd75f9;
	uint8_t m_unk_snd75ff;
	uint8_t m_sndtimer[4];

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
	void decode_inline_header(int &flipx, int &flipy, int &test, int& pal, int debug_packets);

	bitmap_ind16 m_zbuffer;

	uint8_t m_spritereg;

	// variables used by rendering
	int m_tmp_dataaddress;
	int m_tmp_databit;
	uint8_t m_bit;

	void set_data_address(int address, int bit);
	uint8_t get_next_bit();
	uint8_t get_next_byte();

	int get_current_address_byte();

	required_device<xavix_sound_device> m_sound;


	DECLARE_READ8_MEMBER(sound_regram_read_cb);

protected:
	optional_device<xavix2002_io_device> m_xavix2002io;

	uint8_t m_extbusctrl[3];

	virtual DECLARE_READ8_MEMBER(extintrf_790x_r);
	virtual DECLARE_WRITE8_MEMBER(extintrf_790x_w);

	// additional SuperXaviX / XaviX2002 stuff
	uint8_t m_sx_extended_extbus[3];

	DECLARE_WRITE8_MEMBER(extended_extbus_reg0_w);
	DECLARE_WRITE8_MEMBER(extended_extbus_reg1_w);
	DECLARE_WRITE8_MEMBER(extended_extbus_reg2_w);
};

class xavix_i2c_state : public xavix_state
{
public:
	xavix_i2c_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag),
		m_i2cmem(*this, "i2cmem"),
		hackaddress1(-1),
		hackaddress2(-1)
	{ }

	void xavix_i2c_24lc02(machine_config &config);
	void xavix_i2c_24lc04(machine_config &config);
	void xavix_i2c_24c02(machine_config &config);
	void xavix_i2c_24c08(machine_config &config);

	void xavix2000_i2c_24c04(machine_config &config);
	void xavix2000_i2c_24c02(machine_config &config);

	void xavix2002_i2c_24c04(machine_config &config);

	void init_epo_efdx()
	{
		init_xavix();
		hackaddress1 = 0x958a;
		hackaddress2 = 0x8524;
	}

	DECLARE_CUSTOM_INPUT_MEMBER(i2c_r);

protected:
	virtual void write_io1(uint8_t data, uint8_t direction) override;

	required_device<i2cmem_device> m_i2cmem;

private:
	int hackaddress1;
	int hackaddress2;
};

class xavix_i2c_ltv_tam_state : public xavix_i2c_state
{
public:
	xavix_i2c_ltv_tam_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
	{ }

private:
	virtual void write_io1(uint8_t data, uint8_t direction) override;
};


class xavix_i2c_jmat_state : public xavix_i2c_state
{
public:
	xavix_i2c_jmat_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
	{ }

	void xavix2002_i2c_jmat(machine_config &config);

private:
	READ8_MEMBER(read_extended_io0);
	READ8_MEMBER(read_extended_io1);
	READ8_MEMBER(read_extended_io2);
	WRITE8_MEMBER(write_extended_io0);
	WRITE8_MEMBER(write_extended_io1);
	WRITE8_MEMBER(write_extended_io2);
};


class xavix_i2c_lotr_state : public xavix_i2c_state
{
public:
	xavix_i2c_lotr_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
	{ }

	DECLARE_CUSTOM_INPUT_MEMBER(camera_r);

protected:
	//virtual void write_io1(uint8_t data, uint8_t direction) override;
};

class xavix_i2c_bowl_state : public xavix_i2c_state
{
public:
	xavix_i2c_bowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
	{ }

	DECLARE_CUSTOM_INPUT_MEMBER(camera_r);
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


class xavix_cart_state : public xavix_state
{
public:
	xavix_cart_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag),
		m_cartslot(*this, "cartslot")
	{ }

	void xavix_cart(machine_config &config);
	void xavix_cart_ekara(machine_config &config);
	void xavix_cart_popira(machine_config &config);
	void xavix_cart_ddrfammt(machine_config &config);

protected:

	// for Cart cases this memory bypass becomes more complex

	virtual uint8_t opcodes_000000_r(offs_t offset) override
	{
		if (offset & 0x8000)
		{
			if (offset & 0x400000)
			{
				return m_rgn[(offset) & (m_rgnlen - 1)];
			}
			else
			{
				if (m_cartslot->has_cart())
				{
					return m_cartslot->read_cart(*m_cpuspace, offset);
				}
				else
				{
					return m_rgn[(offset) & (m_rgnlen - 1)];
				}
			}
		}
		else
		{
			return m_lowbus->read8(offset & 0x7fff);
		}
	}

	virtual uint8_t opcodes_800000_r(offs_t offset) override
	{
		if (offset & 0x400000)
		{
			return m_rgn[(offset) & (m_rgnlen - 1)];
		}
		else
		{
			if (m_cartslot->has_cart())
			{
				return m_cartslot->read_cart(*m_cpuspace, offset);
			}
			else
			{
				return m_rgn[(offset) & (m_rgnlen - 1)];
			}
		}
	}

	// TODO, use callbacks?
	virtual DECLARE_READ8_MEMBER(extintrf_790x_r) override
	{
		return xavix_state::extintrf_790x_r(space,offset,mem_mask);
	}

	virtual DECLARE_WRITE8_MEMBER(extintrf_790x_w) override
	{
		xavix_state::extintrf_790x_w(space,offset,data, mem_mask);

		if (offset < 3)
		{
			if (m_cartslot->has_cart())
				m_cartslot->write_bus_control(space,offset,data,mem_mask);
		}
	};

	virtual uint8_t extbus_r(offs_t offset) override
	{
		if (m_cartslot->has_cart() && m_cartslot->is_read_access_not_rom())
		{
			logerror("%s: read from external bus %06x (SEEPROM READ?)\n", machine().describe_context(), offset);
			return m_cartslot->read_extra(*m_cpuspace, offset);
		}
		else
		{
			if (offset & 0x400000)
			{
				return m_rgn[(offset) & (m_rgnlen - 1)];
			}
			else
			{
				if (m_cartslot->has_cart())
				{
					return m_cartslot->read_cart(*m_cpuspace, offset);
				}
				else
				{
					return m_rgn[(offset) & (m_rgnlen - 1)];
				}
			}
		}
	}
	virtual void extbus_w(offs_t offset, uint8_t data) override
	{
		if (m_cartslot->has_cart() && m_cartslot->is_write_access_not_rom())
		{
			logerror("%s: write to external bus %06x %02x (SEEPROM WRITE?)\n", machine().describe_context(), offset, data);
			return m_cartslot->write_extra(*m_cpuspace, offset, data);
		}
		else
		{
			if (m_cartslot->has_cart())
			{
				return m_cartslot->write_cart(*m_cpuspace, offset, data);
			}
			else
			{
				logerror("%s: write to external bus %06x %02x\n", machine().describe_context(), offset, data);
			}
		}
	}

	virtual inline uint8_t read_full_data_sp_bypass(uint32_t offset) override
	{
		uint8_t databank = offset >> 16;

		if (databank >= 0x80)
		{
			if (offset & 0x400000)
			{
				return m_rgn[(offset) & (m_rgnlen - 1)];
			}
			else
			{
				if (m_cartslot->has_cart())
				{
					return m_cartslot->read_cart(*m_cpuspace, offset);
				}
				else
				{
					return m_rgn[(offset) & (m_rgnlen - 1)];
				}
			}
		}
		else
		{
			if ((offset & 0xffff) >= 0x8000)
			{
				if (offset & 0x400000)
				{
					return m_rgn[(offset) & (m_rgnlen - 1)];
				}
				else
				{
					if (m_cartslot->has_cart())
					{
						return m_cartslot->read_cart(*m_cpuspace, offset);
					}
					else
					{
						return m_rgn[(offset) & (m_rgnlen - 1)];
					}
				}
			}
			else
			{
				return read_full_data_sp_lowbus_bypass(offset);
			}
		}
	}

	required_device<ekara_cart_slot_device> m_cartslot;
};

class xavix_i2c_cart_state : public xavix_cart_state
{
public:
	xavix_i2c_cart_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_cart_state(mconfig,type,tag),
		m_i2cmem(*this, "i2cmem")
	{ }

	void xavix_i2c_taiko(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(i2c_r);

protected:
	virtual void write_io1(uint8_t data, uint8_t direction) override;

	required_device<i2cmem_device> m_i2cmem;
};

class xavix_popira2_cart_state : public xavix_cart_state
{
public:
	xavix_popira2_cart_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_cart_state(mconfig,type,tag)
	{ }

	DECLARE_CUSTOM_INPUT_MEMBER(i2c_r);

protected:
	virtual void write_io1(uint8_t data, uint8_t direction) override;

};


class xavix_ekara_state : public xavix_cart_state
{
public:
	xavix_ekara_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_cart_state(mconfig, type, tag),
		m_extra0(*this, "EXTRA0"),
		m_extra1(*this, "EXTRA1"),
		m_extraioselect(0),
		m_extraiowrite(0)
	{ }

	DECLARE_CUSTOM_INPUT_MEMBER(ekara_multi0_r);
	DECLARE_CUSTOM_INPUT_MEMBER(ekara_multi1_r);

//  void xavix_ekara(machine_config &config);

protected:

	required_ioport m_extra0;
	required_ioport m_extra1;

	virtual void write_io0(uint8_t data, uint8_t direction) override;
	virtual void write_io1(uint8_t data, uint8_t direction) override;

	uint8_t m_extraioselect;
	uint8_t m_extraiowrite;
};


#endif // MAME_INCLUDES_XAVIX_H
