// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/*****************************************************************************
 *
 * includes/avigo.h
 *
 ****************************************************************************/

#ifndef MAME_TI_AVIGO_H
#define MAME_TI_AVIGO_H

#pragma once

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/snapquik.h"
#include "machine/bankdev.h"
#include "machine/ins8250.h"
#include "machine/intelfsh.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/rp5c01.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"
#include "emupal.h"

#define AVIGO_SCREEN_WIDTH        160
#define AVIGO_SCREEN_HEIGHT       240
#define AVIGO_PANEL_HEIGHT        26


class avigo_state : public driver_device
{
public:
	avigo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_speaker(*this, "speaker")
		, m_uart(*this, "ns16550")
		, m_serport(*this, "serport")
		, m_palette(*this, "palette")
		, m_bankdev1(*this, "bank0")
		, m_bankdev2(*this, "bank1")
		, m_flash1(*this, "flash1")
		, m_nvram(*this, "nvram")
	{ }

	void avigo(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(pen_irq);
	DECLARE_INPUT_CHANGED_MEMBER(pen_move_irq);
	DECLARE_INPUT_CHANGED_MEMBER(kb_irq);
	DECLARE_INPUT_CHANGED_MEMBER(power_down_irq);

protected:
	// defined in drivers/avigo.cpp
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void refresh_ints();
	void nvram_init(nvram_device &nvram, void *base, size_t size);

	void tc8521_alarm_int(int state);
	void com_interrupt(int state);

	uint8_t key_data_read_r();
	void set_key_line_w(uint8_t data);
	void port2_w(uint8_t data);
	uint8_t irq_r();
	void irq_w(uint8_t data);
	uint8_t bank1_r(offs_t offset);
	uint8_t bank2_r(offs_t offset);
	void bank1_w(offs_t offset, uint8_t data);
	void bank2_w(offs_t offset, uint8_t data);
	uint8_t ad_control_status_r();
	void ad_control_status_w(uint8_t data);
	uint8_t ad_data_r();
	void speaker_w(uint8_t data);
	uint8_t port_04_r();

	// defined in video/avigo.cpp
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint8_t vid_memory_r(offs_t offset);
	void vid_memory_w(offs_t offset, uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(avigo_scan_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(avigo_1hz_timer);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	void avigo_banked_map(address_map &map) ATTR_COLD;
	void avigo_io(address_map &map) ATTR_COLD;
	void avigo_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<speaker_sound_device> m_speaker;
	required_device<ns16550_device> m_uart;
	required_device<rs232_port_device> m_serport;
	required_device<palette_device> m_palette;
	required_device<address_map_bank_device> m_bankdev1;
	required_device<address_map_bank_device> m_bankdev2;
	required_device<intelfsh8_device> m_flash1;
	required_shared_ptr<uint8_t> m_nvram;

	// driver state
	uint8_t               m_key_line = 0U;
	uint8_t               m_irq = 0U;
	uint8_t               m_port2 = 0U;
	uint8_t               m_bank2_l = 0U;
	uint8_t               m_bank2_h = 0U;
	uint8_t               m_bank1_l = 0U;
	uint8_t               m_bank1_h = 0U;
	uint8_t               m_ad_control_status = 0U;
	uint16_t              m_ad_value = 0U;
	std::unique_ptr<uint8_t[]> m_video_memory;
	uint8_t               m_screen_column = 0U;
	uint8_t               m_warm_start = 0U;
};

#endif // MAME_TI_AVIGO_H
