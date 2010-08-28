/***************************************************************************

    Konami 056230

***************************************************************************/

#pragma once

#ifndef __K056230_H__
#define __K056230_H__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_K056230_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, K0506230, 0) \
	MDRV_DEVICE_CONFIG(_config)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> k056230_interface

struct k056230_interface
{
	const char         *m_cpu;
	int                m_is_thunderh;
};



// ======================> k056230_device_config

class k056230_device_config : public device_config,
                              public k056230_interface
{
    friend class k056230_device;

    // construction/destruction
    k056230_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
    // device_config overrides
    virtual void device_config_complete();
};



// ======================> k056230_device

class k056230_device :  public device_t
{
    friend class k056230_device_config;

    // construction/destruction
    k056230_device(running_machine &_machine, const k056230_device_config &_config);

public:

	UINT32 lanc_ram_r(UINT32 offset);
	void lanc_ram_w(UINT32 offset, UINT32 data, UINT32 mem_mask);

	UINT8 k056230_r(UINT32 offset);
	void k056230_w(UINT32 offset, UINT8 data);

	static TIMER_CALLBACK( network_irq_clear_callback );

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset() { }
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

private:

	void network_irq_clear();

	UINT32 *m_ram;
	int		m_is_thunderh;

	running_device *m_cpu;

    const k056230_device_config &m_config;
};


// device type definition
extern const device_type K0506230;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

extern READ32_DEVICE_HANDLER( lanc_ram_r );
extern WRITE32_DEVICE_HANDLER( lanc_ram_w );
extern READ8_DEVICE_HANDLER( k056230_r );
extern WRITE8_DEVICE_HANDLER( k056230_w );


#endif	/* __K056230_H__ */
