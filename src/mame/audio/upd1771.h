// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    NEC uPD1771

**********************************************************************/

#ifndef __UPD1771_H__
#define __UPD1771_H__

#include "emu.h"

#define MAX_PACKET_SIZE 0x8000

#define MCFG_UPD1771_ACK_HANDLER(_devcb) \
	devcb = &upd1771c_device::set_ack_handler(*device, DEVCB_##_devcb);


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class upd1771c_device : public device_t,
						public device_sound_interface
{
public:
	upd1771c_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~upd1771c_device() {}

	template<class _Object> static devcb_base &set_ack_handler(device_t &device, _Object object) { return downcast<upd1771c_device &>(device).m_ack_handler.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	WRITE_LINE_MEMBER( pcm_write );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	sound_stream *m_channel;
	devcb_write_line m_ack_handler;
	emu_timer *m_timer;

	TIMER_CALLBACK_MEMBER(ack_callback);

	UINT8   m_packet[MAX_PACKET_SIZE];
	UINT32  m_index;
	UINT8   m_expected_bytes;

	UINT8   m_state;//0:silence, 1 noise, 2 tone
	UINT8   m_pc3;

	//tone
	UINT8    m_t_timbre; //[0;  7]
	UINT8    m_t_offset; //[0; 32]
	UINT16   m_t_period; //[0;255]
	UINT8    m_t_volume; //[0; 31]
	UINT8    m_t_tpos;//timbre pos
	UINT16   m_t_ppos;//period pos

	//noise wavetable LFSR
	UINT8    m_nw_timbre; //[0;  7]
	UINT8    m_nw_volume; //[0; 31]
	UINT32   m_nw_period;
	UINT32   m_nw_tpos;   //timbre pos
	UINT32   m_nw_ppos;   //period pos

	//noise pulse components
	UINT8    m_n_value[3];  //[0;1]
	UINT16   m_n_volume[3]; //[0; 31]
	UINT32   m_n_period[3];
	UINT32   m_n_ppos[3];   //period pos
};

extern const device_type UPD1771C;


#endif /* __UPD1771_H__ */
