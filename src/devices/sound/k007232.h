// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*********************************************************/
/*    Konami PCM controller                              */
/*********************************************************/

#pragma once

#ifndef __K007232_H__
#define __K007232_H__

#define  KDAC_A_PCM_MAX    (2)      /* Channels per chip */

#define MCFG_K007232_PORT_WRITE_HANDLER(_devcb) \
	devcb = &k007232_device::set_port_write_handler(*device, DEVCB_##_devcb);


class k007232_device : public device_t,
									public device_sound_interface
{
public:
	k007232_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~k007232_device() {}

	template<class _Object> static devcb_base &set_port_write_handler(device_t &device, _Object object) { return downcast<k007232_device &>(device).m_port_write_handler.set_callback(object); }

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
	// internal state
	required_region_ptr<UINT8> m_rom;

	UINT8           m_vol[KDAC_A_PCM_MAX][2]; /* volume for the left and right channel */
	UINT32          m_addr[KDAC_A_PCM_MAX];
	UINT32          m_start[KDAC_A_PCM_MAX];
	UINT32          m_step[KDAC_A_PCM_MAX];
	UINT32          m_bank[KDAC_A_PCM_MAX];
	int             m_play[KDAC_A_PCM_MAX];

	UINT8           m_wreg[0x10]; /* write data */

	UINT32          m_pcmlimit;

	sound_stream *  m_stream;
	UINT32          m_fncode[0x200];
	devcb_write8 m_port_write_handler;
};

extern const device_type K007232;


#endif /* __K007232_H__ */
