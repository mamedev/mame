// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BT Merlin M2105

    TODO:
    - speech implementation is not verified
    - modem

**********************************************************************/


#include "emu.h"
#include "m2105.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_M2105, electron_m2105_device, "electron_m2105", "Acorn M2105 Expansion")


//-------------------------------------------------
//  ROM( m2105 )
//-------------------------------------------------

ROM_START( m2105 )
	ROM_REGION(0x10000, "exp_rom", 0)
	ROM_DEFAULT_BIOS("350")

	ROM_SYSTEM_BIOS(0, "350", "V3.50 16/02/87")
	ROMX_LOAD("ic22-sm-35l-1.ic22", 0x0000, 0x4000, CRC(e8f8a639) SHA1(eb7fa1e884be9c072ae0c1e598507b802422127f), ROM_BIOS(0))
	ROMX_LOAD("ic23-sm-35l-1.ic23", 0x4000, 0x4000, CRC(b1bb1d83) SHA1(07ca3a93744519b8d03bbf1c3c3537c0a0a3c6fe), ROM_BIOS(0))
	ROMX_LOAD("sk01-pc-35l-1.ic24", 0x8000, 0x4000, CRC(54fd4c09) SHA1(9588296306581580ba223cf6bce4be61476f14c4), ROM_BIOS(0))
	ROMX_LOAD("sk02-pc-35l-1.ic24", 0xc000, 0x4000, CRC(c08de988) SHA1(86f2da5f8e9a5301ad40360e286f841f42e94a99), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "341", "V3.41 26/11/85")
	ROMX_LOAD("ic22-sm-34l-1.ic22", 0x0000, 0x4000, CRC(b514b15f) SHA1(a9c6c20b5a4f860b000511dde2f54497bcdd97b0), ROM_BIOS(1))
	ROMX_LOAD("ic23-sm-34l-1.ic23", 0x4000, 0x4000, CRC(18875889) SHA1(d1a7dd87c4d99869a1961becec5e9d567d8fad53), ROM_BIOS(1))
	ROMX_LOAD("sk01-pc-34l-1.ic24", 0x8000, 0x4000, CRC(a8796c9e) SHA1(29bc01b8f7617b252e4b243d13b1bbd3cd32cc3b), ROM_BIOS(1))
	ROMX_LOAD("sk02-pc-34l-1.ic24", 0xc000, 0x4000, CRC(fa74063c) SHA1(cdc31c606e69e7a6d221b7340a310d475d487fc9), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "207", "V2.07 14/03/85")
	ROMX_LOAD("ic22-sm-207l-1.ic22", 0x0000, 0x4000, CRC(0c431547) SHA1(13d2eab49b9c79f507b7dd8436d1e56cf43be412), ROM_BIOS(2))
	ROMX_LOAD("ic23-sm-207l-1.ic23", 0x4000, 0x4000, CRC(15044d49) SHA1(e75fe4321579a9027527a0e256050d1444b3fe82), ROM_BIOS(2))
	ROMX_LOAD("sk01-pc-207l-1.ic24", 0x8000, 0x4000, CRC(0850bcea) SHA1(270e7a31e69e1454cfb70ced23a50f5d97efe4d5), ROM_BIOS(2))
	ROMX_LOAD("sk02-pc-207l-1.ic24", 0xc000, 0x4000, CRC(d8b9143f) SHA1(4e132c7a6dae4caf7203139b51882706d508c449), ROM_BIOS(2))

	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
	ROM_LOAD("cm62024.bin", 0x3c000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_m2105_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::irq_w));

	MOS6522(config, m_via[0], DERIVED_CLOCK(1, 16));
	m_via[0]->readpa_handler().set("vsp", FUNC(tms5220_device::status_r));
	m_via[0]->writepa_handler().set("vsp", FUNC(tms5220_device::data_w));
	m_via[0]->writepb_handler().set("vsp", FUNC(tms5220_device::combined_rsq_wsq_w)).mask(0x03);
	//m_via[0]->writepb_handler().set().bit(5); SPK ENABLE
	//m_via[0]->writepb_handler().set().bit(6); SND ENABLE
	m_via[0]->cb1_handler().set(m_via[0], FUNC(via6522_device::write_pb4));
	m_via[0]->cb2_handler().set(m_via[1], FUNC(via6522_device::write_cb1));
	m_via[0]->irq_handler().set("irqs", FUNC(input_merger_device::in_w<0>));

	MOS6522(config, m_via[1], DERIVED_CLOCK(1, 16));
	m_via[1]->writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	//m_via[1]->writepb_handler().set().bit(1); // DORELAY
	m_via[1]->writepb_handler().append("modem", FUNC(rs232_port_device::write_dtr)).bit(2); // LSRELAY
	//m_via[1]->readpb_handler().set().bit(3); // RA16
	//m_via[1]->writepb_handler().set().bit(4); // DIALEN
	//m_via[1]->writepb_handler().set().bit(5); // PULSE TRAIN
	//m_via[1]->readpb_handler().set().bit(6); // RING
	m_via[1]->ca2_handler().set("centronics", FUNC(centronics_device::write_strobe));
	m_via[1]->irq_handler().set("irqs", FUNC(input_merger_device::in_w<1>));

	SCN2681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->irq_cb().set("irqs", FUNC(input_merger_device::in_w<2>));
	m_duart->a_tx_cb().set("rs232", FUNC(rs232_port_device::write_txd));
	m_duart->b_tx_cb().set("modem", FUNC(rs232_port_device::write_txd));
	m_duart->outport_cb().set("rs232", FUNC(rs232_port_device::write_rts)).bit(0);
	m_duart->outport_cb().append("modem", FUNC(rs232_port_device::write_rts)).bit(1);
	m_duart->outport_cb().append("rs232", FUNC(rs232_port_device::write_dtr)).bit(2);
	m_duart->outport_cb().append("modem", FUNC(rs232_port_device::write_rts)).bit(5);

	rs232_port_device &modem(RS232_PORT(config, "modem", default_rs232_devices, "null_modem")); // Am7910
	modem.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_b_w));
	modem.cts_handler().set(m_duart, FUNC(scn2681_device::ip1_w));
	modem.dcd_handler().set(m_duart, FUNC(scn2681_device::ip3_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));
	rs232.cts_handler().set(m_duart, FUNC(scn2681_device::ip0_w));
	rs232.dsr_handler().set(m_duart, FUNC(scn2681_device::ip5_w));
	rs232.dcd_handler().set(m_duart, FUNC(scn2681_device::ip6_w));

	centronics_device &centronics(CENTRONICS(config, "centronics", centronics_devices, "printer"));
	centronics.ack_handler().set(m_via[1], FUNC(via6522_device::write_ca1));
	centronics.busy_handler().set(m_via[1], FUNC(via6522_device::write_pb0));
	centronics.select_handler().set(m_via[1], FUNC(via6522_device::write_pb7));
	centronics.select_handler().append(m_via[1], FUNC(via6522_device::write_cb2));

	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(latch);

	tms5220_device &tms(TMS5220(config, "vsp", 640000));
	tms.ready_cb().set(m_via[0], FUNC(via6522_device::write_ca1));
	tms.ready_cb().append(m_via[0], FUNC(via6522_device::write_pb2));
	tms.irq_cb().set(m_via[0], FUNC(via6522_device::write_ca2));
	tms.irq_cb().append(m_via[0], FUNC(via6522_device::write_pb3));
	tms.add_route(ALL_OUTPUTS, "mono", 0.5);

	TMS6100(config, "vsm", 0);
	tms.m0_cb().set("vsm", FUNC(tms6100_device::m0_w));
	tms.m1_cb().set("vsm", FUNC(tms6100_device::m1_w));
	tms.addr_cb().set("vsm", FUNC(tms6100_device::add_w));
	tms.data_cb().set("vsm", FUNC(tms6100_device::data_line_r));
	tms.romclk_cb().set("vsm", FUNC(tms6100_device::clk_w));

	SPEAKER(config, "mono").front_center();
}

