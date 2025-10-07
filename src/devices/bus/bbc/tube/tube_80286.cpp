// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn 80286 2nd Processor

**********************************************************************/

#include "emu.h"
#include "tube_80286.h"

#include "cpu/i86/i286.h"
#include "machine/ram.h"
#include "machine/tube.h"

#include "softlist_dev.h"


namespace {

// ======================> bbc_tube_80286_device

class bbc_tube_80286_device : public device_t, public device_bbc_tube_interface
{
public:
	bbc_tube_80286_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_TUBE_80286, tag, owner, clock)
		, device_bbc_tube_interface(mconfig, *this)
		, m_i80286(*this, "i80286")
		, m_ula(*this, "ula")
		, m_ram(*this, "ram")
		, m_bootstrap(*this, "bootstrap")
		, m_irq_latch(0)
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

private:
	IRQ_CALLBACK_MEMBER( irq_callback );

	required_device<i80286_cpu_device> m_i80286;
	required_device<tube_device> m_ula;
	required_device<ram_device> m_ram;
	required_memory_region m_bootstrap;

	uint8_t m_irq_latch;
	uint8_t disable_boot_rom();

	void tube_80286_io(address_map &map) ATTR_COLD;
	void tube_80286_mem(address_map &map) ATTR_COLD;

	void prst_w(int state);
};


//-------------------------------------------------
//  ADDRESS_MAP( tube_80286_mem )
//-------------------------------------------------

void bbc_tube_80286_device::tube_80286_mem(address_map &map)
{
	map.global_mask(0xfffff);
	map(0x00000, 0xfffff).ram().share("ram");
	map(0xc0000, 0xc3fff).rom().region("bootstrap", 0).mirror(0x3c000);
}

//-------------------------------------------------
//  ADDRESS_MAP( tube_80286_io )
//-------------------------------------------------

void bbc_tube_80286_device::tube_80286_io(address_map &map)
{
	map.unmap_value_high();
	map(0x50, 0x50).r(FUNC(bbc_tube_80286_device::disable_boot_rom));
	map(0x60, 0x60).lw8(NAME([this](uint8_t data) { m_irq_latch = data; }));
	map(0x80, 0x8f).rw("ula", FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w)).umask16(0x00ff);
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START(tube_80286)
	ROM_REGION16_LE(0x4000, "bootstrap", 0)
	ROM_LOAD16_BYTE("m512_lo.ic31", 0x0000, 0x2000, CRC(c0df8707) SHA1(7f6d843d5aea6bdb36cbd4623ae942b16b96069d)) // 2201,287-02
	ROM_LOAD16_BYTE("m512_hi.ic32", 0x0001, 0x2000, CRC(e47f10b2) SHA1(45dc8d7e7936afbec6de423569d9005a1c350316)) // 2201,288-02
ROM_END

const tiny_rom_entry *bbc_tube_80286_device::device_rom_region() const
{
	return ROM_NAME( tube_80286 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_tube_80286_device::device_add_mconfig(machine_config &config)
{
	I80286(config, m_i80286, 12_MHz_XTAL / 2);
	m_i80286->set_addrmap(AS_PROGRAM, &bbc_tube_80286_device::tube_80286_mem);
	m_i80286->set_addrmap(AS_IO, &bbc_tube_80286_device::tube_80286_io);
	m_i80286->set_irq_acknowledge_callback(FUNC(bbc_tube_80286_device::irq_callback));

	TUBE(config, m_ula);
	m_ula->pnmi_handler().set_inputline(m_i80286, INPUT_LINE_NMI);
	m_ula->pirq_handler().set_inputline(m_i80286, INPUT_LINE_INT0);
	m_ula->prst_handler().set(FUNC(bbc_tube_80286_device::prst_w));

	RAM(config, m_ram).set_default_size("1M");

	SOFTWARE_LIST(config, "flop_ls_80186").set_original("bbc_flop_80186");
	SOFTWARE_LIST(config, "pc_disk_list").set_compatible("ibm5150");
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_80286_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_80286_device::device_reset()
{
	address_space &program = m_i80286->space(AS_PROGRAM);

	program.install_rom(0xc0000, 0xc3fff, 0x3c000, m_bootstrap->base());
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_tube_80286_device::prst_w(int state)
{
	device_reset();

	m_i80286->set_input_line(INPUT_LINE_RESET, state);
}

uint8_t bbc_tube_80286_device::host_r(offs_t offset)
{
	return m_ula->host_r(offset);
}

void bbc_tube_80286_device::host_w(offs_t offset, uint8_t data)
{
	m_ula->host_w(offset, data);
}

uint8_t bbc_tube_80286_device::disable_boot_rom()
{
	if (!machine().side_effects_disabled())
		m_i80286->space(AS_PROGRAM).install_ram(0xc0000, 0xfffff, m_ram->pointer() + 0xc0000);

	return 0xff;
}

//-------------------------------------------------
//  irq vector callback
//-------------------------------------------------

IRQ_CALLBACK_MEMBER(bbc_tube_80286_device::irq_callback)
{
	return m_irq_latch;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_TUBE_80286, device_bbc_tube_interface, bbc_tube_80286_device, "bbc_tube_80286", "Acorn 80286 2nd Processor")
