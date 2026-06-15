// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * National Semiconductor ICM-3216 Integrated Computer Module
 *
 * The lead engineer on the ICM-3216 project was William (Bill) Fox.
 *
 * Sources:
 *  - ICM-3216 Integrated Computer Module, January 1986, National Semiconductor (DS6786S-10M16)
 *  - ICM-3216 CPU Board Specification, revision PB, 10/17/85, National Semiconductor Corporation (426610289-000)
 *
 * The disk and tape I/O is offloaded to a Z80 I/O processor (IOP) running
 * its own firmware ROM; the IOP owns an NCR5385 SCSI controller and moves
 * data to and from 32016 main memory across the MiniBus.  The emulated Z80 IOP
 * firmware runs and drives the NCR5385 over the nscsi bus (root disk at -scsi:1
 * harddisk, tape at -scsi:7 tape), reproducing the MiniBus address-latch protocol;
 * this boots UNIX System V to single user.
 *
 * A faster but less faithful high-level (HLE) I/O processor -- the Z80 held suspended with
 * the host mailbox serviced in software against the same nscsi disk/tape -- is selectable in
 * the Machine Configuration menu ("I/O Processor").  The default is the faithful LLE Z80 IOP.
 *
 * TODO:
 *  - configurable ram size (1M, 2M, 4M, 8M)
 */

#include "emu.h"

// cpus and memory
#include "cpu/ns32000/ns32000.h"
#include "cpu/z80/z80.h"

// various hardware
#include "machine/mc68681.h"
#include "machine/mm58274c.h"
#include "machine/ncr5385.h"
#include "machine/ns32081.h"
#include "machine/ns32082.h"
#include "machine/ns32202.h"

// busses and connectors
#include "bus/rs232/rs232.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/tape.h"
#include "machine/nscsi_bus.h"

// the HLE I/O processor accesses the nscsi disk/tape images directly
#include "imagedev/harddriv.h"
#include "imagedev/simh_tape_image.h"
#include "util/tape_file_interface.h"
#include "multibyte.h"

#define LOG_PARITY (1U << 1)

//#define VERBOSE (LOG_GENERAL|LOG_PARITY)
#include "logmacro.h"

namespace {

class icm3216_state : public driver_device
{
public:
	icm3216_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_fpu(*this, "fpu")
		, m_mmu(*this, "mmu")
		, m_icu(*this, "icu")
		, m_rtc(*this, "rtc")
		, m_duart(*this, "duart%u", 0U)
		, m_serial(*this, "serial%u", 0U)
		, m_iop(*this, "iop")
		, m_scsi(*this, "ncr5385")
		, m_scsibus(*this, "scsi")
		, m_hle_disk(*this, "scsi:1:harddisk")
		, m_hle_tape(*this, "scsi:7:tape")
		, m_led(*this, "led%u", 1U)
		, m_iopcfg(*this, "IOPCFG")
		, m_boot(*this, "boot")
	{
	}

	// machine config
	void icm3216(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	template <unsigned ST> void cpu_map(address_map &map) ATTR_COLD;
	void iop_mem_map(address_map &map) ATTR_COLD;
	void iop_pio_map(address_map &map) ATTR_COLD;

	// register read handlers
	u8 iop_r();
	u16 nmi_status_r();
	u8 nmi_enable_r();

	// register write handlers
	void iop_w(u8 data);
	void parity_select_w(u8 data);
	void parity_enable_w(u8 data);

	// iop<->host mailbox + main-memory DMA bridge (c010-c017)
	u8 iop_brg_r(offs_t offset);
	void iop_brg_w(offs_t offset, u8 data);

	// parity handlers
	void parity_r(offs_t offset, u16 &data, u16 mem_mask);
	void parity_w(offs_t offset, u16 &data, u16 mem_mask);

	IRQ_CALLBACK_MEMBER(iop_ack);
	template <unsigned Source> void iop_int(int state);
	void scsi_dreq_w(int state); // MiniBus DMA engine: '646 byte<->main-mem word mover

	// HLE (simulated) SCSI I/O processor
	void hle_start_io(u8 cmd);
	TIMER_CALLBACK_MEMBER(hle_complete);
	u8 hle_tape(u8 const *cdb, u32 dataptr, u32 tptptr, u32 length, u8 &chanstatus);
	// translate a transfer offset to a physical byte address.  Buffered I/O has
	// tptptr==0 and a flat dataptr; raw (B_PHYS) I/O has tptptr = phys addr of the
	// user buffer's NS32082 PTE array (512-byte pages) with dataptr = offset within
	// the first page -- walk the page table to scatter/gather (do_scsi in io/scsi.c).
	u32 hle_xlate(u32 dataptr, u32 tptptr, u32 off);

private:
	required_device<ns32016_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<ns32082_device> m_mmu;
	required_device<ns32202_device> m_icu;

	required_device<mm58274c_device> m_rtc;
	required_device_array<scn2681_device, 2> m_duart;
	required_device_array<rs232_port_device, 4> m_serial;

	required_device<z80_device> m_iop;
	required_device<ncr5385_device> m_scsi;
	required_device<nscsi_bus_device> m_scsibus;
	optional_device<nscsi_harddisk_device> m_hle_disk; // scsi:1 disk, accessed directly by the HLE
	optional_device<nscsi_tape_device> m_hle_tape;     // scsi:7 tape, accessed directly by the HLE

	output_finder<5> m_led;

	required_ioport m_iopcfg;     // machine-config: 0=HLE (simulated IOP), 1=LLE (Z80 IOP)
	bool m_hle = false;           // resolved from m_iopcfg at machine_reset (default LLE)

	// HLE I/O-processor state
	emu_timer *m_hle_timer = nullptr; // allocated in machine_start
	u8 m_hle_cmd;
	u8 m_hle_tape_sense; // pending tape sense byte2 (filemark/EOM flags)

	memory_view m_boot;
	memory_passthrough_handler m_boot_mph;

	enum nmi_status_mask : u16
	{
		NMI_CHIP   = 0x000f, // parity error chip(s)
		NMI_EVEN   = 0x0010, // parity error on even byte
		NMI_ODD    = 0x0020, // parity error on odd byte
		NMI_MMU    = 0x0040, // mmu interrupt
		NMI_MBIC   = 0x0080, // mbic interrupt
		NMI_PARITY = 0x0100, // parity error
	};

	enum iop_status_mask : u8
	{
		IOP_IID = 0x07, // subchannel interrupt ID
		IOP_IRS = 0x10, // interrupt request
		IOP_RST = 0x20, // scsi reset
		IOP_ABT = 0x40, // scsi abort interrupt?
		IOP_BSY = 0x80, // busy
	};

	// machine registers
	u8 m_iop_cmd;
	u8 m_iop_sts;
	u8 m_iop_vec;
	// iop<->host main-memory DMA bridge state
	u32 m_iop_base;     // cpt base byte-address latched from host cmd-00 + 3 bytes (HLE)
	u8  m_iop_basecnt;  // countdown while capturing the 3 base bytes (HLE)
	u32 m_iop_waddr;    // bridge word-address counter (byte addr = waddr << 1)
	u8  m_iop_hipage;   // U76 page latch (A16-23), loaded from C012 by the NMI handler
	u8  m_u44;          // U44 low address counter (AD01-08), loaded from C010
	u8  m_u59;          // U59 high address latch (AD09-15), from C011 by the NMI handler
	bool m_dma_hi;      // '646 (U60/U61) byte lane: false=DMALODAT, true=DMAHIDAT
	bool m_iop_cmd_pending; // C011 bit 0x40: host command waiting to be read
	u8   m_iop_wlow;        // low byte stashed by a C016 main-mem write
	bool m_iop_wpending;    // a C016 write is awaiting its C017 high byte
	// resolved handle to the 32016 program space for the '646 DMA byte mover and the
	// C012-C017 word ports; avoids a space lookup on every DREQ.  Resolved in machine_start.
	memory_access<24, 1, 0, ENDIANNESS_LITTLE>::specific m_dma_space;
	// console (DUART1 ch B) 8N1-force
	u8    m_con_mrptr;        // shadow of the channel MR pointer (0=MR1, 1=MR2)

	u16 m_nmi_state;      // non-maskable interrupt status register

	bool m_nmi_enable;    // non-maskable interrupt enable
	bool m_parity_enable; // parity checking enable
	bool m_parity_select; // parity mode even/odd

	// other internal state
	s32 m_parity_delta; // parity count increment/decrement
	u32 m_parity_count; // count mismatched parity writes

	std::unique_ptr<u8[]> m_parity;
	memory_passthrough_handler m_parity_mph;

	static constexpr unsigned RAM_SIZE = 0x400000;
};

void icm3216_state::machine_start()
{
	m_cpu->space(0).specific(m_dma_space);

	save_item(NAME(m_iop_cmd));
	save_item(NAME(m_iop_sts));
	save_item(NAME(m_iop_vec));
	save_item(NAME(m_iop_base));
	save_item(NAME(m_iop_basecnt));
	save_item(NAME(m_iop_waddr));
	save_item(NAME(m_iop_hipage));
	save_item(NAME(m_u44));
	save_item(NAME(m_u59));
	save_item(NAME(m_dma_hi));
	save_item(NAME(m_iop_cmd_pending));
	save_item(NAME(m_iop_wlow));
	save_item(NAME(m_iop_wpending));
	save_item(NAME(m_hle_cmd));
	save_item(NAME(m_hle_tape_sense));
	save_item(NAME(m_con_mrptr));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_parity_enable));
	save_item(NAME(m_parity_select));
	save_item(NAME(m_parity_delta));
	save_item(NAME(m_parity_count));

