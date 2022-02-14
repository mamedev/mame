// license:GPL-2.0+
// copyright-holders:Dirk Best
/**********************************************************************

    EPSON PF-10

    Battery operated portable 3.5" floppy drive

    Status: Needs lots of work.

**********************************************************************/

#include "emu.h"
#include "pf10.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(EPSON_PF10, epson_pf10_device, "epson_pf10", "EPSON PF-10 Portable Floppy Unit")


//-------------------------------------------------
//  address maps
//-------------------------------------------------

void epson_pf10_device::cpu_mem(address_map &map)
{
	map(0x0000, 0x001f).m("maincpu", FUNC(hd6303y_cpu_device::m6801_io));
	map(0x0040, 0x00ff).ram(); /* 192 bytes internal ram */
	map(0x0800, 0x0fff).ram(); /* external 2k ram */
	map(0x1000, 0x17ff).rw(FUNC(epson_pf10_device::fdc_r), FUNC(epson_pf10_device::fdc_w));
	map(0x1800, 0x1fff).w(FUNC(epson_pf10_device::fdc_tc_w));
	map(0xe000, 0xffff).rom().region("maincpu", 0);
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( pf10 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("k3pf1.bin", 0x0000, 0x2000, CRC(eef4593a) SHA1(bb176e4baf938fe58c2d32f7c46d7bb7b0627755))
ROM_END

const tiny_rom_entry *epson_pf10_device::device_rom_region() const
{
	return ROM_NAME( pf10 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

static void pf10_floppies(device_slot_interface &device)
{
	device.option_add("smd165", EPSON_SMD_165);
}

void epson_pf10_device::device_add_mconfig(machine_config &config)
{
	HD6303Y(config, m_cpu, XTAL(4'915'200)); // HD63A03XF
	m_cpu->set_addrmap(AS_PROGRAM, &epson_pf10_device::cpu_mem);
	m_cpu->in_p1_cb().set(FUNC(epson_pf10_device::port1_r));
	m_cpu->out_p1_cb().set(FUNC(epson_pf10_device::port1_w));
	m_cpu->in_p2_cb().set(FUNC(epson_pf10_device::port2_r));
	m_cpu->out_p2_cb().set(FUNC(epson_pf10_device::port2_w));
	m_cpu->out_ser_tx_cb().set(FUNC(epson_pf10_device::hd6303_tx_w));

	UPD765A(config, m_fdc, 4'000'000, false, true);
	FLOPPY_CONNECTOR(config, m_floppy, pf10_floppies, "smd165", floppy_image_device::default_mfm_floppy_formats);

	EPSON_SIO(config, m_sio_output, nullptr);
	m_sio_output->rx_callback().set(DEVICE_SELF, FUNC(epson_pf10_device::rxc_w));
	m_sio_output->pin_callback().set(DEVICE_SELF, FUNC(epson_pf10_device::pinc_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  epson_pf10_device - constructor
//-------------------------------------------------

epson_pf10_device::epson_pf10_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EPSON_PF10, tag, owner, clock),
	device_epson_sio_interface(mconfig, *this),
	m_cpu(*this, "maincpu"),
	m_fdc(*this, "upd765a"),
	m_sio_output(*this, "sio"),
	m_floppy(*this, "upd765a:0"),
	m_timer(nullptr),
	m_port1(0xff),
	m_port2(0xff),
	m_rxc(1), m_hd6303_tx(0), m_pinc(0)
{
	m_sio_input = dynamic_cast<epson_sio_device *>(owner);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_pf10_device::device_start()
{
	m_timer = timer_alloc(0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_pf10_device::device_reset()
{
	m_timer->adjust(attotime::zero, 0, attotime::from_hz(38400 * 16));
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void epson_pf10_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case 0:
		m_cpu->m6801_clock_serial();
		break;
	}
}


//**************************************************************************
//  CPU
//**************************************************************************

uint8_t epson_pf10_device::port1_r()
{
	logerror("%s: port1_r(%02x)\n", tag(), m_port1);
	return m_port1;
}

void epson_pf10_device::port1_w(uint8_t data)
{
	logerror("%s: port1_w(%02x)\n", tag(), data);
}

uint8_t epson_pf10_device::port2_r()
{
	logerror("%s: port2_r(%02x)\n", tag(), m_port2);
	return m_port2;
}

void epson_pf10_device::port2_w(uint8_t data)
{
	if (m_floppy->get_device() != nullptr)
		m_floppy->get_device()->mon_w(data & PORT2_MON);
	logerror("%s: port2_w(%02x)\n", tag(), data);
}

uint8_t epson_pf10_device::fdc_r(offs_t offset)
{
	logerror("%s: fdc_r @ %04x\n", tag(), offset);
	return 0xff;
}

void epson_pf10_device::fdc_w(offs_t offset, uint8_t data)
{
	logerror("%s: fdc_w @ %04x (%02x)\n", tag(), offset, data);
}

void epson_pf10_device::fdc_tc_w(uint8_t data)
{
	logerror("%s: fdc_tc_w(%02x)\n", tag(), data);
}


//**************************************************************************
//  SIO INTERFACE
//**************************************************************************

//-------------------------------------------------
//  rxc_w - rx input
//-------------------------------------------------

WRITE_LINE_MEMBER( epson_pf10_device::rxc_w )
{
	m_rxc = state;
	m_sio_input->rx_w(m_hd6303_tx & m_rxc);
}

//-------------------------------------------------
//  pinc_w - pin input
//-------------------------------------------------

WRITE_LINE_MEMBER( epson_pf10_device::pinc_w )
{
	m_pinc = state;
	m_sio_input->pin_w(m_pinc);
}

//-------------------------------------------------
//  hd6303_tx_w - rx output
//-------------------------------------------------

WRITE_LINE_MEMBER( epson_pf10_device::hd6303_tx_w )
{
	m_hd6303_tx = state;
	m_sio_input->rx_w(m_hd6303_tx & m_rxc);
}

//-------------------------------------------------
//  tx_w - tx input
//-------------------------------------------------

void epson_pf10_device::tx_w(int level)
{
	if (level)
		m_port2 |= PORT2_RXD;
	else
		m_port2 &= ~PORT2_RXD;

	m_sio_output->tx_w(level);
}

//-------------------------------------------------
//  pout_w - pout input
//-------------------------------------------------

void epson_pf10_device::pout_w(int level)
{
	m_sio_output->pout_w(level);
}
