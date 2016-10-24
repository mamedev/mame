// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 * includes/gb.h
 *
 ****************************************************************************/

#ifndef GB_H_
#define GB_H_

#include "sound/gb.h"
#include "cpu/lr35902/lr35902.h"
#include "bus/gameboy/gb_slot.h"
#include "machine/ram.h"
#include "video/gb_lcd.h"


class gb_state : public driver_device
{
public:
	gb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cartslot(*this, "gbslot"),
		m_maincpu(*this, "maincpu"),
		m_apu(*this, "apu"),
		m_region_maincpu(*this, "maincpu"),
		m_rambank(*this, "cgb_ram"),
		m_inputs(*this, "INPUTS"),
		m_bios_hack(*this, "SKIP_CHECK"),
		m_ram(*this, RAM_TAG),
		m_ppu(*this, "ppu") { }

	uint8_t       m_gb_io[0x10];

	/* Timer related */
	uint16_t      m_divcount;
	uint8_t       m_shift;
	uint16_t      m_shift_cycles;
	uint8_t       m_triggering_irq;
	uint8_t       m_reloading;

	/* Serial I/O related */
	uint16_t      m_internal_serial_clock;
	uint16_t      m_internal_serial_frequency;
	uint32_t      m_sio_count;             /* Serial I/O counter */

	/* SGB variables */
	int8_t m_sgb_packets;
	uint8_t m_sgb_bitcount;
	uint8_t m_sgb_bytecount;
	uint8_t m_sgb_start;
	uint8_t m_sgb_rest;
	uint8_t m_sgb_controller_no;
	uint8_t m_sgb_controller_mode;
	uint8_t m_sgb_data[0x100];

	/* CGB variables */
	uint8_t       *m_gbc_rammap[8];           /* (CGB) Addresses of internal RAM banks */
	uint8_t       m_gbc_rambank;          /* (CGB) Current CGB RAM bank */

	bool m_bios_disable;

	void gb_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gb_io2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sgb_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gb_ie_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gb_ie_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gb_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gbc_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gbc_io2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gbc_io2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void palette_init_gb(palette_device &palette);
	void machine_start_sgb();
	void machine_reset_sgb();
	void palette_init_sgb(palette_device &palette);
	void palette_init_gbp(palette_device &palette);
	void machine_start_gbc();
	void machine_reset_gbc();
	void palette_init_gbc(palette_device &palette);
	void gb_timer_callback(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t gb_cart_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t gbc_cart_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gb_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gb_ram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gb_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gb_echo_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gb_echo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	optional_device<gb_cart_slot_device> m_cartslot;

protected:
	enum {
		SIO_ENABLED = 0x80,
		SIO_FAST_CLOCK = 0x02,
		SIO_INTERNAL_CLOCK = 0x01
	};

	required_device<lr35902_cpu_device> m_maincpu;
	required_device<gameboy_sound_device> m_apu;
	required_memory_region m_region_maincpu;
	optional_memory_bank m_rambank;   // cgb
	required_ioport m_inputs;
	required_ioport m_bios_hack;
	optional_device<ram_device> m_ram;
	required_device<dmg_ppu_device> m_ppu;

	void gb_timer_increment();
	void gb_timer_check_irq();
	void gb_init();
	void gb_init_regs();
	void gb_serial_timer_tick();

	void save_gb_base();
	void save_gbc_only();
	void save_sgb_only();

	virtual void machine_start() override;
	virtual void machine_reset() override;
};


class megaduck_state : public gb_state
{
public:
	megaduck_state(const machine_config &mconfig, device_type type, const char *tag)
		: gb_state(mconfig, type, tag)
		, m_cartslot(*this, "duckslot")
	{ }

	uint8_t megaduck_video_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void megaduck_video_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void megaduck_sound_w1(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t megaduck_sound_r1(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void megaduck_sound_w2(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t megaduck_sound_r2(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void machine_start_megaduck();
	void machine_reset_megaduck();
	void palette_init_megaduck(palette_device &palette);

	uint8_t cart_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bank1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bank2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	optional_device<megaduck_cart_slot_device> m_cartslot;
};



#endif /* GB_H_ */
