// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_TVGAMES_XAVIX_H
#define MAME_TVGAMES_XAVIX_H

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

#include "xavix_mtrk_wheel.h"
#include "xavix_madfb_ball.h"
#include "xavix2002_io.h"
#include "xavix_io.h"
#include "xavix_adc.h"
#include "xavix_anport.h"
#include "xavix_math.h"

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
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_stream = nullptr;

	struct xavix_voice {
		bool enabled[2]{};
		uint32_t position[2]{};
		uint32_t startposition[2]{};
		uint8_t bank = 0; // no samples appear to cross a bank boundary, so likely wraps
		int type = 0;
		int rate = 0;
		int vol = 0;
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
		m_lightgun(*this, "GUN1_%u", 0U),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram"),
		m_screen(*this, "screen"),
		m_lowbus(*this, "lowbus"),
		m_sprite_xhigh_ignore_hack(true),
		m_mainram(*this, "mainram"),
		m_fragment_sprite(*this, "fragment_sprite"),
		m_rom_dma_src(*this, "rom_dma_src"),
		m_rom_dma_dst(*this, "rom_dma_dst"),
		m_rom_dma_len(*this, "rom_dma_len"),
		m_palram_sh(*this, "palram_sh"),
		m_palram_l(*this, "palram_l"),
		m_colmix_sh(*this, "colmix_sh"),
		m_colmix_l(*this, "colmix_l"),
		m_colmix_ctrl(*this, "colmix_ctrl"),
		m_posirq_x(*this, "posirq_x"),
		m_posirq_y(*this, "posirq_y"),
		m_segment_regs(*this, "segment_regs"),
		m_ext_segment_regs(*this, "ext_segment_regs"),
		m_palette(*this, "palette"),
		m_region(*this, "REGION"),
		m_gfxdecode(*this, "gfxdecode"),
		m_sound(*this, "xavix_sound"),
		m_adc(*this, "adc"),
		m_anport(*this, "anport"),
		m_math(*this, "math")
	{
		m_video_hres_multiplier = 1;
	}

	void xavix(machine_config &config);
	void xavix_nv(machine_config &config);
	void xavix_1mb_nv(machine_config &config);
	void xavix_2mb_nv(machine_config &config);
	void xavix_4mb_nv(machine_config &config);
	void xavix_4mb(machine_config &config);
	void xavix_2mb(machine_config &config);
	void xavix_1mb(machine_config &config);

	void xavixp(machine_config &config);
	void xavixp_nv(machine_config &config);
	void xavixp_1mb_nv(machine_config &config);
	void xavixp_4mb(machine_config &config);
	void xavixp_2mb(machine_config &config);
	void xavixp_1mb(machine_config &config);

	void xavix2000(machine_config &config);
	void xavix2000_4mb(machine_config &config);
	void xavix2000_nv(machine_config &config);
	void xavix2000_4mb_nv(machine_config &config);

	void xavix_43mhz(machine_config &config);

	void init_xavix();
	void init_no_timer() { init_xavix(); m_disable_timer_irq_hack = true; }

	void ioevent_trg01(int state);
	void ioevent_trg02(int state);
	void ioevent_trg04(int state);
	void ioevent_trg08(int state);

	virtual void xavix_interrupt_extra() { }

	int m_rgnlen = 0;
	uint8_t* m_rgn = nullptr;

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

	int unknown_random_r()
	{
		if (!machine().side_effects_disabled())
			return machine().rand();
		else
			return 0;
	}

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual uint8_t read_io0(uint8_t direction);
	virtual uint8_t read_io1(uint8_t direction);
	virtual void write_io0(uint8_t data, uint8_t direction);
	virtual void write_io1(uint8_t data, uint8_t direction);

	void set_xavix_cpumaps(machine_config &config);

	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport_array<8> m_an_in;
	optional_ioport m_mouse0x;
	optional_ioport m_mouse0y;
	optional_ioport m_mouse1x;
	optional_ioport m_mouse1y;
	optional_ioport_array<2> m_lightgun;
	required_device<xavix_device> m_maincpu;
	optional_device<nvram_device> m_nvram;
	required_device<screen_device> m_screen;
	required_device<address_map_bank_device> m_lowbus;
	address_space* m_cpuspace = nullptr;

	bool m_disable_timer_irq_hack = false; // hack for epo_mini which floods timer IRQs to the point it won't do anything else

	virtual void xavix_extbus_map(address_map &map) ATTR_COLD;

	void xavix_4mb_extbus_map(address_map &map) ATTR_COLD;
	void xavix_2mb_extbus_map(address_map &map) ATTR_COLD;
	void xavix_1mb_extbus_map(address_map &map) ATTR_COLD;

	// screen updates
	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void xavix_map(address_map &map) ATTR_COLD;

	void xavix_lowbus_map(address_map &map) ATTR_COLD;

	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);


	virtual void video_start() override ATTR_COLD;

	void debug_mem_w(offs_t offset, uint8_t data)
	{
		m_mainram[offset] = data;
	};

	virtual uint8_t opcodes_000000_r(offs_t offset)
	{
		if (offset & 0x8000)
		{
			if (m_disable_memory_bypass)
			{
				return m_maincpu->space(6).read_byte(offset & 0x7fffff);
			}
			else
			{
				return m_rgn[(offset) & (m_rgnlen - 1)];
			}
		}
		else
		{
			return m_lowbus->read8(offset & 0x7fff);
		}
	}

	virtual uint8_t opcodes_800000_r(offs_t offset)
	{
		if (m_disable_memory_bypass)
		{
			return m_maincpu->space(6).read_byte(offset & 0x7fffff);
		}
		else
		{
			// rad_fb, rad_madf confirm that for >0x800000 the CPU only sees ROM when executing opcodes
			return m_rgn[(offset) & (m_rgnlen - 1)];
		}
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
	uint8_t m_ioevent_enable = 0;
	uint8_t m_ioevent_active = 0;
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

	uint8_t m_io0_data = 0;
	uint8_t m_io1_data = 0;
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
	uint8_t m_sound_irqstatus = 0;
	uint8_t m_soundreg16_0[2]{};
	uint8_t m_soundreg16_1[2]{};
	uint8_t m_sound_regbase = 0;

	TIMER_CALLBACK_MEMBER(sound_timer_done);
	emu_timer *m_sound_timer[4]{};


	uint8_t timer_status_r();
	void timer_control_w(uint8_t data);
	uint8_t timer_baseval_r();
	void timer_baseval_w(uint8_t data);
	uint8_t timer_freq_r();
	void timer_freq_w(uint8_t data);
	uint8_t timer_curval_r();
	uint8_t m_timer_control = 0;
	uint8_t m_timer_freq = 0;
	TIMER_CALLBACK_MEMBER(freq_timer_done);
	emu_timer *m_freq_timer = nullptr;

	void palram_sh_w(offs_t offset, uint8_t data);
	void palram_l_w(offs_t offset, uint8_t data);
	void colmix_sh_w(offs_t offset, uint8_t data);
	void colmix_l_w(offs_t offset, uint8_t data);
	void spriteram_w(offs_t offset, uint8_t data);
	void mainram_w(offs_t offset, uint8_t data);
	bool m_sprite_xhigh_ignore_hack;

	void tmap1_regs_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	void tmap2_regs_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t tmap1_regs_r(offs_t offset);
	uint8_t tmap2_regs_r(offs_t offset);

	void spriteregs_w(uint8_t data);

	uint8_t pal_ntsc_r();

	virtual uint8_t lightgun_r(offs_t offset) { logerror("%s: unhandled lightgun_r %d\n", machine().describe_context(), offset); return 0xff;  }

	uint8_t xavix_memoryemu_txarray_r(offs_t offset);
	void xavix_memoryemu_txarray_w(offs_t offset, uint8_t data);
	uint8_t m_txarray[3]{};

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
			return (((~offset >> 4) & 0x0f) | (offset << 4));
		}
		else if (offset < 0x400)
		{
			offset &= 0xff;
			return (((~offset >> 4) & 0x0f) | (~offset << 4));
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
	uint8_t m_irqsource = 0;

	uint8_t m_vectorenable = 0;
	uint8_t m_nmi_vector_lo_data = 0;
	uint8_t m_nmi_vector_hi_data = 0;
	uint8_t m_irq_vector_lo_data = 0;
	uint8_t m_irq_vector_hi_data = 0;

	uint8_t m_spritefragment_dmaparam1[2]{};
	uint8_t m_spritefragment_dmaparam2[2]{};

	uint8_t m_tmap1_regs[8]{};
	uint8_t m_tmap2_regs[8]{};

	int m_arena_start = 0;
	int m_arena_end = 0;
	uint8_t m_arena_control = 0;

	uint8_t m_6ff0 = 0;
	uint8_t m_video_ctrl = 0;

	uint8_t m_mastervol = 0;
	uint8_t m_unk_snd75f8 = 0;
	uint8_t m_unk_snd75f9 = 0;
	uint8_t m_unk_snd75ff = 0;
	uint8_t m_sndtimer[4]{};

	uint8_t m_timer_baseval = 0;

	int16_t get_vectors(int which, int half);

	// raster IRQ
	TIMER_CALLBACK_MEMBER(interrupt_gen);
	emu_timer *m_interrupt_timer = nullptr;
	void dispctrl_posirq_x_w(uint8_t data);
	void dispctrl_posirq_y_w(uint8_t data);

	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_fragment_sprite;
	required_shared_ptr<uint8_t> m_rom_dma_src;
	required_shared_ptr<uint8_t> m_rom_dma_dst;
	required_shared_ptr<uint8_t> m_rom_dma_len;

	required_shared_ptr<uint8_t> m_palram_sh;
	required_shared_ptr<uint8_t> m_palram_l;

	required_shared_ptr<uint8_t> m_colmix_sh;
	required_shared_ptr<uint8_t> m_colmix_l;
	required_shared_ptr<uint8_t> m_colmix_ctrl;

	required_shared_ptr<uint8_t> m_posirq_x;
	required_shared_ptr<uint8_t> m_posirq_y;

	required_shared_ptr<uint8_t> m_segment_regs;
	optional_shared_ptr<uint8_t> m_ext_segment_regs;

	required_device<palette_device> m_palette;

	required_ioport m_region;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<xavix_sound_device> m_sound;
	required_device<xavix_adc_device> m_adc;
	required_device<xavix_anport_device> m_anport;
	required_device<xavix_math_device> m_math;

	uint8_t get_pen_lightness_from_dat(uint16_t dat);
	uint8_t get_pen_saturation_from_dat(uint16_t dat);
	uint8_t get_pen_hue_from_dat(uint16_t dat);
	uint16_t apply_pen_lightness_to_dat(uint16_t dat, uint16_t lightness);
	uint16_t apply_pen_saturation_to_dat(uint16_t dat, uint16_t saturation);
	uint16_t apply_pen_hue_to_dat(uint16_t dat, uint16_t hue);

	virtual void get_tile_pixel_dat(uint8_t& dat, int bpp);

	rectangle do_arena(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void update_pen(int pen, uint8_t shval, uint8_t lval);
	void draw_regular_layers(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &clip);
	virtual void draw_tile_line(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int tile, int bpp, int xpos, int ypos, int drawheight, int drawwidth, int flipx, int flipy, int pal, int zval, int line);
	virtual void draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int which);
	void draw_tilemap_line(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int which, int line);
	virtual void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_sprites_line(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int line);
	void decode_inline_header(int &flipx, int &flipy, int &test, int& pal, int debug_packets);

	bitmap_ind16 m_zbuffer;

	uint8_t m_spritereg = 0;

	// variables used by rendering
	int m_tmp_dataaddress = 0;
	int m_tmp_databit = 0;
	uint8_t m_bit = 0;

	void set_data_address(int address, int bit);
	uint8_t get_next_bit();
	uint8_t get_next_byte();

	int get_current_address_byte();

	uint8_t sound_regram_read_cb(offs_t offset);

	uint8_t m_extbusctrl[3]{};

	virtual uint8_t extintrf_790x_r(offs_t offset);
	virtual void extintrf_790x_w(offs_t offset, uint8_t data);

	bool m_disable_memory_bypass = false;
	bool m_disable_sprite_yflip = false;
	bool m_disable_tile_regs_flip = false;
	int m_video_hres_multiplier;
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

class superxavix_state : public xavix_state
{
public:
	superxavix_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_xavix2002io(*this, "xavix2002io")
		, m_sx_crtc_1(*this, "sx_crtc_1")
		, m_sx_crtc_2(*this, "sx_crtc_2")
		, m_sx_plt_loc(*this, "sx_plt_loc")
		, m_bmp_palram_sh(*this, "bmp_palram_sh")
		, m_bmp_palram_l(*this, "bmp_palram_l")
		, m_bmp_base(*this, "bmp_base")
		, m_extra(*this, "extra")
		, m_exio(*this, "EX%u", 0U)
	{
		m_video_hres_multiplier = 2;
	}

	void xavix2002(machine_config &config);
	void xavix2002_4mb(machine_config &config);

	void init_epo_doka();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

	void superxavix_lowbus_map(address_map &map) ATTR_COLD;

	required_device<xavix2002_io_device> m_xavix2002io;

	virtual void get_tile_pixel_dat(uint8_t &dat, int bpp) override;

private:
	void ext_segment_regs_w(offs_t offset, uint8_t data);
	void superxavix_plt_flush_w(uint8_t data);
	uint8_t superxavix_plt_dat_r();
	void superxavix_plt_dat_w(uint8_t data);
	void superxavix_plt_loc_w(offs_t offset, uint8_t data);
	uint8_t superxavix_plt_loc_r(offs_t offset);

	void superxavix_bitmap_pal_index_w(uint8_t data);
	uint8_t superxavix_bitmap_pal_index_r();
	void superxavix_chr_pal_index_w(uint8_t data);
	uint8_t superxavix_chr_pal_index_r();
	uint8_t superxavix_bitmap_pal_hue_r();
	uint8_t superxavix_bitmap_pal_saturation_r();
	uint8_t superxavix_bitmap_pal_lightness_r();
	uint8_t superxavix_chr_pal_hue_r();
	uint8_t superxavix_chr_pal_saturation_r();
	uint8_t superxavix_chr_pal_lightness_r();
	uint8_t superxavix_pal_hue_r(bool bitmap);
	uint8_t superxavix_pal_saturation_r(bool bitmap);
	uint8_t superxavix_pal_lightness_r(bool bitmap);
	void superxavix_bitmap_pal_hue_w(uint8_t data);
	void superxavix_bitmap_pal_saturation_w(uint8_t data);
	void superxavix_bitmap_pal_lightness_w(uint8_t data);
	void superxavix_chr_pal_hue_w(uint8_t data);
	void superxavix_chr_pal_saturation_w(uint8_t data);
	void superxavix_chr_pal_lightness_w(uint8_t data);
	void superxavix_pal_hue_w(uint8_t data, bool bitmap);
	void superxavix_pal_saturation_w(uint8_t data, bool bitmap);
	void superxavix_pal_lightness_w(uint8_t data, bool bitmap);

	uint8_t bitmap_params_r(offs_t offset);
	void bitmap_params_w(offs_t offset, uint8_t data);

	void superxavix_crtc_1_w(offs_t offset, uint8_t data);
	uint8_t superxavix_crtc_1_r(offs_t offset);
	void superxavix_crtc_2_w(offs_t offset, uint8_t data);
	uint8_t superxavix_crtc_2_r(offs_t offset);

	void bmp_palram_sh_w(offs_t offset, uint8_t data);
	void bmp_palram_l_w(offs_t offset, uint8_t data);

	void extended_extbus_reg0_w(uint8_t data);
	void extended_extbus_reg1_w(uint8_t data);
	void extended_extbus_reg2_w(uint8_t data);

	uint8_t superxavix_read_extended_io0(offs_t offset, uint8_t mem_mask) { logerror("%s: superxavix_read_extended_io0 (mask %02x)\n", machine().describe_context(), mem_mask); return m_exio[0]->read(); }
	uint8_t superxavix_read_extended_io1(offs_t offset, uint8_t mem_mask) { logerror("%s: superxavix_read_extended_io1 (mask %02x)\n", machine().describe_context(), mem_mask); return m_exio[1]->read(); }
	uint8_t superxavix_read_extended_io2(offs_t offset, uint8_t mem_mask) { logerror("%s: superxavix_read_extended_io2 (mask %02x)\n", machine().describe_context(), mem_mask); return m_exio[2]->read(); }

	void superxavix_write_extended_io0(offs_t offset, uint8_t data, uint8_t mem_mask) { logerror("%s: superxavix_write_extended_io0 %02x (mask %02x)\n", machine().describe_context(), data, mem_mask); }
	void superxavix_write_extended_io1(offs_t offset, uint8_t data, uint8_t mem_mask) { logerror("%s: superxavix_write_extended_io1 %02x (mask %02x)\n", machine().describe_context(), data, mem_mask); }
	void superxavix_write_extended_io2(offs_t offset, uint8_t data, uint8_t mem_mask) { logerror("%s: superxavix_write_extended_io2 %02x (mask %02x)\n", machine().describe_context(), data, mem_mask); }

	void draw_bitmap_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t get_next_bit_sx();
	virtual void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	virtual void draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int which) override;

	uint8_t m_superxavix_pal_index = 0;
	uint8_t m_superxavix_bitmap_pal_index = 0;
	uint32_t m_sx_plt_address = 0;
	uint8_t m_sx_plt_mode = 0;
	uint8_t m_plotter_has_byte = 0;
	uint8_t m_plotter_current_byte = 0x00;

	uint8_t m_sx_extended_extbus[3]{};

	required_shared_ptr<uint8_t> m_sx_crtc_1;
	required_shared_ptr<uint8_t> m_sx_crtc_2;
	required_shared_ptr<uint8_t> m_sx_plt_loc;

	required_shared_ptr<uint8_t> m_bmp_palram_sh;
	required_shared_ptr<uint8_t> m_bmp_palram_l;
	required_shared_ptr<uint8_t> m_bmp_base;

	optional_region_ptr<uint8_t> m_extra;
	required_ioport_array<3> m_exio;

	bool m_use_superxavix_extra; // does not need saving
};


