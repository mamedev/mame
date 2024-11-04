// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Asante MC3NB NuBus Ethernet card (DP83902) (NuBus)
  Apple NuBus Ethernet Card (DP8390) (NuBus)
  Farallon EtherMac 30i-TH (DP83901) (SE/30 and IIsi PDS)

  The declaration ROM calls the Farallon PDS card "EtherMac 30i-TH", but that
  full name doesn't appear anywhere user-visible unless you're using software
  such as TattleTech or TechTool that can show declaration ROM names.

  These cards are all a Mac adaptation of the ISA NE2000, with a DP8390
  or DP8390x Ethernet controller plus 16K or more of on-card RAM.  There
  is no DMA, the CPU must read/write packets to the on-card RAM and the
  DP839* chip can DMA to and from that RAM (but not the host system).

  FssD0000 - RAM buffer (16K on Apple cards expandable to 64K,
             64K stock on Farallon and Asante cards)
  FssE0000 - DP83902 registers

***************************************************************************/

#include "emu.h"
#include "nubus_asntmc3b.h"

#include "machine/dp8390.h"

#include "multibyte.h"

#define MAC8390_ROM_REGION  "asntm3b_rom"

namespace {

ROM_START( asntm3nb )
	ROM_REGION(0x4000, MAC8390_ROM_REGION, 0)
	ROM_LOAD( "asante_mc3b.bin", 0x000000, 0x004000, CRC(4f86d451) SHA1(d0a41df667e6b51fbc63f9251d593f4fc49104ba) )
ROM_END

ROM_START( appleenet )
	ROM_REGION(0x4000, MAC8390_ROM_REGION, 0)
	ROM_LOAD( "aenet1",       0x000000, 0x004000, CRC(e3ae8c26) SHA1(01ddc15ee84b17128203cb812f29bac6b20fd642) )
ROM_END

ROM_START( ethermac30i )
	ROM_REGION(0x4000, MAC8390_ROM_REGION, 0)
	ROM_LOAD( "5000118-00-01.bin", 0x000000, 0x004000, CRC(49a602ec) SHA1(65a8e87180a793cd54d30930cc030b95723369f1) )
ROM_END

ROM_START( macconilc )
	ROM_REGION(0x4000, MAC8390_ROM_REGION, 0)
	ROM_LOAD( "asante_maccon_lc_mcilc_1.1.bin", 0x000000, 0x004000, CRC(b95940be) SHA1(317255bcb552ef43f43f85109706ed860baaf6dc) )
ROM_END

class nubus_mac8390_device : public device_t,
							 public device_nubus_card_interface
{
protected:
	// construction/destruction
	nubus_mac8390_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	u8 asntm3b_ram_r(offs_t offset);
	void asntm3b_ram_w(offs_t offset, u8 data);
	u32 dp_r(offs_t offset, u32 mem_mask = ~0);
	void dp_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	required_device<dp8390_device> m_dp83902;

private:
	void dp_irq_w(int state);
	u8 dp_mem_read(offs_t offset);
	void dp_mem_write(offs_t offset, u8 data);

	std::unique_ptr<u8[]> m_ram;
	u8 m_prom[16];
};

class nubus_asntmc3nb_device : public nubus_mac8390_device
{
public:
	nubus_asntmc3nb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class nubus_appleenet_device : public nubus_mac8390_device
{
public:
	nubus_appleenet_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class pds030_ethermac30i_device : public nubus_mac8390_device
{
public:
	pds030_ethermac30i_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
};

class pdslc_macconilc_device : public nubus_mac8390_device
{
public:
	pdslc_macconilc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
};

void nubus_mac8390_device::device_add_mconfig(machine_config &config)
{
	DP8390D(config, m_dp83902, 0);
	m_dp83902->irq_callback().set(FUNC(nubus_mac8390_device::dp_irq_w));
	m_dp83902->mem_read_callback().set(FUNC(nubus_mac8390_device::dp_mem_read));
	m_dp83902->mem_write_callback().set(FUNC(nubus_mac8390_device::dp_mem_write));
}

const tiny_rom_entry *nubus_mac8390_device::device_rom_region() const
{
	return ROM_NAME( asntm3nb );
}

const tiny_rom_entry *nubus_appleenet_device::device_rom_region() const
{
	return ROM_NAME( appleenet );
}

const tiny_rom_entry *pds030_ethermac30i_device::device_rom_region() const
{
	return ROM_NAME( ethermac30i );
}

const tiny_rom_entry *pdslc_macconilc_device::device_rom_region() const
{
	return ROM_NAME(macconilc);
}

nubus_mac8390_device::nubus_mac8390_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_dp83902(*this, "dp83902")
{
}

nubus_asntmc3nb_device::nubus_asntmc3nb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_mac8390_device(mconfig, NUBUS_ASNTMC3NB, tag, owner, clock)
{
}

nubus_appleenet_device::nubus_appleenet_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_mac8390_device(mconfig, NUBUS_APPLEENET, tag, owner, clock)
{
}

pds030_ethermac30i_device::pds030_ethermac30i_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_mac8390_device(mconfig, PDS030_ETHERMAC30I, tag, owner, clock)
{
}

pdslc_macconilc_device::pdslc_macconilc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_mac8390_device(mconfig, PDSLC_MACCONILC, tag, owner, clock)
{
}

void nubus_mac8390_device::device_start()
{
	const u32 slotspace = get_slotspace();
	u8 mac[6];
	u32 num = machine().rand();
	memset(m_prom, 0x57, 16);
	mac[2] = 0x1b;
	put_u24be(mac+3, num);
	mac[0] = mac[1] = 0;  // avoid gcc warning
	memcpy(m_prom, mac, 6);
	m_dp83902->set_mac(mac);

	m_ram = std::make_unique<u8[]>(0x10000);
	save_pointer(NAME(m_ram), 0x10000);

	install_declaration_rom(MAC8390_ROM_REGION);

//  printf("[ASNTMC3NB %p] slotspace = %x\n", this, slotspace);

	// TODO: move 24-bit mirroring down into nubus.cpp
	u32 ofs_24bit = slotno()<<20;
	nubus().install_device(slotspace+0xd0000, slotspace+0xdffff, read8sm_delegate(*this, FUNC(nubus_mac8390_device::asntm3b_ram_r)), write8sm_delegate(*this, FUNC(nubus_mac8390_device::asntm3b_ram_w)));
	nubus().install_device(slotspace+0xe0000, slotspace+0xe003f, read32s_delegate(*this, FUNC(nubus_mac8390_device::dp_r)), write32s_delegate(*this, FUNC(nubus_mac8390_device::dp_w)));
	nubus().install_device(slotspace+0xd0000+ofs_24bit, slotspace+0xdffff+ofs_24bit, read8sm_delegate(*this, FUNC(nubus_mac8390_device::asntm3b_ram_r)), write8sm_delegate(*this, FUNC(nubus_mac8390_device::asntm3b_ram_w)));
	nubus().install_device(slotspace+0xe0000+ofs_24bit, slotspace+0xe003f+ofs_24bit, read32s_delegate(*this, FUNC(nubus_mac8390_device::dp_r)), write32s_delegate(*this, FUNC(nubus_mac8390_device::dp_w)));
}

void pds030_ethermac30i_device::device_start()
{
	// this card is hardwired to slot 9 and the driver looks for it there
	set_pds_slot(0x9);
	nubus_mac8390_device::device_start();
}

void pdslc_macconilc_device::device_start()
{
	// this card is hardwired to slot E and the driver looks for it there
	set_pds_slot(0xe);
	nubus_mac8390_device::device_start();
}

void nubus_mac8390_device::device_reset()
{
	m_dp83902->dp8390_reset(0);
	memcpy(m_prom, &m_dp83902->get_mac()[0], 6);
}

void nubus_mac8390_device::asntm3b_ram_w(offs_t offset, u8 data)
{
//    printf("MC3NB: CPU wrote %02x to RAM @ %x\n", data, offset);
	m_ram[offset] = data;
}

u8 nubus_mac8390_device::asntm3b_ram_r(offs_t offset)
{
//    printf("MC3NB: CPU read %02x @ RAM %x\n", m_ram[offset], offset);
	return m_ram[offset];
}

void nubus_mac8390_device::dp_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (mem_mask == 0xff000000)
	{
//        printf("%02x to 8390 @ %x\n", data>>24, 0xf-offset);
		m_dp83902->cs_write(0xf-offset, data>>24);
	}
	else if (mem_mask == 0xffff0000)
	{
		m_dp83902->remote_write(data>>16);
	}
	else
	{
		fatalerror("%s", util::string_format("asntmc3nb: write %08x to DP83902 @ %x with unhandled mask %08x %s\n", data, offset, mem_mask, machine().describe_context()).c_str());
	}
}

u32 nubus_mac8390_device::dp_r(offs_t offset, u32 mem_mask)
{
	if (mem_mask == 0xff000000)
	{
		return (m_dp83902->cs_read(0xf-offset)<<24);
	}
	else if (mem_mask == 0xffff0000)
	{
		return (m_dp83902->remote_read()<<16);
	}
	else
	{
		fatalerror("%s", util::string_format("asntmc3nb: read DP83902 @ %x with unhandled mask %08x %s\n", offset, mem_mask, machine().describe_context()).c_str());
	}

	return 0;
}

void nubus_mac8390_device::dp_irq_w(int state)
{
	if (state)
	{
		raise_slot_irq();
	}
	else
	{
		lower_slot_irq();
	}
}

u8 nubus_mac8390_device::dp_mem_read(offs_t offset)
{
//    printf("MC3NB: 8390 read RAM @ %x = %02x\n", offset, m_ram[offset]);
	return m_ram[offset];
}

void nubus_mac8390_device::dp_mem_write(offs_t offset, u8 data)
{
//    printf("MC3NB: 8390 wrote %02x to RAM @ %x\n", data, offset);
	m_ram[offset] = data;
}

}

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_ASNTMC3NB, device_nubus_card_interface, nubus_asntmc3nb_device, "nb_amc3b", "Asante MC3NB Ethernet card")
DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_APPLEENET, device_nubus_card_interface, nubus_appleenet_device, "nb_aenet", "Apple NuBus Ethernet card")
DEFINE_DEVICE_TYPE_PRIVATE(PDS030_ETHERMAC30I, device_nubus_card_interface, pds030_ethermac30i_device, "pds30_emac", "Farallon EtherMac 30i-TH Ethernet card")
DEFINE_DEVICE_TYPE_PRIVATE(PDSLC_MACCONILC, device_nubus_card_interface, pdslc_macconilc_device, "pdslc_macconlc", "Asante MacCON i LC Ethernet card")
