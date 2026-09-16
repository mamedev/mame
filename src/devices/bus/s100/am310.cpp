// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Alpha Micro Systems AM-310 Communications Controller

    This S-100 serial communications board for the S-100 originally used
    a Signetics 2651 PCI for each of its four ports. Some boards populate
    their locations with the (compatible) SCN2661C instead.

    The S-100 host interface is a quartet of 74LS374 latches. The status
    and command latches are written and read (respectively) by the Z80,
    while data latches provide indirect access to the 2K of onboard SRAM
    through the 9517 DMA controller.

***************************************************************************/

#include "emu.h"
#include "am310.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/am9517a.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "machine/scn_pci.h"

class s100_am310_device : public device_t, public device_s100_card_interface
{
public:
	// construction/destruction
	s100_am310_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_s100_card_interface overrides
	virtual u8 s100_sinp_r(offs_t offset) override;
	virtual void s100_sout_w(offs_t offset, u8 data) override;

private:
	// internal read/write handlers
	u8 dma_memr(offs_t offset);
	void dma_memw(offs_t offset, u8 data);
	u8 dma_ior();
	void dma_iow(u8 data);
	u8 int_r(offs_t offset);
	void int_w(offs_t offset, u8 data);

	// address map
	void z80_map(address_map &map) ATTR_COLD;

	// object finders
	required_device<z80_device> m_z80;
	required_device<generic_latch_8_device> m_dolatch;
	required_device<generic_latch_8_device> m_cmdlatch;
	required_device<generic_latch_8_device> m_dilatch;
	required_device<generic_latch_8_device> m_statlatch;
	required_device<am9517a_device> m_dma;
	required_device_array<scn_pci_device, 4> m_pci;
	required_ioport m_adr;
	required_ioport m_intr;
};

DEFINE_DEVICE_TYPE_PRIVATE(S100_AM310, device_s100_card_interface, s100_am310_device, "s100_am310", "Alpha Micro AM-310 Communications Controller")

s100_am310_device::s100_am310_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, S100_AM310, tag, owner, clock)
	, device_s100_card_interface(mconfig, *this)
	, m_z80(*this, "z80")
	, m_dolatch(*this, "dolatch")
	, m_cmdlatch(*this, "cmdlatch")
	, m_dilatch(*this, "dilatch")
	, m_statlatch(*this, "statlatch")
	, m_dma(*this, "dma")
	, m_pci(*this, "pci%u", 0U)
	, m_adr(*this, "ADR")
	, m_intr(*this, "INTR")
{
}


void s100_am310_device::device_start()
{
}

void s100_am310_device::device_reset()
{
	m_dma->dreq0_w(0);
	int_w(8, 0);
}


u8 s100_am310_device::s100_sinp_r(offs_t offset)
{
	if ((offset & 0x00fe) == (m_adr->read() << 2)) // AB1 must be 0
	{
		if (BIT(offset, 0))
			return m_statlatch->read();
		else
		{
			if (!machine().side_effects_disabled())
				m_dma->dreq0_w(1);
			return m_dilatch->read();
		}
	}

	return 0xff;
}

void s100_am310_device::s100_sout_w(offs_t offset, u8 data)
{
	if ((offset & 0x00fe) == (m_adr->read() << 2)) // AB1 must be 0
	{
		if (BIT(offset, 0))
			m_cmdlatch->write(data);
		else
		{
			if (!machine().side_effects_disabled())
				m_dma->dreq0_w(1);
			m_dolatch->write(data);
		}
	}
}

u8 s100_am310_device::dma_memr(offs_t offset)
{
	return m_z80->space(AS_PROGRAM).read_byte(offset);
}

void s100_am310_device::dma_memw(offs_t offset, u8 data)
{
	m_z80->space(AS_PROGRAM).write_byte(offset, data);
}

u8 s100_am310_device::dma_ior()
{
	if (!machine().side_effects_disabled())
		m_dma->dreq0_w(0);
	return m_dolatch->read();
}

void s100_am310_device::dma_iow(u8 data)
{
	m_dilatch->write(data);
	if (!machine().side_effects_disabled())
		m_dma->dreq0_w(0);
}

u8 s100_am310_device::int_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		int_w(offset, 0);
	return 0;
}