class superxavix_i2c_state : public superxavix_state
{
public:
	superxavix_i2c_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_state(mconfig, type, tag),
		m_i2cmem(*this, "i2cmem")
	{ }

	void superxavix_i2c_24c16(machine_config &config);
	void superxavix_i2c_24c08(machine_config &config);
	void superxavix_i2c_24c04(machine_config &config);
	void superxavix_i2c_24c04_4mb(machine_config &config);
	void superxavix_i2c_24c02(machine_config &config);
	void superxavix_i2c_mrangbat(machine_config& config);

protected:
	virtual void write_io1(uint8_t data, uint8_t direction) override;

	required_device<i2cmem_device> m_i2cmem;
};



class xavix_i2c_state : public xavix_state
{
public:
	xavix_i2c_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag),
		m_i2cmem(*this, "i2cmem")
	{ }

	void xavix_i2c_24lc04(machine_config &config);
	void xavix_i2c_24lc04_4mb(machine_config &config);
	void xavix_i2c_24lc04_2mb(machine_config &config);
	void xavix_i2c_24lc04_1mb(machine_config &config);
	void xavix_i2c_24c02(machine_config &config);
	void xavix_i2c_24c02_4mb(machine_config &config);
	void xavix_i2c_24c02_2mb(machine_config &config);
	void xavix_i2c_24c02_43mhz(machine_config &config);
	void xavix_i2c_24c08(machine_config &config);
	void xavix_i2c_24c08_4mb(machine_config &config);
	void xavix_i2c_24c16(machine_config &config);
	void xavix_i2c_24c16_4mb(machine_config &config);

	void xavix2000_i2c_24c08(machine_config &config);
	void xavix2000_i2c_24c08_4mb(machine_config &config);
	void xavix2000_i2c_24c04(machine_config &config);
	void xavix2000_i2c_24c04_2mb(machine_config &config);
	void xavix2000_i2c_24c04_4mb(machine_config &config);
	void xavix2000_i2c_24c02(machine_config &config);

