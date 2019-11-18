// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9895.cpp

    HP9895 floppy disk drive

    Phew, this one was tough!

    This is a dual 8" floppy disk drive that interfaces through
    HPIB/IEEE-488 bus. It implements the so-called "Amigo" command
    set.

    Its main components are:
    * A Z80A CPU @ 4 MHz with 8 kB of firmware ROM and 1 kB of
      static RAM
    * A HP PHI chip that interfaces CPU to HPIB bus
    * A disk controller implemented with a lot of discrete TTLs
    * 2 MPI 8" disk drives

    Data I/O with the disk is carried out through 2 shift registers,
    one for data bits (@ 0x60 address) and one for clock bits (@ 0x61
    address). CPU is stalled by setting WAIT/ to 0 whenever it accesses
    the data register and the hw is not ready for the byte. Once
    the next byte boundary is reached (the SDOK signal activates) the
    CPU is released and either the data byte is read from shift register
    or written into it. At the same time clock shift register is
    copied into clock register when reading or viceversa when writing.

    The 9895 drive can operate in 2 modes: HP/High density or IBM/low
    density. This table summarizes the differences between the modes.
    See also page 2-12 of service manual.

    | Characteristic | HP mode  | IBM mode  |
    |----------------+----------+-----------|
    | Bit cell size  | 2 µs     | 4 µs      |
    | Modulation     | MMFM     | FM        |
    | Bit order      | LS first | MS first  |
    | Sync bytes     | 4x FF    | 6x 00     |
    | Formatted size | 1155 kB  | 250.25 kB |

    Reference manual:
    HP 09895-90030, feb 81, 9895A Flexible Disc Memory Service Manual

    Reference manual for the floppy drives:
    Magnetic Peripherals, inc., feb 83, 9406-4 Flexible Disk Drive
    Hardware Maintenance Manual

    TODO/Issues:
    * floppy_image_device sometimes reports the wrong state for wpt
      signal
    * IBM mode hasn't been tested yet

*********************************************************************/

#include "emu.h"
#include "hp9895.h"
#include "formats/hpi_dsk.h"

// Debugging
#define VERBOSE 1
#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)
#define VERBOSE_0 0
#define LOG_0(x)  do { if (VERBOSE_0) logerror x; } while (0)

// Macros to clear/set single bits
#define BIT_MASK(n) (1U << (n))
#define BIT_CLR(w , n)  ((w) &= ~BIT_MASK(n))
#define BIT_SET(w , n)  ((w) |= BIT_MASK(n))

// Bits in RESET register
#define REG_RESET_TIMEOUT_START_BIT 0   // Start TIMEOUT oneshot (1)
#define REG_RESET_OVERUN_CLEAR_BIT  1   // Clear OVERUN (sic) (1)
#define REG_RESET_PROGRES_BIT       3   // PROGRES (1)

// Bits in CNTL register
#define REG_CNTL_READON_BIT         1   // Enable reading (1)
#define REG_CNTL_WRITON_BIT         2   // Enable writing (1)
#define REG_CNTL_WRITDRV_BIT        3   // Enable writing to floppy (1)
#define REG_CNTL_CRCOUT_BIT         4   // Enable output of CRC word (1)
#define REG_CNTL_CRCON_BIT          5   // Enable updating of CRC word (1) or preset CRC to 0xffff (0)

// Bits in DRV register
#define REG_DRV_STEP_BIT            0   // Step pulse to drive (1)
#define REG_DRV_MOVEIN_BIT          1   // Move heads inward (1)
#define REG_DRV_MGNENA_BIT          2   // Enable checking of bit cell margins (1)
#define REG_DRV_IN_USE_BIT          3   // "In use" signal to drive (1)
#define REG_DRV_LOWCURR_BIT         4   // Reduce write current in inner tracks (1)
#define REG_DRV_HEADSEL_BIT         7   // Head selection (1 = Head 1)

// Bits in XV register
#define REG_XV_DRIVE3_BIT           0   // Select drive #3 (1)
#define REG_XV_DRIVE2_BIT           1   // Select drive #2 (1)
#define REG_XV_DRIVE1_BIT           2   // Select drive #1 (1)
#define REG_XV_DRIVE0_BIT           3   // Select drive #0 (1)
#define REG_XV_HIDEN_BIT            4   // Select HP/High density mode (1) or IBM/Low density mode (0)
#define REG_XV_PRECMP_BIT           5   // Enable pre-compensation

