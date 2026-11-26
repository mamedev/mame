// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_BUS_NSCSI_CR504_H
#define MAME_BUS_NSCSI_CR504_H

#pragma once

#include "cpu/mn1880/mn1880.h"
#include "imagedev/cdromimg.h"
#include "machine/mn66403.h"

class cr504_device : public device_t, public nscsi_slot_card_interface, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::DISK | feature::SOUND; }

	cr504_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	u8 port0_r();
	u8 port1_r();
	u8 port2_r();
	u8 port8_r();
	u8 port9_r();
	void port1_w(u8 data);
	void port3_w(u8 data);
	void port9_w(u8 data);
	void servo_serial_w(u8 data);
	u8 servo_serial_r();
	u8 subcode_serial_r();
	u8 sled_level_r();
	void sled_pwm_w(int state);

	void servo_command();
	bool disc_present() const;
	void make_subcode();
	u32 optical_track() const;
	bool read_sector(u8 *data, u32 track, u32 type);
	void move_pickup(int tracks);

	TIMER_CALLBACK_MEMBER(tray_tick);
	TIMER_CALLBACK_MEMBER(optical_tick);

	required_device<mn1880023_device> m_mcu;
	required_device<mn664031_device> m_controller;
	required_device<cdrom_image_device> m_image;

	sound_stream *m_stream;
	emu_timer *m_tray_timer;
	emu_timer *m_optical_timer;

	u8 m_audio_sector[2352];
	u16 m_audio_sample;
	u8 m_audio_control;
	u8 m_audio_gain[2];
	u8 m_audio_route;
	u8 m_port1;
	u8 m_port3;
	u8 m_port9;
	u8 m_serial_out[3];
	u8 m_serial_count;
	u8 m_servo_registers[256];
	u8 m_servo_read;
	bool m_servo_sense;
	u8 m_servo_status;
	bool m_servo_frame_reset;
	u8 m_subcode[10];
	u8 m_subcode_index;
	u16 m_toc_index;
	s32 m_frame;
	s8 m_traverse;
	u8 m_speed;
	u8 m_board_control;
	double m_sled_fraction;
	bool m_tracking;
	bool m_focus;
	bool m_tray_open;
	u32 m_image_sequence;
};

DECLARE_DEVICE_TYPE(CR504, cr504_device)

#endif // MAME_BUS_NSCSI_CR504_H
