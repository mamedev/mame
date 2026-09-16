// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

  cassette.h - Atari 8 bit cassette player(s)


Known cassette players:
- Atari XC11
- Atari XC12 (no SIO connection for an additional device)

***************************************************************************/

#ifndef MAME_BUS_A800_CASSETTE_H
#define MAME_BUS_A800_CASSETTE_H

#pragma once


#include "a8sio.h"
#include "imagedev/cassette.h"


class a8sio_cassette_device
	: public device_t
	, public device_a8sio_card_interface
{
public:
	// construction/destruction
	a8sio_cassette_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void motor_w(int state) override;

protected:
	a8sio_cassette_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(read_tick);

	required_device<cassette_image_device> m_cassette;
	emu_timer *m_read_timer;

	uint8_t m_old_cass_signal;
	uint8_t m_signal_count;
};

// device type definition
DECLARE_DEVICE_TYPE(A8SIO_CASSETTE, a8sio_cassette_device)


#endif // MAME_BUS_A800_CASSETTE_H
