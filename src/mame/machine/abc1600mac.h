// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 1600 Memory Access Controller emulation

**********************************************************************/

#pragma once

#ifndef __ABC1600_MAC__
#define __ABC1600_MAC__

#include "emu.h"
#include "cpu/m68000/m68000.h"



///*************************************************************************
//  MACROS / CONSTANTS
///*************************************************************************

#define ABC1600_MAC_TAG "mac"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_ABC1600_MAC_ADD(_cpu_tag, _program_map) \
	MCFG_DEVICE_ADD(ABC1600_MAC_TAG, ABC1600_MAC, 0) \
	MCFG_DEVICE_ADDRESS_MAP(AS_PROGRAM, _program_map) \
	downcast<abc1600_mac_device *>(device)->set_cpu_tag(_cpu_tag);



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> abc1600_mac_device

class abc1600_mac_device : public device_t,
							public device_memory_interface
{
public:
	abc1600_mac_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;

	void set_cpu_tag(const char *cpu_tag) { m_cpu_tag = cpu_tag; }

	virtual DECLARE_ADDRESS_MAP(map, 8);

	DECLARE_READ8_MEMBER( cause_r );
	DECLARE_WRITE8_MEMBER( task_w );
	DECLARE_READ8_MEMBER( segment_r );
	DECLARE_WRITE8_MEMBER( segment_w );
	DECLARE_READ8_MEMBER( page_r );
	DECLARE_WRITE8_MEMBER( page_w );
	DECLARE_WRITE8_MEMBER( dmamap_w );

	DECLARE_READ8_MEMBER( dma0_mreq_r ) { return dma_mreq_r(DMAMAP_R0_LO, offset); }
	DECLARE_WRITE8_MEMBER( dma0_mreq_w ) { dma_mreq_w(DMAMAP_R0_LO, offset, data); }
	DECLARE_READ8_MEMBER( dma0_iorq_r ) { return dma_iorq_r(DMAMAP_R0_LO, offset); }
	DECLARE_WRITE8_MEMBER( dma0_iorq_w ) { dma_iorq_w(DMAMAP_R0_LO, offset, data); }
	DECLARE_READ8_MEMBER( dma1_mreq_r ) { return dma_mreq_r(DMAMAP_R1_LO, offset); }
	DECLARE_WRITE8_MEMBER( dma1_mreq_w ) { dma_mreq_w(DMAMAP_R1_LO, offset, data); }
	DECLARE_READ8_MEMBER( dma1_iorq_r ) { return dma_iorq_r(DMAMAP_R1_LO, offset); }
	DECLARE_WRITE8_MEMBER( dma1_iorq_w ) { dma_iorq_w(DMAMAP_R1_LO, offset, data); }
	DECLARE_READ8_MEMBER( dma2_mreq_r ) { return dma_mreq_r(DMAMAP_R2_LO, offset); }
	DECLARE_WRITE8_MEMBER( dma2_mreq_w ) { dma_mreq_w(DMAMAP_R2_LO, offset, data); }
	DECLARE_READ8_MEMBER( dma2_iorq_r ) { return dma_iorq_r(DMAMAP_R2_LO, offset); }
	DECLARE_WRITE8_MEMBER( dma2_iorq_w ) { dma_iorq_w(DMAMAP_R2_LO, offset, data); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_PROGRAM) const override;

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

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	int get_current_task(offs_t offset);
	offs_t get_segment_address(offs_t offset);
	offs_t get_page_address(offs_t offset, UINT8 segd);
	offs_t translate_address(offs_t offset, int *nonx, int *wp);
	UINT8 read_user_memory(offs_t offset);
	void write_user_memory(offs_t offset, UINT8 data);
	int get_fc();
	UINT8 read_supervisor_memory(address_space &space, offs_t offset);
	void write_supervisor_memory(address_space &space, offs_t offset, UINT8 data);
	offs_t get_dma_address(int index, UINT16 offset);
	UINT8 dma_mreq_r(int index, UINT16 offset);
	void dma_mreq_w(int index, UINT16 offset, UINT8 data);
	UINT8 dma_iorq_r(int index, UINT16 offset);
	void dma_iorq_w(int index, UINT16 offset, UINT8 data);

	const address_space_config m_space_config;

	required_memory_region m_rom;
	optional_shared_ptr<UINT8> m_segment_ram;
	optional_shared_ptr<UINT16> m_page_ram;

	const char *m_cpu_tag;
	m68000_base_device *m_cpu;

	int m_ifc2;
	UINT8 m_task;
	UINT8 m_dmamap[8];
	UINT8 m_cause;
};


// device type definition
extern const device_type ABC1600_MAC;



#endif