	save_pointer(NAME(m_parity), RAM_SIZE);

	m_hle_timer = timer_alloc(FUNC(icm3216_state::hle_complete), this);

	// HACK: the kernel switches the console DUART (DUART1 ch B, regs 0x08-0x0b at 0xa00050-
	// 0xa00056) to 7E1 once it goes interrupt-driven.  Real hardware ran a 7E1 terminal, but
	// MAME's built-in terminal can't display 7E1, so pin the framing back to 8N1 to keep the
	// console usable.  This is a host-terminal workaround, not an scn2681 emulation bug.
	m_cpu->space(0).install_write_tap(0xa0'0050, 0xa0'0057, "icm_console",
		[this](offs_t a, u16 &data, u16 mem_mask)
		{
			if (!(mem_mask & 0x00ff))
				return;
			u8 b = data & 0xff;
			switch ((a - 0xa0'0050) >> 1)
			{
			case 0: // MR1B / MR2B (MR pointer selects which)
				if (m_con_mrptr == 0) { b = (b & 0xe0) | 0x13; m_con_mrptr = 1; } // 8 bits, no parity
				else                    b &= 0xf3;                                 // 1 stop bit
				data = (data & 0xff00) | b;
				break;
			case 2: // CRB: command 1 (bits 6:4 = 001) resets the MR pointer to MR1
				if (((b >> 4) & 7) == 1)
					m_con_mrptr = 0;
				break;
			}
		});
	// reads of MR also advance the device's MR pointer -- keep the shadow in sync
	m_cpu->space(0).install_read_tap(0xa0'0050, 0xa0'0051, "icm_console_mr",
		[this](offs_t, u16 &, u16)
		{
			if (!machine().side_effects_disabled() && m_con_mrptr == 0)
				m_con_mrptr = 1;
		});
}

void icm3216_state::machine_reset()
{
	m_hle = !BIT(m_iopcfg->read(), 0); // IOPCFG: 0 = HLE, 1 = LLE (default)

	m_iop_cmd = 0;
	m_iop_sts = 0;
	m_iop_vec = 0;
	m_iop_base = 0;
	m_iop_basecnt = 0;
	m_iop_waddr = 0;
	m_iop_hipage = 0;
	m_u44 = 0;
	m_u59 = 0;
	m_dma_hi = false;
	m_iop_cmd_pending = false;
	m_iop_wlow = 0;
	m_iop_wpending = false;
	m_hle_cmd = 0;
	m_hle_tape_sense = 0;

	m_boot.select(0);

	m_boot_mph = m_cpu->space(0).install_read_tap(0x800000, 0xffffff, "boot",
		[this](offs_t offset, uint16_t &data, uint16_t mem_mask)
		{
			if (!machine().side_effects_disabled())
			{
				m_boot.disable();
				m_boot_mph.remove();
			}
		});

	// HLE: hold the Z80 I/O processor in suspension so it doesn't run the
	// firmware or drive the (now-bypassed) NCR5385; the host mailbox is serviced
	// directly in software (iop_w -> hle_start_io).
	if (m_hle)
		m_iop->suspend(SUSPEND_REASON_DISABLE, true);

	m_duart[0]->ip6_w(1); // IP6 = J4-3 SINGLE STEP input (N.O. switch + ~5K pull-up): idle = high

	// RTC: set_use_utc makes the device seed itself from UTC, but it writes a 1-based month
	// and the year mod 100.  This kernel (io/rt_clock.c rtc_read) reads the month into a
	// 0-based tm_mon (no -1) and computes the year as 1970 + the 2-digit register, so override
	// just those two (BCD nibbles 0x0a/0b=month, 0x0c/0d=year; units then tens).
	{
		system_time now;
		machine().current_datetime(now);
		auto const &t = now.utc_time;
		auto wbcd = [this](offs_t ru, int v) { m_rtc->write(ru, v % 10); m_rtc->write(ru + 1, (v / 10) % 10); };
		wbcd(0x0a, t.month);                 // 0-based -> kernel's tm_mon
		wbcd(0x0c, (t.year - 1970) % 100);   // years-since-1970 -> kernel 1970+nn
	}

	m_con_mrptr = 0; // channel MR pointer points to MR1 after reset

	m_nmi_state = 0;

	m_nmi_enable = true;
	m_parity_enable = false;
	m_parity_select = false;

	m_parity_delta = 0;
	m_parity_count = 0;

	m_parity.reset();
	m_parity_mph.remove();
}

template <unsigned ST> void icm3216_state::cpu_map(address_map &map)
{
	map(0x00'0000, RAM_SIZE - 1).ram();
	map(RAM_SIZE, 0x7f'ffff).noprw();

	if (ST == 0)
	{
		map(0x00'0000, 0x01'ffff).view(m_boot);
		m_boot[0](0x00'0000, 0x01'ffff).rom().region("eprom", 0);
	}

	map(0x80'0000, 0x81'ffff).rom().region("eprom", 0).mirror(0x100000);

	map(0xa0'0000, 0xa0'001f).umask16(0x00ff).rw(m_rtc, FUNC(mm58274c_device::read), FUNC(mm58274c_device::write));
	map(0xa0'0020, 0xa0'003f).umask16(0x00ff).rw(m_duart[0], FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0xa0'0040, 0xa0'005f).umask16(0x00ff).rw(m_duart[1], FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	//map(0xa0'0080, 0xa0'0083); // parallel

	map(0xa0'00a0, 0xa0'00a1).umask16(0x00ff).rw(FUNC(icm3216_state::iop_r), FUNC(icm3216_state::iop_w));
	map(0xa0'00c0, 0xa0'00c1).r(FUNC(icm3216_state::nmi_status_r)); // TODO: minibus hold (w)
	map(0xa0'00c2, 0xa0'00cb).umask16(0x00ff).lw8(NAME([this](offs_t offset, u8 data) { m_led[offset] = data; })); // TODO: minibus reset
	map(0xa0'00cc, 0xa0'00cc).w(FUNC(icm3216_state::parity_select_w));
	map(0xa0'00ce, 0xa0'00ce).w(FUNC(icm3216_state::parity_enable_w));

	map(0xa0'00e0, 0xa0'00e0).r(FUNC(icm3216_state::nmi_enable_r)); // TODO: minibus mhl/mcl (w)

	//map(0xc0'0000, 0xfd'ffff); // minibus memory 0x000000-0x3dffff
	//map(0xfe'0000, 0xfe'ffff); // minibus 8-bit i/o
	//map(0xff'0000, 0xff'7fff); // minibus 16-bit i/o

	map(0xff'fe00, 0xff'feff).m(m_icu, FUNC(ns32202_device::map<BIT(ST, 1)>)).umask16(0x00ff);

	if (ST == 4)
		map(0xff'ff00, 0xff'ff01).nopr(); // silence cpu nmi vector
}

void icm3216_state::iop_mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("iop", 0);
	map(0x4000, 0x7fff).ram();

	//map(0xc010, 0xc010).lr8(NAME([this]() { logerror("0xc010 0x%02x (%s)\n", m_iop_cmd, machine().describe_context()); return m_iop_cmd; }));
	//map(0xc011, 0xc011).lr8(NAME([this]() { logerror("0xc011 0x%02x (%s)\n", 0x40, machine().describe_context()); return 0x40; }));
	// iop<->host mailbox + main-mem DMA bridge
	map(0xc010, 0xc017).rw(FUNC(icm3216_state::iop_brg_r), FUNC(icm3216_state::iop_brg_w));

	map(0xc020, 0xc02f).m(m_scsi, FUNC(ncr5385_device::map));
}

void icm3216_state::iop_pio_map(address_map &map)
{
}

static INPUT_PORTS_START(icm3216)
	// Selects how the SCSI I/O processor is emulated.  Default = LLE: the real Z80 IOP firmware
	// drives the NCR5385 over the nscsi bus.  HLE holds the Z80 suspended and services the host
	// mailbox in software against the same nscsi disk (scsi:1) / tape (scsi:7).  Machine Config menu.
	PORT_START("IOPCFG")
	PORT_CONFNAME(0x01, 0x01, "I/O Processor")
	PORT_CONFSETTING(   0x00, "Simulated (HLE)")
	PORT_CONFSETTING(   0x01, "Emulated Z80 (LLE)")
INPUT_PORTS_END

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("tape", NSCSI_TAPE);
}


void icm3216_state::icm3216(machine_config &config)
{
	NS32016(config, m_cpu, 20_MHz_XTAL / 2);
	m_cpu->set_addrmap(0, &icm3216_state::cpu_map<0>);
	m_cpu->set_addrmap(4, &icm3216_state::cpu_map<4>);

	NS32081(config, m_fpu, 20_MHz_XTAL / 2);
	m_cpu->set_fpu(m_fpu);

	NS32082(config, m_mmu, 20_MHz_XTAL / 2);
	m_cpu->set_mmu(m_mmu);

	// 8 bit mode, irq: 4 serial, 1 minibus, 1 scsi, 1 printer, 1 rtc, 8 free
	NS32202(config, m_icu, 18.432_MHz_XTAL / 10);
	m_icu->out_int().set_inputline(m_cpu, INPUT_LINE_IRQ0).invert();
	// The on-chip counter output (COUT/SCIN pin) is wired back to interrupt
	// input 6 on the board -- the "Real time clock / Test mode input" of the
	// CPU Board Spec interrupt table (= ICU_CLOCK, input 6, per
	// buts/ns32000/sys/icu.h). The kernel programs the counter for HZ=20
	// (buts/ns32000/ml/intr.m4) and COUT drives IR6, which is the UNIX
	// scheduler tick. Without this feedback the system clock never fires and
	// the kernel hangs in early init. (MAME's ir_w<6> auto-ignores this when
	// the counter is instead assigned internally via CIPTR, so it is correct
	// regardless of which delivery path the kernel selects.)
	m_icu->out_cout().set(m_icu, FUNC(ns32202_device::ir_w<6>));

	MM58274C(config, m_rtc, 0).set_mode24(1); // kernel reads hours as 24h (rt_clock.c)
	m_rtc->set_use_utc(true);                 // rt_clock.c treats the chip as GMT -- seed from UTC

	// we are dte, therefore: tx,rx,rts,cts,dsr,dtr,dcd
	// rts o
	// cts i
	// dsr i
	// rlsd i
	// dtr o

	SCN2681(config, m_duart[0], 3'686'400); // SCN2681A
	// DUART0 INT -> ICU_SIG_INT1 (input 10); DUART1 INT -> ICU_SIG_INT2 (input 12,
	// the console) per buts/ns32000/sys/icu.h + sig2681.c (SIGCONSOLE=3=DUART1:B).
	// The kernel programs these inputs active-low (TPL bits 10/12 = 0), so invert
	// the SCN2681's active-high logical irq onto the ICU's active-low line --
	// otherwise the DUART interrupt never latches and the console tty blocks.
	m_duart[0]->irq_cb().set(m_icu, FUNC(ns32202_device::ir_w<10>)).invert();
	// The DUART RxRDY outputs (OP4=RxRDYA, OP5=RxRDYB, driven active-low when the
	// kernel programs OPCR) are wired to the ICU's high-priority "Receiver Ready"
	// inputs -- IR2 for DUART0, IR4 for DUART1 (SIG_R1/SIG_R2 per icu.h). The kernel
	// reads received characters via this path (sighintr), NOT via the main DUART
	// interrupt; without it RxRDY is never serviced and the low-priority drain loop
	// spins. Assert the (level, active-low) input when either channel has a char.
	m_duart[0]->outport_cb().set([this](u8 data) { m_icu->ir_w<2>(BIT(data, 4) & BIT(data, 5)); });
	m_duart[0]->a_tx_cb().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_duart[0]->b_tx_cb().set(m_serial[1], FUNC(rs232_port_device::write_txd));

	SCN2681(config, m_duart[1], 3'686'400); // SCN2681A
	m_duart[1]->irq_cb().set(m_icu, FUNC(ns32202_device::ir_w<12>)).invert();
	m_duart[1]->outport_cb().set([this](u8 data) { m_icu->ir_w<4>(BIT(data, 4) & BIT(data, 5)); }); // RxRDY -> IR4 (SIG_R2, console receiver)
	m_duart[1]->a_tx_cb().set(m_serial[2], FUNC(rs232_port_device::write_txd));
	m_duart[1]->b_tx_cb().set(m_serial[3], FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_serial[0], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_serial[0]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_a_w));
	m_serial[1]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_b_w));