void s100_am310_device::int_w(offs_t offset, u8 data)
{
	u8 intr = m_intr->read();
	if (!BIT(intr, 0))
		m_bus->vi0_w(BIT(offset, 3) ? CLEAR_LINE : ASSERT_LINE);
	if (!BIT(intr, 1))
		m_bus->vi1_w(BIT(offset, 3) ? CLEAR_LINE : ASSERT_LINE);
	if (!BIT(intr, 2))
		m_bus->vi2_w(BIT(offset, 3) ? CLEAR_LINE : ASSERT_LINE);
	if (!BIT(intr, 3))
		m_bus->vi3_w(BIT(offset, 3) ? CLEAR_LINE : ASSERT_LINE);
	if (!BIT(intr, 4))
		m_bus->vi4_w(BIT(offset, 3) ? CLEAR_LINE : ASSERT_LINE);
	if (!BIT(intr, 5))
		m_bus->vi5_w(BIT(offset, 3) ? CLEAR_LINE : ASSERT_LINE);
	if (!BIT(intr, 6))
		m_bus->vi6_w(BIT(offset, 3) ? CLEAR_LINE : ASSERT_LINE);
	if (!BIT(intr, 7))
		m_bus->vi7_w(BIT(offset, 3) ? CLEAR_LINE : ASSERT_LINE);
}

void s100_am310_device::z80_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x3800).rom().region("prom", 0);
	map(0x4000, 0x47ff).mirror(0x3000).ram(); // AB11 is actually decoded as well as AB10
	map(0x8000, 0x800f).mirror(0x3ff0).rw(m_dma, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0xc000, 0xc000).mirror(0x3fc7).w(m_statlatch, FUNC(generic_latch_8_device::write));
	map(0xc008, 0xc008).mirror(0x3fc7).r(m_cmdlatch, FUNC(generic_latch_8_device::read));
	map(0xc010, 0xc013).mirror(0x3fc0).r(m_pci[0], FUNC(scn_pci_device::read));
	map(0xc014, 0xc017).mirror(0x3fc0).w(m_pci[0], FUNC(scn_pci_device::write));
	map(0xc018, 0xc01b).mirror(0x3fc0).r(m_pci[1], FUNC(scn_pci_device::read));
	map(0xc01c, 0xc01f).mirror(0x3fc0).w(m_pci[1], FUNC(scn_pci_device::write));
	map(0xc020, 0xc023).mirror(0x3fc0).r(m_pci[2], FUNC(scn_pci_device::read));
	map(0xc024, 0xc027).mirror(0x3fc0).w(m_pci[2], FUNC(scn_pci_device::write));
	map(0xc028, 0xc02b).mirror(0x3fc0).r(m_pci[3], FUNC(scn_pci_device::read));
	map(0xc02c, 0xc02f).mirror(0x3fc0).w(m_pci[3], FUNC(scn_pci_device::write));
	map(0xc030, 0xc030).mirror(0x3fc7).select(8).rw(FUNC(s100_am310_device::int_r), FUNC(s100_am310_device::int_w));
}


