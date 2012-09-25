/*****************************************************************************
 *
 * includes/nc.h
 *
 ****************************************************************************/

#ifndef NC_H_
#define NC_H_


#define NC_NUM_COLOURS 4

#define NC_SCREEN_WIDTH        480
#define NC_SCREEN_HEIGHT       64

#define NC200_SCREEN_WIDTH		480
#define NC200_SCREEN_HEIGHT		128

#define NC200_NUM_COLOURS 4

enum
{
	NC_TYPE_1xx, /* nc100/nc150 */
	NC_TYPE_200  /* nc200 */
};


class nc_state : public driver_device
{
public:
	nc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	emu_timer *m_serial_timer;
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
	emu_file *m_file;
	int m_previous_inputport_10_state;
	int m_previous_alarm_state;
	UINT8 m_nc200_uart_interrupt_irq;
	unsigned char *m_card_ram;
	int m_membank_card_ram_mask;
	unsigned long m_display_memory_start;
	UINT8 m_type;
	int m_card_size;
	int m_nc200_backlight;
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
	DECLARE_WRITE8_MEMBER(nc200_display_memory_start_w);
	DECLARE_READ8_MEMBER(nc200_card_battery_status_r);
	DECLARE_READ8_MEMBER(nc200_printer_status_r);
	DECLARE_WRITE8_MEMBER(nc200_uart_control_w);
	DECLARE_WRITE8_MEMBER(nc200_memory_card_wait_state_w);
	DECLARE_WRITE8_MEMBER(nc200_poweroff_control_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	DECLARE_MACHINE_START(nc200);
	DECLARE_MACHINE_RESET(nc200);
	UINT32 screen_update_nc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(nc_keyboard_timer_callback);
	TIMER_CALLBACK_MEMBER(nc_serial_timer_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(dummy_timer_callback);
};


/*----------- defined in video/nc.c -----------*/

void nc200_video_set_backlight(running_machine &machine, int state);


/*----------- defined in drivers/nc.c -----------*/

/* pointer to loaded data */
/* mask used to stop access over end of card ram area */


void nc_set_card_present_state(running_machine &machine, int state);


/*----------- defined in machine/nc.c -----------*/

DEVICE_START( nc_pcmcia_card );
DEVICE_IMAGE_LOAD( nc_pcmcia_card );
DEVICE_IMAGE_UNLOAD( nc_pcmcia_card );

#endif /* NC_H_ */
