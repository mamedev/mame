// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Olivier Galibert
/**************************************************************************************************

NEC PC-80S31(K) Mini Disk Unit

Z80 + μPD765 + PPI that connects in cross with an host PPI

"Mini" as compared to the PC-8881 8-inch floppy I/F

Design is decidedly derived from Epson TF-20 and friends,
cfr. devices/bus/epson_sio/tf20.cpp

PC-80S32 is the external version of what PC-8801mkII uses built-in,
capabilities are unknown at current time.

Eventually internalized in later PC-88 models,
Observed PCBs for internal boards are marked PWD-516 with a 8 digit revision number.
For simplicity's sake we just name these board over host classes instead.

TODO:
- Bus option;
- Cascade mode, i.e. the CN2 connector used to accept a second disk unit for drive 2 & 3;
- PC=0x7dd reads from FDC bit 3 in ST3 (twosid_r fn),
  expecting a bit 3 high for all the PC8001 games otherwise keeps looping and eventually dies.
  Are those incorrectly identified as 2DD? Hacked to work for now;
- set_input_line_vector fn doesn't work properly when issued from a device_reset,
  we currently just implement the irq_callback instead;
- pc80s31k: verify that irq vector write (I/O port $f0) belongs here or just
  whatever PC88VA uses.
- printer interface (used for debugging? 4-bit serial?)
- Pinpoint what host I/O ports $f6, $f7 truly are
  (direct FDC access from this device or a different beast? cfr. play6lim with pc8001mk2)
- filemst tries to access undocumented I/O port $09 at PC=5000:
  \- If that's 0 then it tries to read a vector at [0x8000];
  \- It then tries to read at memory [0xc0ff], set the value read in [0xf012];
  \- Expects that ROM [0x0000] is not equal to 0xc3;
  Bottom line: Is it trying to access some custom HW?
- Hookup a bridge for internal BIOSes (later PC8801 models);
- Save state support (resuming fails latch hookups here);
- pc88va2_fd_if_device: currently not hooked up to pc88va2, deasserts DRQ too fast, sub CPU
  incorrectly tells to master that floppies aren't 2HD?

===================================================================================================

PCB (PC-80S31K)

    |--------------------------------------|
    |      P1 P2 P3         X1             |
    |                                      |
    | CN2                                  |
    |                 B6102C022            |
    |      FD1 FD2              (**)       |
    |       |   |                          |
    | CN1 -------       D765AC  D2364EC (*)|
    |                                   (*)|
    |      D8255AC-2  B6101C017 D780C-1    |
    |                                      |
    |--------------------------------------|

    PCB is marked PC-80S31KFDC NEC-14T
    P1, P2, P3 are power supplies (+5V, +12V, -5V for P1, lacks -5V for the other two)
    μPD2364EC is at position IC13, it's a 8192 size ROM. (***)
    (*) are μPD41416C
    (**) marked as JP, unpopulated x 3;
    μPB6101/2 are bipolar TTL gate arrays, presumably
    Cannot read marking of X1 (8 MHz?)

    (***) Given this, we guess that PC80S31 is the 2D version while the 'K
    variant is the 2D/2DD/2HD version.

===================================================================================================

Command Protocol

Command & parameters are normally communicated from Host via port B
(read on port A on FDC side)
An RPi implementation can be seen at https://github.com/MinatsuT/RPi_PC-80S31

[0x00] Initialize
[0x01] Write to disk
   %1  number of sectors
   %2  drive number
   %3  track number
   %4  sector number +1
[0x02] Read from disk
   %1  number of sectors
   %2  drive number
   %3  track number
   %4  sector number +1
[0x03] Send data to host
[0x04] Copy data in-place
   %1  number of sectors
   %2  source drive
   %3  source track
   %4  source sector number +1
   %5  destination drive
   %6  destination track
   %7  destination sector number +1
[0x05] Format
   %1  drive number
[0x06] Send result status to Host
       x--- ---- I/O complete
       -x-- ---- has unread buffer
       ---- ---x error occurred
[0x07] Drive status
[0x0b] Send memory data
%1-%2  address start
%3-%4  length
[0x11] Fast write to disk
   %1  number of sectors
   %2  drive number
   %3  track number
   %4  sector number +1
[0x12] Fast send data
       (picks up number of sector etc. from previous issued commands?)
[0x14] Device status
       x--- ---- ESIG: error
       -x-- ---- WPDR: write protected
       --x- ---- RDY:  ready
       ---x ---- TRK0: track 0
       ---- x--- DSDR: double sided drive
       ---- -x-- HDDR: head
       ---- --xx DS1, DS2: drive select
       (same as 765 ST3?)
[0x17] Mode change
   %1  ---- xxxx mode select

FDC normally puts ST0-1-2 to RAM buffers $7f0d-f, CHRN data in $7f10-13

===================================================================================================

Port C
Used as a communication protocol flags

 Host side
 (swap 4-bit nibbles and r/w direction for FDC side, all bits are active high):
 x--- ---- (w) ATN AtenTioN:
               host sends a command to FDC, interrupts current one
               (looks unconnected the other way around?)
 -x-- ---- (w) DAC DAta aCcepted:
               host just picked up data from FDC
 --x- ---- (w) RFD Ready For Data:
               host requests data from FDC
 ---x ---- (w) DAV DAta Valid:
               host outputs data to port B
 ---- -x-- (r) DAC DAta aCcepted:
               FDC has accepted data from port B
 ---- --x- (r) RFD Ready For Data:
               FDC requests data from host
 ---- ---x (r) DAV DAta Valid:
               FDC has output data to port A

**************************************************************************************************/

