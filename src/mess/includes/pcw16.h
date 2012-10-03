/*****************************************************************************
 *
 * includes/pcw16.h
 *
 ****************************************************************************/

#ifndef PCW16_H_
#define PCW16_H_


#define PCW16_BORDER_HEIGHT 8
#define PCW16_BORDER_WIDTH 8
#define PCW16_NUM_COLOURS 32
#define PCW16_DISPLAY_WIDTH 640
#define PCW16_DISPLAY_HEIGHT 480

#define PCW16_SCREEN_WIDTH	(PCW16_DISPLAY_WIDTH + (PCW16_BORDER_WIDTH<<1))
#define PCW16_SCREEN_HEIGHT	(PCW16_DISPLAY_HEIGHT  + (PCW16_BORDER_HEIGHT<<1))


class pcw16_state : public driver_device
{
public:
	pcw16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	unsigned long m_interrupt_counter;
	int m_banks[4];
	int m_4_bit_port;
	int m_fdc_int_code;
	int m_system_status;
	char *m_mem_ptr[4];
	unsigned char m_keyboard_data_shift;
	int m_keyboard_parity_table[256];
	int m_keyboard_bits;
	int m_keyboard_bits_output;
	int m_keyboard_state;
	int m_keyboard_previous_state;
	unsigned char m_rtc_seconds;
	unsigned char m_rtc_minutes;
	unsigned char m_rtc_hours;
	unsigned char m_rtc_days_max;
	unsigned char m_rtc_days;
	unsigned char m_rtc_months;
	unsigned char m_rtc_years;
	unsigned char m_rtc_control;
	unsigned char m_rtc_256ths_seconds;
	int m_previous_fdc_int_state;
	int m_colour_palette[16];
	int m_video_control;
	DECLARE_WRITE8_MEMBER(pcw16_palette_w);
	DECLARE_READ8_MEMBER(pcw16_bankhw_r);
	DECLARE_WRITE8_MEMBER(pcw16_bankhw_w);
	DECLARE_WRITE8_MEMBER(pcw16_video_control_w);
	DECLARE_READ8_MEMBER(pcw16_keyboard_data_shift_r);
	DECLARE_WRITE8_MEMBER(pcw16_keyboard_data_shift_w);
	DECLARE_READ8_MEMBER(pcw16_keyboard_status_r);
	DECLARE_WRITE8_MEMBER(pcw16_keyboard_control_w);
	DECLARE_READ8_MEMBER(rtc_year_invalid_r);
	DECLARE_READ8_MEMBER(rtc_month_r);
	DECLARE_READ8_MEMBER(rtc_days_r);
	DECLARE_READ8_MEMBER(rtc_hours_r);
	DECLARE_READ8_MEMBER(rtc_minutes_r);
	DECLARE_READ8_MEMBER(rtc_seconds_r);
	DECLARE_READ8_MEMBER(rtc_256ths_seconds_r);
	DECLARE_WRITE8_MEMBER(rtc_control_w);
	DECLARE_WRITE8_MEMBER(rtc_seconds_w);
	DECLARE_WRITE8_MEMBER(rtc_minutes_w);
	DECLARE_WRITE8_MEMBER(rtc_hours_w);
	DECLARE_WRITE8_MEMBER(rtc_days_w);
	DECLARE_WRITE8_MEMBER(rtc_month_w);
	DECLARE_WRITE8_MEMBER(rtc_year_w);
	DECLARE_READ8_MEMBER(pcw16_system_status_r);
	DECLARE_READ8_MEMBER(pcw16_timer_interrupt_counter_r);
	DECLARE_WRITE8_MEMBER(pcw16_system_control_w);
	DECLARE_WRITE8_MEMBER(pcw16_superio_fdc_datarate_w);
	DECLARE_WRITE8_MEMBER(pcw16_superio_fdc_digital_output_register_w);
	DECLARE_WRITE8_MEMBER(pcw16_superio_fdc_data_w);
	DECLARE_READ8_MEMBER(pcw16_superio_fdc_data_r);
	DECLARE_READ8_MEMBER(pcw16_superio_fdc_main_status_register_r);
	DECLARE_READ8_MEMBER(pcw16_superio_fdc_digital_input_register_r);
	DECLARE_READ8_MEMBER(pcw16_mem_r);
	DECLARE_WRITE8_MEMBER(pcw16_mem_w);
	void pcw16_keyboard_init();
	void pcw16_keyboard_refresh_outputs();
	void pcw16_keyboard_set_clock_state(int state);
	void pcw16_keyboard_int(int state);
	void pcw16_keyboard_reset();
	int pcw16_keyboard_can_transmit();
	void pcw16_keyboard_signal_byte_received(int data);
	void pcw16_refresh_ints();
	void rtc_setup_max_days();
	UINT8 pcw16_read_mem(UINT8 bank, UINT16 offset);
	void pcw16_write_mem(UINT8 bank, UINT16 offset, UINT8 data);
	UINT8 read_bank_data(UINT8 type, UINT16 offset);
	void write_bank_data(UINT8 type, UINT16 offset, UINT8 data);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_pcw16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(pcw16_timer_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(pcw16_keyboard_timer_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(rtc_timer_callback);
	DECLARE_WRITE_LINE_MEMBER(pcw16_com_interrupt_1);
	DECLARE_WRITE_LINE_MEMBER(pcw16_com_interrupt_2);
	DECLARE_WRITE_LINE_MEMBER(pcw16_com_tx_0);
	DECLARE_WRITE_LINE_MEMBER(pcw16_com_dtr_0);
	DECLARE_WRITE_LINE_MEMBER(pcw16_com_rts_0);
	DECLARE_WRITE_LINE_MEMBER(pcw16_com_tx_1);
	DECLARE_WRITE_LINE_MEMBER(pcw16_com_dtr_1);
	DECLARE_WRITE_LINE_MEMBER(pcw16_com_rts_1);
};

#endif /* PCW16_H_ */
