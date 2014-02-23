/*****************************************************************************
 *
 * includes/pc.h
 *
 ****************************************************************************/

#ifndef PC_H_
#define PC_H_

#include "machine/ins8250.h"
#include "machine/i8255.h"
#include "machine/am9517a.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "machine/upd765.h"
#include "sound/speaker.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "bus/centronics/ctronics.h"

class pc_state : public driver_device
{
public:
	pc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic8259(*this, "pic8259"),
		m_dma8237(*this, "dma8237"),
		m_pit8253(*this, "pit8253"),
		m_pc_kbdc(*this, "pc_kbdc"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_centronics(*this, "centronics"),
		m_ram(*this, RAM_TAG) { }

	required_device<cpu_device> m_maincpu;
	optional_device<pic8259_device> m_pic8259;
	optional_device<am9517a_device> m_dma8237;
	optional_device<pit8253_device> m_pit8253;
	optional_device<pc_kbdc_device>  m_pc_kbdc;
	optional_device<speaker_sound_device> m_speaker;
	optional_device<cassette_image_device> m_cassette;
	optional_device<centronics_device> m_centronics;
	optional_device<ram_device> m_ram;

	/* U73 is an LS74 - dual flip flop */
	/* Q2 is set by OUT1 from the 8253 and goes to DRQ1 on the 8237 */
	UINT8   m_u73_q2;
	UINT8   m_out1;
	int m_dma_channel;
	UINT8 m_dma_offset[2][4];
	int m_cur_eop;
	UINT8 m_pc_spkrdata;
	UINT8 m_pit_out2;
	UINT8 m_pcjr_dor;
	emu_timer *m_pcjr_watchdog;
	UINT8 m_pcjx_1ff_count;
	UINT8 m_pcjx_1ff_val;
	UINT8 m_pcjx_1ff_bankval;
	UINT8 m_pcjx_1ff_bank[20][2];

	int                     m_ppi_portc_switch_high;
	int                     m_ppi_speaker;
	int                     m_ppi_keyboard_clear;
	UINT8                   m_ppi_keyb_clock;
	UINT8                   m_ppi_portb;
	UINT8                   m_ppi_clock_signal;
	UINT8                   m_ppi_data_signal;
	UINT8                   m_ppi_shift_register;
	UINT8                   m_ppi_shift_enable;

	// interface to the keyboard
	DECLARE_WRITE_LINE_MEMBER( keyboard_clock_w );
	DECLARE_WRITE_LINE_MEMBER( keyboard_data_w );

	DECLARE_READ8_MEMBER(pc_page_r);
	DECLARE_WRITE8_MEMBER(pc_page_w);
	DECLARE_READ8_MEMBER(pc_dma_read_byte);
	DECLARE_WRITE8_MEMBER(pc_dma_write_byte);
	DECLARE_WRITE8_MEMBER(pc_nmi_enable_w);
	DECLARE_READ8_MEMBER(pcjr_nmi_enable_r);
	DECLARE_READ8_MEMBER(pc_rtc_r);
	DECLARE_WRITE8_MEMBER(pc_rtc_w);
	DECLARE_WRITE8_MEMBER(pc_EXP_w);
	DECLARE_READ8_MEMBER(pc_EXP_r);
	DECLARE_READ8_MEMBER(unk_r);
	DECLARE_DRIVER_INIT(bondwell);
	DECLARE_DRIVER_INIT(pcjr);
	DECLARE_DRIVER_INIT(pccga);
	DECLARE_DRIVER_INIT(ibm5150);
	DECLARE_DRIVER_INIT(pcmda);
	DECLARE_MACHINE_START(pc);
	DECLARE_MACHINE_RESET(pc);
	DECLARE_MACHINE_START(pcjr);
	DECLARE_MACHINE_RESET(pcjr);
	TIMER_CALLBACK_MEMBER(pcjr_delayed_pic8259_irq);
	TIMER_CALLBACK_MEMBER(pcjr_keyb_signal_callback);
	TIMER_CALLBACK_MEMBER(pcjr_fdc_watchdog);
	TIMER_CALLBACK_MEMBER(pc_rtc_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(pc_frame_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(pc_vga_frame_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(pcjr_frame_interrupt);
	DECLARE_WRITE_LINE_MEMBER(pc_dma_hrq_changed);
	DECLARE_READ8_MEMBER(pc_dma8237_fdc_dack_r);
	DECLARE_READ8_MEMBER(pc_dma8237_hdc_dack_r);
	DECLARE_WRITE8_MEMBER(pc_dma8237_fdc_dack_w);
	DECLARE_WRITE8_MEMBER(pc_dma8237_hdc_dack_w);
	DECLARE_WRITE8_MEMBER(pc_dma8237_0_dack_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dma8237_out_eop);
	DECLARE_WRITE_LINE_MEMBER(pc_dack0_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack1_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack2_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack3_w);
	DECLARE_WRITE_LINE_MEMBER(pcjr_pic8259_set_int_line);
	DECLARE_WRITE_LINE_MEMBER(ibm5150_pit8253_out1_changed);
	DECLARE_WRITE_LINE_MEMBER(ibm5150_pit8253_out2_changed);
	DECLARE_WRITE_LINE_MEMBER(pc_com_interrupt_1);
	DECLARE_WRITE_LINE_MEMBER(pc_com_interrupt_2);
	DECLARE_READ8_MEMBER(ibm5160_ppi_porta_r);
	DECLARE_READ8_MEMBER(ibm5160_ppi_portc_r);
	DECLARE_WRITE8_MEMBER(ibm5160_ppi_portb_w);
	DECLARE_READ8_MEMBER(pc_ppi_porta_r);
	DECLARE_WRITE8_MEMBER(pc_ppi_portb_w);
	DECLARE_WRITE8_MEMBER(pcjr_ppi_portb_w);
	DECLARE_READ8_MEMBER(pcjr_ppi_porta_r);
	DECLARE_READ8_MEMBER(pcjr_ppi_portc_r);
	DECLARE_WRITE8_MEMBER(pcjr_fdc_dor_w);
	DECLARE_READ8_MEMBER(pcjx_port_1ff_r);
	DECLARE_WRITE8_MEMBER(pcjx_port_1ff_w);
	DECLARE_WRITE8_MEMBER(asst128_fdc_dor_w);
	void pcjx_set_bank(int unk1, int unk2, int unk3);

	void fdc_interrupt(bool state);
	void fdc_dma_drq(bool state);
	void pc_select_dma_channel(int channel, bool state);
	void pc_eop_w(int channel, bool state);
	DECLARE_FLOPPY_FORMATS( floppy_formats );
	DECLARE_FLOPPY_FORMATS( asst128_formats );
	IRQ_CALLBACK_MEMBER(pc_irq_callback);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( pcjr_cartridge );
	UINT8 pc_speaker_get_spk();
	void pc_speaker_set_spkrdata(UINT8 data);
	void pcjr_keyb_init();
	void mess_init_pc_common(void (*set_keyb_int_func)(running_machine &, int));
	void pc_rtc_init();

	// turbo support
	TIMER_CALLBACK_MEMBER(pc_turbo_callback);
	void pc_turbo_setup(double off_speed, double on_speed);

	int m_turbo_cur_val;
	double m_turbo_off_speed;
	double m_turbo_on_speed;

	// keyboard
	void init_pc_common(void (*set_keyb_int_func)(running_machine &, int));
	TIMER_CALLBACK_MEMBER( pc_keyb_timer );
	void pc_keyboard();
	UINT8 pc_keyb_read();
	void pc_keyb_set_clock(int on);
	void pc_keyb_clear();
	void (*m_pc_keyb_int_cb)(running_machine &, int);
	emu_timer *m_pc_keyb_timer;
	UINT8 m_pc_keyb_data;
	int m_pc_keyb_on;
	int m_pc_keyb_self_test;
};

void pc_set_keyb_int(running_machine &machine, int state);

/*----------- defined in machine/pc.c -----------*/

extern const struct am9517a_interface ibm5150_dma8237_config;
extern const ins8250_interface ibm5150_com_interface[4];
extern const i8255_interface ibm5160_ppi8255_interface;
extern const i8255_interface pc_ppi8255_interface;
extern const i8255_interface pcjr_ppi8255_interface;

#endif /* PC_H_ */
