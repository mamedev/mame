// license:GPL-2.0+
// copyright-holders:Miodrag Milanovic,Karl-Ludwig Deisenhofer
/**********************************************************************

DEC VT Terminal video emulation
[ DC012 and DC011 emulation ]

01/05/2009 Initial implementation [Miodrag Milanovic]

**********************************************************************/

#ifndef __VT_VIDEO__
#define __VT_VIDEO__

#include "emu.h"

class vt100_video_device : public device_t,
	public device_video_interface
{
public:
	vt100_video_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	vt100_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~vt100_video_device() {}

	template<class _Object> static devcb_base &set_ram_rd_callback(device_t &device, _Object object) { return downcast<vt100_video_device &>(device).m_read_ram.set_callback(object); }
	template<class _Object> static devcb_base &set_clear_video_irq_wr_callback(device_t &device, _Object object) { return downcast<vt100_video_device &>(device).m_write_clear_video_interrupt.set_callback(object); }

	static void set_chargen_tag(device_t &device, const char *tag) { downcast<vt100_video_device &>(device).m_char_rom.set_tag(tag); }

	DECLARE_READ8_MEMBER(lba7_r);
	DECLARE_WRITE8_MEMBER(dc012_w);
	DECLARE_WRITE8_MEMBER(dc011_w);
	DECLARE_WRITE8_MEMBER(brightness_w);

	virtual void video_update(bitmap_ind16 &bitmap, const rectangle &cliprect);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// internal state
	void recompute_parameters();
	virtual void display_char(bitmap_ind16 &bitmap, UINT8 code, int x, int y, UINT8 scroll_region, UINT8 display_type);
	TIMER_CALLBACK_MEMBER(lba7_change);

	devcb_read8        m_read_ram;
	devcb_write8       m_write_clear_video_interrupt;

	int m_lba7;

	bool MHFU_FLAG;
	int MHFU_counter;

	// dc012 attributes
	UINT8 m_scroll_latch;
	bool m_scroll_latch_valid;
	UINT8 m_blink_flip_flop;
	UINT8 m_reverse_field;
	UINT8 m_basic_attribute;
	// dc011 attributes
	UINT8 m_columns;
	UINT8 m_height;
	UINT8 m_height_MAX;
	UINT8 m_fill_lines;
	UINT8 m_frequency;
	UINT8 m_interlaced;

	required_region_ptr<UINT8> m_char_rom; /* character rom region */
	required_device<palette_device> m_palette;

	bool m_notify_vblank;
	int m_last_scroll;

	bool m_linedoubler;
};


class rainbow_video_device : public vt100_video_device
{
public:
	rainbow_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void video_update(bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	virtual void video_blanking(bitmap_ind16 &bitmap, const rectangle &cliprect);

	int MHFU(int);
	void palette_select(int choice);
	void notify_vblank(bool choice);
protected:
	virtual void display_char(bitmap_ind16 &bitmap, UINT8 code, int x, int y, UINT8 scroll_region, UINT8 display_type) override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
};

extern const device_type VT100_VIDEO;
extern const device_type RAINBOW_VIDEO;


#define MCFG_VT_SET_SCREEN MCFG_VIDEO_SET_SCREEN

#define MCFG_VT_CHARGEN(_tag) \
	vt100_video_device::set_chargen_tag(*device, "^" _tag);

#define MCFG_VT_VIDEO_RAM_CALLBACK(_read) \
	devcb = &vt100_video_device::set_ram_rd_callback(*device, DEVCB_##_read);

#define MCFG_VT_VIDEO_CLEAR_VIDEO_INTERRUPT_CALLBACK(_write) \
	devcb = &vt100_video_device::set_clear_video_irq_wr_callback(*device, DEVCB_##_write);

#endif
