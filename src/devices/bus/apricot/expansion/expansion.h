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

#ifndef MAME_BUS_APRICOT_EXPANSION_EXPANSION_H
#define MAME_BUS_APRICOT_EXPANSION_EXPANSION_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_EXPANSION_ADD(_tag, _cpu_tag) \
	MCFG_DEVICE_ADD(_tag, APRICOT_EXPANSION_BUS, 0) \
	downcast<apricot_expansion_bus_device &>(*device).set_cpu_tag(this, _cpu_tag);

#define MCFG_EXPANSION_IOP_ADD(_tag) \
	downcast<apricot_expansion_bus_device &>(*device).set_iop_tag(this, _tag);

#define MCFG_EXPANSION_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, APRICOT_EXPANSION_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_EXPANSION_DMA1_HANDLER(_devcb) \
	devcb = &downcast<apricot_expansion_bus_device &>(*device).set_dma1_handler(DEVCB_##_devcb);

#define MCFG_EXPANSION_DMA2_HANDLER(_devcb) \
	devcb = &downcast<apricot_expansion_bus_device &>(*device).set_dma2_handler(DEVCB_##_devcb);

#define MCFG_EXPANSION_EXT1_HANDLER(_devcb) \
	devcb = &downcast<apricot_expansion_bus_device &>(*device).set_ext1_handler(DEVCB_##_devcb);

#define MCFG_EXPANSION_EXT2_HANDLER(_devcb) \
	devcb = &downcast<apricot_expansion_bus_device &>(*device).set_ext2_handler(DEVCB_##_devcb);

#define MCFG_EXPANSION_INT2_HANDLER(_devcb) \
	devcb = &downcast<apricot_expansion_bus_device &>(*device).set_int2_handler(DEVCB_##_devcb);

#define MCFG_EXPANSION_INT3_HANDLER(_devcb) \
	devcb = &downcast<apricot_expansion_bus_device &>(*device).set_int3_handler(DEVCB_##_devcb);


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
	apricot_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	apricot_expansion_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
};

// device type definition
DECLARE_DEVICE_TYPE(APRICOT_EXPANSION_SLOT, apricot_expansion_slot_device)


// ======================> apricot_expansion_bus_device

class apricot_expansion_bus_device : public device_t
{
public:
	// construction/destruction
	apricot_expansion_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~apricot_expansion_bus_device();

	template <class Object> devcb_base &set_dma1_handler(Object &&cb) { return m_dma1_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dma2_handler(Object &&cb) { return m_dma2_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_ext1_handler(Object &&cb) { return m_ext1_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_ext2_handler(Object &&cb) { return m_ext2_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_int2_handler(Object &&cb) { return m_int2_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_int3_handler(Object &&cb) { return m_int3_handler.set_callback(std::forward<Object>(cb)); }

	// inline configuration
	void set_cpu_tag(device_t *owner, const char *tag) { m_cpu_tag = tag; }
	void set_iop_tag(device_t *owner, const char *tag) { m_iop_tag = tag; }

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
	virtual void device_start() override;
	virtual void device_reset() override;

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
DECLARE_DEVICE_TYPE(APRICOT_EXPANSION_BUS, apricot_expansion_bus_device)


// ======================> device_apricot_expansion_card_interface

class device_apricot_expansion_card_interface : public device_slot_card_interface
{
	template <class ElementType> friend class simple_list;

public:
	// construction/destruction
	virtual ~device_apricot_expansion_card_interface();

	void set_bus_device(apricot_expansion_bus_device *bus);

	device_apricot_expansion_card_interface *next() const { return m_next; }

protected:
	device_apricot_expansion_card_interface(const machine_config &mconfig, device_t &device);

	apricot_expansion_bus_device *m_bus;

	device_apricot_expansion_card_interface *m_next;
};


// include here so drivers don't need to
#include "cards.h"


#endif // MAME_BUS_APRICOT_EXPANSION_EXPANSION_H
