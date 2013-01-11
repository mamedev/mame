/***************************************************************************

    Intel 8257 Programmable DMA Controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************
                            _____   _____
                 _I/OR   1 |*    \_/     | 40  A7
                 _I/OW   2 |             | 39  A6
                 _MEMR   3 |             | 38  A5
                 _MEMW   4 |             | 37  A4
                  MARK   5 |             | 36  TC
                 READY   6 |             | 35  A3
                  HLDA   7 |             | 34  A2
                 ADSTB   8 |             | 33  A1
                   AEN   9 |             | 32  A0
                   HRQ  10 |     8257    | 31  Vcc
                   _CS  11 |             | 30  D0
                   CLK  12 |             | 29  D1
                 RESET  13 |             | 28  D2
                _DACK2  14 |             | 27  D3
                _DACK3  15 |             | 26  D4
                  DRQ3  16 |             | 25  _DACK0
                  DRQ2  17 |             | 24  _DACK1
                  DRQ1  18 |             | 23  D5
                  DRQ0  19 |             | 22  D6
                   GND  20 |_____________| 21  D7

***************************************************************************/

#pragma once

#ifndef __I8257__
#define __I8257__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_I8257_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, I8257, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define I8257_INTERFACE(_name) \
	const i8257_interface (_name) =

#define I8257_NUM_CHANNELS      (4)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> i8257_interface

struct i8257_interface
{
	devcb_write_line    m_out_hrq_cb;
	devcb_write_line    m_out_tc_cb;
	devcb_write_line    m_out_mark_cb;

	/* accessors to main memory */
	devcb_read8         m_in_memr_cb; // TODO m_in_memr_cb[I8257_NUM_CHANNELS];
	devcb_write8        m_out_memw_cb; // TODO m_out_memw_cb[I8257_NUM_CHANNELS];

	/* channel accesors */
	devcb_read8         m_in_ior_cb[I8257_NUM_CHANNELS];
	devcb_write8        m_out_iow_cb[I8257_NUM_CHANNELS];
};



// ======================> i8257_device

class i8257_device :  public device_t,
						public i8257_interface
{
public:
	// construction/destruction
	i8257_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	/* register access */
	UINT8 i8257_r(UINT32 offset);
	void i8257_w(UINT32 offset, UINT8 data);

	/* data request */
	void i8257_drq_w(int channel, int state);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	static const device_timer_id TIMER_OPERATION = 0;
	static const device_timer_id TIMER_MSBFLIP = 1;
	static const device_timer_id TIMER_DRQ_SYNC = 2;

	int i8257_do_operation(int channel);
	void i8257_update_status();
	void i8257_prepare_msb_flip();

	devcb_resolved_write_line   m_out_hrq_func;
	devcb_resolved_write_line   m_out_tc_func;
	devcb_resolved_write_line   m_out_mark_func;
	devcb_resolved_read8        m_in_memr_func;
	devcb_resolved_write8       m_out_memw_func;
	devcb_resolved_read8        m_in_ior_func[I8257_NUM_CHANNELS];
	devcb_resolved_write8       m_out_iow_func[I8257_NUM_CHANNELS];

	emu_timer *m_timer;
	emu_timer *m_msbflip_timer;

	UINT16 m_registers[I8257_NUM_CHANNELS*2];

	UINT16 m_address[I8257_NUM_CHANNELS];
	UINT16 m_count[I8257_NUM_CHANNELS];
	UINT8  m_rwmode[I8257_NUM_CHANNELS];

	UINT8 m_mode;
	UINT8 m_rr;

	UINT8 m_msb;
	UINT8 m_drq;

	/* bits  0- 3 :  Terminal count for channels 0-3 */
	UINT8 m_status;
};


// device type definition
extern const device_type I8257;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* register access */
DECLARE_READ8_DEVICE_HANDLER( i8257_r );
DECLARE_WRITE8_DEVICE_HANDLER( i8257_w );

/* hold acknowledge */
WRITE_LINE_DEVICE_HANDLER( i8257_hlda_w );

/* ready */
WRITE_LINE_DEVICE_HANDLER( i8257_ready_w );

/* data request */
WRITE_LINE_DEVICE_HANDLER( i8257_drq0_w );
WRITE_LINE_DEVICE_HANDLER( i8257_drq1_w );
WRITE_LINE_DEVICE_HANDLER( i8257_drq2_w );
WRITE_LINE_DEVICE_HANDLER( i8257_drq3_w );

#endif
