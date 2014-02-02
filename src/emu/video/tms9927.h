/**********************************************************************

    TI TMS9927 and compatible CRT controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#ifndef __TMS9927__
#define __TMS9927__


#define MCFG_TMS9927_VSYN_CALLBACK(_write) \
	devcb = &tms9927_device::set_vsyn_wr_callback(*device, DEVCB2_##_write);

/* interface */
struct tms9927_interface
{
	int m_hpixels_per_column;         /* number of pixels per video memory address */
	const char *m_selfload_region;    /* name of the region with self-load data */
};


class tms9927_device : public device_t,
						public device_video_interface,
						public tms9927_interface
{
public:
	tms9927_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms9927_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	~tms9927_device() {}

	template<class _Object> static devcb2_base &set_vsyn_wr_callback(device_t &device, _Object object) { return downcast<tms9927_device &>(device).m_write_vsyn.set_callback(object); }

	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8_MEMBER(read);

	int screen_reset();
	int upscroll_offset();
	int cursor_bounds(rectangle &bounds);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	enum
	{
		TIMER_VSYNC
	};

	void state_postload();
	void recompute_parameters(int postload);
	void generic_access(address_space &space, offs_t offset);

	devcb2_write_line m_write_vsyn;

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



#define MCFG_TMS9927_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, TMS9927, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_TMS9927_RECONFIG(_tag, _clock, _config) \
	MCFG_DEVICE_MODIFY(_tag) \
	MCFG_DEVICE_CLOCK(_clock) \
	MCFG_DEVICE_CONFIG(_config)


#endif
