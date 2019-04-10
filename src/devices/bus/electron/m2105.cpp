// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BT Merlin M2105

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
	ROM_DEFAULT_BIOS("v350")

	ROM_SYSTEM_BIOS(0, "v350", "V3.50 16/02/87")
	ROMX_LOAD("ic22-sm-35l-1.ic22", 0x0000, 0x4000, CRC(e8f8a639) SHA1(eb7fa1e884be9c072ae0c1e598507b802422127f), ROM_BIOS(0))
	ROMX_LOAD("ic23-sm-35l-1.ic23", 0x4000, 0x4000, CRC(b1bb1d83) SHA1(07ca3a93744519b8d03bbf1c3c3537c0a0a3c6fe), ROM_BIOS(0))
	ROMX_LOAD("sk01-pc-35l-1.ic24", 0x8000, 0x4000, CRC(54fd4c09) SHA1(9588296306581580ba223cf6bce4be61476f14c4), ROM_BIOS(0))
	ROMX_LOAD("sk02-pc-35l-1.ic24", 0xc000, 0x4000, CRC(c08de988) SHA1(86f2da5f8e9a5301ad40360e286f841f42e94a99), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v341", "V3.41 26/11/85")
	ROMX_LOAD("ic22-sm-34l-1.ic22", 0x0000, 0x4000, CRC(b514b15f) SHA1(a9c6c20b5a4f860b000511dde2f54497bcdd97b0), ROM_BIOS(1))
	ROMX_LOAD("ic23-sm-34l-1.ic23", 0x4000, 0x4000, CRC(18875889) SHA1(d1a7dd87c4d99869a1961becec5e9d567d8fad53), ROM_BIOS(1))
	ROMX_LOAD("sk01-pc-34l-1.ic24", 0x8000, 0x4000, CRC(a8796c9e) SHA1(29bc01b8f7617b252e4b243d13b1bbd3cd32cc3b), ROM_BIOS(1))
	ROMX_LOAD("sk02-pc-34l-1.ic24", 0xc000, 0x4000, CRC(fa74063c) SHA1(cdc31c606e69e7a6d221b7340a310d475d487fc9), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "v207", "V2.07 14/03/85")
	ROMX_LOAD("ic22-sm-207l-1.ic22", 0x0000, 0x4000, CRC(0c431547) SHA1(13d2eab49b9c79f507b7dd8436d1e56cf43be412), ROM_BIOS(2))
	ROMX_LOAD("ic23-sm-207l-1.ic23", 0x4000, 0x4000, CRC(15044d49) SHA1(e75fe4321579a9027527a0e256050d1444b3fe82), ROM_BIOS(2))
	ROMX_LOAD("sk01-pc-207l-1.ic24", 0x8000, 0x4000, CRC(0850bcea) SHA1(270e7a31e69e1454cfb70ced23a50f5d97efe4d5), ROM_BIOS(2))
	ROMX_LOAD("sk02-pc-207l-1.ic24", 0xc000, 0x4000, CRC(d8b9143f) SHA1(4e132c7a6dae4caf7203139b51882706d508c449), ROM_BIOS(2))

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phroma.bin", 0x0000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_m2105_device::device_add_mconfig(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::irq_w));

	/* nvram */
	RAM(config, m_ram).set_default_size("64K");

	/* system via */
	VIA6522(config, m_via6522_0, DERIVED_CLOCK(1, 16));
	//m_via6522_0->readpa_handler().set(FUNC(electron_m2105_device::m2105_via_system_read_porta));
	m_via6522_0->readpb_handler().set(m_tms, FUNC(tms5220_device::status_r));
	//m_via6522_0->writepa_handler().set(FUNC(electron_m2105_device::m2105_via_system_write_porta));
	m_via6522_0->writepb_handler().set(m_tms, FUNC(tms5220_device::data_w));
	m_via6522_0->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	/* user via */
	VIA6522(config, m_via6522_1, DERIVED_CLOCK(1, 16));
	m_via6522_1->writepb_handler().set("cent_data_out", FUNC(output_latch_device::bus_w));
	m_via6522_1->ca2_handler().set(m_centronics, FUNC(centronics_device::write_strobe));
	m_via6522_1->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

		/* duart */
	SCN2681(config, m_duart, XTAL(3'686'400)); // TODO: confirm clock
	m_duart->irq_cb().set(m_irqs, FUNC(input_merger_device::in_w<2>));
	m_duart->a_tx_cb().set("rs232", FUNC(rs232_port_device::write_txd));
	//m_duart->outport_cb().set(FUNC(electron_m2105_device::sio_out_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));

	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_via6522_1, FUNC(via6522_device::write_ca1)).invert(); // ack seems to be inverted?
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);

	/* AM7910 modem */

	/* speech hardware */
	SPEECHROM(config, "vsm", 0);
	TMS5220(config, m_tms, 640000);
	m_tms->set_speechrom_tag("vsm");
	//m_tms->irq_handler().set(m_via6522_0, FUNC(via6522_device::write_cb1));
	//m_tms->readyq_handler().set(m_via6522_0, FUNC(via6522_device::write_cb2));
	m_tms->add_route(ALL_OUTPUTS, "mono", 1.0);
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
	, m_ram(*this, RAM_TAG)
	, m_via6522_0(*this, "via6522_0")
	, m_via6522_1(*this, "via6522_1")
	, m_duart(*this, "duart")
	, m_tms(*this, "tms5220")
	, m_centronics(*this, "centronics")
	, m_irqs(*this, "irqs")
	, m_ram_page(0)
	, m_romsel(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_m2105_device::device_start()
{
	save_item(NAME(m_ram_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void electron_m2105_device::device_reset()
{
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
		case 0:
			data = m_exp_rom->base()[0x8000 | (offset & 0x3fff)];
			break;
		case 2:
			data = m_exp_rom->base()[0xc000 | (offset & 0x3fff)];
			break;
		case 12:
			data = m_exp_rom->base()[0x0000 | (offset & 0x3fff)];
			break;
		case 13:
			data = m_exp_rom->base()[0x4000 | (offset & 0x3fff)];
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			logerror("read %04x\n", offset);
			if (offset >= 0xfc50 && offset < 0xfc60)
			{
				data = m_duart->read(offset & 0x0f);
			}
			else if (offset >= 0xfc60 && offset < 0xfc70)
			{
				data = m_via6522_1->read(offset & 0x0f);
			}
			else if (offset >= 0xfc70 && offset < 0xfc80)
			{
				data = m_via6522_0->read(offset & 0x0f);
			}
			break;

		case 0xfd:
			//if (m_ram_page < 0x80)
				data = m_ram->pointer()[(m_ram_page << 8) | (offset & 0xff)];
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
			logerror("write %04x %02x\n", offset, data);
			if (offset >= 0xfc50 && offset < 0xfc60)
			{
				m_duart->write(offset & 0x0f, data);
			}
			else if (offset >= 0xfc60 && offset < 0xfc70)
			{
				m_via6522_1->write(offset & 0x0f, data);
			}
			else if (offset >= 0xfc70 && offset < 0xfc80)
			{
				m_via6522_0->write(offset & 0x0f, data);
			}
			else if (offset == 0xfcff)
			{
				m_ram_page = data;
			}
			break;

		case 0xfd:
			//if (m_ram_page < 0x80)
				m_ram->pointer()[(m_ram_page << 8) | (offset & 0xff)] = data;
			break;

		case 0xfe:
			if (offset == 0xfe05)
			{
				m_romsel = data & 0x0f;
			}
			break;
		}
	}
}
