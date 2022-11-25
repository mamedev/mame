// license:BSD-3-Clause
// copyright-holders:windyfairy
/*
 * Konami Windy2 I/O (JVS)
 *
 */
#ifndef MAME_MACHINE_WINDY2_H
#define MAME_MACHINE_WINDY2_H

#pragma once

#include "machine/jvsdev.h"
#include "machine/timer.h"

class windy2_device : public jvs_device
{
public:
	enum : uint8_t {
		// Corresponds to positions on DIPSW8 that controls panel type
		PANEL_2L6B  = 0b00000010, // "Windy II Standard" 2L6B Panel (2x 1L3B)
		//PANEL_MAHJONG  = 0b11111111, // Mahjong Panel (23B) (GU707-JA), dipswitch setting unknown
		PANEL_QUIZ  = 0b00000000, // Quiz Panel (4B) (GU707-JB)
		PANEL_2L8B  = 0b00000011, // 2L8B Panel (2x 1L4B) (GU707-JC)
		PANEL_1L6B  = 0b10010101, // 1L6B Panel (GU707-JD)
		PANEL_2L12B = 0b00010101, // 2L12B Panel (2x 1L6B) (GU707-JG)
	};

	template <typename T>
	windy2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&jvs_host_tag, uint8_t panel_type)
		: windy2_device(mconfig, tag, owner, clock)
	{
		host.set_tag(std::forward<T>(jvs_host_tag));
		m_panel_type = panel_type;
	}

	windy2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	template <uint8_t First> void set_port_tags() { }

	virtual void device_start() override;
	virtual void device_reset() override;

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

	uint8_t m_panel_type;
	uint16_t m_coin_counter[2];
};

DECLARE_DEVICE_TYPE(KONAMI_WINDY2_JVS_IO, windy2_device)

#endif // MAME_MACHINE_WINDY2_H
