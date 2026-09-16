// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Pierpaolo Prazzoli,Grazvydas Ignotas
#ifndef MAME_BUS_MEGADRIVE_SVP_H
#define MAME_BUS_MEGADRIVE_SVP_H

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
	md_rom_svp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	md_rom_svp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;

	virtual void set_bank_to_rom(const char *banktag, uint32_t offset) override;

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual uint16_t read_a15(offs_t offset) override;
	virtual void write_a15(offs_t offset, uint16_t data) override;

	virtual int read_test() override;

	virtual uint16_t rom_read1(offs_t offset);
	virtual uint16_t rom_read2(offs_t offset);

	virtual uint16_t read_pm0();
	virtual uint16_t read_pm1();
	virtual uint16_t read_pm2();
	virtual uint16_t read_pm4();
	virtual uint16_t read_xst();
	virtual uint16_t read_pmc();
	virtual uint16_t read_al();
	virtual void write_pm0(uint16_t data);
	virtual void write_pm1(uint16_t data);
	virtual void write_pm2(uint16_t data);
	virtual void write_pm4(uint16_t data);
	virtual void write_xst(uint16_t data);
	virtual void write_pmc(uint16_t data);
	virtual void write_al(uint16_t data);

private:
	required_device<cpu_device> m_svp;
	required_ioport m_test_ipt;

	uint32_t pm_io(int reg, int write, uint32_t d);

	uint32_t m_pmac_read[6];  // read modes/addrs for PM0-PM5
	uint32_t m_pmac_write[6]; // write ...
	PAIR m_pmc;
	uint32_t m_emu_status;
	uint16_t m_xst;       // external status, mapped at a15000 and a15002 on 68k side.
	uint16_t m_xst2;      // status of XST (bit1 set when 68k writes to XST)
	uint8_t m_iram[0x800]; // IRAM (0-0x7ff)
	uint8_t m_dram[0x20000]; // [0x20000];

	void md_svp_ext_map(address_map &map) ATTR_COLD;
	void md_svp_ssp_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(MD_ROM_SVP, md_rom_svp_device)

#endif // MAME_BUS_MEGADRIVE_SVP_H
