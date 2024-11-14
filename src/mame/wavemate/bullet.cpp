// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

Wave Mate Bullet

PCB Layout
----------

|-------------------------------------------|
|                           CN7     CN6     |
|                                   PIO     |
|                           FDC             |
|       4164 4164                   CN5     |
|       4164 4164       SW1              CN2|
|       4164 4164                           |
|       4164 4164                   PROM    |
|       4164 4164   4.9152MHz               |
|       4164 4164   16MHz   DMA     CPU     |
|       4164 4164                           |
|       4164 4164           CN4 CN3      CN1|
|                                           |
|                           DART    CTC     |
|                                   CN8     |
|-------------------------------------------|

Notes:
    Relevant IC's shown.

    CPU     - SGS Z80ACPUB1 Z80A CPU
    DMA     - Zilog Z8410APS Z80A DMA
    PIO     - SGS Z80APIOB1 Z80A PIO
    DART    - Zilog Z8470APS Z80A DART
    CTC     - Zilog Z8430APS Z80A CTC
    FDC     - Synertek SY1793-02 FDC
    PROM    - AMD AM27S190C 32x8 TTL PROM
    4164    - Fujitsu MB8264-15 64Kx1 RAM
    CN1     - 2x25 PCB header, external DMA bus
    CN2     - 2x17 PCB header, Winchester / 2x25 header, SCSI (in board revision E)
    CN3     - 2x5 PCB header, RS-232 A (system console)
    CN4     - 2x5 PCB header, RS-232 B
    CN5     - 2x17 PCB header, Centronics
    CN6     - 2x25 PCB header, 8" floppy drives
    CN7     - 2x17 PCB header, 5.25" floppy drives
    CN8     - 4-pin Molex

*/

/*

    TODO:

    - memory banking is broken
    - z80dart wait/ready
    - IMI 7710 Winchester controller
        chdman createhd -o imi7710.chd -chs 350,3,10 -ss 1024
    - revision E model

*/

#include "emu.h"
#include "bullet.h"

#include "bus/rs232/rs232.h"
#include "bus/scsi/scsihd.h"
#include "softlist.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

// DMA ready sources
enum
{
	FDRDY = 0,
	DARTARDY,
	DARTBRDY,
	WINRDY,
	EXRDY1,
	EXRDY2
};

#define SEG0 \
	BIT(m_mbank, 0)

#define SEG5 \
	BIT(m_mbank, 5)

#define DMB4 \
	BIT(m_xdma0, 4)

#define DMB6 \
	BIT(m_xdma0, 6)



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  mreq_r -
//-------------------------------------------------

uint8_t bullet_state::mreq_r(offs_t offset)
{
	uint8_t data = 0;

	if (!m_brom && !BIT(offset, 5))
	{
		data = m_rom->base()[offset & 0x1f];
	}
	else
	{
		if (offset < 0xc000)
		{
			data = m_ram->pointer()[(m_segst << 16) | offset];
		}
		else
		{
			data = m_ram->pointer()[offset];
		}
	}

	return data;
}


//-------------------------------------------------
//  mreq_w -
//-------------------------------------------------

void bullet_state::mreq_w(offs_t offset, uint8_t data)
{
	if (offset < 0xc000)
	{
		m_ram->pointer()[(m_segst << 16) | offset] = data;
	}
	else
	{
		m_ram->pointer()[offset] = data;
	}
}


//-------------------------------------------------
//  win_r -
//-------------------------------------------------

uint8_t bullet_state::win_r()
{
	return 0;
}


//-------------------------------------------------
//  wstrobe_w -
//-------------------------------------------------

void bullet_state::wstrobe_w(uint8_t data)
{
}


//-------------------------------------------------
//  brom_r -
//-------------------------------------------------

uint8_t bullet_state::brom_r()
{
	m_brom = 1;

	return 0;
}


//-------------------------------------------------
//  brom_w -
//-------------------------------------------------

void bullet_state::brom_w(uint8_t data)
{
	m_brom = 1;
}


