/*********************************************************************

    mc146818.h

    Implementation of the MC146818 chip

    Real time clock chip with batteru buffered ram
    Used in IBM PC/AT, several PC clones, Amstrad NC200

    Peter Trauner (peter.trauner@jk.uni-linz.ac.at)

*********************************************************************/

#ifndef __MC146818_H__
#define __MC146818_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MC146818_IRQ_ADD(_tag, _type, _intrf) \
	MCFG_DEVICE_ADD(_tag, MC146818, 0) \
	mc146818_device::static_set_type(*device, mc146818_device::_type); \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_MC146818_ADD(_tag, _type) \
	MCFG_DEVICE_ADD(_tag, MC146818, 0) \
	mc146818_device::static_set_type(*device, mc146818_device::_type);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mc146818_interface

struct mc146818_interface
{
	devcb_write_line    m_out_irq_cb;
};

// ======================> mc146818_device

class mc146818_device : public device_t,
						public device_rtc_interface,
						public device_nvram_interface,
						public mc146818_interface
{
public:
	// values
	enum mc146818_type
	{
		MC146818_STANDARD,
		MC146818_IGNORE_CENTURY, // century is NOT set, for systems having other usage of this byte
		MC146818_ENHANCED,
		MC146818_UTC
	};

	// construction/destruction
	mc146818_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_type(device_t &device, mc146818_type type);

	// read/write access
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second);

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);

	// internal helpers
	int dec_2_local(int a);
	void set_base_datetime();

	// internal state
	static const int MC146818_DATA_SIZE = 0x80;

	mc146818_type   m_type;

	UINT8           m_index;
	UINT8           m_data[MC146818_DATA_SIZE];

	UINT16          m_eindex;
	UINT8           m_edata[0x2000];

	bool            m_updated;  /* update ended interrupt flag */

	attotime        m_last_refresh;
	attotime        m_period;

	static const device_timer_id TIMER_CLOCK = 0;
	static const device_timer_id TIMER_PERIODIC = 1;

	emu_timer *m_clock_timer;
	emu_timer *m_periodic_timer;

	devcb_resolved_write_line m_out_irq_func;
};


// device type definition
extern const device_type MC146818;


#endif /* __MC146818_H__ */
