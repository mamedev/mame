// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_BUS_JVS_JVSHLE_H
#define MAME_BUS_JVS_JVSHLE_H

#pragma once

#include "jvs.h"

class jvs_hle_device :
	public device_t,
	public device_jvs_interface
{
protected:
	jvs_hle_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	// device_t
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_jvs_interface
	virtual void rxd(int state) override;

	virtual const char *device_id();
	virtual uint8_t command_revision();
	virtual uint8_t jvs_revision();
	virtual uint8_t communication_revision();
	virtual uint8_t player_count() { return 0; }
	virtual uint8_t switch_count() { return 0; }
	virtual uint32_t switch_mask();
	virtual uint8_t coin_slots() { return 0; }
	virtual uint8_t coin_offset() { return 1; }
	virtual uint8_t analog_input_channels() { return 0; }
	virtual uint8_t analog_input_bits() { return 0; }
	virtual uint8_t rotary_input_channels() { return 0; }
	virtual uint8_t screen_position_input_channels() { return 0; }
	virtual uint8_t screen_position_input_xbits() { return 0; }
	virtual uint8_t screen_position_input_ybits() { return 0; }
	virtual uint8_t output_slots() { return 0; }
	virtual uint8_t analog_output_channels() { return 0; }
	virtual void feature_check();
	virtual void execute(uint8_t command);
	void status_code(uint8_t status_code);
	bool consume(int size, const uint8_t *&parameters);
	bool produce(int size, uint8_t *&response);

	template<typename T, typename U> constexpr T jvs_bitmask(U n)
	{
		return make_bitmask<T>((n + 7) & ~7) ^ (make_bitmask<T>((-n) & 7) << (n & ~7));
	}

	struct CommandCode { enum : uint8_t
	{
		Reset = 0xf0,
		SetAddress = 0xf1,
		CommMethodChange = 0xf2,

		IOIDENT = 0x10,
		CMDREV = 0x11,
		JVSREV = 0x12,
		COMMVER = 0x13,
		FEATCHK = 0x14,
		MAINID = 0x15,

		SWINP = 0x20,
		COININP = 0x21,
		ANLINP = 0x22,
		ROTINP = 0x23,
		KEYINP = 0x24,
		SCRPOSINP = 0x25,
		MISCSWINP = 0x26,

		PAYCNT = 0x2e,
		RETRANSMIT = 0x2f,
		COINDEC = 0x30,
		PAYINC = 0x31,
		OUTPUT1 = 0x32,
		ANLOUT = 0x33,
		CHAROUT = 0x34,
		COININC = 0x35,
		PAYDEC = 0x36,
		OUTPUT2 = 0x37,
		OUTPUT3 = 0x38,
	}; };

	struct StatusCode { enum : uint8_t
	{
		Normal = 0x01,
		UnknownCommand = 0x02,
		ChecksumError = 0x03,
		AcknowledgeOverflow = 0x04,
	}; };

	struct ReportCode { enum : uint8_t
	{
		Normal = 0x01,
		IncorrectNumberOfParameters = 0x02,
		InvalidParameter = 0x03,
		Busy = 0x04,
	}; };

	struct FunctionCode { enum : uint8_t
	{
		End = 0x00,

		SwitchInput = 0x01,
		CoinInput = 0x02,
		AnalogInput = 0x03,
		RotaryInput = 0x04,
		KeycodeInput = 0x05,
		ScreenPositionInput = 0x06,
		MiscSwitchInput = 0x07,

		CardSystem = 0x10,
		MedalHopper = 0x11,
		GeneralPurposeOutput = 0x12,
		AnalogOutput = 0x13,
		CharacterOutput = 0x14,
		Backup = 0x15,
	}; };

private:
	void packet_received();
	TIMER_CALLBACK_MEMBER(rx_timer_callback);
	TIMER_CALLBACK_MEMBER(tx_timer_callback);
	TIMER_CALLBACK_MEMBER(coin_timer_callback);
	void rx_clear();
	void rx_discard();
	void rx_stop();
	void update_output(uint64_t output);
	void update_analog_output(int offset, uint16_t output);

	emu_timer *m_tx_timer;
	emu_timer *m_rx_timer;
	emu_timer *m_coin_timer;

	uint8_t m_address;
	std::vector<uint8_t> m_mainid;
	uint8_t m_mainid_length;
	std::vector<uint8_t> m_retransmit_buffer;
	uint16_t m_retransmit_length;
	uint16_t m_rx_index;
	int32_t m_rx_bit;
	std::vector<uint8_t> m_rx_buffer;
	int32_t m_rx_length;
	uint8_t m_rx_state;
	int32_t m_rxd;
	int32_t m_tx_bit;
	std::vector<uint8_t> m_tx_buffer;
	int32_t m_tx_index;
	uint32_t m_tx_length;
	uint32_t m_tx_safe_length;
	uint8_t m_tx_state;

	uint8_t m_player_count;
	uint8_t m_switch_count;
	uint32_t m_switch_mask;
	uint8_t m_coin_slots;
	std::array<uint16_t, 8> m_coin_counter;
	uint8_t m_coin_offset;
	std::array<uint8_t, 8> m_coin_prev;
	uint8_t m_analog_input_channels;
	uint8_t m_analog_input_bits;
	uint8_t m_rotary_input_channels;
	uint8_t m_screen_position_input_channels;
	uint8_t m_screen_position_input_xbits;
	uint8_t m_screen_position_input_ybits;
	uint64_t m_output;
	uint64_t m_output_mask;
	uint8_t m_output_slots;
	std::array<uint16_t, 8> m_analog_output;
	uint8_t m_analog_output_channels;
};

#endif // MAME_BUS_JVS_JVSHLE_H
