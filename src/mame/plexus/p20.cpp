// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Plexus P20

    UNIX server

    Hardware:
    - 2x MC68010L12 CPU (named JOB and DMA)
    - 4x SEEQ DQ5143-300 27128-30 EPROM
    - 2x M5M5165P RAM (8k each)
    - 6x TMM2018D-45 RAM (2k each)
    - 4x MK68564N-3A SIO
    - MC146818P RTC (unreadable XTAL next to it)
    - 20 MHz and 9.8304 MHz XTAL
    - Multibus
    - 2 MB RAM on RAM card
    - Omti 5200 SCSI-1 to MFM/Floppy
    - Fujitsu M2243AS HDD
    - 5.25" floppy drive
    - Archive Scorpion 5945C QIC-24 tape drive

    TODO:
    - Almost everything

    Notes:
    - Implemented just enough to operate the terminal

***************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68010.h"
#include "machine/input_merger.h"
#include "machine/mc146818.h"
#include "machine/z80sio.h"

#include "p20.lh"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class p20_state : public driver_device
{
public:
	p20_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_dmacpu(*this, "dmacpu"),
		m_jobcpu(*this, "jobcpu"),
		m_sio(*this, "sio%u", 0U),
		m_sio_irqs(*this, "sio_irqs"),
		m_serial0(*this, "serial0"),
		m_rtc(*this, "rtc"),
		m_leds(*this, "led%u", 0U),
		m_dma_boot(*this, "boot"),
		m_job_boot(*this, "job_boot")
	{ }

	void p20(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void common_map(address_map &map) ATTR_COLD;
	void dma_map(address_map &map) ATTR_COLD;
	void job_map(address_map &map) ATTR_COLD;
	void dma_cpu_space_map(address_map &map) ATTR_COLD;
	void job_cpu_space_map(address_map &map) ATTR_COLD;

	uint16_t csr_parity_r(offs_t offset, uint16_t mem_mask);
	void csr_reset_select_w(uint16_t data);
	uint16_t csr_status_r(offs_t offset, uint16_t mem_mask);
	uint16_t csr_mberr_r(offs_t offset, uint16_t mem_mask);
	uint16_t csr_scsi_count_hi_r(offs_t offset, uint16_t mem_mask);
	void csr_scsi_count_hi_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t csr_scsi_count_lo_r(offs_t offset, uint16_t mem_mask);
	void csr_scsi_count_lo_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t csr_scsi_pointer_hi_r(offs_t offset, uint16_t mem_mask);
	void csr_scsi_pointer_hi_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t csr_scsi_pointer_lo_r(offs_t offset, uint16_t mem_mask);
	void csr_scsi_pointer_lo_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t csr_scsi_r(offs_t offset, uint16_t mem_mask);
	void csr_scsi_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void csr_led_w(offs_t offset, uint8_t data);
	uint16_t csr_error_r(offs_t offset, uint16_t mem_mask);
	uint16_t csr_misc_r(offs_t offset, uint16_t mem_mask);
	void csr_misc_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t csr_kill_r(offs_t offset, uint16_t mem_mask);
	void csr_kill_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t csr_trce_r(offs_t offset, uint16_t mem_mask);
	void csr_trce_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t csr_mapid_r(offs_t offset);
	void csr_mapid_w(offs_t offset, uint8_t data);
	void csr_reset_mbus_error_w(uint16_t data);
	void csr_reset_scsi_parity_w(uint16_t data);
	void csr_clear_job_interrupt_w(uint16_t data);
	void csr_set_job_interrupt_w(uint16_t data);
	void csr_clear_dma_interrupt_w(uint16_t data);
	void csr_set_dma_interrupt_w(uint16_t data);
	void csr_reset_job_clock_interrupt_w(uint16_t data);
	void csr_reset_dma_clock_interrupt_w(uint16_t data);
	void csr_reset_job_bus_error_w(uint16_t data);
	void csr_reset_dma_bus_error_w(uint16_t data);
	void csr_reset_memory_parity_w(uint16_t data);
	void csr_reset_switch_interrupt_w(uint16_t data);
	void csr_reset_scsi_bus_error_w(uint16_t data);

	uint8_t dma_interrupt_vector_r();
	uint8_t job_interrupt_vector_r();
	uint8_t sio_vector_r();

	required_device<m68010_device> m_dmacpu;
	required_device<m68010_device> m_jobcpu;
	required_device_array<mk68564_device, 4> m_sio;
	required_device<input_merger_device> m_sio_irqs;
	required_device<rs232_port_device> m_serial0;
	required_device<mc146818_device> m_rtc;
	output_finder<8> m_leds;

	// can force a23 high
	memory_view m_dma_boot;
	memory_view m_job_boot;

	uint32_t m_csr_scsi_count = 0;
	uint32_t m_csr_scsi_pointer = 0;
	uint16_t m_csr_scsi = 0;
	uint16_t m_csr_misc = 0;
	uint16_t m_csr_kill = 0;
	uint16_t m_csr_trce = 0;
	uint8_t m_csr_mapid = 0;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void p20_state::common_map(address_map &map)
{
	map(0x800000, 0x80ffff).rom().region("firmware", 0).nopw();
	map(0xa00000, 0xa0003f).rw(m_sio[0], FUNC(mk68564_device::read), FUNC(mk68564_device::write)).umask16(0x00ff);
	map(0xa10000, 0xa1003f).rw(m_sio[1], FUNC(mk68564_device::read), FUNC(mk68564_device::write)).umask16(0x00ff);
	map(0xa20000, 0xa2003f).rw(m_sio[2], FUNC(mk68564_device::read), FUNC(mk68564_device::write)).umask16(0x00ff);
	map(0xa30000, 0xa3003f).rw(m_sio[3], FUNC(mk68564_device::read), FUNC(mk68564_device::write)).umask16(0x00ff);
	map(0xc00000, 0xc03fff).ram().share("sram");
	map(0xd00000, 0xd0007f).rw(m_rtc, FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct)).umask16(0x00ff);
	map(0xe00000, 0xe00001).rw(FUNC(p20_state::csr_parity_r), FUNC(p20_state::csr_reset_select_w));
	map(0xe00002, 0xe00003).r(FUNC(p20_state::csr_status_r));
	map(0xe00004, 0xe00005).r(FUNC(p20_state::csr_mberr_r));
	map(0xe00006, 0xe00007).rw(FUNC(p20_state::csr_scsi_count_hi_r), FUNC(p20_state::csr_scsi_count_hi_w));
	map(0xe00008, 0xe00009).rw(FUNC(p20_state::csr_scsi_count_lo_r), FUNC(p20_state::csr_scsi_count_lo_w));
	map(0xe0000a, 0xe0000b).rw(FUNC(p20_state::csr_scsi_pointer_hi_r), FUNC(p20_state::csr_scsi_pointer_hi_w));
	map(0xe0000c, 0xe0000d).rw(FUNC(p20_state::csr_scsi_pointer_lo_r), FUNC(p20_state::csr_scsi_pointer_lo_w));
	map(0xe0000e, 0xe0000f).rw(FUNC(p20_state::csr_scsi_r), FUNC(p20_state::csr_scsi_w));
	map(0xe00010, 0xe00011).w(FUNC(p20_state::csr_led_w)).umask16(0xffff);
	map(0xe00014, 0xe00015).r(FUNC(p20_state::csr_error_r));
	map(0xe00016, 0xe00017).rw(FUNC(p20_state::csr_misc_r), FUNC(p20_state::csr_misc_w));
	map(0xe00018, 0xe00019).rw(FUNC(p20_state::csr_kill_r), FUNC(p20_state::csr_kill_w));
	map(0xe0001a, 0xe0001b).rw(FUNC(p20_state::csr_trce_r), FUNC(p20_state::csr_trce_w));
	map(0xe0001e, 0xe0001f).rw(FUNC(p20_state::csr_mapid_r), FUNC(p20_state::csr_mapid_w)).umask16(0xffff);
	map(0xe00020, 0xe00021).w(FUNC(p20_state::csr_reset_mbus_error_w));
	map(0xe00040, 0xe00041).w(FUNC(p20_state::csr_reset_scsi_parity_w));
	map(0xe00060, 0xe00061).w(FUNC(p20_state::csr_clear_job_interrupt_w));
	map(0xe00080, 0xe00081).w(FUNC(p20_state::csr_set_job_interrupt_w));
	map(0xe000a0, 0xe000a1).w(FUNC(p20_state::csr_clear_dma_interrupt_w));
	map(0xe000c0, 0xe000c1).w(FUNC(p20_state::csr_set_dma_interrupt_w));
	map(0xe000e0, 0xe000e1).w(FUNC(p20_state::csr_reset_job_clock_interrupt_w));
	map(0xe00100, 0xe00101).w(FUNC(p20_state::csr_reset_dma_clock_interrupt_w));
	map(0xe00120, 0xe00121).w(FUNC(p20_state::csr_reset_job_bus_error_w));
	map(0xe00140, 0xe00141).w(FUNC(p20_state::csr_reset_dma_bus_error_w));
	map(0xe00160, 0xe00161).w(FUNC(p20_state::csr_reset_memory_parity_w));
	map(0xe00180, 0xe00181).w(FUNC(p20_state::csr_reset_switch_interrupt_w));
	map(0xe001a0, 0xe001a1).w(FUNC(p20_state::csr_reset_scsi_bus_error_w));
}

void p20_state::dma_map(address_map &map)
{
	map(0x000000, 0x7fffff).view(m_dma_boot);
	m_dma_boot[0](0x000000, 0x00ffff).rom().region("firmware", 0);
	m_dma_boot[1](0x000000, 0x1fffff).ram().share("mainram"); // on RAM board
	m_dma_boot[1](0x200000, 0x7fffff).noprw(); // space for more RAM

	common_map(map);
}

void p20_state::job_map(address_map &map)
{
	map(0x000000, 0x7fffff).view(m_job_boot);
	m_job_boot[0](0x000000, 0x00ffff).rom().region("firmware", 0);
	m_job_boot[1](0x000000, 0x1fffff).ram().share("mainram"); // on RAM board
	m_job_boot[1](0x200000, 0x7fffff).noprw(); // space for more RAM

	common_map(map);
}

void p20_state::dma_cpu_space_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_dmacpu, FUNC(m68000_base_device::autovectors_map));
	map(0xfffff5, 0xfffff5).r(FUNC(p20_state::dma_interrupt_vector_r));
	map(0xfffffb, 0xfffffb).r(FUNC(p20_state::sio_vector_r));
}