	RS232_PORT(config, m_serial[2], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[3], default_rs232_devices, "terminal");
	m_serial[2]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_a_w));
	m_serial[3]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_b_w));

	Z80(config, m_iop, 20_MHz_XTAL / 4);
	m_iop->set_addrmap(AS_PROGRAM, &icm3216_state::iop_mem_map);
	m_iop->set_addrmap(AS_IO, &icm3216_state::iop_pio_map);
	m_iop->set_irq_acknowledge_callback(FUNC(icm3216_state::iop_ack));

	// The 32016 host and the Z80 IOP rendezvous through the mailbox at 0xa000a0: the host's
	// SCSI_WAIT (io/scsi.c) is a fixed 50000-iteration poll of the IOP status, and the IOP toggles
	// the BUSY bit (via its C013 status writes) as it runs a command.  A fine maximum quantum
	// interleaves the two cores tightly enough that the IOP makes progress on every host poll loop,
	// so BUSY clears well before the 50000-poll budget expires -- no artificial cycle-eating needed.
	config.set_maximum_quantum(attotime::from_usec(10));

	auto &scsi(NSCSI_BUS(config, "scsi"));
	// Bus IDs (ICM manual / kernel scsi.h): the NCR5385 initiator is strapped to id 0,
	// so targets occupy ids 1-7 -- root disk at id 1, tape at id 7 (above the initiator,
	// which is why the manual specifies id 7).  No connector at id 0: the 5385 is there.
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, "harddisk", false); // root disk, id 1 (-hard)
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7", scsi_devices, "tape", false); // tape, id 7 (-tape)


	NCR5385(config, m_scsi, 10'000'000);
	scsi.set_external_device(0, m_scsi); // initiator strapped to SCSI id 0
	m_scsi->set_own_id(0);               // /ID0-2 straps -> arbitration/reselection id 0
	m_scsi->irq().set(DEVICE_SELF, FUNC(icm3216_state::iop_int<2>));
	// DREQ drives the MiniBus DMA state machine (PALs) + '646 data path, NOT the Z80.
	// The Z80 only programs the address latch/counter (m_iop_waddr) and arms the chip.
	m_scsi->dreq().set(FUNC(icm3216_state::scsi_dreq_w));
}

