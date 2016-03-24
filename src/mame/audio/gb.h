// license:BSD-3-Clause
// copyright-holders:Anthony Kruize
#ifndef __GBSOUND_H__
#define __GBSOUND_H__


#define MAX_FREQUENCIES 2048


struct SOUND
{
	/* Common */
	UINT8  on;
	UINT8  channel;
	INT32  length;
	INT32  pos;
	UINT32 period;
	INT32  count;
	INT8   mode;
	/* Mode 1, 2, 3 */
	INT8   duty;
	/* Mode 1, 2, 4 */
	INT32  env_value;
	INT8   env_direction;
	INT32  env_length;
	INT32  env_count;
	INT8   signal;
	/* Mode 1 */
	UINT32 frequency;
	INT32  swp_shift;
	INT32  swp_direction;
	INT32  swp_time;
	INT32  swp_count;
	/* Mode 3 */
	INT8   level;
	UINT8  offset;
	UINT32 dutycount;
	/* Mode 4 */
	INT32  ply_step;
	INT16  ply_value;
};

struct SOUNDC
{
	UINT8 on;
	UINT8 vol_left;
	UINT8 vol_right;
	UINT8 mode1_left;
	UINT8 mode1_right;
	UINT8 mode2_left;
	UINT8 mode2_right;
	UINT8 mode3_left;
	UINT8 mode3_right;
	UINT8 mode4_left;
	UINT8 mode4_right;
};


class gameboy_sound_device : public device_t,
									public device_sound_interface
{
public:
	gameboy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(sound_r);
	DECLARE_READ8_MEMBER(wave_r);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(wave_w);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;

	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	void sound_w_internal(int offset, UINT8 data);

	sound_stream *m_channel;
	int m_rate;

	INT32 m_env_length_table[8];
	INT32 m_swp_time_table[8];
	UINT32 m_period_table[MAX_FREQUENCIES];
	UINT32 m_period_mode3_table[MAX_FREQUENCIES];
	UINT32 m_period_mode4_table[8][16];
	UINT32 m_length_table[64];
	UINT32 m_length_mode3_table[256];

	struct SOUND  m_snd_1;
	struct SOUND  m_snd_2;
	struct SOUND  m_snd_3;
	struct SOUND  m_snd_4;
	struct SOUNDC m_snd_control;

	UINT8 m_snd_regs[0x30];
};

extern const device_type GAMEBOY;

#endif
