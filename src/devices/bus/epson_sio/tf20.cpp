// license:GPL-2.0+
// copyright-holders:Dirk Best
/**********************************************************************

    EPSON TF-20

    Dual 5.25" floppy drive with HX-20 factory option

    Status: Needs testing.

    http://fjkraan.home.xs4all.nl/comp/tf20/index.html

**********************************************************************/

#include "emu.h"
#include "tf20.h"

#define XTAL_CR1    XTAL(8'000'000)
#define XTAL_CR2    XTAL(4'915'200)


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(EPSON_TF20, epson_tf20_device, "epson_tf20", "EPSON TF-20 Dual Floppy Disk Drive")

//-------------------------------------------------
//  address maps
//-------------------------------------------------

void epson_tf20_device::cpu_mem(address_map &map)
{
	map(0x0000, 0x7fff).bankrw("bank1");
	map(0x8000, 0xffff).bankrw("bank2");
}

void epson_tf20_device::cpu_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xf0, 0xf3).rw(m_mpsc, FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w));
	map(0xf6, 0xf6).r(FUNC(epson_tf20_device::rom_disable_r));
	map(0xf7, 0xf7).portr("tf20_dip");
	map(0xf8, 0xf8).rw(FUNC(epson_tf20_device::upd765_tc_r), FUNC(epson_tf20_device::fdc_control_w));
	map(0xfa, 0xfb).m("5a", FUNC(upd765a_device::map));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( tf20 )
	ROM_REGION(0x0800, "rom", 0)
	ROM_LOAD("tfx.15e", 0x0000, 0x0800, CRC(af34f084) SHA1(c9bdf393f757ba5d8f838108ceb2b079be1d616e))
ROM_END

