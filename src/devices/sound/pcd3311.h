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

#ifndef MAME_SOUND_PCD3311_H
#define MAME_SOUND_PCD3311_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pcd3311_device

class pcd3311_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	pcd3311_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(uint8_t data) { m_data = data; }
	void strobe_w(int state);
	void mode_w(int state) { m_mode = state; }
	void a0_w(int state);

	void scl_w(int state);
	void sda_w(int state);
	int sda_r();

	// use in memory maps to avoid strobe logic.
	void write_direct(uint8_t data) { load_data(data); }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr uint8_t PCD3311_SLAVE_ADDRESS = 0x48;

	enum { STATE_IDLE, STATE_DEVSEL, STATE_DATAIN };

	void load_data(uint8_t data);

	int m_mode;
	int m_strobe;
	uint8_t m_data;

	int m_slave_address;
	int m_scl;
	int m_sdaw;
	int m_sdar;
	int m_state;
	int m_bits;
	int m_shift;
	int m_devsel;

	sound_stream *m_stream;               // stream number
	uint32_t m_freq[2];                   // current frequencies
	int32_t m_incr[2];                    // initial wave state
	sound_stream::sample_t m_signal[2];   // current signals
};


// device type definition
DECLARE_DEVICE_TYPE(PCD3311, pcd3311_device)

#endif // MAME_SOUND_PCD3311_H
