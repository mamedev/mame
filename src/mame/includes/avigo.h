// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/*****************************************************************************
 *
 * includes/avigo.h
 *
 ****************************************************************************/

#ifndef AVIGO_H_
#define AVIGO_H_

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/rp5c01.h"
#include "machine/ins8250.h"
#include "machine/intelfsh.h"
#include "machine/nvram.h"
#include "sound/speaker.h"
#include "machine/ram.h"
#include "imagedev/snapquik.h"

#define AVIGO_NUM_COLOURS 2

#define AVIGO_SCREEN_WIDTH        160
#define AVIGO_SCREEN_HEIGHT       240
#define AVIGO_PANEL_HEIGHT        26


class avigo_state : public driver_device
{
public:
	avigo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ram(*this, RAM_TAG),
			m_speaker(*this, "speaker"),
			m_uart(*this, "ns16550"),
			m_serport(*this, "serport"),
			m_palette(*this, "palette")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<speaker_sound_device> m_speaker;
	required_device<ns16550_device> m_uart;
	required_device<rs232_port_device> m_serport;
	required_device<palette_device> m_palette;

	// defined in drivers/avigo.c
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void postload();
	void refresh_memory(UINT8 bank, UINT8 chip_select);
	void refresh_ints();
	void nvram_init(nvram_device &nvram, void *base, size_t size);

	DECLARE_WRITE_LINE_MEMBER( tc8521_alarm_int );
	DECLARE_WRITE_LINE_MEMBER( com_interrupt );

	DECLARE_READ8_MEMBER(flash_0x0000_read_handler);
	DECLARE_WRITE8_MEMBER(flash_0x0000_write_handler);
	DECLARE_READ8_MEMBER(flash_0x4000_read_handler);
	DECLARE_WRITE8_MEMBER(flash_0x4000_write_handler);
	DECLARE_READ8_MEMBER(flash_0x8000_read_handler);
	DECLARE_WRITE8_MEMBER(flash_0x8000_write_handler);

	DECLARE_READ8_MEMBER(key_data_read_r);
	DECLARE_WRITE8_MEMBER(set_key_line_w);
	DECLARE_WRITE8_MEMBER(port2_w);
	DECLARE_READ8_MEMBER(irq_r);
	DECLARE_WRITE8_MEMBER(irq_w);
	DECLARE_READ8_MEMBER(bank1_r);
	DECLARE_READ8_MEMBER(bank2_r);
	DECLARE_WRITE8_MEMBER(bank1_w);
	DECLARE_WRITE8_MEMBER(bank2_w);
	DECLARE_READ8_MEMBER(ad_control_status_r);
	DECLARE_WRITE8_MEMBER(ad_control_status_w);
	DECLARE_READ8_MEMBER(ad_data_r);
	DECLARE_WRITE8_MEMBER(speaker_w);
	DECLARE_READ8_MEMBER(port_04_r);

	DECLARE_INPUT_CHANGED_MEMBER(pen_irq);
	DECLARE_INPUT_CHANGED_MEMBER(pen_move_irq);
	DECLARE_INPUT_CHANGED_MEMBER(kb_irq);
	DECLARE_INPUT_CHANGED_MEMBER(power_down_irq);

	// defined in video/avigo.c
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_READ8_MEMBER(vid_memory_r);
	DECLARE_WRITE8_MEMBER(vid_memory_w);

	// driver state
	UINT8               m_key_line;
	UINT8               m_irq;
	UINT8               m_port2;
	UINT8               m_bank2_l;
	UINT8               m_bank2_h;
	UINT8               m_bank1_l;
	UINT8               m_bank1_h;
	UINT8               m_ad_control_status;
	intelfsh8_device *  m_flashes[3];
	int                 m_flash_at_0x4000;
	int                 m_flash_at_0x8000;
	UINT16              m_ad_value;
	UINT8 *             m_video_memory;
	UINT8               m_screen_column;
	UINT8               m_warm_start;
	UINT8 *             m_ram_base;
	DECLARE_PALETTE_INIT(avigo);
	TIMER_DEVICE_CALLBACK_MEMBER(avigo_scan_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(avigo_1hz_timer);

	DECLARE_QUICKLOAD_LOAD_MEMBER( avigo);
};
#endif /* AVIGO_H_ */
