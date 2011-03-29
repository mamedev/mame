#include "machine/intelfsh.h"

#define FIFO_SIZE 512

class seibuspi_state : public driver_device
{
public:
	seibuspi_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT32 *spimainram;
	UINT32 *spi_scrollram;
	intel_e28f008sa_device *flash[2];
	UINT8 *z80_rom;
	int z80_prg_fifo_pos;
	int z80_lastbank;
	int fifoin_rpos;
	int fifoin_wpos;
	UINT8 fifoin_data[FIFO_SIZE];
	int fifoin_read_request;
	int fifoout_rpos;
	int fifoout_wpos;
	UINT8 fifoout_data[FIFO_SIZE];
	int fifoout_read_request;
	UINT8 sb_coin_latch;
	UINT8 ejsakura_input_port;
	tilemap_t *text_layer;
	tilemap_t *back_layer;
	tilemap_t *mid_layer;
	tilemap_t *fore_layer;
	UINT32 layer_bank;
	UINT32 layer_enable;
	UINT32 video_dma_length;
	UINT32 video_dma_address;
	UINT32 sprite_dma_length;
	int rf2_layer_bank[3];
	UINT32 *tilemap_ram;
	UINT32 *palette_ram;
	UINT32 *sprite_ram;
	int mid_layer_offset;
	int fore_layer_offset;
	int text_layer_offset;
	UINT32 bg_fore_layer_position;
	UINT8 alpha_table[8192];
	UINT8 sprite_bpp;
};


/*----------- defined in machine/spisprit.c -----------*/

void seibuspi_sprite_decrypt(UINT8 *src, int romsize);


/*----------- defined in video/seibuspi.c -----------*/

VIDEO_START( spi );
SCREEN_UPDATE( spi );

VIDEO_START( sys386f2 );
SCREEN_UPDATE( sys386f2 );

READ32_HANDLER( spi_layer_bank_r );
WRITE32_HANDLER( spi_layer_bank_w );
WRITE32_HANDLER( spi_layer_enable_w );

void rf2_set_layer_banks(running_machine &machine, int banks);

WRITE32_HANDLER( tilemap_dma_start_w );
WRITE32_HANDLER( palette_dma_start_w );
WRITE32_HANDLER( video_dma_length_w );
WRITE32_HANDLER( video_dma_address_w );
WRITE32_HANDLER( sprite_dma_start_w );
