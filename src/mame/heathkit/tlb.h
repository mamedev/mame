// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Terminal Logic Board (TLB)

****************************************************************************/

#ifndef MAME_HEATHKIT_TLB_H
#define MAME_HEATHKIT_TLB_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/ins8250.h"
#include "machine/mm5740.h"
#include "sound/beep.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class heath_tlb_device : public device_t
{
public:
	heath_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// interface routines
	auto serial_data_callback() { return m_write_sd.bind(); }

	void cb1_w(int state);

protected:
	heath_tlb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_resolve_objects() override;

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<cpu_device> m_maincpu;

private:
	void key_click_w(uint8_t data);
	void bell_w(uint8_t data);
	uint8_t kbd_key_r();
	uint8_t kbd_flags_r();
	uint16_t translate_mm5740_b(uint16_t b);

	void checkBeepState();

	void serial_out_b(uint8_t data);

	int mm5740_shift_r();
	int mm5740_control_r();
	void mm5740_data_ready_w(int state);

	MC6845_UPDATE_ROW(crtc_update_row);

	TIMER_CALLBACK_MEMBER(key_click_off);
	TIMER_CALLBACK_MEMBER(bell_off);

	emu_timer *m_key_click_timer;
	emu_timer *m_bell_timer;

	devcb_write_line m_write_sd;

	required_device<palette_device> m_palette;
	required_device<mc6845_device>  m_crtc;
	required_device<ins8250_device> m_ace;
	required_device<beep_device>    m_beep;
	required_shared_ptr<uint8_t>    m_p_videoram;
	required_region_ptr<u8>         m_p_chargen;
	required_device<mm5740_device>  m_mm5740;
	required_memory_region          m_kbdrom;
	required_ioport                 m_kbspecial;

	uint8_t  m_transchar;
	bool     m_strobe;
	bool     m_keyclickactive;
	bool     m_bellactive;

};

class heath_super19_tlb_device : public heath_tlb_device
{
public:
	heath_super19_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
};

class heath_watz_tlb_device : public heath_tlb_device
{
public:
	heath_watz_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
};

class heath_ultra_tlb_device : public heath_tlb_device
{
public:
	heath_ultra_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	void mem_map(address_map &map);
};

DECLARE_DEVICE_TYPE(HEATH_TLB, heath_tlb_device)
DECLARE_DEVICE_TYPE(HEATH_SUPER19, heath_super19_tlb_device)
DECLARE_DEVICE_TYPE(HEATH_WATZ, heath_watz_tlb_device)
DECLARE_DEVICE_TYPE(HEATH_ULTRA, heath_ultra_tlb_device)

#endif // MAME_HEATHKIT_TLB_H
