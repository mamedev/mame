class retofinv_state : public driver_device
{
public:
	retofinv_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 cpu2_m6000;
	UINT8 *fg_videoram;
	UINT8 *bg_videoram;
	UINT8 *sharedram;
	UINT8 from_main;
	UINT8 from_mcu;
	int mcu_sent;
	int main_sent;
	UINT8 portA_in;
	UINT8 portA_out;
	UINT8 ddrA;
	UINT8 portB_in;
	UINT8 portB_out;
	UINT8 ddrB;
	UINT8 portC_in;
	UINT8 portC_out;
	UINT8 ddrC;
	int fg_bank;
	int bg_bank;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
};


/*----------- defined in machine/retofinv.c -----------*/

READ8_HANDLER( retofinv_68705_portA_r );
WRITE8_HANDLER( retofinv_68705_portA_w );
WRITE8_HANDLER( retofinv_68705_ddrA_w );
READ8_HANDLER( retofinv_68705_portB_r );
WRITE8_HANDLER( retofinv_68705_portB_w );
WRITE8_HANDLER( retofinv_68705_ddrB_w );
READ8_HANDLER( retofinv_68705_portC_r );
WRITE8_HANDLER( retofinv_68705_portC_w );
WRITE8_HANDLER( retofinv_68705_ddrC_w );
WRITE8_HANDLER( retofinv_mcu_w );
READ8_HANDLER( retofinv_mcu_r );
READ8_HANDLER( retofinv_mcu_status_r );


/*----------- defined in video/retofinv.c -----------*/

VIDEO_START( retofinv );
PALETTE_INIT( retofinv );
VIDEO_UPDATE( retofinv );
WRITE8_HANDLER( retofinv_bg_videoram_w );
WRITE8_HANDLER( retofinv_fg_videoram_w );
WRITE8_HANDLER( retofinv_gfx_ctrl_w );
