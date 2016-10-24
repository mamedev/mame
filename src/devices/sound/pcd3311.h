// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    PCD3311 DTMF/modem/musical tone generator emulation

**********************************************************************
                            _____   _____
                  OSCI   1 |*    \_/     | 16  Vdd
                  OSCO   2 |             | 15  Vss
                  MODE   3 |             | 14  D4
                    D5   4 |  PCD3311T   | 13  N/C
                   N/C   5 |             | 12  D3
                STROBE   6 |             | 11  D2
                  TONE   7 |             | 10  D1/SDA
                    A0   8 |_____________| 9   D0/SCL

**********************************************************************/

#pragma once

#ifndef __PCD3311__
#define __PCD3311__

#include "emu.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pcd3311_t

class pcd3311_t :  public device_t,
				   public device_sound_interface
{
public:
	// construction/destruction
	pcd3311_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_data = data; }
	void strobe_w(int state) { m_strobe = state; }
	void mode_w(int state) { m_mode = state; }
	void a0_w(int state) { m_a0 = state; }

protected:
	// device-level overrides
	virtual void device_start() override;

	// internal callbacks
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	int m_a0;
	int m_mode;
	int m_strobe;
	uint8_t m_data;
};


// device type definition
extern const device_type PCD3311;



#endif
