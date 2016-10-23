// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Pierpaolo Prazzoli,Grazvydas Ignotas
#ifndef __MD_SVP_H
#define __MD_SVP_H

#include "md_slot.h"
#include "cpu/ssp1601/ssp1601.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> md_rom_svp_device

class md_rom_svp_device : public device_t,
						public device_md_cart_interface
{
public:
	// construction/destruction
	md_rom_svp_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	md_rom_svp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

//protected:
	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void set_bank_to_rom(const char *banktag, uint32_t offset) override;

	required_device<device_t> m_svp;
	required_ioport m_test_ipt;

	// reading and writing
	virtual uint16_t read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override;
	virtual void write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override;
	virtual uint16_t read_a15(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override;
	virtual void write_a15(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override;

	virtual int read_test() override;

	virtual uint16_t rom_read1(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	virtual uint16_t rom_read2(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	virtual uint16_t read_pm0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	virtual uint16_t read_pm1(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	virtual uint16_t read_pm2(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	virtual uint16_t read_pm4(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	virtual uint16_t read_xst(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	virtual uint16_t read_pmc(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	virtual uint16_t read_al(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	virtual void write_pm0(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	virtual void write_pm1(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	virtual void write_pm2(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	virtual void write_pm4(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	virtual void write_xst(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	virtual void write_pmc(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	virtual void write_al(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint32_t pm_io(int reg, int write, uint32_t d);

	uint32_t m_pmac_read[6];  // read modes/addrs for PM0-PM5
	uint32_t m_pmac_write[6]; // write ...
	PAIR m_pmc;
	uint32_t m_emu_status;
	uint16_t m_xst;       // external status, mapped at a15000 and a15002 on 68k side.
	uint16_t m_xst2;      // status of XST (bit1 set when 68k writes to XST)
	uint8_t m_iram[0x800]; // IRAM (0-0x7ff)
	uint8_t m_dram[0x20000]; // [0x20000];
};


// device type definition
extern const device_type MD_ROM_SVP;

#endif
