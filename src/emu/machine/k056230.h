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

#define MCFG_K056230_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, K056230, 0) \
	MCFG_DEVICE_CONFIG(_config)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> k056230_interface

struct k056230_interface
{
	const char         *m_cpu_tag;
	bool                m_is_thunderh;
};



// ======================> k056230_device

class k056230_device :  public device_t,
						public k056230_interface
{
public:
	// construction/destruction
	k056230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	UINT32 lanc_ram_r(UINT32 offset);
	void lanc_ram_w(UINT32 offset, UINT32 data, UINT32 mem_mask);

	UINT8 k056230_r(UINT32 offset);
	void k056230_w(UINT32 offset, UINT8 data);

	static TIMER_CALLBACK( network_irq_clear_callback );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset() { }
	virtual void device_post_load() { }
	virtual void device_clock_changed() { }

private:

	void network_irq_clear();

	UINT32 *m_ram;
	device_t *m_cpu;
};


// device type definition
extern const device_type K056230;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

extern DECLARE_READ32_DEVICE_HANDLER( lanc_ram_r );
extern DECLARE_WRITE32_DEVICE_HANDLER( lanc_ram_w );
extern DECLARE_READ8_DEVICE_HANDLER( k056230_r );
extern DECLARE_WRITE8_DEVICE_HANDLER( k056230_w );


#endif  /* __K056230_H__ */
