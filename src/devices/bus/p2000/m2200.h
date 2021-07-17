// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************
Implementation of the P2000T Miniware Muliti purpose extention board
  This emulation was based on documentation from the P2000T presevation page
  https://github.com/p2000t/documentation/tree/master/hardware 
   [M2200.pdf and FieldSupportManual.pdf]
  
    P2000 M2200 Multi Purpose Floppy Dics Controller Card

    Ports:
        80-83       CTCBSEL - Z80-CTCB (SIO baud control) (channel 0-3)
        84-87       SIO 
                        84: data reg. RS232
                        85: cmd/status RS232
                        86: data RS422
                        87: md/status RS422)
        88-8b       CTCSEL - Z80-CTC (channel 0-3)
        8c-8f       FDCSEL - Floppy ctrl (fdc) uPD765
        90          IOSEL - Floppy/DC control port
        94          SWSEL - RAM Bank select
        
        95-97       RAM disk 
                        95: set track
                        96: set sector (+ reset data cnt)
                        97: data (in/out)
        98-9b       Centronics 
                        98: data reg. [out]
                        99: status reg. [in]
                        9a: strobe on
                        9b: strobe off
        9c-9d       RTC (9c=set register  9d=data in/out)

**********************************************************************/

#ifndef MAME_BUS_P2000_M2200_FDC_H
#define MAME_BUS_P2000_M2200_FDC_H

#pragma once

#include "bus/p2000/exp.h"
#include "bus/rs232/rs232.h"
#include "bus/centronics/ctronics.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "machine/clock.h"
#include "machine/z80pio.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/upd765.h"
#include "imagedev/floppy.h"
#include "machine/mc146818.h"
#include "formats/p2000t_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======= p2000_fdc_device ================

class p2000_fdc_device :  
    public device_t,
	public device_p2000_expansion_slot_card_interface
{

public:
	// construction/destruction
	p2000_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
    p2000_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
    
	// device-level overrides
	virtual void device_start() override;
    virtual void device_reset() override;

    // optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

    DECLARE_WRITE_LINE_MEMBER(fdc_interrupt);
    uint8_t dew_r() override;
    
    /* FDC control lines */
    DECLARE_WRITE_LINE_MEMBER(fdc_irq_trigger);
    DECLARE_WRITE_LINE_MEMBER(fdc_index_trigger);
    DECLARE_WRITE_LINE_MEMBER(fdc_hdl_wr_trigger);
    
    /* FDC control ports */
    void fdc_control(uint8_t data);
    uint8_t fdc_fcdr();
    void fdc_write(uint8_t data);
    uint8_t fdc_read();

    // Drives spin at 300 RPM so 200ms per revolution if after 2 rev. stil no index
    // marker is found there is no disc present.
    const static unsigned int m_ready_control_delay = 400;
    TIMER_CALLBACK_MEMBER(ready_timer_cb);
    
    required_device<z80ctc_device> m_ctc; 
    required_device<upd765a_device> m_fdc;

    const static unsigned int m_num_of_drives = 4;
    optional_device_array<floppy_connector, m_num_of_drives> m_floppy;
    const char* m_floppy_def_param[m_num_of_drives] = {
        "N/A",
        "525ds40",
        "525ds40",
        "525ds80"
    };

    static void floppy_formats(format_registration &fr);

    virtual const z80_daisy_config * get_z80_daisy_config() {
        static const z80_daisy_config m2200_daisy_chain[] =
        {
            { ":ext1:fdc:ctc" },
            { nullptr }
        };
        return m2200_daisy_chain;
    }
    emu_timer *m_ready_control_timer;
    int m_ready_control_pulse_cnt = 0;
    emu_timer *m_reset_timer;

private:
    int m_fdc_hdl_line = 0;
    int m_fdc_index_n = 0;
    int m_fdc_control_reg = -1;
};


// ======= p2000_m2200_multipurpose_device ================

class p2000_m2200_multipurpose_device :  public p2000_fdc_device
{

public:
	// construction/destruction
	p2000_m2200_multipurpose_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
    p2000_m2200_multipurpose_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
    virtual void device_reset() override;
    
    // optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

    /* RAM Drive control ports */
    void port_95_w(uint8_t data);
    uint8_t port_95_r();
    void port_96_w(uint8_t data);
    uint8_t port_96_r();
    void port_97_w(uint8_t data);
    uint8_t port_97_r() ;
    
    /* Centonics control ports */
    void port_98_w(uint8_t data);
    uint8_t port_99_r();
    void port_9a_w(uint8_t data);
    uint8_t port_9a_r();
    void port_9b_w(uint8_t data);
    uint8_t port_9b_r();
    
    required_device<mc146818_device> m_rtc;
    required_device<z80ctc_device> m_ctc2; 
    required_device<z80sio_device> m_sio; 
    required_device<centronics_device> m_centronics; 

    DECLARE_WRITE_LINE_MEMBER(centronics_ack_w);
    DECLARE_WRITE_LINE_MEMBER(centronics_busy_w);
    DECLARE_WRITE_LINE_MEMBER(centronics_paper_empty_w);
    DECLARE_WRITE_LINE_MEMBER(centronics_printer_on_w);
    DECLARE_WRITE_LINE_MEMBER(centronics_error_w);

    virtual uint32_t get_ramdrive_max_tracks_mask() { return (0x40000 / 4096) - 1; }
    virtual uint32_t get_ramdrive_size()            { return 0x40000; /* 256KB */ }
    const z80_daisy_config * get_z80_daisy_config() override {
        static const z80_daisy_config m2200_daisy_chain[] =
        {
            { ":ext1:m2200:ctc" },
            { ":ext1:m2200:sio" },
            { ":ext1:m2200:ctc2" },
            { nullptr }
        };
        return m2200_daisy_chain;
    }

private:
    uint8_t m_centronics_status = 0x0f;

    uint8_t m_ramdisk_track = 0;
    uint8_t m_ramdisk_sector = 0;
    uint8_t m_ramdisk_index = 0;
    std::unique_ptr<uint8_t[]> m_ramdisk;
};

// ======= p2000_m2200d_multipurpose_device ================

class p2000_m2200d_multipurpose_device :  public p2000_m2200_multipurpose_device
{
   
public:
	// construction/destruction
	p2000_m2200d_multipurpose_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
    uint32_t get_ramdrive_max_tracks_mask() override { return (0x10000 / 4096) - 1;  }
    uint32_t get_ramdrive_size() override            { return  0x10000; /* 64KB */ }
    virtual const z80_daisy_config * get_z80_daisy_config() override {
        static const z80_daisy_config m2200_daisy_chain[] =
        {
            { ":ext1:m2200d:ctc" },
            { ":ext1:m2200d:sio" },
            { ":ext1:m2200d:ctc2" },
            { nullptr }
        };
        return m2200_daisy_chain;
    }
    
};

// device type definition
DECLARE_DEVICE_TYPE(P2000_FDC,      p2000_fdc_device)
DECLARE_DEVICE_TYPE(P2000_M2200,    p2000_m2200_multipurpose_device)
DECLARE_DEVICE_TYPE(P2000_M2200D,   p2000_m2200d_multipurpose_device)

#endif // MAME_BUS_P2000_M2200_FDC_H
