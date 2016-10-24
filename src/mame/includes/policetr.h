// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    P&P Marketing Police Trainer hardware

**************************************************************************/
#include "machine/eepromser.h"
class policetr_state : public driver_device
{
public:
	enum
	{
		TIMER_IRQ5_GEN
	};

	policetr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_rambase(*this, "rambase"),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	uint32_t m_control_data;
	uint32_t m_bsmt_data_bank;
	uint32_t m_bsmt_data_offset;
	uint32_t *m_speedup_data;
	uint64_t m_last_cycles;
	uint32_t m_loop_count;
	offs_t m_speedup_pc;
	required_shared_ptr<uint32_t> m_rambase;
	uint32_t m_palette_offset;
	uint8_t m_palette_index;
	uint8_t m_palette_data[3];
	rectangle m_render_clip;
	uint8_t *m_srcbitmap;
	std::unique_ptr<uint8_t[]> m_dstbitmap;
	uint16_t m_src_xoffs;
	uint16_t m_src_yoffs;
	uint16_t m_dst_xoffs;
	uint16_t m_dst_yoffs;
	uint8_t m_video_latch;
	uint32_t m_srcbitmap_height_mask;
	void control_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void policetr_bsmt2000_reg_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void policetr_bsmt2000_data_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t bsmt2000_data_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void speedup_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void policetr_video_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t policetr_video_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void policetr_palette_offset_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void policetr_palette_data_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	ioport_value bsmt_status_r(ioport_field &field, void *param);
	void init_sshoot12();
	void init_policetr();
	void init_sshooter();
	void init_plctr13b();
	virtual void video_start() override;
	uint32_t screen_update_policetr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void irq4_gen(device_t &device);
	void render_display_list(offs_t offset);
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