protected:
	virtual void write_io1(uint8_t data, uint8_t direction) override;

	required_device<i2cmem_device> m_i2cmem;
};

class xavix_i2c_mj_state : public xavix_i2c_state
{
public:
	xavix_i2c_mj_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
		, m_dial(*this, "DIAL")
	{ }

	void xavix_i2c_24lc02_mj(machine_config &config);

protected:
	virtual void write_io1(uint8_t data, uint8_t direction) override;

	uint8_t mj_anport0_r() { return m_dial->read()^0x7f; }

	required_ioport m_dial;
};

class xavix_epo_hamc_state : public xavix_state
{
public:
	xavix_epo_hamc_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
	{ }

protected:
};


class xavix_i2c_lotr_state : public xavix_i2c_state
{
public:
	xavix_i2c_lotr_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
	{ }

protected:
	//virtual void write_io1(uint8_t data, uint8_t direction) override;
};

class xavix_duelmast_state : public xavix_i2c_state
{
public:
	xavix_duelmast_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
	{ }

protected:
	virtual uint8_t read_io1(uint8_t direction) override;
};

class xavix_i2c_tomshoot_state : public xavix_i2c_state
{
public:
	xavix_i2c_tomshoot_state(const machine_config& mconfig, device_type type, const char* tag)
		: xavix_i2c_state(mconfig, type, tag)
	{ }

