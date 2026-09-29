// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    cdplayer.h

    Audio CD player transport

*********************************************************************/

#ifndef MAME_IMAGEDEV_CDPLAYER_H
#define MAME_IMAGEDEV_CDPLAYER_H

#pragma once

#include "cdromimg.h"


class cd_player_device : public cdrom_image_device
{
public:
	static constexpr u32 SAMPLE_RATE = 44100;
	static constexpr u32 FRAMES_PER_SECOND = 75;

	enum class transport : u8 { STOPPED, PLAYING, PAUSED };

	using sample_delegate = device_delegate<void (s16 left, s16 right)>;

	cd_player_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename... T> void set_sample_callback(T &&... args) { m_sample_cb.set(std::forward<T>(args)...); }

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	void play();
	void pause();
	void stop();
	void previous_track();
	void next_track();
	void select_track(int track);

	transport state() const { return m_state; }
	int track() const { return m_track + 1; }
	int track_count() const { return get_last_track(); }
	u32 track_elapsed_frames() const;
	u32 track_length_frames() const;

protected:
	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(sample_tick);

	u32 track_start(int track) const;
	u32 disc_end() const;
	void seek_track(int track);
	void set_state(transport state);

	sample_delegate m_sample_cb;
	emu_timer *m_sample_timer;

	transport m_state;
	s32 m_track;
	u32 m_lba;
	u32 m_sample;

	s32 m_cached_lba;
	u8 m_sector[cdrom_file::MAX_SECTOR_DATA];
};

// zero-crossing comparator with hysteresis for turning a line output back into a logic level,
// locating each crossing on the band-limited signal rather than between the raw samples
class cd_audio_comparator
{
public:
	static constexpr int TAPS = 16;

	cd_audio_comparator(s16 hysteresis);

	bool state() const { return m_state; }

	// returns true when the output changes, with the zero crossing's position in 0..1 after the
	// sample TAPS / 2 samples back; callers delaying every edge by that much keep them in step
	bool update(s16 sample, double &position);

	void register_save(device_t &device, int index = 0);

private:
	s16 const m_hysteresis;
	bool m_state;
	s16 m_history[TAPS];
};

DECLARE_DEVICE_TYPE(CD_PLAYER, cd_player_device)

using cd_player_device_enumerator = device_type_enumerator<cd_player_device>;

#endif // MAME_IMAGEDEV_CDPLAYER_H
