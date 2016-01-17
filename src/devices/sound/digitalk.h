// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef _DIGITALKER_H_
#define _DIGITALKER_H_


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DIGITALKER_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, DIGITALKER, _clock)
#define MCFG_DIGITALKER_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, DIGITALKER, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> digitalker_device

class digitalker_device : public device_t,
							public device_sound_interface
{
public:
	digitalker_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~digitalker_device() { }

	void digitalker_0_cs_w(int line);
	void digitalker_0_cms_w(int line);
	void digitalker_0_wr_w(int line);
	int digitalker_0_intr_r();

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	DECLARE_WRITE8_MEMBER(digitalker_data_w);

private:
	void digitalker_write(UINT8 *adr, UINT8 vol, INT8 dac);
	UINT8 digitalker_pitch_next(UINT8 val, UINT8 prev, int step);
	void digitalker_set_intr(UINT8 intr);
	void digitalker_start_command(UINT8 cmd);
	void digitalker_step_mode_0();
	void digitalker_step_mode_1();
	void digitalker_step_mode_2();
	void digitalker_step_mode_3();
	void digitalker_step();
	void digitalker_cs_w(int line);
	void digitalker_cms_w(int line);
	void digitalker_wr_w(int line);
	int digitalker_intr_r();
	void digitalker_register_for_save();

private:
	const UINT8 *m_rom;
	sound_stream *m_stream;

	// Port/lines state
	UINT8 m_data;
	UINT8 m_cs;
	UINT8 m_cms;
	UINT8 m_wr;
	UINT8 m_intr;

	// Current decoding state
	UINT16 m_bpos;
	UINT16 m_apos;

	UINT8 m_mode;
	UINT8 m_cur_segment;
	UINT8 m_cur_repeat;
	UINT8 m_segments;
	UINT8 m_repeats;

	UINT8 m_prev_pitch;
	UINT8 m_pitch;
	UINT8 m_pitch_pos;

	UINT8 m_stop_after;
	UINT8 m_cur_dac;
	UINT8 m_cur_bits;

	// Zero-range size
	UINT32 m_zero_count; // 0 for done

	// Waveform and current index in it
	UINT8 m_dac_index; // 128 for done
	INT16 m_dac[128];
};

extern const device_type DIGITALKER;


#endif
