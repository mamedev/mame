// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Western Digital WD1002A-WX1 Winchester Disk Controller emulation

**********************************************************************/

/*

PCB Layout
----------

ASSY 61-000003-19

|---------------------------------------------------|
|  J2   J3           REV                J1          |
|  |1   |1                               1|     34  |
|                                                   |
| MC3486D            74LS14D                        |
|            10MHz                                  |
|                                                   |
| MC3487    WD10C20B WD1010A  WD1015A  7406D 74LS244|
|                                                   |
|                                     74LS00D       |
|     74LS244 ROM        WD11C00-17     RAM         |
|                        17-02                      |
|     74LS260D 74LS13D                DIPSW         |
|                                                   |
|                                    J4             |
|---|                                        |------|
    |----------------------------------------|

Notes:
    All IC's shown.

    WD1010A     - Western Digital WD1010A-05 Winchester Disk Controller
    WD1015A     - Western Digital WD1015A-JM Buffer Manager Control Processor
    WD11C00-17  - Western Digital WD11C00-JT-17-02 PC/XT Host Interface Logic Device
    WD10C20B    - Western Digital WD10C20B-JH-05 Self-Adjusting Data Separator
    RAM         - NEC uPD446G-20L 2Kx8 static RAM, the shared sector buffer
    ROM         - 28-pin 8Kx8 mask ROM, the host BIOS extension
    MC3486D     - Motorola quad RS-422 line receiver, drive control/data lines
    MC3487      - Motorola quad RS-422 line driver, drive control/data lines
    DIPSW       - 8-position DIP switch, drive type/configuration select
    J1          - 2x17 pin PCB header, drive control cable (shared, both drives)
    J2          - 2x10 pin PCB header, drive 0 data cable
    J3          - 2x10 pin PCB header, drive 1 data cable
    J4          - 3-pin header, hard disk activity LED

*/

#include "emu.h"
#include "wd1002a_wx1.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_WD1002A_WX1, isa8_wd1002a_wx1_device, "wd1002a_wx1", "WD1002A-WX1")


//-------------------------------------------------
//  ROM( wd1002a_wx1 )
//-------------------------------------------------

