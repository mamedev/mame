/*************************************************************************

    Cinemat/Leland driver

*************************************************************************/

#include "devlegcy.h"

#define LELAND_BATTERY_RAM_SIZE 0x4000
#define ATAXX_EXTRA_TRAM_SIZE 0x800


/*----------- defined in machine/leland.c -----------*/

#define SERIAL_TYPE_NONE		0
#define SERIAL_TYPE_ADD			1
#define SERIAL_TYPE_ADD_XOR		2
#define SERIAL_TYPE_ENCRYPT		3
#define SERIAL_TYPE_ENCRYPT_XOR	4

extern UINT8 leland_dac_control;
extern void (*leland_update_master_bank)(running_machine *machine);

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
void cerberus_bankswitch(running_machine *machine);
void mayhem_bankswitch(running_machine *machine);
void dangerz_bankswitch(running_machine *machine);
void basebal2_bankswitch(running_machine *machine);
void redline_bankswitch(running_machine *machine);
void viper_bankswitch(running_machine *machine);
void offroad_bankswitch(running_machine *machine);
void ataxx_bankswitch(running_machine *machine);

void leland_init_eeprom(running_machine *machine, UINT8 default_val, const UINT16 *data, UINT8 serial_offset, UINT8 serial_type);
void ataxx_init_eeprom(running_machine *machine, const UINT16 *data);

READ8_DEVICE_HANDLER( ataxx_eeprom_r );
WRITE8_DEVICE_HANDLER( ataxx_eeprom_w );

WRITE8_HANDLER( leland_battery_ram_w );
WRITE8_HANDLER( ataxx_battery_ram_w );

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

READ8_DEVICE_HANDLER( leland_sound_port_r );
WRITE8_DEVICE_HANDLER( leland_sound_port_w );

WRITE8_HANDLER( leland_slave_small_banksw_w );
WRITE8_HANDLER( leland_slave_large_banksw_w );
WRITE8_HANDLER( ataxx_slave_banksw_w );

READ8_HANDLER( leland_raster_r );

void leland_rotate_memory(running_machine *machine, const char *cpuname);


/*----------- defined in audio/leland.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(LELAND, leland_sound);
DECLARE_LEGACY_SOUND_DEVICE(LELAND_80186, leland_80186_sound);
DECLARE_LEGACY_SOUND_DEVICE(REDLINE_80186, redline_80186_sound);

void leland_dac_update(device_t *device, int dacnum, UINT8 sample);

READ8_DEVICE_HANDLER( leland_80186_response_r );

WRITE8_DEVICE_HANDLER( leland_80186_control_w );
WRITE8_DEVICE_HANDLER( leland_80186_command_lo_w );
WRITE8_DEVICE_HANDLER( leland_80186_command_hi_w );
WRITE8_DEVICE_HANDLER( ataxx_80186_control_w );

ADDRESS_MAP_EXTERN(leland_80186_map_program, 16);
ADDRESS_MAP_EXTERN(leland_80186_map_io, 16);
ADDRESS_MAP_EXTERN(redline_80186_map_io, 16);
ADDRESS_MAP_EXTERN(ataxx_80186_map_io, 16);


/*----------- defined in video/leland.c -----------*/

extern UINT8 *ataxx_qram;

WRITE8_HANDLER( leland_scroll_w );
WRITE8_DEVICE_HANDLER( leland_gfx_port_w );

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

MACHINE_CONFIG_EXTERN( leland_video );
MACHINE_CONFIG_EXTERN( ataxx_video );
