/***************************************************************************

    Fujistu MB3773

    Power Supply Monitor with Watch Dog Timer (i.e. Reset IC)

***************************************************************************/

#pragma once

#ifndef __MB3773_H__
#define __MB3773_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MDRV_MB3773_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, MB3773, 0)


// ======================> mb3773_device_config

class mb3773_device_config :
	public device_config
{
	friend class mb3773_device;

	// construction/destruction
	mb3773_device_config( const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock );

public:
	// allocators
	static device_config *static_alloc_device_config( const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock );
	virtual device_t *alloc_device( running_machine &machine ) const;

protected:
	// device_config overrides
	virtual void device_config_complete();
	virtual bool device_validity_check( const game_driver &driver ) const;
};


// ======================> mb3773_device

class mb3773_device :
	public device_t
{
	friend class mb3773_device_config;

	// construction/destruction
	mb3773_device( running_machine &_machine, const mb3773_device_config &config );

public:
	// I/O operations
	void set_ck( int state );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// internal helpers
	static TIMER_CALLBACK( watchdog_timeout );
	void reset_timer();

	// internal state
	const mb3773_device_config &m_config;
	emu_timer *m_watchdog_timer;
	int m_ck;
};


// device type definition
extern const device_type MB3773;


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_DEVICE_HANDLER( mb3773_set_ck );

#endif
