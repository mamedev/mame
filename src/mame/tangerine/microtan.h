// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/******************************************************************************
 *  Microtan 65
 *
 *  Thanks go to Geoff Macdonald <mail@geoff.org.uk>
 *  for his site http://www.geoff.org.uk/microtan/index.htm
 *  and to Fabrice Frances <frances@ensica.fr>
 *  for his site http://oric.free.fr/microtan.html
 *
 ******************************************************************************/

#ifndef MAME_TANGERINE_MICROTAN_H
#define MAME_TANGERINE_MICROTAN_H

#pragma once

#include "cpu/m6502/m6502.h"
#include "cpu/m6809/m6809.h"
#include "machine/input_merger.h"
#include "bus/tanbus/tanbus.h"
#include "bus/tanbus/keyboard/keyboard.h"
#include "imagedev/snapquik.h"
#include "tilemap.h"

class microtan_state : public driver_device
{
public:
	microtan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_irq_line(*this, "irq_line")
		, m_keyboard(*this, "keyboard")
		, m_tanbus(*this, "tanbus")
		, m_videoram(*this, "videoram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_gfx1(*this, "gfx1")
	{ }

	void mt65(machine_config &config);
	void micron(machine_config &config);
	void spinveti(machine_config &config);

	void trigger_reset(int state);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_irq_line;
	required_device<microtan_kbd_slot_device> m_keyboard;
	optional_device<tanbus_device> m_tanbus;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_memory_region m_gfx1;

	emu_timer *m_pulse_nmi_timer = nullptr;

	enum { IRQ_KBD, IRQ_TANBUS };

	uint8_t bffx_r(offs_t offset);
	void bffx_w(offs_t offset, uint8_t data);

	void mt65_map(address_map &map) ATTR_COLD;
	void spinv_map(address_map &map) ATTR_COLD;

	void kbd_int(int state);
	int m_keyboard_int_flag = 0;

	uint8_t m_chunky_graphics = 0;
	std::unique_ptr<uint8_t[]> m_chunky_buffer;
	tilemap_t *m_bg_tilemap = nullptr;

	void videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void pgm_chargen_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(pulse_nmi);

	std::error_condition verify_snapshot(const uint8_t *data, int size);
	std::error_condition parse_intel_hex(uint8_t *snapshot_buff, const char *src);
	std::error_condition parse_zillion_hex(uint8_t *snapshot_buff, const char *src);
	void set_cpu_regs(const uint8_t *snapshot_buff, int base);
	void snapshot_copy(uint8_t *snapshot_buff, int snapshot_size);
	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
};


class mt6809_state : public driver_device
{
public:
	mt6809_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_keyboard(*this, "keyboard")
		, m_tanbus(*this, "tanbus")
	{ }

	void mt6809(machine_config &config);

	void trigger_reset(int state);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<microtan_kbd_slot_device> m_keyboard;
	required_device<tanbus_device> m_tanbus;

	void mt6809_map(address_map &map) ATTR_COLD;

	uint8_t bffx_r(offs_t offset);
	void bffx_w(offs_t offset, uint8_t data);
};

#endif // MAME_TANGERINE_MICROTAN_H
