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
#include "machine/xavix_adc.h"
#include "machine/xavix_anport.h"
#include "machine/xavix_math.h"

// NTSC clock for regular XaviX?
#define MAIN_CLOCK XTAL(21'477'272)
// some games (eg Radica Opus) run off a 3.579545MHz XTAL ( same as the above /6 ) so presumably there is a divider / multiplier circuit on some PCBs?
// TODO: what's the PAL clock?


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
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

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
		m_adc(*this, "adc"),
		m_anport(*this, "anport"),
		m_math(*this, "math"),
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
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

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

	void debug_mem_w(offs_t offset, uint8_t data)
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


	uint8_t ioevent_enable_r();
	void ioevent_enable_w(uint8_t data);
	uint8_t ioevent_irqstate_r();
	void ioevent_irqack_w(uint8_t data);
	uint8_t m_ioevent_enable;
	uint8_t m_ioevent_active;
	void process_ioevent(uint8_t bits);

	void slotreg_7810_w(uint8_t data);

	void rom_dmatrg_w(uint8_t data);

	void rom_dmasrc_w(offs_t offset, uint8_t data);

	void rom_dmadst_w(offs_t offset, uint8_t data);
	void rom_dmalen_w(offs_t offset, uint8_t data);
	uint8_t rom_dmastat_r();

	void spritefragment_dma_params_1_w(offs_t offset, uint8_t data);
	void spritefragment_dma_params_2_w(offs_t offset, uint8_t data);
	void spritefragment_dma_trg_w(uint8_t data);
	uint8_t spritefragment_dma_status_r();

	uint8_t io0_data_r();
	uint8_t io1_data_r();
	void io0_data_w(uint8_t data);
	void io1_data_w(uint8_t data);

	uint8_t io0_direction_r();
	uint8_t io1_direction_r();
	void io0_direction_w(uint8_t data);
	void io1_direction_w(uint8_t data);

	uint8_t m_io0_data;
	uint8_t m_io1_data;
	uint8_t m_io0_direction;
	uint8_t m_io1_direction;

	uint8_t nmi_vector_lo_r();
	uint8_t nmi_vector_hi_r();
	uint8_t irq_vector_lo_r();
	uint8_t irq_vector_hi_r();

	void vector_enable_w(uint8_t data);
	void nmi_vector_lo_w(uint8_t data);
	void nmi_vector_hi_w(uint8_t data);
	void irq_vector_lo_w(uint8_t data);
	void irq_vector_hi_w(uint8_t data);

	uint8_t irq_source_r();
	void irq_source_w(uint8_t data);

	uint8_t arena_start_r();
	void arena_start_w(uint8_t data);
	uint8_t arena_end_r();
	void arena_end_w(uint8_t data);
	uint8_t arena_control_r();
	void arena_control_w(uint8_t data);

	uint8_t colmix_6ff0_r();
	void colmix_6ff0_w(uint8_t data);

	void colmix_6ff1_w(uint8_t data);
	void colmix_6ff2_w(uint8_t data);

	uint8_t dispctrl_6ff8_r();
	void dispctrl_6ff8_w(uint8_t data);

	uint8_t sound_startstop_r(offs_t offset);
	void sound_startstop_w(offs_t offset, uint8_t data);
	uint8_t sound_updateenv_r(offs_t offset);
	void sound_updateenv_w(offs_t offset, uint8_t data);

	uint8_t sound_sta16_r(offs_t offset);
	uint8_t sound_75f5_r();
	uint8_t sound_volume_r();
	void sound_volume_w(uint8_t data);

	void sound_regbase_w(uint8_t data);

	uint8_t sound_75f8_r();
	void sound_75f8_w(uint8_t data);

	uint8_t sound_75f9_r();
	void sound_75f9_w(uint8_t data);

	uint8_t sound_timer0_r();
	void sound_timer0_w(uint8_t data);
	uint8_t sound_timer1_r();
	void sound_timer1_w(uint8_t data);
	uint8_t sound_timer2_r();
	void sound_timer2_w(uint8_t data);
	uint8_t sound_timer3_r();
	void sound_timer3_w(uint8_t data);

	uint8_t sound_irqstatus_r();
	void sound_irqstatus_w(uint8_t data);
	void sound_75ff_w(uint8_t data);
	uint8_t m_sound_irqstatus;
	uint8_t m_soundreg16_0[2];
	uint8_t m_soundreg16_1[2];
	uint8_t m_sound_regbase;

	TIMER_CALLBACK_MEMBER(sound_timer_done);
	emu_timer *m_sound_timer[4];


	uint8_t timer_status_r();
	void timer_control_w(uint8_t data);
	uint8_t timer_baseval_r();
	void timer_baseval_w(uint8_t data);
	uint8_t timer_freq_r();
	void timer_freq_w(uint8_t data);
	uint8_t timer_curval_r();
	uint8_t m_timer_control;
	uint8_t m_timer_freq;
	TIMER_CALLBACK_MEMBER(freq_timer_done);
	emu_timer *m_freq_timer;

	void palram_sh_w(offs_t offset, uint8_t data);
	void palram_l_w(offs_t offset, uint8_t data);
	void colmix_sh_w(offs_t offset, uint8_t data);
	void colmix_l_w(offs_t offset, uint8_t data);
	void bmp_palram_sh_w(offs_t offset, uint8_t data);
	void bmp_palram_l_w(offs_t offset, uint8_t data);
	void spriteram_w(offs_t offset, uint8_t data);
	bool m_sprite_xhigh_ignore_hack;

	void tmap1_regs_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	void tmap2_regs_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t tmap1_regs_r(offs_t offset);
	uint8_t tmap2_regs_r(offs_t offset);

	void spriteregs_w(uint8_t data);

	uint8_t pal_ntsc_r();

	uint8_t xavix_memoryemu_txarray_r(offs_t offset);
	void xavix_memoryemu_txarray_w(offs_t offset, uint8_t data);
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


	uint8_t adc0_r() { return m_an_in[0]->read(); }
	uint8_t adc1_r() { return m_an_in[1]->read(); }
	uint8_t adc2_r() { return m_an_in[2]->read(); }
	uint8_t adc3_r() { return m_an_in[3]->read(); }
	uint8_t adc4_r() { return m_an_in[4]->read(); }
	uint8_t adc5_r() { return m_an_in[5]->read(); }
	uint8_t adc6_r() { return m_an_in[6]->read(); }
	uint8_t adc7_r() { return m_an_in[7]->read(); }

	uint8_t anport0_r() { logerror("%s: unhandled anport0_r\n", machine().describe_context()); return 0xff; }
	uint8_t anport1_r() { logerror("%s: unhandled anport1_r\n", machine().describe_context()); return 0xff; }
	uint8_t anport2_r() { logerror("%s: unhandled anport2_r\n", machine().describe_context()); return 0xff; }
	uint8_t anport3_r() { logerror("%s: unhandled anport3_r\n", machine().describe_context()); return 0xff; }

	void update_irqs();
	uint8_t m_irqsource;

	uint8_t m_vectorenable;
	uint8_t m_nmi_vector_lo_data;
	uint8_t m_nmi_vector_hi_data;
	uint8_t m_irq_vector_lo_data;
	uint8_t m_irq_vector_hi_data;

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
	void dispctrl_posirq_x_w(uint8_t data);
	void dispctrl_posirq_y_w(uint8_t data);

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
	void draw_tile_line(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int tile, int bpp, int xpos, int ypos, int drawheight, int drawwidth, int flipx, int flipy, int pal, int zval, int line);
	void draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int which);
	void draw_tilemap_line(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int which, int line);
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_sprites_line(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int line);
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


	uint8_t sound_regram_read_cb(offs_t offset);

