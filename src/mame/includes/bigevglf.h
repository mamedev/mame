
class bigevglf_state : public driver_device
{
public:
	bigevglf_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  paletteram;
	UINT8 *  spriteram1;
	UINT8 *  spriteram2;

	/* video-related */
	bitmap_t *tmp_bitmap[4];
	UINT8    *vidram;
	UINT32   vidram_bank, plane_selected, plane_visible;

	/* sound-related */
	int      sound_nmi_enable, pending_nmi;
	UINT8    for_sound;
	UINT8    from_sound;
	UINT8    sound_state;

	/* MCU related */
	UINT8    from_mcu;
	int      mcu_sent, main_sent;
	UINT8    port_a_in, port_a_out, ddr_a;
	UINT8    port_b_in, port_b_out, ddr_b;
	UINT8    port_c_in, port_c_out, ddr_c;
	int      mcu_coin_bit5;

	/* misc */
	UINT32   beg_bank;
	UINT8    beg13_ls74[2];
	UINT8    port_select;     /* for muxed controls */

	/* devices */
	running_device *audiocpu;
	running_device *mcu;
};


/*----------- defined in machine/bigevglf.c -----------*/

READ8_HANDLER( bigevglf_68705_port_a_r );
WRITE8_HANDLER( bigevglf_68705_port_a_w );
READ8_HANDLER( bigevglf_68705_port_b_r );
WRITE8_HANDLER( bigevglf_68705_port_b_w );
READ8_HANDLER( bigevglf_68705_port_c_r );
WRITE8_HANDLER( bigevglf_68705_port_c_w );
WRITE8_HANDLER( bigevglf_68705_ddr_a_w );
WRITE8_HANDLER( bigevglf_68705_ddr_b_w );
WRITE8_HANDLER( bigevglf_68705_ddr_c_w );

WRITE8_HANDLER( bigevglf_mcu_w );
READ8_HANDLER( bigevglf_mcu_r );
READ8_HANDLER( bigevglf_mcu_status_r );


/*----------- defined in video/bigevglf.c -----------*/

VIDEO_START( bigevglf );
VIDEO_UPDATE( bigevglf );

READ8_HANDLER( bigevglf_vidram_r );
WRITE8_HANDLER( bigevglf_vidram_w );
WRITE8_HANDLER( bigevglf_vidram_addr_w );

WRITE8_HANDLER( bigevglf_gfxcontrol_w );
WRITE8_HANDLER( bigevglf_palette_w );