const tiny_rom_entry *epson_tf20_device::device_rom_region() const
{
	return ROM_NAME( tf20 );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

INPUT_PORTS_START( tf20 )
	PORT_START("tf20_dip")
	PORT_DIPNAME(0x0f, 0x00, "Drive extension")
	PORT_DIPLOCATION("TF-20:8,7,6,5")
	PORT_DIPSETTING(0x00, "A & B Drive")
	PORT_DIPSETTING(0x01, "C & D Drive")
INPUT_PORTS_END

ioport_constructor epson_tf20_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( tf20 );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

static void tf20_floppies(device_slot_interface &device)
{
	device.option_add("sd320", EPSON_SD_320);
}

void epson_tf20_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_cpu, XTAL_CR1 / 2); /* uPD780C */
	m_cpu->set_addrmap(AS_PROGRAM, &epson_tf20_device::cpu_mem);
	m_cpu->set_addrmap(AS_IO, &epson_tf20_device::cpu_io);
	m_cpu->set_irq_acknowledge_callback(FUNC(epson_tf20_device::irq_callback));

	// 64k internal ram
	RAM(config, "ram").set_default_size("64K");

	// upd7201 serial interface
	UPD7201(config, m_mpsc, XTAL_CR1 / 2);
	m_mpsc->out_txda_callback().set(FUNC(epson_tf20_device::txda_w));
	m_mpsc->out_dtra_callback().set(FUNC(epson_tf20_device::dtra_w));

	// floppy disk controller
	UPD765A(config, m_fdc, XTAL_CR1, true, true);
	m_fdc->intrq_wr_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);

	// floppy drives
	for (auto &fd : m_fd)
		FLOPPY_CONNECTOR(config, fd, tf20_floppies, "sd320", floppy_image_device::default_mfm_floppy_formats);

	// serial interface to another device
	EPSON_SIO(config, m_sio_output, nullptr);
	m_sio_output->rx_callback().set(DEVICE_SELF, FUNC(epson_tf20_device::rxc_w));
	m_sio_output->pin_callback().set(DEVICE_SELF, FUNC(epson_tf20_device::pinc_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  epson_tf20_device - constructor
//-------------------------------------------------

epson_tf20_device::epson_tf20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EPSON_TF20, tag, owner, clock),
	device_epson_sio_interface(mconfig, *this),
	m_cpu(*this, "19b"),
	m_ram(*this, "ram"),
	m_fdc(*this, "5a"),
	m_mpsc(*this, "3a"),
	m_sio_output(*this, "sio"),
	m_fd(*this, "5a:%u", 0U),
	m_timer_serial(nullptr), m_timer_tc(nullptr),
	m_rxc(1), m_txda(0), m_dtra(0), m_pinc(0)
{
	m_sio_input = dynamic_cast<epson_sio_device *>(owner);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_tf20_device::device_start()
{
	// make sure the ram device is already running
	if (!m_ram->started())
		throw device_missing_dependencies();

	m_timer_serial = timer_alloc(0);
	m_timer_tc = timer_alloc(1);

	// enable second half of ram
	m_cpu->space(AS_PROGRAM).install_ram(0x8000, 0xffff, m_ram->pointer() + 0x8000);

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_tf20_device::device_reset()
{
	// init timers
	m_timer_serial->adjust(attotime::zero, 0, attotime::from_hz(XTAL_CR2 / 8));
	m_timer_tc->adjust(attotime::never);

	m_mpsc->rxa_w(1);
	m_mpsc->rxb_w(1);

	// enable rom
	m_cpu->space(AS_PROGRAM).install_rom(0x0000, 0x07ff, 0x7800, memregion("rom")->base());
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void epson_tf20_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case 0:
		m_mpsc->rxca_w(1);
		m_mpsc->rxca_w(0);
		m_mpsc->txca_w(1);
		m_mpsc->txca_w(0);
		m_mpsc->rxcb_w(1);
		m_mpsc->rxcb_w(0);
		m_mpsc->txcb_w(1);
		m_mpsc->txcb_w(0);
		break;

	case 1:
		logerror("%s: tc off\n", tag());
		m_fdc->tc_w(false);
		break;
	}
}


//**************************************************************************
//  CPU & MEMORY
//**************************************************************************

//-------------------------------------------------
//  irq vector callback
//-------------------------------------------------

IRQ_CALLBACK_MEMBER( epson_tf20_device::irq_callback )
{
	return 0x00;
}

// a read from this location disables the rom
uint8_t epson_tf20_device::rom_disable_r()
{
	// switch in ram
	m_cpu->space(AS_PROGRAM).install_ram(0x0000, 0x7fff, m_ram->pointer());
	return 0xff;
}


//**************************************************************************
//  FLOPPY DISK CONTROLLER
//**************************************************************************

//-------------------------------------------------
//  fdc interrupt
//-------------------------------------------------

uint8_t epson_tf20_device::upd765_tc_r()
{
	logerror("%s: upd765_tc_r\n", machine().describe_context());

	// toggle tc on read
	m_fdc->tc_w(true);
	m_timer_tc->adjust(attotime::zero);

	return 0xff;
}

void epson_tf20_device::fdc_control_w(uint8_t data)
{
	logerror("%s: tf20_fdc_control_w(%02x)\n", machine().describe_context(), data);

	// bit 0, motor on signal
	for (auto &fd : m_fd)
		if (fd->get_device() != nullptr)
			fd->get_device()->mon_w(!BIT(data, 0));
}


//**************************************************************************
//  SIO INTERFACE
//**************************************************************************

//-------------------------------------------------
//  rxc_w - rx input
//-------------------------------------------------

WRITE_LINE_MEMBER( epson_tf20_device::rxc_w )
{
	m_rxc = state;
	m_sio_input->rx_w(m_txda && m_rxc);
}

//-------------------------------------------------
//  pinc_w - pin input
//-------------------------------------------------

WRITE_LINE_MEMBER( epson_tf20_device::pinc_w )
{
	m_pinc = state;
	m_sio_input->pin_w(!m_dtra || m_pinc);
}

//-------------------------------------------------
//  txda_w - rx output
//-------------------------------------------------

WRITE_LINE_MEMBER( epson_tf20_device::txda_w )
{
	m_txda = state;
	m_sio_input->rx_w(m_txda && m_rxc);
}

//-------------------------------------------------
//  dtra_w - pin output
//-------------------------------------------------

WRITE_LINE_MEMBER( epson_tf20_device::dtra_w )
{
	m_dtra = state;
	m_sio_input->pin_w(!m_dtra || m_pinc);
}

//-------------------------------------------------
//  tx_w - tx input
//-------------------------------------------------

void epson_tf20_device::tx_w(int level)
{
	m_mpsc->rxa_w(level);
	m_sio_output->tx_w(level);
}

//-------------------------------------------------
//  pout_w - pout input
//-------------------------------------------------

void epson_tf20_device::pout_w(int level)
{
	m_mpsc->ctsa_w(!level);
	m_sio_output->pout_w(level);
}
