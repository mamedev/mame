// license:GPL2+
// copyright-holders:Felipe Sanches
#ifndef MAME_USP_PATINHOFEIO_H
#define MAME_USP_PATINHOFEIO_H

#pragma once

#include "teleprinter.h"

class patinho_feio_state : public driver_device {
public:
	patinho_feio_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_decwriter(*this, "decwriter")
		, m_tty(*this, "teletype")
	{ }

	void init_patinho_feio();

	void decwriter_data_w(uint8_t data);
	void decwriter_kbd_input(u8 data);
	TIMER_CALLBACK_MEMBER(decwriter_callback);

	void teletype_data_w(uint8_t data);
	void teletype_kbd_input(u8 data);
	TIMER_CALLBACK_MEMBER(teletype_callback);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( tape_load );

	void update_panel(uint8_t ACC, uint8_t opcode, uint8_t mem_data, uint16_t mem_addr, uint16_t PC, uint8_t FLAGS, uint16_t RC, uint8_t mode);

	void patinho_feio(machine_config &config);
protected:
	virtual void machine_start() override ATTR_COLD;

	void load_tape(const char* name);
	void load_raw_data(const char* name, unsigned int start_address, unsigned int data_length);

	required_device<patinho_feio_cpu_device> m_maincpu;
	required_device<teleprinter_device> m_decwriter;
	required_device<teleprinter_device> m_tty;

private:
	uint8_t* paper_tape_data = nullptr;
	uint32_t paper_tape_length = 0;
	uint32_t paper_tape_address = 0;

	emu_timer *m_decwriter_timer = nullptr;
	emu_timer *m_teletype_timer = nullptr;
	output_manager *m_out = nullptr;
	uint8_t m_prev_ACC = 0;
	uint8_t m_prev_opcode = 0;
	uint8_t m_prev_mem_data = 0;
	uint16_t m_prev_mem_addr = 0;
	uint16_t m_prev_PC = 0;
	uint8_t m_prev_FLAGS = 0;
	uint16_t m_prev_RC = 0;
};

#endif // MAME_USP_PATINHOFEIO_H
