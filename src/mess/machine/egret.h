#pragma once

#ifndef __EGRET_H__
#define __EGRET_H__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define EGRET_TAG	"egret"

#define EGRET_341S0851  0x1100
#define EGRET_341S0850  0x2200
#define EGRET_344S0100  0x3300

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_EGRET_ADD(_type) \
    MCFG_DEVICE_ADD(EGRET_TAG, EGRET, 0) \
    MCFG_EGRET_TYPE(_type)

#define MCFG_EGRET_REPLACE(_type) \
    MCFG_DEVICE_REPLACE(EGRET_TAG, EGRET, 0) \
    MCFG_EGRET_TYPE(_type)

#define MCFG_EGRET_TYPE(_type) \
    egret_device::static_set_type(*device, _type);

#define MCFG_EGRET_REMOVE() \
    MCFG_DEVICE_REMOVE(EGRET_TAG)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> egret_device

class egret_device :  public device_t, public device_nvram_interface
{
public:
    // construction/destruction
    egret_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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

    // interface routines
    UINT8 get_xcvr_session() { return xcvr_session; }
    void set_via_full(UINT8 val) { via_full = val; }
    void set_sys_session(UINT8 val) { sys_session = val; }
    UINT8 get_via_data() { return via_data; }
    void set_via_data(UINT8 dat) { via_data = dat; }
    UINT8 get_via_clock() { return via_clock; }
    void set_adb_line(int linestate) { adb_in = (linestate == ASSERT_LINE) ? true : false; }

    int rom_offset;

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
    virtual void device_config_complete() { m_shortname = "egret"; }
    virtual machine_config_constructor device_mconfig_additions() const;
    virtual const rom_entry *device_rom_region() const;

    required_device<cpu_device> m_maincpu;

    virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
    UINT8 ddrs[3];
    UINT8 ports[3];
    UINT8 pll_ctrl;
    UINT8 timer_ctrl;
    UINT8 timer_counter;
    UINT8 onesec;
    UINT8 xcvr_session, via_full, sys_session, via_data, via_clock, last_adb;
    UINT64 last_adb_time;
    bool egret_controls_power;
    bool adb_in;
    int reset_line;
    emu_timer *m_timer;
    UINT8 pram[0x100], disk_pram[0x100];
    bool pram_loaded;

    void send_port(address_space &space, UINT8 offset, UINT8 data);
};

// device type definition
extern const device_type EGRET;

#endif
