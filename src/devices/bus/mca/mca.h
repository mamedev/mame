// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***********************************************************************************************************

    IBM Micro Channel Architecture

    16-bit works
	32-bit is a skeleton
    
***********************************************************************************************************/

#ifndef MAME_BUS_MCA_MCA_H
#define MAME_BUS_MCA_MCA_H

#pragma once

#include <forward_list>

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mca16_device;

namespace MCABus
{
	typedef enum
	{
		ADAPTER_ID_LO = 0,
		ADAPTER_ID_HI = 1,
		OPTION_SELECT_DATA_1 = 2,
		OPTION_SELECT_DATA_2 = 3,
		OPTION_SELECT_DATA_3 = 4,
		OPTION_SELECT_DATA_4 = 5,
		SUBADDRESS_EXT_LO = 6,
		SUBADDRESS_EXT_HI = 7,
	} POS;

	const uint16_t CARD_NOT_PRESENT = 0xffff;
};

// The class that represents a 16-bit MCA slot.
class mca16_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T, typename U>
	mca16_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&mca_tag, U &&opts, const char *dflt, bool fixed)
		: mca16_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
		m_mca_bus.set_tag(std::forward<T>(mca_tag));
	}
	mca16_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	mca16_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// configuration
	required_device<device_t> m_mca_bus;
};

// device type definition
DECLARE_DEVICE_TYPE(MCA16_SLOT, mca16_slot_device)

class device_mca16_card_interface;