static INPUT_PORTS_START(s100_am310)
	// The physical positions of the jumper blocks are not numbered on the PCB.

	PORT_START("ADR")
	PORT_DIPNAME(0x3f, 0x2c, "I/O Address Block") PORT_DIPLOCATION("ADR:6,4,2,1,5,3")
	PORT_DIPSETTING(0x00, "00-01")
	PORT_DIPSETTING(0x01, "04-05")
	PORT_DIPSETTING(0x02, "08-09")
	PORT_DIPSETTING(0x03, "0C-0D")
	PORT_DIPSETTING(0x04, "10-11")
	PORT_DIPSETTING(0x05, "14-15")
	PORT_DIPSETTING(0x06, "18-19")
	PORT_DIPSETTING(0x07, "1C-1D")
	PORT_DIPSETTING(0x08, "20-21")
	PORT_DIPSETTING(0x09, "24-25")
	PORT_DIPSETTING(0x0a, "28-29")
	PORT_DIPSETTING(0x0b, "2C-2D")
	PORT_DIPSETTING(0x0c, "30-31")
	PORT_DIPSETTING(0x0d, "34-35")
	PORT_DIPSETTING(0x0e, "38-39")
	PORT_DIPSETTING(0x0f, "3C-3D")
	PORT_DIPSETTING(0x10, "40-41")
	PORT_DIPSETTING(0x11, "44-45")
	PORT_DIPSETTING(0x12, "48-49")
	PORT_DIPSETTING(0x13, "4C-4D")
	PORT_DIPSETTING(0x14, "50-51")
	PORT_DIPSETTING(0x15, "54-55")
	PORT_DIPSETTING(0x16, "58-59")
	PORT_DIPSETTING(0x17, "5C-1D")
	PORT_DIPSETTING(0x18, "60-61")
	PORT_DIPSETTING(0x19, "64-65")
	PORT_DIPSETTING(0x1a, "68-69")
	PORT_DIPSETTING(0x1b, "6C-6D")
	PORT_DIPSETTING(0x1c, "70-71")
	PORT_DIPSETTING(0x1d, "74-75")
	PORT_DIPSETTING(0x1e, "78-79")
	PORT_DIPSETTING(0x1f, "7C-7D")
	PORT_DIPSETTING(0x20, "80-81")
	PORT_DIPSETTING(0x21, "84-85")
	PORT_DIPSETTING(0x22, "88-89")
	PORT_DIPSETTING(0x23, "8C-8D")
	PORT_DIPSETTING(0x24, "90-91")
	PORT_DIPSETTING(0x25, "94-95")
	PORT_DIPSETTING(0x26, "98-99")
	PORT_DIPSETTING(0x27, "9C-9D")
	PORT_DIPSETTING(0x28, "A0-A1")
	PORT_DIPSETTING(0x29, "A4-A5")
	PORT_DIPSETTING(0x2a, "A8-A9")
	PORT_DIPSETTING(0x2b, "AC-AD")
	PORT_DIPSETTING(0x2c, "B0-B1")
	PORT_DIPSETTING(0x2d, "B4-B5")
	PORT_DIPSETTING(0x2e, "B8-B9")
	PORT_DIPSETTING(0x2f, "BC-BD")
	PORT_DIPSETTING(0x30, "C0-C1")
	PORT_DIPSETTING(0x31, "C4-C5")
	PORT_DIPSETTING(0x32, "C8-C9")
	PORT_DIPSETTING(0x33, "CC-CD")
	PORT_DIPSETTING(0x34, "D0-D1")
	PORT_DIPSETTING(0x35, "D4-D5")
	PORT_DIPSETTING(0x36, "D8-D9")
	PORT_DIPSETTING(0x37, "DC-DD")
	PORT_DIPSETTING(0x38, "E0-E1")
	PORT_DIPSETTING(0x39, "E4-E5")
	PORT_DIPSETTING(0x3a, "E8-E9")
	PORT_DIPSETTING(0x3b, "EC-ED")
	PORT_DIPSETTING(0x3c, "F0-F1")
	PORT_DIPSETTING(0x3d, "F4-F5")
	PORT_DIPSETTING(0x3e, "F8-F9")
	PORT_DIPSETTING(0x3f, "FC-FD")

	PORT_START("INTR")
	PORT_DIPNAME(0xff, 0xfd, "Interrupt Line") PORT_DIPLOCATION("INTR:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(0xff, "None")
	PORT_DIPSETTING(0xfe, "VI0")
	PORT_DIPSETTING(0xfd, "VI1")
	PORT_DIPSETTING(0xfb, "VI2")
	PORT_DIPSETTING(0xf7, "VI3")
	PORT_DIPSETTING(0xef, "VI4")
	PORT_DIPSETTING(0xdf, "VI5")
	PORT_DIPSETTING(0xbf, "VI6")
	PORT_DIPSETTING(0x7f, "VI7")
INPUT_PORTS_END

ioport_constructor s100_am310_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(s100_am310);
}