void p20_state::job_cpu_space_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_jobcpu, FUNC(m68000_base_device::autovectors_map));
	map(0xfffff9, 0xfffff9).r(FUNC(p20_state::job_interrupt_vector_r));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( p20 )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

uint16_t p20_state::csr_parity_r(offs_t, uint16_t)
{
	// --d-------------  PEH (high-byte parity error)
	// ---c------------  PEL (low-byte parity error)
	// ----b-----------  EN.BLK
	// -----a----------  EN.MBUS
	// ------9---------  EN.DMA
	// -------8--------  EN.JOB
	// --------7-------  MWT (multibus write transaction)
	// ---------6------  multibus address 9
	// ----------543210  multibus address 21-16

	return 0;
}

void p20_state::csr_reset_select_w(uint16_t)
{
	// data is ignored
}

uint16_t p20_state::csr_status_r(offs_t, uint16_t)
{
	// f---------------  DIS.MAP
	// -e--------------  RES.DMA- (active low)
	// --d-------------  HALT.DMA- (active low)
	// ---c------------  RES.JOB- (active low)
	// ----b-----------  HALT.JOB- (active low)
	// -----a----------  UPS
	// ------9---------  TEMP
	// -------8--------  PFW (power fail warning)
	// --------7-------  BREQ3- (active low)
	// ---------6------  BREQ2- (active low)
	// ----------5-----  BREQ1- (active low)
	// -----------4----  BREQ- (active low)
	// ------------3---  BUSY
	// -------------2--  SCSIBSY
	// --------------1-  BERR.JOB- (active low)
	// ---------------0  BERR.DMA- (active low)

	return 0;
}

