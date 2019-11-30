// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Tangerine TANEX (MT002 Iss2)

    http://www.microtan.ukpc.net/pageProducts.html#TANEX

    This emulates the Tanex board with additional H2 and E2 expansion
    boards. The expansion boards are pre-populated with commonly used
    language and toolkit ROMs that can be selected in Machine
    Configuration.
    Alternatively, the expansion boards can be overridden and any
    selection of ROMs can be inserted into five ROM sockets

**********************************************************************/


#include "emu.h"
#include "tanex.h"
#include "speaker.h"

//#define VERBOSE 1
#include "logmacro.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS_TANEX, tanbus_tanex_device, "tanbus_tanex", "Tangerine Tanex Board")

//-------------------------------------------------
//  INPUT_PORTS( tanex )
//-------------------------------------------------

static INPUT_PORTS_START(tanex)
	PORT_START("CONFIG")
	PORT_CONFNAME(0x0f, 0x00, "H2 Extension Eprom Board")
	PORT_CONFSETTING(0x00, "Microsoft BASIC [GE2ED]")
	PORT_CONFSETTING(0x01, "Microtanic FORTH [GC000]")
	PORT_CONFSETTING(0x02, "2-Pass Assembler [GC000]")
	PORT_CONFNAME(0xf0, 0x00, "E2 Extension Eprom Board")
	PORT_CONFSETTING(0x00, "Toolkit")
	PORT_CONFSETTING(0x10, "HRG Toolkit (TANHRG)")
	PORT_CONFSETTING(0x20, "Video 80/82 Toolkit (VID8082)")
	PORT_CONFSETTING(0x30, "PGM Toolkit (TUGPGM)")

	PORT_START("JOY") // VIA #1 PORT A
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_4WAY
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_4WAY
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_4WAY
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_4WAY
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor tanbus_tanex_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tanex);
}

//-------------------------------------------------
//  ROM( tanex )
//-------------------------------------------------

ROM_START(tanex)
	ROM_REGION(0x3800, "rom_tanex", 0)
	/* tanex - default configuration */
	ROM_LOAD("basl.j2",     0x0000, 0x1000, CRC(3e09d384) SHA1(15a98941a672ff16242cc73f1dcf1d81fccd8910))
	ROM_LOAD("basm.h2",     0x1000, 0x1000, CRC(75105113) SHA1(c6fea4d65b7c52f43aa1589cace9467349a0f290))
	ROM_LOAD("bash.d3",     0x2000, 0x0800, CRC(ee6e8412) SHA1(7e1bca84bab79d94a4ab8554d23e2bc28ccd0384))
	ROM_LOAD("toolkit.e2",  0x2800, 0x0800, CRC(bd87fd34) SHA1(f41895df4a733dddfaf1c89ecff5040addcab804))
	ROM_LOAD("xbug.g2",     0x3000, 0x0800, CRC(4a875dda) SHA1(9f2626e09e7604ae6e8aa55c4f3a8ace3667348b))

	/* eprom extension board - h2 */
	ROM_REGION(0x5000, "rom_h2", 0)
	ROM_LOAD("basl.j2",     0x0000, 0x1000, CRC(3e09d384) SHA1(15a98941a672ff16242cc73f1dcf1d81fccd8910))
	ROM_LOAD("basm.h2",     0x1000, 0x1000, CRC(75105113) SHA1(c6fea4d65b7c52f43aa1589cace9467349a0f290))
	ROM_LOAD("fforth.j2",   0x2000, 0x1000, CRC(18c8e389) SHA1(3201c90d8183e49b8ca683dcc383278b31683297))
	ROM_LOAD("fforth.h2",   0x3000, 0x1000, CRC(b1869543) SHA1(b82900f40de52beb5a4ea12abb8eaaf55665cf01))
	ROM_LOAD("assem12.j2",  0x4000, 0x1000, CRC(5d0cf5ec) SHA1(8832964eb647b5bf3fe954e52218ed27cf885667))

	/* eprom extension board - e2 */
	ROM_REGION(0x2000, "rom_e2", 0)
	ROM_LOAD("toolkit.e2",  0x0000, 0x0800, CRC(bd87fd34) SHA1(f41895df4a733dddfaf1c89ecff5040addcab804))
	ROM_LOAD("hrgtkt.e2",   0x0800, 0x0800, CRC(592c13de) SHA1(544c2a3d58f07dfaa90f022f059129d6b8b659d6))
	ROM_LOAD("vdutkt.e2",   0x1000, 0x0800, CRC(068cbb9e) SHA1(38d99c23966b69440c8498c342a1486741f4d817))
	ROM_LOAD("pgmtkt.e2",   0x1800, 0x0800, CRC(340f6ddd) SHA1(93d205ffb07a339f801b9aa9469e069de321c41c))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tanbus_tanex_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, m_irq_line).output_handler().set(FUNC(tanbus_tanex_device::bus_irq_w));

	/* acia */
	MOS6551(config, m_acia, 0);
	m_acia->set_xtal(1.8432_MHz_XTAL);
	m_acia->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_acia->dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_acia->irq_handler().set(m_irq_line, FUNC(input_merger_device::in_w<IRQ_ACIA>));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	m_rs232->dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
	m_rs232->dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	m_rs232->cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));

	/* via */
	VIA6522(config, m_via6522[0], DERIVED_CLOCK(1, 8));
	m_via6522[0]->readpa_handler().set(FUNC(tanbus_tanex_device::via_0_in_a));
	m_via6522[0]->writepa_handler().set(FUNC(tanbus_tanex_device::via_0_out_a));
	m_via6522[0]->writepb_handler().set(FUNC(tanbus_tanex_device::via_0_out_b));
	m_via6522[0]->ca2_handler().set(FUNC(tanbus_tanex_device::via_0_out_ca2));
	m_via6522[0]->cb2_handler().set(FUNC(tanbus_tanex_device::via_0_out_cb2));
	m_via6522[0]->irq_handler().set(m_irq_line, FUNC(input_merger_device::in_w<IRQ_VIA_0>));

	VIA6522(config, m_via6522[1], DERIVED_CLOCK(1, 8));
	m_via6522[1]->writepa_handler().set(FUNC(tanbus_tanex_device::via_1_out_a));
	m_via6522[1]->writepb_handler().set(FUNC(tanbus_tanex_device::via_1_out_b));
	m_via6522[1]->ca2_handler().set(FUNC(tanbus_tanex_device::via_1_out_ca2));
	m_via6522[1]->cb2_handler().set(FUNC(tanbus_tanex_device::via_1_out_cb2));
	m_via6522[1]->irq_handler().set(m_irq_line, FUNC(input_merger_device::in_w<IRQ_VIA_1>));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	/* cassette */
	CASSETTE(config, m_cassette);
	m_cassette->add_route(ALL_OUTPUTS, "speaker", 0.05);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->set_interface("mt65_cass");
	TIMER(config, "read_cassette").configure_periodic(FUNC(tanbus_tanex_device::read_cassette), attotime::from_hz(20000));

	/* 4K sockets */
	GENERIC_SOCKET(config, m_rom[0], generic_linear_slot, "rom_c000", "bin,rom");
	GENERIC_SOCKET(config, m_rom[1], generic_linear_slot, "rom_d000", "bin,rom");

	/* 2K sockets */
	GENERIC_SOCKET(config, m_rom[2], generic_linear_slot, "rom_e000", "bin,rom");
	GENERIC_SOCKET(config, m_rom[3], generic_linear_slot, "rom_e800", "bin,rom");
	GENERIC_SOCKET(config, m_rom[4], generic_linear_slot, "rom_f000", "bin,rom");
}

