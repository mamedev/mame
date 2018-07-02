// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    P&P Marketing Police Trainer hardware

**************************************************************************/

#include "cpu/mips/r3000.h"
#include "machine/eepromser.h"
#include "sound/bsmt2000.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class policetr_state : public driver_device
{
public:
	policetr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_rambase(*this, "rambase"),
		m_maincpu(*this, "maincpu"),
		m_bsmt(*this, "bsmt"),
		m_bsmt_region(*this, "bsmt"),
		m_lspeaker(*this, "lspeaker"),
		m_rspeaker(*this, "rspeaker"),
		m_eeprom(*this, "eeprom"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	void sshooter(machine_config &config);
	void policetr(machine_config &config);

	void init_sshoot12();
	void init_policetr();
	void init_sshooter();
	void init_plctr13b();

	DECLARE_CUSTOM_INPUT_MEMBER(bsmt_status_r);

private:
	enum
	{
		TIMER_IRQ5_GEN
	};

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void machine_start() override;
	virtual void video_start() override;

	void policetr_map(address_map &map);
	void sshooter_map(address_map &map);

	INTERRUPT_GEN_MEMBER(irq4_gen);

	DECLARE_WRITE32_MEMBER(control_w);
	DECLARE_WRITE32_MEMBER(speedup_w);

	DECLARE_WRITE32_MEMBER(policetr_bsmt2000_reg_w);
	DECLARE_WRITE32_MEMBER(policetr_bsmt2000_data_w);
	DECLARE_READ32_MEMBER(bsmt2000_data_r);

	DECLARE_WRITE32_MEMBER(policetr_video_w);
	DECLARE_READ32_MEMBER(policetr_video_r);
	DECLARE_WRITE32_MEMBER(policetr_palette_offset_w);
	DECLARE_WRITE32_MEMBER(policetr_palette_data_w);
	void render_display_list(offs_t offset);
	uint32_t screen_update_policetr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_shared_ptr<uint32_t> m_rambase;
	required_device<r3041_device> m_maincpu;
	required_device<bsmt2000_device> m_bsmt;
	required_memory_region m_bsmt_region;
	required_device<speaker_device> m_lspeaker;
	required_device<speaker_device> m_rspeaker;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint32_t m_control_data;
	uint32_t m_bsmt_data_bank;
	uint32_t m_bsmt_data_offset;
	uint32_t *m_speedup_data;
	uint64_t m_last_cycles;
	uint32_t m_loop_count;
	offs_t m_speedup_pc;
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
	emu_timer *m_irq5_gen_timer;
};