	void xavix_interrupt_extra() override
	{
		ioevent_trg01(1); // causes reads from the lightgun 1 port
		//ioevent_trg02(1); // causes reads from the lightgun 2 port
	}


private:
	virtual uint8_t lightgun_r(offs_t offset) override
	{
		uint16_t ret = m_lightgun[offset>>1]->read();

		if (offset & 1)
			ret >>= 8;
		else
			ret &= 0xff;

		if (offset == 0)
			ret += 0x20;

		return ret;
	}

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

class xavix_tom_tvho_state : public xavix_state
{
public:
	xavix_tom_tvho_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
	{ }

	void xavix_tom_tvho(machine_config &config);

private:

private:
	uint8_t tvho_anport0_r() { return (m_mouse0x->read()^0x7f)+1; }
	uint8_t tvho_anport1_r() { return (m_mouse0y->read()^0x7f)+1; }
	uint8_t tvho_anport2_r() { return (m_mouse1x->read()^0x7f)+1; }
	uint8_t tvho_anport3_r() { return (m_mouse1y->read()^0x7f)+1; }
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

	int mtrk_wheel_r();

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
		// all signals 0x000000 - 0x5fffff go to the cart
		// even if the largest cart is 0x400000 in size
		m_cartlimit = 0x600000;
	}