const tiny_rom_entry *electron_m2105_device::device_rom_region() const
{
	return ROM_NAME( m2105 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_m2105_device - constructor
//-------------------------------------------------

electron_m2105_device::electron_m2105_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_M2105, tag, owner, clock)
	, device_electron_expansion_interface(mconfig, *this)
	, m_exp_rom(*this, "exp_rom")
	, m_nvram(*this, "nvram")
	, m_via(*this, "via%u", 0U)
	, m_duart(*this, "duart")
	, m_ram_page(0)
	, m_romsel(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_m2105_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x10000);
	m_nvram->set_base(m_ram.get(), 0x10000);

	save_item(NAME(m_ram_page));
	save_pointer(NAME(m_ram), 0x10000);
}

//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

uint8_t electron_m2105_device::expbus_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset >> 12)
	{
	case 0x8:
	case 0x9:
	case 0xa:
	case 0xb:
		switch (m_romsel)
		{
		 case 0: case 12:
			data = m_exp_rom->base()[0x0000 | (offset & 0x3fff)];
			break;
		case 1: case 13:
			data = m_exp_rom->base()[0x4000 | (offset & 0x3fff)];
			break;
		case 2: case 14:
			data = m_exp_rom->base()[0x8000 | (offset & 0x3fff)];
			break;
		case 3: case 15:
			data = m_exp_rom->base()[0xc000 | (offset & 0x3fff)];
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			switch (offset & 0xf0)
			{
			case 0x50:
				data = m_duart->read(offset & 0x0f);
				break;
			case 0x60:
				data = m_via[0]->read(offset & 0x0f);
				break;
			case 0x70:
				data = m_via[1]->read(offset & 0x0f);
				break;
			}
			break;

		case 0xfd:
			data = m_ram[(m_ram_page << 8) | (offset & 0xff)];
			break;
		}
	}

	return data;
}

//-------------------------------------------------
//  expbus_w - expansion data write
//-------------------------------------------------

void electron_m2105_device::expbus_w(offs_t offset, uint8_t data)
{
	switch (offset >> 12)
	{
	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			switch (offset & 0xf0)
			{
			case 0x50:
				m_duart->write(offset & 0x0f, data);
				break;
			case 0x60:
				m_via[0]->write(offset & 0x0f, data);
				break;
			case 0x70:
				m_via[1]->write(offset & 0x0f, data);
				break;
			case 0xf0:
				m_ram_page = data;
				break;
			}
			break;

		case 0xfd:
			m_ram[(m_ram_page << 8) | (offset & 0xff)] = data;
			break;

		case 0xfe:
			if ((offset == 0xfe05) && !(data & 0xf0))
			{
				m_romsel = data & 0x0f;
			}
			break;
		}
	}
}
