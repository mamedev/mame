// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Nicola Salmoria
#define NVRAM_UNLOCK_SEQ_LEN 10

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "video/tlc34076.h"

class coolpool_state : public driver_device
{
public:
	coolpool_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dsp(*this, "dsp"),
		m_tlc34076(*this, "tlc34076"),
		m_vram_base(*this, "vram_base"),
		m_nvram(*this, "nvram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_dsp;
	optional_device<tlc34076_device> m_tlc34076;

	required_shared_ptr<uint16_t> m_vram_base;
	required_shared_ptr<uint16_t> m_nvram;

	uint8_t m_cmd_pending;
	uint16_t m_iop_cmd;
	uint16_t m_iop_answer;
	int m_iop_romaddr;

	uint8_t m_newx[3];
	uint8_t m_newy[3];
	uint8_t m_oldx[3];
	uint8_t m_oldy[3];
	int m_dx[3];
	int m_dy[3];

	uint16_t m_result;
	uint16_t m_lastresult;

	uint16_t m_nvram_write_seq[NVRAM_UNLOCK_SEQ_LEN];
	uint8_t m_nvram_write_enable;
	uint8_t m_old_cmd;
	uint8_t m_same_cmd_count;
	void nvram_thrash_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nvram_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nvram_thrash_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void amerdart_misc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	int amerdart_dsp_bio_line_r();
	uint16_t amerdart_iop_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void amerdart_iop_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t amerdart_dsp_cmd_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void amerdart_dsp_answer_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t amerdart_trackball_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void coolpool_misc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void coolpool_iop_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t coolpool_iop_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t dsp_cmd_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_answer_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_bio_line_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t dsp_hold_line_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t dsp_rom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_romaddr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t coolpool_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(amerdart_scanline);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(coolpool_scanline);
	void init_coolpool();
	void init_amerdart();
	void init_9ballsht();
	void machine_reset_amerdart();
	void machine_reset_coolpool();
	void deferred_iop_w(void *ptr, int32_t param);
	void nvram_write_timeout(timer_device &timer, void *ptr, int32_t param);
	void amerdart_audio_int_gen(timer_device &timer, void *ptr, int32_t param);
	void register_state_save();
	int amerdart_trackball_direction(int num, int data);
};
