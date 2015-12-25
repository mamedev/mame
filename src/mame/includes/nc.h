// license:GPL-2.0+
// copyright-holders:Wilbert Pol, Kevin Thacker
/*****************************************************************************
 *
 * includes/nc.h
 *
 ****************************************************************************/

#ifndef NC_H_
#define NC_H_

#include "bus/centronics/ctronics.h"
#include "machine/i8251.h"
#include "machine/clock.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "sound/beep.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#define NC_NUM_COLOURS 4

#define NC_SCREEN_WIDTH        480
#define NC_SCREEN_HEIGHT       64

#define NC200_SCREEN_WIDTH      480
#define NC200_SCREEN_HEIGHT     128

#define NC200_NUM_COLOURS 4

enum
{
	NC_TYPE_1xx, /* nc100/nc150 */
	NC_TYPE_200  /* nc200 */
};


class nc_state : public driver_device
{
public:
	nc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_beeper1(*this, "beep.1"),
		m_beeper2(*this, "beep.2"),
		m_centronics(*this, "centronics"),
		m_card(*this, "cardslot"),
		m_uart(*this, "uart"),
		m_uart_clock(*this, "uart_clock"),
		m_nvram(*this, "nvram")
	{
	}

	DECLARE_READ8_MEMBER(nc_memory_management_r);
	DECLARE_WRITE8_MEMBER(nc_memory_management_w);
	DECLARE_WRITE8_MEMBER(nc_irq_mask_w);
	DECLARE_WRITE8_MEMBER(nc_irq_status_w);
	DECLARE_READ8_MEMBER(nc_irq_status_r);
	DECLARE_READ8_MEMBER(nc_key_data_in_r);
	DECLARE_WRITE8_MEMBER(nc_sound_w);
	DECLARE_WRITE8_MEMBER(nc_uart_control_w);
	DECLARE_WRITE8_MEMBER(nc100_display_memory_start_w);
	DECLARE_WRITE8_MEMBER(nc100_uart_control_w);
	DECLARE_WRITE8_MEMBER(nc100_poweroff_control_w);
	DECLARE_READ8_MEMBER(nc100_card_battery_status_r);
	DECLARE_WRITE8_MEMBER(nc100_memory_card_wait_state_w);
	DECLARE_READ8_MEMBER(nc200_card_battery_status_r);
	DECLARE_READ8_MEMBER(nc200_printer_status_r);
	DECLARE_WRITE8_MEMBER(nc200_uart_control_w);
	DECLARE_WRITE8_MEMBER(nc200_memory_card_wait_state_w);
	DECLARE_WRITE8_MEMBER(nc200_poweroff_control_w);

	DECLARE_PALETTE_INIT(nc);
	DECLARE_MACHINE_START(nc200);
	DECLARE_MACHINE_RESET(nc200);
	UINT32 screen_update_nc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(nc_keyboard_timer_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(dummy_timer_callback);
	DECLARE_WRITE_LINE_MEMBER(nc100_tc8521_alarm_callback);
	DECLARE_WRITE_LINE_MEMBER(nc100_txrdy_callback);
	DECLARE_WRITE_LINE_MEMBER(nc100_rxrdy_callback);
	DECLARE_WRITE_LINE_MEMBER(write_uart_clock);
	DECLARE_WRITE_LINE_MEMBER(write_nc100_centronics_ack);
	DECLARE_WRITE_LINE_MEMBER(write_nc200_centronics_ack);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_WRITE_LINE_MEMBER(nc200_txrdy_callback);
	DECLARE_WRITE_LINE_MEMBER(nc200_rxrdy_callback);
	DECLARE_WRITE_LINE_MEMBER(nc200_fdc_interrupt);

	DECLARE_DRIVER_INIT( nc );
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( nc_pcmcia_card );
	DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER( nc_pcmcia_card );

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	void nc200_video_set_backlight(int state);
	int card_calculate_mask(int size);
	void set_card_present_state(int state);
	void nc_update_interrupts();
	void nc_refresh_memory_bank_config(int bank);
	void nc_refresh_memory_config();
	void nc_common_init_machine();
	void nc_sound_update(int channel);
	void nc200_refresh_uart_interrupt();

public: // HACK FOR MC6845
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<beep_device> m_beeper1;
	required_device<beep_device> m_beeper2;
	required_device<centronics_device> m_centronics;
	required_device<generic_slot_device> m_card;
	required_device<i8251_device> m_uart;
	required_device<clock_device> m_uart_clock;
	required_device<nvram_device> m_nvram;

	char m_memory_config[4];
	emu_timer *m_keyboard_timer;
	int m_membank_rom_mask;
	int m_membank_internal_ram_mask;
	UINT8 m_poweroff_control;
	int m_card_status;
	unsigned char m_uart_control;
	int m_irq_mask;
	int m_irq_status;
	int m_irq_latch;
	int m_irq_latch_mask;
	int m_sound_channel_periods[2];
	int m_previous_inputport_10_state;
	int m_previous_alarm_state;
	UINT8 m_nc200_uart_interrupt_irq;
	memory_region *m_card_ram;
	int m_membank_card_ram_mask;
	int m_card_size;
	unsigned long m_display_memory_start;
	UINT8 m_type;
	int m_nc200_backlight;

	int m_centronics_ack;
	int m_centronics_busy;
};



#endif /* NC_H_ */
