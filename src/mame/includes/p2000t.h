// license:BSD-3-Clause
// copyright-holders:Paul Daniels
/*****************************************************************************
 *
 * includes/p2000t.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_P2000T_H
#define MAME_INCLUDES_P2000T_H

#pragma once

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/p2000/exp.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "sound/spkrdev.h"
#include "video/saa5050.h"
#include "machine/p2000t_mdcr.h"
#include "machine/ram.h"
#include "emupal.h"
//#include "schedule.h"
#include "screen.h"
#include "softlist.h"

class p2000t_state : public driver_device
{
public:
    p2000t_state(const machine_config &mconfig, device_type type, const char *tag)
        : driver_device(mconfig, type, tag)
        , m_saa5050(*this, "saa5050")
        , m_screen(*this, "screen")
        , m_videoram(*this, "videoram")
        , m_maincpu(*this, "maincpu")
        , m_speaker(*this, "speaker")
        , m_mdcr(*this, "mdcr")
        , m_printer(*this, "printer")
        , m_slot1(*this, "slot1")
        , m_slot2(*this, "slot2")
        , m_ext1(*this, "ext1")
        , m_ext2(*this, "ext2")
        , m_ram(*this, RAM_TAG)
        , m_bank(*this, "bank")
        , m_keyboard(*this, "KEY.%u", 0)
        , m_jumper(*this, "jumper")
    {
    }

    void p2000t(machine_config &config);
    
protected:
    uint8_t p2000t_port_000f_r(offs_t offset);
    uint8_t p2000t_port_202f_r();

    void p2000t_port_00_w(uint8_t data);
    void p2000t_port_101f_w(uint8_t data);
    void p2000t_port_303f_w(uint8_t data);
    void p2000t_port_505f_w(uint8_t data);
    void p2000t_port_707f_w(uint8_t data);
    uint8_t p2000t_port_707f_r();

    uint8_t p2000t_port_888b_r(offs_t channel);
    void p2000t_port_888b_w(offs_t channel, uint8_t data);

    void p2000t_port_9494_w(uint8_t data);

    uint8_t videoram_r(offs_t offset);
    virtual void machine_start() override;
    
    INTERRUPT_GEN_MEMBER(p2000_interrupt);
    DECLARE_WRITE_LINE_MEMBER(p2000_slot_interrupt);
    DECLARE_DEVICE_IMAGE_LOAD_MEMBER(card_load);

    void p2000t_mem(address_map &map);
    void p2000t_io(address_map &map);
    int in_80char_mode() { return BIT(m_port_707f, 0) ? 1 : 0; }
    
    uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
    optional_device<saa5050_device> m_saa5050; // Only available on P2000T not on M-model
    required_device<screen_device> m_screen;
    required_shared_ptr<uint8_t> m_videoram;
    
    required_device<z80_device> m_maincpu;
    required_device<speaker_sound_device> m_speaker;
    required_device<mdcr_device> m_mdcr;
    required_device<rs232_port_device> m_printer;
    
    required_device<generic_slot_device> m_slot1;
	required_device<p2000_expansion_slot_device> m_slot2;
	required_device<p2000_expansion_slot_device> m_ext1;
    required_device<p2000_expansion_slot_device> m_ext2;

    required_device<ram_device> m_ram;
    
    required_memory_bank m_bank;

private:
    required_ioport_array<10> m_keyboard;
    required_ioport m_jumper;
    
    uint8_t m_port_101f = 0;
    uint8_t m_port_303f = 0;
    uint8_t m_port_707f = 0;
};


class p2000m_state : public p2000t_state
{
public:
    p2000m_state(const machine_config &mconfig, device_type type, const char *tag)
        : p2000t_state(mconfig, type, tag)
        , m_gfxdecode(*this, "gfxdecode")
        , m_palette(*this, "palette")
    {
    }

    void p2000m(machine_config &config);

protected:
    virtual void video_start() override;
    void p2000m_palette(palette_device &palette) const;
    uint32_t screen_update_p2000m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
    
    void p2000m_mem(address_map &map);

private:
    required_device<gfxdecode_device> m_gfxdecode;
    required_device<palette_device> m_palette;

    int8_t m_frame_count;
};


#endif // MAME_INCLUDES_P2000T_H
