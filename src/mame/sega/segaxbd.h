// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega X-Board hardware

***************************************************************************/
#ifndef MAME_SEGA_SEGAXBD_H
#define MAME_SEGA_SEGAXBD_H

#pragma once

#include "segaic16.h"
#include "segaic16_road.h"
#include "sega16sp.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/cxd1095.h"
#include "machine/i8251.h"
#include "machine/mb3773.h"
#include "machine/mb8421.h"
#include "segaic16_m.h"
#include "video/resnet.h"
#include "emupal.h"
#include "screen.h"

// ======================> segaxbd_state


class segaxbd_state : public device_t
{
public:
	// construction/destruction
	segaxbd_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void xboard_base_mconfig(machine_config &config);

	void install_aburner2(void);
	void install_loffire(void);
	void install_smgp(void);
	void install_gprider(void);

	// devices
	required_device<m68000_device> m_maincpu;

	// custom I/O
	uint8_t aburner2_motor_r();
	void aburner2_motor_w(uint8_t data);
	uint8_t smgp_motor_r();
	void smgp_motor_w(uint8_t data);
	uint8_t lastsurv_port_r();
	void lastsurv_muxer_w(uint8_t data);

	// game-specific main CPU read/write handlers
	void loffire_sync0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t smgp_excs_r(offs_t offset);
	void smgp_excs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// main CPU read/write handlers
	uint8_t analog_r();
	void iocontrol_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// video updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// palette helpers
	void paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void smgp_airdrive_map(address_map &map) ATTR_COLD;
	void smgp_airdrive_portmap(address_map &map) ATTR_COLD;
	void smgp_comm_map(address_map &map) ATTR_COLD;
	void smgp_comm_portmap(address_map &map) ATTR_COLD;
	void smgp_sound2_map(address_map &map) ATTR_COLD;
	void smgp_sound2_portmap(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	segaxbd_state(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device overrides
//  virtual void machine_reset();
	virtual void video_start();

	TIMER_CALLBACK_MEMBER(scanline_tick);

	// internal helpers
	void update_main_irqs();
	void m68k_reset_callback(int state);
	void generic_iochip0_lamps_w(uint8_t data);

	// compare/timer chip callbacks
	void timer_irq_w(int state);

	void pc_0_w(uint8_t data);
	void pd_0_w(uint8_t data);

	// devices
	required_device<m68000_device> m_subcpu;
	optional_device<z80_device> m_soundcpu;
	optional_device<z80_device> m_soundcpu2;
	optional_device<i8751_device> m_mcu;
	required_device<mb3773_device> m_watchdog;
	required_device_array<cxd1095_device, 2> m_iochip;
	required_device<sega_315_5250_compare_timer_device> m_cmptimer_1;
	required_device<sega_xboard_sprite_device> m_sprites;
	required_device<segaic16_video_device> m_segaic16vid;
	required_device<segaic16_road_device> m_segaic16road;
	required_shared_ptr<uint16_t> m_subram0;

	// configuration
	bool            m_adc_reverse[8]{};
	uint8_t           m_road_priority = 0;

	// internal state
	emu_timer *     m_scanline_timer = nullptr;
	uint8_t           m_timer_irq_state = 0;
	uint8_t           m_vblank_irq_state = 0;
	uint8_t           m_pc_0 = 0;

	// game-specific state
	uint16_t *        m_loffire_sync = 0;
	uint8_t           m_lastsurv_mux = 0;

	// memory pointers
	required_shared_ptr<uint16_t> m_paletteram;
	bool            m_gprider_hack = false;

	void palette_init();
	uint32_t      m_palette_entries = 0;          // number of palette entries
	uint8_t       m_palette_normal[32]{};       // RGB translations for normal pixels
	uint8_t       m_palette_shadow[32]{};       // RGB translations for shadowed pixels
	uint8_t       m_palette_hilight[32]{};      // RGB translations for hilighted pixels
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_ioport m_io0_porta;
	optional_ioport_array<8> m_adc_ports;
	optional_ioport_array<4> m_mux_ports;
	output_finder<4> m_lamps;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


class segaxbd_regular_state :  public segaxbd_state
{
public:
	segaxbd_regular_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};



class segaxbd_fd1094_state :  public segaxbd_state
{
public:
	segaxbd_fd1094_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class segaxbd_aburner2_state :  public segaxbd_state
{
public:
	segaxbd_aburner2_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
//  virtual void device_start();
//  virtual void device_reset();
};

class segaxbd_lastsurv_fd1094_state :  public segaxbd_state
{
public:
	segaxbd_lastsurv_fd1094_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class segaxbd_lastsurv_state :  public segaxbd_state
{
public:
	segaxbd_lastsurv_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


class segaxbd_smgp_fd1094_state :  public segaxbd_state
{
public:
	segaxbd_smgp_fd1094_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


class segaxbd_smgp_state :  public segaxbd_state
{
public:
	segaxbd_smgp_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


class segaxbd_rascot_state :  public segaxbd_state
{
public:
	segaxbd_rascot_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t commram_r(offs_t offset);
	void commram_w(offs_t offset, uint8_t data);
	void commram_bank_w(uint8_t data);

	void sub_map(address_map &map) ATTR_COLD;
	void comm_map(address_map &map) ATTR_COLD;

	required_device<mb8421_device> m_commram;
	required_device<i8251_device> m_usart;

	uint8_t m_commram_bank = 0;
};

#endif // MAME_SEGA_SEGAXBD_H
