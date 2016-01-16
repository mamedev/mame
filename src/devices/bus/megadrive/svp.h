// license:???
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
	md_rom_svp_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	md_rom_svp_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

//protected:
	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void set_bank_to_rom(const char *banktag, UINT32 offset) override;

	required_device<device_t> m_svp;
	required_ioport m_test_ipt;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) override;
	virtual DECLARE_WRITE16_MEMBER(write) override;
	virtual DECLARE_READ16_MEMBER(read_a15) override;
	virtual DECLARE_WRITE16_MEMBER(write_a15) override;

	virtual int read_test() override;

	virtual DECLARE_READ16_MEMBER(rom_read1);
	virtual DECLARE_READ16_MEMBER(rom_read2);

	virtual DECLARE_READ16_MEMBER(read_pm0);
	virtual DECLARE_READ16_MEMBER(read_pm1);
	virtual DECLARE_READ16_MEMBER(read_pm2);
	virtual DECLARE_READ16_MEMBER(read_pm4);
	virtual DECLARE_READ16_MEMBER(read_xst);
	virtual DECLARE_READ16_MEMBER(read_pmc);
	virtual DECLARE_READ16_MEMBER(read_al);
	virtual DECLARE_WRITE16_MEMBER(write_pm0);
	virtual DECLARE_WRITE16_MEMBER(write_pm1);
	virtual DECLARE_WRITE16_MEMBER(write_pm2);
	virtual DECLARE_WRITE16_MEMBER(write_pm4);
	virtual DECLARE_WRITE16_MEMBER(write_xst);
	virtual DECLARE_WRITE16_MEMBER(write_pmc);
	virtual DECLARE_WRITE16_MEMBER(write_al);

	UINT32 pm_io(int reg, int write, UINT32 d);

	UINT32 m_pmac_read[6];  // read modes/addrs for PM0-PM5
	UINT32 m_pmac_write[6]; // write ...
	PAIR m_pmc;
	UINT32 m_emu_status;
	UINT16 m_xst;       // external status, mapped at a15000 and a15002 on 68k side.
	UINT16 m_xst2;      // status of XST (bit1 set when 68k writes to XST)
	UINT8 m_iram[0x800]; // IRAM (0-0x7ff)
	UINT8 m_dram[0x20000]; // [0x20000];
};


// device type definition
extern const device_type MD_ROM_SVP;

#endif
