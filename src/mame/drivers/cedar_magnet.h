// license:BSD-3-Clause
// copyright-holders:David Haywood
/*


*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/bankdev.h"
#include "machine/z80ctc.h"
#include "sound/ay8910.h"

#include "cedar_magnet_sound.h"
#include "cedar_magnet_plane.h"
#include "cedar_magnet_sprite.h"
#include "cedar_magnet_flop.h"

#define LOG_IC49_PIO_PB 0
#define LOG_IC48_PIO_PB 0
#define LOG_IC48_PIO_PA 0

class cedar_magnet_state : public driver_device
{
public:
	cedar_magnet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bank0(*this, "bank0"),
		m_sub_ram_bankdev(*this, "mb_sub_ram"),
		m_sub_pal_bankdev(*this, "mb_sub_pal"),
		m_ram0(*this, "ram0"),
		m_pal_r(*this, "pal_r"),
		m_pal_g(*this, "pal_g"),
		m_pal_b(*this, "pal_b"),

		m_ic48_pio(*this, "z80pio_ic48"),
		m_ic49_pio(*this, "z80pio_ic49"),
		m_palette(*this, "palette"),
		m_maincpu(*this, "maincpu"),
		m_cedsound(*this, "cedtop"),
		m_cedplane0(*this, "cedplane0"),
		m_cedplane1(*this, "cedplane1"),
		m_cedsprite(*this, "cedsprite")
		{
			m_ic48_pio_pa_val = 0xff;
			m_ic48_pio_pb_val = 0xff;
			m_ic49_pio_pb_val = 0xff;
			m_prothack = nullptr;
		}

	required_device<address_map_bank_device> m_bank0;
	required_device<address_map_bank_device> m_sub_ram_bankdev;
	required_device<address_map_bank_device> m_sub_pal_bankdev;

	required_shared_ptr<uint8_t> m_ram0;
	required_shared_ptr<uint8_t> m_pal_r;
	required_shared_ptr<uint8_t> m_pal_g;
	required_shared_ptr<uint8_t> m_pal_b;

	required_device<z80pio_device> m_ic48_pio;
	required_device<z80pio_device> m_ic49_pio;

	uint8_t ic48_pio_pa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ic48_pio_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t ic48_pio_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ic48_pio_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t ic49_pio_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ic49_pio_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// 1x range ports
	void port18_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port19_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port1b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t port18_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t port19_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t port1a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// 7x range ports
	void rambank_palbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palupload_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void paladdr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t watchdog_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t port7c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// other ports
	void soundlatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t portff_data;

	uint8_t other_cpu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void other_cpu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t m_paladdr;
	int m_palbank;

	uint8_t m_ic48_pio_pa_val;
	uint8_t m_ic48_pio_pb_val;
	uint8_t m_ic49_pio_pb_val;

	void set_palette(int offset);
	void palette_r_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palette_g_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palette_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void handle_sub_board_cpu_lines(cedar_magnet_board_device* dev, int old_data, int data);
	void irq(device_t &device);
	void (*m_prothack)(cedar_magnet_state*);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_cedar_magnet(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<palette_device> m_palette;
	required_device<cpu_device> m_maincpu;

	required_device<cedar_magnet_sound_device> m_cedsound;
	required_device<cedar_magnet_plane_device> m_cedplane0;
	required_device<cedar_magnet_plane_device> m_cedplane1;
	required_device<cedar_magnet_sprite_device> m_cedsprite;

	void init_mag_time();
	void init_mag_xain();
	void init_mag_exzi();

};


