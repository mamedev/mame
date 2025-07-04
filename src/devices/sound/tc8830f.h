// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    tc8830f.h - Toshiba TC8830F, CMOS voice recording/reproducing LSI

***************************************************************************/

#ifndef MAME_SOUND_TC8830F_H
#define MAME_SOUND_TC8830F_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tc8830f_device : public device_t,
						public device_sound_interface
{
public:
	// construction/destruction
	tc8830f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void reset();
	void write_p(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;

	virtual void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_stream;

	bool m_playing;
	uint32_t m_address;
	uint32_t m_stop_address;
	uint8_t m_bitcount;
	uint8_t m_bitrate;
	uint8_t m_prevbits;
	int m_delta;
	int m_output;
	uint8_t m_command;
	int m_cmd_rw;
	uint8_t m_phrase;

	required_region_ptr<uint8_t> m_mem;
};


// device type definition
DECLARE_DEVICE_TYPE(TC8830F, tc8830f_device)

#endif // MAME_SOUND_TC8830F_H
