// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Nicola Salmoria
#define NVRAM_UNLOCK_SEQ_LEN 10

#include "cpu/tms34010/tms34010.h"
#include "machine/timer.h"
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
	DECLARE_WRITE16_MEMBER(nvram_thrash_w);
	DECLARE_WRITE16_MEMBER(nvram_data_w);
	DECLARE_WRITE16_MEMBER(nvram_thrash_data_w);
	DECLARE_WRITE16_MEMBER(amerdart_misc_w);
	DECLARE_READ_LINE_MEMBER(amerdart_dsp_bio_line_r);
	DECLARE_READ16_MEMBER(amerdart_iop_r);
	DECLARE_WRITE16_MEMBER(amerdart_iop_w);
	DECLARE_READ16_MEMBER(amerdart_dsp_cmd_r);
	DECLARE_WRITE16_MEMBER(amerdart_dsp_answer_w);
	DECLARE_READ16_MEMBER(amerdart_trackball_r);
	DECLARE_WRITE16_MEMBER(coolpool_misc_w);
	DECLARE_WRITE16_MEMBER(coolpool_iop_w);
	DECLARE_READ16_MEMBER(coolpool_iop_r);
	DECLARE_READ16_MEMBER(dsp_cmd_r);
	DECLARE_WRITE16_MEMBER(dsp_answer_w);
	DECLARE_READ16_MEMBER(dsp_bio_line_r);
	DECLARE_READ16_MEMBER(dsp_hold_line_r);
	DECLARE_READ16_MEMBER(dsp_rom_r);
	DECLARE_WRITE16_MEMBER(dsp_romaddr_w);
	DECLARE_READ16_MEMBER(coolpool_input_r);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(amerdart_scanline);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(coolpool_scanline);
	DECLARE_DRIVER_INIT(coolpool);
	DECLARE_DRIVER_INIT(amerdart);
	DECLARE_DRIVER_INIT(9ballsht);
	DECLARE_MACHINE_RESET(amerdart);
	DECLARE_MACHINE_RESET(coolpool);
	TIMER_CALLBACK_MEMBER(deferred_iop_w);
	TIMER_DEVICE_CALLBACK_MEMBER(nvram_write_timeout);
	TIMER_DEVICE_CALLBACK_MEMBER(amerdart_audio_int_gen);
	void register_state_save();
	int amerdart_trackball_direction(int num, int data);
	void _9ballsht(machine_config &config);
	void coolpool(machine_config &config);
	void amerdart(machine_config &config);
	void amerdart_dsp_io_map(address_map &map);
	void amerdart_dsp_pgm_map(address_map &map);
	void amerdart_map(address_map &map);
	void coolpool_dsp_io_map(address_map &map);
	void coolpool_dsp_pgm_map(address_map &map);
	void coolpool_map(address_map &map);
	void nballsht_map(address_map &map);
};
