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

#define MCFG_PPI8255_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, PPI8255, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_PPI8255_RECONFIG(_tag, _intrf) \
	MCFG_DEVICE_MODIFY(_tag) \
	MCFG_DEVICE_CONFIG(_intrf)


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


// ======================> ppi8255_device

class ppi8255_device :  public device_t,
						public ppi8255_interface
{
public:
    // construction/destruction
    ppi8255_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	void set_port_read(int which, const devcb_read8 &config) { m_port_read[which].resolve(config, *this); }
	void set_port_write(int which, const devcb_write8 &config) { m_port_write[which].resolve(config, *this); }

	void set_port(int which, UINT8 data) { input(which, data); }
	UINT8 get_port(int which) const { return m_output[which]; }

	void set_port_a(UINT8 data) { set_port(0, data); }
	void set_port_b(UINT8 data) { set_port(1, data); }
	void set_port_c(UINT8 data) { set_port(2, data); }

	UINT8 get_port_a() const { return get_port(0); }
	UINT8 get_port_b() const { return get_port(1); }
	UINT8 get_port_c() const { return get_port(2); }

protected:
    // device-level overrides
    virtual void device_config_complete();
    virtual void device_start();
    virtual void device_reset();
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

	static TIMER_CALLBACK( callback );

private:

	void get_handshake_signals(bool is_read, UINT8 &result);
	void input(int port, UINT8 data);

	UINT8 read_port(int port);
	void write_port(int port);

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
};


// device type definition
extern const device_type PPI8255;


#endif /* __8255PPI_H_ */
