#define MCU_BUFFER_MAX 6

class renegade_state : public driver_device
{
public:
	renegade_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 bank;
	int mcu_sim;
	int from_main;
	int from_mcu;
	int main_sent;
	int mcu_sent;
	UINT8 ddr_a;
	UINT8 ddr_b;
	UINT8 ddr_c;
	UINT8 port_a_out;
	UINT8 port_b_out;
	UINT8 port_c_out;
	UINT8 port_a_in;
	UINT8 port_b_in;
	UINT8 port_c_in;
	UINT8 mcu_buffer[MCU_BUFFER_MAX];
	UINT8 mcu_input_size;
	UINT8 mcu_output_byte;
	INT8 mcu_key;
	int mcu_checksum;
	const UINT8 *mcu_encrypt_table;
	int mcu_encrypt_table_len;
	int coin;
	UINT8 *videoram;
	UINT8 *videoram2;
	INT32 scrollx;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
};


/*----------- defined in video/renegade.c -----------*/

SCREEN_UPDATE( renegade );
VIDEO_START( renegade );
WRITE8_HANDLER( renegade_scroll0_w );
WRITE8_HANDLER( renegade_scroll1_w );
WRITE8_HANDLER( renegade_videoram_w );
WRITE8_HANDLER( renegade_videoram2_w );
WRITE8_HANDLER( renegade_flipscreen_w );