protected:
	required_device<xavix_adc_device> m_adc;
	required_device<xavix_anport_device> m_anport;
	required_device<xavix_math_device> m_math;
	optional_device<xavix2002_io_device> m_xavix2002io;

	uint8_t m_extbusctrl[3];

	virtual uint8_t extintrf_790x_r(offs_t offset);
	virtual void extintrf_790x_w(offs_t offset, uint8_t data);

	// additional SuperXaviX / XaviX2002 stuff
	uint8_t m_sx_extended_extbus[3];

	void extended_extbus_reg0_w(uint8_t data);
	void extended_extbus_reg1_w(uint8_t data);
	void extended_extbus_reg2_w(uint8_t data);
};

class xavix_guru_state : public xavix_state
{
public:
	xavix_guru_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
	{ }

	void xavix_guru(machine_config &config);

protected:

private:
	uint8_t guru_anport2_r() { uint8_t ret = m_mouse1x->read()-0x10; return ret; }
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

	void xavix_i2c_24lc04(machine_config &config);
	void xavix_i2c_24c02(machine_config &config);
	void xavix_i2c_24c08(machine_config &config);

	void xavix2000_i2c_24c04(machine_config &config);
	void xavix2000_i2c_24c02(machine_config &config);

	void xavix2002_i2c_24c04(machine_config &config);
	void xavix2002_i2c_mrangbat(machine_config& config);

	void init_epo_efdx()
	{
		init_xavix();
		hackaddress1 = 0x958a;
		hackaddress2 = 0x8524;
	}

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

	void xavix_i2c_24lc04_tam(machine_config &config);

private:
	virtual void write_io1(uint8_t data, uint8_t direction) override;

private:
	uint8_t tam_anport0_r() { return m_mouse0x->read()^0x7f; }
	uint8_t tam_anport1_r() { return m_mouse0y->read()^0x7f; }
	uint8_t tam_anport2_r() { return m_mouse1x->read()^0x7f; }
	uint8_t tam_anport3_r() { return m_mouse1y->read()^0x7f; }
};



class xavix_i2c_lotr_state : public xavix_i2c_state
{
public:
	xavix_i2c_lotr_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
	{ }

