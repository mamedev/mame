// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Atari 1050 Dual Density Disk Drive

    This SIO peripheral device controls one 5¼" floppy disk drive. Two
    soft-sectored data formats are supported:
    * Single density: FM, 40 tracks, 18 sectors/track (88K bytes)
    * Double density: MFM, 40 tracks, 26 sectors/track (133K bytes;
      requires DOS3)

    As with the 810, the 1050 uses parallel port outputs to operate the
    drive mechanism's four-phase stepper motor at 288 RPM. The FDC's own
    stepping outputs are not used, and its index pulse is synthesized.

***************************************************************************/

#include "emu.h"
#include "atari1050.h"

#include "cpu/m6502/m6507.h"


// device type definition
DEFINE_DEVICE_TYPE(ATARI1050, atari1050_device, "atari1050", "Atari 1050 Dual Density Disk Drive")

atari1050_device::atari1050_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ATARI1050, tag, owner, clock)
	, device_a8sio_card_interface(mconfig, *this)
	, m_pia(*this, "pia")
	, m_fdc(*this, "fdc")
{
}

void atari1050_device::device_start()
{
}

void atari1050_device::step_w(u8 data)
{
	m_a8sio->data_in_w(BIT(data, 0));
}

void atari1050_device::mem_map(address_map &map)
{
	map(0x0000, 0x007f).mirror(0x0900).ram(); // MCM6810
	map(0x0080, 0x00ff).mirror(0x0900).m(m_pia, FUNC(mos6532_device::ram_map));
	map(0x0280, 0x029f).mirror(0x0960).m(m_pia, FUNC(mos6532_device::io_map));
	map(0x0400, 0x0403).mirror(0x0bfc).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write));
	map(0x1000, 0x1fff).rom().region("rom", 0);
}


void atari1050_device::data_out_w(int state)
{
	m_pia->pb_bit_w<6>(!state);
}

void atari1050_device::command_w(int state)
{
	m_pia->pb_bit_w<7>(!state);
}

void atari1050_device::ready_w(int state)
{
	m_pia->pb_bit_w<1>(!state);
}


static INPUT_PORTS_START(atari1050)
	PORT_START("SELECT")
	PORT_DIPNAME(0x03, 0x03, "Drive Code") PORT_DIPLOCATION("S2:1,2")
	PORT_DIPSETTING(0x03, "No. 1")
	PORT_DIPSETTING(0x01, "No. 2")
	PORT_DIPSETTING(0x00, "No. 3")
	PORT_DIPSETTING(0x02, "No. 4")
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor atari1050_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(atari1050);
}

void atari1050_device::device_add_mconfig(machine_config &config)
{
	m6507_device &fdcpu(M6507(config, "fdcpu", 4_MHz_XTAL / 4));
	fdcpu.set_addrmap(AS_PROGRAM, &atari1050_device::mem_map);

	MOS6532(config, m_pia, 4_MHz_XTAL / 4);
	m_pia->pa_rd_callback().set_ioport("SELECT");
	m_pia->pa_wr_callback().set(m_fdc, FUNC(wd2793_device::dden_w)).bit(3);
	//m_pia->pa_wr_callback().append(atari1050_device::motor_control_w)).bit(1);
	m_pia->pb_wr_callback().set(FUNC(atari1050_device::step_w));
	m_pia->irq_wr_callback().set(m_pia, FUNC(mos6532_device::pa_bit_w<6>)).invert();
	//m_pia->irq_wr_callback().append(m_fdc, FUNC(wd2793_device::ip_w));

	WD2793(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->drq_wr_callback().set(m_pia, FUNC(mos6532_device::pa_bit_w<7>));
	m_fdc->enp_wr_callback().set(m_pia, FUNC(mos6532_device::pa_bit_w<4>));
}


ROM_START(atari1050)
	ROM_REGION(0x1000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "l", "Rev. L")
	ROMX_LOAD("1050-revl.rom", 0x0000, 0x1000, CRC(fb4b8757) SHA1(8b540906049864a18bdc4eff6c0a2160eac8ce39), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "k", "Rev. K")
	ROMX_LOAD("1050-revk.rom", 0x0000, 0x1000, CRC(3abe7ef4) SHA1(f281b64a4d8bd91127776b631313785c6886d963), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "j", "Rev. J")
	ROMX_LOAD("1050-revj.rom", 0x0000, 0x1000, CRC(91ba303d) SHA1(82f103e9026359587eb4ee2239e36d9cb291e3c9), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "h", "Rev. H")
	ROMX_LOAD("1050-revh.rom", 0x0000, 0x1000, CRC(6d9d589b) SHA1(fe0b289adf746d00dd3af780f363377f75eb584f), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "wst", "World Storage Technology (Rev. K modified)")
	ROMX_LOAD("wstr5.bin", 0x0000, 0x1000, CRC(dffb4b47) SHA1(2e735fe1973000219dc81c463ca0d3fd90690d4a), ROM_BIOS(4))
ROM_END

const tiny_rom_entry *atari1050_device::device_rom_region() const
{
	return ROM_NAME(atari1050);
}