void icm3216_state::parity_select_w(u8 data)
{
	LOGMASKED(LOG_PARITY, "parity %s (%s)\n", BIT(data, 0) ? "odd" : "even", machine().describe_context());

	if (m_parity_select != BIT(data, 0))
	{
		if (!m_parity)
		{
			LOGMASKED(LOG_PARITY, "parity handlers installed\n");

			// allocate and initialise parity store
			m_parity = std::make_unique<u8[]>(RAM_SIZE >> 3);
			if (m_parity_select)
				std::fill_n(m_parity.get(), RAM_SIZE >> 3, u8(0xff));

			// install parity handlers
			m_parity_mph = m_cpu->space(0).install_readwrite_tap(0, RAM_SIZE - 1, "parity",
				std::bind(&icm3216_state::parity_r, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
				std::bind(&icm3216_state::parity_w, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

			// count mismatched parity writes
			m_parity_delta = 1;
		}
		else if (!m_parity_count)
		{
			LOGMASKED(LOG_PARITY, "parity handlers removed\n");

			m_parity_mph.remove();
			m_parity.reset();

			m_parity_delta = 0;
		}
		else
			// invert mismatched parity count direction
			m_parity_delta *= -1;

		m_parity_select = BIT(data, 0);
	}
}

void icm3216_state::parity_enable_w(u8 data)
{
	LOGMASKED(LOG_PARITY, "parity %s (%s)\n", BIT(data, 0) ? "enabled" : "disabled", machine().describe_context());

	m_parity_enable = BIT(data, 0);
}

void icm3216_state::parity_r(offs_t offset, u16 &data, u16 mem_mask)
{
	if (!machine().side_effects_disabled() && m_parity_enable && !m_nmi_state)
	{
		LOGMASKED(LOG_PARITY, "parity r 0x%06x mask 0x%04x %s (%s)\n",
			offset, mem_mask, m_parity_select ? "odd" : "even", machine().describe_context());

		for (unsigned byte = 0; byte < 2; byte++)
		{
			if (BIT(mem_mask, byte * 8, 8))
			{
				if (BIT(m_parity[offset >> 3], BIT(offset, 1, 2) * 2 + byte) != m_parity_select)
					m_nmi_state |= NMI_PARITY | (NMI_EVEN << byte);
			}
		}

		if (m_nmi_state && m_nmi_enable)
		{
			LOGMASKED(LOG_PARITY, "parity error 0x%06x (%s)\n", offset, machine().describe_context());

			m_cpu->set_input_line(INPUT_LINE_NMI, 1);
		}
	}
}

void icm3216_state::parity_w(offs_t offset, u16 &data, u16 mem_mask)
{
	LOGMASKED(LOG_PARITY, "parity w 0x%06x mask 0x%04x %s (%s)\n",
		offset, mem_mask, m_parity_select ? "odd" : "even", machine().describe_context());

	for (unsigned byte = 0; byte < 2; byte++)
	{
		unsigned const parity_bit = BIT(offset, 1, 2) * 2 + byte;

		if (BIT(mem_mask, byte * 8, 8) && BIT(m_parity[offset >> 3], parity_bit) != m_parity_select)
		{
			if (m_parity_select)
				m_parity[offset >> 3] |= 1U << parity_bit;
			else
				m_parity[offset >> 3] &= ~(1U << parity_bit);

			m_parity_count += m_parity_delta;
		}
	}

	if (!m_parity_count)
	{
		LOGMASKED(LOG_PARITY, "parity handlers removed\n");

		m_parity_mph.remove();
		m_parity.reset();

		m_parity_delta = 0;
	}
}

u16 icm3216_state::nmi_status_r()
{
	u16 const data = m_nmi_state;

	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_PARITY, "nmi status 0x%04x (%s)\n", data, machine().describe_context());

		m_nmi_state = 0;
	}

	return data;
}

u8 icm3216_state::nmi_enable_r()
{
	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_PARITY, "nmi enable (%s)\n", machine().describe_context());

		m_nmi_enable = true;
	}

	return 0;
}

