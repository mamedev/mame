/*********************************************************************

    8255ppi.h

    Intel 8255 PPI I/O chip

*********************************************************************/

#pragma once

#ifndef __8255PPI_H__
#define __8255PPI_H__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_PPI8255_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PPI8255, 0) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PPI8255_RECONFIG(_tag, _intrf) \
	MDRV_DEVICE_MODIFY(_tag) \
	MDRV_DEVICE_CONFIG(_intrf)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> ppi8255_interface

struct ppi8255_interface
{
	devcb_read8 m_port_a_read;
	devcb_read8 m_port_b_read;
	devcb_read8 m_port_c_read;
	devcb_write8 m_port_a_write;
	devcb_write8 m_port_b_write;
	devcb_write8 m_port_c_write;
};


// ======================> ppi8255_device_config

class ppi8255_device_config : public device_config,
                              public ppi8255_interface
{
    friend class ppi8255_device;

    // construction/destruction
    ppi8255_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
    // device_config overrides
    virtual void device_config_complete();
};


// ======================> ppi8255_device

class ppi8255_device :  public device_t
{
    friend class ppi8255_device_config;

    // construction/destruction
    ppi8255_device(running_machine &_machine, const ppi8255_device_config &_config);

public:

	UINT8 ppi8255_r(UINT32 offset);
	void ppi8255_w(UINT32 offset, UINT8 data);

	void ppi8255_set_port_read(int which, const devcb_read8 *config) { devcb_resolve_read8(&m_port_read[which], config, this); }
	void ppi8255_set_port_write(int which, const devcb_write8 *config) { devcb_resolve_write8(&m_port_write[which], config, this); }

	void ppi8255_set_port(int which, UINT8 data) { ppi8255_input(which, data); }
	UINT8 ppi8255_get_port(int which) { return m_output[which]; }

	void ppi8255_set_port_a(UINT8 data);
	void ppi8255_set_port_b(UINT8 data);
	void ppi8255_set_port_c(UINT8 data);

	UINT8 ppi8255_get_port_a();
	UINT8 ppi8255_get_port_b();
	UINT8 ppi8255_get_port_c();

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

	static TIMER_CALLBACK( callback );

private:

	void ppi8255_get_handshake_signals(int is_read, UINT8 *result);
	void ppi8255_input(int port, UINT8 data);

	UINT8 ppi8255_read_port(int port);
	void ppi8255_write_port(int port);

	void set_mode(int data, int call_handlers);

	devcb_resolved_read8 m_port_read[3];
	devcb_resolved_write8 m_port_write[3];

	/* mode flags */
	UINT8 m_group_a_mode;
	UINT8 m_group_b_mode;
	UINT8 m_port_a_dir;
	UINT8 m_port_b_dir;
	UINT8 m_port_ch_dir;
	UINT8 m_port_cl_dir;

	/* handshake signals (1=asserted; 0=non-asserted) */
	UINT8 m_obf_a;
	UINT8 m_obf_b;
	UINT8 m_ibf_a;
	UINT8 m_ibf_b;
	UINT8 m_inte_a;
	UINT8 m_inte_b;
	UINT8 m_inte_1;
	UINT8 m_inte_2;

	UINT8 m_in_mask[3];		/* input mask */
	UINT8 m_out_mask[3];	/* output mask */
	UINT8 m_read[3];		/* data read from ports */
	UINT8 m_latch[3];		/* data written to ports */
	UINT8 m_output[3];		/* actual output data */
	UINT8 m_control;		/* mode control word */

    const ppi8255_device_config &m_config;
};


// device type definition
extern const device_type PPI8255;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

READ8_DEVICE_HANDLER( ppi8255_r );
WRITE8_DEVICE_HANDLER( ppi8255_w );

void ppi8255_set_port_a_read( running_device *device, const devcb_read8 *config );
void ppi8255_set_port_b_read( running_device *device, const devcb_read8 *config );
void ppi8255_set_port_c_read( running_device *device, const devcb_read8 *config );

void ppi8255_set_port_a_write( running_device *device, const devcb_write8 *config );
void ppi8255_set_port_b_write( running_device *device, const devcb_write8 *config );
void ppi8255_set_port_c_write( running_device *device, const devcb_write8 *config );

void ppi8255_set_port_a( running_device *device, UINT8 data );
void ppi8255_set_port_b( running_device *device, UINT8 data );
void ppi8255_set_port_c( running_device *device, UINT8 data );

UINT8 ppi8255_get_port_a( running_device *device );
UINT8 ppi8255_get_port_b( running_device *device );
UINT8 ppi8255_get_port_c( running_device *device );

#endif /* __8255PPI_H_ */