//-------------------------------------------------
//  exdsk_w -
//-------------------------------------------------

void bullet_state::exdsk_w(uint8_t data)
{
	/*

	    bit     signal      description

	    0                   drive select 0
	    1                   drive select 1
	    2                   select 8" floppy
	    3       MSE         select software control of port
	    4       SIDE        select side 2
	    5       _MOTOR      disable 5" floppy spindle motors
	    6
	    7       WPRC        enable write precompensation

	*/

	if (BIT(data, 3))
	{
		m_exdsk_sw = true;
	}

	if (m_exdsk_sw)
	{
		// drive select
		m_floppy = nullptr;

		switch (data & 0x07)
		{
		// 5.25"
		case 0: m_floppy = m_floppy0->get_device(); break;
		case 1: m_floppy = m_floppy1->get_device(); break;
		case 2: m_floppy = m_floppy2->get_device(); break;
		case 3: m_floppy = m_floppy3->get_device(); break;
		// 8"
		case 4: m_floppy = m_floppy4->get_device(); break;
		case 5: m_floppy = m_floppy5->get_device(); break;
		case 6: m_floppy = m_floppy6->get_device(); break;
		case 7: m_floppy = m_floppy7->get_device(); break;
		}

		m_fdc->set_floppy(m_floppy);
	}

	if (m_floppy)
	{
		// side select
		m_floppy->ss_w(BIT(data, 4));

		// floppy motor
		m_floppy->mon_w(BIT(data, 5));
	}
}


//-------------------------------------------------
//  exdma_w -
//-------------------------------------------------

void bullet_state::exdma_w(uint8_t data)
{
	/*

	    bit     description

	    0       DMA ready source select 0
	    1       DMA ready source select 1
	    2       DMA ready source select 2
	    3       memory control 0
	    4       memory control 1
	    5
	    6
	    7

	*/

	m_exdma = data;

	m_buf = BIT(data, 3);

	update_dma_rdy();
}


//-------------------------------------------------
//  hdcon_w -
//-------------------------------------------------

void bullet_state::hdcon_w(uint8_t data)
{
	/*

	    bit     signal  description

	    0       PLO     phase lock oscillator
	    1       RCD     read clock frequency
	    2       EXC     MB8877 clock frequency
	    3       DEN     MB8877 density select
	    4               enable software control of mode
	    5
	    6
	    7

	*/

	if (BIT(data, 4))
	{
		m_hdcon_sw = true;
	}

	if (m_hdcon_sw)
	{
		// FDC clock
		m_fdc->set_unscaled_clock(16_MHz_XTAL / (BIT(data, 2) ? 16 : 8));

		// density select
		m_fdc->dden_w(BIT(data, 3));
	}
}


//-------------------------------------------------
//  info_r -
//-------------------------------------------------

uint8_t bullet_state::info_r()
{
	/*

	    bit     signal      description

	    0       SW1         DIP switch 1
	    1       SW2         DIP switch 2
	    2       SW3         DIP switch 3
	    3       SW4         DIP switch 4
	    4       HLDST       floppy disk head load status
	    5       *XDCG       floppy disk exchange (8" only)
	    6       FDIRQ       FDC interrupt request line
	    7       FDDRQ       FDC data request line

	*/

	uint8_t data = 0;

	// DIP switches
	data |= m_sw1->read() & 0x0f;

	// floppy
	data |= m_fdc->hld_r() << 4;
	data |= (m_floppy ? m_floppy->dskchg_r() : 1) << 5;
	data |= m_fdc->intrq_r() << 6;
	data |= m_fdc->drq_r() << 7;

	return data;
}


//-------------------------------------------------
//  segst_w -
//-------------------------------------------------

void bullet_state::segst_w(uint8_t data)
{
	m_segst = BIT(data, 0);
}


//-------------------------------------------------
//  mreq_r -
//-------------------------------------------------

