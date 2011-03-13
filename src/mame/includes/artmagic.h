/*************************************************************************

    Art & Magic hardware

**************************************************************************/

class artmagic_state : public driver_device
{
public:
	artmagic_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *control;
	UINT8 tms_irq;
	UINT8 hack_irq;
	UINT8 prot_input[16];
	UINT8 prot_input_index;
	UINT8 prot_output[16];
	UINT8 prot_output_index;
	UINT8 prot_output_bit;
	UINT8 prot_bit_index;
	UINT16 prot_save;
	void (*protection_handler)(running_machine *);
	UINT16 *vram0;
	UINT16 *vram1;
	int _xor[16];
	int is_stoneball;
	UINT16 *blitter_base;
	UINT32 blitter_mask;
	UINT16 blitter_data[8];
	UINT8 blitter_page;
	attotime blitter_busy_until;
};


/*----------- defined in video/artmagic.c -----------*/

VIDEO_START( artmagic );

void artmagic_to_shiftreg(address_space *space, offs_t address, UINT16 *data);
void artmagic_from_shiftreg(address_space *space, offs_t address, UINT16 *data);

READ16_HANDLER( artmagic_blitter_r );
WRITE16_HANDLER( artmagic_blitter_w );

void artmagic_scanline(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);
