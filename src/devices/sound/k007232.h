// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*********************************************************/
/*    Konami PCM controller                              */
/*********************************************************/

#ifndef MAME_SOUND_K007232_H
#define MAME_SOUND_K007232_H

#pragma once

class k007232_device : public device_t, public device_sound_interface
{
public:
	k007232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto port_write() { return m_port_write_handler.bind(); }

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

	/*
	The 007232 has two channels and produces two outputs. The volume control
	is external, however to make it easier to use we handle that inside the
	emulation. You can control volume and panning: for each of the two channels
	you can set the volume of the two outputs. If panning is not required,
	then volumeB will be 0 for channel 0, and volumeA will be 0 for channel 1.
	Volume is in the range 0-255.
	*/
	void set_volume(int channel,int volumeA,int volumeB);

	void set_bank( int chABank, int chBBank );

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	void KDAC_A_make_fncode();

private:
	static constexpr unsigned KDAC_A_PCM_MAX = 2;      /* Channels per chip */

	// internal state
	required_region_ptr<uint8_t> m_rom;

	uint8_t           m_vol[KDAC_A_PCM_MAX][2]; /* volume for the left and right channel */
	uint32_t          m_addr[KDAC_A_PCM_MAX];
	uint32_t          m_start[KDAC_A_PCM_MAX];
	uint32_t          m_step[KDAC_A_PCM_MAX];
	uint32_t          m_bank[KDAC_A_PCM_MAX];
	int             m_play[KDAC_A_PCM_MAX];

	uint8_t           m_wreg[0x10]; /* write data */

	uint32_t          m_pcmlimit;

	sound_stream *  m_stream;
	uint32_t          m_fncode[0x200];
	devcb_write8 m_port_write_handler;
};

DECLARE_DEVICE_TYPE(K007232, k007232_device)

#endif // MAME_SOUND_K007232_H