uint16_t p20_state::csr_mberr_r(offs_t, uint16_t)
{
	// --------7654321-  job address 18-12
	// ---------------0  READ.JOB

	return 0;
}

uint16_t p20_state::csr_scsi_count_hi_r(offs_t, uint16_t)
{
	return (m_csr_scsi_count >> 16) & 0x0f;
}

void p20_state::csr_scsi_count_hi_w(offs_t, uint16_t data, uint16_t mem_mask)
{
	uint16_t high = (m_csr_scsi_count >> 16) & 0x0f;
	COMBINE_DATA(&high);
	m_csr_scsi_count = (m_csr_scsi_count & 0x0000ffff) | ((high & 0x0f) << 16);
}

uint16_t p20_state::csr_scsi_count_lo_r(offs_t, uint16_t)
{
	return m_csr_scsi_count;
}

void p20_state::csr_scsi_count_lo_w(offs_t, uint16_t data, uint16_t mem_mask)
{
	uint16_t low = m_csr_scsi_count;
	COMBINE_DATA(&low);
	m_csr_scsi_count = (m_csr_scsi_count & 0x000f0000) | low;
}

uint16_t p20_state::csr_scsi_pointer_hi_r(offs_t, uint16_t)
{
	return (m_csr_scsi_pointer >> 16) & 0x0f;
}

