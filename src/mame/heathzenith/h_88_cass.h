// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit H-88-5 Cassette Interface card

****************************************************************************/

#ifndef MAME_HEATHKIT_H_88_CASS_H
#define MAME_HEATHKIT_H_88_CASS_H

#pragma once

#include "imagedev/cassette.h"
#include "machine/i8251.h"
#include "machine/timer.h"

#include "formats/h8_cas.h"


class heath_h_88_cass_device : public device_t
{
public:
	heath_h_88_cass_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void write(offs_t reg, u8 val);
	u8 read(offs_t reg);

protected:

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void uart_rts(u8 data);
	void uart_tx_empty(u8 data);

	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);

	required_device<i8251_device>          m_uart;
	required_device<cassette_image_device> m_cass_player;
	required_device<cassette_image_device> m_cass_recorder;

	u8                                     m_cass_data[4];
	bool                                   m_cassbit;
	bool                                   m_cassold;
};

DECLARE_DEVICE_TYPE(HEATH_H88_CASS, heath_h_88_cass_device)


#endif // MAME_HEATHKIT_H_88_CASS_H
