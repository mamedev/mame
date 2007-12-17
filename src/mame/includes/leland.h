/*************************************************************************

    Cinemat/Leland driver

*************************************************************************/

#include "sound/custom.h"

#define LELAND_BATTERY_RAM_SIZE 0x4000
#define ATAXX_EXTRA_TRAM_SIZE 0x800


/*----------- defined in machine/leland.c -----------*/

#define SERIAL_TYPE_NONE		0
#define SERIAL_TYPE_ADD			1
#define SERIAL_TYPE_ADD_XOR		2
#define SERIAL_TYPE_ENCRYPT		3
#define SERIAL_TYPE_ENCRYPT_XOR	4

extern UINT8 leland_dac_control;
extern void (*leland_update_master_bank)(void);

READ8_HANDLER( cerberus_dial_1_r );
READ8_HANDLER( cerberus_dial_2_r );

extern UINT8 *alleymas_kludge_mem;
WRITE8_HANDLER( alleymas_joystick_kludge );

READ8_HANDLER( dangerz_input_y_r );
READ8_HANDLER( dangerz_input_x_r );
READ8_HANDLER( dangerz_input_upper_r );

READ8_HANDLER( redline_pedal_1_r );
READ8_HANDLER( redline_pedal_2_r );
READ8_HANDLER( redline_wheel_1_r );
READ8_HANDLER( redline_wheel_2_r );

READ8_HANDLER( offroad_wheel_1_r );
READ8_HANDLER( offroad_wheel_2_r );
READ8_HANDLER( offroad_wheel_3_r );

READ8_HANDLER( ataxx_trackball_r );

READ8_HANDLER( indyheat_wheel_r );
READ8_HANDLER( indyheat_analog_r );
WRITE8_HANDLER( indyheat_analog_w );

MACHINE_START( leland );
MACHINE_RESET( leland );
MACHINE_START( ataxx );
MACHINE_RESET( ataxx );

INTERRUPT_GEN( leland_master_interrupt );

WRITE8_HANDLER( leland_master_alt_bankswitch_w );
void cerberus_bankswitch(void);
void mayhem_bankswitch(void);
void dangerz_bankswitch(void);
void basebal2_bankswitch(void);
void redline_bankswitch(void);
void viper_bankswitch(void);
void offroad_bankswitch(void);
void ataxx_bankswitch(void);

void leland_init_eeprom(UINT8 default_val, const UINT16 *data, UINT8 serial_offset, UINT8 serial_type);
void ataxx_init_eeprom(UINT8 default_val, const UINT16 *data, UINT8 serial_offset);

READ8_HANDLER( ataxx_eeprom_r );
WRITE8_HANDLER( ataxx_eeprom_w );

WRITE8_HANDLER( leland_battery_ram_w );
WRITE8_HANDLER( ataxx_battery_ram_w );
NVRAM_HANDLER( leland );
NVRAM_HANDLER( ataxx );

READ8_HANDLER( leland_master_analog_key_r );
WRITE8_HANDLER( leland_master_analog_key_w );

READ8_HANDLER( leland_master_input_r );
WRITE8_HANDLER( leland_master_output_w );
READ8_HANDLER( ataxx_master_input_r );
WRITE8_HANDLER( ataxx_master_output_w );

WRITE8_HANDLER( leland_gated_paletteram_w );
READ8_HANDLER( leland_gated_paletteram_r );
WRITE8_HANDLER( ataxx_paletteram_and_misc_w );
READ8_HANDLER( ataxx_paletteram_and_misc_r );

READ8_HANDLER( leland_sound_port_r );
WRITE8_HANDLER( leland_sound_port_w );

WRITE8_HANDLER( leland_slave_small_banksw_w );
WRITE8_HANDLER( leland_slave_large_banksw_w );
WRITE8_HANDLER( ataxx_slave_banksw_w );

READ8_HANDLER( leland_raster_r );

void leland_rotate_memory(int cpunum);


/*----------- defined in audio/leland.c -----------*/

void *leland_sh_start(int clock, const struct CustomSound_interface *config);

void *leland_80186_sh_start(int clock, const struct CustomSound_interface *config);
void *redline_80186_sh_start(int clock, const struct CustomSound_interface *config);
void leland_dac_update(int dacnum, UINT8 sample);

void leland_80186_sound_init(void);

READ8_HANDLER( leland_80186_response_r );

WRITE8_HANDLER( leland_80186_control_w );
WRITE8_HANDLER( leland_80186_command_lo_w );
WRITE8_HANDLER( leland_80186_command_hi_w );
WRITE8_HANDLER( ataxx_80186_control_w );

ADDRESS_MAP_EXTERN(leland_80186_map_program);
ADDRESS_MAP_EXTERN(leland_80186_map_io);
ADDRESS_MAP_EXTERN(redline_80186_map_io);
ADDRESS_MAP_EXTERN(ataxx_80186_map_io);


/*----------- defined in video/leland.c -----------*/

extern UINT8 *ataxx_qram;
extern UINT8 leland_last_scanline_int;

VIDEO_START( leland );
VIDEO_START( ataxx );

WRITE8_HANDLER( leland_gfx_port_w );

WRITE8_HANDLER( leland_master_video_addr_w );
WRITE8_HANDLER( leland_mvram_port_w );
READ8_HANDLER( leland_mvram_port_r );

WRITE8_HANDLER( leland_slave_video_addr_w );
WRITE8_HANDLER( leland_svram_port_w );
READ8_HANDLER( leland_svram_port_r );

WRITE8_HANDLER( ataxx_mvram_port_w );
WRITE8_HANDLER( ataxx_svram_port_w );
READ8_HANDLER( ataxx_mvram_port_r );
READ8_HANDLER( ataxx_svram_port_r );

VIDEO_EOF( leland );
VIDEO_UPDATE( leland );
VIDEO_UPDATE( ataxx );