// Bits in DRIVSTAT register
#define REG_DRIVSTAT_INDEX_BIT      0   // Index pulse from drive (1)
#define REG_DRIVSTAT_DISCHNG_BIT    1   // Disk changed (1)
#define REG_DRIVSTAT_TRACK0_BIT     2   // Heads on track #0 (1)
#define REG_DRIVSTAT_WRPROT_BIT     3   // Disk is write-protected (1)
#define REG_DRIVSTAT_READY_BIT      4   // Disk is ready (1)
#define REG_DRIVSTAT_CRCERR_BIT     5   // Error in CRC (1)
#define REG_DRIVSTAT_OVERUN_BIT     6   // I/O overrun between disk and CPU (1)
#define REG_DRIVSTAT_TWOSIDE_BIT    7   // 2-sided disk (1)

// Bits in SWITCHES(2) registers
#define REG_SWITCHES_HPIB_ADDR_SHIFT    0   // LSB of HPIB address
#define REG_SWITCHES_HPIB_ADDR_MASK     7   // Mask of HPIB address
#define REG_SWITCHES_W_TEST_BIT     3   // "W" test push-button (1)
#define REG_SWITCHES_S_TEST_BIT     4   // "S" test push-button (1)
#define REG_SWITCHES_LOOP_BIT       5   // Test loop option (1)
#define REG_SWITCHES_TIMEOUT_BIT    6   // TIMEOUT (1)
#define REG_SWITCHES_AMDT_BIT       7   // Address mark detected (1)

// Timers
enum {
	TIMEOUT_TMR_ID,
	BYTE_TMR_ID,
	HALF_BIT_TMR_ID
};

// Timings
#define TIMEOUT_MSEC        450     // Timeout duration (ms)
#define HPMODE_BIT_FREQ     500000  // HP-mode bit frequency (Hz)
#define IBMMODE_BIT_FREQ    250000  // IBM-mode bit frequency (Hz)

#define MIN_SYNC_BITS       29      // Number of bits to synchronize

// device type definition
DEFINE_DEVICE_TYPE(HP9895, hp9895_device, "hp9895", "HP9895")

// Masks of drive selectors in XV register
static const uint8_t xv_drive_masks[] = {
	BIT_MASK(REG_XV_DRIVE0_BIT),
	BIT_MASK(REG_XV_DRIVE1_BIT)
};

hp9895_device::hp9895_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP9895, tag, owner, clock),
	  device_ieee488_interface(mconfig, *this),
	  m_cpu(*this , "cpu"),
	  m_phi(*this , "phi"),
	  m_drives{{*this , "floppy0"} , {*this , "floppy1"}},
	  m_switches{*this , "switches"}
{
}

static INPUT_PORTS_START(hp9895_port)
	PORT_START("switches")
	PORT_CONFNAME(REG_SWITCHES_HPIB_ADDR_MASK << REG_SWITCHES_HPIB_ADDR_SHIFT , 0x00 , "HPIB address")
	PORT_CONFSETTING(0 << REG_SWITCHES_HPIB_ADDR_SHIFT , "0")
	PORT_CONFSETTING(1 << REG_SWITCHES_HPIB_ADDR_SHIFT , "1")
	PORT_CONFSETTING(2 << REG_SWITCHES_HPIB_ADDR_SHIFT , "2")
	PORT_CONFSETTING(3 << REG_SWITCHES_HPIB_ADDR_SHIFT , "3")
	PORT_CONFSETTING(4 << REG_SWITCHES_HPIB_ADDR_SHIFT , "4")
	PORT_CONFSETTING(5 << REG_SWITCHES_HPIB_ADDR_SHIFT , "5")
	PORT_CONFSETTING(6 << REG_SWITCHES_HPIB_ADDR_SHIFT , "6")
	PORT_CONFSETTING(7 << REG_SWITCHES_HPIB_ADDR_SHIFT , "7")
	PORT_CONFNAME(BIT_MASK(REG_SWITCHES_W_TEST_BIT) , 0x00 , "W Test")
	PORT_CONFSETTING(0x00 , DEF_STR(Off))
	PORT_CONFSETTING(BIT_MASK(REG_SWITCHES_W_TEST_BIT) , DEF_STR(On))
	PORT_CONFNAME(BIT_MASK(REG_SWITCHES_S_TEST_BIT) , 0x00 , "S Test")
	PORT_CONFSETTING(0x00 , DEF_STR(Off))
	PORT_CONFSETTING(BIT_MASK(REG_SWITCHES_S_TEST_BIT) , DEF_STR(On))
	PORT_CONFNAME(BIT_MASK(REG_SWITCHES_LOOP_BIT) , 0x00 , "Loop")
	PORT_CONFSETTING(0x00 , DEF_STR(Off))
	PORT_CONFSETTING(BIT_MASK(REG_SWITCHES_LOOP_BIT) , DEF_STR(On))
