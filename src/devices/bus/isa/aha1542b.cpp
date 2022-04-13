// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Adaptec AHA-1540/42A and AHA-1540/42B SCSI controllers

    The alternate BIOSes using port 334h instead of 330h are provided due
    to certain MIDI cards requiring the 330h port.

***************************************************************************/

#include "emu.h"
#include "aha1542b.h"

#include "bus/nscsi/devices.h"
#include "cpu/i8085/i8085.h"
#include "machine/aic580.h"
#include "machine/gen_latch.h"
#include "machine/nscsi_bus.h"


DEFINE_DEVICE_TYPE(AHA1542A, aha1542a_device, "aha1542a", "AHA-1542A SCSI Controller")
DEFINE_DEVICE_TYPE(AHA1542B, aha1542b_device, "aha1542b", "AHA-1542B SCSI Controller")


aha154x_device::aha154x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_localcpu(*this, "localcpu")
	, m_scsic(*this, "scsi:7:scsic")
	, m_fdc(*this, "fdc")
	, m_bios(*this, "bios")
{
}

aha1542a_device::aha1542a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: aha154x_device(mconfig, AHA1542A, tag, owner, clock)
{
}

aha1542b_device::aha1542b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: aha154x_device(mconfig, AHA1542B, tag, owner, clock)
	, m_busaic(*this, "busaic")
{
}

void aha154x_device::device_start()
{
}

void aha154x_device::device_reset()
{
}

void aha154x_device::i8085_base_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("mcode", 0);
	map(0x8000, 0x800f).m(m_scsic, FUNC(aic6250_device::map));
	map(0xc000, 0xc0ff).m("dmaaic", FUNC(aic580_device::mpu_map));
	map(0xe000, 0xe7ff).ram();
}

void aha1542a_device::local_status_w(u8 data)
{
	logerror("Setting local status = %02X\n", data);
}

void aha1542a_device::int_status_w(u8 data)
{
	logerror("Setting interrupt status = %02X\n", data);
}

void aha1542a_device::srst_clear_w(u8 data)
{
	m_localcpu->set_input_line(I8085_TRAP_LINE, CLEAR_LINE);
}

void aha1542a_device::scsi_rstreq_clear_w(u8 data)
{
}

READ_LINE_MEMBER(aha1542a_device::host_int_r)
{
	return 0;
}

READ_LINE_MEMBER(aha1542a_device::scsi_rstreq_r)
{
	return 0;
}

void aha1542a_device::i8085_map(address_map &map)
{
	i8085_base_map(map);
	map(0xa000, 0xa000).w(FUNC(aha1542a_device::local_status_w));
	map(0xa001, 0xa001).r("fromhost", FUNC(generic_latch_8_device::read));
	map(0xa001, 0xa001).w("tohost", FUNC(generic_latch_8_device::write));
	map(0xa002, 0xa002).w(FUNC(aha1542a_device::int_status_w));
	map(0xa003, 0xa003).w(FUNC(aha1542a_device::srst_clear_w));
	map(0xa004, 0xa004).w(FUNC(aha1542a_device::scsi_rstreq_clear_w));
}

void aha1542b_device::i8085_map(address_map &map)
{
	i8085_base_map(map);
	map(0xa000, 0xa003).rw(m_busaic, FUNC(aic565_device::local_r), FUNC(aic565_device::local_w));
}

