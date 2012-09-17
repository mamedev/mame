/*************************************************************************

    Cinemat/Leland driver

*************************************************************************/

#include "devlegcy.h"

#define LELAND_BATTERY_RAM_SIZE 0x4000
#define ATAXX_EXTRA_TRAM_SIZE 0x800


struct vram_state_data
{
	UINT16	m_addr;
	UINT8	m_latch[2];
};



class leland_state : public driver_device
{
public:
	leland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_dac_control;
	UINT8 *m_alleymas_kludge_mem;
	UINT8 *m_ataxx_qram;
	UINT8 m_gfx_control;
	UINT8 m_wcol_enable;
	emu_timer *m_master_int_timer;
	UINT8 *m_master_base;
	UINT8 *m_slave_base;
	UINT8 *m_xrom_base;
	UINT32 m_master_length;
	UINT32 m_slave_length;
	UINT32 m_xrom_length;
	int m_dangerz_x;
	int m_dangerz_y;
	UINT8 m_analog_result;
	UINT8 m_dial_last_input[4];
	UINT8 m_dial_last_result[4];
	UINT8 m_keycard_shift;
	UINT8 m_keycard_bit;
	UINT8 m_keycard_state;
	UINT8 m_keycard_clock;
	UINT8 m_keycard_command[3];
	UINT8 m_top_board_bank;
	UINT8 m_sound_port_bank;
	UINT8 m_alternate_bank;
	UINT8 m_master_bank;
	void (*m_update_master_bank)(running_machine &machine);
	UINT32 m_xrom1_addr;
	UINT32 m_xrom2_addr;
	UINT8 m_battery_ram_enable;
	UINT8 *m_battery_ram;
	UINT8 *m_extra_tram;
	UINT8 *m_video_ram;
	struct vram_state_data m_vram_state[2];
	UINT16 m_xscroll;
	UINT16 m_yscroll;
	UINT8 m_gfxbank;
	UINT16 m_last_scanline;
	emu_timer *m_scanline_timer;
	DECLARE_READ8_MEMBER(cerberus_dial_1_r);
	DECLARE_READ8_MEMBER(cerberus_dial_2_r);
	DECLARE_WRITE8_MEMBER(alleymas_joystick_kludge);
	DECLARE_READ8_MEMBER(dangerz_input_y_r);
	DECLARE_READ8_MEMBER(dangerz_input_x_r);
	DECLARE_READ8_MEMBER(dangerz_input_upper_r);
	DECLARE_READ8_MEMBER(redline_pedal_1_r);
	DECLARE_READ8_MEMBER(redline_pedal_2_r);
	DECLARE_READ8_MEMBER(redline_wheel_1_r);
	DECLARE_READ8_MEMBER(redline_wheel_2_r);
	DECLARE_READ8_MEMBER(offroad_wheel_1_r);
	DECLARE_READ8_MEMBER(offroad_wheel_2_r);
	DECLARE_READ8_MEMBER(offroad_wheel_3_r);
	DECLARE_READ8_MEMBER(ataxx_trackball_r);
	DECLARE_READ8_MEMBER(indyheat_wheel_r);
	DECLARE_READ8_MEMBER(indyheat_analog_r);
	DECLARE_WRITE8_MEMBER(indyheat_analog_w);
	DECLARE_WRITE8_MEMBER(leland_master_alt_bankswitch_w);
	DECLARE_WRITE8_MEMBER(leland_battery_ram_w);
	DECLARE_WRITE8_MEMBER(ataxx_battery_ram_w);
	DECLARE_READ8_MEMBER(leland_master_analog_key_r);
	DECLARE_WRITE8_MEMBER(leland_master_analog_key_w);
	DECLARE_READ8_MEMBER(leland_master_input_r);
	DECLARE_WRITE8_MEMBER(leland_master_output_w);
	DECLARE_READ8_MEMBER(ataxx_master_input_r);
	DECLARE_WRITE8_MEMBER(ataxx_master_output_w);
	DECLARE_WRITE8_MEMBER(leland_gated_paletteram_w);
	DECLARE_READ8_MEMBER(leland_gated_paletteram_r);
	DECLARE_WRITE8_MEMBER(ataxx_paletteram_and_misc_w);
	DECLARE_READ8_MEMBER(ataxx_paletteram_and_misc_r);
	DECLARE_WRITE8_MEMBER(leland_slave_small_banksw_w);
	DECLARE_WRITE8_MEMBER(leland_slave_large_banksw_w);
	DECLARE_WRITE8_MEMBER(ataxx_slave_banksw_w);
	DECLARE_READ8_MEMBER(leland_raster_r);
	DECLARE_WRITE8_MEMBER(leland_scroll_w);
	DECLARE_WRITE8_MEMBER(leland_master_video_addr_w);
	DECLARE_WRITE8_MEMBER(leland_mvram_port_w);
	DECLARE_READ8_MEMBER(leland_mvram_port_r);
	DECLARE_WRITE8_MEMBER(leland_slave_video_addr_w);
	DECLARE_WRITE8_MEMBER(leland_svram_port_w);
	DECLARE_READ8_MEMBER(leland_svram_port_r);
	DECLARE_WRITE8_MEMBER(ataxx_mvram_port_w);
	DECLARE_WRITE8_MEMBER(ataxx_svram_port_w);
	DECLARE_READ8_MEMBER(ataxx_mvram_port_r);
	DECLARE_READ8_MEMBER(ataxx_svram_port_r);
	DECLARE_DRIVER_INIT(dblplay);
	DECLARE_DRIVER_INIT(viper);
	DECLARE_DRIVER_INIT(quarterb);
	DECLARE_DRIVER_INIT(aafb);
	DECLARE_DRIVER_INIT(redlin2p);
	DECLARE_DRIVER_INIT(aafbb);
	DECLARE_DRIVER_INIT(dangerz);
	DECLARE_DRIVER_INIT(mayhem);
	DECLARE_DRIVER_INIT(offroad);
	DECLARE_DRIVER_INIT(pigout);
	DECLARE_DRIVER_INIT(alleymas);
	DECLARE_DRIVER_INIT(offroadt);
	DECLARE_DRIVER_INIT(teamqb);
	DECLARE_DRIVER_INIT(strkzone);
	DECLARE_DRIVER_INIT(wseries);
	DECLARE_DRIVER_INIT(powrplay);
	DECLARE_DRIVER_INIT(basebal2);
	DECLARE_DRIVER_INIT(upyoural);
	DECLARE_DRIVER_INIT(cerberus);
	DECLARE_DRIVER_INIT(aafbd2p);
	DECLARE_DRIVER_INIT(ataxx);
	DECLARE_DRIVER_INIT(ataxxj);
	DECLARE_DRIVER_INIT(wsf);
	DECLARE_DRIVER_INIT(indyheat);
	DECLARE_DRIVER_INIT(brutforc);
	DECLARE_DRIVER_INIT(asylum);
	DECLARE_MACHINE_START(ataxx);
	DECLARE_MACHINE_RESET(ataxx);
	DECLARE_MACHINE_START(leland);
	DECLARE_MACHINE_RESET(leland);
	DECLARE_VIDEO_START(leland);
	DECLARE_VIDEO_START(ataxx);
};