#include "emu.h"
#include "pc80s31k.h"

//#define VERBOSE 1
#include "logmacro.h"

#include "formats/pc98fdi_dsk.h"
#include "formats/xdf_dsk.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


// device type definition
DEFINE_DEVICE_TYPE(PC80S31,       pc80s31_device,       "pc80s31",       "NEC PC-80S31 Mini Disk Unit I/F")
DEFINE_DEVICE_TYPE(PC80S31K,      pc80s31k_device,      "pc80s31k",      "NEC PC-80S31K Mini Disk Unit I/F")
DEFINE_DEVICE_TYPE(PC88VA2_FD_IF, pc88va2_fd_if_device, "pc88va2_fd_if", "NEC PC-88VA2 floppy disk interface \"PWD-516 72405162\"")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************


//-------------------------------------------------
//  pc80s31_device - constructor
//-------------------------------------------------


pc80s31_device::pc80s31_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0U)
	, m_fdc_cpu(*this, "fdc_cpu")
	, m_fdc_rom(*this, "fdc_rom")
	, m_ppi_host(*this, "ppi_host")
	, m_ppi_fdc(*this, "ppi_fdc")
	, m_latch(*this, "latch_%u", 0U)
{
}


pc80s31_device::pc80s31_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc80s31_device(mconfig, PC80S31, tag, owner, clock)
{
}

