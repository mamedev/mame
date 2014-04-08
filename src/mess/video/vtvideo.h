/**********************************************************************

    DEC VT Terminal video emulation
    [ DC012 and DC011 emulation ]

    01/05/2009 Initial implementation [Miodrag Milanovic]

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#ifndef __VT_VIDEO__
#define __VT_VIDEO__

#include "emu.h"

#define MCFG_VT_VIDEO_RAM_CALLBACK(_read) \
	devcb = &vt100_video_device::set_ram_rd_callback(*device, DEVCB2_##_read);

#define MCFG_VT_VIDEO_CLEAR_VIDEO_INTERRUPT_CALLBACK(_write) \
	devcb = &vt100_video_device::set_clear_video_irq_wr_callback(*device, DEVCB2_##_write);

struct vt_video_interface
{
	const char *m_char_rom_tag; /* character rom region */
};


class vt100_video_device : public device_t,
							public device_video_interface,
							public vt_video_interface
{
public:
	vt100_video_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	vt100_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~vt100_video_device() {}

	template<class _Object> static devcb2_base &set_ram_rd_callback(device_t &device, _Object object) { return downcast<vt100_video_device &>(device).m_read_ram.set_callback(object); }
	template<class _Object> static devcb2_base &set_clear_video_irq_wr_callback(device_t &device, _Object object) { return downcast<vt100_video_device &>(device).m_write_clear_video_interrupt.set_callback(object); }

	DECLARE_READ8_MEMBER(lba7_r);
	DECLARE_WRITE8_MEMBER(dc012_w);
	DECLARE_WRITE8_MEMBER(dc011_w);
	DECLARE_WRITE8_MEMBER(brightness_w);

	virtual void video_update(bitmap_ind16 &bitmap, const rectangle &cliprect);
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

	// internal state
	void recompute_parameters();
	virtual void display_char(bitmap_ind16 &bitmap, UINT8 code, int x, int y, UINT8 scroll_region, UINT8 display_type);
	TIMER_CALLBACK_MEMBER(lba7_change);

	devcb2_read8        m_read_ram;
	devcb2_write8       m_write_clear_video_interrupt;

	UINT8 *m_gfx;     /* content of char rom */

	int m_lba7;

	bool MHFU_FLAG;
	int MHFU_counter;


	// dc012 attributes
	UINT8 m_scroll_latch;
	UINT8 m_scroll_latch_valid;
	UINT8 m_blink_flip_flop;
	UINT8 m_reverse_field;
	UINT8 m_basic_attribute;
	// dc011 attributes
	UINT8 m_columns;
	UINT8 m_height;
	UINT8 m_height_MAX;
	UINT8 m_skip_lines;
	UINT8 m_frequency;
	UINT8 m_interlaced;

	required_device<palette_device> m_palette;
};


class rainbow_video_device : public vt100_video_device
{
public:
	rainbow_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void video_update(bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void video_blanking(bitmap_ind16 &bitmap, const rectangle &cliprect);

	int MHFU(int);
	void palette_select(int choice);
protected:
	virtual void display_char(bitmap_ind16 &bitmap, UINT8 code, int x, int y, UINT8 scroll_region, UINT8 display_type);
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;
};

extern const device_type VT100_VIDEO;
extern const device_type RAINBOW_VIDEO;


#define MCFG_VT100_VIDEO_ADD(_tag, _screen_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, VT100_VIDEO, 0) \
	MCFG_DEVICE_CONFIG(_intrf) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag)

#define MCFG_RAINBOW_VIDEO_ADD(_tag, _screen_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, RAINBOW_VIDEO, 0) \
	MCFG_DEVICE_CONFIG(_intrf) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag)



#endif
