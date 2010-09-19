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

#define MDRV_MC146818_ADD(_tag, _type) \
	MDRV_DEVICE_ADD(_tag, MC146818, 0) \
	mc146818_device_config::static_set_type(device, mc146818_device_config::_type); \



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mc146818_device;


// ======================> mc146818_device_config

class mc146818_device_config :	public device_config,
								public device_config_nvram_interface
{
	friend class mc146818_device;

	// construction/destruction
	mc146818_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	// values
	enum mc146818_type
	{
		MC146818_STANDARD,
		MC146818_IGNORE_CENTURY, // century is NOT set, for systems having other usage of this byte
		MC146818_ENHANCED
	};

	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

	// inline configuration helpers
	static void static_set_type(device_config *device, mc146818_type type);

protected:
	// internal state
	mc146818_type		m_type;
};


// ======================> mc146818_device

class mc146818_device :	public device_t,
						public device_nvram_interface
{
	friend class mc146818_device_config;

	// construction/destruction
	mc146818_device(running_machine &_machine, const mc146818_device_config &config);

public:
	// read/write access
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_mc146818_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(mame_file &file);
	virtual void nvram_write(mame_file &file);

	// internal helpers
	int dec_2_local(int a);
	void set_base_datetime();

	// internal state
	static const int MC146818_DATA_SIZE	= 0x80;

	const mc146818_device_config &	m_config;

	UINT8			m_index;
	UINT8			m_data[MC146818_DATA_SIZE];

	UINT16			m_eindex;
	UINT8			m_edata[0x2000];

	bool			m_updated;  /* update ended interrupt flag */

	attotime		m_last_refresh;
};


// device type definition
extern const device_type MC146818;


#endif /* __MC146818_H__ */