INPUT_PORTS_END

ioport_constructor hp9895_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp9895_port);
}

void hp9895_device::device_start()
{
	save_item(NAME(m_cpu_irq));
	save_item(NAME(m_current_drive_idx));
	save_item(NAME(m_dskchg));
	save_item(NAME(m_crc));
	save_item(NAME(m_crcerr_syn));
	save_item(NAME(m_overrun));
	save_item(NAME(m_accdata));
	save_item(NAME(m_timeout));
	save_item(NAME(m_cntl_reg));
	save_item(NAME(m_clock_sr));
	save_item(NAME(m_clock_reg));
	save_item(NAME(m_data_sr));
	save_item(NAME(m_wr_context));
	save_item(NAME(m_had_transition));
	save_item(NAME(m_lckup));
	save_item(NAME(m_amdt));
	save_item(NAME(m_sync_cnt));
	save_item(NAME(m_hiden));
	save_item(NAME(m_mgnena));

	m_timeout_timer = timer_alloc(TIMEOUT_TMR_ID);
	m_byte_timer = timer_alloc(BYTE_TMR_ID);
	m_half_bit_timer = timer_alloc(HALF_BIT_TMR_ID);

	for (auto& d : m_drives) {
		d->get_device()->setup_ready_cb(floppy_image_device::ready_cb(&hp9895_device::floppy_ready_cb , this));
	}
}

void hp9895_device::device_reset()
{
	m_cpu_irq = false;
	m_current_drive = nullptr;
	m_current_drive_idx = ~0;
	for (auto& d : m_dskchg) {
		d = true;
	}
	preset_crc();
	m_crcerr_syn = false;
	m_overrun = false;
	m_accdata = false;
	m_timeout = true;
	m_cntl_reg = 0;
	m_clock_sr = 0;
	m_clock_reg = 0;
	m_data_sr = 0;
	m_wr_context = 0;
	m_had_transition = false;
	m_lckup = true; // Because READON = 0
	m_amdt = false;
	m_sync_cnt = 0;
	m_hiden = false;
	m_mgnena = false;
	m_timeout_timer->reset();
	m_byte_timer->reset();
	m_half_bit_timer->reset();
}

