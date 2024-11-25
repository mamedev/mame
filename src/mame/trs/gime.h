// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    gime.h

    Implementation of CoCo GIME (Graphics Interrupt Memory Enhancement)
    video chip.

***************************************************************************/

#ifndef MAME_TRS_GIME_H
#define MAME_TRS_GIME_H

#pragma once

#include "machine/6883sam.h"
#include "machine/ram.h"
#include "video/mc6847.h"


//**************************************************************************
//  GIME CORE
//**************************************************************************

class cococart_slot_device;

class gime_device : public mc6847_friend_device, public sam6883_friend_device_interface
{
public:
	auto irq_wr_callback() { return m_write_irq.bind(); }
	auto firq_wr_callback() { return m_write_firq.bind(); }
	auto floating_bus_rd_callback() { return m_read_floating_bus.bind(); }

	// read/write
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	// used to turn on/off reading/writing to $FF40-$FF5F
	bool spare_chip_select_enabled() { return m_gime_registers[0] & 0x04 ? true : false; }

	// the GIME seems to intercept writes to $FF22 (not precisely sure how)
	void pia_write(offs_t offset, uint8_t data);

	// updates the cart ROM
	void update_cart_rom();

	/* updates the screen -- this will call begin_update(),
	   followed by update_row() repeatedly and after all row
	   updating is complete, end_update() */
	bool update_composite(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	bool update_rgb(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// interrupt outputs
	bool firq_r() const { return m_firq != 0x00; }
	bool irq_r() const { return m_irq != 0x00; }

	// interrupt inputs
	void set_il0(bool value) { set_interrupt_value(INTERRUPT_EI0, value); }
	void set_il1(bool value) { set_interrupt_value(INTERRUPT_EI1, value); }
	void set_il2(bool value) { set_interrupt_value(INTERRUPT_EI2, value); }

protected:
	gime_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const uint8_t *fontdata, bool pal);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_pre_save() override;
	virtual void device_post_load() override;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// other overrides
	virtual void new_frame() override;
	virtual TIMER_CALLBACK_MEMBER(horizontal_sync_changed) override;
	virtual void enter_bottom_border() override;
	virtual void record_border_scanline(uint16_t physical_scanline) override;
	virtual void record_full_body_scanline(uint16_t physical_scanline, uint16_t logical_scanline) override;
	virtual void record_partial_body_scanline(uint16_t physical_scanline, uint16_t logical_scanline, int32_t start_clock, int32_t end_clock) override;

protected:
	typedef mc6847_friend_device super;

	struct scanline_record
	{
		uint8_t m_border;
		uint8_t m_line_in_row;
		uint8_t m_ff22_value;
		uint8_t m_ff98_value;
		uint8_t m_ff99_value;
		uint8_t m_mode[160];
		uint8_t m_data[160];
		uint16_t m_palette[160];
	};

	typedef uint32_t (gime_device::*get_data_func)(uint32_t, uint8_t *, uint8_t *);

	class palette_resolver
	{
	public:
		palette_resolver(gime_device &gime, const pixel_t *palette);
		const pixel_t *get_palette(uint16_t palette_rotation);
		pixel_t lookup(uint8_t color);

	private:
		gime_device &m_gime;
		const pixel_t *m_palette;
		pixel_t m_resolved_palette[16];
		int m_current_resolved_palette;
	};

	enum
	{
		INTERRUPT_TMR       = 0x20,
		INTERRUPT_HBORD     = 0x10,
		INTERRUPT_VBORD     = 0x08,
		INTERRUPT_EI2       = 0x04,
		INTERRUPT_EI1       = 0x02,
		INTERRUPT_EI0       = 0x01
	};

	enum timer_type_t
	{
		GIME_TIMER_63USEC,
		GIME_TIMER_279NSEC
	};

	// statics
	static const uint8_t lowres_font[];
	static const uint8_t hires_font[128][12];

	// callbacks
	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_firq;
	devcb_read8        m_read_floating_bus;

	// device state
	uint8_t                     m_gime_registers[16];
	uint8_t                     m_mmu[16];
	uint8_t                     m_ff22_value;
	uint8_t                     m_ff23_value;
	uint8_t                     m_interrupt_value;
	uint8_t                     m_irq;
	uint8_t                     m_firq;
	uint16_t                    m_timer_value;
	bool                        m_is_blinking;
	bool                        m_composite_phase_invert;

	// video state
	bool                        m_legacy_video;
	uint32_t                    m_video_position;
	uint8_t                     m_line_in_row;
	scanline_record             m_scanlines[25+192+26];
	bool                        m_displayed_rgb;

	// palette state
	uint8_t                     m_palette_rotated[1024][16];
	uint16_t                    m_palette_rotated_position;
	bool                        m_palette_rotated_position_used;

	// incidentals
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	emu_timer *                 m_gime_clock_timer;
	required_device<cococart_slot_device> m_cart_device;
	memory_bank *               m_read_banks[9];
	memory_bank *               m_write_banks[9];
	uint8_t *                   m_rom;
	required_memory_region      m_rom_region;
	uint8_t *                   m_cart_rom;
	uint32_t                    m_cart_size;
	pixel_t                     m_composite_palette[64];
	pixel_t                     m_composite_bw_palette[64];
	pixel_t                     m_rgb_palette[64];
	uint8_t                     m_dummy_bank[0x2000];

	// read/write
	uint8_t read_gime_register(offs_t offset);
	uint8_t read_mmu_register(offs_t offset);
	uint8_t read_palette_register(offs_t offset);
	uint8_t read_floating_bus();
	void write_gime_register(offs_t offset, uint8_t data);
	void write_mmu_register(offs_t offset, uint8_t data);
	void write_palette_register(offs_t offset, uint8_t data);
	void write_sam_register(offs_t offset);

	// memory
	void update_memory();
	void update_memory(int bank);
	uint8_t *memory_pointer(uint32_t address);

	// interrupts
	void interrupt_rising_edge(uint8_t interrupt);
	void change_gime_irq(uint8_t data);
	void change_gime_firq(uint8_t data);

	ATTR_FORCE_INLINE void set_interrupt_value(uint8_t interrupt, bool value)
	{
		/* save the old interrupt value */
		uint8_t old_interrupt_value = m_interrupt_value;

		/* update the interrupt value */
		if (value)
			m_interrupt_value |= interrupt;
		else
			m_interrupt_value &= ~interrupt;

		/* was this a rising edge? */
		if (value && ((old_interrupt_value & interrupt) == 0))
		{
			interrupt_rising_edge(interrupt);
		}
	}

	// timer
	timer_type_t timer_type();
	const char *timer_type_string();
	void reset_timer();
	TIMER_CALLBACK_MEMBER(timer_elapsed);

	// video
	bool update_screen(bitmap_rgb32 &bitmap, const rectangle &cliprect, const pixel_t *palette);
	void update_geometry();
	void update_rgb_palette();
	void update_composite_palette();
	void update_border(uint16_t physical_scanline);
	pixel_t get_composite_color(int color);
	pixel_t get_rgb_color(int color);
	offs_t get_video_base();
	uint16_t get_lines_per_row();
	uint32_t get_data_mc6847(uint32_t video_position, uint8_t *data, uint8_t *mode);
	uint32_t get_data_without_attributes(uint32_t video_position, uint8_t *data, uint8_t *mode);
	uint32_t get_data_with_attributes(uint32_t video_position, uint8_t *data, uint8_t *mode);

	// template function for doing video update collection
	template<uint8_t xres, get_data_func get_data, bool record_mode>
	uint32_t record_scanline_res(int scanline);

	// rendering sampled graphics
	typedef uint32_t (gime_device::*emit_samples_proc)(const scanline_record *scanline, int sample_start, int sample_count, pixel_t *pixels, const pixel_t *palette);
	uint32_t emit_dummy_samples(const scanline_record *scanline, int sample_start, int sample_count, pixel_t *pixels, const pixel_t *palette);
	uint32_t emit_mc6847_samples(const scanline_record *scanline, int sample_start, int sample_count, pixel_t *pixels, const pixel_t *palette);
	template<int xscale>
	uint32_t emit_gime_text_samples(const scanline_record *scanline, int sample_start, int sample_count, pixel_t *pixels, const pixel_t *palette);
	template<int xscale, int bits_per_pixel>
	uint32_t emit_gime_graphics_samples(const scanline_record *scanline, int sample_start, int sample_count, pixel_t *pixels, const pixel_t *palette);
	template<int sample_count, emit_samples_proc emit_samples>
	void render_scanline(const scanline_record *scanline, pixel_t *pixels, int min_x, int max_x, palette_resolver *resolver);
};


//**************************************************************************
//  VARIATIONS
//**************************************************************************

class gime_ntsc_device : public gime_device
{
public:
	template <typename T, typename U, typename V, typename W>
	gime_ntsc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&ram_tag, V &&ext_tag, W &&region_tag)
		: gime_ntsc_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_ram.set_tag(std::forward<U>(ram_tag));
		m_cart_device.set_tag(std::forward<V>(ext_tag));
		m_rom_region.set_tag(std::forward<W>(region_tag));
	}

	gime_ntsc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class gime_pal_device : public gime_device
{
public:
	template <typename T, typename U, typename V, typename W>
	gime_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&ram_tag, V &&ext_tag, W &&region_tag)
		: gime_pal_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_ram.set_tag(std::forward<U>(ram_tag));
		m_cart_device.set_tag(std::forward<V>(ext_tag));
		m_rom_region.set_tag(std::forward<W>(region_tag));
	}

	gime_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(GIME_NTSC, gime_ntsc_device)
DECLARE_DEVICE_TYPE(GIME_PAL, gime_pal_device)

#endif //MAME_TRS_GIME_H
