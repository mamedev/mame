/***************************************************************************

    Sigma Spiders hardware

***************************************************************************/


#define NUM_PENS	(8)

class spiders_state : public driver_device
{
public:
	spiders_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *ram;
	UINT8 flipscreen;
	UINT16 gfx_rom_address;
	UINT8 gfx_rom_ctrl_mode;
	UINT8 gfx_rom_ctrl_latch;
	UINT8 gfx_rom_ctrl_data;
	pen_t pens[NUM_PENS];
};


/*----------- defined in audio/spiders.c -----------*/

WRITE8_DEVICE_HANDLER( spiders_audio_command_w );
WRITE8_DEVICE_HANDLER( spiders_audio_a_w );
WRITE8_DEVICE_HANDLER( spiders_audio_b_w );
WRITE8_DEVICE_HANDLER( spiders_audio_ctrl_w );

MACHINE_CONFIG_EXTERN( spiders_audio );
