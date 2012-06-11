/***************************************************************************

    AMD AM9517A/8237A Multimode DMA Controller emulation

    Copyright the MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************
                            _____   _____
                  _IOR   1 |*    \_/     | 40  A7
                  _IOW   2 |             | 39  A6
                 _MEMR   3 |             | 38  A5
                 _MEMW   4 |             | 37  A4
                     *   5 |             | 36  _EOP
                 READY   6 |             | 35  A3
                  HACK   7 |             | 34  A2
                 ADSTB   8 |             | 33  A1
                   AEN   9 |             | 32  A0
                  HREQ  10 |   AM9517A   | 31  Vcc
                   _CS  11 |    8237A    | 30  DB0
                   CLK  12 |             | 29  DB1
                 RESET  13 |             | 28  DB2
                 DACK2  14 |             | 27  DB3
                 DACK3  15 |             | 26  DB4
                 DREQ3  16 |             | 25  DACK0
                 DREQ2  17 |             | 24  DACK1
                 DREQ1  18 |             | 23  DB5
                 DREQ0  19 |             | 22  DB6
                   Vss  20 |_____________| 21  DB7

***************************************************************************/

#pragma once

#ifndef __AM9517A__
#define __AM9517A__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_AM9517A_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, AM9517A, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_I8237_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, AM9517A, _clock) \
	MCFG_DEVICE_CONFIG(_config)


#define AM9517A_INTERFACE(_name) \
	const am9517a_interface (_name) =

#define I8237_INTERFACE(_name) \
	const am9517a_interface (_name) =



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> am9517a_interface

struct am9517a_interface
{
	devcb_write_line	m_out_hreq_cb;
	devcb_write_line	m_out_eop_cb;

	devcb_read8			m_in_memr_cb;
	devcb_write8		m_out_memw_cb;

	devcb_read8			m_in_ior_cb[4];
	devcb_write8		m_out_iow_cb[4];
	devcb_write_line	m_out_dack_cb[4];
};


// ======================> am9517a_device

class am9517a_device :  public device_t,
					    public device_execute_interface,
					    public am9517a_interface
{
public:
	// construction/destruction
	am9517a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( hack_w );
	DECLARE_WRITE_LINE_MEMBER( ready_w );
	DECLARE_WRITE_LINE_MEMBER( eop_w );

	DECLARE_WRITE_LINE_MEMBER( dreq0_w );
	DECLARE_WRITE_LINE_MEMBER( dreq1_w );
	DECLARE_WRITE_LINE_MEMBER( dreq2_w );
	DECLARE_WRITE_LINE_MEMBER( dreq3_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void execute_run();

    int m_icount;

private:
	inline void dma_request(int channel, int state);
	inline bool is_request_active(int channel);
	inline bool is_software_request_active(int channel);
	inline void set_hreq(int state);
	inline void set_dack();
	inline void set_eop(int state);
	inline int get_state1(bool msb_changed);
	inline void dma_read();
	inline void dma_write();
	inline void dma_advance();
	inline void end_of_process();

	devcb_resolved_write_line	m_out_hreq_func;
	devcb_resolved_write_line	m_out_eop_func;
	devcb_resolved_read8		m_in_memr_func;
	devcb_resolved_write8		m_out_memw_func;

	struct
	{
		devcb_resolved_read8		m_in_ior_func;
		devcb_resolved_write8		m_out_iow_func;
		devcb_resolved_write_line	m_out_dack_func;

		UINT16 m_address;
		UINT16 m_count;
		UINT16 m_base_address;
		UINT16 m_base_count;
		UINT8 m_mode;
	} m_channel[4];

	int m_msb;
	int m_hreq;
	int m_hack;
	int m_ready;
	int m_eop;
	int m_state;
	int m_current_channel;
	int m_last_channel;
	UINT8 m_command;
	UINT8 m_mask;
	UINT8 m_status;
	UINT8 m_temp;
	UINT8 m_request;
};


// device type definition
extern const device_type AM9517A;



#endif