const tiny_rom_entry *tanbus_tanex_device::device_rom_region() const
{
	return ROM_NAME(tanex);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_tanex_device - constructor
//-------------------------------------------------

tanbus_tanex_device::tanbus_tanex_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS_TANEX, tag, owner, clock)
	, device_tanbus_interface(mconfig, *this)
	, m_rom_tanex(*this, "rom_tanex")
	, m_rom_h2(*this, "rom_h2")
	, m_rom_e2(*this, "rom_e2")
	, m_rom(*this, "rom%u", 0)
	, m_cassette(*this, "cassette")
	, m_acia(*this, "acia")
	, m_rs232(*this, "rs232")
	, m_via6522(*this, "via6522%u", 0)
	, m_irq_line(*this, "irq_line")
	, m_config(*this, "CONFIG")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_tanex_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x1c00);

	save_pointer(NAME(m_ram), 0x1c00);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tanbus_tanex_device::device_reset()
{
	m_via6522[0]->write_ca1(1);
	m_via6522[0]->write_ca2(1);

	m_via6522[0]->write_pb(0xff);
	m_via6522[0]->write_cb1(1);
	m_via6522[0]->write_cb2(1);

	m_via6522[1]->write_pa(0xff);
	m_via6522[1]->write_ca1(1);
	m_via6522[1]->write_ca2(1);

	m_via6522[1]->write_pb(0xff);
	m_via6522[1]->write_cb1(1);
	m_via6522[1]->write_cb2(1);
}

//-------------------------------------------------
//  read - card read
//-------------------------------------------------