void s100_am310_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_z80, 4_MHz_XTAL);
	m_z80->set_addrmap(AS_PROGRAM, &s100_am310_device::z80_map);

	input_merger_device &irq(INPUT_MERGER_ANY_HIGH(config, "irq")); // open collector
	irq.output_handler().set_inputline(m_z80, INPUT_LINE_IRQ0);

	AM9517A(config, m_dma, 4_MHz_XTAL);
	m_dma->out_hreq_callback().set_inputline(m_z80, INPUT_LINE_HALT);
	m_dma->out_hreq_callback().append(m_dma, FUNC(am9517a_device::hack_w));
	m_dma->in_memr_callback().set(FUNC(s100_am310_device::dma_memr));
	m_dma->out_memw_callback().set(FUNC(s100_am310_device::dma_memw));
	m_dma->in_ior_callback<0>().set(FUNC(s100_am310_device::dma_ior));
	m_dma->out_iow_callback<0>().set(FUNC(s100_am310_device::dma_iow));

	GENERIC_LATCH_8(config, m_dolatch); // U38
	GENERIC_LATCH_8(config, m_cmdlatch); // U39
	GENERIC_LATCH_8(config, m_dilatch); // U40
	GENERIC_LATCH_8(config, m_statlatch); // U41

	SCN2651(config, m_pci[0], 5.0688_MHz_XTAL); // U30
	m_pci[0]->rts_handler().set("ch1", FUNC(rs232_port_device::write_rts));
	m_pci[0]->dtr_handler().set("ch1", FUNC(rs232_port_device::write_dtr));
	m_pci[0]->txd_handler().set("ch1", FUNC(rs232_port_device::write_txd));
	m_pci[0]->txemt_dschg_handler().set("irq", FUNC(input_merger_device::in_w<0>));
	m_pci[0]->txrdy_handler().set("irq", FUNC(input_merger_device::in_w<1>));
	m_pci[0]->rxrdy_handler().set("irq", FUNC(input_merger_device::in_w<2>));

	SCN2651(config, m_pci[1], 5.0688_MHz_XTAL); // U28
	m_pci[1]->rts_handler().set("ch2", FUNC(rs232_port_device::write_rts));
	m_pci[1]->dtr_handler().set("ch2", FUNC(rs232_port_device::write_dtr));
	m_pci[1]->txd_handler().set("ch2", FUNC(rs232_port_device::write_txd));
	m_pci[1]->txemt_dschg_handler().set("irq", FUNC(input_merger_device::in_w<3>));
	m_pci[1]->txrdy_handler().set("irq", FUNC(input_merger_device::in_w<4>));
	m_pci[1]->rxrdy_handler().set("irq", FUNC(input_merger_device::in_w<5>));

	SCN2651(config, m_pci[2], 5.0688_MHz_XTAL); // U27
	m_pci[2]->rts_handler().set("ch3", FUNC(rs232_port_device::write_rts));
	m_pci[2]->dtr_handler().set("ch3", FUNC(rs232_port_device::write_dtr));
	m_pci[2]->txd_handler().set("ch3", FUNC(rs232_port_device::write_txd));
	m_pci[2]->txemt_dschg_handler().set("irq", FUNC(input_merger_device::in_w<6>));
	m_pci[2]->txrdy_handler().set("irq", FUNC(input_merger_device::in_w<7>));
	m_pci[2]->rxrdy_handler().set("irq", FUNC(input_merger_device::in_w<8>));

	SCN2651(config, m_pci[3], 5.0688_MHz_XTAL); // U29
	m_pci[3]->rts_handler().set("ch4", FUNC(rs232_port_device::write_rts));
	m_pci[3]->dtr_handler().set("ch4", FUNC(rs232_port_device::write_dtr));
	m_pci[3]->txd_handler().set("ch4", FUNC(rs232_port_device::write_txd));
	m_pci[3]->txemt_dschg_handler().set("irq", FUNC(input_merger_device::in_w<9>));
	m_pci[3]->txrdy_handler().set("irq", FUNC(input_merger_device::in_w<10>));
	m_pci[3]->rxrdy_handler().set("irq", FUNC(input_merger_device::in_w<11>));

	rs232_port_device &ch1(RS232_PORT(config, "ch1", default_rs232_devices, nullptr)); // J2
	ch1.rxd_handler().set(m_pci[0], FUNC(scn_pci_device::rxd_w));
	ch1.dcd_handler().set(m_pci[0], FUNC(scn_pci_device::dcd_w));
	ch1.dsr_handler().set(m_pci[0], FUNC(scn_pci_device::dsr_w));
	ch1.cts_handler().set(m_pci[0], FUNC(scn_pci_device::cts_w));

	rs232_port_device &ch2(RS232_PORT(config, "ch2", default_rs232_devices, nullptr)); // J3
	ch2.rxd_handler().set(m_pci[1], FUNC(scn_pci_device::rxd_w));
	ch2.dcd_handler().set(m_pci[1], FUNC(scn_pci_device::dcd_w));
	ch2.dsr_handler().set(m_pci[1], FUNC(scn_pci_device::dsr_w));
	ch2.cts_handler().set(m_pci[1], FUNC(scn_pci_device::cts_w));

	rs232_port_device &ch3(RS232_PORT(config, "ch3", default_rs232_devices, nullptr)); // J4
	ch3.rxd_handler().set(m_pci[2], FUNC(scn_pci_device::rxd_w));
	ch3.dcd_handler().set(m_pci[2], FUNC(scn_pci_device::dcd_w));
	ch3.dsr_handler().set(m_pci[2], FUNC(scn_pci_device::dsr_w));
	ch3.cts_handler().set(m_pci[2], FUNC(scn_pci_device::cts_w));

	rs232_port_device &ch4(RS232_PORT(config, "ch4", default_rs232_devices, nullptr)); // J5
	ch4.rxd_handler().set(m_pci[3], FUNC(scn_pci_device::rxd_w));
	ch4.dcd_handler().set(m_pci[3], FUNC(scn_pci_device::dcd_w));
	ch4.dsr_handler().set(m_pci[3], FUNC(scn_pci_device::dsr_w));
	ch4.cts_handler().set(m_pci[3], FUNC(scn_pci_device::cts_w));
}


ROM_START(s100_am310)
	ROM_REGION(0x800, "prom", 0)
	ROM_LOAD("311-02_a00.u33", 0x000, 0x800, CRC(1f771ec8) SHA1(20716814381eb751261ee0607f3efff0d5375bd5))
ROM_END

const tiny_rom_entry *s100_am310_device::device_rom_region() const
{
	return ROM_NAME(s100_am310);
}
