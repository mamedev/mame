// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        ISA bus device


    8 bit ISA bus connector:

    A1    O  I/O CH CHK    B1       GND
    A2   IO  D7            B2   I   RESET
    A3   IO  D6            B3       +5V
    A4   IO  D5            B4    O  IRQ2
    A5   IO  D4            B5       -5V
    A6   IO  D3            B6    O  DRQ2
    A7   IO  D2            B7       -12V
    A8   IO  D1            B8    O  /NOWS
    A9   IO  D0            B9       +12V
    A10   O  I/O CH RDY    B10      GND
    A11  I   AEN           B11  I   /SMEMW
    A12  IO  A19           B12  I   /SMEMR
    A13  IO  A18           B13  I   /IOW
    A14  IO  A17           B14  I   /IOR
    A15  IO  A16           B15  I   /DACK3
    A16  IO  A15           B16   O  DRQ3
    A17  IO  A14           B17  I   /DACK1
    A18  IO  A13           B18   O  DRQ1
    A19  IO  A12           B19  IO  /REFRESH
    A20  IO  A11           B20  I   CLOCK
    A21  IO  A10           B21   O  IRQ7
    A22  IO  A9            B22   O  IRQ6
    A23  IO  A8            B23   O  IRQ5
    A24  IO  A7            B24   O  IRQ4
    A25  IO  A6            B25   O  IRQ3
    A26  IO  A5            B26  I   /DACK2
    A27  IO  A4            B27  I   T/C
    A28  IO  A3            B28  I   ALE
    A29  IO  A2            B29      +5V
    A30  IO  A1            B30  I   OSC
    A31  IO  A0            B31      GND

    16 bit ISA bus extension

    C1   I   SBHE          D1   I   /MEM CS 16
    C2   IO  A23           D2   I   /I/O CS 16
    C3   IO  A22           D3    O  IRQ10
    C4   IO  A21           D4    O  IRQ11
    C5   IO  A20           D5    O  IRQ12
    C6   IO  A19           D6    O  IRQ15
    C7   IO  A18           D7    O  IRQ14
    C8   IO  A17           D8   I   /DACK0
    C9   I   /MEMR         D9    O  DRQ0
    C10  I   /MEMW         D10  I   /DACK5
    C11  IO  D8            D11   O  DRQ5
    C12  IO  D9            D12  I   /DACK6
    C13  IO  D10           D13   O  DRQ6
    C14  IO  D11           D14  I   /DACK7
    C15  IO  D12           D15   O  DRQ7
    C16  IO  D13           D16      +5V
    C17  IO  D14           D17  I   MASTER
    C18  IO  D15           D18      GND

***************************************************************************/

#ifndef MAME_BUS_ISA_ISA_H
#define MAME_BUS_ISA_ISA_H

#pragma once

#include <forward_list>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class isa8_device;

class isa8_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T, typename U>
	isa8_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&isa_tag, U &&opts, const char *dflt, bool fixed)
		: isa8_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
		m_isa_bus.set_tag(std::forward<T>(isa_tag));
	}
	isa8_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	isa8_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<device_t> m_isa_bus;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_SLOT, isa8_slot_device)

