// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn A500 2nd processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_A5002ndProc.html

    Notes:
      The dual MEMC ROMs are claimed to be from an Acorn M4 board, it's original
      purpose as a co-processor is unknown.

**********************************************************************/

#include "emu.h"
#include "tube_a500.h"

#include "cpu/arm/arm.h"
#include "machine/acorn_ioc.h"
#include "machine/acorn_memc.h"
#include "machine/acorn_vidc.h"
#include "machine/archimedes_keyb.h"
#include "machine/input_merger.h"
#include "machine/tube.h"

#include "screen.h"
#include "softlist_dev.h"


namespace {

class bbc_tube_a500_device : public device_t, public device_bbc_tube_interface
{
public:
	bbc_tube_a500_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	bbc_tube_a500_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t overrides
	virtual void device_start() override { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t host_r(offs_t offset) override { return m_ula->host_r(offset); }
	virtual void host_w(offs_t offset, uint8_t data) override { m_ula->host_w(offset, data); }

	required_device<arm_cpu_device> m_maincpu;
	required_device<acorn_ioc_device> m_ioc;
	required_device<acorn_memc_device> m_memc;
	required_device<acorn_vidc10_device> m_vidc;
	required_device<tube_device> m_ula;
	required_device<input_merger_device> m_irqs;
	required_device<input_merger_device> m_fiqs;

private:
	void arm_mem(address_map &map) ATTR_COLD;
	void a500_map(address_map &map) ATTR_COLD;

	void prst_w(int state) { m_maincpu->set_input_line(INPUT_LINE_RESET, state); }
};


// ======================> bbc_tube_a500d_device

class bbc_tube_a500d_device : public bbc_tube_a500_device
{
public:
	bbc_tube_a500d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void a500d_map(address_map &map) ATTR_COLD;
};


//-------------------------------------------------
//  ADDRESS_MAP( a500_map )
//-------------------------------------------------

void bbc_tube_a500_device::arm_mem(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(m_memc, FUNC(acorn_memc_device::logical_r), FUNC(acorn_memc_device::logical_w));
	map(0x02000000, 0x03ffffff).rw(m_memc, FUNC(acorn_memc_device::high_mem_r), FUNC(acorn_memc_device::high_mem_w));
}

void bbc_tube_a500_device::a500_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(m_memc, FUNC(acorn_memc_device::logical_r), FUNC(acorn_memc_device::logical_w));
	map(0x02000000, 0x023fffff).ram().mirror(0xc00000);
	map(0x03000000, 0x033fffff).m(m_ioc, FUNC(acorn_ioc_device::map));
	map(0x03000000, 0x03001fff).rw(m_ula, FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w)).umask32(0x000000ff);
	map(0x03400000, 0x035fffff).w(m_vidc, FUNC(acorn_vidc10_device::write));
	map(0x03600000, 0x037fffff).w(m_memc, FUNC(acorn_memc_device::registers_w));
	map(0x03800000, 0x039fffff).rom().mirror(0x600000).region("maincpu", 0).w(m_memc, FUNC(acorn_memc_device::page_w));
}

void bbc_tube_a500d_device::a500d_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(m_memc, FUNC(acorn_memc_device::logical_r), FUNC(acorn_memc_device::logical_w));
	map(0x02000000, 0x027fffff).ram().mirror(0x800000);
	map(0x03000000, 0x033fffff).m(m_ioc, FUNC(acorn_ioc_device::map));
	map(0x03400000, 0x035fffff).w(m_vidc, FUNC(acorn_vidc10_device::write));
	map(0x03600000, 0x037fffff).w(m_memc, FUNC(acorn_memc_device::registers_w));
	map(0x03800000, 0x039fffff).rom().mirror(0x600000).region("maincpu", 0).w(m_memc, FUNC(acorn_memc_device::page_w));
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START(tube_a500)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD32_BYTE("0.bin", 0x000000, 0x04000, CRC(7ecfeb0e) SHA1(f9cd49f86e4b210cde6b14752cdbc2962e86551f))
	ROM_LOAD32_BYTE("1.bin", 0x000001, 0x04000, CRC(f3bf6c3b) SHA1(e66496a1bf7a7f7bf300e8baebc40a4b37c19c1e))
	ROM_LOAD32_BYTE("2.bin", 0x000002, 0x04000, CRC(6b73d4af) SHA1(7ebc8c7268cda6c8bd4dcbf3b63a7ffff4d2c43c))
	ROM_LOAD32_BYTE("3.bin", 0x000003, 0x04000, CRC(607f2f49) SHA1(c46448435b86d1397c7cb144bece87e73a9220aa))
ROM_END