/*----------- defined in machine/leland.c -----------*/

#define SERIAL_TYPE_NONE		0
#define SERIAL_TYPE_ADD			1
#define SERIAL_TYPE_ADD_XOR		2
#define SERIAL_TYPE_ENCRYPT		3
#define SERIAL_TYPE_ENCRYPT_XOR	4













INTERRUPT_GEN( leland_master_interrupt );

void cerberus_bankswitch(running_machine &machine);
void mayhem_bankswitch(running_machine &machine);
void dangerz_bankswitch(running_machine &machine);
void basebal2_bankswitch(running_machine &machine);
void redline_bankswitch(running_machine &machine);
void viper_bankswitch(running_machine &machine);
void offroad_bankswitch(running_machine &machine);
void ataxx_bankswitch(running_machine &machine);

void leland_init_eeprom(running_machine &machine, UINT8 default_val, const UINT16 *data, UINT8 serial_offset, UINT8 serial_type);
void ataxx_init_eeprom(running_machine &machine, const UINT16 *data);

DECLARE_READ8_DEVICE_HANDLER( ataxx_eeprom_r );
DECLARE_WRITE8_DEVICE_HANDLER( ataxx_eeprom_w );





DECLARE_READ8_DEVICE_HANDLER( leland_sound_port_r );
DECLARE_WRITE8_DEVICE_HANDLER( leland_sound_port_w );



void leland_rotate_memory(running_machine &machine, const char *cpuname);


/*----------- defined in audio/leland.c -----------*/

class leland_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	leland_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	leland_sound_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~leland_sound_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type LELAND;

class leland_80186_sound_device : public leland_sound_device
{
public:
	leland_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	leland_80186_sound_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
};

extern const device_type LELAND_80186;

class redline_80186_sound_device : public leland_80186_sound_device
{
public:
	redline_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
};

extern const device_type REDLINE_80186;


void leland_dac_update(device_t *device, int dacnum, UINT8 sample);

DECLARE_READ8_DEVICE_HANDLER( leland_80186_response_r );

DECLARE_WRITE8_DEVICE_HANDLER( leland_80186_control_w );
DECLARE_WRITE8_DEVICE_HANDLER( leland_80186_command_lo_w );
DECLARE_WRITE8_DEVICE_HANDLER( leland_80186_command_hi_w );
DECLARE_WRITE8_DEVICE_HANDLER( ataxx_80186_control_w );

ADDRESS_MAP_EXTERN(leland_80186_map_program, 16);
ADDRESS_MAP_EXTERN(leland_80186_map_io, 16);
ADDRESS_MAP_EXTERN(redline_80186_map_io, 16);
ADDRESS_MAP_EXTERN(ataxx_80186_map_io, 16);


/*----------- defined in video/leland.c -----------*/

DECLARE_WRITE8_DEVICE_HANDLER( leland_gfx_port_w );




MACHINE_CONFIG_EXTERN( leland_video );
MACHINE_CONFIG_EXTERN( ataxx_video );