void p20_state::csr_scsi_pointer_hi_w(offs_t, uint16_t data, uint16_t mem_mask)
{
	uint16_t high = (m_csr_scsi_pointer >> 16) & 0x0f;
	COMBINE_DATA(&high);
	m_csr_scsi_pointer = (m_csr_scsi_pointer & 0x0000ffff) | ((high & 0x0f) << 16);
}

uint16_t p20_state::csr_scsi_pointer_lo_r(offs_t, uint16_t)
{
	return m_csr_scsi_pointer;
}

void p20_state::csr_scsi_pointer_lo_w(offs_t, uint16_t data, uint16_t mem_mask)
{
	uint16_t low = m_csr_scsi_pointer;
	COMBINE_DATA(&low);
	m_csr_scsi_pointer = (m_csr_scsi_pointer & 0x000f0000) | low;
}

uint16_t p20_state::csr_scsi_r(offs_t, uint16_t)
{
	// f---------------  ARBR- (arbitration in progress, active low)
	// -e--------------  SCZERO
	// --d-------------  SCPERR- (scsi parity error, active low)
	// ---c------------  SCBERR- (scsi bus error, active low)
	// ----b-----------  STIME (selection timeout)
	// -----a----------  SEL
	// ------9---------  BSY
	// -------8--------  MYBIT (initiator id match)
	// --------7-------  REQ
	// ---------6------  MSG
	// ----------5-----  SCRST
	// -----------4----  I/O
	// ------------3---  C/D
	// -------------2--  ATN
	// --------------1-  ACK
	// ---------------0  DATEN

	// TODO: honor actual scsi lines
	return m_csr_scsi;
}

void p20_state::csr_scsi_w(offs_t, uint16_t data, uint16_t mem_mask)
{
	// f---------------  IOPTR
	// -e--------------  MSGPTR
	// --d-------------  CDPTR
	// ---c------------  buffer in static ram
	// ----b-----------  reset scsi
	// -----a----------  scsi selection enable
	// ------9---------  set SCSIBSY- (active low)
	// -------8--------  arbitration flag
	// --------7-------  set SCSIREQ- (active low)
	// ---------6------  set SCSIMSG- (active low)
	// ----------5-----  set SCSIRST- (active low)
	// -----------4----  set SCSII/O- (active low)
	// ------------3---  set SCSIC/D- (active low)
	// -------------2--  set SCSIATN- (active low)
	// --------------1-  set SCSIACK- (active low)
	// ---------------0  automatic data-transfer enable

	COMBINE_DATA(&m_csr_scsi);
}

void p20_state::csr_led_w(offs_t, uint8_t data)
{
	for (unsigned i = 0; i < 8; i++)
		m_leds[i] = BIT(data, i);
}

