/*****************************************************************************
 *
 * includes/pc.h
 *
 ****************************************************************************/

#ifndef PC_H_
#define PC_H_

#include "machine/ins8250.h"
#include "machine/i8255.h"
#include "machine/8237dma.h"
#include "machine/serial.h"
#include "machine/ser_mouse.h"
#include "machine/pc_kbdc.h"

class pc_state : public driver_device
{
public:
	pc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_pc_kbdc(*this, "pc_kbdc")
	{
	}

	cpu_device *m_maincpu;
	device_t *m_pic8259;
	device_t *m_dma8237;
	device_t *m_pit8253;
	optional_device<pc_kbdc_device>  m_pc_kbdc;

	/* U73 is an LS74 - dual flip flop */
	/* Q2 is set by OUT1 from the 8253 and goes to DRQ1 on the 8237 */
	UINT8	m_u73_q2;
	UINT8	m_out1;
	UINT8	m_memboard[4];		/* used only by ec1840 and ec1841 */
	int m_dma_channel;
	UINT8 m_dma_offset[2][4];
	UINT8 m_pc_spkrdata;
	UINT8 m_pc_input;

	int						m_ppi_portc_switch_high;
	int						m_ppi_speaker;
	int						m_ppi_keyboard_clear;
	UINT8					m_ppi_keyb_clock;
	UINT8					m_ppi_portb;
	UINT8					m_ppi_clock_signal;
	UINT8					m_ppi_data_signal;
	UINT8					m_ppi_shift_register;
	UINT8					m_ppi_shift_enable;

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
	DECLARE_READ8_MEMBER(ec1841_memboard_r);
	DECLARE_WRITE8_MEMBER(ec1841_memboard_w);
	DECLARE_DRIVER_INIT(europc);
	DECLARE_DRIVER_INIT(mc1502);
	DECLARE_DRIVER_INIT(bondwell);
	DECLARE_DRIVER_INIT(pcjr);
	DECLARE_DRIVER_INIT(pccga);
	DECLARE_DRIVER_INIT(t1000hx);
	DECLARE_DRIVER_INIT(ppc512);
	DECLARE_DRIVER_INIT(pc200);
	DECLARE_DRIVER_INIT(ibm5150);
	DECLARE_DRIVER_INIT(pcmda);
	DECLARE_DRIVER_INIT(pc1512);
	DECLARE_DRIVER_INIT(pc1640);
	DECLARE_MACHINE_START(pc);
	DECLARE_MACHINE_RESET(pc);
	DECLARE_MACHINE_RESET(tandy1000rl);
	DECLARE_MACHINE_START(pcjr);
	DECLARE_MACHINE_RESET(pcjr);
	DECLARE_MACHINE_START(mc1502);
	TIMER_CALLBACK_MEMBER(pcjr_delayed_pic8259_irq);
	TIMER_CALLBACK_MEMBER(pcjr_keyb_signal_callback);
	TIMER_CALLBACK_MEMBER(mc1502_keyb_signal_callback);
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
	DECLARE_READ8_MEMBER(mc1502_ppi_porta_r);
	DECLARE_WRITE8_MEMBER(mc1502_ppi_porta_w);
	DECLARE_WRITE8_MEMBER(mc1502_ppi_portb_w);
	DECLARE_READ8_MEMBER(mc1502_ppi_portc_r);
	DECLARE_READ8_MEMBER(mc1502_kppi_porta_r);
	DECLARE_WRITE8_MEMBER(mc1502_kppi_portb_w);
	DECLARE_WRITE8_MEMBER(mc1502_kppi_portc_w);
	DECLARE_WRITE8_MEMBER(pcjr_ppi_portb_w);
	DECLARE_READ8_MEMBER(pcjr_ppi_porta_r);
	DECLARE_READ8_MEMBER(pcjr_ppi_portc_r);
	DECLARE_READ8_MEMBER(mc1502_wd17xx_aux_r);
	DECLARE_WRITE8_MEMBER(mc1502_wd17xx_aux_w);
	DECLARE_READ8_MEMBER(mc1502_wd17xx_drq_r);
	DECLARE_READ8_MEMBER(mc1502_wd17xx_motor_r);
};

/*----------- defined in machine/pc.c -----------*/

extern const i8237_interface ibm5150_dma8237_config;
extern const struct pit8253_config ibm5150_pit8253_config;
extern const struct pit8253_config pcjr_pit8253_config;
extern const struct pit8253_config mc1502_pit8253_config;
extern const struct pic8259_interface ibm5150_pic8259_config;
extern const struct pic8259_interface pcjr_pic8259_config;
extern const ins8250_interface ibm5150_com_interface[4];
extern const rs232_port_interface ibm5150_serport_config[4];
extern const i8255_interface ibm5150_ppi8255_interface;
extern const i8255_interface ibm5160_ppi8255_interface;
extern const i8255_interface pc_ppi8255_interface;
extern const i8255_interface pcjr_ppi8255_interface;
extern const i8255_interface mc1502_ppi8255_interface;
extern const i8255_interface mc1502_ppi8255_interface_2;

UINT8 pc_speaker_get_spk(running_machine &machine);
void pc_speaker_set_spkrdata(running_machine &machine, UINT8 data);
void pc_speaker_set_input(running_machine &machine, UINT8 data);

void mess_init_pc_common( running_machine &machine, UINT32 flags, void (*set_keyb_int_func)(running_machine &, int), void (*set_hdc_int_func)(running_machine &,int,int));


DEVICE_IMAGE_LOAD( pcjr_cartridge );

void pc_rtc_init(running_machine &machine);


#endif /* PC_H_ */