class device_isa8_card_interface;
// ======================> isa8_device
class isa8_device : public device_t,
					public device_memory_interface
{
public:
	enum
	{
		AS_ISA_MEM    = 0,
		AS_ISA_IO     = 1,
		AS_ISA_MEMALT = 2,
		AS_ISA_IOALT  = 3
	};

	// construction/destruction
	isa8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T> void set_memspace(T &&tag, int spacenum) { m_memspace.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_iospace(T &&tag, int spacenum) { m_iospace.set_tag(std::forward<T>(tag), spacenum); }
	auto iochrdy_callback() { return m_write_iochrdy.bind(); }
	auto iochck_callback() { return m_write_iochck.bind(); }
	auto irq2_callback() { return m_out_irq2_cb.bind(); }
	auto irq3_callback() { return m_out_irq3_cb.bind(); }
	auto irq4_callback() { return m_out_irq4_cb.bind(); }
	auto irq5_callback() { return m_out_irq5_cb.bind(); }
	auto irq6_callback() { return m_out_irq6_cb.bind(); }
	auto irq7_callback() { return m_out_irq7_cb.bind(); }
	auto drq1_callback() { return m_out_drq1_cb.bind(); }
	auto drq2_callback() { return m_out_drq2_cb.bind(); }
	auto drq3_callback() { return m_out_drq3_cb.bind(); }

	// include this in a driver to have ISA allocate its own address spaces (e.g. non-x86)
	void set_custom_spaces() { m_allocspaces = true; }

	// for ISA8, put the 8-bit configs in the primary slots and the 16-bit configs in the secondary
	virtual space_config_vector memory_space_config() const override;

	template<typename R, typename W> void install_device(offs_t start, offs_t end, R rhandler, W whandler)
	{
		install_space(AS_ISA_IO, start, end, rhandler, whandler);
	}
	template<typename T> void install_device(offs_t addrstart, offs_t addrend, T &device, void (T::*map)(class address_map &map), uint64_t unitmask = ~u64(0))
	{
		m_iospace->install_device(addrstart, addrend, device, map, unitmask);
	}
	void install_bank(offs_t start, offs_t end, uint8_t *data);
	void install_bank(offs_t start, offs_t end, memory_bank *bank);
	void install_rom(device_t *dev, offs_t start, offs_t end, const char *region);
	template<typename R, typename W> void install_memory(offs_t start, offs_t end, R rhandler, W whandler)
	{
		install_space(AS_ISA_MEM, start, end, rhandler, whandler);
	}
	template<typename T> void install_memory(offs_t addrstart, offs_t addrend, T &device, void (T:: *map)(class address_map &map), uint64_t unitmask = ~u64(0))
	{
		m_memspace->install_device(addrstart, addrend, device, map, unitmask);
	}

	void unmap_device(offs_t start, offs_t end) const { m_iospace->unmap_readwrite(start, end); }
	void unmap_bank(offs_t start, offs_t end);
	void unmap_rom(offs_t start, offs_t end);
	void unmap_readwrite(offs_t start, offs_t end);
	bool is_option_rom_space_available(offs_t start, int size);

	// FIXME: shouldn't need to expose this
	address_space &memspace() const { return *m_memspace; }

	void irq2_w(int state);
	void irq3_w(int state);
	void irq4_w(int state);
	void irq5_w(int state);
	void irq6_w(int state);
	void irq7_w(int state);

	void drq1_w(int state);
	void drq2_w(int state);
	void drq3_w(int state);

	// 8 bit accessors for ISA-defined address spaces
	uint8_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint8_t data);
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);

	uint8_t dack_r(int line);
	void dack_w(int line, uint8_t data);
	void dack_line_w(int line, int state);
	void eop_w(int channels, int state);

	void set_ready(int state);
	void nmi();

	virtual void set_dma_channel(uint8_t channel, device_isa8_card_interface *dev, bool do_eop);

	void add_slot(const char *tag);
	void add_slot(device_slot_interface *slot);
	virtual void remap(int space_id, offs_t start, offs_t end);

	const address_space_config m_mem_config, m_io_config, m_mem16_config, m_io16_config;

protected:
	isa8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	template<typename R, typename W> void install_space(int spacenum, offs_t start, offs_t end, R rhandler, W whandler);

	// device_t implementation
	virtual void device_config_complete() override;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// address spaces
	required_address_space m_memspace, m_iospace;
	int m_memwidth, m_iowidth;
	bool m_allocspaces;

	devcb_write_line    m_out_irq2_cb;
	devcb_write_line    m_out_irq3_cb;
	devcb_write_line    m_out_irq4_cb;
	devcb_write_line    m_out_irq5_cb;
	devcb_write_line    m_out_irq6_cb;
	devcb_write_line    m_out_irq7_cb;
	devcb_write_line    m_out_drq1_cb;
	devcb_write_line    m_out_drq2_cb;
	devcb_write_line    m_out_drq3_cb;

	device_isa8_card_interface *m_dma_device[8];
	bool                        m_dma_eop[8];
	std::forward_list<device_slot_interface *> m_slot_list;

private:
	devcb_write_line m_write_iochrdy;
	devcb_write_line m_write_iochck;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8, isa8_device)

// ======================> device_isa8_card_interface