static INPUT_PORTS_START(aha1542a)
	PORT_START("SETUP")
	PORT_DIPNAME(0x01, 0x01, "Synchronous Negotiation") PORT_DIPLOCATION("J1:1")
	PORT_DIPSETTING(0x01, "Disabled")
	PORT_DIPSETTING(0x00, "Enabled")
	PORT_DIPNAME(0x02, 0x02, "Diagnostic Test Loop") PORT_DIPLOCATION("J1:2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("tohost", generic_latch_8_device, pending_r)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, aha1542a_device, host_int_r)
	PORT_DIPNAME(0x10, 0x10, "SCSI Parity Checking") PORT_DIPLOCATION("J1:3")
	PORT_DIPSETTING(0x00, "Disabled")
	PORT_DIPSETTING(0x10, "Enabled")
	PORT_DIPNAME(0x60, 0x60, "DMA Transfer Speed") PORT_DIPLOCATION("J1:12,13")
	PORT_DIPSETTING(0x60, "5.0 MB/s")
	PORT_DIPSETTING(0x40, "5.7 MB/s")
	PORT_DIPSETTING(0x20, "6.7 MB/s")
	PORT_DIPSETTING(0x00, "8.0 MB/s")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, aha1542a_device, scsi_rstreq_r)

	PORT_START("CONFIG")
	PORT_DIPNAME(0x07, 0x07, "SCSI Address ID") PORT_DIPLOCATION("J1:4,5,6")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x01, "1")
	PORT_DIPSETTING(0x02, "2")
	PORT_DIPSETTING(0x03, "3")
	PORT_DIPSETTING(0x04, "4")
	PORT_DIPSETTING(0x05, "5")
	PORT_DIPSETTING(0x06, "6")
	PORT_DIPSETTING(0x07, "7")
	PORT_DIPNAME(0x18, 0x08, "DMA Channel") PORT_DIPLOCATION("J1:7,8")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x08, "5")
	PORT_DIPSETTING(0x10, "6")
	PORT_DIPSETTING(0x18, "7")
	PORT_DIPNAME(0xe0, 0xa0, "Interrupt Channel") PORT_DIPLOCATION("J1:9,10,11")
	PORT_DIPSETTING(0xe0, "9")
	PORT_DIPSETTING(0xc0, "10")
	PORT_DIPSETTING(0xa0, "11")
	PORT_DIPSETTING(0x80, "12")
	PORT_DIPSETTING(0x60, "14")
	PORT_DIPSETTING(0x40, "15")

	PORT_START("DMAREQ")
	PORT_DIPNAME(0xf, 0xd, "DMA Request") PORT_DIPLOCATION("J14:1,2,3,4")
	PORT_DIPSETTING(0xe, "Channel 0")
	PORT_DIPSETTING(0xd, "Channel 5")
	PORT_DIPSETTING(0xb, "Channel 6")
	PORT_DIPSETTING(0x7, "Channel 7")

	PORT_START("DMAACK")
	PORT_DIPNAME(0xf, 0xd, "DMA Acknowledge") PORT_DIPLOCATION("J15:1,2,3,4")
	PORT_DIPSETTING(0xe, "Channel 0")
	PORT_DIPSETTING(0xd, "Channel 5")
	PORT_DIPSETTING(0xb, "Channel 6")
	PORT_DIPSETTING(0x7, "Channel 7")

	PORT_START("IRQ")
	PORT_DIPNAME(0x3f, 0x3b, "Interrupt") PORT_DIPLOCATION("J16:1,2,3,4,5,6")
	PORT_DIPSETTING(0x3e, "IRQ 9")
	PORT_DIPSETTING(0x3d, "IRQ 10")
	PORT_DIPSETTING(0x3b, "IRQ 11")
	PORT_DIPSETTING(0x37, "IRQ 12")
	PORT_DIPSETTING(0x2f, "IRQ 14")
	PORT_DIPSETTING(0x1f, "IRQ 15")

	PORT_START("BIOSWAIT")
	PORT_DIPNAME(0xf, 0xe, "BIOS Wait State") PORT_DIPLOCATION("J7:1,2,3,4")
	PORT_DIPSETTING(0xe, "Disabled")
	PORT_DIPSETTING(0xd, "100 ns")
	PORT_DIPSETTING(0xb, "200 ns")
	PORT_DIPSETTING(0x7, "300 ns")

	PORT_START("PORTADDR")
	PORT_DIPNAME(0x7, 0x6, "I/O Port Address") PORT_DIPLOCATION("J6:1,2,3")
	PORT_DIPSETTING(0x2, "130h")
	PORT_DIPSETTING(0x3, "134h")
	PORT_DIPSETTING(0x4, "230h")
	PORT_DIPSETTING(0x5, "234h")
	PORT_DIPSETTING(0x6, "330h")
	PORT_DIPSETTING(0x7, "334h")

	PORT_START("BIOSADDR")
	PORT_DIPNAME(0x3, 0x3, "BIOS Base Address") PORT_DIPLOCATION("J10:2,1")
	PORT_DIPSETTING(0x0, "0C8000h")
	PORT_DIPSETTING(0x1, "0CC000h")
	PORT_DIPSETTING(0x2, "0D8000h")
	PORT_DIPSETTING(0x3, "0DC000h")
	PORT_DIPNAME(0x4, 0x0, "System BIOS") PORT_DIPLOCATION("J10:3")
	PORT_DIPSETTING(0x4, "Disabled")
	PORT_DIPSETTING(0x0, "Enabled")

	PORT_START("AUX")
	PORT_DIPNAME(0x1, 0x1, DEF_STR(Unused)) PORT_DIPLOCATION("J9:1")
	PORT_DIPSETTING(0x1, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPNAME(0x2, 0x2, DEF_STR(Unused)) PORT_DIPLOCATION("J9:2")
	PORT_DIPSETTING(0x2, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPNAME(0x4, 0x4, DEF_STR(Unused)) PORT_DIPLOCATION("J9:3")
	PORT_DIPSETTING(0x4, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPNAME(0x8, 0x8, "Automatic Request Sense") PORT_DIPLOCATION("J9:4")
	PORT_DIPSETTING(0x0, "Disabled")
	PORT_DIPSETTING(0x8, "Enabled")

	PORT_START("FDCCFG")
	PORT_DIPNAME(0x1, 0x1, "FDC Secondary Address") PORT_DIPLOCATION("J12:1")
	PORT_DIPSETTING(0x0, "370h-377h")
	PORT_DIPSETTING(0x1, "3F0h-3F7h")
	PORT_DIPNAME(0x2, 0x0, "Flexible Disk Controller") PORT_DIPLOCATION("J13:1")
	PORT_DIPSETTING(0x2, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))

	PORT_START("FDCIRQ")
	PORT_DIPNAME(0x7, 0x4, "FDC Interrupt") PORT_DIPLOCATION("J17:1,2,3")
	PORT_DIPSETTING(0x4, "Channel 6")
	PORT_DIPSETTING(0x1, "Channel 10")

	PORT_START("FDCDREQ")
	PORT_DIPNAME(0x7, 0x4, "FDC DMA Request") PORT_DIPLOCATION("J18:1,2,3")
	PORT_DIPSETTING(0x4, "Channel 2")
	PORT_DIPSETTING(0x1, "Channel 3")

	PORT_START("FDCDACK")
	PORT_DIPNAME(0x7, 0x4, "FDC DMA Acknowledge") PORT_DIPLOCATION("J19:1,2,3")
	PORT_DIPSETTING(0x4, "Channel 2")
	PORT_DIPSETTING(0x1, "Channel 3")
INPUT_PORTS_END

static INPUT_PORTS_START(aha1542b)
	PORT_START("SETUP")
	PORT_DIPNAME(0x01, 0x01, "Synchronous Negotiation") PORT_DIPLOCATION("J5:1")
	PORT_DIPSETTING(0x01, "Disabled")
	PORT_DIPSETTING(0x00, "Enabled")
	PORT_DIPNAME(0x02, 0x02, "Diagnostic Test Loop") PORT_DIPLOCATION("J5:2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_DIPNAME(0x10, 0x10, "SCSI Parity Checking") PORT_DIPLOCATION("J5:3")
	PORT_DIPSETTING(0x00, "Disabled")
	PORT_DIPSETTING(0x10, "Enabled")
	PORT_DIPNAME(0x60, 0x60, "DMA Transfer Speed") PORT_DIPLOCATION("J5:12,13")
	PORT_DIPSETTING(0x60, "5.0 MB/s")
	PORT_DIPSETTING(0x40, "5.7 MB/s")
	PORT_DIPSETTING(0x20, "6.7 MB/s")
	PORT_DIPSETTING(0x00, "8.0 MB/s")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("CONFIG")
	PORT_DIPNAME(0x07, 0x07, "SCSI Address ID") PORT_DIPLOCATION("J5:4,5,6")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x01, "1")
	PORT_DIPSETTING(0x02, "2")
	PORT_DIPSETTING(0x03, "3")
	PORT_DIPSETTING(0x04, "4")
	PORT_DIPSETTING(0x05, "5")
	PORT_DIPSETTING(0x06, "6")
	PORT_DIPSETTING(0x07, "7")
	PORT_DIPNAME(0x18, 0x08, "DMA Channel") PORT_DIPLOCATION("J5:7,8")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x08, "5")
	PORT_DIPSETTING(0x10, "6")
	PORT_DIPSETTING(0x18, "7")
	PORT_DIPNAME(0xe0, 0xa0, "Interrupt Channel") PORT_DIPLOCATION("J5:9,10,11")
	PORT_DIPSETTING(0xe0, "9")
	PORT_DIPSETTING(0xc0, "10")
	PORT_DIPSETTING(0xa0, "11")
	PORT_DIPSETTING(0x80, "12")
	PORT_DIPSETTING(0x60, "14")
	PORT_DIPSETTING(0x40, "15")

	PORT_START("DMAREQ")
	PORT_DIPNAME(0xf, 0xd, "DMA Request") PORT_DIPLOCATION("J9:1,2,3,4")
	PORT_DIPSETTING(0xe, "Channel 0")
	PORT_DIPSETTING(0xd, "Channel 5")
	PORT_DIPSETTING(0xb, "Channel 6")
	PORT_DIPSETTING(0x7, "Channel 7")

	PORT_START("DMAACK")
	PORT_DIPNAME(0xf, 0xd, "DMA Acknowledge") PORT_DIPLOCATION("J9:5,6,7,8")
	PORT_DIPSETTING(0xe, "Channel 0")
	PORT_DIPSETTING(0xd, "Channel 5")
	PORT_DIPSETTING(0xb, "Channel 6")
	PORT_DIPSETTING(0x7, "Channel 7")

	PORT_START("IRQ")
	PORT_DIPNAME(0x3f, 0x3b, "Interrupt") PORT_DIPLOCATION("J9:9,10,11,12,13,14")
	PORT_DIPSETTING(0x3e, "IRQ 9")
	PORT_DIPSETTING(0x3d, "IRQ 10")
	PORT_DIPSETTING(0x3b, "IRQ 11")
	PORT_DIPSETTING(0x37, "IRQ 12")
	PORT_DIPSETTING(0x2f, "IRQ 14")
	PORT_DIPSETTING(0x1f, "IRQ 15")

	PORT_START("BIOSWAIT")
	PORT_DIPNAME(0x3, 0x3, "BIOS Wait State") PORT_DIPLOCATION("J7:5,6")
	PORT_DIPSETTING(0x3, "Disabled")
	PORT_DIPSETTING(0x2, "100 ns")
	PORT_DIPSETTING(0x1, "200 ns")
	PORT_DIPSETTING(0x0, "300 ns")

	PORT_START("PORTADDR")
	PORT_DIPNAME(0x7, 0x6, "I/O Port Address") PORT_DIPLOCATION("J7:2,3,4")
	PORT_DIPSETTING(0x2, "130h")
	PORT_DIPSETTING(0x3, "134h")
	PORT_DIPSETTING(0x4, "230h")
	PORT_DIPSETTING(0x5, "234h")
	PORT_DIPSETTING(0x6, "330h")
	PORT_DIPSETTING(0x7, "334h")

	PORT_START("BIOSADDR")
	PORT_DIPNAME(0x3, 0x3, "BIOS Base Address") PORT_DIPLOCATION("J7:8,7")
	PORT_DIPSETTING(0x0, "0C8000h")
	PORT_DIPSETTING(0x1, "0CC000h")
	PORT_DIPSETTING(0x2, "0D8000h")
	PORT_DIPSETTING(0x3, "0DC000h")
	PORT_DIPNAME(0x4, 0x0, "System BIOS") PORT_DIPLOCATION("J6:1")
	PORT_DIPSETTING(0x4, "Disabled")
	PORT_DIPSETTING(0x0, "Enabled")

	PORT_START("AUX")
	PORT_DIPNAME(0x1, 0x1, DEF_STR(Unused)) PORT_DIPLOCATION("J6:2")
	PORT_DIPSETTING(0x1, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPNAME(0x2, 0x2, DEF_STR(Unused)) PORT_DIPLOCATION("J6:3")
	PORT_DIPSETTING(0x2, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPNAME(0x4, 0x4, DEF_STR(Unused)) PORT_DIPLOCATION("J6:4")
	PORT_DIPSETTING(0x4, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPNAME(0x8, 0x8, "Automatic Request Sense") PORT_DIPLOCATION("J6:5")
	PORT_DIPSETTING(0x0, "Disabled")
	PORT_DIPSETTING(0x8, "Enabled")

	PORT_START("FDCCFG")
	PORT_DIPNAME(0x1, 0x1, "Floppy Secondary Address") PORT_DIPLOCATION("J7:1")
	PORT_DIPSETTING(0x1, "170h-177h")
	PORT_DIPSETTING(0x0, "1F0h-1F7h")
	PORT_DIPNAME(0x2, 0x0, "Floppy Disk Controller") PORT_DIPLOCATION("J8:1")
	PORT_DIPSETTING(0x2, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPNAME(0x4, 0x4, "Floppy Dual Channel Speed") PORT_DIPLOCATION("J8:8")
	PORT_DIPSETTING(0x4, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))

	PORT_START("FDCIRQ")
	PORT_DIPNAME(0x3, 0x2, "FDC Interrupt") PORT_DIPLOCATION("J8:6,7")
	PORT_DIPSETTING(0x2, "Channel 6")
	PORT_DIPSETTING(0x1, "Channel 10")

	PORT_START("FDCDREQ")
	PORT_DIPNAME(0x3, 0x2, "FDC DMA Request") PORT_DIPLOCATION("J8:2,3")
	PORT_DIPSETTING(0x2, "Channel 2")
	PORT_DIPSETTING(0x1, "Channel 3")

	PORT_START("FDCDACK")
	PORT_DIPNAME(0x3, 0x2, "FDC DMA Acknowledge") PORT_DIPLOCATION("J8:4,5")
	PORT_DIPSETTING(0x2, "Channel 2")
	PORT_DIPSETTING(0x1, "Channel 3")
INPUT_PORTS_END

ioport_constructor aha1542a_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(aha1542a);
}

ioport_constructor aha1542b_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(aha1542b);
}

void aha154x_device::scsic_config(device_t *device)
{
	device->set_clock(20_MHz_XTAL);
	downcast<aic6250_device &>(*device).int_cb().set_inputline("^^localcpu", I8085_RST65_LINE);
	downcast<aic6250_device &>(*device).breq_cb().set("^^dmaaic", FUNC(aic580_device::breq_w));
	downcast<aic6250_device &>(*device).port_a_r_cb().set_ioport("^^SETUP");
	downcast<aic6250_device &>(*device).port_b_r_cb().set_ioport("^^CONFIG");
}

void aha154x_device::scsi_add(machine_config &config)
{
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("scsic", AIC6250)
		.machine_config([this] (device_t *device) { scsic_config(device); });

	aic580_device &dmaaic(AIC580(config, "dmaaic", 20_MHz_XTAL));
	dmaaic.bdin_callback().set(m_scsic, FUNC(aic6250_device::dma_r));
	dmaaic.bdout_callback().set(m_scsic, FUNC(aic6250_device::dma_w));
	dmaaic.back_callback().set(m_scsic, FUNC(aic6250_device::back_w));
}

void aha1542a_device::device_add_mconfig(machine_config &config)
{
	i8085a_cpu_device &localcpu(I8085A(config, m_localcpu, 20_MHz_XTAL / 2));
	localcpu.set_addrmap(AS_PROGRAM, &aha1542a_device::i8085_map);

	generic_latch_8_device &fromhost(GENERIC_LATCH_8(config, "fromhost"));
	fromhost.data_pending_callback().set_inputline(m_localcpu, I8085_RST55_LINE);

	GENERIC_LATCH_8(config, "tohost");

	scsi_add(config);

	DP8473(config, m_fdc, 24_MHz_XTAL);
}

void aha1542b_device::device_add_mconfig(machine_config &config)
{
	i8085a_cpu_device &localcpu(I8085A(config, m_localcpu, 20_MHz_XTAL / 2));
	localcpu.set_addrmap(AS_PROGRAM, &aha1542b_device::i8085_map);

	AIC565(config, m_busaic);
	m_busaic->hrst_callback().set_inputline(m_localcpu, INPUT_LINE_RESET);
	// Soft reset interrupt is not used

	scsi_add(config);

	DP8473(config, m_fdc, 24_MHz_XTAL);
}


ROM_START(aha1542a)
	ROM_REGION(0x4000, "bios", 0)
	ROM_LOAD("b_9700.bin", 0x0000, 0x4000, CRC(35f546e9) SHA1(f559b08f52044f53836021a83f56f628e32216bd))

	ROM_REGION(0x4000, "mcode", 0)
	ROM_LOAD("m_e7bc.bin", 0x0000, 0x4000, CRC(985b7a31) SHA1(bba0d84fa1b67ea71905953c25201fa2020cf465))
ROM_END

ROM_START(aha1542b)
	ROM_REGION(0x4000, "bios", 0)
	//ROM_LOAD("adaptec_inc_420413-00_a_bios_bb00_1988.u13", 0x0000, 0x4000, NO_DUMP)
	ROM_SYSTEM_BIOS(0, "v310", "AT/SCSI BIOS Version 3.10")
	ROMX_LOAD("adaptec_inc_420412-00_b_bios_bc00_1990.u13", 0x0000, 0x4000, CRC(bd3f74e7) SHA1(c38d82fd50e5439812fa093e0d4f5fd136c63844), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v310a", "AT/SCSI BIOS Version 3.10A (port 334h)")
	ROMX_LOAD("154xp334.bin", 0x0000, 0x4000, CRC(4911f232) SHA1(2e24ce380c6f7694c45484019857cb919e2a9965), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v310b", "AT/SCSI BIOS Version 3.1b (port 334h)")
	ROMX_LOAD("154xp334_v31b.bin", 0x0000, 0x4000, CRC(c7115ccb) SHA1(e552efdbefb3c9b94649204a60eff25f0f5d8025), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v311", "AT/SCSI BIOS Version 3.11")
	ROMX_LOAD("adaptec_inc_420421-00_a_bios_c900_1990.u13", 0x0000, 0x4000, CRC(4660d0c1) SHA1(a581291de96836b6f6cc0b6244b8fa1ee333346a), ROM_BIOS(3)) // also provided with v310 firmware
	ROM_SYSTEM_BIOS(4, "v320g", "AT/SCSI BIOS Version 3.20 (> 1 GB support)")
	ROMX_LOAD("b_bd00.bin", 0x0000, 0x4000, CRC(2387197b) SHA1(703e1fe1ba924c02d617ac37ec7a20e12bef1cc7), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v320gt", "AT/SCSI BIOS Version 3.20 (extended timeout)")
	ROMX_LOAD("b_b300.bin", 0x0000, 0x4000, CRC(4c5b07d8) SHA1(692e824f916f55519c9905839f5f6609f5e8c0a5), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "v320a", "AT/SCSI BIOS Version 3.20Alt (port 334h)")
	ROMX_LOAD("b_ac00.bin", 0x0000, 0x4000, CRC(becd6d08) SHA1(b5e7cbdeb241c1ff57602291e87c58ac0ee72d54), ROM_BIOS(6))

	ROM_REGION(0x4000, "mcode", 0)
	ROMX_LOAD("adaptec_inc_434108-00_a_mcode_fc8a_1990.u12", 0x0000, 0x4000, CRC(6801f89e) SHA1(33d36bc93734105b950414e7c433a283032838e9), ROM_BIOS(0))
	ROMX_LOAD("adaptec_inc_434108-00_a_mcode_fc8a_1990.u12", 0x0000, 0x4000, CRC(6801f89e) SHA1(33d36bc93734105b950414e7c433a283032838e9), ROM_BIOS(1)) // assumed compatible with v310a BIOS
	ROMX_LOAD("adaptec_inc_434108-00_a_mcode_fc8a_1990.u12", 0x0000, 0x4000, CRC(6801f89e) SHA1(33d36bc93734105b950414e7c433a283032838e9), ROM_BIOS(2)) // assumed compatible with v310b BIOS
	ROMX_LOAD("firmware_62d3.u12", 0x0000, 0x4000, CRC(6056ca33) SHA1(8dd4aaffcb107dbcc85ac87d878fa6093b904a20), ROM_BIOS(3))
	ROMX_LOAD("m_3054.bin", 0x0000, 0x4000, CRC(461b1885) SHA1(50dc49b0fd88b116b83e3c71f58c758b618d1ddf), ROM_BIOS(4))
	ROMX_LOAD("m_5d98.bin", 0x0000, 0x4000, CRC(f7d51536) SHA1(5ad1bb4bde3e8c30380b05d32ac273c781ab12a8), ROM_BIOS(5)) // also provided with v320g BIOS
	ROMX_LOAD("m_3054.bin", 0x0000, 0x4000, CRC(461b1885) SHA1(50dc49b0fd88b116b83e3c71f58c758b618d1ddf), ROM_BIOS(6))
ROM_END

const tiny_rom_entry *aha1542a_device::device_rom_region() const
{
	return ROM_NAME(aha1542a);
}

const tiny_rom_entry *aha1542b_device::device_rom_region() const
{
	return ROM_NAME(aha1542b);
}