ROM_START( wd1002a_wx1 )
	ROM_REGION( 0x2000, "wd1002a_wx1", 0 )
	ROM_LOAD( "600693-001 type 5.u12", 0x0000, 0x2000, CRC(f3daf85f) SHA1(3bd29538832d3084cbddeec92593988772755283) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_wd1002a_wx1_device::device_rom_region() const
{
	return ROM_NAME( wd1002a_wx1 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_wd1002a_wx1_device::device_add_mconfig(machine_config &config)
{
	WD11C00_17(config, m_host, XTAL(10'000'000)/2);
	m_host->out_irq5_callback().set(FUNC(isa8_wd1002a_wx1_device::irq5_w));
	m_host->out_drq3_callback().set(FUNC(isa8_wd1002a_wx1_device::drq3_w));
	m_host->out_mr_callback().set(FUNC(isa8_wd1002a_wx1_device::mr_w));
	m_host->out_busy_callback().set(FUNC(isa8_wd1002a_wx1_device::busy_w));
	m_host->in_rd322_callback().set(FUNC(isa8_wd1002a_wx1_device::rd322_r));
	m_host->in_ramcs_callback().set(FUNC(isa8_wd1002a_wx1_device::ram_r));
	m_host->out_ramwr_callback().set(FUNC(isa8_wd1002a_wx1_device::ram_w));

	WD1010(config, m_hdc, XTAL(10'000'000)/2);
	m_hdc->out_intrq_callback().set(FUNC(isa8_wd1002a_wx1_device::hdc_intrq_w));
	m_hdc->in_data_callback().set(FUNC(isa8_wd1002a_wx1_device::hdc_data_r));
	m_hdc->out_data_callback().set(FUNC(isa8_wd1002a_wx1_device::hdc_data_w));

	HARDDISK(config, "hdc:0", 0);
	HARDDISK(config, "hdc:1", 0);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_wd1002a_wx1_device - constructor
//-------------------------------------------------

isa8_wd1002a_wx1_device::isa8_wd1002a_wx1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA8_WD1002A_WX1, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_host(*this, "host")
	, m_hdc(*this, "hdc")
	, m_hdd(*this, "hdc:%u", 0U)
	, m_cdb{ 0 }
	, m_cdb_ptr(0)
	, m_completion(0)
	, m_sense{ 0 }
	, m_secbuf{ 0 }
	, m_secptr(0)
	, m_din_sense(false)
	, m_din_len(0)
	, m_phase(PHASE_CDB)
	, m_pending_command(0)
	, m_remaining(0)
	, m_sector_number(0)
	, m_error(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_wd1002a_wx1_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc8000, 0xc9fff, "wd1002a_wx1");
	m_isa->install_device(0x0320, 0x0323, read8sm_delegate(*m_host, FUNC(wd11c00_17_device::io_r)), write8sm_delegate(*m_host, FUNC(wd11c00_17_device::io_w)));
	m_isa->set_dma_channel(3, this, false);

	save_item(NAME(m_cdb));
	save_item(NAME(m_cdb_ptr));
	save_item(NAME(m_completion));
	save_item(NAME(m_sense));
	save_item(NAME(m_secbuf));
	save_item(NAME(m_secptr));
	save_item(NAME(m_din_sense));
	save_item(NAME(m_din_len));
	save_item(NAME(m_phase));
	save_item(NAME(m_pending_command));
	save_item(NAME(m_remaining));
	save_item(NAME(m_sector_number));
	save_item(NAME(m_error));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_wd1002a_wx1_device::device_reset()
{
	m_hdc->drdy_w((m_hdd[0]->exists() || m_hdd[1]->exists()) ? 1 : 0);
	m_hdc->sc_w(1);
	// the WD1015A buffer manager isn't modeled; its BRDY handshake is tied
	// permanently ready since the sector buffer here is plain host RAM
	m_hdc->brdy_w(1);

	m_phase = PHASE_CDB;
	m_cdb_ptr = 0;
	m_secptr = 0;
	m_completion = 0;
	m_error = false;
}


//-------------------------------------------------
//  device_isa8_card_interface overrides -- DMA channel 3
//-------------------------------------------------

uint8_t isa8_wd1002a_wx1_device::dack_r(int line)
{
	return m_host->dack_r();
}

void isa8_wd1002a_wx1_device::dack_w(int line, uint8_t data)
{
	m_host->dack_w(data);
}

void isa8_wd1002a_wx1_device::dack_line_w(int line, int state)
{
	m_host->dack3_w(state);
}


//-------------------------------------------------
//  WD11C00-17 host interface glue
//-------------------------------------------------

void isa8_wd1002a_wx1_device::irq5_w(int state)
{
	m_isa->irq5_w(state);
}

void isa8_wd1002a_wx1_device::drq3_w(int state)
{
	m_isa->drq3_w(state);
}

void isa8_wd1002a_wx1_device::mr_w(int state)
{
	// board-level software reset: abort whatever command phase we were in
	if (state)
	{
		m_phase = PHASE_CDB;
		m_cdb_ptr = 0;
	}
}

void isa8_wd1002a_wx1_device::busy_w(int state)
{
	// out_busy_callback is active low; state == 0 means the host has just
	// written the Select register (BUSY newly asserted) -- a new command begins
	if (state == 0)
	{
		m_phase = PHASE_CDB;
		m_cdb_ptr = 0;
	}
}

uint8_t isa8_wd1002a_wx1_device::rd322_r()
{
	// "Read Drive Configuration Information" (port 322h): reflects an on-board
	// jumper/DIP-switch bank on real hardware.  Not modeled; TEST DRIVE READY
	// and the CDB-level drive checks are authoritative instead (matches the
	// same simplification wdxt_gen.cpp makes for the same WD11C00-17 register).
	return 0xff;
}


//-------------------------------------------------
//  shared RAM window (WD11C00-17 Random Address counter) -- this is where the
//  undumped WD1015A's own command-block parsing and buffer management is
//  high-level emulated.  ram_r/ram_w see every byte the host (PIO or DMA) or
//  our own WD1010 command sequencing moves through the Data register; which
//  buffer they land in is tracked here by protocol phase rather than by the
//  WD11C00-17's internal RA value (see wd1002a_wx1.h for why).
//-------------------------------------------------

uint8_t isa8_wd1002a_wx1_device::ram_r(offs_t offset)
{
	if (m_phase == PHASE_DATA_IN && m_secptr < m_din_len)
	{
		uint8_t const data = m_din_sense ? m_sense[m_secptr] : m_secbuf[m_secptr];
		m_secptr++;
		if (m_secptr == m_din_len)
			advance_or_finish();
		return data;
	}

	// PHASE_STATUS (or a stray read once the data phase is drained): serve the
	// completion byte and release BUSY so the host can select() the next command
	m_host->ireq_w(0);
	return m_completion;
}

void isa8_wd1002a_wx1_device::ram_w(offs_t offset, uint8_t data)
{
	switch (m_phase)
	{
	case PHASE_CDB:
		m_cdb[m_cdb_ptr++] = data;
		if (m_cdb_ptr == 6)
			begin_command();
		break;

	case PHASE_INIT_DATA:
		// INITIALIZE DRIVE CHARACTERISTICS parameter block: geometry already
		// comes from the mounted CHD, so the bytes are only counted, not stored
		if (++m_secptr >= 8)
			complete(0);
		break;

	case PHASE_DATA_OUT:
		if (m_secptr < 512)
			m_secbuf[m_secptr++] = data;
		if (m_secptr >= 512)
		{
			issue_hdc_command(0x30); // WRITE SECTOR
			m_phase = PHASE_WAIT_HDC;
		}
		break;

	default:
		break;
	}
}


//-------------------------------------------------
//  WD1010 sector-data callbacks -- during cmd_read_sector()/cmd_write_sector()
//  the WD1010 core moves a whole 512-byte sector through these synchronously
//-------------------------------------------------

uint8_t isa8_wd1002a_wx1_device::hdc_data_r()
{
	return (m_secptr < 512) ? m_secbuf[m_secptr++] : 0;
}

void isa8_wd1002a_wx1_device::hdc_data_w(uint8_t data)
{
	if (m_secptr < 512)
		m_secbuf[m_secptr++] = data;
}

void isa8_wd1002a_wx1_device::hdc_intrq_w(int state)
{
	if (!state || m_phase != PHASE_WAIT_HDC)
		return;

	uint8_t const status = m_hdc->read(7);
	m_error = BIT(status, 0);
	if (m_error)
	{
		m_sense[0] = m_hdc->read(1); // WD1010 error register
		m_sense[1] = m_cdb[1];
		m_sense[2] = m_cdb[2];
		m_sense[3] = m_cdb[3];

		complete(1);
		return;
	}

	if (m_pending_command == CDB_READ)
		start_data_in(false, 512); // sector already sitting in m_secbuf via hdc_data_w
	else
		advance_or_finish();
}


//-------------------------------------------------
//  WD1002/XT command descriptor block protocol (WD1015A high-level emulation)
//-------------------------------------------------

void isa8_wd1002a_wx1_device::setup_task_file(int drive, int head, int cylinder, int sector)
{
	m_hdc->drdy_w(m_hdd[drive & 1]->exists() ? 1 : 0);
	m_hdc->head_w(head);
	m_hdc->write(2, 1); // sector count: we always run the WD1010 one sector at a time
	m_sector_number = sector;
	m_hdc->write(3, m_sector_number);
	m_hdc->write(4, cylinder & 0xff);
	m_hdc->write(5, (cylinder >> 8) & 0xff);
	m_hdc->write(6, (drive & 1) << 3); // SDH drive-select bits (head is applied via head_w above)
}

void isa8_wd1002a_wx1_device::issue_hdc_command(uint8_t command)
{
	m_secptr = 0;
	m_hdc->write(7, command);
}

void isa8_wd1002a_wx1_device::begin_command()
{
	int const drive = BIT(m_cdb[1], 5);
	int const head = m_cdb[1] & 0x1f;
	int const cylinder = ((m_cdb[2] & 0xc0) << 2) | m_cdb[3];
	int const sector = m_cdb[2] & 0x3f;
	int const count = m_cdb[4] ? m_cdb[4] : 1;

	m_remaining = 0;
	m_pending_command = m_cdb[0];

	switch (m_cdb[0])
	{
	case CDB_TEST_READY:
	{
		bool const ready = m_hdd[drive]->exists();
		if (!ready)
		{
			m_sense[0] = 0x04; // drive not ready (approximate sense code)
			m_sense[1] = m_cdb[1];
			m_sense[2] = m_cdb[2];
			m_sense[3] = m_cdb[3];
		}
		complete(ready ? 0 : 1);
		break;
	}

	case CDB_SENSE:
		start_data_in(true, 4);
		break;

	case CDB_INIT_DRIVE:
		m_phase = PHASE_INIT_DATA;
		m_secptr = 0;
		m_host->cd_w(0);
		m_host->io_w(0);
		break;

	case CDB_RECALIBRATE:
		setup_task_file(drive, 0, 0, 0);
		issue_hdc_command(0x10 | (m_cdb[5] & 0x0f)); // RESTORE
		m_phase = PHASE_WAIT_HDC;
		break;

	case CDB_SEEK:
		setup_task_file(drive, head, cylinder, sector);
		issue_hdc_command(0x70 | (m_cdb[5] & 0x0f)); // SEEK
		m_phase = PHASE_WAIT_HDC;
		break;

	case CDB_READ:
	case CDB_VERIFY:
		m_remaining = count;
		setup_task_file(drive, head, cylinder, sector);
		issue_hdc_command(0x20); // READ SECTOR
		m_phase = PHASE_WAIT_HDC;
		break;

	case CDB_WRITE:
		m_remaining = count;
		setup_task_file(drive, head, cylinder, sector);
		m_phase = PHASE_DATA_OUT;
		m_secptr = 0;
		m_host->cd_w(0);
		m_host->io_w(0);
		break;

	default:
		// FORMAT DRIVE/TRACK/BAD TRACK and anything else: not implemented
		m_sense[0] = 0x20; // illegal command (approximate sense code)
		m_sense[1] = m_cdb[1];
		m_sense[2] = m_cdb[2];
		m_sense[3] = m_cdb[3];
		complete(1);
		break;
	}
}

void isa8_wd1002a_wx1_device::advance_or_finish()
{
	if (m_remaining > 0)
		m_remaining--;

	if (m_remaining > 0 && !m_error)
	{
		m_sector_number++;
		m_hdc->write(3, m_sector_number);

		if (m_pending_command == CDB_WRITE)
		{
			m_phase = PHASE_DATA_OUT;
			m_secptr = 0;
			m_host->cd_w(0);
			m_host->io_w(0);
		}
		else // READ or VERIFY
		{
			issue_hdc_command(0x20);
			m_phase = PHASE_WAIT_HDC;
		}
	}
	else
	{
		complete(m_error ? 1 : 0);
	}
}

void isa8_wd1002a_wx1_device::complete(uint8_t error)
{
	m_error = error != 0;
	m_completion = (m_error ? 0x01 : 0x00) | (m_cdb[1] & 0x20); // bit0 = error, bit5 = echoed drive select
	enter_status_phase();
}

void isa8_wd1002a_wx1_device::start_data_in(bool sense, int length)
{
	m_din_sense = sense;
	m_din_len = length;
	m_secptr = 0;
	m_phase = PHASE_DATA_IN;
	m_host->cd_w(0);
	m_host->io_w(1);
}

void isa8_wd1002a_wx1_device::enter_status_phase()
{
	m_phase = PHASE_STATUS;
	m_host->cd_w(1);
	m_host->ireq_w(1); // also forces I_O=1 and fires IRQ5: the completion interrupt
}