// class representing interface-specific live isa8 card
class device_isa8_card_interface : public device_interface
{
	friend class isa8_device;
public:
	// construction/destruction
	virtual ~device_isa8_card_interface();

	void set_isa_device();
	// configuration access
	virtual uint8_t dack_r(int line);
	virtual void dack_w(int line, uint8_t data);
	virtual void dack_line_w(int line, int state);
	virtual void eop_w(int state);

	virtual void remap(int space_id, offs_t start, offs_t end) {}

	// inline configuration
	void set_isabus(device_t *isa_device) { m_isa_dev = isa_device; }

public:
	device_isa8_card_interface(const machine_config &mconfig, device_t &device);

	isa8_device  *m_isa;
	device_t     *m_isa_dev;
};

class isa16_device;

class isa16_slot_device : public isa8_slot_device
{
public:
	// construction/destruction
	template <typename T, typename U>
	isa16_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&isa_tag, U &&opts, const char *dflt, bool fixed)
		: isa16_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
		m_isa_bus.set_tag(std::forward<T>(isa_tag));
	}
	isa16_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA16_SLOT, isa16_slot_device)

// ======================> isa16_device
class isa16_device : public isa8_device
{
public:
	// construction/destruction
	isa16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq10_callback() { return m_out_irq10_cb.bind(); }
	auto irq11_callback() { return m_out_irq11_cb.bind(); }
	auto irq12_callback() { return m_out_irq12_cb.bind(); }
	auto irq14_callback() { return m_out_irq14_cb.bind(); }
	auto irq15_callback() { return m_out_irq15_cb.bind(); }
	auto drq0_callback() { return m_out_drq0_cb.bind(); }
	auto drq5_callback() { return m_out_drq5_cb.bind(); }
	auto drq6_callback() { return m_out_drq6_cb.bind(); }
	auto drq7_callback() { return m_out_drq7_cb.bind(); }

	template<typename R, typename W> void install16_device(offs_t start, offs_t end, R rhandler, W whandler);

	// for ISA16, put the 16-bit configs in the primary slots and the 8-bit configs in the secondary
	virtual space_config_vector memory_space_config() const override;

	void irq10_w(int state);
	void irq11_w(int state);
	void irq12_w(int state);
	void irq14_w(int state);
	void irq15_w(int state);

	void drq0_w(int state);
	void drq5_w(int state);
	void drq6_w(int state);
	void drq7_w(int state);

	uint16_t dack16_r(int line);
	void dack16_w(int line, uint16_t data);
	virtual void remap(int space_id, offs_t start, offs_t end) override;

	// 16 bit accessors for ISA-defined address spaces
	uint16_t mem16_r(offs_t offset, uint16_t mem_mask = 0xffff);
	void mem16_w(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t io16_r(offs_t offset, uint16_t mem_mask = 0xffff);
	void io16_w(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	// byte-swapped versions of 16-bit accessors
	uint16_t mem16_swap_r(offs_t offset, uint16_t mem_mask = 0xffff);
	void mem16_swap_w(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t io16_swap_r(offs_t offset, uint16_t mem_mask = 0xffff);
	void io16_swap_w(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

private:
	// internal state
	devcb_write_line    m_out_irq10_cb;
	devcb_write_line    m_out_irq11_cb;
	devcb_write_line    m_out_irq12_cb;
	devcb_write_line    m_out_irq14_cb;
	devcb_write_line    m_out_irq15_cb;
	devcb_write_line    m_out_drq0_cb;
	devcb_write_line    m_out_drq5_cb;
	devcb_write_line    m_out_drq6_cb;
	devcb_write_line    m_out_drq7_cb;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA16, isa16_device)

// ======================> device_isa16_card_interface

// class representing interface-specific live isa16 card
class device_isa16_card_interface : public device_isa8_card_interface
{
	friend class isa16_device;
public:
	// construction/destruction
	virtual ~device_isa16_card_interface();
	virtual uint16_t dack16_r(int line);
	virtual void dack16_w(int line, uint16_t data);

	void set_isa_device();

protected:
	device_isa16_card_interface(const machine_config &mconfig, device_t &device);

	isa16_device  *m_isa;
};

#endif // MAME_BUS_ISA_ISA_H
