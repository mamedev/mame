class pitnrun_state : public driver_device
{
public:
	pitnrun_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in machine/pitnrun.c -----------*/

WRITE8_HANDLER (pitnrun_68705_portA_w);
WRITE8_HANDLER (pitnrun_68705_portB_w);

READ8_HANDLER (pitnrun_68705_portA_r);
READ8_HANDLER (pitnrun_68705_portB_r);
READ8_HANDLER (pitnrun_68705_portC_r);

MACHINE_RESET( pitnrun );

READ8_HANDLER( pitnrun_mcu_data_r );
READ8_HANDLER( pitnrun_mcu_status_r );
WRITE8_HANDLER( pitnrun_mcu_data_w );


/*----------- defined in video/pitnrun.c -----------*/

extern UINT8* pitnrun_videoram2;

WRITE8_HANDLER( pitnrun_videoram_w );
WRITE8_HANDLER( pitnrun_videoram2_w );
WRITE8_HANDLER(pitnrun_ha_w);
WRITE8_HANDLER(pitnrun_h_heed_w);
WRITE8_HANDLER(pitnrun_v_heed_w);
WRITE8_HANDLER(pitnrun_color_select_w);
WRITE8_HANDLER( pitnrun_char_bank_select );
WRITE8_HANDLER( pitnrun_scroll_w );

PALETTE_INIT(pitnrun);
VIDEO_START(pitnrun);
VIDEO_UPDATE(pitnrun);
