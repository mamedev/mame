// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**********************************************************************

    TI TMS9927 and compatible CRT controller emulation

**********************************************************************/

#ifndef MAME_VIDEO_TMS9927_H
#define MAME_VIDEO_TMS9927_H


#define MCFG_TMS9927_VSYN_CALLBACK(_write) \
	devcb = &downcast<tms9927_device &>(*device).set_vsyn_wr_callback(DEVCB_##_write);

#define MCFG_TMS9927_HSYN_CALLBACK(_write) \
	devcb = &downcast<tms9927_device &>(*device).set_hsyn_wr_callback(DEVCB_##_write);

#define MCFG_TMS9927_CHAR_WIDTH(_pixels) \
	downcast<tms9927_device &>(*device).set_char_width(_pixels);

#define MCFG_TMS9927_REGION(_tag) \
	downcast<tms9927_device &>(*device).set_region_tag(_tag);

#define MCFG_TMS9927_OVERSCAN(_left, _right, _top, _bottom) \
	downcast<tms9927_device &>(*device).set_overscan(_left, _right, _top, _bottom);

class tms9927_device : public device_t, public device_video_interface
{
public:
	tms9927_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_vsyn_wr_callback(Object &&cb) { return m_write_vsyn.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_hsyn_wr_callback(Object &&cb) { return m_write_hsyn.set_callback(std::forward<Object>(cb)); }

	void set_char_width(int pixels) { m_hpixels_per_column = pixels; }
	void set_region_tag(const char *tag) { m_selfload.set_tag(tag); }
	void set_overscan(int left, int right, int top, int bottom) {
		m_overscan_left = left;
		m_overscan_right = right;
		m_overscan_top = top;
		m_overscan_bottom = bottom;
	}

	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8_MEMBER(read);

	DECLARE_READ_LINE_MEMBER(bl_r);

	bool screen_reset() const { return m_reset; }
	int upscroll_offset() const { return m_start_datarow; }
	bool cursor_bounds(rectangle &bounds) const;

protected:
	tms9927_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum
	{
		TIMER_VSYNC,
		TIMER_HSYNC
	};

	void state_postload();
	void recompute_parameters(bool postload);
	void generic_access(address_space &space, offs_t offset);

	devcb_write_line m_write_vsyn;
	devcb_write_line m_write_hsyn;

	int m_hpixels_per_column;         /* number of pixels per video memory address */
	uint16_t  m_overscan_left;
	uint16_t  m_overscan_right;
	uint16_t  m_overscan_top;
	uint16_t  m_overscan_bottom;

	// internal state
	optional_region_ptr<uint8_t> m_selfload;

	/* live state */
	uint8_t   m_reg[9];
	uint8_t   m_start_datarow;
	bool      m_reset;
	bool      m_vsyn;
	bool      m_hsyn;

	/* derived state; no need to save */
	bool      m_valid_config;
	uint16_t  m_total_hpix, m_total_vpix;
	uint16_t  m_visible_hpix, m_visible_vpix;
	uint16_t  m_vsyn_start, m_vsyn_end;
	uint16_t  m_hsyn_start, m_hsyn_end;

	emu_timer *m_vsync_timer;
	emu_timer *m_hsync_timer;
};


class crt5027_device : public tms9927_device
{
public:
	crt5027_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class crt5037_device : public tms9927_device
{
public:
	crt5037_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class crt5057_device : public tms9927_device
{
public:
	crt5057_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(TMS9927, tms9927_device)
DECLARE_DEVICE_TYPE(CRT5027, crt5027_device)
DECLARE_DEVICE_TYPE(CRT5037, crt5037_device)
DECLARE_DEVICE_TYPE(CRT5057, crt5057_device)

#endif // MAME_VIDEO_TMS9927_H
