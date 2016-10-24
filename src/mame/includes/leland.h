// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Cinemat/Leland driver

*************************************************************************/

#include "machine/eepromser.h"
#include "sound/ym2151.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
#include "machine/pit8253.h"
#include "cpu/i86/i186.h"

#define LELAND_BATTERY_RAM_SIZE 0x4000
#define ATAXX_EXTRA_TRAM_SIZE 0x800


struct vram_state_data
{
	uint16_t  m_addr;
	uint8_t   m_latch[2];
};

class leland_80186_sound_device;

class leland_state : public driver_device
{
public:
	leland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_master(*this, "master"),
		m_slave(*this, "slave"),
		m_mainram(*this, "mainram"),
		m_eeprom(*this, "eeprom"),
		m_sound(*this, "custom"),
		m_dac0(*this, "dac0"),
		m_dac1(*this, "dac1"),
		m_ay8910(*this, "ay8910"),
		m_ay8912(*this, "ay8912"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")  { }

	required_device<cpu_device> m_master;
	required_device<cpu_device> m_slave;
	required_shared_ptr<uint8_t> m_mainram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<leland_80186_sound_device> m_sound;
	optional_device<dac_byte_interface> m_dac0;
	optional_device<dac_byte_interface> m_dac1;
	optional_device<ay8910_device> m_ay8910;
	optional_device<ay8912_device> m_ay8912;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint8_t m_dac_control;
	uint8_t *m_alleymas_kludge_mem;
	std::unique_ptr<uint8_t[]> m_ataxx_qram;
	uint8_t m_gfx_control;
	uint8_t m_wcol_enable;
	emu_timer *m_master_int_timer;
	uint8_t *m_master_base;
	uint8_t *m_slave_base;
	uint8_t *m_xrom_base;
	uint32_t m_master_length;
	uint32_t m_slave_length;
	uint32_t m_xrom_length;
	int m_dangerz_x;
	int m_dangerz_y;
	uint8_t m_analog_result;
	uint8_t m_dial_last_input[4];
	uint8_t m_dial_last_result[4];
	uint8_t m_keycard_shift;
	uint8_t m_keycard_bit;
	uint8_t m_keycard_state;
	uint8_t m_keycard_clock;
	uint8_t m_keycard_command[3];
	uint8_t m_top_board_bank;
	uint8_t m_sound_port_bank;
	uint8_t m_alternate_bank;
	uint8_t m_master_bank;
	void (leland_state::*m_update_master_bank)();
	uint32_t m_xrom1_addr;
	uint32_t m_xrom2_addr;
	uint8_t m_battery_ram_enable;
	uint8_t *m_battery_ram;
	std::unique_ptr<uint8_t[]> m_extra_tram;
	std::unique_ptr<uint8_t[]> m_video_ram;
	struct vram_state_data m_vram_state[2];
	uint16_t m_xscroll;
	uint16_t m_yscroll;
	uint8_t m_gfxbank;
	uint16_t m_last_scanline;
	emu_timer *m_scanline_timer;
	uint8_t cerberus_dial_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t cerberus_dial_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void alleymas_joystick_kludge(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dangerz_input_y_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dangerz_input_x_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dangerz_input_upper_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t redline_pedal_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t redline_pedal_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t redline_wheel_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t redline_wheel_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t offroad_wheel_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t offroad_wheel_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t offroad_wheel_3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ataxx_trackball_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t indyheat_wheel_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t indyheat_analog_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void indyheat_analog_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void leland_master_alt_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void leland_battery_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ataxx_battery_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t leland_master_analog_key_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void leland_master_analog_key_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t leland_master_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void leland_master_output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ataxx_master_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ataxx_master_output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void leland_gated_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t leland_gated_paletteram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ataxx_paletteram_and_misc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ataxx_paletteram_and_misc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void leland_slave_small_banksw_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void leland_slave_large_banksw_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ataxx_slave_banksw_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t leland_raster_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void leland_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void leland_master_video_addr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void leland_mvram_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t leland_mvram_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void leland_slave_video_addr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void leland_svram_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t leland_svram_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ataxx_mvram_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ataxx_svram_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ataxx_mvram_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ataxx_svram_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_dblplay();
	void init_viper();
	void init_quarterb();
	void init_aafb();
	void init_redlin2p();
	void init_aafbb();
	void init_dangerz();
	void init_mayhem();
	void init_offroad();
	void init_pigout();
	void init_alleymas();
	void init_offroadt();
	void init_teamqb();
	void init_strkzone();
	void init_wseries();
	void init_powrplay();
	void init_basebal2();
	void init_upyoural();
	void init_cerberus();
	void init_aafbd2p();
	void init_ataxx();
	void init_ataxxj();
	void init_wsf();
	void init_indyheat();
	void init_brutforc();
	void init_asylum();
	void machine_start_ataxx();
	void machine_reset_ataxx();
	void machine_start_leland();
	void machine_reset_leland();
	void video_start_leland();
	void video_start_leland2();
	void video_start_ataxx();
	uint32_t screen_update_leland(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ataxx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void leland_master_interrupt(device_t &device);
	void leland_interrupt_callback(void *ptr, int32_t param);
	void ataxx_interrupt_callback(void *ptr, int32_t param);
	void scanline_callback(void *ptr, int32_t param);
	void leland_delayed_mvram_w(void *ptr, int32_t param);
	uint8_t ataxx_eeprom_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ataxx_eeprom_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t leland_sound_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void leland_sound_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void leland_gfx_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void leland_video_addr_w(address_space &space, int offset, int data, int num);
	int leland_vram_port_r(address_space &space, int offset, int num);
	void leland_vram_port_w(address_space &space, int offset, int data, int num);
	int dial_compute_value(int new_val, int indx);
	void update_dangerz_xy();
	void cerberus_bankswitch();
	void mayhem_bankswitch();
	void dangerz_bankswitch();
	void basebal2_bankswitch();
	void redline_bankswitch();
	void viper_bankswitch();
	void offroad_bankswitch();
	void ataxx_bankswitch();
	void leland_init_eeprom(uint8_t default_val, const uint16_t *data, uint8_t serial_offset, uint8_t serial_type);
	void ataxx_init_eeprom(const uint16_t *data);
	int keycard_r();
	void keycard_w(int data);
	void leland_rotate_memory(const char *cpuname);
	void init_master_ports(uint8_t mvram_base, uint8_t io_base);
};


#define SERIAL_TYPE_NONE        0
#define SERIAL_TYPE_ADD         1
#define SERIAL_TYPE_ADD_XOR     2
#define SERIAL_TYPE_ENCRYPT     3
#define SERIAL_TYPE_ENCRYPT_XOR 4


/*----------- defined in audio/leland.c -----------*/

class leland_80186_sound_device : public device_t
{
public:
	leland_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	leland_80186_sound_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	virtual machine_config_constructor device_mconfig_additions() const override;

	void peripheral_ctrl(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void leland_80186_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ataxx_80186_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t peripheral_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void peripheral_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void leland_80186_command_lo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void leland_80186_command_hi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t leland_80186_response_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dac_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ataxx_dac_control(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pit0_2_w(int state);
	void pit1_0_w(int state);
	void pit1_1_w(int state);
	void pit1_2_w(int state);
	void i80186_tmr0_w(int state);
	void i80186_tmr1_w(int state);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	int m_type;

	enum {
		TYPE_LELAND,
		TYPE_REDLINE,
		TYPE_ATAXX,
		TYPE_WSF
	};

	required_device<dac_byte_interface> m_dac1;
	required_device<dac_byte_interface> m_dac2;
	required_device<dac_byte_interface> m_dac3;
	required_device<dac_byte_interface> m_dac4;
	optional_device<dac_byte_interface> m_dac5;
	optional_device<dac_byte_interface> m_dac6;
	optional_device<dac_byte_interface> m_dac7;
	optional_device<dac_byte_interface> m_dac8;
	optional_device<dac_word_interface> m_dac9;
	required_device<dac_byte_interface> m_dac1vol;
	required_device<dac_byte_interface> m_dac2vol;
	required_device<dac_byte_interface> m_dac3vol;
	required_device<dac_byte_interface> m_dac4vol;
	optional_device<dac_byte_interface> m_dac5vol;
	optional_device<dac_byte_interface> m_dac6vol;
	optional_device<dac_byte_interface> m_dac7vol;
	optional_device<dac_byte_interface> m_dac8vol;

private:
	void command_lo_sync(void *ptr, int param);
	void delayed_response_r(void *ptr, int param);
	void set_clock_line(int which, int state) { m_clock_active = state ? (m_clock_active | (1<<which)) : (m_clock_active & ~(1<<which)); }

	// internal state
	i80186_cpu_device *m_audiocpu;
	uint16_t m_peripheral;
	uint8_t m_last_control;
	uint8_t m_clock_active;
	uint8_t m_clock_tick;
	uint16_t m_sound_command;
	uint16_t m_sound_response;
	uint32_t m_ext_start;
	uint32_t m_ext_stop;
	uint8_t m_ext_active;
	uint8_t* m_ext_base;

	required_device<pit8254_device> m_pit0;
	optional_device<pit8254_device> m_pit1;
	optional_device<pit8254_device> m_pit2;
	optional_device<ym2151_device> m_ymsnd;
};

extern const device_type LELAND_80186;

class redline_80186_sound_device : public leland_80186_sound_device
{
public:
	redline_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void redline_dac_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	virtual machine_config_constructor device_mconfig_additions() const override;
};

extern const device_type REDLINE_80186;

class ataxx_80186_sound_device : public leland_80186_sound_device
{
public:
	ataxx_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual machine_config_constructor device_mconfig_additions() const override;
};

extern const device_type ATAXX_80186;

class wsf_80186_sound_device : public leland_80186_sound_device
{
public:
	wsf_80186_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual machine_config_constructor device_mconfig_additions() const override;
};

extern const device_type WSF_80186;

ADDRESS_MAP_EXTERN(leland_80186_map_program, 16);
ADDRESS_MAP_EXTERN(leland_80186_map_io, 16);
ADDRESS_MAP_EXTERN(redline_80186_map_io, 16);
ADDRESS_MAP_EXTERN(ataxx_80186_map_io, 16);


/*----------- defined in video/leland.c -----------*/

MACHINE_CONFIG_EXTERN( leland_video );
MACHINE_CONFIG_EXTERN( ataxx_video );
