#pragma once

#ifndef __CUDA_H__
#define __CUDA_H__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CUDA_TAG	"cuda"

#define CUDA_341S0060   0x1100  // v2.40 (Most common: Performa/Quadra 6xx, PowerMac x200, x400, x500, Pippin, Gossamer G3)
#define CUDA_341S0788   0x2200  // v2.37 (LC 475/575/Quadra 605, Quadra 660AV/840AV, PowerMac x200)
#define CUDA_341S0417   0x3300  // v2.35 (Color Classic)

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CUDA_ADD(_type, _config) \
    MCFG_DEVICE_ADD(CUDA_TAG, CUDA, 0) \
    MCFG_DEVICE_CONFIG(_config) \
    MCFG_CUDA_TYPE(_type)

#define MCFG_CUDA_REPLACE(_type, _config) \
    MCFG_DEVICE_REPLACE(CUDA_TAG, CUDA, 0) \
    MCFG_DEVICE_CONFIG(_config) \
    MCFG_CUDA_TYPE(_type)

#define MCFG_CUDA_REMOVE() \
    MCFG_DEVICE_REMOVE(CUDA_TAG)

#define MCFG_CUDA_TYPE(_type) \
    cuda_device::static_set_type(*device, _type);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct cuda_interface
{
    devcb_write_line    m_out_reset_cb;
};

// ======================> cuda_device

class cuda_device :  public device_t, public device_nvram_interface, public cuda_interface
{
public:
    // construction/destruction
    cuda_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

    // inline configuration helpers
    static void static_set_type(device_t &device, int type);

    // device_config_nvram_interface overrides
    virtual void nvram_default();
    virtual void nvram_read(emu_file &file);
    virtual void nvram_write(emu_file &file);

    DECLARE_READ8_MEMBER( ddr_r );
    DECLARE_WRITE8_MEMBER( ddr_w );
    DECLARE_READ8_MEMBER( ports_r );
    DECLARE_WRITE8_MEMBER( ports_w );
    DECLARE_READ8_MEMBER( pll_r );
    DECLARE_WRITE8_MEMBER( pll_w );
    DECLARE_READ8_MEMBER( timer_ctrl_r );
    DECLARE_WRITE8_MEMBER( timer_ctrl_w );
    DECLARE_READ8_MEMBER( timer_counter_r );
    DECLARE_WRITE8_MEMBER( timer_counter_w );
    DECLARE_READ8_MEMBER( onesec_r );
    DECLARE_WRITE8_MEMBER( onesec_w );
    DECLARE_READ8_MEMBER( pram_r );
    DECLARE_WRITE8_MEMBER( pram_w );

    // VIA interface routines
    UINT8 get_treq() { return treq; }
    void set_tip(UINT8 val) { tip = val; }
    void set_byteack(UINT8 val) { byteack = val; }
    UINT8 get_via_data() { return via_data; }
    void set_via_data(UINT8 dat) { via_data = dat; }
    UINT8 get_via_clock() { return via_clock; }
    void set_adb_line(int linestate) { adb_in = (linestate == ASSERT_LINE) ? true : false; }

    int rom_offset;

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
    virtual void device_config_complete();
    virtual machine_config_constructor device_mconfig_additions() const;
    virtual const rom_entry *device_rom_region() const;

    required_device<cpu_device> m_maincpu;

    virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
    UINT8 ddrs[3];
    UINT8 ports[3];
    UINT8 pll_ctrl;
    UINT8 timer_ctrl;
    UINT8 timer_counter, ripple_counter;
    UINT8 onesec;
    UINT8 treq, byteack, tip, via_data, via_clock, last_adb;
    UINT64 last_adb_time;
    bool cuda_controls_power;
    bool adb_in;
    int reset_line;
    emu_timer *m_timer, *m_prog_timer;
    UINT8 pram[0x100], disk_pram[0x100];
    bool pram_loaded;

    void send_port(address_space &space, UINT8 offset, UINT8 data);

	devcb_resolved_write_line	m_out_reset_func;
};

// device type definition
extern const device_type CUDA;

#endif
