// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************
  Implementation of the P2000T hires graphicscard which was sold as extention board
  This emulation was based on documentation from the P2000T presevation page
  https://github.com/p2000t/documentation/tree/master/hardware [HiRes.pdf]
  To work properly the GOD36.BIN rom and "Taaltje 1.1 32K.cas" is needed 
  https://github.com/p2000t/software/tree/master/cartridges
 
  The hires-card produces a video image (in 2 modes 256*256 pixels or 512 * 256)
  wich is merged as "underlay" with the original P2000 "text" video image. 
  The P2000 video signal is fed back into hires card via the external RGB-videoplug.

    P2000 High Resolution Colour Graphics Card

    P2000 CPU side 
        68-6b       Hires communication ports
                    68: PIO A DATA status channel 
                    6a: PIO A ctrl status channel 
                    69: PIO B DATA status channel 
                    6b: PIO B ctrl status channel 

    Hires CPU: Z80
        0000-1fff   ROM + Video RAM page 0
        2000-3fff   ROM + Video RAM page 1
        4000-5fff   Video RAM page 2
        6000-7fff   Video RAM page 3
        8000-9fff   Video RAM page 4
        a000-bfff   Video RAM page 5
        c000-dfff   Video RAM page 6
        e000-ffff   Video RAM page 7
        
    Hires Ports:
        80-8f       Red color table
        90-9f       Green color table
        a0-af       Red color table
        b0-bf       RGB-P2000T image switch
        c0-cf       Memory map
        d0-df       Scroll register
        e0-ef       Mode register
        f0,f1,f2,f3 Communication channels (PIO A+B)
                    f0: PIO A DATA status channel 
                    f2: PIO A ctrl status channel 
                    f1: PIO B DATA status channel 
                    f3: PIO B ctrl status channel 

**********************************************************************/

#ifndef MAME_BUS_P2000_HIRES_H
#define MAME_BUS_P2000_HIRES_H

#pragma once

#include "bus/p2000/exp.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "machine/z80pio.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "screen.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class p2000_hires_device :
	public device_t,
	public device_p2000_expansion_slot_card_interface
{

public:
	// construction/destruction
	p2000_hires_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
    
protected:
	// device-level overrides
	virtual void device_start() override;
	
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
    virtual const tiny_rom_entry *device_rom_region() const override;

    void screen_update_draw_pixel(bitmap_rgb32 &bitmap, int xpos, int ypos, uint32_t color, int xlen, int ylen );
    uint8_t vidon_r() override;
    uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
    
    void mem(address_map &map);
    void io(address_map &map);
    u8 memory_read(offs_t offset);
    void memory_write(offs_t offset, u8 data);

    /* P2000T CPU side */
    uint8_t mainpio_pa_r_cb();
    void mainpio_pa_w_cb(uint8_t data);
    uint8_t mainpio_pb_r_cb();
    void mainpio_pb_w_cb(uint8_t data);

    /* hires CPU side */
    void port_808f_w(uint8_t data);
    void port_909f_w(uint8_t data);
    void port_a0af_w(uint8_t data);
    void port_b0bf_w(uint8_t data);
    void port_c0cf_w(uint8_t data);
    void port_d0df_w(uint8_t data);
    void port_e0ef_w(uint8_t data);

    uint8_t hirespio_pa_r_cb();
    void hirespio_pa_w_cb(uint8_t data);
    uint8_t hirespio_pb_r_cb();
    void hirespio_pb_w_cb(uint8_t data);

    required_device<z80_device> m_hirescpu;
    required_device<ram_device> m_hiresram;
    required_device<z80pio_device> m_mainpio;
    required_device<z80pio_device> m_hirespio;
    required_region_ptr<uint8_t> m_hiresrom;
    
private:
    uint8_t m_channel_a_data;
    uint8_t m_channel_b_data;

    void port_2c_w(uint8_t data);    
    void hirespio_emulate_sync();
    
    uint8_t m_hires_image_mode;
    uint8_t m_hires_image_select;
    uint8_t m_hires_scroll_reg;

    //  required_memory_bank m_hires_membank;
    bool m_hiresmem_bank0_ROM = true;
    
    static const size_t LUT_TABLE_SIZE = 16;
    uint8_t m_hires_lut_red[LUT_TABLE_SIZE];
    uint8_t m_hires_lut_red_cnt = 0;
    uint8_t m_hires_lut_blue[LUT_TABLE_SIZE];
    uint8_t m_hires_lut_blue_cnt = 0;
    uint8_t m_hires_lut_green[LUT_TABLE_SIZE];
    uint8_t m_hires_lut_green_cnt = 0;
};


// device type definition
DECLARE_DEVICE_TYPE(P2000_HIRES, p2000_hires_device)


#endif // MAME_BUS_P2000_HIRES_H