uint8_t tanbus_tanex_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	uint8_t data = 0xff;

	/* 7K static ram */
	if ((offset >= 0x0400) && (offset < 0x2000))
	{
		data = m_ram[offset - 0x0400];
	}

	/* IO */
	switch (offset & 0xfff0)
	{
	case 0xbfc0:
		data = m_via6522[0]->read(offset & 0x0f);
		break;
	case 0xbfd0:
		data = m_acia->read(offset & 0x03);
		break;
	case 0xbfe0:
		data = m_via6522[1]->read(offset & 0x0f);
		break;
	}

	/* tanex default rom 0xc000-0xf7ff */
	if ((offset >= 0xc000) && (offset < 0xf800))
	{
		data = m_rom_tanex->base()[offset & 0x3fff];
	}

	/* H2 extension eprom board 0xc000-0xdfff */
	if ((offset & 0xe000) == 0xc000)
	{
		if (m_rom[0]->exists() || m_rom[1]->exists())
		{
			if ((offset & 0xf000) == 0xc000)
				data = m_rom[0]->read_rom(offset & 0x0fff);
			else
				data = m_rom[1]->read_rom(offset & 0x0fff);
		}
		else
		{
			switch (m_config->read() & 0x0f)
			{
			case 0x00:
				data = m_rom_h2->base()[offset & 0x1fff];
				break;
			case 0x01:
				data = m_rom_h2->base()[(offset & 0x1fff) + 0x2000];
				break;
			case 0x02:
				data = m_rom_h2->base()[(offset & 0x0fff) + 0x4000];
				break;
			}
		}
	}

	/* D3 0xe000-0e7ff */
	if ((offset & 0xf800) == 0xe000)
	{
		if (m_rom[2]->exists())
		{
			data = m_rom[2]->read_rom(offset & 0x07ff);
		}
	}

	/* E2 extension eprom board 0xe800-0efff */
	if ((offset & 0xf800) == 0xe800)
	{
		if (m_rom[3]->exists())
		{
			data = m_rom[3]->read_rom(offset & 0x07ff);
		}
		else
		{
			switch (m_config->read() & 0xf0)
			{
			case 0x00:
				data = m_rom_e2->base()[offset & 0x07ff];
				break;
			case 0x10:
				data = m_rom_e2->base()[(offset & 0x07ff) + 0x0800];
				break;
			case 0x20:
				data = m_rom_e2->base()[(offset & 0x07ff) + 0x1000];
				break;
			case 0x30:
				data = m_rom_e2->base()[(offset & 0x07ff) + 0x1800];
				break;
			}
		}
	}

	/* G2  0xf000-0f7ff */
	if ((offset & 0xf800) == 0xf000)
	{
		if (m_rom[4]->exists())
		{
			data = m_rom[4]->read_rom(offset & 0x07ff);
		}
	}

	return data;
}

//-------------------------------------------------
//  write - card write
//-------------------------------------------------

void tanbus_tanex_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	/* 7K static ram */
	if ((offset >= 0x0400) && (offset < 0x2000))
	{
		m_ram[offset - 0x0400] = data;
	}

	/* IO */
	switch (offset & 0xfff0)
	{
	case 0xbfc0:
		m_via6522[0]->write(offset & 0x0f, data);
		break;
	case 0xbfd0:
		m_acia->write(offset & 0x03, data);
		break;
	case 0xbfe0:
		m_via6522[1]->write(offset & 0x0f, data);
		break;
	}
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

TIMER_DEVICE_CALLBACK_MEMBER(tanbus_tanex_device::read_cassette)
{
	double level = m_cassette->input();

	LOG("read_cassette: %g\n", level);
	if (level < -0.07)
		m_via6522[0]->write_cb2(0);
	else if (level > +0.07)
		m_via6522[0]->write_cb2(1);
}

WRITE_LINE_MEMBER(tanbus_tanex_device::bus_so_w)
{
	m_tanbus->so_w(state);
}

WRITE_LINE_MEMBER(tanbus_tanex_device::bus_irq_w)
{
	m_tanbus->irq_w(state);
}

//**************************************************************
//  VIA callback functions for VIA #0
//**************************************************************

READ8_MEMBER(tanbus_tanex_device::via_0_in_a)
{
	int data = ioport("JOY")->read();
	LOG("via_0_in_a %02X\n", data);
	return data;
}

WRITE8_MEMBER(tanbus_tanex_device::via_0_out_a)
{
	LOG("via_0_out_a %02X\n", data);
}

WRITE8_MEMBER(tanbus_tanex_device::via_0_out_b)
{
	LOG("via_0_out_b %02X\n", data);
	/* bit #5 is the replay cassette drive */
	/* bit #6 is the record cassette drive */
	/* bit #7 is the cassette output signal */
	m_cassette->output(data & 0x80 ? +1.0 : -1.0);
}

WRITE_LINE_MEMBER(tanbus_tanex_device::via_0_out_ca2)
{
	LOG("via_0_out_ca2 %d\n", state);
}

WRITE_LINE_MEMBER(tanbus_tanex_device::via_0_out_cb2)
{
	LOG("via_0_out_cb2 %d\n", state);
}

//**************************************************************
//  VIA callback functions for VIA #1
//**************************************************************

WRITE8_MEMBER(tanbus_tanex_device::via_1_out_a)
{
	LOG("via_1_out_a %02X\n", data);
}

WRITE8_MEMBER(tanbus_tanex_device::via_1_out_b)
{
	LOG("via_1_out_b %02X\n", data);
}

WRITE_LINE_MEMBER(tanbus_tanex_device::via_1_out_ca2)
{
	LOG("via_1_out_ca2 %d\n", state);
}

WRITE_LINE_MEMBER(tanbus_tanex_device::via_1_out_cb2)
{
	LOG("via_1_out_cb2 %d\n", state);
}
