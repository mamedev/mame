// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Frank Palazzolo, Sean Riddle
/*****************************************************************************
 *
 * fairchild/channelf.h
 *
 ****************************************************************************/

#ifndef MAME_FAIRCHILD_CHANNELF_A_H
#define MAME_FAIRCHILD_CHANNELF_A_H

class channelf_sound_device : public device_t, public device_sound_interface
{
public:
	channelf_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void sound_w(int mode);
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
private:
	// internal state
	sound_stream *m_channel;
	int m_sound_mode;
	int m_incr;
	float m_decay_mult;
	int m_envelope;
	uint32_t m_sample_counter;
	int m_forced_ontime;           //  added for improved sound
	int m_min_ontime;              //  added for improved sound

};

DECLARE_DEVICE_TYPE(CHANNELF_SOUND, channelf_sound_device)

#endif // MAME_FAIRCHILD_CHANNELF_A_H
