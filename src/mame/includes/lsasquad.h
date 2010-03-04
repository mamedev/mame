
class lsasquad_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, lsasquad_state(machine)); }

	lsasquad_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *      scrollram;
	UINT8 *      videoram;
	UINT8 *      spriteram;
	size_t       spriteram_size;
	size_t       videoram_size;

	/* misc */
	int invertcoin;
	int sound_pending;
	int sound_nmi_enable, pending_nmi, sound_cmd, sound_result;

	/* daikaiju */
	int daikaiju_xor, daikaiju_command, daikaiju_length, daikaiju_prev, daikaiju_cnt, daikaiju_cntr;
	int daikaiju_buffer[256];

	/* mcu */
	UINT8 from_main, from_mcu;
	int mcu_sent, main_sent;
	UINT8 port_a_in, port_a_out, ddr_a;
	UINT8 port_b_in, port_b_out, ddr_b;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *mcu;
};


/*----------- defined in machine/daikaiju.c -----------*/

READ8_HANDLER( daikaiju_mcu_r);
WRITE8_HANDLER( daikaiju_mcu_w);
READ8_HANDLER( daikaiju_mcu_status_r);

/*----------- defined in machine/lsasquad.c -----------*/

WRITE8_HANDLER( lsasquad_sh_nmi_disable_w );
WRITE8_HANDLER( lsasquad_sh_nmi_enable_w );
WRITE8_HANDLER( lsasquad_sound_command_w );
READ8_HANDLER( lsasquad_sh_sound_command_r );
WRITE8_HANDLER( lsasquad_sh_result_w );
READ8_HANDLER( lsasquad_sound_result_r );
READ8_HANDLER( lsasquad_sound_status_r );

READ8_HANDLER( lsasquad_68705_port_a_r );
WRITE8_HANDLER( lsasquad_68705_port_a_w );
WRITE8_HANDLER( lsasquad_68705_ddr_a_w );
READ8_HANDLER( lsasquad_68705_port_b_r );
WRITE8_HANDLER( lsasquad_68705_port_b_w );
WRITE8_HANDLER( lsasquad_68705_ddr_b_w );
WRITE8_HANDLER( lsasquad_mcu_w );
READ8_HANDLER( lsasquad_mcu_r );
READ8_HANDLER( lsasquad_mcu_status_r );

READ8_HANDLER( daikaiju_sound_status_r );
READ8_HANDLER( daikaiju_sh_sound_command_r );


/*----------- defined in video/lsasquad.c -----------*/

VIDEO_UPDATE( lsasquad );
VIDEO_UPDATE( daikaiju );
