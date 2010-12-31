
class mexico86_state : public driver_device
{
public:
	mexico86_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

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
	device_t *maincpu;
	device_t *audiocpu;
	device_t *subcpu;
	device_t *mcu;
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