void hp9895_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id) {
	case TIMEOUT_TMR_ID:
		LOG(("Timeout!\n"));
		m_timeout = true;
		if (m_mgnena) {
			// CPU is resumed by timeout if MGNENA=1
			m_cpu->trigger(1);
		}
		break;

	case BYTE_TMR_ID:
		{
			if (m_accdata) {
				// Resume CPU when it's waiting for SDOK
				m_cpu->trigger(1);
			} else {
				// No access to data register by CPU
				LOG(("Data overrun!\n"));
				m_overrun = true;
			}
			m_accdata = false;

			m_crcerr_syn = m_crc != 0;

			if (!BIT(m_cntl_reg , REG_CNTL_CRCON_BIT)) {
				// CRC not enabled, keep it in preset state (all ones)
				preset_crc();
			}

			attotime sdok_time{machine().time()};
			LOG_0(("SDOK @ %.06f\n" , sdok_time.as_double()));
			bool do_crc_upd = true;
			if (BIT(m_cntl_reg , REG_CNTL_WRITON_BIT)) {
				// Writing
				m_pll.commit(get_write_device() , sdok_time);
				m_pll.ctime = sdok_time;

				// Check for AMDT when in loopback mode
				if (!m_lckup && !m_amdt && BIT(m_cntl_reg , REG_CNTL_READON_BIT)) {
					if (m_hiden) {
						m_amdt = m_data_sr != 0xff;
					} else {
						m_amdt = m_data_sr != 0;
					}
				}

				LOG_0(("WR D=%02x/C=%02x\n" , m_data_sr , m_clock_sr));
				do_crc_upd = false;
				for (unsigned i = 0; i < 8; i++) {
					bool clock_bit;
					bool data_bit;

					clock_bit = shift_sr(m_clock_sr, false);
					data_bit = shift_sr(m_data_sr, true);

					if (BIT(m_cntl_reg , REG_CNTL_CRCOUT_BIT)) {
						// Substitute data bits from DSR with those from CRC when CRCOUT=1
						data_bit = BIT(m_crc , 15);
						m_crc <<= 1;
					} else if (BIT(m_cntl_reg , REG_CNTL_CRCON_BIT)) {
						// Update CRC
						update_crc(data_bit);
					}
					write_bit(data_bit, clock_bit);
				}
				// When shifting is done DSR is filled with 1s and CSR with 0s
			}
			if (BIT(m_cntl_reg , REG_CNTL_READON_BIT)) {
				// Reading
				m_pll.ctime = sdok_time;

				for (unsigned i = 0; i < 8; i++) {
					read_bit(do_crc_upd);
				}
				LOG_0(("RD D=%02x/C=%02x\n" , m_data_sr , m_clock_sr));
			}
			LOG_0(("next SDOK @ %.06f\n" , m_pll.ctime.as_double()));
			timer.adjust(m_pll.ctime - sdok_time);
		}
		break;

	case HALF_BIT_TMR_ID:
		{
			m_pll.ctime = machine().time();
			if (m_lckup) {
				// Trying to lock on synchronization bytes
				attotime edge;
				attotime tm;
				get_next_transition(m_pll.ctime, edge);
				bool half_bit0 = m_pll.feed_read_data(tm , edge , attotime::never);
				get_next_transition(m_pll.ctime, edge);
				bool half_bit1 = m_pll.feed_read_data(tm , edge , attotime::never);
				if (half_bit0 == half_bit1) {
					// If half bits are equal, no synch
					LOG_0(("Reset sync_cnt\n"));
					m_sync_cnt = 0;
				} else if (++m_sync_cnt >= MIN_SYNC_BITS) {
					// Synchronized, now wait for AM
					LOG_0(("Synchronized @ %.6f\n" , machine().time().as_double()));
					m_lckup = false;
					if (BIT(m_cntl_reg , REG_CNTL_WRITON_BIT)) {
						// When loopback is active, leave AM detection to byte timer as
						// byte boundary is already synchronized
						timer.reset();
						return;
					} else {
						// Align with bit cell
						// Synchronization bits in HP mode: 32x 1s -> C/D bits = 01010101...
						// Synchronization bits in IBM mode: 32x 0s -> C/D bits = 10101010...
						if (m_hiden != half_bit1) {
							// Discard 1/2 bit cell if synchronization achieved in the clock part
							get_next_transition(m_pll.ctime, edge);
							m_pll.feed_read_data(tm , edge , attotime::never);
						}
						// Load CSR & DSR as they are after synchronization bits
						if (m_hiden) {
							m_clock_sr = 0;
							m_data_sr = ~0;
						} else {
							m_clock_sr = ~0;
							m_data_sr = 0;
						}
					}
				}
			} else {
				// Looking for AM
				/// CRC is not updated because it can't be possibly enabled at this point
				read_bit(false);
				if ((m_hiden && !BIT(m_data_sr , 7)) ||
					(!m_hiden && BIT(m_data_sr , 0))) {
					// Got AM as soon as bits being shifted into DSR change value wrt synchronization bits
					m_amdt = true;
					// Finish the current byte
					for (unsigned i = 0; i < 7; i++) {
						read_bit(false);
					}
					attotime adjust{m_pll.ctime - machine().time()};
					LOG_0(("Got AM @ %.6f, ctime=%.6f, adj=%.6f, D=%02x/C=%02x\n" , machine().time().as_double() , m_pll.ctime.as_double() , adjust.as_double() , m_data_sr , m_clock_sr));
					// Disable half-bit timer & enable byte timer
					timer.reset();
					m_byte_timer->adjust(adjust);
					return;
				}
			}
			timer.adjust(m_pll.ctime - machine().time());
		}
		break;

	default:
		break;
	}
}

void hp9895_device::ieee488_eoi(int state)
{
	m_phi->eoi_w(state);
}

void hp9895_device::ieee488_dav(int state)
{
	m_phi->dav_w(state);
}

