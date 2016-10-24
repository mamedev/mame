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
#include "machine/bankdev.h"
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
			m_palette(*this, "palette"),
			m_bankdev1(*this, "bank0"),
			m_bankdev2(*this, "bank1"),
			m_flash1(*this, "flash1"),
			m_nvram(*this, "nvram")
		{ }

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

	// defined in drivers/avigo.c
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void refresh_ints();
	void nvram_init(nvram_device &nvram, void *base, size_t size);

	void tc8521_alarm_int(int state);
	void com_interrupt(int state);

	uint8_t key_data_read_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void set_key_line_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t irq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void irq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bank1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t bank2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bank1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bank2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ad_control_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ad_control_status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ad_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void speaker_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port_04_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void pen_irq(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void pen_move_irq(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void kb_irq(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void power_down_irq(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	// defined in video/avigo.c
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint8_t vid_memory_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void vid_memory_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// driver state
	uint8_t               m_key_line;
	uint8_t               m_irq;
	uint8_t               m_port2;
	uint8_t               m_bank2_l;
	uint8_t               m_bank2_h;
	uint8_t               m_bank1_l;
	uint8_t               m_bank1_h;
	uint8_t               m_ad_control_status;
	uint16_t              m_ad_value;
	uint8_t *             m_video_memory;
	uint8_t               m_screen_column;
	uint8_t               m_warm_start;
	void palette_init_avigo(palette_device &palette);
	void avigo_scan_timer(timer_device &timer, void *ptr, int32_t param);
	void avigo_1hz_timer(timer_device &timer, void *ptr, int32_t param);

	DECLARE_QUICKLOAD_LOAD_MEMBER( avigo);
};
#endif /* AVIGO_H_ */
