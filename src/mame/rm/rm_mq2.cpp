// license:BSD-3-Clause
// copyright-holders:Robin Sergeant
/**********************************************************************

    Research Machines MQ2 dual floppy drive

An RS232 serial communication protocol is used for communcation between the MQ2 and 480Z.

Initial requests are sent to the drive at 9600 baud, and the drive's response to this upgrades
the connection to 125000 baud.  The RTS output line is used by the 480Z to reset the baud rate
back to 9600 baud prior to issuing a new request.  When this line is high an NMI is generated
to achieve this.

On first boot the 480Z requests disk handling code which is downloaded from the MQ2 and used
to supplement the 480Z firmware (hence most of the disk handling code is actually contained
in the MQ2 itself).

The RS232 cable is connected as follows:

480Z    MQ2
RxD <-- TxD
TxD --> RxD
DCD <-- RTS
RTS --> DCD
CTS <-- DTR
DTR --> CTS
GND --- GND

Interrupt mode 1 is used and only the FDC generates interrupts.

**********************************************************************/

#include "emu.h"
#include "rm_mq2.h"
#include "machine/clock.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(RM_MQ2, rmMQ2_device, "rm_mq2", "Research Machines MQ2")


//-------------------------------------------------
//  ROM( c1551 )
//-------------------------------------------------

ROM_START( rmMQ2 )
	ROM_REGION( 0x2000, "idc_rom", 0 )
	ROM_LOAD( "idc3-1i.rom",   0x0000, 0x2000, CRC(39e2cdf0) SHA1(ba523af357b61bbe6192727139850f36597d79f1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *rmMQ2_device::device_rom_region() const
{
	return ROM_NAME( rmMQ2 );
}

//-------------------------------------------------
//  ADDRESS_MAP( rmMQ2_mem )
//-------------------------------------------------

void rmMQ2_device::rmMQ2_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("idc_rom", 0);
	map(0xe000, 0xe7ff).mirror(0x1800).ram(); // 2K RAM mirrored at 0xe800, 0xf000 and 0xf800
}

//-------------------------------------------------
//  ADDRESS_MAP( rmMQ2_io )
//-------------------------------------------------

void rmMQ2_device::rmMQ2_io(address_map &map)
{
	map(0x00, 0x03).mirror(0xff00).r(FUNC(rmMQ2_device::fdc_read)).w(m_fdc, FUNC(fd1793_device::write));
	map(0x04, 0x04).mirror(0xff00).r(FUNC(rmMQ2_device::drive_status_r));
	map(0x07, 0x07).mirror(0xff00).r(FUNC(rmMQ2_device::status_r));
	map(0x0d, 0x0d).mirror(0xff00).w(FUNC(rmMQ2_device::port1_w));
	map(0x0e, 0x0e).mirror(0xff00).w(FUNC(rmMQ2_device::port0_w));
	map(0xe0, 0xe3).mirror(0xff00).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0xe4, 0xe7).mirror(0xff00).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

static void rmMQ2_floppies(device_slot_interface &device)
{
	device.option_add("525sd", FLOPPY_525_SD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525qd", FLOPPY_525_QD);
}

void rmMQ2_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &rmMQ2_device::rmMQ2_mem);
	m_maincpu->set_addrmap(AS_IO, &rmMQ2_device::rmMQ2_io);

	Z80SIO(config, m_sio, 8_MHz_XTAL / 2);
	m_sio->out_txdb_callback().set(FUNC(rmMQ2_device::output_rxd));
	m_sio->out_dtrb_callback().set(FUNC(rmMQ2_device::output_cts));
	m_sio->out_rtsb_callback().set(FUNC(rmMQ2_device::output_dcd));

	Z80CTC(config, m_ctc, 8_MHz_XTAL / 2);
	m_ctc->zc_callback<0>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));

	CLOCK(config, "ctc_clock", 8_MHz_XTAL / 4).signal_handler().set(m_ctc, FUNC(z80ctc_device::trg0));

	FD1793(config, m_fdc, 8_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set(FUNC(rmMQ2_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(rmMQ2_device::fdc_drq_w));
	FLOPPY_CONNECTOR(config, m_floppy0, rmMQ2_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy1, rmMQ2_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
}

//-------------------------------------------------
//  c1551_device - constructor
//-------------------------------------------------

rmMQ2_device::rmMQ2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, RM_MQ2, tag, owner, clock),
		device_rs232_port_interface(mconfig, *this),
		m_maincpu(*this, "maincpu"),
		m_ctc(*this, "ctc"),
		m_sio(*this, "sio"),
		m_fdc(*this, "wd1793"),
		m_floppy0(*this, "wd1793:0"),
		m_floppy1(*this, "wd1793:1")
{
}

void rmMQ2_device::device_start()
{
}

void rmMQ2_device::input_txd(int state)
{
	if (started())
	{
		m_sio->rxb_w(state);
	}
}

void rmMQ2_device::input_dtr(int state)
{
	if (started())
	{
		m_sio->ctsb_w(state);
	}
}

void rmMQ2_device::input_rts(int state)
{
	if (started())
	{
		m_sio->dcdb_w(state);
		// give firmware ~50ms to finish initialisation before allowing first NMI
		if (m_maincpu->total_cycles() > 200'000)
		{
			// NMI generated to reset baud rate to 9600
			m_maincpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
		}
	}
}

void rmMQ2_device::port0_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;
	const int drive_no = ~data & 0x0f;

	switch (drive_no)
	{
	case 1:
		floppy = m_floppy0->get_device();
		break;
	case 2:
		floppy = m_floppy1->get_device();
		break;
	default:
		// ignore as only two drives supported
		break;
	}

	if (floppy)
	{
		m_fdc->set_floppy(floppy);
		// motor should be controlled by bit 4, but it changes state too frequently so keep on instead
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 5));
	}
}

void rmMQ2_device::port1_w(uint8_t data)
{
	// bit 1 is set for DD (MFM encoding), and cleared for SD (FM encoding)
	m_fdc->dden_w(!BIT(data, 1));
	m_8inch_sel = BIT(data, 0);
}

uint8_t rmMQ2_device::status_r()
{
	if (!m_fdc->intrq_r() && !m_fdc->drq_r())
	{
		/* this port is read by the firmware to syncronize access to the FDC.  If
		   neither the DRQ or INTRQ lines are active it suspends the CPU. */
		m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);
	}
	return 0;
}

uint8_t rmMQ2_device::drive_status_r()
{
	// return high bits to indicate that the drives are ok
	return 0xff;
}

void rmMQ2_device::fdc_intrq_w(int state)
{
	if (state)
	{
		m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
	}
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state);
}

void rmMQ2_device::fdc_drq_w(int state)
{
	if (state)
	{
		m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
	}
}

uint8_t rmMQ2_device::fdc_read(offs_t offset)
{
	uint8_t data = m_fdc->read(offset);

	/* When the 8INCH line is high the Index Pulse FDC input is connected to pin 4 of the 
	   floppy connector, which is unused by 5.25" drives.  This fact is used by the firmware
	   to detect the drive type being used, i.e. it sets 8INCH high and checks to see if the IP
	   is received within a set time.  We must therefore ensure that the INDEX bit of the
	   FDC status byte remains low during this detection period to prevent 8" drive parameters
	   from being mistakenly used. */
	if ((offset == 0) && m_8inch_sel)
	{
		data &= 0xfd;
	}

	return data;
}
