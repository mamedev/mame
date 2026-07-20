// license:BSD-3-Clause
// copyright-holders:Dave Rand
/***************************************************************************

    Colonial Data Services Corp. SB-80 (Single Board 80)

    Z80A-based CP/M 2.2 single-board computer.  Headless: console is a
    serial terminal on SIO channel A; no on-board video or keyboard in
    the configuration Colonial Data shipped.  The board has socketed-but-
    unpopulated positions for an optional video / extended-I/O sub-block
    (U46, U48-U50, U57-U66, U67-U75) which never reached production --
    those sockets are empty on every known board and on the 1982 BYTE
    review unit.

    Hardware (verified against the BIOS source, the BYTE November 1982
    review, and direct inspection of two production boards S/N 1692 and
    S/N 1693):

        Z80A CPU @ 4 MHz, IM 2 (I = $FF, vector table at $FF00)
        64 KB DRAM with 9th parity bit (parity error halts CPU; not
            modelled -- the no-op success case)
        WD1793 floppy controller, up to 4 drives, 8" or 5.25"
            command/status  $FC      track  $FD
            sector          $FE      data   $FF
            drive/density/side latch at $ED
            DRQ wired to Z80 /NMI (per-byte transfer via INI in the
            $0066 handler)
        Z80 SIO (dual): CH A console, CH B aux/RS232
            CH A data $F4, command $F6
            CH B data $F5, command $F7
            CH A baud generator $E9, CH B baud generator $E8
            default CH A 9600 baud, CH B 300 baud
        Z80 CTC, four channels:
            ports $F0-$F3
            CH 3 -> real-time clock tick into BIOS
        Z80 PIO present on board, two ports; one wired Centronics-style
            for the printer via the J1 riser daughtercard.  Not used by
            Dave's CBIOS, so not wired in this driver yet.
        256-byte bootstrap ROM chip = two parallel 128-byte programs: the
            cold-boot IPL (chip $00-$7F) and a standalone "POWER ON RESET"
            memory-test diagnostic (chip $80-$FF, ends in HLT).  The board
            reads the ROM through a 128-byte window at $0000-$007F with a
            bank bit selecting which half is visible; the diagnostic runs
            only when its half is banked down to $0000 (its own SIO table
            sits at chip $F2 = window offset $72, matching its LD HL,$0072).
            On a normal cold boot the IPL half occupies $0000-$007F (reset
            vector at $0000, DRQ/NMI handler at $0066, SIO init table at
            $006C) and $0080-$00FF is RAM (also the CP/M default DMA buffer);
            OUT ($EA),A then banks the window out to RAM.  The full 256-byte
            chip is dumped; only the IPL half is mapped (the diagnostic
            bank-select is a power-on mode, not modelled).

    Cold boot:

        $0000 IPL section
            select drive A (OUT $ED), set SP = $4180
            step in twice, RESTORE
            program SIO ch A from STB table at $006C
            set sector 1, issue READ command
            DRQ NMIs at $0066 move bytes into $4000
            JMP $4000 (the loaded BOOT routine from boot.asm)
        $4000 BOOT routine
            loads CP/M loader into $D400+ via more disk reads
            OUT ($EA) -- turn off boot ROM
            jump to BIOS at $EC00

    System banner (in Dave Rand's v1.22 CBIOS, 15 April 1984):

        "SB 80 CP/M 2.2 61K Version 1.22   Dave Rand   04/15/84"

    Cross-references:
        - BIOS source: github-davidlrand:roms/sb80/  (sb80.asm v1.22)
        - Disk image: sb80-dr.imd (Don Maslin archive, imaged 1993-02-09)

***************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/com8116.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"

#include "softlist_dev.h"

#include "formats/imd_dsk.h"
#include "formats/td0_dsk.h"


namespace {

class sb80_state : public driver_device
{
public:
	sb80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bootrom(*this, "bootrom")
		, m_ram(*this, "mainram")
		, m_bootbank(*this, "bootbank")
		, m_ctc(*this, "ctc")
		, m_sio(*this, "sio")
		, m_pio(*this, "pio")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_brg(*this, "brg")
	{ }

	void sb80(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void unboot_w(uint8_t data);
	void dsel_w(uint8_t data);
	void fdc_drq_w(int state);
	TIMER_CALLBACK_MEMBER(nmi_assert_tick);

	required_device<z80_device>           m_maincpu;
	required_region_ptr<uint8_t>          m_bootrom;
	required_shared_ptr<uint8_t>          m_ram;
	required_memory_bank                  m_bootbank;
	required_device<z80ctc_device>        m_ctc;
	required_device<z80sio_device>        m_sio;
	required_device<z80pio_device>        m_pio;
	required_device<fd1793_device>        m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<com8116_device>       m_brg;

	bool      m_boot_active = true;
	emu_timer *m_nmi_assert_timer = nullptr;
	attotime  m_drq_drop_time = attotime::zero;
	bool      m_nmi_assert_pending = false;
};


// $00-$7F: 128-byte bootstrap-ROM window at /RESET, banked out via OUT ($EA),A.
// The 256-byte chip holds two parallel 128-byte programs: the cold-boot IPL
// (chip $00-$7F) and a standalone power-on memory-test diagnostic (chip
// $80-$FF).  The board reads the ROM through a 128-byte window at $0000-$007F
// with a bank bit selecting which half appears there; the diagnostic runs only
// when banked down to $0000 (its own SIO table lives at chip $F2 = window
// offset $72, which is why its LD HL,$0072 resolves correctly).  So on a normal
// cold boot only the IPL half is visible at $00-$7F and $0080-$00FF is RAM
// (also the CP/M default DMA buffer).  We model the IPL window only; the
// diagnostic bank-select is a power-on mode, not used in the boot path.
void sb80_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("mainram");
	map(0x0000, 0x007f).bankr("bootbank");
}

void sb80_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	// $E8 / $E9 = dual COM8116-class BRG strobes (write-only, low nibble
	// selects one of 16 standard async rates per the COM5016/8116/WD1943-00
	// "STD" table at 5.0688 MHz).  $E9 = STT (drives FT -> SIO ch A TXC/RXC),
	// $E8 = STR (drives FR -> SIO ch B TXC/RXC).
	map(0xe8, 0xe8).w(m_brg, FUNC(com8116_device::str_w));
	map(0xe9, 0xe9).w(m_brg, FUNC(com8116_device::stt_w));

	// $EA = UNBOOT: any write here disables the boot ROM overlay.
	map(0xea, 0xea).w(FUNC(sb80_state::unboot_w));

	// $ED = drive / density / side select latch.
	map(0xed, 0xed).w(FUNC(sb80_state::dsel_w));

	// $F0-$F3 = Z80 CTC.
	map(0xf0, 0xf3).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));

	// $F4-$F7 = Z80 SIO.  The SB-80 puts the data ports below the control
	// ports (data $F4/$F5, control $F6/$F7), which is neither the ba_cd
	// nor the cd_ba helper -- map each port individually.
	map(0xf4, 0xf4).rw(m_sio, FUNC(z80sio_device::da_r), FUNC(z80sio_device::da_w));
	map(0xf5, 0xf5).rw(m_sio, FUNC(z80sio_device::db_r), FUNC(z80sio_device::db_w));
	map(0xf6, 0xf6).rw(m_sio, FUNC(z80sio_device::ca_r), FUNC(z80sio_device::ca_w));
	map(0xf7, 0xf7).rw(m_sio, FUNC(z80sio_device::cb_r), FUNC(z80sio_device::cb_w));

	// $FC-$FF = WD1793.
	map(0xfc, 0xff).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
}


void sb80_state::unboot_w(uint8_t data)
{
	if (m_boot_active)
	{
		m_bootbank->set_entry(1); // swing to RAM
		m_boot_active = false;
	}
}


// Drive / density / side select latch at $ED.
//   bits 0..3  drive 0..3 select (one-hot)
//   bit 4      side select (0 = head 0)
//   bit 5      density:   0 = FM (single density), 1 = MFM (double density)
//              -- inverted before driving the WD1793's active-low DDEN pin,
//              so the default 0 selects FM, which is what the boot ROM
//              expects for the IBM-3740 cold-boot track.
//   bit 6,7    reserved on this revision
void sb80_state::dsel_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;
	for (int i = 0; i < 4; i++)
		if (BIT(data, i))
			floppy = m_floppy[i]->get_device();
	m_fdc->set_floppy(floppy);
	m_fdc->dden_w(!BIT(data, 5));
	if (floppy)
		floppy->ss_w(BIT(data, 4));
}


// DRQ -> Z80 /NMI: the boot ROM's NMI handler at $0066 does EXAF / EXX /
// INI / EXX / EXAF / RETN for one byte per DRQ.  After the boot ROM banks
// out, the CBIOS replaces the $0066 vector with its own equivalent (the
// EXAF/EXX dance assumes atomic per-byte dispatch -- there's no protection
// against re-entry).
//
// Workaround for the MAME-side nested-NMI bug (see the CTC-cascade TODO
// above and mamedev/mame issue #15416): if a DRQ rising edge would arrive
// less than `nmi_settle` after the previous falling edge, defer the NMI
// line assertion via a timer.  That gap is what the FDC's next set_drq()
// can take to fire after the host's data_r drops drq -- in MAME the next
// rising edge can land within a microsecond or two of the falling edge,
// which is shorter than the BIOS NMI handler's ~12 us total execution
// time.  Real WD1793 pipeline silicon guarantees one byte-time of spacing
// between rising edges; until the upstream wd_fdc fix lands, we close the
// equivalent window here.
//
// 6 us is the empirically-tuned threshold: it's enough to cover the
// post-data_r tail of the BIOS handler (EXX/EXAF/RETN ~= 5.5 us at
// 4 MHz at the BIOS handler's instruction sequence) and short enough
// that, when the FDC is firing bytes at 25 us per byte at MFM 500 kbps,
// the deferred assertion still lands well before the next byte
// boundary -- no spurious LOST_DATA.  Tested at 8 us and observed the
// FDC raising LOST_DATA after the workaround pushed valid NMI service
// just past byte boundaries on CTC-INT-preempted byte arrivals; 6 us
// is the sweet spot where the nested-NMI window is closed without
// over-deferring legitimate dispatches.
void sb80_state::fdc_drq_w(int state)
{
	if (state)
	{
		// Rising edge.  Check whether we're inside the settle window.
		attotime now = machine().time();
		if (!m_drq_drop_time.is_zero()
		    && (now - m_drq_drop_time) < attotime::from_usec(6))
		{
			m_nmi_assert_pending = true;
			m_nmi_assert_timer->adjust(attotime::from_usec(6) - (now - m_drq_drop_time));
			return;
		}
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
	else
	{
		// Falling edge.  Clear NMI and cancel any pending deferred assert
		// (the chip won't have anything ready for the host until the next
		// byte boundary, which is well past our settle window).
		if (m_nmi_assert_pending)
		{
			m_nmi_assert_timer->reset();
			m_nmi_assert_pending = false;
		}
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		m_drq_drop_time = machine().time();
	}
}

TIMER_CALLBACK_MEMBER(sb80_state::nmi_assert_tick)
{
	if (m_nmi_assert_pending)
	{
		m_nmi_assert_pending = false;
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
}


static const z80_daisy_config sb80_daisy_chain[] =
{
	{ "ctc" },
	{ "sio" },
	{ "pio" },
	{ nullptr }
};


static void sb80_floppies(device_slot_interface &device)
{
	device.option_add("8dsdd", FLOPPY_8_DSDD);
	device.option_add("8ssdd", FLOPPY_8_SSDD);
}


static void sb80_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_IMD_FORMAT);
	fr.add(FLOPPY_TD0_FORMAT);
}


void sb80_state::machine_start()
{
	m_bootbank->configure_entry(0, &m_bootrom[0]);    // /RESET state
	m_bootbank->configure_entry(1, &m_ram[0]);        // post-UNBOOT
	m_nmi_assert_timer = timer_alloc(FUNC(sb80_state::nmi_assert_tick), this);
	save_item(NAME(m_boot_active));
	save_item(NAME(m_drq_drop_time));
	save_item(NAME(m_nmi_assert_pending));
}


void sb80_state::machine_reset()
{
	m_bootbank->set_entry(0);
	m_boot_active = true;

	// The Shugart 800/850-series 8" drives Colonial Data shipped (and that
	// the boot ROM was written against) use a 120V AC spindle motor that
	// runs continuously whenever the drive is powered -- there is no
	// motor-control signal from the host, hence no motor-on bit anywhere
	// in the drive-select latch.  Assert mon_w(0) on every populated drive
	// so the WD1793 sees READY and can complete type-I commands.
	for (auto &fc : m_floppy)
		if (floppy_image_device *f = fc->get_device())
			f->mon_w(0);
}


void sb80_state::sb80(machine_config &config)
{
	// 4 MHz Z80A per the BYTE 1982 review.
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sb80_state::mem_map);
	m_maincpu->set_addrmap(AS_IO,      &sb80_state::io_map);
	m_maincpu->set_daisy_config(sb80_daisy_chain);

	// CTC: clocked at the CPU rate.  The BIOS' real-time-clock chain wants
	// ch 2 in timer mode (x16 prescaler, divide by 25) producing a 10 kHz
	// pulse train, cascaded into ch 3 in counter mode (divide by 249) to
	// yield a 40 Hz tick that drives `clock:` in the BIOS.  The cascade is
	// a physical board trace from ch 2's ZC/TO pin to ch 3's CLK/TRG pin.
	//
	// The cascade exposes a MAME-side bug -- NOT a flaw in this BIOS.
	// Real-hardware testimony from the BIOS author (the SB-80 booted
	// reliably with this exact CBIOS + CTC cascade + WD1793 + Z80 from
	// 04/15/84 onward, in production, no boot problems).  Per-EXX
	// instrumentation in MAME's Z80 captured the signature: pairs of
	// `[EXX] PC=0068 ... PC=0068 ...` -- two consecutive NMI-entry EXXs
	// with no PC=006B (exit EXX) in between, i.e. a nested NMI
	// dispatched before the first handler's exit EXX.  The nested EXX
	// swaps a state that's already swapped, the inner INI reads port
	// (caller's C, typically $00) instead of port $FF, and `$FF`
	// garbage gets scribbled at (caller's HL) inside BIOS working
	// memory.  For the nested NMI to happen, the second drq_cb(true)
	// must fire while the first NMI handler is still executing -- i.e.
	// within ~14 us of the previous data_r dropping DRQ.  Real WD1793
	// cannot do this: the bit-decoder pipeline guarantees >= one full
	// byte-time between successive DRQ rising edges.  MAME's wd_fdc
	// fires set_drq() from the live-FSM at byte-boundary timing, but
	// under heavy INT-preemption load (40 Hz CTC + tight FDC bit
	// timing) the rising-edge spacing can fall short.  Filed upstream
	// as mamedev/mame issue #15416.
	//
	// Until the wd_fdc fix lands, the driver-side workaround in
	// fdc_drq_w() gates the NMI line transitions with a 6 us settle
	// floor, which closes the nested-NMI window without over-deferring
	// legitimate dispatches.  bigbord2 doesn't surface this bug
	// because its floppy path is Z80DMA-driven, not NMI-per-byte -- so
	// DRQ races never hit the Z80 instruction stream.
	Z80CTC(config, m_ctc, 4_MHz_XTAL);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));

	// SIO: CH A console, CH B RS232 aux.
	Z80SIO(config, m_sio, 4_MHz_XTAL);
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// Baud-rate generator: a single dual-output BRG of the COM5016 /
	// COM8116 / WD1943-00 / AY-5-8116 "STD" family, fed by a 5.0688 MHz
	// crystal.  Its programmable code table at 16x clocks/baud matches the
	// BIOS' B3..B19 equates exactly (codes 5..F = 300, 600, 1200, 1800,
	// 2000, 2400, 3600, 4800, 7200, 9600, 19200 baud).  FT (transmit-side
	// output, programmed via STT on port $E9) clocks SIO ch A; FR (receive-
	// side output, programmed via STR on port $E8) clocks SIO ch B.  Each
	// SIO channel uses the same clock for TX and RX since the host code is
	// always async at the same baud both directions.
	COM8116(config, m_brg, 5.0688_MHz_XTAL);
	m_brg->ft_handler().set(m_sio, FUNC(z80sio_device::txca_w));
	m_brg->ft_handler().append(m_sio, FUNC(z80sio_device::rxca_w));
	m_brg->fr_handler().set(m_sio, FUNC(z80sio_device::txcb_w));
	m_brg->fr_handler().append(m_sio, FUNC(z80sio_device::rxcb_w));

	m_sio->out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_sio->out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	m_sio->out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	m_sio->out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	m_sio->out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));
	m_sio->out_dtrb_callback().set("rs232b", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	rs232a.cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));
	rs232a.dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
	rs232b.cts_handler().set(m_sio, FUNC(z80sio_device::ctsb_w));
	rs232b.dcd_handler().set(m_sio, FUNC(z80sio_device::dcdb_w));

	// PIO present on the board (Centronics on chan A via J1 riser) but not
	// wired by Dave's CBIOS; instantiate so the daisy chain is correct,
	// leave its handlers floating until the printer path is added.
	Z80PIO(config, m_pio, 4_MHz_XTAL);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// WD1793 + four 8" SSDD drives.  IBM-3740-style cold-boot tracks
	// (25x128 FM track 0 + MFM data tracks) and the all-MFM 8x1024
	// variant are both produced by the IMD and TD0 readers.
	FD1793(config, m_fdc, 2_MHz_XTAL); // 1MHz clock for 5.25", 2MHz for 8"
	m_fdc->drq_wr_callback().set(FUNC(sb80_state::fdc_drq_w));
	// INTRQ is not wired to the CPU in the IPL flow -- the BIOS polls.

	FLOPPY_CONNECTOR(config, m_floppy[0], sb80_floppies, "8dsdd", sb80_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], sb80_floppies, "8dsdd", sb80_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[2], sb80_floppies, nullptr, sb80_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[3], sb80_floppies, nullptr, sb80_floppy_formats);

	SOFTWARE_LIST(config, "flop_list").set_original("sb80_flop");
}


ROM_START( sb80 )
	ROM_REGION( 0x0100, "bootrom", 0 )
	ROM_LOAD( "bootrom.bin", 0x0000, 0x0100, CRC(fa521576) SHA1(32153db6f37e7f5666e89aa0b8712d79daafeb4b) )
ROM_END

} // anonymous namespace


//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY                          FULLNAME  FLAGS
COMP( 1982, sb80, 0,      0,      sb80,    0,     sb80_state,   empty_init, "Colonial Data Services",  "SB-80",  MACHINE_SUPPORTS_SAVE )