// Class representing the 16-bit MCA bus.
class mca16_device : public device_t,
					public device_memory_interface
{
public:
	enum
	{
		AS_MCA_MEM16   	= 0,
		AS_MCA_IO16    	= 1
	};

	// construction/destruction
	mca16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template<int Line> void dreq_w(int state)
	{
		m_out_drq_cb[Line](state);
	}
	template<int Line> void ireq_w(int state)
	{
		m_out_irq_cb[Line](state);
	};

	template <typename T> void set_memspace(T &&tag, int spacenum) { m_memspace.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_iospace(T &&tag, int spacenum) { m_iospace.set_tag(std::forward<T>(tag), spacenum); }
	auto iochrdy_callback() { return m_write_iochrdy.bind(); }
	auto iochck_callback() { return m_write_iochck.bind(); }
	auto cs_feedback_callback() { printf("cs_feedback_callback %p\n", this); return m_cs_feedback.bind(); }

	template<int Line> auto irq_callback() { return m_out_irq_cb[Line].bind(); }
	template<int Line> auto drq_callback() { return m_out_drq_cb[Line].bind(); }

	// include this in a driver to have MCA allocate its own address spaces (e.g. non-x86)
	void set_custom_spaces() { m_allocspaces = true; }
	virtual space_config_vector memory_space_config() const override;

	template<typename R, typename W> void install_device(offs_t start, offs_t end, R rhandler, W whandler)
	{
		install_space(AS_MCA_IO16, start, end, rhandler, whandler);
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
		install_space(AS_MCA_MEM16, start, end, rhandler, whandler);
	}

	void unmap_device(offs_t start, offs_t end) const { m_iospace->unmap_readwrite(start, end); }
	void unmap_bank(offs_t start, offs_t end);
	void unmap_rom(offs_t start, offs_t end);
	void unmap_readwrite(offs_t start, offs_t end);
	bool is_option_rom_space_available(offs_t start, int size);

	uint8_t dack_r(int line);
	void dack_w(int line, uint8_t data);
	void dack_line_w(int line, int state);
	void eop_w(int channels, int state);

	uint16_t dack16_r(int line);
	void dack16_w(int line, uint16_t data);

	void set_ready(int state);
	void nmi();

	virtual void set_dma_channel(uint8_t channel, device_mca16_card_interface *dev, bool do_eop);
	virtual void unset_dma_channel(uint8_t channel);

	void add_slot(const char *tag);
	void add_slot(device_slot_interface *slot);
	virtual void remap(int space_id, offs_t start, offs_t end) {};

	const address_space_config m_mem16_config, m_io16_config;

	required_address_space get_iospace() { return m_iospace; }

protected:
	mca16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	template<typename R, typename W> void install_space(int spacenum, offs_t start, offs_t end, R rhandler, W whandler);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// address spaces
	required_address_space m_memspace, m_iospace;
	int m_memwidth, m_iowidth;
	bool m_allocspaces;

	device_mca16_card_interface *m_dma_device[8];
	bool                        m_dma_eop[8];
	std::forward_list<device_slot_interface *> m_slot_list;

	void irq_request(unsigned line, bool state);
	void dma_request(int channel, bool state);

	devcb_write_line::array<16> m_out_irq_cb;
	devcb_write_line::array<8> m_out_drq_cb;

private:
	devcb_write_line m_write_iochrdy;
	devcb_write_line m_write_iochck;
	devcb_write_line m_cs_feedback;
};


// device type definition
DECLARE_DEVICE_TYPE(MCA16, mca16_device)

// ======================> device_mca16_card_interface

// The class representing a card with a 16-bit MCA interface.
class device_mca16_card_interface : public device_interface
{
	friend class mca16_device;
	template <class ElementType> friend class simple_list;
public:
	// construction/destruction
	virtual ~device_mca16_card_interface();

	virtual uint16_t get_card_id() { return m_card_id; }

	// DMA signals
	virtual uint8_t dack_r(int line) { return 0xff; };
	virtual void dack_w(int line, uint8_t data) {};
	virtual void dack_line_w(int line, int state) {};
	virtual void eop_w(int state) {};

	/// @brief Unmap MCA resources
	///
	/// Unmaps any resources that the MCA device is currently using.
	/// This includes I/O ports, IRQ lines, DMA request lines, memory allocation.
	virtual void unmap() { fatalerror("unmap() not defined for MCA card %04X", m_card_id); }

	/// @brief Remap MCA resources
	///
	/// Installs all resources that the MCA device is currently using,
	/// according to the current configuration in the POS registers.
	/// This includes I/O ports, IRQ lines, DMA request lines, memory allocation.
	virtual void remap() { fatalerror("remap() not defined for MCA card %04X", m_card_id); }

	// map_has_changed will recalculate the resources specified in POS versus the
	// resources in the IO/IRQ/DMA/memory range variables.
	// Returns true if the card needs to be remapped, false if the card does not.
	/// @brief Check for if the resource map has changed since the last remap().
	///
	/// Reads the device's POS data, checking what resources it is assigned to
	/// through POS. If the resource map is different from the current assigned
	/// resources, returns true and the caller should follow it up with an
	/// unmap/remap cycle to update the assignment.
	virtual bool map_has_changed() { fatalerror("remap() not defined for MCA card %04X", m_card_id); }

	/// @brief 8-bit I/O port read.
	/// @param offset The offset into the I/O mapping.
	/// @return The value read from the MCA card device.
	virtual uint8_t io8_r(offs_t offset) { return 0xFF; }

	/// @brief 8-bit I/O port write.
	/// @param offset The offset into the I/O mapping.
	/// @return The value written to the MCA card device.
	virtual void io8_w(offs_t offset, uint8_t data) {}

	/// @brief 16-bit I/O port read.
	/// @param offset The offset into the I/O mapping.
	/// @return The value read from the MCA card device.	
	virtual uint16_t io16_r(offs_t offset);

	/// @brief 16-bit I/O port write.
	/// @param offset The offset into the I/O mapping.
	/// @return The value written to the MCA card device.
	virtual void io16_w(offs_t offset, uint16_t data);

	// Stuff for in-band configuation.
	virtual uint8_t pos_r(offs_t offset);
	virtual void pos_w(offs_t offset, uint8_t data);

	virtual void update_pos_data_1(uint8_t data) {}
	virtual void update_pos_data_2(uint8_t data) {}
	virtual void update_pos_data_3(uint8_t data) {}
	virtual void update_pos_data_4(uint8_t data) {}

	// Stuff for out-of-band bus/card device configuration.
	void set_host_bus(device_t *mca_device) { m_mca_dev = mca_device; }
	virtual void set_mca_device() { m_mca = dynamic_cast<mca16_device *>(m_mca_dev); }

	device_mca16_card_interface *next() const { return m_next; }

	bool m_is_mapped;

protected:
	/// @brief Assert /CD_SFDBK
	///
	/// Asserts the Card Selected Feedback line, which informs the bus
	/// that a device is present at a particular I/O address.
	/// Asserted by the receiving device, not the transmitting device.
	/// If no device is present, the line is never asserted.
	virtual void assert_card_feedback() { } //printf("16-bit card interface: assert card feedback %p\n", m_mca); m_mca->cs_feedback_callback(); }
	virtual void reset_option_select() { memset(m_option_select, 0, 8);  }
		
	uint8_t 	m_option_select[8];
	uint16_t 	m_card_id;
public:
	device_mca16_card_interface(const machine_config &mconfig, device_t &device, uint16_t card_id);

	mca16_device	*m_mca;
	device_t    	*m_mca_dev;

private:
	device_mca16_card_interface *m_next;
};

////////////////////////////
class device_mca32_card_interface;

// The class representing the 32-bit MCA bus.
class mca32_device : public mca16_device
{
public:
	mca32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	enum
	{
		AS_MCA_MEM32   	= 0,
		AS_MCA_IO32    	= 1
	};

	const address_space_config m_mem32_config, m_io32_config;
		
	virtual space_config_vector memory_space_config() const override;

	template<typename R, typename W> void install_memory(offs_t start, offs_t end, R rhandler, W whandler)
	{
		install_space(AS_MCA_MEM32, start, end, rhandler, whandler);
	}

	template<typename R, typename W> void install_space(int spacenum, offs_t start, offs_t end, R rhandler, W whandler);

	uint8_t io8_r(offs_t offset) { return m_iospace->read_byte(offset); }
	void io8_w(offs_t offset, uint8_t data) { return m_iospace->write_byte(offset, data); }

protected:
	mca32_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_config_complete() override;
};

DECLARE_DEVICE_TYPE(MCA32, mca32_device);

// The class that represents a 32-bit MCA slot.
class mca32_slot_device : public mca16_slot_device
{
public:
	// construction/destruction
	template <typename T, typename U>
	mca32_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&mca_tag, U &&opts, const char *dflt, bool fixed)
		: mca32_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
		m_mca_bus.set_tag(std::forward<T>(mca_tag));
	}
	mca32_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	mca32_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
};

// The class that represents a 32-bit MCA card.
class device_mca32_card_interface : public device_mca16_card_interface
{
	friend class mca32_device;

	public:
	device_mca32_card_interface(const machine_config &mconfig, device_t &device, uint16_t card_id);

	virtual void set_mca_device() override;

	mca32_device	*m_mca;
};

DECLARE_DEVICE_TYPE(MCA32_SLOT, mca32_slot_device)

#endif // MAME_BUS_MCA_MCA_H
