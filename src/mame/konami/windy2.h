// license:BSD-3-Clause
// copyright-holders:windyfairy
/*
 * Konami Windy2 I/O (JVS)
 *
 */
#ifndef MAME_KONAMI_WINDY2_H
#define MAME_KONAMI_WINDY2_H

#pragma once

#include "machine/jvsdev.h"
#include "machine/timer.h"

class windy2_device : public jvs_device
{
public:
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	windy2_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t player_count, uint8_t switch_count);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// JVS device overrides
	virtual const char *device_id() override;
	virtual uint8_t command_format_version() override;
	virtual uint8_t jvs_standard_version() override;
	virtual uint8_t comm_method_version() override;
	virtual void function_list(uint8_t*& buf) override;
	virtual bool switches(uint8_t*& buf, uint8_t count_players, uint8_t bytes_per_switch) override;
	virtual bool coin_counters(uint8_t*& buf, uint8_t count) override;

private:
	required_ioport m_test_port;
	required_ioport_array<2> m_player_ports;

	uint16_t m_coin_counter[2];
	const uint8_t m_player_count, m_switch_count;
};

/////////////////////////////////////////////

// Default Windy 2 cabinet panel setup for 2 players with 3 buttons + 4 levers + start + service per player
// Corresponds to the dipswitch setting of 01000000 on I/O board
class windy2_2l6b_device : public windy2_device
{
public:
	windy2_2l6b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	windy2_2l6b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&jvs_host_tag)
		: windy2_2l6b_device(mconfig, tag, owner, clock)
	{
		host.set_tag(std::forward<T>(jvs_host_tag));
	}

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

/////////////////////////////////////////////

class windy2_quiz_device : public windy2_device
{
public:
	windy2_quiz_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	windy2_quiz_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&jvs_host_tag)
		: windy2_quiz_device(mconfig, tag, owner, clock)
	{
		host.set_tag(std::forward<T>(jvs_host_tag));
	}

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

/////////////////////////////////////////////

class windy2_2l8b_device : public windy2_device
{
public:
	windy2_2l8b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	windy2_2l8b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&jvs_host_tag)
		: windy2_2l8b_device(mconfig, tag, owner, clock)
	{
		host.set_tag(std::forward<T>(jvs_host_tag));
	}

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

/////////////////////////////////////////////

class windy2_1l6b_device : public windy2_device
{
public:
	windy2_1l6b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	windy2_1l6b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&jvs_host_tag)
		: windy2_1l6b_device(mconfig, tag, owner, clock)
	{
		host.set_tag(std::forward<T>(jvs_host_tag));
	}

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

/////////////////////////////////////////////

class windy2_2l12b_device : public windy2_device
{
public:
	windy2_2l12b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	windy2_2l12b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&jvs_host_tag)
		: windy2_2l12b_device(mconfig, tag, owner, clock)
	{
		host.set_tag(std::forward<T>(jvs_host_tag));
	}

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(KONAMI_WINDY2_JVS_IO_2L6B_PANEL,  windy2_2l6b_device)
DECLARE_DEVICE_TYPE(KONAMI_WINDY2_JVS_IO_QUIZ_PANEL,  windy2_quiz_device)
DECLARE_DEVICE_TYPE(KONAMI_WINDY2_JVS_IO_2L8B_PANEL,  windy2_2l8b_device)
DECLARE_DEVICE_TYPE(KONAMI_WINDY2_JVS_IO_1L6B_PANEL,  windy2_1l6b_device)
DECLARE_DEVICE_TYPE(KONAMI_WINDY2_JVS_IO_2L12B_PANEL, windy2_2l12b_device)

#endif // MAME_KONAMI_WINDY2_H
