// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    gime.h

    Implementation of CoCo GIME (Graphics Interrupt Memory Enhancement)
    video chip.

***************************************************************************/

#ifndef MAME_VIDEO_GIME_H
#define MAME_VIDEO_GIME_H

#pragma once

#include "video/mc6847.h"
#include "machine/6883sam.h"
#include "machine/ram.h"


//**************************************************************************
//  GIME CONFIG/INTERFACE
//**************************************************************************

#define MCFG_GIME_HSYNC_CALLBACK    MCFG_MC6847_HSYNC_CALLBACK

#define MCFG_GIME_FSYNC_CALLBACK    MCFG_MC6847_FSYNC_CALLBACK

#define MCFG_GIME_IRQ_CALLBACK(_write) \
	devcb = &downcast<gime_device &>(*device).set_irq_wr_callback(DEVCB_##_write);

#define MCFG_GIME_FIRQ_CALLBACK(_write) \
	devcb = &downcast<gime_device &>(*device).set_firq_wr_callback(DEVCB_##_write);

#define MCFG_GIME_FLOATING_BUS_CALLBACK(_read) \
	devcb = &downcast<gime_device &>(*device).set_floating_bus_rd_callback(DEVCB_##_read);

#define MCFG_GIME_MAINCPU(_tag) \
	downcast<gime_device &>(*device).set_maincpu_tag(_tag);

#define MCFG_GIME_RAM(_tag) \
	downcast<gime_device &>(*device).set_ram_tag(_tag);

#define MCFG_GIME_EXT(_tag) \
	downcast<gime_device &>(*device).set_ext_tag(_tag);


//**************************************************************************
//  GIME CORE
//**************************************************************************

class cococart_slot_device;

class gime_device : public mc6847_friend_device, public sam6883_friend_device_interface
{
public:
	template <class Object> devcb_base &set_irq_wr_callback(Object &&cb) { return m_write_irq.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_firq_wr_callback(Object &&cb) { return m_write_firq.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_floating_bus_rd_callback(Object &&cb) { return m_read_floating_bus.set_callback(std::forward<Object>(cb)); }
	void set_maincpu_tag(const char *tag) { m_maincpu_tag = tag; }
	void set_ram_tag(const char *tag) { m_ram_tag = tag; }
	void set_ext_tag(const char *tag) { m_ext_tag = tag; }

	// read/write
	DECLARE_READ8_MEMBER( read ) { return read(offset); }
	DECLARE_WRITE8_MEMBER( write ) { write(offset, data); }

	// used to turn on/off reading/writing to $FF40-$FF5F
	bool spare_chip_select_enabled(void) { return m_gime_registers[0] & 0x04 ? true : false; }

	// the GIME seems to intercept writes to $FF22 (not precisely sure how)
	void ff22_write(uint8_t data) { m_ff22_value = data; }

	// updates the cart ROM
	void update_cart_rom(void);

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
	gime_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const uint8_t *fontdata);

	// device-level overrides
	virtual void device_start(void) override;
	virtual void device_reset(void) override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_pre_save(void) override;
	virtual void device_post_load(void) override;
	virtual ioport_constructor device_input_ports() const override;

	// other overrides
	virtual void new_frame(void) override;
	virtual void horizontal_sync_changed(bool line) override;
	virtual void enter_bottom_border(void) override;
	virtual void record_border_scanline(uint16_t physical_scanline) override;
	virtual void record_body_scanline(uint16_t physical_scanline, uint16_t logical_scanline) override;
	virtual void record_partial_body_scanline(uint16_t physical_scanline, uint16_t logical_scanline, int32_t start_clock, int32_t end_clock) override;

private:
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
		GIME_TIMER_HBORD,
		GIME_TIMER_CLOCK
	};

	// timer constants
	static const device_timer_id TIMER_GIME_CLOCK = 4;

	// statics
	static const uint8_t hires_font[128][12];

	// callbacks
	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_firq;
	devcb_read8        m_read_floating_bus;

	// device state
	uint8_t                       m_gime_registers[16];
	uint8_t                       m_mmu[16];
	uint8_t                       m_ff22_value;
	uint8_t                       m_interrupt_value;
	uint8_t                       m_irq;
	uint8_t                       m_firq;
	uint16_t                      m_timer_value;
	bool                        m_is_blinking;
	bool                        m_composite_phase_invert;

	// video state
	bool                        m_legacy_video;
	uint32_t                      m_video_position;
	uint8_t                       m_line_in_row;
	scanline_record             m_scanlines[25+192+26];
	bool                        m_displayed_rgb;

	// palette state
	uint8_t                       m_palette_rotated[1024][16];
	uint16_t                      m_palette_rotated_position;
	bool                        m_palette_rotated_position_used;

	// incidentals
	ram_device *                m_ram;
	emu_timer *                 m_gime_clock_timer;
	cococart_slot_device *      m_cart_device;
	memory_bank *               m_read_banks[9];
	memory_bank *               m_write_banks[9];
	uint8_t *                   m_rom;
	uint8_t *                   m_cart_rom;
	uint32_t                    m_cart_size;
	pixel_t                     m_composite_palette[64];
	pixel_t                     m_composite_bw_palette[64];
	pixel_t                     m_rgb_palette[64];
	uint8_t                     m_dummy_bank[0x2000];

	const char *m_maincpu_tag;  /* tag of main CPU */
	const char *m_ram_tag;      /* tag of RAM device */
	const char *m_ext_tag;      /* tag of expansion device */

	// timer constants
	static const device_timer_id TIMER_FRAME = 0;
	static const device_timer_id TIMER_HSYNC_OFF = 1;
	static const device_timer_id TIMER_HSYNC_ON = 2;
	static const device_timer_id TIMER_FSYNC_OFF = 3;
	static const device_timer_id TIMER_FSYNC_ON = 4;

	// read/write
	uint8_t read(offs_t offset);
	uint8_t read_gime_register(offs_t offset);
	uint8_t read_mmu_register(offs_t offset);
	uint8_t read_palette_register(offs_t offset);
	uint8_t read_floating_bus(void);
	void write(offs_t offset, uint8_t data);
	void write_gime_register(offs_t offset, uint8_t data);
	void write_mmu_register(offs_t offset, uint8_t data);
	void write_palette_register(offs_t offset, uint8_t data);
	void write_sam_register(offs_t offset);

	// memory
	void update_memory(void);
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
	timer_type_t timer_type(void);
	const char *timer_type_string(void);
	void reset_timer(void);
	void timer_elapsed(void);

	// video
	bool update_screen(bitmap_rgb32 &bitmap, const rectangle &cliprect, const pixel_t *palette);
	void update_geometry(void);
	void update_rgb_palette(void);
	void update_composite_palette(void);
	void update_border(uint16_t physical_scanline);
	pixel_t get_composite_color(int color);
	pixel_t get_rgb_color(int color);
	offs_t get_video_base(void);
	uint16_t get_lines_per_row(void);
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

DECLARE_DEVICE_TYPE(GIME_NTSC, gime_device)
DECLARE_DEVICE_TYPE(GIME_PAL, gime_device)

#endif //MAME_VIDEO_GIME_H