ROM_START( pc80s31 )
	// TODO: exact identification of these
	ROM_REGION( 0x2000, "fdc_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0,  "disk",    "disk BIOS" )
	ROMX_LOAD( "disk.rom", 0x0000, 0x0800, CRC(2158d307) SHA1(bb7103a0818850a039c67ff666a31ce49a8d516f), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1,  "mk2fr",   "mkIIFR disk BIOS" )
	ROMX_LOAD( "mk2fr_disk.rom", 0x0000, 0x0800, CRC(2163b304) SHA1(80da2dee49d4307f00895a129a5cfeff00cf5321), ROM_BIOS(1) )

	ROM_FILL( 0x7df, 1, 0x00 )
	ROM_FILL( 0x7e0, 1, 0x00 )
ROM_END

const tiny_rom_entry *pc80s31_device::device_rom_region() const
{
	return ROM_NAME( pc80s31 );
}

//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void pc80s31_device::host_map(address_map &map)
{
	map(0, 3).rw(m_ppi_host, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void pc80s31_device::fdc_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("fdc_rom", 0);
	map(0x4000, 0x7fff).ram();
}

void pc80s31_device::fdc_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
//  map(0x09, 0x09).r accessed by filemst (pc8801), cfr. notes

//  map(0xf0, 0xf0).w(FUNC(pc8801_state::fdc_irq_vector_w)); // Interrupt Opcode Port
//  map(0xf4, 0xf4).w(FUNC(pc8801_state::fdc_drive_mode_w)); // Drive mode, 2d, 2dd, 2hd
//  map(0xf6, 0xf6).nopw(); // printer related
	map(0xf7, 0xf7).nopw(); // printer port output
	map(0xf8, 0xf8).rw(FUNC(pc80s31_device::terminal_count_r), FUNC(pc80s31_device::motor_control_w));
	map(0xfa, 0xfb).m(m_fdc, FUNC(upd765a_device::map));
	map(0xfc, 0xff).rw(m_ppi_fdc, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

static void pc88_floppies(device_slot_interface &device)
{
	// TODO: definitely not correct for base device
	// TODO: eventually needs inheriting for pc88va3 (2TD with 9.3 MB capacity)
	device.option_add("525hd", FLOPPY_525_HD);
}

IRQ_CALLBACK_MEMBER(pc80s31_device::irq_cb)
{
	return m_irq_vector;
}

// need FDI and XDF for PC-88VA
static void pc88_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_XDF_FORMAT);
	fr.add(FLOPPY_PC98FDI_FORMAT);
}

void pc80s31_device::device_add_mconfig(machine_config &config)
{
	// TODO: confirm clock arrangement
	constexpr XTAL fdc_xtal = XTAL(8'000'000);
	Z80(config, m_fdc_cpu, fdc_xtal / 2);
	m_fdc_cpu->set_addrmap(AS_PROGRAM, &pc80s31_device::fdc_map);
	m_fdc_cpu->set_addrmap(AS_IO, &pc80s31_device::fdc_io);
	m_fdc_cpu->set_irq_acknowledge_callback(FUNC(pc80s31_device::irq_cb));

	UPD765A(config, m_fdc, fdc_xtal, true, true);
	m_fdc->intrq_wr_callback().set_inputline(m_fdc_cpu, INPUT_LINE_IRQ0);

	for (auto &floppy : m_floppy)
	{
		FLOPPY_CONNECTOR(config, floppy, pc88_floppies, "525hd", pc88_floppy_formats);
		floppy->enable_sound(true);
	}

	for (auto &latch : m_latch)
		GENERIC_LATCH_8(config, latch);

	I8255A(config, m_ppi_host);
	m_ppi_host->in_pa_callback().set(FUNC(pc80s31_device::latch_r<0>));
	m_ppi_host->out_pa_callback().set(FUNC(pc80s31_device::latch_w<1>));
	m_ppi_host->in_pb_callback().set(FUNC(pc80s31_device::latch_r<2>));
	m_ppi_host->out_pb_callback().set(FUNC(pc80s31_device::latch_w<3>));
	m_ppi_host->in_pc_callback().set(FUNC(pc80s31_device::latch_r<4>));
	m_ppi_host->out_pc_callback().set(FUNC(pc80s31_device::latch_w<5>));

	// 8255AC-2
	I8255A(config, m_ppi_fdc);
	m_ppi_fdc->in_pa_callback().set(FUNC(pc80s31_device::latch_r<3>));
	m_ppi_fdc->out_pa_callback().set(FUNC(pc80s31_device::latch_w<2>));
	m_ppi_fdc->in_pb_callback().set(FUNC(pc80s31_device::latch_r<1>));
	m_ppi_fdc->out_pb_callback().set(FUNC(pc80s31_device::latch_w<0>));
	m_ppi_fdc->in_pc_callback().set(FUNC(pc80s31_device::latch_r<5>));
	m_ppi_fdc->out_pc_callback().set(FUNC(pc80s31_device::latch_w<4>));
}

//-------------------------------------------------
//  tc_zero_tick
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(pc80s31_device::tc_zero_tick)
{
	m_fdc->tc_w(false);

	// several games tries to scan invalid IDs from their structures, if this hits then
	// it's possibly an attempt to scan a missing sector from the floppy structure.
	// cfr. acrojet: the third read data command issued tries to access a CHRN of (0, 0, 16, 256)
	// and checks at PC=500B if any of these status flags are satisfied:
	// ST0 & 0xdf
	// ST1 & 0xff
	// ST2 & 0x73
	// Data doesn't matter, it also seems to have some activity to the printer port
	// (debugging left on?)
	// Update: mostly fixed by separating upd765 irq types when the event above occurs.
	if ((u8)m_fdc_cpu->state_int(Z80_HALT) == 1)
	{
		logerror("%s: attempt to trigger TC while in HALT state (read ID copy protection warning)\n", machine().describe_context());
//      throw emu_fatalerror("copy protection hit");
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc80s31_device::device_start()
{
	m_tc_zero_timer = timer_alloc(FUNC(pc80s31_device::tc_zero_tick), this);

	save_item(NAME(m_irq_vector));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------


void pc80s31_device::device_reset()
{
	m_floppy[0]->get_device()->set_rpm(300);
	m_floppy[1]->get_device()->set_rpm(300);
	m_fdc->set_rate(250000);

	// TODO: doesn't seem to work for devices?
	m_fdc_cpu->set_input_line_vector(0, 0);
	m_irq_vector = 0;

	m_tc_zero_timer->adjust(attotime::never);
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

// Comms are simple dual port connected in cross fashion.
// Even at "perfect" interleave tho we need to mailbox the connections.
// - barbatus will hang at Artec logo the first time around (works if you soft reset);
template <unsigned N> u8 pc80s31_device::latch_r()
{
	const int port_mask = N & 4 ? 0x0f : 0xff;
//  machine().scheduler().synchronize();
	return m_latch[N]->read() & port_mask;
}

template <unsigned N> void pc80s31_device::latch_w(u8 data)
{
	const int lower_nibble = N & 4;
//  machine().scheduler().synchronize();
	if (lower_nibble)
	{
		LOG("%s: %s port C write %02x  (ATN=%d DAC=%d RFD=%d DAV=%d)\n"
			, N & 1 ? "host" : "fdc"
			, machine().describe_context()
			, data
			, BIT(data, 7)
			, BIT(data, 6)
			, BIT(data, 5)
			, BIT(data, 4)
		);
	}
	return m_latch[N]->write(data >> lower_nibble);
}

u8 pc80s31_device::terminal_count_r(address_space &space)
{
	if (!machine().side_effects_disabled())
	{
		m_fdc->tc_w(true);
		// TODO: accurate time of this going off
		m_tc_zero_timer->reset();
		m_tc_zero_timer->adjust(attotime::from_usec(50));
	}
	// value is meaningless (never readback)
	// TODO: verify this being 0xff or open bus
	return space.unmap();
}

void pc80s31_device::motor_control_w(uint8_t data)
{
	// FIXME: on pc80s31k device (particularly on later releases) this stays always on
	// babylon: just spins indefinitely at PC=6d8 (using the internal routines),
	//          waiting for DAV or ATN being on. Never hits the port until a flag is issued.
	// prajator: on idle times it spins at PC=7060, waiting for ATN and keep issuing a 0xff here.
	// valis2: calls PC=7009 subroutine for idle, waits for ATN on.
	//         It eventually writes a 0 here, not before an extremely long time
	//         (~10000 frames!)
	m_floppy[0]->get_device()->mon_w(!(data & 1));
	m_floppy[1]->get_device()->mon_w(!(data & 2));

	// TODO: according to docs a value of 0x07 enables precompensation to tracks 0-19, 0xf enables it on 20-39
}

//**************************************************************************
//
//  PC80S31K device overrides
//
//**************************************************************************

pc80s31k_device::pc80s31k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pc80s31_device(mconfig, type, tag, owner, clock)
{}

pc80s31k_device::pc80s31k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc80s31k_device(mconfig, PC80S31K, tag, owner, clock)
{
}

ROM_START( pc80s31k )
	// TODO: exact identification of these
	ROM_REGION( 0x2000, "fdc_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "ma",       "MA disk BIOS")
	ROMX_LOAD( "ma_disk.rom", 0x0000, 0x2000, CRC(a222ecf0) SHA1(79e9c0786a14142f7a83690bf41fb4f60c5c1004), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "mk2mr",    "mkIIMR disk BIOS" )
	ROMX_LOAD( "m2mr_disk.rom", 0x0000, 0x2000, CRC(2447516b) SHA1(1492116f15c426f9796dc2bb6fcccf2656c0ca75), ROM_BIOS(1) )
	// same as MA BIOS
//  ROM_SYSTEM_BIOS( 2, "mh",       "MH disk BIOS" )
//  ROMX_LOAD( "mh_disk.rom", 0x0000, 0x2000, CRC(a222ecf0) SHA1(79e9c0786a14142f7a83690bf41fb4f60c5c1004), ROM_BIOS(2) )
ROM_END

const tiny_rom_entry *pc80s31k_device::device_rom_region() const
{
	return ROM_NAME( pc80s31k );
}

void pc80s31k_device::drive_mode_w(uint8_t data)
{
	// TODO: fix implementation
	// anything that isn't a 2D doesn't really set proper parameters in here
	logerror("FDC drive mode %02x\n", data);
	m_floppy[0]->get_device()->set_rpm(BIT(data, 0) ? 360 : 300);
	m_floppy[1]->get_device()->set_rpm(BIT(data, 1) ? 360 : 300);

	m_fdc->set_rate(BIT(data, 5) ? 500000 : 250000);
}

void pc80s31k_device::fdc_io(address_map &map)
{
	pc80s31_device::fdc_io(map);

	map(0xf0, 0xf0).lw8(NAME([this] (u8 data) { m_irq_vector = data; }));
	map(0xf4, 0xf4).w(FUNC(pc80s31k_device::drive_mode_w));
}

//**************************************************************************
//
//  PC-88VA2 internal board overrides
//
// FD-55GFR-351 disk drives
//
//**************************************************************************

pc88va2_fd_if_device::pc88va2_fd_if_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc80s31k_device(mconfig, PC88VA2_FD_IF, tag, owner, clock)
	, m_write_irq(*this)
	, m_write_drq(*this)
{
}


ROM_START( pc88va2_fd_if )
	ROM_REGION( 0x2000, "fdc_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "vasubsys.rom", 0x0000, 0x2000, CRC(08962850) SHA1(a9375aa480f85e1422a0e1385acb0ea170c5c2e0) )
ROM_END

const tiny_rom_entry *pc88va2_fd_if_device::device_rom_region() const
{
	return ROM_NAME( pc88va2_fd_if );
}

void pc88va2_fd_if_device::device_add_mconfig(machine_config &config)
{
	pc80s31k_device::device_add_mconfig(config);
	m_fdc->intrq_wr_callback().append([this](int state) { if(m_fdc_mode & 1) { m_write_irq(state); } });
	m_fdc->drq_wr_callback().set([this](int state) { if(m_fdc_mode & 1) { m_write_drq(state); } });

//  m_fdc->set_ready_line_connected(false);
}

void pc88va2_fd_if_device::device_start()
{
	pc80s31k_device::device_start();

	m_fdc_timer = timer_alloc(FUNC(pc88va2_fd_if_device::fdc_timer_cb), this);
	m_fdc_timer->adjust(attotime::never);

	m_motor_start_timer[0] = timer_alloc(FUNC(pc88va2_fd_if_device::motor_start_timer_cb<0>), this);
	m_motor_start_timer[1] = timer_alloc(FUNC(pc88va2_fd_if_device::motor_start_timer_cb<1>), this);
	m_motor_start_timer[0]->adjust(attotime::never);
	m_motor_start_timer[1]->adjust(attotime::never);

	floppy_image_device *floppy;
	floppy = m_floppy[0]->get_device();
	if(floppy)
		floppy->setup_ready_cb(floppy_image_device::ready_cb(&pc88va2_fd_if_device::fdc_update_ready, this));

	floppy = m_floppy[1]->get_device();
	if(floppy)
		floppy->setup_ready_cb(floppy_image_device::ready_cb(&pc88va2_fd_if_device::fdc_update_ready, this));
}

void pc88va2_fd_if_device::device_reset()
{
	pc80s31k_device::device_reset();

	m_fdc_mode = 0;
//  m_fdc_irq_opcode = 0x00; //0x7f ld a,a !
	m_xtmask = false;
	m_dmae = false;
}

void pc88va2_fd_if_device::host_io(address_map &map)
{
	map(0x00, 0x00).w(FUNC(pc88va2_fd_if_device::host_mode_w));
	map(0x02, 0x02).w(FUNC(pc88va2_fd_if_device::host_drive_rate_w));
	map(0x04, 0x04).w(FUNC(pc88va2_fd_if_device::host_motor_control_w));
	map(0x06, 0x06).rw(FUNC(pc88va2_fd_if_device::host_ready_r), FUNC(pc88va2_fd_if_device::host_fdc_control_w));
	map(0x08, 0x08).r(m_fdc, FUNC(upd765a_device::msr_r));
	map(0x0a, 0x0a).rw(m_fdc, FUNC(upd765a_device::fifo_r), FUNC(upd765a_device::fifo_w));
}

template <unsigned DriveN> TIMER_CALLBACK_MEMBER(pc88va2_fd_if_device::motor_start_timer_cb)
{
	m_floppy[DriveN]->get_device()->mon_w(0);
}

TIMER_CALLBACK_MEMBER(pc88va2_fd_if_device::fdc_timer_cb)
{
	if(m_xtmask)
	{
		m_write_irq(1);
//      m_write_irq(0);
	}
}

void pc88va2_fd_if_device::fdc_update_ready(floppy_image_device *, int)
{
	if (!BIT(m_fdc_ctrl_2, 5))
		return;
	bool force_ready = bool(BIT(m_fdc_ctrl_2, 6));

	floppy_image_device *floppy0, *floppy1;
	floppy0 = m_floppy[0]->get_device();
	floppy1 = m_floppy[1]->get_device();
	if (!floppy0 && !floppy1)
		force_ready = false;

	//if(floppy && force_ready)
	//  ready = floppy->ready_r();

	//if(floppy && force_ready)
	//  ready = floppy->ready_r();

	LOG("Force ready signal %d\n", force_ready);

	if (force_ready)
	{
		m_fdc->set_ready_line_connected(false);
		m_fdc->ready_w(0);
	}
	else
		m_fdc->ready_w(1);
}

void pc88va2_fd_if_device::host_mode_w(u8 data)
{
	m_fdc_mode = data & 1;
	LOG("$1b0 FDC op mode (%02x) %s mode\n"
		, data
		, m_fdc_mode ? "DMA" : "Intelligent (PIO)"
	);
//  m_fdc_cpu->set_input_line(INPUT_LINE_HALT, (m_fdc_mode) ? ASSERT_LINE : CLEAR_LINE);
}

/*
 * --x- ---- CLK: FDC clock selection (0) 4.8MHz (1) 8 MHz
 * ---x ---- DS1: Prohibition of the drive selection of FDC (0) Permission (1) Prohibition
 * ---- xx-- TD1/TD0: Drive 1/0 track density (0) 48 TPI (1) 96 TPI
 * ---- --xx RV1/RV0: Drive 1/0 mode selection (0) 2D and 2DD mode (1) 2HD mode
 */
void pc88va2_fd_if_device::host_drive_rate_w(u8 data)
{
	const bool clk = bool(BIT(data, 5));
	const bool rv1 = bool(BIT(data, 1));
	const bool rv0 = bool(BIT(data, 0));
	LOG("$1b2 FDC control port 0 (%02x) %s CLK| %d DS1| %d%d TD1/TD0| %d%d RV1/RV0\n"
		, data
		, clk ? "  8 MHz" : "4.8 MHz"
		, !bool(BIT(data, 4))
		, bool(BIT(data, 3))
		, bool(BIT(data, 2))
		, rv1
		, rv0
	);
	m_floppy[0]->get_device()->set_rpm(rv0 ? 360 : 300);
	m_floppy[1]->get_device()->set_rpm(rv1 ? 360 : 300);

	//m_fdd[0]->get_device()->ds_w(!BIT(data, 4));
	//m_fdd[1]->get_device()->ds_w(!BIT(data, 4));

	// TODO: is this correct? sounds more like a controller clock change, while TD1/TD0 should do the rate change
	m_fdc->set_rate(clk ? 500000 : 250000);
}

/*
 * ---- x--- PCM: precompensation control (1) on
 * ---- --xx M1/M0: Drive 1/0 motor control (0) NOP (1) Change motor status
 */
void pc88va2_fd_if_device::host_motor_control_w(u8 data)
{
	const bool m0 = bool(BIT(data, 0));
	const bool m1 = bool(BIT(data, 1));

	LOG("$1b4 FDC control port 1 (%02x) %d PCM| %d%d M1/M0\n"
		, data
		, bool(BIT(data, 3))
		, m1
		, m0
	);

	// TODO: fine grain motor timings
	// docs claims 600 msecs, must be more complex than that
	if( m0 )
		m_motor_start_timer[0]->adjust(attotime::from_msec(505));
	else
		m_floppy[0]->get_device()->mon_w(1);


	if( m1 )
		m_motor_start_timer[1]->adjust(attotime::from_msec(505));
	else
		m_floppy[1]->get_device()->mon_w(1);
}

/*
 * FDC control port 2
 * x--- ---- FDCRST: FDC Reset
 * -xx- ---- FDCFRY FRYCEN: FDC force ready control
 * -x0- ---- ignored
 * -01- ---- force ready release
 * -11- ---- force ready assert
 * ---x ---- DMAE: DMA Enable (0) Prohibit DMA (1) Enable DMA
 * ---- -x-- XTMASK: FDC timer IRQ mask (0) Disable (1) Enable
 * ---- ---x TTRG: FDC timer trigger (0) FDC timer clearing (1) FDC timer start
 */
void pc88va2_fd_if_device::host_fdc_control_w(u8 data)
{
	const bool fdcrst = !!(BIT(data, 7));
	const bool ttrg = !!(BIT(data, 0));
	const bool cur_xtmask = !!(BIT(data, 2));
	const bool cur_dmae = !!(BIT(data, 4));
	LOG("$1b6 FDC control port 2 (%02x) %d FDCRST| %d%d FDCFRY| %d DMAE| %d XTMASK| %d TTRG\n"
		, data
		, fdcrst
		, bool(BIT(data, 6))
		, bool(BIT(data, 5))
		, cur_dmae
		, cur_xtmask
		, ttrg
	);

	if( ttrg && !BIT(m_fdc_ctrl_2, 0) )
		m_fdc_timer->adjust(attotime::from_msec(100));
	else if (!ttrg && BIT(m_fdc_ctrl_2, 0) )
		m_fdc_timer->adjust(attotime::never);

	m_xtmask = cur_xtmask;

	// TODO: 0 -> 1 transition?
	if( fdcrst )
		m_fdc->reset();

	m_dmae = cur_dmae;

	m_fdc_ctrl_2 = data;

	//m_fdd[0]->get_device()->mon_w(!(BIT(data, 5)));

	// TODO: verify if this requires updating drive B: as well
	fdc_update_ready(nullptr, 0);
}

/*
 * ---x ---- RDY: (0) Busy (1) Ready
 */
u8 pc88va2_fd_if_device::host_ready_r()
{
	// TODO: easy to implement, but no SW accesses it so far
	if (!machine().side_effects_disabled())
		logerror("Unhandled read $1b6!\n");

	return 0;
}
