/**********************************************************************

    MM74C922/MM74C923 16/20-Key Encoder emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                ROW Y1   1 |*    \_/     | 18  Vcc
                ROW Y2   2 |             | 17  DATA OUT A
                ROW Y3   3 |             | 16  DATA OUT B
                ROW Y4   4 |             | 15  DATA OUT C
            OSCILLATOR   5 |  MM74C922   | 14  DATA OUT D
        KEYBOUNCE MASK   6 |             | 13  _OUTPUT ENABLE
             COLUMN X4   7 |             | 12  DATA AVAILABLE
             COLUMN X3   8 |             | 11  COLUMN X1
                   GND   9 |_____________| 10  COLUMN X2

                            _____   _____
                ROW Y1   1 |*    \_/     | 20  Vcc
                ROW Y2   2 |             | 19  DATA OUT A
                ROW Y3   3 |             | 18  DATA OUT B
                ROW Y4   4 |             | 17  DATA OUT C
                ROW Y5   5 |  MM74C923   | 16  DATA OUT D
            OSCILLATOR   6 |             | 15  DATA OUT E
        KEYBOUNCE MASK   7 |             | 14  _OUTPUT ENABLE
             COLUMN X4   8 |             | 13  DATA AVAILABLE
             COLUMN X3   9 |             | 12  COLUMN X1
                   GND  10 |_____________| 11  COLUMN X2

**********************************************************************/

#pragma once

#ifndef __MM74C922__
#define __MM74C922__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MM74C922_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, MM74C922, 0) \
	MCFG_DEVICE_CONFIG(_config) \
	mm74c922_device::static_set_config(*device, 4);


#define MCFG_MM74C923_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, MM74C923, 0) \
	MCFG_DEVICE_CONFIG(_config) \
	mm74c922_device::static_set_config(*device, 5);


#define MM74C922_INTERFACE(name) \
	const mm74c922_interface (name)=


#define MM74C923_INTERFACE(name) \
	const mm74c922_interface (name)=



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mm74c922_interface

struct mm74c922_interface
{
	double              m_cap_osc;
	double              m_cap_debounce;

	devcb_write_line    m_out_da_cb;

	devcb_read8         m_in_x1_cb;
	devcb_read8         m_in_x2_cb;
	devcb_read8         m_in_x3_cb;
	devcb_read8         m_in_x4_cb;
	devcb_read8         m_in_x5_cb;
};


// ======================> mm74c922_device

class mm74c922_device :  public device_t,
							public mm74c922_interface
{
public:
	// construction/destruction
	mm74c922_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_config(device_t &device, int max_y);

	UINT8 data_out_r();

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	inline void change_output_lines();
	inline void clock_scan_counters();
	inline void detect_keypress();

	int m_max_y;

	devcb_resolved_write_line   m_out_da_func;
	devcb_resolved_read8        m_in_x_func[5];

	int m_inhibit;              // scan counter clock inhibit
	int m_x;                    // currently scanned column
	int m_y;                    // latched row

	UINT8 m_data;               // data latch

	int m_da;                   // data available flag
	int m_next_da;              // next value of data available flag

	// timers
	emu_timer *m_scan_timer;    // keyboard scan timer
};


// device type definition
extern const device_type MM74C922;
extern const device_type MM74C923;



#endif