ROM_START(tube_a500d)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD32_BYTE("m4_brazil_8mbaddr_rom0.bin", 0x000000, 0x04000, CRC(f01fb7a6) SHA1(840a15882797572db4764f37b725cf9c5a07a8cb))
	ROM_LOAD32_BYTE("m4_brazil_8mbaddr_rom1.bin", 0x000001, 0x04000, CRC(924e4181) SHA1(4f1903ef83cb6e0cef130005b0442a6548915b8a))
	ROM_LOAD32_BYTE("m4_brazil_8mbaddr_rom2.bin", 0x000002, 0x04000, CRC(c210e9a5) SHA1(ee09b8bac275153467ec31f7a16c366a0f97b550))
	ROM_LOAD32_BYTE("m4_brazil_8mbaddr_rom3.bin", 0x000003, 0x04000, CRC(1e520555) SHA1(9b6bdeef8d7fb22ef0203c2f531a4e0a55e22c6f))
ROM_END

const tiny_rom_entry *bbc_tube_a500_device::device_rom_region() const
{
	return ROM_NAME( tube_a500 );
}

const tiny_rom_entry *bbc_tube_a500d_device::device_rom_region() const
{
	return ROM_NAME( tube_a500d );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_tube_a500_device::device_add_mconfig(machine_config &config)
{
	ARM(config, m_maincpu, 24_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_tube_a500_device::arm_mem);

	INPUT_MERGER_ANY_HIGH(config, m_fiqs).output_handler().set_inputline(m_maincpu, ARM_FIRQ_LINE);
	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, ARM_IRQ_LINE);

	TUBE(config, m_ula);
	m_ula->pnmi_handler().set(m_fiqs, FUNC(input_merger_device::in_w<0>));
	m_ula->pirq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));
	m_ula->prst_handler().set(FUNC(bbc_tube_a500_device::prst_w));

	ACORN_MEMC(config, m_memc, 24_MHz_XTAL / 3, m_vidc);
	m_memc->set_addrmap(0, &bbc_tube_a500_device::a500_map);
	m_memc->sirq_w().set(m_ioc, FUNC(acorn_ioc_device::il1_w));

	ACORN_IOC(config, m_ioc, 24_MHz_XTAL / 3);
	m_ioc->fiq_w().set(m_fiqs, FUNC(input_merger_device::in_w<1>));
	m_ioc->irq_w().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_ioc->kout_w().set("keyboard", FUNC(archimedes_keyboard_device::kin_w));
	m_ioc->peripheral_r<6>().set([this]() { logerror("%s IOC: External Expansion R\n", machine().describe_context()); return 0xffffffff; });
	m_ioc->peripheral_w<6>().set([this](uint32_t data) { logerror("%s IOC: External Expansion W %08X\n", machine().describe_context(), data); });

	ARCHIMEDES_KEYBOARD(config, "keyboard").kout().set(m_ioc, FUNC(acorn_ioc_device::kin_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.screen_vblank().set(m_ioc, FUNC(acorn_ioc_device::ir_w));

	ACORN_VIDC1(config, m_vidc, 24_MHz_XTAL);
	m_vidc->set_screen("screen");
	m_vidc->vblank().set(m_memc, FUNC(acorn_memc_device::vidrq_w));
	m_vidc->sound_drq().set(m_memc, FUNC(acorn_memc_device::sndrq_w));

	//SOFTWARE_LIST(config, "flop_ls_arm").set_original("bbc_flop_arm").set_filter("A500");
}

void bbc_tube_a500d_device::device_add_mconfig(machine_config &config)
{
	bbc_tube_a500_device::device_add_mconfig(config);

	m_memc->set_addrmap(0, &bbc_tube_a500d_device::a500d_map);

	m_ioc->peripheral_r<4>().set(m_ula, FUNC(tube_device::parasite_r));
	m_ioc->peripheral_w<4>().set(m_ula, FUNC(tube_device::parasite_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_a500_device - constructor
//-------------------------------------------------

bbc_tube_a500_device::bbc_tube_a500_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_tube_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_ioc(*this, "ioc")
	, m_memc(*this, "memc")
	, m_vidc(*this, "vidc")
	, m_ula(*this, "ula")
	, m_irqs(*this, "irqs")
	, m_fiqs(*this, "fiqs")
{
}

bbc_tube_a500_device::bbc_tube_a500_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_tube_a500_device(mconfig, BBC_TUBE_A500, tag, owner, clock)
{
}

bbc_tube_a500d_device::bbc_tube_a500d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_tube_a500_device(mconfig, BBC_TUBE_A500D, tag, owner, clock)
{
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_TUBE_A500, device_bbc_tube_interface, bbc_tube_a500_device, "bbc_tube_a500", "Acorn A500 2nd Processor")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_TUBE_A500D, device_bbc_tube_interface, bbc_tube_a500d_device, "bbc_tube_a500d", "Acorn A500 (Dual MEMC) 2nd Processor")
