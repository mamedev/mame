/***************************************************************************

  a2bus.h - Apple II slot bus and card emulation

  by R. Belmont

***************************************************************************/

#pragma once

#ifndef __A2BUS_H__
#define __A2BUS_H__

#include "emu.h"

#define INH_SLOT_INVALID    (255)

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_A2BUS_BUS_ADD(_tag, _cputag, _config) \
    MCFG_DEVICE_ADD(_tag, A2BUS, 0) \
    MCFG_DEVICE_CONFIG(_config) \
    a2bus_device::static_set_cputag(*device, _cputag); \

#define MCFG_A2BUS_SLOT_ADD(_nbtag, _tag, _slot_intf, _def_slot, _def_inp) \
    MCFG_DEVICE_ADD(_tag, A2BUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false) \
	a2bus_slot_device::static_set_a2bus_slot(*device, _nbtag, _tag); \

#define MCFG_A2BUS_SLOT_REMOVE(_tag)    \
    MCFG_DEVICE_REMOVE(_tag)

#define MCFG_A2BUS_ONBOARD_ADD(_nbtag, _tag, _dev_type, _def_inp) \
    MCFG_DEVICE_ADD(_tag, _dev_type, 0) \
	MCFG_DEVICE_INPUT_DEFAULTS(_def_inp) \
	device_a2bus_card_interface::static_set_a2bus_tag(*device, _nbtag, _tag);

#define MCFG_A2BUS_BUS_REMOVE(_tag) \
    MCFG_DEVICE_REMOVE(_tag)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_device;

class a2bus_slot_device : public device_t,
						 public device_slot_interface
{
public:
	// construction/destruction
	a2bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a2bus_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();

    // inline configuration
    static void static_set_a2bus_slot(device_t &device, const char *tag, const char *slottag);
protected:
	// configuration
	const char *m_a2bus_tag, *m_a2bus_slottag;
};

// device type definition
extern const device_type A2BUS_SLOT;

// ======================> a2bus_interface

struct a2bus_interface
{
    devcb_write_line	m_out_irq_cb;
    devcb_write_line	m_out_nmi_cb;
    devcb_write_line	m_out_inh_cb;
};

class device_a2bus_card_interface;
// ======================> a2bus_device
class a2bus_device : public device_t,
                    public a2bus_interface
{
public:
	// construction/destruction
	a2bus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a2bus_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	// inline configuration
	static void static_set_cputag(device_t &device, const char *tag);

	void add_a2bus_card(int slot, device_a2bus_card_interface *card);
    device_a2bus_card_interface *get_a2bus_card(int slot);

    void set_irq_line(int state);
    void set_nmi_line(int state);
    void set_inh_slotnum(int slot);

	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

	// internal state
	cpu_device   *m_maincpu;

	devcb_resolved_write_line	m_out_irq_func;
	devcb_resolved_write_line	m_out_nmi_func;
	devcb_resolved_write_line	m_out_inh_func;

    device_a2bus_card_interface *m_device_list[8];
	const char *m_cputag;
};


// device type definition
extern const device_type A2BUS;

// ======================> device_a2bus_card_interface

// class representing interface-specific live a2bus card
class device_a2bus_card_interface : public device_slot_card_interface
{
	friend class a2bus_device;
public:
	// construction/destruction
	device_a2bus_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_a2bus_card_interface();

    virtual UINT8 read_c0nx(address_space &space, UINT8 offset) { printf("a2bus: unhandled read at C0n%x\n", offset); return 0; }       // C0nX - /DEVSEL
    virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data) { printf("a2bus: unhandled write %02x to C0n%x\n", data, offset); }
    virtual UINT8 read_cnxx(address_space &space, UINT8 offset) { return 0; }       // CnXX - /IOSEL
    virtual void write_cnxx(address_space &space, UINT8 offset, UINT8 data) { printf("a2bus: unhandled write %02x to Cn%02x\n", data, offset); }
    virtual UINT8 read_c800(address_space &space, UINT16 offset) { return 0; }      // C800 - /IOSTB
    virtual void write_c800(address_space &space, UINT16 offset, UINT8 data) { printf("a2bus: unhandled write %02x to %04x\n", data, offset + 0xc800); }
    virtual bool take_c800() { return true; }   // override and return false if your card doesn't take over the c800 space
    virtual UINT8 read_inh_rom(address_space &space, UINT16 offset) { return 0; }
    virtual void write_inh_rom(address_space &space, UINT16 offset, UINT8 data) { }

	device_a2bus_card_interface *next() const { return m_next; }

	void set_a2bus_device();

    UINT32 get_slotromspace() { return 0xc000 | (m_slot<<8); }      // return Cn00 address for this slot
    UINT32 get_slotiospace() { return 0xc080 + (m_slot<<4); }       // return C0n0 address for this slot

    void raise_slot_irq() { m_a2bus->set_irq_line(ASSERT_LINE); }
    void lower_slot_irq() { m_a2bus->set_irq_line(CLEAR_LINE); }
    void raise_slot_nmi() { m_a2bus->set_nmi_line(ASSERT_LINE); }
    void lower_slot_nmi() { m_a2bus->set_nmi_line(CLEAR_LINE); }
    void raise_slot_inh() { m_a2bus->set_inh_slotnum(m_slot); }
    void lower_slot_inh() { m_a2bus->set_inh_slotnum(INH_SLOT_INVALID); }

    // inline configuration
    static void static_set_a2bus_tag(device_t &device, const char *tag, const char *slottag);
public:
	a2bus_device  *m_a2bus;
    const char *m_a2bus_tag, *m_a2bus_slottag;
    int m_slot;
	device_a2bus_card_interface *m_next;
};

#endif  /* __A2BUS_H__ */
