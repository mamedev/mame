// license:GPL2+
// copyright-holders:Felipe Sanches
#ifndef MAME_INCLUDES_PATINHOFEIO_H
#define MAME_INCLUDES_PATINHOFEIO_H

#pragma once

#include "machine/teleprinter.h"

class patinho_feio_state : public driver_device {
public:
	patinho_feio_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_decwriter(*this, "decwriter")
		, m_tty(*this, "teletype")
	{ }

	void init_patinho_feio();

	DECLARE_WRITE8_MEMBER(decwriter_data_w);
	void decwriter_kbd_input(u8 data);
	TIMER_CALLBACK_MEMBER(decwriter_callback);

	DECLARE_WRITE8_MEMBER(teletype_data_w);
	void teletype_kbd_input(u8 data);
	TIMER_CALLBACK_MEMBER(teletype_callback);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( tape_load );

	void update_panel(uint8_t ACC, uint8_t opcode, uint8_t mem_data, uint16_t mem_addr, uint16_t PC, uint8_t FLAGS, uint16_t RC, uint8_t mode);

	void patinho_feio(machine_config &config);
protected:
	virtual void machine_start() override;

	void load_tape(const char* name);
	void load_raw_data(const char* name, unsigned int start_address, unsigned int data_length);

	required_device<patinho_feio_cpu_device> m_maincpu;
	required_device<teleprinter_device> m_decwriter;
	required_device<teleprinter_device> m_tty;

private:
	uint8_t* paper_tape_data;
	uint32_t paper_tape_length;
	uint32_t paper_tape_address;

	emu_timer *m_decwriter_timer;
	emu_timer *m_teletype_timer;
	output_manager *m_out;
	uint8_t m_prev_ACC;
	uint8_t m_prev_opcode;
	uint8_t m_prev_mem_data;
	uint16_t m_prev_mem_addr;
	uint16_t m_prev_PC;
	uint8_t m_prev_FLAGS;
	uint16_t m_prev_RC;
};

#endif // MAME_INCLUDES_PATINHOFEIO_H