u8 icm3216_state::iop_r()
{
	return m_iop_sts;
}

void icm3216_state::iop_w(u8 data)
{
	/*
	 * cmd  function
	 *  0x00   write command pointer table (followed by address, lsb first)
	 *  0x01   acknowledge interrupt
	 *  0x02   reset i/o controller?
	 *  0x03   scsi reset
	 *  0x05   reset i/o controller?
	 *  0x1n   start i/o subchannel n
	 *  0x2n   abort i/o subchannel n
	 */
	LOG("iop_w 0x%02x (%s)\n", data, machine().describe_context());
	// snoop the "write command pointer table" sequence (cmd 0x00 followed by 3
	// base-address bytes, LSB first) and latch the cpt base. Bill's bridge uses this
	// base to supply the high address bits; the IOP then streams by writing only the
	// low byte to C010.
	if (m_iop_basecnt)
	{
		m_iop_base |= u32(data) << (8 * (3 - m_iop_basecnt));
		m_iop_basecnt--;
	}
	else
	{
		if (data == 0x00)
		{
			m_iop_base = 0;
			m_iop_basecnt = 3;
		}
	}
	if (m_hle)
	{
		// HLE: service the mailbox in software, bypassing the Z80 + ncr5385.
		// During the 3-byte base capture (m_iop_basecnt was just decremented) there is
		// no command to run; otherwise dispatch the command.
		if (!m_iop_basecnt && data != 0x00)
			hle_start_io(data);
		return;
	}

	m_iop_cmd = data;
	m_iop_cmd_pending = true; // C011 bit 0x40 -> IOP command poll @0x032e

	m_iop_sts |= IOP_BSY;

	iop_int<1>(1);
}

// HLE (simulated) SCSI I/O processor ----------------------------------------
// Host writes a mailbox command (0x1n = start i/o on subchannel n).  We latch it,
// raise BSY, and complete after a short delay -- the host polls iop_r() for BSY to
// clear.  The real work (read the iocb from 32016 memory, run the SCSI command
// against the disk image, write results + status back) happens in hle_complete().
void icm3216_state::hle_start_io(u8 cmd)
{
	switch (cmd & 0xf0)
	{
	case 0x10: // start i/o on subchannel (cmd & 0x0f)
		m_hle_cmd = cmd;
		m_iop_sts |= IOP_BSY;
		m_hle_timer->adjust(attotime::from_usec(50)); // mimic IOP latency
		break;
	case 0x00:
		if (cmd == 0x01) // acknowledge interrupt -> clear completion flags
			m_iop_sts &= ~(IOP_IRS | IOP_IID);
		else             // 0x02/0x03/0x05 resets
			m_iop_sts &= ~(IOP_BSY | IOP_IRS | IOP_IID);
		m_icu->ir_w<13>(0); // drop the SCSI completion interrupt
		break;
	default: // 0x2n abort etc.
		m_iop_sts &= ~IOP_BSY;
		break;
	}
}

u32 icm3216_state::hle_xlate(u32 dataptr, u32 tptptr, u32 off)
{
	if (!tptptr)
		return dataptr + off; // buffered: flat physical address
	// raw (B_PHYS): dataptr is the offset within the first buffer page; tptptr points
	// at the user buffer's PTE array.  Pages are 512 bytes; physical page = PTE & PFNMASK.
	address_space &m = m_cpu->space(0);
	u32 const pos = dataptr + off;
	offs_t const ptea = tptptr + (pos >> 9) * 4; // 4-byte PTEs, 512-byte pages
	u32 const pte = u32(m.read_byte(ptea)) | (u32(m.read_byte(ptea + 1)) << 8)
		| (u32(m.read_byte(ptea + 2)) << 16) | (u32(m.read_byte(ptea + 3)) << 24);
	return (pte & 0xfffe00u) | (pos & 0x1ff); // PFNMASK | in-page offset
}