uint16_t p20_state::csr_error_r(offs_t, uint16_t)
{
	// f---------------  AS26 (DMA transfer timeout)
	// -e--------------  S.OOPS (illegal mapper/multibus access)
	// --d-------------  Spare
	// ---c------------  UBE.DMA (DMA user-ID mismatch)
	// ----b-----------  ABE.DMA (DMA access privilege error)
	// -----a----------  EN.BLK
	// ------9---------  EN.DMA
	// -------8--------  EN.JOB
	// --------7-------  AERR.JOB (Job access to system space)
	// ---------6------  DERR.JOB (Job access to DMA bus)
	// ----------5-----  MBTO (multibus timeout)
	// -----------4----  UBE.JOB (Job user-ID mismatch)
	// ------------3---  ABE.JOB (Job access privilege error)
	// -------------2--  EN.JOB
	// --------------1-  EN.BLK
	// ---------------0  EN.MBUS

	return 0;
}

uint16_t p20_state::csr_misc_r(offs_t, uint16_t)
{
	// TODO: bit 7 is the read-only TBUSY input
	return m_csr_misc;
}

void p20_state::csr_misc_w(offs_t, uint16_t data, uint16_t mem_mask)
{
	// f---------------  BOOT.DMA- (active low)
	// -e--------------  BOOT.JOB- (active low)
	// --d-------------  SCSIDL- (diagnostic latch, active low)
	// ---c------------  DIAG.PEH (force high-byte parity error)
	// ----b-----------  DIAG.PEL (force low-byte parity error)
	// -----a----------  DIAG.PESC (force scsi parity error)
	// ------9---------  DIAG.MB (multibus diagnostic mode)
	// -------8--------  DIS.MAP
	// --------7-------  spare (TBUSY when read)
	// ---------6------  DIAG.UART
	// ----------5-----  HOLDMBUS
	// -----------4----  RESMB- (multibus reset, active low)
	// ------------3---  CINTD.EN (dma clock interrupt enable)
	// -------------2--  CINTJ.EN (job clock interrupt enable)
	// --------------1-  TINT.EN (temperature interrupt enable)
	// ---------------0  UINT.EN (ups interrupt enable)

	COMBINE_DATA(&m_csr_misc);
	m_csr_misc &= ~0x0080;

	// BOOT.DMA- and BOOT.JOB- force A23 high when clear
	m_dma_boot.select(BIT(m_csr_misc, 15));
	m_job_boot.select(BIT(m_csr_misc, 14));
}

uint16_t p20_state::csr_kill_r(offs_t, uint16_t)
{
	// --------7-------  EN.JOB
	// ---------6------  JKPD (job-control protection disable)
	// ------------3---  INT.JOB
	// -------------2--  INT.DMA
	// --------------1-  KILL.JOB- (active low)
	// ---------------0  KILL.DMA

	return m_csr_kill;
}

void p20_state::csr_kill_w(offs_t, uint16_t data, uint16_t mem_mask)
{
	// ---------6------  JKPD (job-control protection disable)
	// --------------1-  KILL.JOB- (active low)
	// ---------------0  KILL.DMA

	uint16_t const interrupt_status = m_csr_kill & 0x000c;
	COMBINE_DATA(&m_csr_kill);
	m_csr_kill = (m_csr_kill & 0x0043) | interrupt_status;

	// KILL.DMA is active high, KILL.JOB- is active low
	m_dmacpu->set_input_line(INPUT_LINE_RESET, BIT(m_csr_kill, 0) ? ASSERT_LINE : CLEAR_LINE);
	m_jobcpu->set_input_line(INPUT_LINE_RESET, BIT(m_csr_kill, 1) ? CLEAR_LINE : ASSERT_LINE);
}

uint16_t p20_state::csr_trce_r(offs_t, uint16_t)
{
	// --d-------------  RI.B
	// ---c------------  RI.A
	// ----b-----------  TCE.B- (active low)
	// -----a----------  TCE.A- (active low)
	// ------9---------  RCE.B- (active low)
	// -------8--------  RCE.A- (active low)
	// ------------3---  DSR.D
	// -------------2--  DSR.C
	// --------------1-  DSR.B
	// ---------------0  DSR.A

	return m_csr_trce;
}

