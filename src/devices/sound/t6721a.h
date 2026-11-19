// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Toshiba T6721A C2MOS Voice Synthesizing LSI emulation

**********************************************************************
                            _____   _____
                   SP3   1 |*    \_/     | 42  Vdd
                  LOSS   2 |             | 41  SP2
                    TS   3 |             | 40  SP1
                   TSN   4 |             | 39  SP0
                     W   5 |             | 38  TEM
                  TDAI   6 |             | 37  FR
                  TFIO   7 |             | 36  BR
                   DAO   8 |             | 35  OD
                   APD   9 |             | 34  REP
                  phi2  10 |             | 33  EXP
                    PD  11 |    T6721A   | 32  CK2
           ROM ADR RST  12 |             | 31  CK1
               ROM RST  13 |             | 30  M-START
                   ALD  14 |             | 29  TPN
                    DI  15 |             | 28  _ACL
                  DTRD  16 |             | 27  CPUM
                    D3  17 |             | 26  _EOS
                    D2  18 |             | 25  _BSY
                    D1  19 |             | 24  _CE
                    D0  20 |             | 23  _RD
                   GND  21 |_____________| 22  _WR

**********************************************************************/

#ifndef MAME_SOUND_T6721A_H
#define MAME_SOUND_T6721A_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> t6721a_device

class t6721a_device : public device_t,
						public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	t6721a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto rom_handler() { return m_read_rom.bind(); }
	auto bsy_handler() { return m_write_bsy.bind(); }
	auto eos_handler() { return m_write_eos.bind(); }
	auto phi2_handler() { return m_write_phi2.bind(); }
	auto dtrd_handler() { return m_write_dtrd.bind(); }
	auto apd_handler() { return m_write_apd.bind(); }

	uint8_t read();
	void write(uint8_t data);

	void di_w(int state);

	int eos_r();
	int bsy_r();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	enum
	{
		CMD_NOP = 0,
		CMD_STRT,
		CMD_STOP,
		CMD_ADLD,
		CMD_AAGN,
		CMD_SPLD,
		CMD_CNDT1,
		CMD_CNDT2,
		CMD_RRDM,
		CMD_SPDN,
		CMD_APDN,
		CMD_SAGN
	};

	TIMER_CALLBACK_MEMBER(serial_tick);
	TIMER_CALLBACK_MEMBER(frame_tick);
	TIMER_CALLBACK_MEMBER(busy_tick);
	TIMER_CALLBACK_MEMBER(eos_tick);

	void set_busy(unsigned clocks);
	void set_eos(int state);
	void set_apd(int state);
	void stop();
	void begin_serial(bool rom_read);
	void accept_bit(int bit);
	void silent_frame();
	void end_of_speech();
	void decode_frame();
	unsigned frame_samples() const;

	devcb_read8 m_read_rom;
	devcb_write_line m_write_bsy;
	devcb_write_line m_write_eos;
	devcb_write_line m_write_phi2;
	devcb_write_line m_write_dtrd;
	devcb_write_line m_write_apd;

	sound_stream *m_stream;
	emu_timer *m_serial_timer;
	emu_timer *m_frame_timer;
	emu_timer *m_busy_timer;
	emu_timer *m_eos_timer;

	uint8_t m_status;
	uint8_t m_command;
	uint8_t m_arguments;
	uint8_t m_speed;
	uint8_t m_condition1;
	uint8_t m_condition2;
	uint32_t m_address;
	uint32_t m_rom_bit;
	uint8_t m_rom_data;
	bool m_rom_read;
	bool m_serial_rom;
	bool m_speaking;
	bool m_first_frame;
	bool m_all_ones;
	bool m_busy;
	bool m_eos;
	bool m_apd;
	bool m_di;
	uint8_t m_serial_bits;
	uint8_t m_nibble_bit;
	uint8_t m_parameter;
	uint8_t m_parameter_bits;
	uint16_t m_parameters[12];
	uint16_t m_sample;
	uint16_t m_samples;
	uint16_t m_pitch_count;
	uint16_t m_noise;
	int32_t m_glottal[2];
	int32_t m_previous[12];
	int32_t m_target[12];
	int32_t m_delay[10];
};


// device type definition
DECLARE_DEVICE_TYPE(T6721A, t6721a_device)

#endif // MAME_SOUND_T6721A_H