	DECLARE_READ_LINE_MEMBER(camera_r);

protected:
	//virtual void write_io1(uint8_t data, uint8_t direction) override;
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

	DECLARE_READ_LINE_MEMBER( mtrk_wheel_r );

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
	xavix_cart_state(const machine_config &mconfig, device_type type, const char *tag) :
		xavix_state(mconfig, type, tag),
		m_cartslot(*this, "cartslot")
	{
		m_cartlimit = 0x400000;
	}

	void xavix_cart(machine_config &config);
	void xavix_cart_ekara(machine_config &config);
	void xavix_cart_popira(machine_config &config);
	void xavix_cart_ddrfammt(machine_config &config);
	void xavix_cart_evio(machine_config &config);

protected:

	// for Cart cases this memory bypass becomes more complex

	virtual uint8_t opcodes_000000_r(offs_t offset) override
	{
		if (offset & 0x8000)
		{
			if ((offset & 0x7fffff) >= m_cartlimit)
			{
				return m_rgn[(offset) & (m_rgnlen - 1)];
			}
			else
			{
				if (m_cartslot->has_cart())
				{
					return m_cartslot->read_cart(offset);
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
		if ((offset & 0x7fffff) >= m_cartlimit)
		{
			return m_rgn[(offset) & (m_rgnlen - 1)];
		}
		else
		{
			if (m_cartslot->has_cart())
			{
				return m_cartslot->read_cart(offset);
			}
			else
			{
				return m_rgn[(offset) & (m_rgnlen - 1)];
			}
		}
	}

	// TODO, use callbacks?
	virtual uint8_t extintrf_790x_r(offs_t offset) override
	{
		return xavix_state::extintrf_790x_r(offset);
	}

	virtual void extintrf_790x_w(offs_t offset, uint8_t data) override
	{
		xavix_state::extintrf_790x_w(offset,data);

		if (offset < 3)
		{
			if (m_cartslot->has_cart())
				m_cartslot->write_bus_control(offset,data);
		}
	};

	virtual uint8_t extbus_r(offs_t offset) override
	{
		if (m_cartslot->has_cart() && m_cartslot->is_read_access_not_rom())
		{
			logerror("%s: read from external bus %06x (SEEPROM READ?)\n", machine().describe_context(), offset);
			return m_cartslot->read_extra(offset);
		}
		else
		{
			if ((offset & 0x7fffff) >= m_cartlimit)
			{
				return m_rgn[(offset) & (m_rgnlen - 1)];
			}
			else
			{
				if (m_cartslot->has_cart())
				{
					return m_cartslot->read_cart(offset);
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
			return m_cartslot->write_extra(offset, data);
		}
		else
		{
			if (m_cartslot->has_cart())
			{
				return m_cartslot->write_cart(offset, data);
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
			if ((offset & 0x7fffff) >= m_cartlimit)
			{
				return m_rgn[(offset) & (m_rgnlen - 1)];
			}
			else
			{
				if (m_cartslot->has_cart())
				{
					return m_cartslot->read_cart(offset);
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
				if ((offset & 0x7fffff) >= m_cartlimit)
				{
					return m_rgn[(offset) & (m_rgnlen - 1)];
				}
				else
				{
					if (m_cartslot->has_cart())
					{
						return m_cartslot->read_cart(offset);
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

	int m_cartlimit;
	required_device<ekara_cart_slot_device> m_cartslot;
};


class xavix_cart_gcslottv_state : public xavix_cart_state
{
public:
	xavix_cart_gcslottv_state(const machine_config &mconfig, device_type type, const char *tag) :
		xavix_cart_state(mconfig, type, tag)
	{
		m_cartlimit = 0x800000;
	}

	void xavix_cart_gcslottv(machine_config &config);

protected:
};

class xavix_i2c_cart_state : public xavix_cart_state
{
public:
	xavix_i2c_cart_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_cart_state(mconfig,type,tag),
		m_i2cmem(*this, "i2cmem")
	{ }

	void xavix_i2c_taiko(machine_config &config);
	void xavix_i2c_jpopira(machine_config &config);

protected:
	virtual void write_io1(uint8_t data, uint8_t direction) override;

	required_device<i2cmem_device> m_i2cmem;
};

class xavix_popira2_cart_state : public xavix_cart_state
{
public:
	xavix_popira2_cart_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_cart_state(mconfig,type,tag),
		m_p2(*this, "P2")
	{ }

	void xavix_cart_popira2(machine_config &config);

	DECLARE_READ_LINE_MEMBER(i2c_r);

protected:
	virtual void write_io1(uint8_t data, uint8_t direction) override;

private:
	uint8_t popira2_adc0_r();
	uint8_t popira2_adc1_r();

	required_ioport m_p2;
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

	DECLARE_READ_LINE_MEMBER(ekara_multi0_r);
	DECLARE_READ_LINE_MEMBER(ekara_multi1_r);

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
