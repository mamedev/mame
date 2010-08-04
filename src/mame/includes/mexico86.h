
class mexico86_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mexico86_state(machine)); }

	mexico86_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *     protection_ram;
	UINT8 *     videoram;
	UINT8 *     objectram;
	size_t      objectram_size;

	/* video-related */
	int      charbank;

	/* mcu */
	/* mexico86 68705 protection */
	UINT8    port_a_in, port_a_out, ddr_a;
	UINT8    port_b_in, port_b_out, ddr_b;
	int      address, latch;
	/* kikikai mcu simulation */
	int      mcu_running, mcu_initialised;
	int      coin_last;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *subcpu;
	running_device *mcu;
};


/*----------- defined in machine/mexico86.c -----------*/

WRITE8_HANDLER( mexico86_f008_w );
INTERRUPT_GEN( kikikai_interrupt );
INTERRUPT_GEN( mexico86_m68705_interrupt );
READ8_HANDLER( mexico86_68705_port_a_r );
WRITE8_HANDLER( mexico86_68705_port_a_w );
WRITE8_HANDLER( mexico86_68705_ddr_a_w );
READ8_HANDLER( mexico86_68705_port_b_r );
WRITE8_HANDLER( mexico86_68705_port_b_w );
WRITE8_HANDLER( mexico86_68705_ddr_b_w );


/*----------- defined in video/mexico86.c -----------*/

WRITE8_HANDLER( mexico86_bankswitch_w );

VIDEO_UPDATE( mexico86 );
VIDEO_UPDATE( kikikai );
