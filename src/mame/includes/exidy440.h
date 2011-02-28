/*************************************************************************

    Exidy 440 hardware

*************************************************************************/

#define EXIDY440_MASTER_CLOCK		(XTAL_12_9792MHz)


class exidy440_state : public driver_device
{
public:
	exidy440_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 bank;
	const UINT8 *showdown_bank_data[2];
	INT8 showdown_bank_select;
	UINT8 showdown_bank_offset;
	UINT8 *imageram;
	UINT8 *scanline;
	UINT8 firq_vblank;
	UINT8 firq_beam;
	UINT8 *topsecex_yscroll;
	UINT8 latched_x;
	UINT8 *local_videoram;
	UINT8 *local_paletteram;
	UINT8 firq_enable;
	UINT8 firq_select;
	UINT8 palettebank_io;
	UINT8 palettebank_vis;
};


/*----------- defined in drivers/exidy440.c -----------*/

void exidy440_bank_select(running_machine *machine, UINT8 bank);


/*----------- defined in video/exidy440.c -----------*/

INTERRUPT_GEN( exidy440_vblank_interrupt );

READ8_HANDLER( exidy440_videoram_r );
WRITE8_HANDLER( exidy440_videoram_w );
READ8_HANDLER( exidy440_paletteram_r );
WRITE8_HANDLER( exidy440_paletteram_w );
WRITE8_HANDLER( exidy440_spriteram_w );
WRITE8_HANDLER( exidy440_control_w );
READ8_HANDLER( exidy440_vertical_pos_r );
READ8_HANDLER( exidy440_horizontal_pos_r );
WRITE8_HANDLER( exidy440_interrupt_clear_w );

MACHINE_CONFIG_EXTERN( exidy440_video );
MACHINE_CONFIG_EXTERN( topsecex_video );
