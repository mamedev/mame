// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_SOUND_DAC3350A_H
#define MAME_SOUND_DAC3350A_H

#pragma once

class dac3350a_device : public device_t, public device_sound_interface
{
public:
	dac3350a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void i2c_scl_w(int line);
	void i2c_sda_w(int line);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	void i2c_device_handle_write();
	float calculate_volume(int val);

	enum
	{
		CMD_DEV_WRITE = 0x9a,
	};

	enum
	{
		IDLE = 0,
		STARTED,
		NAK,
		ACK,
		ACK2,
	};

	enum
	{
		UNKNOWN = 0,
		VALIDATED,
		WRONG,
	};

	enum
	{
		REG_UNKNOWN = 0,
		REG_SR = 1,
		REG_AVOL,
		REG_GCFG,
	};

	enum
	{
		// sample rate control register
		SR_LR_SEL_BIT = 4, // L/R-bit
		SR_LR_SEL_LEFT = 0, // (WSI = 0 -> left channel
		SR_LR_SEL_RIGHT = 1, // (WSI = 0 -> right channel

		SR_SP_SEL_BIT = 3, // delay bit
		SR_SP_SEL_NO_DELAY = 0,
		SR_SP_SEL_1_BIT_DELAY = 1,

		SR_SRC_BIT = 0, // 3 bits wide, sample rate control
		SR_SRC_48 = 0, // 32-48 kHz
		SR_SRC_32, // 26-32 kHz
		SR_SRC_24, // 20-26 kHz
		SR_SRC_16, // 14-20 kHz
		SR_SRC_12, // 10-14 kHz
		SR_SRC_8, // 8-10 kHz
		SR_SRC_A, // autoselect
	};

	enum
	{
		// analog volume register
		AVOL_DEEM_BIT = 14, // deemphasis on/off
		AVOL_DEEM_OFF = 0,
		AVOL_DEEM_ON = 1,

		AVOL_L_BIT = 8, // 6 bits wide, analog audio volume level left
		AVOL_R_BIT = 0, // 6 bits wide, analog audio volume level right
	};

	enum
	{
		// global configuration register
		GCFG_SEL_53V_BIT = 6, // select 3V-5V mode
		GCFG_SEL_53V_3V = 0,
		GCFG_SEL_53V_5V = 1,

		GCFG_PWMD_BIT = 5, // power-mode
		GCFG_PWMD_NORMAL = 0,
		GCFG_PWMD_LOW_POWER = 1,

		GCFG_INSEL_AUX2_BIT = 4, // AUX2 select
		GCFG_INSEL_AUX2_OFF = 0,
		GCFG_INSEL_AUX2_ON = 1,

		GCFG_INSEL_AUX1_BIT = 3, // AUX1 select
		GCFG_INSEL_AUX1_OFF = 0,
		GCFG_INSEL_AUX1_ON = 1,

		GCFG_INSEL_DAC_BIT = 2, // DAC select
		GCFG_INSEL_DAC_OFF = 0,
		GCFG_INSEL_DAC_ON = 1,

		GCFG_AUX_MS_BIT = 1, // aux-mono/stereo
		GCFG_AUX_MS_STEREO = 0,
		GCFG_AUX_MS_MONO = 1,

		GCFG_IPRA_BIT = 0, // invert right power amplifier
		GCFG_IPRA_NOT_INVERTED = 0,
		GCFG_IPRA_INVERTED = 1,
	};

	sound_stream *m_stream;

	uint8_t m_i2c_bus_state;
	uint8_t m_i2c_bus_address;

	uint8_t m_i2c_scli, m_i2c_sdai;
	int32_t m_i2c_bus_curbit;
	uint8_t m_i2c_bus_curval;
	uint32_t m_i2c_bytecount;

	uint8_t m_i2c_subadr;
	uint16_t m_i2c_data;

	bool m_dac_enable;

	float m_volume[2];
};

DECLARE_DEVICE_TYPE(DAC3350A, dac3350a_device)

#endif // MAME_SOUND_DAC3350A_H
