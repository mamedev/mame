// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Sprow ARM7TDMI Co-Processor

    http://www.sprow.co.uk/bbc/armcopro.htm

**********************************************************************/


#ifndef MAME_BUS_BBC_TUBE_ARM7_H
#define MAME_BUS_BBC_TUBE_ARM7_H

#include "tube.h"
#include "cpu/arm7/arm7.h"
#include "machine/ram.h"
#include "machine/tube.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_arm7_device

class bbc_tube_arm7_device :
	public device_t,
	public device_bbc_tube_interface
{
	enum
	{
		INTERRUPT = 0, // 7800_0000
		EXT_MEMORY,    // 7810_0000
		DRAM_CONTROL,  // 7818_0000
		CACHE_MEMORY,  // 7820_0000
		EXP_INTERRUPT, // 7BF0_0000
		CHIP_CONFIG,   // B700_0000
		PORT_CONTROL,  // B7A0_1000
		SYSTEM,        // B800_0000
		SYSTEM_TIMER,  // B800_1000
		NUM_REGS
	};

public:
	// construction/destruction
	bbc_tube_arm7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

private:
	required_device<cpu_device> m_maincpu;
	required_memory_region m_flash;
	required_device<tube_device> m_ula;
	required_device<ram_device> m_ram;
	memory_view m_bank0_view;

	void arm7_map(address_map &map) ATTR_COLD;

	void prst_w(int state);

	uint32_t oki_reg_r(offs_t offset);
	void oki_reg_w(offs_t offset, uint32_t data);

	// registers
	uint32_t m_registers[NUM_REGS][32];
	uint8_t m_tube10;

	void update_bank0();
	void update_interrupts();

	void efiq_w(int state);
	void exint3_w(int state);

	int m_efiq_state;
	int m_exint3_state;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_ARM7, bbc_tube_arm7_device)


#endif // MAME_BUS_BBC_TUBE_ARM7_H
