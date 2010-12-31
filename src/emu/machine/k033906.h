/***************************************************************************

    Konami 033906

***************************************************************************/

#pragma once

#ifndef __K033906_H__
#define __K033906_H__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_K033906_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, K033906, 0) \
	MCFG_DEVICE_CONFIG(_config)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> k033906_interface

struct k033906_interface
{
	const char         *m_voodoo_tag;
};



// ======================> k033906_device_config

class k033906_device_config : public device_config,
                              public k033906_interface
{
    friend class k033906_device;

    // construction/destruction
    k033906_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
    // device_config overrides
    virtual void device_config_complete();
};



// ======================> k033906_device

class k033906_device :  public device_t
{
    friend class k033906_device_config;

    // construction/destruction
    k033906_device(running_machine &_machine, const k033906_device_config &_config);

public:

	UINT32 k033906_r(UINT32 offset);
	void k033906_w(UINT32 offset, UINT32 data, UINT32 mem_mask);
	void k033906_set_reg(UINT8 state);

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset() { }
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

private:

	UINT32 k033906_reg_r(int reg);
	void k033906_reg_w(int reg, UINT32 data);

	/* i/o lines */
	UINT32 *     m_reg;
	UINT32 *     m_ram;

	int          m_reg_set;	// 1 = access reg / 0 = access ram

	device_t *m_voodoo;

    const k033906_device_config &m_config;
};


// device type definition
extern const device_type K033906;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

extern READ32_DEVICE_HANDLER( k033906_r );
extern WRITE32_DEVICE_HANDLER( k033906_w );
extern WRITE_LINE_DEVICE_HANDLER( k033906_set_reg );


#endif	/* __K033906_H__ */