void p20_state::csr_trce_w(offs_t, uint16_t data, uint16_t mem_mask)
{
	// ----b-----------  TCE.B
	// -----a----------  TCE.A
	// ------9---------  RCE.B
	// -------8--------  RCE.A

	COMBINE_DATA(&m_csr_trce);
	m_csr_trce &= 0x0f00;
}

uint8_t p20_state::csr_mapid_r(offs_t)
{
	return m_csr_mapid;
}

void p20_state::csr_mapid_w(offs_t, uint8_t data)
{
	m_csr_mapid = data;
}

void p20_state::csr_reset_mbus_error_w(uint16_t data)
{
	logerror("%s: unimplemented CSR write E00020 = %04X (reset Multibus interface error flag)\n", machine().describe_context(), data);
}

void p20_state::csr_reset_scsi_parity_w(uint16_t data)
{
	logerror("%s: unimplemented CSR write E00040 = %04X (reset SCSI parity error flag)\n", machine().describe_context(), data);
}

void p20_state::csr_clear_job_interrupt_w(uint16_t)
{
	m_csr_kill &= ~0x0008;
	m_jobcpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
}

void p20_state::csr_set_job_interrupt_w(uint16_t)
{
	m_csr_kill |= 0x0008;
	m_jobcpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
}

void p20_state::csr_clear_dma_interrupt_w(uint16_t)
{
	m_csr_kill &= ~0x0004;
	m_dmacpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
}

void p20_state::csr_set_dma_interrupt_w(uint16_t)
{
	m_csr_kill |= 0x0004;
	m_dmacpu->set_input_line(M68K_IRQ_2, ASSERT_LINE);
}

void p20_state::csr_reset_job_clock_interrupt_w(uint16_t data)
{
	logerror("%s: unimplemented CSR write E000E0 = %04X (reset Job clock interrupt)\n", machine().describe_context(), data);
}

void p20_state::csr_reset_dma_clock_interrupt_w(uint16_t data)
{
	logerror("%s: unimplemented CSR write E00100 = %04X (reset DMA clock interrupt)\n", machine().describe_context(), data);
}

void p20_state::csr_reset_job_bus_error_w(uint16_t data)
{
	logerror("%s: unimplemented CSR write E00120 = %04X (reset Job bus error flag)\n", machine().describe_context(), data);
}

void p20_state::csr_reset_dma_bus_error_w(uint16_t data)
{
	logerror("%s: unimplemented CSR write E00140 = %04X (reset DMA bus error flag)\n", machine().describe_context(), data);
}

void p20_state::csr_reset_memory_parity_w(uint16_t data)
{
	logerror("%s: unimplemented CSR write E00160 = %04X (reset memory parity error flag)\n", machine().describe_context(), data);
}

void p20_state::csr_reset_switch_interrupt_w(uint16_t data)
{
	logerror("%s: unimplemented CSR write E00180 = %04X (reset switch interrupt latch)\n", machine().describe_context(), data);
}

void p20_state::csr_reset_scsi_bus_error_w(uint16_t data)
{
	logerror("%s: unimplemented CSR write E001A0 = %04X (reset SCSI bus error flag)\n", machine().describe_context(), data);
}

uint8_t p20_state::sio_vector_r()
{
	if (m_sio_irqs->in_r<0>())
		return m_sio[0]->m1_r();
	if (m_sio_irqs->in_r<1>())
		return m_sio[1]->m1_r();
	if (m_sio_irqs->in_r<2>())
		return m_sio[2]->m1_r();
	if (m_sio_irqs->in_r<3>())
		return m_sio[3]->m1_r();

	return m68000_base_device::autovector(5);
}

uint8_t p20_state::dma_interrupt_vector_r()
{
	return 0xc2;
}

uint8_t p20_state::job_interrupt_vector_r()
{
	return 0xc1;
}

void p20_state::machine_start()
{
	save_item(NAME(m_csr_scsi_count));
	save_item(NAME(m_csr_scsi_pointer));
	save_item(NAME(m_csr_scsi));
	save_item(NAME(m_csr_misc));
	save_item(NAME(m_csr_kill));
	save_item(NAME(m_csr_trce));
	save_item(NAME(m_csr_mapid));
}