uint8_t bulletf_state::mreq_r(offs_t offset)
{
	uint8_t data = 0;

	if (!m_rome && !BIT(offset, 5))
	{
		data = m_rom->base()[offset & 0x1f];
	}
	else
	{
		if (offset < 0xc000)
		{
			if (!SEG5)
			{
				data = m_ram->pointer()[(SEG0 << 16) | offset];
			}
			else if (offset >= 0x8000)
			{
				data = m_ram->pointer()[0x1c000 + (offset - 0x8000)];
			}
		}
		else
		{
			data = m_ram->pointer()[offset];
		}
	}

	return data;
}


//-------------------------------------------------
//  mreq_w -
//-------------------------------------------------

void bulletf_state::mreq_w(offs_t offset, uint8_t data)
{
	if (offset < 0xc000)
	{
		if (!SEG5)
		{
			m_ram->pointer()[(SEG0 << 16) | offset] = data;
		}
		else if (offset >= 0x8000)
		{
			m_ram->pointer()[0x1c000 + (offset - 0x8000)] = data;
		}
	}
	else
	{
		m_ram->pointer()[offset] = data;
	}
}


//-------------------------------------------------
//  xdma0_w -
//-------------------------------------------------

void bulletf_state::xdma0_w(uint8_t data)
{
	/*

	    bit     signal

	    0       device select (0=FDC, 1=SCSI)
	    1
	    2
	    3
	    4       DMB4        Source bank
	    5
	    6       DMB6        Destination bank
	    7

	*/

	m_rome = 1;

	m_xdma0 = data;
}


//-------------------------------------------------
//  xfdc_w -
//-------------------------------------------------

void bulletf_state::xfdc_w(uint8_t data)
{
	/*

	    bit     signal

	    0       Unit select number
	    1       Unit select number
	    2       Unit select number
	    3       Unit select number
	    4       Select side 2
	    5       Disable 3 & 5 inch spindle motors
	    6       Set for 1 MHz controller operation, reset for 2 MHz controller operation
	    7       Set to select single density

	*/

	// drive select
	m_floppy = nullptr;

	switch (data & 0x0f)
	{
	// 5.25"
	case 0: m_floppy = m_floppy0->get_device(); break;
	case 1: m_floppy = m_floppy1->get_device(); break;
	case 2: m_floppy = m_floppy2->get_device(); break;
	case 3: m_floppy = m_floppy3->get_device(); break;
	// 8"
	case 4: m_floppy = m_floppy4->get_device(); break;
	case 5: m_floppy = m_floppy5->get_device(); break;
	case 6: m_floppy = m_floppy6->get_device(); break;
	case 7: m_floppy = m_floppy7->get_device(); break;
	// 3.5"
	case 8: m_floppy = m_floppy8->get_device(); break;
	case 9: m_floppy = m_floppy9->get_device(); break;
	}

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		// side select
		m_floppy->ss_w(BIT(data, 4));

		// floppy motor
		m_floppy->mon_w(BIT(data, 5));
	}

	// FDC clock
	m_fdc->set_unscaled_clock(16_MHz_XTAL / (BIT(data, 6) ? 16 : 8));

	// density select
	m_fdc->dden_w(BIT(data, 7));
}


//-------------------------------------------------
//  mbank_w -
//-------------------------------------------------

void bulletf_state::mbank_w(uint8_t data)
{
	/*

	    bit     signal

	    0       Select active bank (0=bank 0, 1=bank 1)
	    1
	    2
	    3
	    4       Select system space overlay (0=no overlay, 1=overlay)
	    5
	    6
	    7

	*/

	m_mbank = data;
}


//-------------------------------------------------
//  scsi_r -
//-------------------------------------------------

uint8_t bulletf_state::scsi_r()
{
	uint8_t data = m_scsi_data_in->read();

	m_scsibus->write_ack(1);

	m_wack = 0;
	update_dma_rdy();

	return data;
}


//-------------------------------------------------
//  scsi_w -
//-------------------------------------------------

void bulletf_state::scsi_w(uint8_t data)
{
	m_scsi_data_out->write(data);

	m_scsibus->write_ack(1);

	m_wack = 0;
	update_dma_rdy();
}

