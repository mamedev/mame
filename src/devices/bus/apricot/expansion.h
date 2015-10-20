// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Expansion Slot

       A          B

     -12V   32   +12V
      +5V   31   +5V
     DB0    30   DB1
     DB2    29   DB3
     DB4    28   DB5
     DB6    27   DB7
     AB10   26   AB9
     AB11   25   AB12
    /AMWC   24   /MRDC
    /DMA2   23   DT/R
    /DMA1   22   /IORC
    /MWTC   21   /RES
    /IOWC   20   /AIOWC
      GND   19   GND
    /CLK5   18   DEN
    /IRDY   17   /MRDY
    /EXT1   16   /EXT2
    /INT3   15   /ALE
      AB6   14   /INT2
      AB8   13   AB7
      DB9   12   DB8
     DB11   11   DB10
     DB13   10   DB12
     DB15    9   DB14
      AB2    8   AB1
      AB4    7   AB3
      AB0    6   AB5
     AB14    5   AB13
     AB15    4   AB16
     AB17    3   AB18
     AB19    2   /BHE
      NMI    1   CLK15

***************************************************************************/

#pragma once

#ifndef __APRICOT_EXPANSION_H__
#define __APRICOT_EXPANSION_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_EXPANSION_ADD(_tag, _cpu_tag) \
	MCFG_DEVICE_ADD(_tag, APRICOT_EXPANSION_BUS, 0) \
	apricot_expansion_bus_device::set_cpu_tag(*device, owner, _cpu_tag);

#define MCFG_EXPANSION_IOP_ADD(_tag) \
	apricot_expansion_bus_device::set_iop_tag(*device, owner, _tag);

#define MCFG_EXPANSION_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, APRICOT_EXPANSION_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_EXPANSION_DMA1_HANDLER(_devcb) \
	devcb = &apricot_expansion_bus_device::set_dma1_handler(*device, DEVCB_##_devcb);

#define MCFG_EXPANSION_DMA2_HANDLER(_devcb) \
	devcb = &apricot_expansion_bus_device::set_dma2_handler(*device, DEVCB_##_devcb);

#define MCFG_EXPANSION_EXT1_HANDLER(_devcb) \
	devcb = &apricot_expansion_bus_device::set_ext1_handler(*device, DEVCB_##_devcb);

#define MCFG_EXPANSION_EXT2_HANDLER(_devcb) \
	devcb = &apricot_expansion_bus_device::set_ext2_handler(*device, DEVCB_##_devcb);

#define MCFG_EXPANSION_INT2_HANDLER(_devcb) \
	devcb = &apricot_expansion_bus_device::set_int2_handler(*device, DEVCB_##_devcb);

#define MCFG_EXPANSION_INT3_HANDLER(_devcb) \
	devcb = &apricot_expansion_bus_device::set_int3_handler(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class device_apricot_expansion_card_interface;


// ======================> apricot_expansion_slot_device

class apricot_expansion_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	apricot_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	apricot_expansion_slot_device(const machine_config &mconfig, device_type type, const char *name,
		const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start();
};

// device type definition
extern const device_type APRICOT_EXPANSION_SLOT;


// ======================> apricot_expansion_bus_device

class apricot_expansion_bus_device : public device_t
{
public:
	// construction/destruction
	apricot_expansion_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~apricot_expansion_bus_device();

	template<class _Object> static devcb_base &set_dma1_handler(device_t &device, _Object object)
		{ return downcast<apricot_expansion_bus_device &>(device).m_dma1_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_dma2_handler(device_t &device, _Object object)
		{ return downcast<apricot_expansion_bus_device &>(device).m_dma2_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_ext1_handler(device_t &device, _Object object)
		{ return downcast<apricot_expansion_bus_device &>(device).m_ext1_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_ext2_handler(device_t &device, _Object object)
		{ return downcast<apricot_expansion_bus_device &>(device).m_ext2_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_int2_handler(device_t &device, _Object object)
		{ return downcast<apricot_expansion_bus_device &>(device).m_int2_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_int3_handler(device_t &device, _Object object)
		{ return downcast<apricot_expansion_bus_device &>(device).m_int3_handler.set_callback(object); }

	// inline configuration
	static void set_cpu_tag(device_t &device, device_t *owner, const char *tag);
	static void set_iop_tag(device_t &device, device_t *owner, const char *tag);

	void add_card(device_apricot_expansion_card_interface *card);

	// from cards
	DECLARE_WRITE_LINE_MEMBER( dma1_w );
	DECLARE_WRITE_LINE_MEMBER( dma2_w );
	DECLARE_WRITE_LINE_MEMBER( ext1_w );
	DECLARE_WRITE_LINE_MEMBER( ext2_w );
	DECLARE_WRITE_LINE_MEMBER( int2_w );
	DECLARE_WRITE_LINE_MEMBER( int3_w );

	void install_ram(offs_t addrstart, offs_t addrend, void *baseptr);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	simple_list<device_apricot_expansion_card_interface> m_dev;

	// address spaces we have access to
	address_space *m_program;
	address_space *m_io;
	address_space *m_program_iop;
	address_space *m_io_iop;

	devcb_write_line m_dma1_handler;
	devcb_write_line m_dma2_handler;
	devcb_write_line m_ext1_handler;
	devcb_write_line m_ext2_handler;
	devcb_write_line m_int2_handler;
	devcb_write_line m_int3_handler;

	// configuration
	const char *m_cpu_tag;
	const char *m_iop_tag;
};

// device type definition
extern const device_type APRICOT_EXPANSION_BUS;


// ======================> device_apricot_expansion_card_interface

class device_apricot_expansion_card_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_apricot_expansion_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_apricot_expansion_card_interface();

	void set_bus_device(apricot_expansion_bus_device *bus);

	device_apricot_expansion_card_interface *next() const { return m_next; }
	device_apricot_expansion_card_interface *m_next;

protected:
	apricot_expansion_bus_device *m_bus;
};


// include here so drivers don't need to
#include "cards.h"


#endif // __APRICOT_EXPANSION_H__
