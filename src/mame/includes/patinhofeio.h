// license:GPL2+
// copyright-holders:Felipe Sanches
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
	uint16_t rc_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t buttons_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void decwriter_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decwriter_kbd_input(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void decwriter_callback(void *ptr, int32_t param);

	void teletype_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void teletype_kbd_input(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void teletype_callback(void *ptr, int32_t param);

	image_init_result device_image_load_patinho_tape(device_image_interface &image);
	void load_tape(const char* name);
	void load_raw_data(const char* name, unsigned int start_address, unsigned int data_length);
	void update_panel(uint8_t ACC, uint8_t opcode, uint8_t mem_data, uint16_t mem_addr, uint16_t PC, uint8_t FLAGS, uint16_t RC, uint8_t mode);
	virtual void machine_start() override;

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