//-------------------------------------------------
//  hwsts_r -
//-------------------------------------------------

uint8_t bulletf_state::hwsts_r()
{
	/*

	    bit     signal

	    0       CBUSY   Centronics busy
	    1       SW2     DIP switch 2
	    2       SW3     DIP switch 3
	    3       FD2S    *Floppy disk two sided
	    4       HLDST   Floppy disk head load status
	    5       XDCG    *Floppy disk exchange (8-inch only)
	    6       FDIRQ   FDC interrupt request line
	    7       FDDRQ   FDC data request line

	*/

	uint8_t data = 0;

	// centronics busy
	data |= m_centronics_busy;

	// DIP switches
	data |= m_sw1->read() & 0x06;

	// floppy
	data |= (m_floppy ? m_floppy->twosid_r() : 1) << 3;
	data |= m_fdc->hld_r() << 4;
	data |= (m_floppy ? m_floppy->dskchg_r() : 1) << 5;
	data |= m_fdc->intrq_r() << 6;
	data |= m_fdc->drq_r() << 7;

	return data;
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( bullet_mem )
//-------------------------------------------------

void bullet_state::bullet_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(bullet_state::mreq_r), FUNC(bullet_state::mreq_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( bullet_io )
//-------------------------------------------------

void bullet_state::bullet_io(address_map &map)
{
	map.global_mask(0x1f);
	map(0x00, 0x03).rw(m_dart, FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x04, 0x07).rw(Z80PIO_TAG, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x08, 0x0b).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x0c, 0x0c).mirror(0x03).rw(FUNC(bullet_state::win_r), FUNC(bullet_state::wstrobe_w));
	map(0x10, 0x13).rw(m_fdc, FUNC(mb8877_device::read), FUNC(mb8877_device::write));
	map(0x14, 0x14).rw(m_dmac, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x15, 0x15).rw(FUNC(bullet_state::brom_r), FUNC(bullet_state::brom_w));
	map(0x16, 0x16).w(FUNC(bullet_state::exdsk_w));
	map(0x17, 0x17).w(FUNC(bullet_state::exdma_w));
	map(0x18, 0x18).w(FUNC(bullet_state::hdcon_w));
	map(0x19, 0x19).r(FUNC(bullet_state::info_r));
	map(0x1a, 0x1a).w(FUNC(bullet_state::segst_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( bulletf_mem )
//-------------------------------------------------

void bulletf_state::bulletf_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(bulletf_state::mreq_r), FUNC(bulletf_state::mreq_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( bulletf_io )
//-------------------------------------------------

void bulletf_state::bulletf_io(address_map &map)
{
	map.global_mask(0x3f);
	map(0x00, 0x03).rw(m_dart, FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x04, 0x07).rw(Z80PIO_TAG, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x08, 0x0b).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x10, 0x13).rw(m_fdc, FUNC(mb8877_device::read), FUNC(mb8877_device::write));
	map(0x14, 0x14).w(FUNC(bulletf_state::xdma0_w));
	map(0x16, 0x16).w(FUNC(bulletf_state::xfdc_w));
	map(0x17, 0x17).w(FUNC(bulletf_state::mbank_w));
	map(0x19, 0x19).rw(FUNC(bulletf_state::scsi_r), FUNC(bulletf_state::scsi_w));
	map(0x1a, 0x1a).rw(m_dmac, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x1b, 0x1b).r(FUNC(bulletf_state::hwsts_r));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( bullet )
//-------------------------------------------------

INPUT_PORTS_START( bullet )
	PORT_START("SW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPNAME( 0xf0, 0x50, "Floppy Type" ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0xf0, "5.25\" SD" )
	PORT_DIPSETTING(    0x50, "5.25\" DD" )
	PORT_DIPSETTING(    0x90, "8\" SD" )
	PORT_DIPSETTING(    0x00, "8\" DD" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( bulletf )
//-------------------------------------------------

INPUT_PORTS_START( bulletf )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "SCSI Bus Termination" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, "Boot ROM Device" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Onboard" )
	PORT_DIPSETTING(    0x08, "EPROM" )
	PORT_DIPNAME( 0xf0, 0xc0, "Floppy Type" ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x10, "3\" DD" )
	PORT_DIPSETTING(    0xc0, "5.25\" SD" )
	PORT_DIPSETTING(    0x40, "5.25\" DD" )
	PORT_DIPSETTING(    0xa0, "8\" SD" )
	PORT_DIPSETTING(    0x20, "8\" DD" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  Z80CTC
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(bullet_state::ctc_tick)
{
	m_ctc->trg0(1);
	m_ctc->trg0(0);

	m_ctc->trg1(1);
	m_ctc->trg1(0);

	m_ctc->trg2(1);
	m_ctc->trg2(0);
}

void bullet_state::dart_rxtxca_w(int state)
{
	m_dart->txca_w(state);
	m_dart->rxca_w(state);
}

//-------------------------------------------------
//  Z80DART
//-------------------------------------------------

void bullet_state::dartardy_w(int state)
{
	m_dartardy = state;
	update_dma_rdy();
}

void bullet_state::dartbrdy_w(int state)
{
	m_dartbrdy = state;
	update_dma_rdy();
}

//-------------------------------------------------
//  Z80DMA
//-------------------------------------------------

void bullet_state::update_dma_rdy()
{
	int rdy = 1;

	switch (m_exdma & 0x07)
	{
	case FDRDY:
		rdy = m_fdrdy;
		break;

	case DARTARDY:
		rdy = m_dartardy;
		break;

	case DARTBRDY:
		rdy = m_dartbrdy;
		break;

	case WINRDY:
		rdy = m_winrdy;
		break;

	case EXRDY1:
		rdy = m_exrdy1;
		break;

	case EXRDY2:
		rdy = m_exrdy2;
		break;
	}

	m_dmac->rdy_w(rdy);
}

uint8_t bullet_state::dma_mreq_r(offs_t offset)
{
	uint8_t data = m_ram->pointer()[(m_buf << 16) | offset];

	if (BIT(m_exdma, 4))
	{
		m_buf = !m_buf;
	}

	return data;
}

void bullet_state::dma_mreq_w(offs_t offset, uint8_t data)
{
	m_ram->pointer()[(m_buf << 16) | offset] = data;

	if (BIT(m_exdma, 4))
	{
		m_buf = !m_buf;
	}
}

uint8_t bullet_state::io_read_byte(offs_t offset)
{
	return m_maincpu->space(AS_IO).read_byte(offset);
}

void bullet_state::io_write_byte(offs_t offset, uint8_t data)
{
	m_maincpu->space(AS_IO).write_byte(offset, data);
}

//-------------------------------------------------
//  Z80DMA for bulletf (not used currently)
//-------------------------------------------------

void bulletf_state::update_dma_rdy()
{
	int rdy = 1;

	if (BIT(m_xdma0, 0))
	{
		rdy = m_wack || m_wrdy;
	}
	else
	{
		rdy = m_fdrdy;
	}

	m_dmac->rdy_w(rdy);
}

uint8_t bulletf_state::dma_mreq_r(offs_t offset)
{
	return m_ram->pointer()[(DMB4 << 16) | offset];
}

void bulletf_state::dma_mreq_w(offs_t offset, uint8_t data)
{
	m_ram->pointer()[(DMB6 << 16) | offset] = data;
}

//-------------------------------------------------
//  Z80PIO
//-------------------------------------------------

void bullet_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

void bullet_state::write_centronics_perror(int state)
{
	m_centronics_perror = state;
}

void bullet_state::write_centronics_select(int state)
{
	m_centronics_select = state;
}

void bullet_state::write_centronics_fault(int state)
{
	m_centronics_fault = state;
}

uint8_t bullet_state::pio_pb_r()
{
	/*

	    bit     signal      description

	    0                   centronics busy
	    1                   centronics paper end
	    2                   centronics selected
	    3       *FAULT      centronics fault
	    4                   external vector
	    5       WBUSDIR     winchester bus direction
	    6       WCOMPLETE   winchester command complete
	    7       *WINRDY     winchester ready

	*/

	uint8_t data = 0;

	// centronics
	data |= m_centronics_busy;
	data |= m_centronics_perror << 1;
	data |= m_centronics_select << 2;
	data |= m_centronics_fault << 3;

	return data;
}


void bulletf_state::pio_pa_w(uint8_t data)
{
	/*

	    bit     signal

	    0       ATN
	    1       RST
	    2       SEL
	    3       BUSY
	    4       MSG
	    5       C/D
	    6       REQ
	    7       I/O

	*/

	m_scsibus->write_atn(BIT(data, 0));
	m_scsibus->write_rst(BIT(data, 1));
	m_scsibus->write_sel(BIT(data, 2));
}

void bulletf_state::cstrb_w(int state)
{
	m_centronics->write_strobe(!state);
}

static void bullet_525_floppies(device_slot_interface &device)
{
	device.option_add("525sd", FLOPPY_525_SD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525qd", FLOPPY_525_QD);
}

static void bullet_8_floppies(device_slot_interface &device)
{
	device.option_add("8dssd", FLOPPY_8_DSSD);
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}

static void bullet_35_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

void bullet_state::fdc_drq_w(int state)
{
	m_fdrdy = !state;
	update_dma_rdy();
}

void bulletf_state::req_w(int state)
{
	if (!state)
	{
		m_scsibus->write_ack(0);

		m_wack = 1;
	}

	m_wrdy = !state;
	update_dma_rdy();

	m_scsi_ctrl_in->write_bit6(state);
}


static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  z80_daisy_config daisy_chain
//-------------------------------------------------

static const z80_daisy_config daisy_chain[] =
{
	{ Z80DMA_TAG },
	{ Z80DART_TAG },
	{ Z80PIO_TAG },
	{ Z80CTC_TAG },
	{ nullptr }
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( bullet )
//-------------------------------------------------

void bullet_state::machine_start()
{
	// state saving
	save_item(NAME(m_segst));
	save_item(NAME(m_brom));
	save_item(NAME(m_exdma));
	save_item(NAME(m_buf));
	save_item(NAME(m_fdrdy));
	save_item(NAME(m_dartardy));
	save_item(NAME(m_dartbrdy));
	save_item(NAME(m_winrdy));
	save_item(NAME(m_exrdy1));
	save_item(NAME(m_exrdy2));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_perror));
	save_item(NAME(m_centronics_select));
	save_item(NAME(m_centronics_fault));
}


//-------------------------------------------------
//  MACHINE_START( bulletf )
//-------------------------------------------------

void bulletf_state::machine_start()
{
	// state saving
	save_item(NAME(m_fdrdy));
	save_item(NAME(m_rome));
	save_item(NAME(m_xdma0));
	save_item(NAME(m_mbank));
	save_item(NAME(m_wack));
	save_item(NAME(m_wrdy));
	save_item(NAME(m_centronics_busy));
}


void bullet_state::machine_reset()
{
	// memory banking
	m_brom = 0;
	m_segst = 0;

	// DMA ready
	m_exdma = 0;
	m_buf = 0;
	update_dma_rdy();

	// disable software control
	m_exdsk_sw = false;
	m_hdcon_sw = false;

	uint8_t sw1 = m_sw1->read();
	int mini = BIT(sw1, 6);
	m_fdc->set_unscaled_clock(16_MHz_XTAL / (mini ? 16 : 8));
	m_fdc->dden_w(BIT(sw1, 7));

	if (mini)
	{
		m_floppy = m_floppy0->get_device();
	}
	else
	{
		m_floppy = m_floppy4->get_device();
	}

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		m_floppy->ss_w(0);
		m_floppy->mon_w(0);
	}
}


void bulletf_state::machine_reset()
{
	// memory banking
	m_rome = 0;
	m_mbank = 0;

	// DMA ready
	m_xdma0 = 0;
	m_wack = 0;
	m_wrdy = 0;
	update_dma_rdy();
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  machine_config( bullet )
//-------------------------------------------------

void bullet_state::bullet(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &bullet_state::bullet_mem);
	m_maincpu->set_addrmap(AS_IO, &bullet_state::bullet_io);
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->busack_cb().set(m_dmac, FUNC(z80dma_device::bai_w));

	// devices
	Z80CTC(config, m_ctc, 16_MHz_XTAL / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(FUNC(bullet_state::dart_rxtxca_w));
	m_ctc->zc_callback<1>().set(m_dart, FUNC(z80dart_device::rxtxcb_w));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));

	TIMER(config, "ctc").configure_periodic(FUNC(bullet_state::ctc_tick), attotime::from_hz(4.9152_MHz_XTAL / 4));

	Z80DART(config, m_dart, 16_MHz_XTAL / 4);
	m_dart->out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_dart->out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_dart->out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_dart->out_wrdya_callback().set(FUNC(bullet_state::dartardy_w));
	m_dart->out_txdb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_dart->out_dtrb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_dart->out_rtsb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));
	m_dart->out_wrdyb_callback().set(FUNC(bullet_state::dartbrdy_w));
	m_dart->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80DMA(config, m_dmac, 16_MHz_XTAL / 4);
	m_dmac->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dmac->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dmac->in_mreq_callback().set(FUNC(bullet_state::dma_mreq_r));
	m_dmac->out_mreq_callback().set(FUNC(bullet_state::dma_mreq_w));
	m_dmac->in_iorq_callback().set(FUNC(bullet_state::io_read_byte));
	m_dmac->out_iorq_callback().set(FUNC(bullet_state::io_write_byte));

	z80pio_device& pio(Z80PIO(config, Z80PIO_TAG, 16_MHz_XTAL / 4));
	pio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio.out_pa_callback().set("cent_data_out", FUNC(output_latch_device::write));
	pio.in_pb_callback().set(FUNC(bullet_state::pio_pb_r));

	MB8877(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->intrq_wr_callback().set(m_dart, FUNC(z80dart_device::dcda_w));
	m_fdc->drq_wr_callback().set(FUNC(bullet_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, MB8877_TAG":0", bullet_525_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":1", bullet_525_floppies, nullptr,    floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":2", bullet_525_floppies, nullptr,    floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":3", bullet_525_floppies, nullptr,    floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":4", bullet_8_floppies, nullptr,      floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":5", bullet_8_floppies, nullptr,      floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":6", bullet_8_floppies, nullptr,      floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":7", bullet_8_floppies, nullptr,      floppy_image_device::default_mfm_floppy_formats);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(bullet_state::write_centronics_busy));
	m_centronics->perror_handler().set(FUNC(bullet_state::write_centronics_perror));
	m_centronics->select_handler().set(FUNC(bullet_state::write_centronics_select));
	m_centronics->fault_handler().set(FUNC(bullet_state::write_centronics_fault));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set(m_dart, FUNC(z80dart_device::rxa_w));
	rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_dart, FUNC(z80dart_device::rxb_w));

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("wmbullet");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("128K");
}


//-------------------------------------------------
//  machine_config( bulletf )
//-------------------------------------------------

void bulletf_state::bulletf(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &bulletf_state::bulletf_mem);
	m_maincpu->set_addrmap(AS_IO, &bulletf_state::bulletf_io);
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->busack_cb().set(m_dmac, FUNC(z80dma_device::bai_w));

	// devices
	Z80CTC(config, m_ctc, 16_MHz_XTAL / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(FUNC(bullet_state::dart_rxtxca_w));
	m_ctc->zc_callback<1>().set(m_dart, FUNC(z80dart_device::rxtxcb_w));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));

	TIMER(config, "ctc").configure_periodic(FUNC(bullet_state::ctc_tick), attotime::from_hz(4.9152_MHz_XTAL / 4));

	Z80DART(config, m_dart, 16_MHz_XTAL / 4);
	m_dart->out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_dart->out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_dart->out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_dart->out_wrdya_callback().set(FUNC(bullet_state::dartardy_w));
	m_dart->out_txdb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_dart->out_dtrb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_dart->out_rtsb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));
	m_dart->out_wrdyb_callback().set(FUNC(bullet_state::dartbrdy_w));
	m_dart->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80DMA(config, m_dmac, 16_MHz_XTAL / 4);
	m_dmac->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dmac->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dmac->in_mreq_callback().set(FUNC(bullet_state::dma_mreq_r));
	m_dmac->out_mreq_callback().set(FUNC(bullet_state::dma_mreq_w));
	m_dmac->in_iorq_callback().set(FUNC(bullet_state::io_read_byte));
	m_dmac->out_iorq_callback().set(FUNC(bullet_state::io_write_byte));

	z80pio_device& pio(Z80PIO(config, Z80PIO_TAG, 16_MHz_XTAL / 4));
	pio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio.in_pa_callback().set("scsi_ctrl_in", FUNC(input_buffer_device::read));
	pio.out_pa_callback().set(FUNC(bulletf_state::pio_pa_w));
	pio.out_ardy_callback().set("cent_data_out", FUNC(output_latch_device::write));
	pio.out_brdy_callback().set(FUNC(bulletf_state::cstrb_w));

	MB8877(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->intrq_wr_callback().set(m_dart, FUNC(z80dart_device::rib_w));
	m_fdc->drq_wr_callback().set(FUNC(bullet_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, MB8877_TAG":0", bullet_525_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":1", bullet_525_floppies, nullptr,    floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":2", bullet_525_floppies, nullptr,    floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":3", bullet_525_floppies, nullptr,    floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":4", bullet_8_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":5", bullet_8_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":6", bullet_8_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":7", bullet_8_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":8", bullet_35_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, MB8877_TAG":9", bullet_35_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(bullet_state::write_centronics_busy));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set(m_dart, FUNC(z80dart_device::rxa_w));
	rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_dart, FUNC(z80dart_device::rxb_w));

	SCSI_PORT(config, m_scsibus, 0);
	m_scsibus->bsy_handler().set(m_scsi_ctrl_in, FUNC(input_buffer_device::write_bit3));
	m_scsibus->msg_handler().set(m_scsi_ctrl_in, FUNC(input_buffer_device::write_bit4));
	m_scsibus->cd_handler().set(m_scsi_ctrl_in, FUNC(input_buffer_device::write_bit5));
	m_scsibus->req_handler().set(FUNC(bulletf_state::req_w));
	m_scsibus->io_handler().set(m_scsi_ctrl_in, FUNC(input_buffer_device::write_bit7));
	m_scsibus->set_data_input_buffer(m_scsi_data_in);
	m_scsibus->set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));

	OUTPUT_LATCH(config, m_scsi_data_out);
	m_scsibus->set_output_latch(*m_scsi_data_out);
	INPUT_BUFFER(config, m_scsi_data_in);
	INPUT_BUFFER(config, m_scsi_ctrl_in);

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("wmbullet");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("128K");
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( bullet )
//-------------------------------------------------

ROM_START( wmbullet )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "sr70x.u8", 0x00, 0x20, CRC(d54b8a30) SHA1(65ff8753dd63c9dd1899bc9364a016225585d050) )
ROM_END


#define rom_wmbulletf rom_wmbullet



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME       PARENT    COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY      FULLNAME               FLAGS
// the setname 'bullet' is used by Sega's Bullet in MAME.
COMP( 1982, wmbullet,  0,        0,      bullet,  bullet,  bullet_state,  empty_init, "Wave Mate", "Bullet",              MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1984, wmbulletf, wmbullet, 0,      bulletf, bulletf, bulletf_state, empty_init, "Wave Mate", "Bullet (Revision F)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
