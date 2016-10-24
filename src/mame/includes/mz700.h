// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Dirk Best
/******************************************************************************
 *  Sharp MZ700
 *
 *  Reference: http://sharpmz.computingmuseum.com
 *
 ******************************************************************************/

#ifndef MZ700_H_
#define MZ700_H_

#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/z80pio.h"
#include "sound/speaker.h"
#include "imagedev/cassette.h"
#include "bus/centronics/ctronics.h"
#include "machine/bankdev.h"
#include "machine/ram.h"

class mz_state : public driver_device
{
public:
	mz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_pit(*this, "pit8253")
		, m_ppi(*this, "ppi8255")
		, m_cassette(*this, "cassette")
		, m_centronics(*this, "centronics")
		, m_ram(*this, RAM_TAG)
		, m_palette(*this, "palette")
		, m_banke(*this, "banke")
		, m_bankf(*this, "bankf")
		{ }

	uint8_t mz700_e008_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mz700_e008_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mz800_bank_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mz700_bank_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mz800_bank_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mz800_bank_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mz700_bank_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mz700_bank_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mz700_bank_3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mz700_bank_4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mz700_bank_5_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mz700_bank_6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mz800_crtc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mz800_write_format_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mz800_read_format_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mz800_display_mode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mz800_scroll_border_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mz800_ramdisk_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mz800_ramdisk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mz800_ramaddr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mz800_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mz800_cgram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_mz800();
	void init_mz700();
	void machine_reset_mz700();
	void machine_reset_mz800();
	virtual void machine_start() override;
	uint32_t screen_update_mz700(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mz800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ne556_cursor_callback(timer_device &timer, void *ptr, int32_t param);
	void ne556_other_callback(timer_device &timer, void *ptr, int32_t param);
	void pit_out0_changed(int state);
	void pit_irq_2(int state);
	uint8_t pio_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pio_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pio_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pio_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mz800_z80pio_irq(int state);
	uint8_t mz800_z80pio_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mz800_z80pio_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void write_centronics_busy(int state);
	void write_centronics_perror(int state);

private:
	int m_mz700;                /* 1 if running on an mz700 */

	int m_cursor_timer;
	int m_other_timer;

	int m_intmsk;   /* PPI8255 pin PC2 */

	int m_mz700_ram_lock;       /* 1 if ram lock is active */
	int m_mz700_ram_vram;       /* 1 if vram is banked in */

	/* mz800 specific */
	std::unique_ptr<uint8_t[]> m_cgram;
	uint8_t *m_p_chargen;

	int m_mz700_mode;           /* 1 if in mz700 mode */
	int m_mz800_ram_lock;       /* 1 if lock is active */
	int m_mz800_ram_monitor;    /* 1 if monitor rom banked in */

	int m_hires_mode;           /* 1 if in 640x200 mode */
	int m_screennum;           /* screen designation */

	int m_centronics_busy;
	int m_centronics_perror;

	uint8_t *m_colorram;
	std::unique_ptr<uint8_t[]> m_videoram;
	uint8_t m_speaker_level;
	uint8_t m_prev_state;
	uint16_t m_mz800_ramaddr;
	uint8_t m_mz800_palette[4];
	uint8_t m_mz800_palette_bank;

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<pit8253_device> m_pit;
	required_device<i8255_device> m_ppi;
	required_device<cassette_image_device> m_cassette;
	optional_device<centronics_device> m_centronics;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;
	optional_device<address_map_bank_device> m_banke;
	optional_device<address_map_bank_device> m_bankf;
};

#endif /* MZ700_H_ */
