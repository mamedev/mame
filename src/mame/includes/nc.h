// license:GPL-2.0+
// copyright-holders:Wilbert Pol, Kevin Thacker
/*****************************************************************************
 *
 * includes/nc.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_NC_H
#define MAME_INCLUDES_NC_H

#pragma once

#include "bus/centronics/ctronics.h"
#include "machine/upd765.h"     // for NC200 disk drive interface
#include "machine/i8251.h"
#include "machine/clock.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "emupal.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#define NC_NUM_COLOURS 4

#define NC_SCREEN_WIDTH        480
#define NC_SCREEN_HEIGHT       64

#define NC200_SCREEN_WIDTH      480
#define NC200_SCREEN_HEIGHT     128

#define NC200_NUM_COLOURS 4

class nc_state : public driver_device
{
public:
	enum nc_type
	{
		NC_TYPE_1xx, // nc100/nc150
		NC_TYPE_200  // nc200
	};

	nc_state(const machine_config &mconfig, device_type type, const char *tag, nc_type variant) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_beeper1(*this, "beep.1"),
		m_beeper2(*this, "beep.2"),
		m_centronics(*this, "centronics"),
		m_card(*this, "cardslot"),
		m_uart(*this, "uart"),
		m_uart_clock(*this, "uart_clock"),
		m_nvram(*this, "nvram"),
		m_fdc(*this, "upd765"),
		m_bankhandler_r(*this, "bank%u", 1U),
		m_bankhandler_w(*this, "bank%u", 5U),
		m_nc_type(variant)
	{
	}

	void nc_base(machine_config &config);

	void init_nc();

protected:
	uint8_t nc_memory_management_r(offs_t offset);
	void nc_memory_management_w(offs_t offset, uint8_t data);
	void nc_irq_mask_w(uint8_t data);
	void nc_irq_status_w(uint8_t data);
	uint8_t nc_irq_status_r();
	uint8_t nc_key_data_in_r(offs_t offset);
	void nc_sound_w(offs_t offset, uint8_t data);
	void nc_uart_control_w(uint8_t data);
	void nc100_display_memory_start_w(uint8_t data);

	void nc_colours(palette_device &palette) const;
	TIMER_CALLBACK_MEMBER(nc_keyboard_timer_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(dummy_timer_callback);
	DECLARE_WRITE_LINE_MEMBER(write_uart_clock);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( load_pcmcia_card );
	DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER( unload_pcmcia_card );

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void nc_map(address_map &map);

	uint32_t screen_update_nc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int height, int width, int const (&pens)[2]);
	void nc_update_interrupts();

private:
	int card_calculate_mask(int size);
	void set_card_present_state(int state);
	void nc_refresh_memory_bank_config(int bank);
	void nc_refresh_memory_config();
	void nc_sound_update(int channel);

protected: // HACK FOR MC6845
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<beep_device> m_beeper1;
	required_device<beep_device> m_beeper2;
	required_device<centronics_device> m_centronics;
	required_device<generic_slot_device> m_card;
	required_device<i8251_device> m_uart;
	required_device<clock_device> m_uart_clock;
	required_device<nvram_device> m_nvram;
	optional_device<upd765a_device> m_fdc;
	required_memory_bank_array<4> m_bankhandler_r, m_bankhandler_w;

	char m_memory_config[4];
	emu_timer *m_keyboard_timer = nullptr;
	int m_membank_rom_mask = 0;
	int m_membank_internal_ram_mask = 0;
	uint8_t m_poweroff_control = 0;
	int m_card_status = 0;
	unsigned char m_uart_control = 0;
	int m_irq_mask = 0;
	int m_irq_status = 0;
	int m_irq_latch = 0;
	int m_irq_latch_mask = 0;
	int m_sound_channel_periods[2];
	int m_previous_inputport_10_state = 0;
	int m_previous_alarm_state = 0;
	memory_region *m_card_ram = nullptr;
	int m_membank_card_ram_mask = 0;
	int m_card_size = 0;
	unsigned long m_display_memory_start = 0;
	const nc_type m_nc_type;

	int m_centronics_ack = 0;
	int m_centronics_busy = 0;
};


class nc100_state : public nc_state
{
public:
	nc100_state(const machine_config &mconfig, device_type type, const char *tag) :
		nc_state(mconfig, type, tag, NC_TYPE_1xx)
	{
	}

	void nc100(machine_config &config);

protected:
	void nc100_uart_control_w(uint8_t data);
	void nc100_poweroff_control_w(uint8_t data);
	uint8_t nc100_card_battery_status_r();
	void nc100_memory_card_wait_state_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(nc100_tc8521_alarm_callback);
	DECLARE_WRITE_LINE_MEMBER(nc100_txrdy_callback);
	DECLARE_WRITE_LINE_MEMBER(nc100_rxrdy_callback);
	DECLARE_WRITE_LINE_MEMBER(write_nc100_centronics_ack);

	virtual void machine_reset() override;

	void nc100_io(address_map &map);

	uint32_t screen_update_nc100(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};



class nc200_state : public nc_state
{
public:
	nc200_state(const machine_config &mconfig, device_type type, const char *tag) :
		nc_state(mconfig, type, tag, NC_TYPE_200)
	{
	}

	void nc200(machine_config &config);

protected:
	void nc200_irq_status_w(uint8_t data);
	uint8_t nc200_card_battery_status_r();
	uint8_t nc200_printer_status_r();
	void nc200_uart_control_w(uint8_t data);
	void nc200_memory_card_wait_state_w(uint8_t data);
	void nc200_poweroff_control_w(uint8_t data);
	[[maybe_unused]] void nc200_display_memory_start_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(write_nc200_centronics_ack);
	DECLARE_WRITE_LINE_MEMBER(nc200_txrdy_callback);
	DECLARE_WRITE_LINE_MEMBER(nc200_rxrdy_callback);
	DECLARE_WRITE_LINE_MEMBER(nc200_fdc_interrupt);

	[[maybe_unused]] void nc200_floppy_drive_index_callback(int drive_id);

	virtual void machine_reset() override;

	uint32_t screen_update_nc200(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void nc200_io(address_map &map);

private:
	void nc200_video_set_backlight(int state);
	void nc200_refresh_uart_interrupt();

	uint8_t m_nc200_uart_interrupt_irq = 0;
	int m_nc200_backlight = 0;
};


#endif // MAME_INCLUDES_NC_H
