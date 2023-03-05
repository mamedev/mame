// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

    Heathkit Terminal Logic Board (TLB)

****************************************************************************/

#ifndef MAME_HEATHKIT_TLB_H
#define MAME_HEATHKIT_TLB_H

#pragma once

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ins8250.h"
#include "machine/mm5740.h"
#include "sound/beep.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class heath_terminal_logic_board_device : public device_t
{
public:
  heath_terminal_logic_board_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

  //void tlb(machine_config &config);

protected:
  virtual ioport_constructor device_input_ports() const override;
  virtual const tiny_rom_entry *device_rom_region() const override;
  virtual void device_add_mconfig(machine_config &config) override;

private:
  virtual void device_start() override;

  void mem_map(address_map &map);
	void io_map(address_map &map);

	void key_click_w(uint8_t data);
	void bell_w(uint8_t data);
	uint8_t kbd_key_r();
	uint8_t kbd_flags_r();
	DECLARE_READ_LINE_MEMBER(mm5740_shift_r);
	DECLARE_READ_LINE_MEMBER(mm5740_control_r);
	DECLARE_WRITE_LINE_MEMBER(mm5740_data_ready_w);

	MC6845_UPDATE_ROW(crtc_update_row);

	TIMER_CALLBACK_MEMBER(key_click_off);
	TIMER_CALLBACK_MEMBER(bell_off);

	emu_timer *m_key_click_timer = nullptr;
	emu_timer *m_bell_timer = nullptr;

	required_device<palette_device> m_palette;
	required_device<cpu_device>     m_maincpu;
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

	uint16_t translate_mm5740_b(uint16_t b);
};

DECLARE_DEVICE_TYPE(TLB, heath_terminal_logic_board_device)

#endif // MAME_HEATHKIT_TLB_H