void hp9895_device::ieee488_nrfd(int state)
{
	m_phi->nrfd_w(state);
}

void hp9895_device::ieee488_ndac(int state)
{
	m_phi->ndac_w(state);
}

void hp9895_device::ieee488_ifc(int state)
{
	m_phi->ifc_w(state);
}

void hp9895_device::ieee488_srq(int state)
{
	m_phi->srq_w(state);
}

void hp9895_device::ieee488_atn(int state)
{
	m_phi->atn_w(state);
}

void hp9895_device::ieee488_ren(int state)
{
	m_phi->ren_w(state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_eoi_w)
{
	m_bus->eoi_w(this , state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_dav_w)
{
	m_bus->dav_w(this , state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_nrfd_w)
{
	m_bus->nrfd_w(this , state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_ndac_w)
{
	m_bus->ndac_w(this , state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_ifc_w)
{
	m_bus->ifc_w(this , state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_srq_w)
{
	m_bus->srq_w(this , state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_atn_w)
{
	m_bus->atn_w(this , state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_ren_w)
{
	m_bus->ren_w(this , state);
}

READ8_MEMBER(hp9895_device::phi_dio_r)
{
	return m_bus->read_dio();
}

WRITE8_MEMBER(hp9895_device::phi_dio_w)
{
	m_bus->dio_w(this , data);
}

WRITE_LINE_MEMBER(hp9895_device::phi_int_w)
{
	m_cpu->set_input_line(INPUT_LINE_NMI , state);
	if (state) {
		// Ensure the event queue is emptied before executing any other instruction
		m_cpu->yield();
	}
}

READ8_MEMBER(hp9895_device::phi_reg_r)
{
	uint16_t reg = m_phi->reg16_r(space , offset , mem_mask);

	// Reading D1=1 from a register sets the Z80 IRQ line
	if (BIT(reg , 14) && !m_cpu_irq) {
		m_cpu_irq = true;
		m_cpu->set_input_line(INPUT_LINE_IRQ0 , ASSERT_LINE);
	}

	return (uint8_t)reg;
}

WRITE8_MEMBER(hp9895_device::z80_m1_w)
{
	// Every M1 cycle of Z80 clears the IRQ line
	if (m_cpu_irq) {
		m_cpu_irq = false;
		m_cpu->set_input_line(INPUT_LINE_IRQ0 , CLEAR_LINE);
	}
}

WRITE8_MEMBER(hp9895_device::data_w)
{
	LOG_0(("W DATA=%02x\n" , data));
	// CPU stalls until next SDOK
	m_cpu->suspend_until_trigger(1 , true);
	m_data_sr = data;
	m_clock_sr = m_clock_reg;
	m_accdata = true;
}

WRITE8_MEMBER(hp9895_device::clock_w)
{
	LOG_0(("W CLOCK=%02x\n" , data));
	m_clock_reg = data;
}

WRITE8_MEMBER(hp9895_device::reset_w)
{
	LOG_0(("W RESET=%02x\n" , data));
	if (BIT(data , REG_RESET_TIMEOUT_START_BIT)) {
		m_timeout = false;
		m_timeout_timer->adjust(attotime::from_msec(TIMEOUT_MSEC));
	}
	if (BIT(data , REG_RESET_OVERUN_CLEAR_BIT)) {
		m_overrun = false;
	}
	// TODO: PROGRES
}

WRITE8_MEMBER(hp9895_device::leds_w)
{
	LOG(("W LEDS=%02x %c%c%c%c%c\n" , data , BIT(data , 4) ? '.' : '*' , BIT(data , 3) ? '.' : '*' , BIT(data , 2) ? '.' : '*' , BIT(data , 1) ? '.' : '*' , BIT(data , 0) ? '.' : '*'));
	// TODO:
}

WRITE8_MEMBER(hp9895_device::cntl_w)
{
	if (data != m_cntl_reg) {
		LOG_0(("W CNTL=%02x -> %02x\n" , m_cntl_reg , data));
		uint8_t old_cntl_reg = m_cntl_reg;
		m_cntl_reg = data;

		bool old_writon = BIT(old_cntl_reg , REG_CNTL_WRITON_BIT);
		bool new_writon = BIT(m_cntl_reg , REG_CNTL_WRITON_BIT);
		bool old_readon = BIT(old_cntl_reg , REG_CNTL_READON_BIT);
		bool new_readon = BIT(m_cntl_reg , REG_CNTL_READON_BIT);

		bool byte_timer_running = old_writon || m_amdt;
		bool byte_timer_needed = new_writon || (new_readon && m_amdt);

		if (!byte_timer_running && byte_timer_needed) {
			LOG_0(("Enable byte tmr\n"));
			attotime byte_period = get_half_bit_cell_period() * 16;
			m_byte_timer->adjust(byte_period);
		} else if (byte_timer_running && !byte_timer_needed) {
			LOG_0(("Disable byte tmr\n"));
			m_byte_timer->reset();
		}

		if (!old_writon && !old_readon && (new_writon || new_readon)) {
			m_pll.set_clock(get_half_bit_cell_period());
		}

		if (!old_writon && new_writon) {
			// Writing enabled
			LOG_0(("Start writing..\n"));
			m_pll.start_writing(machine().time());
			m_wr_context = 0;
			m_had_transition = false;
		} else if (old_writon && !new_writon) {
			// Writing disabled
			LOG_0(("Stop writing..\n"));
			m_pll.stop_writing(get_write_device() , machine().time());
		}
		if (!old_readon && new_readon) {
			// Reading enabled
			LOG_0(("Start reading..\n"));
			m_pll.read_reset(machine().time());
			m_sync_cnt = 0;
			m_half_bit_timer->adjust(get_half_bit_cell_period());
		} else if (old_readon && !new_readon) {
			// Reading disabled
			LOG_0(("Stop reading..\n"));
			m_half_bit_timer->reset();
			m_lckup = true;
			m_amdt = false;
		}
		if (!new_readon && !new_writon) {
			m_crcerr_syn = false;
			BIT_CLR(m_cntl_reg, REG_CNTL_CRCON_BIT);
			BIT_CLR(m_cntl_reg, REG_CNTL_CRCOUT_BIT);
			preset_crc();
		}
	}
}

WRITE8_MEMBER(hp9895_device::drv_w)
{
	LOG_0(("W DRV=%02x\n" , data));
	m_mgnena = BIT(data , REG_DRV_MGNENA_BIT);
	if (m_current_drive != nullptr) {
		m_current_drive->stp_w(!BIT(data , REG_DRV_STEP_BIT));
		m_current_drive->dir_w(!BIT(data , REG_DRV_MOVEIN_BIT));
		// TODO: in use signal
		m_current_drive->ss_w(BIT(data , REG_DRV_HEADSEL_BIT));
	}
}

WRITE8_MEMBER(hp9895_device::xv_w)
{
	LOG_0(("W XV=%02x\n" , data));
	// Disk Changed flag is cleared when drive is ready and it is deselected
	if (m_current_drive_idx < 2 && (data & xv_drive_masks[ m_current_drive_idx ]) == 0 && !m_current_drive->ready_r()) {
		if (m_dskchg[ m_current_drive_idx ]) {
			LOG(("Dskchg %u cleared\n" , m_current_drive_idx));
		}
		m_dskchg[ m_current_drive_idx ] = false;
	}

	m_current_drive = nullptr;
	m_current_drive_idx = ~0;
	for (unsigned i = 0; i < 2; i++) {
		if (data & xv_drive_masks[ i ]) {
			m_current_drive = m_drives[ i ]->get_device();
			m_current_drive_idx = i;
			break;
		}
	}

	m_hiden = BIT(data , REG_XV_HIDEN_BIT);
}

READ8_MEMBER(hp9895_device::data_r)
{
	m_clock_reg = m_clock_sr;
	m_accdata = true;
	LOG_0(("R DATA=%02x\n" , m_data_sr));
	// CPU stalls until next SDOK
	m_cpu->suspend_until_trigger(1 , true);
	return m_data_sr;
}

READ8_MEMBER(hp9895_device::clock_r)
{
	return m_clock_reg;
}

READ8_MEMBER(hp9895_device::drivstat_r)
{
	uint8_t res = 0;

	if (m_current_drive != nullptr) {
		if (m_current_drive->idx_r()) {
			BIT_SET(res , REG_DRIVSTAT_INDEX_BIT);
		}
		if (m_dskchg[ m_current_drive_idx ]) {
			BIT_SET(res , REG_DRIVSTAT_DISCHNG_BIT);
		}
		if (!m_current_drive->trk00_r()) {
			BIT_SET(res , REG_DRIVSTAT_TRACK0_BIT);
		}
		if (m_current_drive->wpt_r()) {
			BIT_SET(res , REG_DRIVSTAT_WRPROT_BIT);
		}
		if (!m_current_drive->ready_r()) {
			BIT_SET(res , REG_DRIVSTAT_READY_BIT);
		}
		if (!m_current_drive->twosid_r()) {
			BIT_SET(res , REG_DRIVSTAT_TWOSIDE_BIT);
		}
	}
	if (m_crcerr_syn) {
		BIT_SET(res , REG_DRIVSTAT_CRCERR_BIT);
	}
	if (m_overrun) {
		BIT_SET(res , REG_DRIVSTAT_OVERUN_BIT);
	}
	LOG_0(("R DRIVSTAT=%02x\n" , res));
	return res;
}

READ8_MEMBER(hp9895_device::switches_r)
{
	uint8_t res = get_switches2();
	res |= m_switches->read();
	return res;
}

READ8_MEMBER(hp9895_device::switches2_r)
{
	return get_switches2();
}

void hp9895_device::floppy_ready_cb(floppy_image_device *floppy , int state)
{
	if (state) {
		// Set Disk Changed flag when a drive is not ready
		for (unsigned i = 0; i < 2; i++) {
			if (floppy == m_drives[ i ]->get_device()) {
				LOG(("Dskchg %u set\n" , i));
				m_dskchg[ i ] = true;
				break;
			}
		}
	}
}

uint8_t hp9895_device::get_switches2(void) const
{
	uint8_t res = 0;

	if (m_timeout) {
		BIT_SET(res, REG_SWITCHES_TIMEOUT_BIT);
	}
	if (m_amdt) {
		BIT_SET(res, REG_SWITCHES_AMDT_BIT);
	}

	return res;
}

attotime hp9895_device::get_half_bit_cell_period(void) const
{
	return attotime::from_hz((m_hiden ? HPMODE_BIT_FREQ : IBMMODE_BIT_FREQ) * 2);
}

floppy_image_device *hp9895_device::get_write_device(void) const
{
	if (!BIT(m_cntl_reg , REG_CNTL_WRITDRV_BIT)) {
		return nullptr;
	} else {
		return m_current_drive;
	}
}

void hp9895_device::preset_crc(void)
{
	m_crc = ~0;
}

void hp9895_device::update_crc(bool bit)
{
	bool msb = BIT(m_crc , 15);

	m_crc <<= 1;
	if (bit ^ msb) {
		m_crc ^= 0x1021;
	}
}

bool hp9895_device::shift_sr(uint8_t& sr , bool input_bit)
{
	bool res;

	if (m_hiden) {
		res = BIT(sr , 0);
		sr >>= 1;
		if (input_bit) {
			BIT_SET(sr , 7);
		}
	} else {
		res = BIT(sr , 7);
		sr <<= 1;
		if (input_bit) {
			BIT_SET(sr , 0);
		}
	}
	return res;
}

void hp9895_device::get_next_transition(const attotime& from_when , attotime& edge)
{
	edge = attotime::never;

	if (BIT(m_cntl_reg , REG_CNTL_WRITON_BIT)) {
		// Loop back write transitions into reading data path
		for (int idx = 0; idx < m_pll.write_position; idx++) {
			if (m_pll.write_buffer[ idx ] >= from_when) {
				edge = m_pll.write_buffer[ idx ];
				break;
			}
		}
	} else if (m_current_drive != nullptr) {
		edge = m_current_drive->get_next_transition(from_when);
	}
}

void hp9895_device::read_bit(bool crc_upd)
{
	attotime edge;
	attotime tm;

	get_next_transition(m_pll.ctime, edge);
	bool clock_bit = m_pll.feed_read_data(tm , edge , attotime::never);
	get_next_transition(m_pll.ctime, edge);
	bool data_bit = m_pll.feed_read_data(tm , edge , attotime::never);

	shift_sr(m_clock_sr, clock_bit);
	data_bit = shift_sr(m_data_sr, data_bit);

	if (crc_upd &&
		BIT(m_cntl_reg , REG_CNTL_CRCON_BIT) &&
		!BIT(m_cntl_reg , REG_CNTL_CRCOUT_BIT)) {
		update_crc(data_bit);
	}
}

void hp9895_device::write_bit(bool data_bit , bool clock_bit)
{
	if (m_hiden) {
		// **** HP mode ****
		// m_wr_context delays data bits by 2 bit cells
		// Bit  Content
		// ============
		// 2    Data @ t-2
		// 1    Data @ t-1
		// 0    Data @ t
		m_wr_context = (m_wr_context << 1) | data_bit;
		data_bit = BIT(m_wr_context , 2);
		clock_bit = !data_bit && (clock_bit || !m_had_transition);
		m_had_transition = data_bit || clock_bit;
	}
	// else... IBM mode, nothing to do

	attotime dummy;

	m_pll.write_next_bit(clock_bit , dummy , nullptr , attotime::never);
	m_pll.write_next_bit(data_bit , dummy , nullptr , attotime::never);
}

ROM_START(hp9895)
	ROM_REGION(0x2000 , "cpu" , 0)
	ROM_LOAD("1818-1391a.bin" , 0 , 0x2000 , CRC(b50dbfb5) SHA1(96edf9af78be75fbad2a0245b8af43958ba32752))
ROM_END

void hp9895_device::z80_program_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom().region("cpu", 0);
	map(0x6000, 0x63ff).ram();
}

void hp9895_device::z80_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x10, 0x17).w("phi", FUNC(phi_device::reg8_w)).r(FUNC(hp9895_device::phi_reg_r));
	map(0x60, 0x60).rw(FUNC(hp9895_device::data_r), FUNC(hp9895_device::data_w));
	map(0x61, 0x61).rw(FUNC(hp9895_device::clock_r), FUNC(hp9895_device::clock_w));
	map(0x62, 0x62).rw(FUNC(hp9895_device::drivstat_r), FUNC(hp9895_device::reset_w));
	map(0x63, 0x63).rw(FUNC(hp9895_device::switches_r), FUNC(hp9895_device::leds_w));
	map(0x64, 0x64).w(FUNC(hp9895_device::cntl_w));
	map(0x65, 0x65).w(FUNC(hp9895_device::drv_w));
	map(0x66, 0x66).w(FUNC(hp9895_device::xv_w));
	map(0x67, 0x67).r(FUNC(hp9895_device::switches2_r));
}

static void hp9895_floppies(device_slot_interface &device)
{
	device.option_add("8dsdd" , FLOPPY_8_DSDD);
}

static const floppy_format_type hp9895_floppy_formats[] = {
	FLOPPY_MFI_FORMAT,
	FLOPPY_HPI_FORMAT,
	nullptr
};

const tiny_rom_entry *hp9895_device::device_rom_region() const
{
	return ROM_NAME(hp9895);
}

void hp9895_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_cpu, 4000000);
	m_cpu->set_addrmap(AS_PROGRAM, &hp9895_device::z80_program_map);
	m_cpu->set_addrmap(AS_IO, &hp9895_device::z80_io_map);
	m_cpu->refresh_cb().set(FUNC(hp9895_device::z80_m1_w));

	PHI(config, m_phi, 0);
	m_phi->eoi_write_cb().set(FUNC(hp9895_device::phi_eoi_w));
	m_phi->dav_write_cb().set(FUNC(hp9895_device::phi_dav_w));
	m_phi->nrfd_write_cb().set(FUNC(hp9895_device::phi_nrfd_w));
	m_phi->ndac_write_cb().set(FUNC(hp9895_device::phi_ndac_w));
	m_phi->ifc_write_cb().set(FUNC(hp9895_device::phi_ifc_w));
	m_phi->srq_write_cb().set(FUNC(hp9895_device::phi_srq_w));
	m_phi->atn_write_cb().set(FUNC(hp9895_device::phi_atn_w));
	m_phi->ren_write_cb().set(FUNC(hp9895_device::phi_ren_w));
	m_phi->dio_read_cb().set(FUNC(hp9895_device::phi_dio_r));
	m_phi->dio_write_cb().set(FUNC(hp9895_device::phi_dio_w));
	m_phi->int_write_cb().set(FUNC(hp9895_device::phi_int_w));
	m_phi->sys_cntrl_read_cb().set_constant(0);

	FLOPPY_CONNECTOR(config, "floppy0", hp9895_floppies, "8dsdd", hp9895_floppy_formats).set_fixed(true);
	FLOPPY_CONNECTOR(config, "floppy1", hp9895_floppies, "8dsdd", hp9895_floppy_formats).set_fixed(true);
}