TIMER_CALLBACK_MEMBER(icm3216_state::hle_complete)
{
	address_space &m = m_cpu->space(0);
	auto rd32 = [&m](offs_t a) -> u32 {
		return u32(m.read_byte(a)) | (u32(m.read_byte(a + 1)) << 8)
			| (u32(m.read_byte(a + 2)) << 16) | (u32(m.read_byte(a + 3)) << 24);
	};

	unsigned const sub = m_hle_cmd & 0x0f;
	u32 const iocb = rd32(m_iop_base + sub * 4); // cpt[sub] -> iocb pointer (LE)
	u8 const idbyte = m.read_byte(iocb + 1);
	unsigned const devid = (idbyte >> 4) & 7;
	unsigned const lun = idbyte & 7;
	u8 cdb[12];
	for (int i = 0; i < 12; i++)
		cdb[i] = m.read_byte(iocb + 2 + i);
	u32 const dataptr = rd32(iocb + 16);
	u32 const tptptr = rd32(iocb + 20); // raw-I/O page-table pointer (0 = buffered/flat)
	u32 const length = rd32(iocb + 24);

	u8 status = 0x00;     // SCSI status byte (00 = GOOD)
	u8 chanstatus = 0x00; // IOP channel status (00 = OK)

	auto put = [&](u32 off, u8 v) { m.write_byte(hle_xlate(dataptr, tptptr, off), v); };
	auto get = [&](u32 off) -> u8 { return m.read_byte(hle_xlate(dataptr, tptptr, off)); };

	u8 block[2048];

	// SCSI target IDs (devid == SCSI id; kernel scsi.h UNIT_TO_CHAN = unit>>2): the
	// initiator (NCR5385) is strapped to id 0, the root disk is id 1, and the tape is
	// id 7 (per the ICM manual -- Unix's /dev/rmt minor 224 -> channel 7).
	if (devid == 7 && lun == 0 && m_hle_tape && m_hle_tape->image()->get_file())
	{
		status = hle_tape(cdb, dataptr, tptptr, length, chanstatus);
	}
	else if (!(devid == 1 && lun == 0 && m_hle_disk && m_hle_disk->image->exists()))
	{
		// no device at this address: selection failure (encoding TBD, empirical)
		status = 0x02;
		chanstatus = 0x10;
	}
	else
	{
	u32 const secbytes = m_hle_disk->image->get_info().sectorbytes ? m_hle_disk->image->get_info().sectorbytes : 512;
	switch (cdb[0])
	{
	case 0x00: // TEST UNIT READY
	case 0x01: // REZERO UNIT (recalibrate) -- no-op for a disk image. The
		   // standalone monitor's _dcopen() (lib/libb2/dc.c) issues this
		   // before reading the block0 label; the kernel never did, which
		   // is why it wasn't needed until V1.283.
		break;

	case 0x12: // INQUIRY
	{
		std::vector<u8> inq;
		m_hle_disk->image->get_inquiry_data(inq);
		u8 buf[96];
		std::fill_n(buf, sizeof(buf), 0);
		buf[0] = 0x00; // direct-access device
		buf[1] = 0x00; // not removable
		buf[2] = 0x02; // SCSI-2
		buf[3] = 0x02;
		buf[4] = 0x1f; // additional length
		std::fill_n(buf + 8, 28, u8(' '));
		if (!inq.empty())
			std::copy_n(inq.begin(), std::min<size_t>(inq.size(), 28), buf + 8);
		else
			std::memcpy(buf + 8, "NSC     ICM-3216 HLE   ", 23);
		u32 const n = std::min<u32>(length, 36);
		for (u32 i = 0; i < n; i++) put(i, buf[i]);
		break;
	}

	case 0x25: // READ CAPACITY (10)
	{
		auto const &info = m_hle_disk->image->get_info();
		u32 const last = info.cylinders * info.heads * info.sectors - 1;
		put(0, last >> 24); put(1, last >> 16); put(2, last >> 8); put(3, last);
		put(4, info.sectorbytes >> 24); put(5, info.sectorbytes >> 16);
		put(6, info.sectorbytes >> 8); put(7, info.sectorbytes);
		break;
	}

	case 0x08: // READ (6)
	case 0x28: // READ (10)
	{
		u32 lba, blocks;
		if (cdb[0] == 0x08) { lba = (u32(cdb[1] & 0x1f) << 16) | (cdb[2] << 8) | cdb[3]; blocks = cdb[4] ? cdb[4] : 256; }
		else { lba = (u32(cdb[2]) << 24) | (cdb[3] << 16) | (cdb[4] << 8) | cdb[5]; blocks = (cdb[7] << 8) | cdb[8]; }
		for (u32 b = 0; b < blocks; b++)
		{
			if (!m_hle_disk->image->read(lba + b, block)) { status = 0x02; break; }
			for (u32 i = 0; i < secbytes; i++) put(b * secbytes + i, block[i]);
		}
		break;
	}

	case 0x0a: // WRITE (6)
	case 0x2a: // WRITE (10)
	{
		u32 lba, blocks;
		if (cdb[0] == 0x0a) { lba = (u32(cdb[1] & 0x1f) << 16) | (cdb[2] << 8) | cdb[3]; blocks = cdb[4] ? cdb[4] : 256; }
		else { lba = (u32(cdb[2]) << 24) | (cdb[3] << 16) | (cdb[4] << 8) | cdb[5]; blocks = (cdb[7] << 8) | cdb[8]; }
		for (u32 b = 0; b < blocks; b++)
		{
			for (u32 i = 0; i < secbytes; i++) block[i] = get(b * secbytes + i);
			if (!m_hle_disk->image->write(lba + b, block)) { status = 0x02; break; }
		}
		break;
	}

	case 0x03: // REQUEST SENSE
	{
		u8 buf[18];
		std::fill_n(buf, sizeof(buf), 0);
		buf[0] = 0x70; // current errors, valid
		buf[7] = 10;   // additional sense length
		u32 const n = std::min<u32>(length, 18);
		for (u32 i = 0; i < n; i++) put(i, buf[i]);
		break;
	}

	case 0x1a: // MODE SENSE (6) -- minimal header
	{
		u8 buf[4] = { 3, 0, 0, 0 };
		u32 const n = std::min<u32>(length, 4);
		for (u32 i = 0; i < n; i++) put(i, buf[i]);
		break;
	}

	case 0x15: // MODE SELECT -- accept (image geometry is fixed)
	case 0x04: // FORMAT UNIT -- the CHD is already a formatted image
		break;

	default:
		status = 0x02; // CHECK CONDITION
		break;
	}
	} // end disk command set

	m.write_byte(iocb + 14, status);     // iocb.status
	m.write_byte(iocb + 15, chanstatus); // iocb.chanstatus

	// completion: clear BSY, raise interrupt-request with the subchannel id.  The
	// host polls iop_r() for IRS (the monitor) and/or takes the interrupt (the
	// kernel's scsiintr).  Acks with mailbox cmd 0x01.
	m_iop_sts = u8((m_iop_sts & ~IOP_BSY) | IOP_IRS | (sub & IOP_IID));

	// signal the SCSI completion on NS32202 input 13 (ICU_SCSI=0x2000, per
	// buts/ns32000/sys/icu.h). The kernel programs IR13 edge-triggered and
	// active-low (ELTG bit13=0, TPL bit13=0), so a steady level never latches --
	// the ICU recognizes only a high->low edge. Pulse it; the falling edge sets
	// IPND bit 13 (cleared by the CPU's interrupt-acknowledge). The monitor is
	// unaffected -- it polls iop_r() for IRS rather than taking the interrupt.
	m_icu->ir_w<13>(1);
	m_icu->ir_w<13>(0);
}

