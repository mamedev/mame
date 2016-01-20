// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**********************************************************************

    TI TMS9927 and compatible CRT controller emulation

**********************************************************************/

#ifndef __TMS9927__
#define __TMS9927__


#define MCFG_TMS9927_VSYN_CALLBACK(_write) \
	devcb = &tms9927_device::set_vsyn_wr_callback(*device, DEVCB_##_write);

#define MCFG_TMS9927_CHAR_WIDTH(_pixels) \
	tms9927_device::set_char_width(*device, _pixels);

#define MCFG_TMS9927_REGION(_tag) \
	tms9927_device::set_region_tag(*device, _tag);


class tms9927_device : public device_t,
						public device_video_interface
{
public:
	tms9927_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms9927_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	~tms9927_device() {}

	template<class _Object> static devcb_base &set_vsyn_wr_callback(device_t &device, _Object object) { return downcast<tms9927_device &>(device).m_write_vsyn.set_callback(object); }

	static void set_char_width(device_t &device, int pixels) { downcast<tms9927_device &>(device).m_hpixels_per_column = pixels; }
	static void set_region_tag(device_t &device, const char *tag) { downcast<tms9927_device &>(device).m_selfload_region = tag; }

	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8_MEMBER(read);

	int screen_reset();
	int upscroll_offset();
	int cursor_bounds(rectangle &bounds);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum
	{
		TIMER_VSYNC
	};

	void state_postload();
	void recompute_parameters(int postload);
	void generic_access(address_space &space, offs_t offset);

	devcb_write_line m_write_vsyn;
	int m_hpixels_per_column;         /* number of pixels per video memory address */
	const char *m_selfload_region;    /* name of the region with self-load data */

	// internal state
	const UINT8 *m_selfload;

	/* live state */
	UINT32  m_clock;
	UINT8   m_reg[9];
	UINT8   m_start_datarow;
	UINT8   m_reset;

	/* derived state; no need to save */
	UINT8   m_valid_config;
	UINT16  m_total_hpix, m_total_vpix;
	UINT16  m_visible_hpix, m_visible_vpix;

	int m_vsyn;

	emu_timer *m_vsync_timer;
};

extern const device_type TMS9927;

class crt5027_device : public tms9927_device
{
public:
	crt5027_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type CRT5027;

class crt5037_device : public tms9927_device
{
public:
	crt5037_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type CRT5037;

class crt5057_device : public tms9927_device
{
public:
	crt5057_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type CRT5057;


#endif
