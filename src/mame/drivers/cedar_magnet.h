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

	DECLARE_READ8_MEMBER(ic48_pio_pa_r);
	DECLARE_WRITE8_MEMBER(ic48_pio_pa_w);

	DECLARE_READ8_MEMBER(ic48_pio_pb_r);
	DECLARE_WRITE8_MEMBER(ic48_pio_pb_w);

	DECLARE_READ8_MEMBER(ic49_pio_pb_r);
	DECLARE_WRITE8_MEMBER(ic49_pio_pb_w);

	// 1x range ports
	DECLARE_WRITE8_MEMBER(port18_w);
	DECLARE_WRITE8_MEMBER(port19_w);
	DECLARE_WRITE8_MEMBER(port1b_w);

	DECLARE_READ8_MEMBER(port18_r);
	DECLARE_READ8_MEMBER(port19_r);
	DECLARE_READ8_MEMBER(port1a_r);

	// 7x range ports
	DECLARE_WRITE8_MEMBER(rambank_palbank_w);
	DECLARE_WRITE8_MEMBER(palupload_w);
	DECLARE_WRITE8_MEMBER(paladdr_w);
	DECLARE_READ8_MEMBER(watchdog_r);
	DECLARE_READ8_MEMBER(port7c_r);

	// other ports
	DECLARE_WRITE8_MEMBER(soundlatch_w);
	uint8_t portff_data;

	DECLARE_READ8_MEMBER(other_cpu_r);
	DECLARE_WRITE8_MEMBER(other_cpu_w);

	uint8_t m_paladdr;
	int m_palbank;

	uint8_t m_ic48_pio_pa_val;
	uint8_t m_ic48_pio_pb_val;
	uint8_t m_ic49_pio_pb_val;

	void set_palette(int offset);
	DECLARE_WRITE8_MEMBER(palette_r_w);
	DECLARE_WRITE8_MEMBER(palette_g_w);
	DECLARE_WRITE8_MEMBER(palette_b_w);

	void handle_sub_board_cpu_lines(cedar_magnet_board_device* dev, int old_data, int data);
	INTERRUPT_GEN_MEMBER(irq);
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

	DECLARE_DRIVER_INIT(mag_time);
	DECLARE_DRIVER_INIT(mag_xain);
	DECLARE_DRIVER_INIT(mag_exzi);

};