// HLE SCSI tape (devid 6) command set, against the owned SIMH .tap image.
// Returns the SCSI status byte; sets chanstatus for filemark/EOM conditions.
u8 icm3216_state::hle_tape(u8 const *cdb, u32 dataptr, u32 tptptr, u32 length, u8 &chanstatus)
{
	address_space &m = m_cpu->space(0);
	tape_file_interface *tf = m_hle_tape->image()->get_file();
	auto put = [&](u32 off, u8 v) { m.write_byte(hle_xlate(dataptr, tptptr, off), v); };
	auto get = [&](u32 off) -> u8 { return m.read_byte(hle_xlate(dataptr, tptptr, off)); };
	u32 const blocklen = 512; // fixed block length
	u8 status = 0x00;

	switch (cdb[0])
	{
	case 0x00: // TEST UNIT READY
		break;

	case 0x01: // REWIND
		tf->rewind(false);
		break;

	case 0x12: // INQUIRY -- sequential-access (tape) device
	{
		u8 buf[36];
		std::fill_n(buf, sizeof(buf), 0);
		buf[0] = 0x01; // sequential-access device
		buf[1] = 0x80; // removable
		buf[2] = 0x02; // SCSI-2
		buf[3] = 0x02;
		buf[4] = 0x1f;
		std::fill_n(buf + 8, 28, u8(' '));
		std::memcpy(buf + 8, "NSC     ICM-3216 HLE TAPE", 25);
		u32 const n = std::min<u32>(length, 36);
		for (u32 i = 0; i < n; i++) put(i, buf[i]);
		break;
	}

	case 0x03: // REQUEST SENSE -- report a pending filemark/EOM if any
	{
		u8 buf[18];
		std::fill_n(buf, sizeof(buf), 0);
		buf[0] = 0x70;              // current errors, valid
		buf[2] = m_hle_tape_sense;  // EOM(0x40)/FILEMARK(0x80) + sense key
		buf[7] = 10;
		u32 const n = std::min<u32>(length, 18);
		for (u32 i = 0; i < n; i++) put(i, buf[i]);
		m_hle_tape_sense = 0;
		break;
	}

	case 0x05: // READ BLOCK LIMITS
		put(0, 0);
		put(1, 0); put(2, blocklen >> 8); put(3, blocklen & 0xff); // max block len
		put(4, blocklen >> 8); put(5, blocklen & 0xff);            // min block len
		break;

	case 0x08: // READ (6)
	{
		bool const fixed = cdb[1] & 0x01;
		u32 const count = get_u24be(&cdb[2]);
		u8 buf[2048];
		if (fixed)
		{
			for (u32 b = 0; b < count; b++)
			{
				auto const [st, len] = tf->read_block(buf, blocklen);
				bool const fm = (st == tape_status::FILEMARK || st == tape_status::FILEMARK_EW);
				bool const eod = (st == tape_status::EOD || st == tape_status::EOM);
				if (fm || eod)
				{
					status = 0x02;
					chanstatus = fm ? 0x01 : 0x02;
					m_hle_tape_sense = fm ? 0x80 : 0x40;
					// zero-fill the unread remainder of the requested transfer so a
					// partition-sized copy past a short tape file matches a clean
					// (formatted, zeroed) disk rather than leaving stale buffer data.
					for (u32 z = b; z < count; z++)
						for (u32 i = 0; i < blocklen; i++) put(z * blocklen + i, 0);
					break;
				}
				for (u32 i = 0; i < len && i < blocklen; i++) put(b * blocklen + i, buf[i]);
			}
		}
		else // variable: one record of up to `count` bytes
		{
			u32 const want = std::min<u32>(count, sizeof(buf));
			auto const [st, len] = tf->read_block(buf, want);
			for (u32 i = 0; i < len; i++) put(i, buf[i]);
			if (st == tape_status::FILEMARK || st == tape_status::FILEMARK_EW)
			{ chanstatus = 0x01; m_hle_tape_sense = 0x80; }
			else if (st == tape_status::EOD || st == tape_status::EOM)
			{ chanstatus = 0x02; m_hle_tape_sense = 0x40; }
		}
		break;
	}

	case 0x11: // SPACE
	{
		u8 const type = cdb[1] & 0x07; // 0=blocks 1=filemarks 3=EOD
		s32 count = s32(get_u24be(&cdb[2]));
		if (count & 0x00800000) count |= ~0x00ffffff; // sign-extend 24-bit
		if (type == 1)
			(count >= 0) ? (void)tf->space_filemarks(count) : (void)tf->space_filemarks_reverse(-count);
		else if (type == 0)
			(count >= 0) ? (void)tf->space_blocks(count) : (void)tf->space_blocks_reverse(-count);
		else if (type == 3)
			tf->space_eod();
		break;
	}

	case 0x0a: // WRITE (6)
	{
		bool const fixed = cdb[1] & 0x01;
		u32 const count = get_u24be(&cdb[2]);
		u8 buf[2048];
		if (fixed)
			for (u32 b = 0; b < count; b++)
			{
				for (u32 i = 0; i < blocklen; i++) buf[i] = get(b * blocklen + i);
				tf->write_block(buf, blocklen);
			}
		else
		{
			u32 const n = std::min<u32>(count, sizeof(buf));
			for (u32 i = 0; i < n; i++) buf[i] = get(i);
			tf->write_block(buf, n);
		}
		break;
	}

	case 0x10: // WRITE FILEMARKS
		tf->write_filemarks(get_u24be(&cdb[2]));
		break;

	case 0x1a: // MODE SENSE
	case 0x15: // MODE SELECT
		break;

	default:
		status = 0x02;
		break;
	}

	return status;
}

// iop<->host mailbox + main-memory DMA bridge (c010-c017).
// offset is relative to 0xc010.
u8 icm3216_state::iop_brg_r(offs_t offset)
{
	u8 data = 0;
	switch (offset)
	{
	case 0: // C010: host command in; reading it consumes the command (clears the
		// command-pending flag and the host BSY)
		data = m_iop_cmd;
		if (!machine().side_effects_disabled())
		{
			// Mailbox handshake: writing the command set BSY (host side); the IOP
			// reading it frees the command register (clears BSY) so the host may
			// queue the next command.  Completion is signalled separately via IRS
			// (C013).  Also clears the pending flag + acks the host interrupt.
			m_iop_sts &= ~IOP_BSY;
			m_iop_cmd_pending = false;
			iop_int<1>(0);
		}
		break;
	case 1: // C011: status. bit7(0x80)=data/bus ready (always, sync bridge);
		// bit6(0x40)=host command pending (polled at 0x032e)
		data = 0x80 | (m_iop_cmd_pending ? 0x40 : 0x00);
		break;
	case 2: // C012: low byte of the 16-bit main-memory word at the word address
		data = m_dma_space.read_byte(m_iop_waddr << 1);
		break;
	case 3: // C013: high byte of the word; reading completes the word -> advance
		data = m_dma_space.read_byte((m_iop_waddr << 1) + 1);
		if (!machine().side_effects_disabled())
			m_iop_waddr++;
		break;
	default:
		break;
	}
	return data;
}

