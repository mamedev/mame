/***************************************************************************

    tecmosys protection simulation

***************************************************************************/

class tecmosys_state : public driver_device
{
public:
	tecmosys_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16* spriteram;
	UINT16* tilemap_paletteram16;
	UINT16* bg2tilemap_ram;
	UINT16* bg1tilemap_ram;
	UINT16* bg0tilemap_ram;
	UINT16* fgtilemap_ram;
	UINT16* bg0tilemap_lineram;
	UINT16* bg1tilemap_lineram;
	UINT16* bg2tilemap_lineram;
	UINT16* a80000regs;
	UINT16* b00000regs;
	UINT16* c00000regs;
	UINT16* c80000regs;
	UINT16* _880000regs;
	int spritelist;
	bitmap_t *sprite_bitmap;
	bitmap_t *tmp_tilemap_composebitmap;
	bitmap_t *tmp_tilemap_renderbitmap;
	tilemap_t *bg0tilemap;
	tilemap_t *bg1tilemap;
	tilemap_t *bg2tilemap;
	tilemap_t *txt_tilemap;
	UINT8 device_read_ptr;
	UINT8 device_status;
	const struct prot_data* device_data;
	UINT8 device_value;
};


/*----------- defined in machine/tecmosys.c -----------*/

void tecmosys_prot_init(running_machine *machine, int which);

READ16_HANDLER(tecmosys_prot_status_r);
WRITE16_HANDLER(tecmosys_prot_status_w);
READ16_HANDLER(tecmosys_prot_data_r);
WRITE16_HANDLER(tecmosys_prot_data_w);