void p20_state::machine_reset()
{
	// reset forces A23 high for both cpus
	m_dma_boot.select(0);
	m_job_boot.select(0);

	// the job cpu is held in reset
	m_dmacpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	m_jobcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static DEVICE_INPUT_DEFAULTS_START( terminal_defaults )
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_2)
DEVICE_INPUT_DEFAULTS_END

void p20_state::p20(machine_config &config)
{
	M68010(config, m_dmacpu, 20_MHz_XTAL / 2); // clock guessed
	m_dmacpu->set_addrmap(AS_PROGRAM, &p20_state::dma_map);
	m_dmacpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &p20_state::dma_cpu_space_map);

	M68010(config, m_jobcpu, 20_MHz_XTAL / 2); // clock guessed
	m_jobcpu->set_addrmap(AS_PROGRAM, &p20_state::job_map);
	m_jobcpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &p20_state::job_cpu_space_map);

	INPUT_MERGER_ANY_HIGH(config, m_sio_irqs).output_handler().set_inputline(m_dmacpu, M68K_IRQ_5);

	MK68564(config, m_sio[0], 3'000'000);
	m_sio[0]->set_xtal(9.8304_MHz_XTAL / 4);
	m_sio[0]->out_int_callback().set(m_sio_irqs, FUNC(input_merger_device::in_w<0>));
	m_sio[0]->out_txdb_callback().set(m_serial0, FUNC(rs232_port_device::write_txd));
	m_sio[0]->out_rtsb_callback().set(m_serial0, FUNC(rs232_port_device::write_rts));

	MK68564(config, m_sio[1], 3'000'000);
	m_sio[1]->set_xtal(9.8304_MHz_XTAL / 4);
	m_sio[1]->out_int_callback().set(m_sio_irqs, FUNC(input_merger_device::in_w<1>));

	MK68564(config, m_sio[2], 3'000'000);
	m_sio[2]->out_int_callback().set(m_sio_irqs, FUNC(input_merger_device::in_w<2>));
	m_sio[2]->set_xtal(9.8304_MHz_XTAL / 4);

	MK68564(config, m_sio[3], 3'000'000);
	m_sio[3]->out_int_callback().set(m_sio_irqs, FUNC(input_merger_device::in_w<3>));
	m_sio[3]->set_xtal(9.8304_MHz_XTAL / 4);

	RS232_PORT(config, m_serial0, default_rs232_devices, "terminal");
	m_serial0->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal_defaults));
	m_serial0->rxd_handler().set(m_sio[0], FUNC(mk68564_device::rxb_w));
	m_serial0->cts_handler().set(m_sio[0], FUNC(mk68564_device::ctsb_w));

	MC146818(config, m_rtc, 32.768_kHz_XTAL); // XTAL unreadable, clock guessed

	config.set_default_layout(layout_p20);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( p20 )
	ROM_REGION16_BE(0x10000, "firmware", 0)
	ROM_LOAD16_BYTE("p20_3_4-4.u17k", 0x0000, 0x4000, CRC(43cd5b79) SHA1(ca4664581e1f1e890ada53dbdaf5a38e49f0f6a0)) // 3/4-4
	ROM_LOAD16_BYTE("p20_3_3-4.u17l", 0x0001, 0x4000, CRC(275ad162) SHA1(0f13a50b422c742e933e73a0dbb47ee18995930e)) // 3/3-4
	ROM_LOAD16_BYTE("p20.u15k",       0x8000, 0x4000, CRC(e64d1e4f) SHA1(f346c12652a6b264b90bde2281cd9a9367e779d8)) // no label
	ROM_LOAD16_BYTE("p20.u15l",       0x8001, 0x4000, CRC(425c72b1) SHA1(8779e237225fc5a425fc82ac30b03c7d9fc78035)) // no label

	ROM_REGION(0x4000, "omti", 0)
	ROM_LOAD("omtie9.8h", 0x0000, 0x4000, CRC(8b4a6e21) SHA1(b5a781f87e26dc2aab81e3581312524f11512849))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY   FULLNAME  FLAGS
COMP( 1985, p20,  0,      0,      p20,     p20,   p20_state, empty_init, "Plexus", "P/20",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