	void xavix_cart(machine_config &config);
	void xavix_cart_ekara(machine_config &config);
	void xavix_cart_isinger(machine_config &config);
	void xavix_cart_popira(machine_config &config);
	void xavix_cart_popirak(machine_config &config);
	void xavix_cart_ddrfammt(machine_config &config);
	void xavix_cart_evio(machine_config &config);
	void xavix_cart_daig(machine_config &config);

protected:

	virtual void xavix_extbus_map(address_map &map) override ATTR_COLD;

	u8 cart_r(offs_t offset)
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

	void cart_w(offs_t offset, uint8_t data)
	{
		if (m_cartslot->has_cart())
		{
			m_cartslot->write_cart(offset, data);
		}
		else
		{
			logerror("%s: unhandled write access to cart area with no cart installed %08x %02x\n", machine().describe_context(), offset, data);
		}
	}

	// for Cart cases this memory bypass becomes more complex

	virtual uint8_t opcodes_000000_r(offs_t offset) override
	{
		if (offset & 0x8000)
		{
			if (m_disable_memory_bypass)
			{
				return m_maincpu->space(6).read_byte(offset & 0x7fffff);
			}
			else
			{
				if ((offset & 0x7fffff) >= m_cartlimit)
				{
					return m_rgn[(offset) & (m_rgnlen - 1)];
				}
				else
				{
					return cart_r(offset);
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
		if (m_disable_memory_bypass)
		{
			return m_maincpu->space(6).read_byte(offset & 0x7fffff);
		}
		else
		{
			if ((offset & 0x7fffff) >= m_cartlimit)
			{
				return m_rgn[(offset) & (m_rgnlen - 1)];
			}
			else
			{
				return cart_r(offset);
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
				return cart_r(offset);
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
					return cart_r(offset);
				}
			}
			else
			{
				return read_full_data_sp_lowbus_bypass(offset);
			}
		}
	}

	int m_cartlimit = 0;
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
	virtual void xavix_extbus_map(address_map &map) override ATTR_COLD;
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

	int i2c_r();

protected:
	virtual void write_io1(uint8_t data, uint8_t direction) override;

private:
	uint8_t popira2_adc0_r();
	uint8_t popira2_adc1_r();

	required_ioport m_p2;
};

class xavix_evio_cart_state : public xavix_cart_state
{
public:
	xavix_evio_cart_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_cart_state(mconfig,type,tag)
	{ }

	int i2c_r();

protected:
	virtual void write_io1(uint8_t data, uint8_t direction) override;
};

class xavix_daig_cart_state : public xavix_cart_state
{
public:
	xavix_daig_cart_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_cart_state(mconfig,type,tag)
	{ }

protected:
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

	virtual int ekara_multi0_r();
	virtual int ekara_multi1_r();

protected:

	required_ioport m_extra0;
	required_ioport m_extra1;

	virtual void write_io0(uint8_t data, uint8_t direction) override;
	virtual void write_io1(uint8_t data, uint8_t direction) override;

	uint8_t m_extraioselect;
	uint8_t m_extraiowrite;
};

class xavix_hikara_state : public xavix_ekara_state
{
public:
	xavix_hikara_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_ekara_state(mconfig, type, tag),
		m_extra2(*this, "EXTRA2"),
		m_extra3(*this, "EXTRA3")
	{ }

	void xavix_cart_hikara(machine_config &config);

	virtual int ekara_multi0_r() override;
	virtual int ekara_multi1_r() override;
	int ekara_multi2_r();
	int ekara_multi3_r();

protected:
	virtual void machine_reset() override ATTR_COLD;

	required_ioport m_extra2;
	required_ioport m_extra3;

};


#endif // MAME_TVGAMES_XAVIX_H
