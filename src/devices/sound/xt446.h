// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert

/*************************************************************************************

    Yamaha XT446 : 32-voice polyphonic/multitimbral General MIDI/GS/XG tone modules

    Embedded version of the MU100B

**************************************************************************************/

#ifndef DEVICE_SOUND_XT446_H
#define DEVICE_SOUND_XT446_H

#include "cpu/h8/h8s2655.h"
#include "sound/swp30.h"

class xt446_device : public device_t, public device_mixer_interface
{
public:
	xt446_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void midi_w(int state) { m_midi_serial->rx_w(state); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	required_device<h8s2655_device> m_maincpu;
	required_device<swp30_device> m_swp30;
	required_device<h8_sci_device> m_midi_serial;

	void xt446_iomap(address_map &map);
	void xt446_map(address_map &map);
	void swp30_map(address_map &map);
};

DECLARE_DEVICE_TYPE(XT446, xt446_device)

#endif