void icm3216_state::iop_brg_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0: // C010 write = MiniBus DMA LOW address counter U44 (AD01-08), data-bus loaded.
		// The IOP firmware writes the low byte here and leads each transfer setup with
		// C010=0xFF (terminal count): U44=0xFF asserts ADOVF -> ADOVF2 -> IOP NMI, and the
		// handler (0x0233: exx; ld (C011),hl; ld (C014),a) loads U59 (C011) and U76 (C012)
		// with this segment's HIGH address from HL'.  So the full 24-bit MiniBus address is
		// supplied entirely by data-bus writes (C010 here, C011/C012 from the NMI handler) --
		// no CPU-register peek.  Within a transfer the 24-bit address simply counts up (the
		// '646/PAL counter chain), handled by scsi_dreq_w(); a new transfer re-seeds it here.
		//   word addr = (U76 << 15) | ((U59 >> 1) << 8) | U44 ;  byte = word << 1
		m_u44 = data;
		if (data == 0xff)
			m_iop->pulse_input_line(INPUT_LINE_NMI, attotime::zero); // ADOVF: load next segment
		m_iop_waddr = (u32(m_iop_hipage) << 15) | ((m_u59 >> 1) << 8) | m_u44;
		m_dma_hi = false;  // a fresh address load restarts the '646 lane at DMALODAT
		break;
	case 1: // C011 write = U59 high address latch (AD09-15), from the NMI handler ld (C011),hl
		m_u59 = data;
		m_iop_waddr = (u32(m_iop_hipage) << 15) | ((m_u59 >> 1) << 8) | m_u44;
		break;
	case 2: // C012 write = U76 page latch (A16-23), high half of the handler's ld (C011),hl
		m_iop_hipage = data;
		m_iop_waddr = (u32(m_iop_hipage) << 15) | ((m_u59 >> 1) << 8) | m_u44;
		break;
	case 3: // C013 write = the IOP's host-facing status register (the byte the host
		// reads via iop_r()).  The IOP mirrors its Z80 shadow ($6005) here: IRS|subchannel
		// (0x10|id) on completion, RST (0x20) on bus reset.  The host monitor polls iop_r()
		// for IRS; the kernel takes it as the NS32202 IR13 SCSI interrupt.
		//
		// Bit 7 (IOP_BSY) is the HARDWARE command-register-full handshake, NOT part of the
		// IOP's status byte: it is set when the host writes a command (iop_w) and cleared
		// when the IOP reads it (C010 read).  The IOP's status write must NOT clobber it --
		// otherwise the IOP's power-on status (it writes 0x80) makes the host's first
		// SCSI_WAIT poll see a spurious "command register busy" and print the cold-boot
		// "SCSI status register timeout - busy" (which real hardware does not).
		{
			bool const rising_irs = (data & IOP_IRS) && !(m_iop_sts & IOP_IRS);
			m_iop_sts = (m_iop_sts & IOP_BSY) | (data & ~IOP_BSY);
			if (rising_irs)
			{
				m_icu->ir_w<13>(1);
				m_icu->ir_w<13>(0);
			}
		}
		break;
	case 5: // C015: per-word transfer index (no host-visible side effect here)
		break;
	case 6: // C016 write = main-memory write, LOW byte (`ld (C016),hl` low half)
		m_iop_wlow = data;
		m_iop_wpending = true;
		break;
	case 7: // C017: if a C016 write is pending, this is its HIGH byte -> commit the
		// 16-bit word to main memory and advance; otherwise it's the read strobe
		if (m_iop_wpending)
		{
			m_dma_space.write_byte(m_iop_waddr << 1, m_iop_wlow);
			m_dma_space.write_byte((m_iop_waddr << 1) + 1, data);
			m_iop_waddr++;
			m_iop_wpending = false;
		}
		// (BSY is owned by the IOP via C013, not poked here.)
		break;
	// C013(3)=subchannel, C015(5)=word index, C016(6) -- not address bits here
	default:
		break;
	}
}

// MiniBus DMA engine (PAL state machine + 74LS646 pair U60/U61).  The NCR5385 is a
// byte-wide device on the Z80 data bus; main memory is the 16-bit 32016 bus.  On each
// 5385 DREQ the '646 shuttles ONE byte: two chip bytes pack into one 32016 word store
// (DATA-IN/STATUS), or one 32016 word fetch unpacks into two chip bytes (COMMAND/
// DATA-OUT).  The Z80 only programs the address latch/counter (m_iop_waddr, loaded via
// C010/C015) and arms the chip; it is NOT in the per-byte loop.  DMALODAT/DMAHIDAT
// (m_dma_hi) select the byte lane; the word counter advances every second byte.
void icm3216_state::scsi_dreq_w(int state)
{
	if (!state)
		return;

	u32 const ctrl = m_scsibus->ctrl_r();
	bool const in = (ctrl & nscsi_device_interface::S_INP);
	offs_t const ba = (m_iop_waddr << 1) | (m_dma_hi ? 1u : 0u);


	if (in)
		m_dma_space.write_byte(ba, m_scsi->dma_r());   // target -> 32016
	else
		m_scsi->dma_w(m_dma_space.read_byte(ba));       // 32016 -> target

	if (m_dma_hi)
		m_iop_waddr++;          // a full 32016 word has now transferred
	m_dma_hi = !m_dma_hi;
}

// iop interrupt vector bits
// bit  source
//  1   host?
//  2   scsi
//  3   reset?

IRQ_CALLBACK_MEMBER(icm3216_state::iop_ack)
{
	u8 const data = m_iop_vec;

	m_iop->set_input_line(INPUT_LINE_IRQ0, 0);

	return data;
}

template <unsigned Source> void icm3216_state::iop_int(int state)
{
	if (state)
		m_iop_vec |= 1U << Source;
	else
		m_iop_vec &= ~(1U << Source);

	// OR the sources: the host and SCSI interrupts share IRQ0, so the line must stay
	// asserted while ANY source is pending (previously each source drove the line
	// directly, so deasserting one could mask the other).
	m_iop->set_input_line(INPUT_LINE_IRQ0, m_iop_vec ? ASSERT_LINE : CLEAR_LINE);
}

ROM_START(icm3216)
	ROM_REGION16_LE(0x20000, "eprom", 0)

	// V2.44 does not boot the UNIX System V distribution; V1.283 is required, so
	// it is the default.
	ROM_DEFAULT_BIOS("v1283")

	ROM_SYSTEM_BIOS(0, "v244", "ICM-3216 Rom Monitor V2.44")
	ROMX_LOAD("610289_002__rev_b_u34.u34", 0x0000, 0x8000, CRC(fb542bd1) SHA1(39b7f1f90ffdae07ddc547eff564096bd2092d58), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("610289_002__rev_b_u35.u35", 0x0001, 0x8000, CRC(78cfc8dd) SHA1(bb87f068a6ab7ef82d8865154856064b9e8a99a6), ROM_BIOS(0) | ROM_SKIP(1))

	// V1.283 (Dec 1985): rebuilt from the monitor source recovered on the SYS3
	// tape (stand/pivot/rommon, "ICM-3216 Rom Monitor V1.283"). Unlike V2.44 this
	// rev's boot ("B" = "x unix") loads /unix directly as COFF with no checksum,
	// matching the V1.283-era root filesystem on the install media.
	// The recovered rebuild left a blank (0xffff) checksum trailer, so rom_chk.s
	// flagged a ROM checksum fault ("Powerup test failure").  The last two bytes of
	// each half are stamped with the computed checksum (rom_chk.s algorithm: byte sum
	// with end-around carry over all but the final 4 bytes) -- u34=0xd2ec, u35=0xbe03
	// -- as ICM's build would have, so the monitor's confidence test passes.
	ROM_SYSTEM_BIOS(1, "v1283", "ICM-3216 Rom Monitor V1.283")
	ROMX_LOAD("610289_001__v1_283_u34.u34", 0x0000, 0x8000, CRC(36296e83) SHA1(f2ad1d0302befdb75ff61f85292363980031b458), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("610289_001__v1_283_u35.u35", 0x0001, 0x8000, CRC(f0aff0bd) SHA1(341e931f6a0ec894c0fdc9605d8e859a4dbeeb6f), ROM_BIOS(1) | ROM_SKIP(1))

	ROM_REGION(0x4000, "iop", 0)
	ROM_LOAD("600045_003__rev_a.u29", 0x0000, 0x4000, CRC(865e3e8b) SHA1(c6e47d304fd67a9b384a139ef917913fc60c0b32))
ROM_END

}

/*   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                   FULLNAME    FLAGS */
COMP(1985, icm3216, 0,      0,      icm3216, icm3216, icm3216_state, empty_init, "National Semiconductor", "ICM-3216", MACHINE_NO_SOUND_HW)
