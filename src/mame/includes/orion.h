// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/orion.h
 *
 ****************************************************************************/

#ifndef ORION_H_
#define ORION_H_

#include "machine/wd_fdc.h"
#include "includes/radio86.h"
#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "machine/mc146818.h"
#include "sound/speaker.h"
#include "sound/ay8910.h"
#include "sound/wave.h"


class orion_state : public radio86_state
{
public:
	orion_state(const machine_config &mconfig, device_type type, const char *tag)
		: radio86_state(mconfig, type, tag)
		, m_fdc(*this, "fd1793")
		, m_ram(*this, RAM_TAG)
		, m_fd0(*this, "fd0")
		, m_fd1(*this, "fd1")
		, m_fd2(*this, "fd2")
		, m_fd3(*this, "fd3")
		, m_rtc(*this, "rtc")
		, m_speaker(*this, "speaker")
		, m_ay8912(*this, "ay8912")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_bank4(*this, "bank4")
		, m_bank5(*this, "bank5")
		, m_bank6(*this, "bank6")
		, m_bank7(*this, "bank7")
		, m_bank8(*this, "bank8")
	{ }

	uint8_t m_orion128_video_mode;
	uint8_t m_orion128_video_page;
	uint8_t m_orion128_video_width;
	uint8_t m_video_mode_mask;
	uint8_t m_orionpro_pseudo_color;
	uint8_t m_romdisk_lsb;
	uint8_t m_romdisk_msb;
	uint8_t m_orion128_memory_page;
	uint8_t m_orionz80_memory_page;
	uint8_t m_orionz80_dispatcher;
	uint8_t m_speaker_data;
	uint8_t m_orionpro_ram0_segment;
	uint8_t m_orionpro_ram1_segment;
	uint8_t m_orionpro_ram2_segment;
	uint8_t m_orionpro_page;
	uint8_t m_orionpro_128_page;
	uint8_t m_orionpro_rom2_segment;
	uint8_t m_orionpro_dispatcher;

	required_device<fd1793_t> m_fdc;

	uint8_t orion128_system_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void orion128_system_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t orion128_romdisk_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void orion128_romdisk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void orion128_video_mode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void orion128_video_page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void orion128_memory_page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void orion_disk_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t orion128_floppy_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void orion128_floppy_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t orionz80_floppy_rtc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void orionz80_floppy_rtc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void orionz80_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void orionz80_sound_fe_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void orionz80_memory_page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void orionz80_dispatcher_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t orionz80_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void orionz80_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void orionpro_memory_page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t orionpro_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void orionpro_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void machine_start_orion128();
	void machine_reset_orion128();
	void video_start_orion128();
	void palette_init_orion128(palette_device &palette);
	void machine_start_orionz80();
	void machine_reset_orionz80();
	void machine_reset_orionpro();
	uint32_t screen_update_orion128(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void orionz80_interrupt(device_t &device);
	uint8_t orion_romdisk_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void orion_romdisk_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void orion_romdisk_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	DECLARE_FLOPPY_FORMATS( orion_floppy_formats );

protected:
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_fd0;
	required_device<floppy_connector> m_fd1;
	required_device<floppy_connector> m_fd2;
	required_device<floppy_connector> m_fd3;
	optional_device<mc146818_device> m_rtc;
	optional_device<speaker_sound_device> m_speaker;
	optional_device<ay8910_device> m_ay8912;
	required_memory_bank m_bank2;
	optional_memory_bank m_bank3;
	optional_memory_bank m_bank4;
	optional_memory_bank m_bank5;
	optional_memory_bank m_bank6;
	optional_memory_bank m_bank7;
	optional_memory_bank m_bank8;

	void orionz80_switch_bank();
	void orion_set_video_mode(int width);
	void orionpro_bank_switch();
};

#endif /* ORION_H_ */
