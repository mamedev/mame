class raiden2_state : public driver_device
{
public:
	raiden2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
};


/*----------- defined in drivers/raiden2.c -----------*/

WRITE16_HANDLER( sprcpt_val_1_w );
WRITE16_HANDLER( sprcpt_val_2_w );
WRITE16_HANDLER( sprcpt_data_1_w );
WRITE16_HANDLER( sprcpt_data_2_w );
WRITE16_HANDLER( sprcpt_data_3_w );
WRITE16_HANDLER( sprcpt_data_4_w );
WRITE16_HANDLER( sprcpt_adr_w );
WRITE16_HANDLER( sprcpt_flags_1_w );
WRITE16_HANDLER( sprcpt_flags_2_w );


/*----------- defined in machine/r2crypt.c -----------*/

void raiden2_decrypt_sprites(running_machine *machine);
