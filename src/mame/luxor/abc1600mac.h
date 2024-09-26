// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 1600 Memory Access Controller emulation

**********************************************************************/
#ifndef MAME_LUXOR_ABC1600MAC_H
#define MAME_LUXOR_ABC1600MAC_H

#pragma once


#include "cpu/m68000/m68008.h"
#include "machine/watchdog.h"



///*************************************************************************
//  MACROS / CONSTANTS
///*************************************************************************

#define ABC1600_MAC_TAG "mac"


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> abc1600_mac_device

class abc1600_mac_device : public device_t,
							public device_memory_interface
{
public:
	abc1600_mac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	auto in_tren0_cb() { return m_read_tren[0].bind(); }
	auto out_tren0_cb() { return m_write_tren[0].bind(); }
	auto in_tren1_cb() { return m_read_tren[1].bind(); }
	auto out_tren1_cb() { return m_write_tren[1].bind(); }
	auto in_tren2_cb() { return m_read_tren[2].bind(); }
	auto out_tren2_cb() { return m_write_tren[2].bind(); }

	void rstbut_w(int state) { m_rstbut = state; }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t cause_r();
	void task_w(offs_t offset, uint8_t data);
	uint8_t segment_r(offs_t offset);
	void segment_w(offs_t offset, uint8_t data);
	uint8_t page_lo_r(offs_t offset);
	void page_lo_w(offs_t offset, uint8_t data);
	uint8_t page_hi_r(offs_t offset);
	void page_hi_w(offs_t offset, uint8_t data);
	void dmamap_w(offs_t offset, uint8_t data);
	void partst_w(int state);

	uint8_t dma0_mreq_r(offs_t offset) { return dma_mreq_r(0, DMAMAP_R0_LO, offset); }
	void dma0_mreq_w(offs_t offset, uint8_t data) { dma_mreq_w(0, DMAMAP_R0_LO, offset, data); }
	uint8_t dma0_iorq_r(offs_t offset) { return dma_iorq_r(DMAMAP_R0_LO, offset); }
	void dma0_iorq_w(offs_t offset, uint8_t data) { dma_iorq_w(DMAMAP_R0_LO, offset, data); }

	uint8_t dma1_mreq_r(offs_t offset) { return dma_mreq_r(1, DMAMAP_R1_LO, offset); }
	void dma1_mreq_w(offs_t offset, uint8_t data) { dma_mreq_w(1, DMAMAP_R1_LO, offset, data); }
	uint8_t dma1_iorq_r(offs_t offset) { return dma_iorq_r(DMAMAP_R1_LO, offset); }
	void dma1_iorq_w(offs_t offset, uint8_t data) { dma_iorq_w(DMAMAP_R1_LO, offset, data); }

	uint8_t dma2_mreq_r(offs_t offset) { return dma_mreq_r(2, DMAMAP_R2_LO, offset); }
	void dma2_mreq_w(offs_t offset, uint8_t data) { dma_mreq_w(2, DMAMAP_R2_LO, offset, data); }
	uint8_t dma2_iorq_r(offs_t offset) { return dma_iorq_r(DMAMAP_R2_LO, offset); }
	void dma2_iorq_w(offs_t offset, uint8_t data) { dma_iorq_w(DMAMAP_R2_LO, offset, data); }

	void dump();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	enum
	{
		DMAMAP_R2_LO = 0,
		DMAMAP_R2_HI,
		DMAMAP_R1_LO = 4,
		DMAMAP_R1_HI,
		DMAMAP_R0_LO,
		DMAMAP_R0_HI
	};

	offs_t get_physical_offset(offs_t offset, int task, bool &nonx, bool &wp);

	offs_t get_dma_address(int index, offs_t offset, bool &rw);
	uint8_t dma_mreq_r(int index, int dmamap, offs_t offset);
	void dma_mreq_w(int index, int dmamap, offs_t offset, uint8_t data);
	uint8_t dma_iorq_r(int dmamap, offs_t offset);
	void dma_iorq_w(int dmamap, offs_t offset, uint8_t data);

	void program_map(address_map &map) ATTR_COLD;
	void mac_map(address_map &map) ATTR_COLD;
	const address_space_config m_program_config;
	const address_space_config m_mac_config;

	required_memory_region m_rom;
	required_device<m68008_device> m_cpu;
	memory_share_creator<u8> m_segment_ram;
	memory_share_creator<u16> m_page_ram;

	required_device<watchdog_timer_device> m_watchdog;

	devcb_read8::array<3> m_read_tren;
	devcb_write8::array<3> m_write_tren;

	bool m_rstbut = 0;
	bool m_boote = 0;
	bool m_magic = 0;
	int m_task = 0;

	uint8_t m_dmamap[8];
	uint8_t m_cause;
	bool m_partst = 0;
};


constexpr int AS_MAC = 1;



// device type definition
DECLARE_DEVICE_TYPE(ABC1600_MAC, abc1600_mac_device)


#endif // MAME_LUXOR_ABC1600MAC_H
