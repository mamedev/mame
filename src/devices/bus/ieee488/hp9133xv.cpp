// license:BSD-3-Clause
// copyright-holders: Sven Schnelle, Pascal Gervaix
/*********************************************************************

    hp9133xv.cpp

    HP 9133V/XV combined disc drive, low level emulation.

    Each model is a unit: a Winchester board and, except on the 9134XV, a
    floppy board, behind one HP-IB connector; a "chain" slot on the unit
    takes the next device on the bus. Each board is a separate HP-IB
    device, with its own 68B09 running its original firmware:

      GPIB_HP9133XV_FDD  floppy board      68B09 + i8291A + MB8876
                         ROM 09133-15500, reset $E000, DP = $40
                         $2000-$2003 MB8876, $4000-$4007 i8291A,
                         $6000 control/status latch

      GPIB_HP9133XV_HDD  Winchester board  68B09 + i8291A + WD1010
                         ROM 09133-15520, reset $EE48, DP = $00
                         $0000 control latch, $0020 address/strap port,
                         $0040-$0047 i8291A, $0060-$0067 WD1010

    The Winchester board and its ROM are the same in the 9133V, 9133XV and
    9134XV: at boot ($E390) the firmware reads the J3 straps at $0020 and
    picks a personality record, which carries the geometry and the AMIGO
    identify bytes.

    Five bus options expose the unit with the straps preset: hp9133v,
    hp9133v004, hp9133xv, hp9133xv010 (option 010, 10M) and hp9134xv.

    The floppy board model started from hp9122c.cpp.

    Reference: HP's 5 1/4-Inch Winchester Disc Drive Service Documentation
    (HP Greeley Division), HP 9133V/XV & 9134XV Disc Memory User's Manual,
    HP 9133/34 V/XV/D/H (5957-6568) and HP's 3 1/2-Inch Flexible Disc Drive
    Service Documentation; board schematics by Tony Duell. The Winchester
    board carries a WD1010AL-00: the WD1010-00/01 data sheet (WD 1984
    databook) agrees with the WD1010-05 pages cited below.

*********************************************************************/

#include "emu.h"
#include "hp9133xv.h"

#include "cpu/m6809/m6809.h"
#include "imagedev/floppy.h"
#include "imagedev/harddriv.h"
#include "machine/i8291a.h"
#include "machine/wd1010.h"
#include "machine/wd_fdc.h"

#include <algorithm>
#include <iterator>

#define LOG_FDD_REG  (1U << 1)   // floppy board: $6000 command/status latch
#define LOG_FDD_XFER (1U << 2)   // floppy board: MB8876 accesses and sector transfers
#define LOG_HDD_REG  (1U << 3)   // Winchester board: $0000 latch and WD1010 task file
#define LOG_HDD_XFER (1U << 4)   // Winchester board: WD1010 to CHD shim (write/format commits)
#define LOG_GPIB     (1U << 5)   // both boards: 8291A INT/DREQ and the DAV/NRFD/NDAC handshake lines
#define LOG_IRQ      (1U << 6)   // both boards: IRQ/FIRQ/HALT state seen by the 6809

#define VERBOSE 0
#include "logmacro.h"

#define LOGFDDREG(...)  LOGMASKED(LOG_FDD_REG,  __VA_ARGS__)
#define LOGFDDXFER(...) LOGMASKED(LOG_FDD_XFER, __VA_ARGS__)
#define LOGHDDREG(...)  LOGMASKED(LOG_HDD_REG,  __VA_ARGS__)
#define LOGHDDXFER(...) LOGMASKED(LOG_HDD_XFER, __VA_ARGS__)
#define LOGGPIB(...)    LOGMASKED(LOG_GPIB,     __VA_ARGS__)
#define LOGIRQ(...)     LOGMASKED(LOG_IRQ,      __VA_ARGS__)

DECLARE_DEVICE_TYPE(GPIB_HP9133XV_FDD, device_ieee488_interface)
DECLARE_DEVICE_TYPE(GPIB_HP9133XV_HDD, device_ieee488_interface)

namespace {

class hp9133xv_fdd_device : public device_t,
							public device_ieee488_interface
{
public:
	hp9133xv_fdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_ieee488_interface implementation
	virtual void ieee488_eoi(int state) override;
	virtual void ieee488_dav(int state) override;
	virtual void ieee488_nrfd(int state) override;
	virtual void ieee488_ndac(int state) override;
	virtual void ieee488_ifc(int state) override;
	virtual void ieee488_srq(int state) override;
	virtual void ieee488_atn(int state) override;
	virtual void ieee488_ren(int state) override;

private:
	// Bits of the $6000 control port (write). Sheet 2 names the '273 outputs, whose data bit order is
	// 2,3,6,7,5,4,1,0: b0 Dr-SideSel, b1 D-Motor-On, b2 Dr-HdLd, b3 Cl-Index, b4 Dr-Sel1, b5 Dr-Sel0,
	// b6 FDCErr (panel LED), b7 HaltEn.
	static constexpr int REG_CNTL_MOTOR    = 1 << 1;
	static constexpr int REG_CNTL_ACK_IRQ  = 1 << 3;
	static constexpr int REG_CNTL_DRVSEL0  = 1 << 5;
	static constexpr int REG_CNTL_ARM_FDC  = 1 << 7;

	// Bits of the $6000 status port (read). b7 is Dr-Rdy inverted (sheet 2: U16c '04 into '244 U5
	// input e), not a readback of the b7 written to the control port -- see status_r().
	static constexpr int REG_STATUS_TEST        = 1 << 4;
	static constexpr int REG_STATUS_BOOT_STRAP  = 1 << 5;
	static constexpr int REG_STATUS_NOT_READY   = 1 << 7;

	void i8291a_eoi_w(int state);
	void i8291a_dav_w(int state);
	void i8291a_nrfd_w(int state);
	void i8291a_ndac_w(int state);
	void i8291a_srq_w(int state);
	uint8_t i8291a_dio_r();
	void i8291a_dio_w(uint8_t data);
	void i8291a_int_w(int state);
	void i8291a_dreq_w(int state);

	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);

	void cmd_w(uint8_t data);
	uint8_t status_r();

	void cpu_map(address_map &map) ATTR_COLD;
	void connect_drive();
	void index_pulse_cb(floppy_image_device *floppy, int state);
	void update_irqs();
	void update_halt();

	TIMER_CALLBACK_MEMBER(firq_delayed);

	required_device<cpu_device> m_cpu;
	required_device<i8291a_device> m_i8291a;
	required_device<mb8876_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_ioport m_hpib_addr;
	required_ioport m_testmode;

	bool m_fdc_irq;
	bool m_fdc_drq;
	bool m_cpuirq;
	bool m_cpufirq;
	bool m_index_int;
	bool m_hpib_int;     // 8291A INT pin asserted
	bool m_drive0_sel;   // Dr-Sel0 state: only then is the fitted drive connected to the FDC
	bool m_arm_fdc;    // b7 ARM_FDC latch -- arms the FDC data path (HALT-pacing + end-of-command FIRQ)
	bool m_fdc_busy;   // FDC command in progress (write cmd -> INTRQ)
	bool m_cpuhalt;    // current state of the 6809's HALT line

	emu_timer *m_firq_timer;
};

hp9133xv_fdd_device::hp9133xv_fdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t{mconfig, GPIB_HP9133XV_FDD, tag, owner, clock},
	device_ieee488_interface{mconfig, *this},
	m_cpu{*this, "cpu"},
	m_i8291a{*this, "i8291a"},
	m_fdc{*this, "mb8876"},
	m_floppy{*this, "floppy0"},
	m_hpib_addr{*this, "ADDRESS"},
	m_testmode{*this, "TESTMODE"},
	m_cpuirq{false},   // read by device_reset
	m_cpufirq{false}
{
}

static INPUT_PORTS_START(hp9133xv_fdd_port)
	// Factory setting for the flexible disc, per the User's Manual p.8, figure 2-3.
	PORT_START("ADDRESS")
	PORT_CONFNAME(0x7, 0x2, "HPIB address")
	PORT_CONFSETTING(0, "0")
	PORT_CONFSETTING(1, "1")
	PORT_CONFSETTING(2, "2")
	PORT_CONFSETTING(3, "3")
	PORT_CONFSETTING(4, "4")
	PORT_CONFSETTING(5, "5")
	PORT_CONFSETTING(6, "6")
	PORT_CONFSETTING(7, "7")

	PORT_START("TESTMODE")
	PORT_CONFNAME(0x01, 0x00, "Test mode")
	PORT_CONFSETTING(0, DEF_STR(No))
	PORT_CONFSETTING(1, DEF_STR(Yes))
INPUT_PORTS_END

ioport_constructor hp9133xv_fdd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp9133xv_fdd_port);
}

void hp9133xv_fdd_device::device_start()
{
	m_floppy->get_device()->setup_index_pulse_cb(
		floppy_image_device::index_pulse_cb(&hp9133xv_fdd_device::index_pulse_cb, this));

	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_cpuirq));
	save_item(NAME(m_cpufirq));
	save_item(NAME(m_index_int));
	save_item(NAME(m_hpib_int));
	save_item(NAME(m_drive0_sel));
	save_item(NAME(m_arm_fdc));
	save_item(NAME(m_fdc_busy));
	save_item(NAME(m_cpuhalt));

	m_firq_timer = timer_alloc(FUNC(hp9133xv_fdd_device::firq_delayed), this);
}

TIMER_CALLBACK_MEMBER(hp9133xv_fdd_device::firq_delayed)
{
	m_fdc_irq = true;
	update_irqs();
	update_halt();
}

void hp9133xv_fdd_device::device_reset()
{
	m_fdc_irq = false;
	m_fdc_drq = false;
	// The m6809 core keeps its input lines across reset: release them so that the mirrors, which gate
	// set_input_line on a change, agree with the CPU.
	if (m_cpufirq) {
		m_cpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
		m_cpufirq = false;
	}
	if (m_cpuirq) {
		m_cpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
		m_cpuirq = false;
	}
	m_index_int = false;
	m_hpib_int = false;
	m_arm_fdc = false;
	m_fdc_busy = false;
	m_cpuhalt = false;
	m_firq_timer->adjust(attotime::never);
	m_cpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	// The '273 latch comes out of reset clear, so no drive is selected yet: the FDC sees an empty bus
	// until the firmware asserts Dr-Sel0 (see cmd_w).
	m_drive0_sel = false;
	connect_drive();
	m_floppy->get_device()->mon_w(1);   // motor off out of reset; the firmware starts it through b1 of $6000
}

void hp9133xv_fdd_device::device_post_load()
{
	// Only Dr-Sel0 is saved: reconnect the drive it selects.
	connect_drive();
}

void hp9133xv_fdd_device::connect_drive()
{
	m_fdc->set_floppy(m_drive0_sel ? m_floppy->get_device() : nullptr);
	// wd_fdc::set_floppy replaces the drive's index callback; reinstall ours.
	m_floppy->get_device()->setup_index_pulse_cb(
		floppy_image_device::index_pulse_cb(&hp9133xv_fdd_device::index_pulse_cb, this));
}

void hp9133xv_fdd_device::ieee488_eoi(int state)  { m_i8291a->eoi_w(state); }
void hp9133xv_fdd_device::ieee488_dav(int state)  { m_i8291a->dav_w(state); }
void hp9133xv_fdd_device::ieee488_nrfd(int state) { m_i8291a->nrfd_w(state); }
void hp9133xv_fdd_device::ieee488_ndac(int state) { m_i8291a->ndac_w(state); }
void hp9133xv_fdd_device::ieee488_ifc(int state)  { m_i8291a->ifc_w(state); }
void hp9133xv_fdd_device::ieee488_srq(int state)  { m_i8291a->srq_w(state); }
void hp9133xv_fdd_device::ieee488_atn(int state)  { m_i8291a->atn_w(state); }
void hp9133xv_fdd_device::ieee488_ren(int state)  { m_i8291a->ren_w(state); }

void hp9133xv_fdd_device::i8291a_eoi_w(int state)  { m_bus->eoi_w(this, state); }
void hp9133xv_fdd_device::i8291a_dav_w(int state)  { LOGGPIB("%s DAV %d\n",  machine().describe_context(), state); m_bus->dav_w(this, state); }
void hp9133xv_fdd_device::i8291a_nrfd_w(int state) { LOGGPIB("%s NRFD %d\n", machine().describe_context(), state); m_bus->nrfd_w(this, state); }
void hp9133xv_fdd_device::i8291a_ndac_w(int state) { LOGGPIB("%s NDAC %d\n", machine().describe_context(), state); m_bus->ndac_w(this, state); }
void hp9133xv_fdd_device::i8291a_srq_w(int state)  { m_bus->srq_w(this, state); }

void hp9133xv_fdd_device::i8291a_int_w(int state)
{
	LOGIRQ("8291A INT %d, CPU %s\n", state, m_cpu->suspended(SUSPEND_REASON_HALT) ? "halted" : "running");
	// The firmware sets AUXB bit 3 ($E0EF), so the pin is active low.
	m_hpib_int = (state == 0);
	update_irqs();
}

void hp9133xv_fdd_device::i8291a_dreq_w(int state)
{
	// DREQ reaches nothing on this board (sheet 1): trace only.
	LOGGPIB("%s 8291A DREQ %d\n", machine().describe_context(), state);
}

void hp9133xv_fdd_device::fdc_intrq_w(int state)
{
	if (state) {
		// Release HALT first so the STA of the last byte completes, then raise FIRQ (see update_halt):
		// 8 us is one pass of the 16-cycle read loop at 2 MHz.
		m_fdc_busy = false;
		update_halt();
		m_firq_timer->adjust(attotime::from_usec(8));
	} else {
		m_fdc_irq = false;
		m_firq_timer->adjust(attotime::never);
		update_irqs();
		update_halt();
	}
}

void hp9133xv_fdd_device::fdc_drq_w(int state)
{
	m_fdc_drq = state;
	update_irqs();
	update_halt();
}

// With the data path armed (b7), the board holds the 6809 in HALT between bytes: the READ SECTOR loop
// at $E571 (LDA $2003 / COMA / STA ,X+ / BRA, 16 cycles) reads $2003 without testing DRQ. DRQ wakes
// the CPU for one byte; INTRQ wakes it for good and the FIRQ handler at $E101 ejects the loop with
// LEAS 3,S -- which must not preempt the STA of the last byte, hence the delayed FIRQ in fdc_intrq_w.
void hp9133xv_fdd_device::update_halt()
{
	bool halt = m_arm_fdc && m_fdc_busy && !m_fdc_drq && !m_fdc_irq;
	if (halt != m_cpuhalt) {
		LOGIRQ("halt %d (arm %d busy %d drq %d irq %d)\n", halt, m_arm_fdc, m_fdc_busy, m_fdc_drq, m_fdc_irq);
		m_cpu->set_input_line(INPUT_LINE_HALT, halt ? ASSERT_LINE : CLEAR_LINE);
		m_cpuhalt = halt;
	}
}

uint8_t hp9133xv_fdd_device::fdc_r(offs_t offset)
{
	uint8_t v = m_fdc->read(offset);
	if (offset == 3 && !machine().side_effects_disabled())
		LOGFDDXFER("%s FDC data read %02X\n", machine().describe_context(), v);
	return v;
}

void hp9133xv_fdd_device::fdc_w(offs_t offset, uint8_t data)
{
	if (offset == 0) {
		// the real command is ~data (the mb8876 inverts the bus internally). Force-interrupt (real $Dx,
		// written as $2F by the firmware) does not keep busy set.
		const uint8_t cmd = ~data & 0xff;
		LOGFDDXFER("%s FDC command %02X (bus %02X), track %02X\n", machine().describe_context(), cmd, data, ~m_fdc->track_r() & 0xff);
		m_fdc_busy = ((cmd & 0xf0) != 0xd0);
	}
	m_fdc->write(offset, data);
	if (offset == 0)
		update_halt();
}

uint8_t hp9133xv_fdd_device::i8291a_dio_r()        { return m_bus->dio_r(); }
void hp9133xv_fdd_device::i8291a_dio_w(uint8_t data) { m_bus->dio_w(this, data); }

void hp9133xv_fdd_device::update_irqs()
{
	// This board has no INTSEL multiplexer: the two interrupt lines have fixed sources.
	// /FIRQ (sheet 1 of 09133-66506) = U1f, low for the armed FDC INTRQ (U3b), for the 8291A INT (U6a)
	// or while Dr-Rdy is false (U2b). The $E101 handler does LEAS 3,S to eject the CPU from the
	// transfer loop, reads the status, issues a cleanup forced-interrupt and returns. INT is what wakes
	// the SYNC of the two HP-IB pumps, and a drive that is not selected holds READY false (HP's 3 1/2-Inch
	// Flexible Disc Drive Service Documentation, p. 2-4).
	floppy_image_device *const drive = m_drive0_sel ? m_floppy->get_device() : nullptr;
	const bool dr_rdy = drive && !drive->ready_r();
	bool firq = (m_arm_fdc && m_fdc_irq) || m_hpib_int || !dr_rdy;
	// IRQ = INDEX interrupt: flip-flop U17a '74 (sheet 2 of 09133-66506), clocked by Dr-Index and
	// reset by Cl-Index (b3 of the latch); the firmware's time base (ISR $EABA).
	bool irq = m_index_int;

	if (m_cpufirq != firq) {
		m_cpu->set_input_line(M6809_FIRQ_LINE, firq ? ASSERT_LINE : CLEAR_LINE);
		m_cpufirq = firq;
	}
	if (m_cpuirq != irq) {
		m_cpu->set_input_line(M6809_IRQ_LINE, irq ? ASSERT_LINE : CLEAR_LINE);
		m_cpuirq = irq;
	}
}

uint8_t hp9133xv_fdd_device::status_r()
{
	// Read $6000: HP-IB address (bits 0-2), testmode, boot strap, Dr-Rdy (see below).
	// b5 is the double-sided strap, sampled once at boot ($E049). The select routine $E246 turns a unit
	// number into a latch pattern: unit 0 -> $20 (Dr-Sel0), unit 1 -> $10 (Dr-Sel1), units 2 and 3 ->
	// $0191/$0192, which the strap fills with $21/$11 (select + Dr-SideSel) when low and $00/$00 when
	// high. Units 2 and 3 are therefore the second SIDE of drives 0 and 1. Held high here, which is
	// what a single-sided OA-D31V needs: no pattern, so no side-1 unit exists.
	uint8_t ret = REG_STATUS_BOOT_STRAP;
	auto addr = m_hpib_addr->read();
	if (!m_testmode->read())
		ret |= REG_STATUS_TEST;
	ret |= (addr & 0x07);

	// b7 is Dr-Rdy inverted (sheet 2: U16c '04 into U5 '244 input e). The FIRQ handler $E101 merges it
	// into the op result at $0166, which $E93F then decodes as NOT READY (S1=$13, DSJ=1).
	floppy_image_device *const drive = m_drive0_sel ? m_floppy->get_device() : nullptr;
	if (!drive || drive->ready_r())
		ret |= REG_STATUS_NOT_READY;
	if (!machine().side_effects_disabled())
		LOGFDDREG("%s status read $6000 => %02X\n", machine().describe_context(), ret);
	return ret;
}

void hp9133xv_fdd_device::cmd_w(uint8_t data)
{
	LOGFDDREG("%s command write $6000 <= %02X\n", machine().describe_context(), data);
	// b5/b4 are the two drive-select lines. A 9133XV carries one floppy, wired to Dr-Sel0; the Dr-Sel1
	// connector is empty. The firmware still probes drive 1 ($E404 restores each unit in turn), so the
	// fitted drive has to leave the bus while Dr-Sel1 is asserted.
	const bool sel0 = (data & REG_CNTL_DRVSEL0) != 0;
	if (sel0 != m_drive0_sel) {
		m_drive0_sel = sel0;
		connect_drive();
	}

	floppy_image_device *floppy = sel0 ? m_floppy->get_device() : nullptr;
	if (floppy) {
		// b1 drives the motor; mon_w is active low.
		floppy->mon_w((data & REG_CNTL_MOTOR) ? 0 : 1);
		floppy->ss_w(0);                  // single-sided drive, no side-select input
	}

	// b2 = Dr-HdLd is not decoded: the emulated head is always loaded.

	// b7 = ARM_FDC: arms/disarms the FDC data path (HALT pacing + end-of-command FIRQ, see update_irqs).
	m_arm_fdc = (data & REG_CNTL_ARM_FDC) != 0;

	// b3 = Cl-Index resets the index interrupt flip-flop (sheet 2, U17a); the $EABA ISR pulses it each
	// tick, or the IRQ stays asserted and $EABA storms.
	if (data & REG_CNTL_ACK_IRQ)
		m_index_int = false;

	update_irqs();
	update_halt();
}

void hp9133xv_fdd_device::index_pulse_cb(floppy_image_device *floppy, int state)
{
	// Index comes off the selected drive: with Dr-Sel1 asserted the fitted drive is not on the bus.
	if (!m_drive0_sel)
		return;
	m_fdc->index_callback(floppy, state);
	if (state)
		m_index_int = true;
	update_irqs();
}

void hp9133xv_fdd_device::cpu_map(address_map &map)
{
	// U11 '138 selects on A13-A15 only (sheet 1), so every block repeats over its whole 8K window;
	// the RAM is mapped as 2K at $0000 (stack at $03FF).
	map(0x0000, 0x07ff).ram();
	// fdc_w tracks the command in progress for update_halt
	map(0x2000, 0x2003).mirror(0x1ffc).rw(FUNC(hp9133xv_fdd_device::fdc_r), FUNC(hp9133xv_fdd_device::fdc_w));
	map(0x4000, 0x4007).mirror(0x1ff8).m(m_i8291a, FUNC(i8291a_device::map));
	map(0x6000, 0x7fff).rw(FUNC(hp9133xv_fdd_device::status_r), FUNC(hp9133xv_fdd_device::cmd_w));
	map(0xe000, 0xffff).rom().region("cpu", 0);                                         // ROM 09133-15500 (8K)
}

static void hp9133xv_fdd_floppies(device_slot_interface &device)
{
	device.option_add("oa_d31v", SONY_OA_D31V);  // the fitted drive, 3.5" single-sided
}

ROM_START(hp9133xv_fdd)
	ROM_REGION(0x2000, "cpu", 0)
	// 2764 at U22 (Duell schematics, 09133-66506 sheet 2); part number in 5957-6568, p. 8-1
	ROM_LOAD("09133-15500.u22", 0x0000, 0x2000, CRC(135ab652) SHA1(ef06409a3540f80aee7d1a85c0215324ae73b8ba))
ROM_END

const tiny_rom_entry *hp9133xv_fdd_device::device_rom_region() const
{
	return ROM_NAME(hp9133xv_fdd);
}

void hp9133xv_fdd_device::device_add_mconfig(machine_config &config)
{
	MC6809(config, m_cpu, 8_MHz_XTAL);   // 68B09 clocked by U9, the Data Separator board's master oscillator
	m_cpu->set_addrmap(AS_PROGRAM, &hp9133xv_fdd_device::cpu_map);

	MB8876(config, m_fdc, 8_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(hp9133xv_fdd_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(hp9133xv_fdd_device::fdc_drq_w));

	// The 8291A runs off HPIBClk: 8 MHz / 4 from U16 '393 on the data separator board (sheet 1 of
	// 09133-66507), brought in on P1.
	I8291A(config, m_i8291a, 8_MHz_XTAL / 4);
	m_i8291a->eoi_write().set(FUNC(hp9133xv_fdd_device::i8291a_eoi_w));
	m_i8291a->dav_write().set(FUNC(hp9133xv_fdd_device::i8291a_dav_w));
	m_i8291a->nrfd_write().set(FUNC(hp9133xv_fdd_device::i8291a_nrfd_w));
	m_i8291a->ndac_write().set(FUNC(hp9133xv_fdd_device::i8291a_ndac_w));
	m_i8291a->srq_write().set(FUNC(hp9133xv_fdd_device::i8291a_srq_w));
	m_i8291a->dio_write().set(FUNC(hp9133xv_fdd_device::i8291a_dio_w));
	m_i8291a->dio_read().set(FUNC(hp9133xv_fdd_device::i8291a_dio_r));
	m_i8291a->int_write().set(FUNC(hp9133xv_fdd_device::i8291a_int_w));
	m_i8291a->dreq_write().set(FUNC(hp9133xv_fdd_device::i8291a_dreq_w));

	FLOPPY_CONNECTOR(config, m_floppy, hp9133xv_fdd_floppies, "oa_d31v",
			floppy_image_device::default_mfm_floppy_formats, true).enable_sound(true);
}

//====================================================================
// hp9133xv_hdd_device -- Winchester board, ROM 09133-15520.
//
// 68B09 + i8291A + WD1010 running the original firmware. The board sits between the WD1010 and the
// CPU with a 512 byte sector buffer (4801 RAM addressed by a hardware counter, not by the CPU bus),
// which the firmware reaches through the auto-incrementing port at $0060. The head number is taken from
// the SDH register write, the WD1010 having no head-select outputs (WD1010-05 p. 3-9); its INTRQ is
// not wired, the firmware polls status $0067.
//====================================================================

class hp9133xv_hdd_device : public device_t,
							public device_ieee488_interface
{
public:
	hp9133xv_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void ieee488_eoi(int state) override;
	virtual void ieee488_dav(int state) override;
	virtual void ieee488_nrfd(int state) override;
	virtual void ieee488_ndac(int state) override;
	virtual void ieee488_ifc(int state) override;
	virtual void ieee488_srq(int state) override;
	virtual void ieee488_atn(int state) override;
	virtual void ieee488_ren(int state) override;

private:
	// Status and error bytes for the commands answered by the board (see hdc_r), with the bit layout of
	// wd1010_device.
	static constexpr uint8_t STATUS_RDY_SC     = 0x50;   // command done
	static constexpr uint8_t STATUS_RDY_SC_DRQ = 0x58;   // buffer waiting for the CPU
	static constexpr uint8_t STATUS_RDY_SC_ERR = 0x51;   // command failed, see the error register
	static constexpr uint8_t ERROR_AC          = 0x04;   // aborted command
	static constexpr uint8_t ERROR_ID          = 0x10;   // ID not found

	void i8291a_eoi_w(int state);
	void i8291a_dav_w(int state);
	void i8291a_nrfd_w(int state);
	void i8291a_ndac_w(int state);
	void i8291a_srq_w(int state);
	uint8_t i8291a_dio_r();
	void i8291a_dio_w(uint8_t data);
	void i8291a_int_w(int state);
	void i8291a_dreq_w(int state);

	void latch_w(uint8_t data);
	uint8_t addr_r();
	uint8_t hdc_r(offs_t offset);
	void hdc_w(offs_t offset, uint8_t data);
	void hdc_intrq_w(int state);
	void hdc_bdrq_w(int state);
	void hdc_bcr_w(int state);
	uint8_t hdc_data_in();
	void hdc_data_out(uint8_t data);
	void update_firq();
	std::error_condition hdd_load(device_image_interface &);
	void write_image(uint32_t lba, const void *buffer);

	void cpu_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_cpu;
	required_device<i8291a_device> m_i8291a;
	required_device<wd1010_device> m_hdc;
	required_device<harddisk_image_device> m_hdd0;
	required_ioport m_hpib_addr;
	required_ioport m_model;   // J3 jumpers (C/B/A) of board 09133-66508 -> bits 3-5 of port $0020

	uint8_t m_latch;          // last byte written to the $00 latch (board control)

	// Sector buffer: a 512-byte RAM (U41, sheet 4) between the 6809, through the auto-incrementing
	// port at $60, and the WD1010, through the buffer-chip callbacks. Bit2 low of the $00 latch clears
	// the address counter.
	uint8_t m_secbuf[512];
	// The board has one buffer counter ('393 pair, sheet 4, 10 bits wide); the CPU and the wd1010
	// callbacks keep their own index here, both reset by BCR.
	uint16_t m_bufcnt;        // CPU side ($60)
	uint16_t m_wdidx;         // WD1010 side
	uint8_t m_sdh_shadow;     // SDH as written by the firmware
	uint16_t m_cyl_shadow;    // task file cylinder
	uint8_t m_sec_shadow;     // task file sector
	bool m_fmt_active;        // last command was a WRITE FORMAT answered here (see hdc_w)
	bool m_fmt_fed;           // ID table complete, track written
	uint16_t m_fmt_bytes;     // ID table bytes fed (2 per sector)
	bool m_wr_active;         // last command was a WRITE SECTOR answered here (see latch_w)
	bool m_wr_done;           // sector written
	uint8_t m_error;          // error register of those commands, 0 on success
	bool m_i8291a_int_active; // FIRQ source (see update_firq)
	bool m_cpufirq;           // mirrors the FIRQ line driven to the 6809
	bool m_cpuirq;            // mirrors the IRQ line (i8291A DREQ)

};

hp9133xv_hdd_device::hp9133xv_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t{mconfig, GPIB_HP9133XV_HDD, tag, owner, clock},
	device_ieee488_interface{mconfig, *this},
	m_cpu{*this, "cpu"},
	m_i8291a{*this, "i8291a"},
	m_hdc{*this, "hdc"},
	m_hdd0{*this, "hdc:0"},
	m_hpib_addr{*this, "ADDRESS"},
	m_model{*this, "MODEL"},
	m_secbuf{},
	m_cpufirq{false},   // read by device_reset
	m_cpuirq{false}
{
}

// HP-IB address as set on the rear panel switch (User's Manual p.8, fig. 2-3); the port reads it
// complemented (sheet 3) and the firmware complements it back ($E2FC). The default is the factory
// setting for the fixed disc.
static INPUT_PORTS_START(hp9133xv_hdd_port)
	PORT_START("ADDRESS")
	PORT_CONFNAME(0x7, 0x0, "HPIB address")
	PORT_CONFSETTING(0, "0")
	PORT_CONFSETTING(1, "1")
	PORT_CONFSETTING(2, "2")
	PORT_CONFSETTING(3, "3")
	PORT_CONFSETTING(4, "4")
	PORT_CONFSETTING(5, "5")
	PORT_CONFSETTING(6, "6")
	PORT_CONFSETTING(7, "7")

	// J3 jumpers C/B/A = bits 5/4/3 of port $0020, installed = 0 (Service Documentation 09134-90032,
	// Section III p. 1-1, Table 1-1; HP 9133/34 V/XV/D/H, 5957-6568, p. 3-2). At boot ($E390) the firmware
	// picks a 15-byte record by these bits: [0-1] last cylinder, [2] heads-1, [3] sectors-1, [8-9] Identify.
	//   $10 -> $E002: cylinders 0-305, 6 heads, 31 sectors, Identify 01 0F (9133XV, 9134XV)
	//   $20 -> $E020: cylinders 0-152, 4 heads, 31 sectors, Identify 01 06 (9133V, on 306 x 2 physical)
	//   $30 -> $E03E: cylinders 0-76, 2 heads, 30 sectors, Identify 00 81 (9133V option 004, per volume)
	//   $00 -> $E011: cylinders 0-305, 4 heads, 31 sectors, Identify 01 0A (9133XV option 010)
	// The last cylinder is the selftest cylinder, kept from read and write commands (p. A-3, A-4).
	PORT_START("MODEL")
	PORT_CONFNAME(0x38, 0x10, "J3 configuration straps")
	PORT_CONFSETTING(0x10, "9133XV / 9134XV, 14.5M")
	PORT_CONFSETTING(0x20, "9133V, 4.8M")
	PORT_CONFSETTING(0x00, "9133XV option 010, 10M")
	PORT_CONFSETTING(0x30, "9133V option 004, 4.6M, 4 volumes")
INPUT_PORTS_END

ioport_constructor hp9133xv_hdd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp9133xv_hdd_port);
}

void hp9133xv_hdd_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_secbuf));
	save_item(NAME(m_bufcnt));
	save_item(NAME(m_wdidx));
	save_item(NAME(m_sdh_shadow));
	save_item(NAME(m_cyl_shadow));
	save_item(NAME(m_fmt_active));
	save_item(NAME(m_fmt_fed));
	save_item(NAME(m_fmt_bytes));
	save_item(NAME(m_wr_active));
	save_item(NAME(m_wr_done));
	save_item(NAME(m_error));
	save_item(NAME(m_sec_shadow));
	save_item(NAME(m_i8291a_int_active));
	save_item(NAME(m_cpufirq));
	save_item(NAME(m_cpuirq));
}

void hp9133xv_hdd_device::device_reset()
{
	m_latch = 0;
	m_bufcnt = 0;
	m_wdidx = 0;
	m_sdh_shadow = 0;
	m_cyl_shadow = 0;
	m_sec_shadow = 0;
	m_i8291a_int_active = false;
	m_fmt_active = false;
	m_fmt_fed = false;
	m_fmt_bytes = 0;
	m_wr_active = false;
	m_wr_done = false;
	m_error = 0;
	// The m6809 core keeps its input lines across reset: release them so that the mirrors, which gate
	// set_input_line on a change, agree with the CPU.
	if (m_cpufirq) {
		m_cpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
		m_cpufirq = false;
	}
	if (m_cpuirq) {
		m_cpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
		m_cpuirq = false;
	}
	// wd1010_device::device_reset() leaves m_status untouched; without DRDY, wd1010_device::write()
	// aborts every command with ERR_AC. DRDY is a drive input, active on a drive at rest.
	m_hdc->drdy_w(1);
	// SC (seek complete) is active on a real drive at rest, and the selftest ($ED6E) polls (status & $50) == $50
	// (RDY+SC) before issuing any command.
	m_hdc->sc_w(1);
}

std::error_condition hp9133xv_hdd_device::hdd_load(device_image_interface &)
{
	// The drive has 256-byte sectors; the sector buffer and the transfers assume them.
	if (m_hdd0->get_info().sectorbytes != 256) {
		osd_printf_error("%s: the Winchester image must have 256-byte sectors\n", tag());
		return image_error::INVALIDLENGTH;
	}
	return std::error_condition();
}

void hp9133xv_hdd_device::ieee488_eoi(int state)  { m_i8291a->eoi_w(state); }
void hp9133xv_hdd_device::ieee488_dav(int state)  { m_i8291a->dav_w(state); }
void hp9133xv_hdd_device::ieee488_nrfd(int state) { m_i8291a->nrfd_w(state); }
void hp9133xv_hdd_device::ieee488_ndac(int state) { m_i8291a->ndac_w(state); }
void hp9133xv_hdd_device::ieee488_ifc(int state)  { m_i8291a->ifc_w(state); }
void hp9133xv_hdd_device::ieee488_srq(int state)  { m_i8291a->srq_w(state); }
void hp9133xv_hdd_device::ieee488_atn(int state)  { m_i8291a->atn_w(state); }
void hp9133xv_hdd_device::ieee488_ren(int state)  { m_i8291a->ren_w(state); }

void hp9133xv_hdd_device::i8291a_eoi_w(int state)  { m_bus->eoi_w(this, state); }
void hp9133xv_hdd_device::i8291a_dav_w(int state)  { LOGGPIB("%s DAV %d\n",  machine().describe_context(), state); m_bus->dav_w(this, state); }
void hp9133xv_hdd_device::i8291a_nrfd_w(int state) { LOGGPIB("%s NRFD %d\n", machine().describe_context(), state); m_bus->nrfd_w(this, state); }
void hp9133xv_hdd_device::i8291a_ndac_w(int state) { LOGGPIB("%s NDAC %d\n", machine().describe_context(), state); m_bus->ndac_w(this, state); }
void hp9133xv_hdd_device::i8291a_srq_w(int state)  { m_bus->srq_w(this, state); }

void hp9133xv_hdd_device::i8291a_int_w(int state)
{
	// INT reaches the 6809 FIRQ over the /HPIBInt wire (sheets 1 and 3); AUXB=$0C makes it active low.
	LOGIRQ("%s 8291A INT pin %d\n", machine().describe_context(), state);
	m_i8291a_int_active = (state == 0);
	update_firq();
}

void hp9133xv_hdd_device::i8291a_dreq_w(int state)
{
	LOGGPIB("%s 8291A DREQ %d\n", machine().describe_context(), state);
	// DREQ paces the HP-IB pump over IRQ, which the firmware leaves masked: SYNC resumes on a masked
	// interrupt (MC6809), once per byte ($F2C5).
	if (m_cpuirq != (state != 0)) {
		m_cpu->set_input_line(M6809_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
		m_cpuirq = state;
	}
}

uint8_t hp9133xv_hdd_device::i8291a_dio_r()          { return m_bus->dio_r(); }
void hp9133xv_hdd_device::i8291a_dio_w(uint8_t data) { m_bus->dio_w(this, data); }

void hp9133xv_hdd_device::latch_w(uint8_t data)
{
	m_latch = data;
	LOGHDDREG("%s latch $00 <= %02X\n", machine().describe_context(), data);

	// bit2 low clears the buffer address counter.
	if (!BIT(data, 2))
		m_bufcnt = 0;

	// HACK: bit7 is BRdy (U18 '174, sheet 4), pulsed by the firmware for 6 us ($F33B). The WD1010 takes
	// it on its rising edge (WD1010-05 p. 3-3, 3-7); wd1010_device samples a level on a 35 us timer, so
	// the level is held here until BDRQ drops (hdc_bdrq_w).
	if (BIT(data, 7))
		m_hdc->brdy_w(1);

	// The same pulse ends a WRITE SECTOR: the buffer goes to the image at the task-file address.
	if (BIT(data, 7) && m_wr_active && !m_wr_done) {
		const uint8_t head = m_sdh_shadow & 0x07;
		const auto *const info = m_hdd0->exists() ? &m_hdd0->get_info() : nullptr;
		// Out of range only on an image with a smaller geometry than the drive (306 cylinders).
		if (!info) {
			m_error = ERROR_AC;
		} else if (m_cyl_shadow < info->cylinders && head < info->heads && m_sec_shadow < info->sectors) {
			const uint32_t lba = (uint32_t(m_cyl_shadow) * info->heads + head) * info->sectors + m_sec_shadow;
			write_image(lba, m_secbuf);
			LOGHDDXFER("%s write sector: cyl %u head %u sec %u -> LBA %u (buffer to CHD)\n",
					machine().describe_context(), m_cyl_shadow, head, m_sec_shadow, lba);
		} else {
			m_error = ERROR_ID;
		}
		m_wr_done = true;
	}

	// The other '174 outputs (sheet 4: PrecompEn, Err LED, WDCRst) are not modelled. bit6 is a plain
	// loopback, written here and read back at $0020 by the self test ($F8DD) -- see addr_r.
}

uint8_t hp9133xv_hdd_device::addr_r()
{
	// bits 0-2: HP-IB address, complemented on its way to the CPU (see the ADDRESS input port above)
	// bits 3-5: J3 personality straps, read once at boot by the ROM (see the MODEL input port above)
	// bit 6   : loopback of the $0000 control latch, exercised by the self test at $F8DD
	return ((~m_hpib_addr->read()) & 0x07) | (m_model->read() & 0x38) | (m_latch & 0x40);
}

uint8_t hp9133xv_hdd_device::hdc_r(offs_t offset)
{
	// offset 0 ($60) is the sector buffer, auto-incrementing; the WD1010's data register is not
	// reachable from the CPU on this board.
	if (offset == 0) {
		uint8_t v = m_secbuf[m_bufcnt & 0x1ff];
		if (!machine().side_effects_disabled())
			m_bufcnt = (m_bufcnt + 1) & 0x3ff;
		return v;
	}
	// SDH reads back what was written (see hdc_w offset 6).
	if (offset == 6)
		return m_sdh_shadow;
	// Error and status of WRITE SECTOR and WRITE FORMAT (see hdc_w): DRQ until done, then RDY|SC, with ERR on
	// failure. They stand until the next command, as ERR does on the WD1010 (WD1010-05 p. 3-10), or an SDH write.
	if (m_wr_active || m_fmt_active) {
		if (offset == 1)
			return m_error;
		if (offset == 7) {
			if (!(m_wr_active ? m_wr_done : m_fmt_fed))
				return STATUS_RDY_SC_DRQ;
			return m_error ? STATUS_RDY_SC_ERR : STATUS_RDY_SC;
		}
	}
	uint8_t v = m_hdc->read(offset);
	if (offset == 7 && !machine().side_effects_disabled())
		LOGHDDREG("%s WD1010 status => %02X\n", machine().describe_context(), v);
	return v;
}

void hp9133xv_hdd_device::hdc_w(offs_t offset, uint8_t data)
{
	// offset 0 ($60) = write into the sector buffer (auto-incrementing).
	if (offset == 0) {
		if (m_fmt_active && !m_fmt_fed && m_hdd0->exists()) {
			const auto &info = m_hdd0->get_info();
			// Two ID table bytes per sector; the track is written once the last one is in.
			if (++m_fmt_bytes >= 2 * info.sectors) {
				const uint8_t fmt_head = m_sdh_shadow & 0x07;
				if (m_cyl_shadow < info.cylinders && fmt_head < info.heads) {
					uint8_t fill[256];
					std::fill(std::begin(fill), std::end(fill), 0xff);
					const uint32_t lba0 = (uint32_t(m_cyl_shadow) * info.heads + fmt_head) * info.sectors;
					for (int s = 0; s < info.sectors; s++)
						write_image(lba0 + s, fill);
					LOGHDDXFER("%s format track: cyl %u head %u -> LBA %u..%u (%d sectors of 0xff)\n",
							machine().describe_context(), m_cyl_shadow, fmt_head, lba0, lba0 + info.sectors - 1, info.sectors);
				} else {
					m_error = ERROR_ID;
				}
				m_fmt_fed = true;
			}
		}
		m_secbuf[m_bufcnt & 0x1ff] = data;
		m_bufcnt = (m_bufcnt + 1) & 0x3ff;
		return;
	}
	LOGHDDREG("%s WD1010 write %d <= %02X\n", machine().describe_context(), offset, data);

	// A command or an SDH write ends the status of a command answered here (see hdc_r).
	if (offset == 6 || offset == 7) {
		m_wr_active = false;
		m_fmt_active = false;
	}

	// SDH: the firmware reads it back ($F6E5) and the self test probes drive 2 ($ED77), expecting status
	// other than $50. The board fits drive 0 only, wd1010_device keeps one RDY/SC status for all drives
	// and does not take the head from SDH.
	if (offset == 6) {
		m_sdh_shadow = data;
		m_hdc->write(offset, data);
		m_hdc->head_w(data & 0x07);
		const bool present = BIT(data, 3, 2) == 0;
		m_hdc->drdy_w(present ? 1 : 0);
		m_hdc->sc_w(present ? 1 : 0);
		return;
	}

	// Task-file copy for the commands answered below; the registers still go to the wd1010 for reads.
	if (offset == 3)
		m_sec_shadow = data;
	else if (offset == 4)
		m_cyl_shadow = (m_cyl_shadow & 0xff00) | data;
	else if (offset == 5)
		m_cyl_shadow = (m_cyl_shadow & 0x00ff) | (uint16_t(data) << 8);

	// HACK: WRITE SECTOR: after RESTORE, with no SEEK, the self test writes the selftest cylinder and
	// polls status seven times for DRQ ($FBEB), giving up with error F6. wd1010_device raises DRQ only
	// after the implied seek (WD1010-05 p. 3-15, 3-16), too late; the board's WD1010AL-00 answers in
	// time, as the self test passes on hardware. DRQ is answered at once and the sector written in
	// latch_w.
	if (offset == 7 && BIT(data, 4, 4) == 0x3) {
		m_wr_active = true;
		// Without an image the command aborts, as wd1010_device does: ERR at once, no DRQ.
		m_error = m_hdd0->exists() ? 0 : ERROR_AC;
		m_wr_done = m_error != 0;
		return;
	}

	// HACK: WRITE FORMAT formats one track from the ID table in the buffer, data fields 0xff (WD1010-05
	// p. 3-17); wd1010_device writes a single sector of 0xe5. The track is written once the table is in.
	if (offset == 7 && BIT(data, 4, 4) == 0x5) {
		m_fmt_active = true;
		m_error = m_hdd0->exists() ? 0 : ERROR_AC;
		m_fmt_fed = m_error != 0;
		m_fmt_bytes = 0;
		return;
	}
	m_hdc->write(offset, data);
}

void hp9133xv_hdd_device::write_image(uint32_t lba, const void *buffer)
{
	// A failed write is the drive's write fault, which aborts the command (WD1010-05 p. 3-15, 3-16).
	if (!m_hdd0->write(lba, buffer))
		m_error = ERROR_AC;
}

// Buffer-chip callbacks: wd1010_device moves the sector through these in one block, bracketed by BCR.
void hp9133xv_hdd_device::hdc_bcr_w(int state)
{
	if (state) {
		m_wdidx = 0;
		// On the real board BCR resets a single buffer counter, shared by the wd1010 and by the CPU
		// side at $60. They are separate here (m_wdidx, m_bufcnt), so both must be reset or the CPU
		// drains the buffer from a stale offset after a successful read.
		m_bufcnt = 0;
		// BRDY is released in hdc_bdrq_w, see latch_w for why it is held as a level.
	}
}

uint8_t hp9133xv_hdd_device::hdc_data_in()
{
	uint8_t v = m_secbuf[m_wdidx & 0x1ff];
	m_wdidx++;
	return v;
}

void hp9133xv_hdd_device::hdc_data_out(uint8_t data)
{
	m_secbuf[m_wdidx & 0x1ff] = data;
	m_wdidx++;
}

void hp9133xv_hdd_device::hdc_intrq_w(int state)
{
	// INTRQ is a bare pin on this board (sheet 5) -- no interrupt to the 6809, trace only.
	LOGIRQ("%s WD1010 INTRQ %d\n", machine().describe_context(), state);
}

void hp9133xv_hdd_device::hdc_bdrq_w(int state)
{
	LOGIRQ("%s WD1010 BDRQ %d\n", machine().describe_context(), state);
	// BDRQ is a bare pin on this board (sheet 5); the firmware polls status $67 instead. Its falling
	// edge only releases the BRDY level held since latch_w.
	if (!state)
		m_hdc->brdy_w(0);
}

// FIRQ has a single source on this board, the 8291A INT: the WD1010's INTRQ and BDRQ pins are bare
// (sheet 5), and the IRQ vector at $FFF8 is blank -- IRQ only wakes the SYNC in the transfer pumps
// (see i8291a_dreq_w).
void hp9133xv_hdd_device::update_firq()
{
	bool firq = m_i8291a_int_active;
	if (firq != m_cpufirq) {
		m_cpu->set_input_line(M6809_FIRQ_LINE, firq ? ASSERT_LINE : CLEAR_LINE);
		m_cpufirq = firq;
	}
}

void hp9133xv_hdd_device::cpu_map(address_map &map)
{
	// U19 '138 decodes A5-A7 below $8000 (sheet 2), A8-A14 ignored: the page repeats over $0000-$7FFF.
	// RAM U38 68B10 (128 bytes, sheet 1) is selected by A7. The latch decodes writes only.
	map(0x0000, 0x0000).mirror(0x7f1f).w(FUNC(hp9133xv_hdd_device::latch_w));                         // board control latch $00
	map(0x0020, 0x0020).mirror(0x7f1f).r(FUNC(hp9133xv_hdd_device::addr_r));                          // HP-IB address port (jumper)
	map(0x0040, 0x0047).mirror(0x7f18).m(m_i8291a, FUNC(i8291a_device::map));                         // i8291A
	map(0x0060, 0x0067).mirror(0x7f18).rw(FUNC(hp9133xv_hdd_device::hdc_r), FUNC(hp9133xv_hdd_device::hdc_w)); // WD1010
	map(0x0080, 0x00ff).mirror(0x7f00).ram();
	// U39 2764 is gated by A15 alone (sheets 1-2) and repeats over $8000-$FFFF.
	map(0x8000, 0x9fff).mirror(0x6000).rom().region("cpu", 0);                                        // ROM 09133-15520
}

ROM_START(hp9133xv_hdd)
	ROM_REGION(0x2000, "cpu", 0)
	// 2764 at U39
	ROM_LOAD("09133-15520.u39", 0x0000, 0x2000, CRC(d02e9a85) SHA1(a8fe810faeabef2f0b1d001bc2e909c696f66981))
ROM_END

const tiny_rom_entry *hp9133xv_hdd_device::device_rom_region() const
{
	return ROM_NAME(hp9133xv_hdd);
}

void hp9133xv_hdd_device::device_add_mconfig(machine_config &config)
{
	MC6809(config, m_cpu, 8_MHz_XTAL);   // 68B09 clocked by crystal Y1 (Duell schematics, sheet 1)
	m_cpu->set_addrmap(AS_PROGRAM, &hp9133xv_hdd_device::cpu_map);

	WD1010(config, m_hdc, 5'000'000);   // WCLK, 5 Mbit/s ST506 rate (Duell schematics, sheets 7-9)
	m_hdc->out_intrq_callback().set(FUNC(hp9133xv_hdd_device::hdc_intrq_w));
	m_hdc->out_bdrq_callback().set(FUNC(hp9133xv_hdd_device::hdc_bdrq_w));
	// in/out_data are wired to m_secbuf, the same 512-byte buffer the 6809 reaches through $60.
	m_hdc->out_bcr_callback().set(FUNC(hp9133xv_hdd_device::hdc_bcr_w));
	m_hdc->in_data_callback().set(FUNC(hp9133xv_hdd_device::hdc_data_in));
	m_hdc->out_data_callback().set(FUNC(hp9133xv_hdd_device::hdc_data_out));

	// wd1010_device looks its drives up as subdevices "0".."3" of its own tag, hence "hdc:0".
	HARDDISK(config, m_hdd0, 0).set_device_load(FUNC(hp9133xv_hdd_device::hdd_load));

	// E clocks the 8291A on this board (sheet 3 of 09133-66508), hence the crystal over four.
	I8291A(config, m_i8291a, 8_MHz_XTAL / 4);
	m_i8291a->eoi_write().set(FUNC(hp9133xv_hdd_device::i8291a_eoi_w));
	m_i8291a->dav_write().set(FUNC(hp9133xv_hdd_device::i8291a_dav_w));
	m_i8291a->nrfd_write().set(FUNC(hp9133xv_hdd_device::i8291a_nrfd_w));
	m_i8291a->ndac_write().set(FUNC(hp9133xv_hdd_device::i8291a_ndac_w));
	m_i8291a->srq_write().set(FUNC(hp9133xv_hdd_device::i8291a_srq_w));
	m_i8291a->dio_write().set(FUNC(hp9133xv_hdd_device::i8291a_dio_w));
	m_i8291a->dio_read().set(FUNC(hp9133xv_hdd_device::i8291a_dio_r));
	m_i8291a->int_write().set(FUNC(hp9133xv_hdd_device::i8291a_int_w));
	m_i8291a->dreq_write().set(FUNC(hp9133xv_hdd_device::i8291a_dreq_w));
}


//====================================================================
// The drive unit: a Winchester board and, except on the 9134XV, a floppy board. One type per model or
// option, which sets the J3 straps of the Winchester board.
//====================================================================

static DEVICE_INPUT_DEFAULTS_START(j3_9133xv)
	DEVICE_INPUT_DEFAULTS("MODEL", 0x38, 0x10)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(j3_9133v)
	DEVICE_INPUT_DEFAULTS("MODEL", 0x38, 0x20)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(j3_9133v004)
	DEVICE_INPUT_DEFAULTS("MODEL", 0x38, 0x30)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(j3_9133xv010)
	DEVICE_INPUT_DEFAULTS("MODEL", 0x38, 0x00)
DEVICE_INPUT_DEFAULTS_END

class hp9133xv_unit_device_base : public device_t
{
protected:
	hp9133xv_unit_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const input_device_default *j3, bool floppy) :
		device_t(mconfig, type, tag, owner, clock),
		m_j3(j3),
		m_floppy(floppy)
	{
	}

	virtual void device_start() override ATTR_COLD { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	const input_device_default *const m_j3;
	const bool m_floppy;
};

class hp9133xv_device : public hp9133xv_unit_device_base
{
public:
	hp9133xv_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		hp9133xv_unit_device_base(mconfig, GPIB_HP9133XV, tag, owner, clock, DEVICE_INPUT_DEFAULTS_NAME(j3_9133xv), true) { }
};

class hp9133v_device : public hp9133xv_unit_device_base
{
public:
	hp9133v_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		hp9133xv_unit_device_base(mconfig, GPIB_HP9133V, tag, owner, clock, DEVICE_INPUT_DEFAULTS_NAME(j3_9133v), true) { }
};

class hp9133v004_device : public hp9133xv_unit_device_base
{
public:
	hp9133v004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		hp9133xv_unit_device_base(mconfig, GPIB_HP9133V_004, tag, owner, clock, DEVICE_INPUT_DEFAULTS_NAME(j3_9133v004), true) { }
};

class hp9134xv_device : public hp9133xv_unit_device_base
{
public:
	hp9134xv_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		hp9133xv_unit_device_base(mconfig, GPIB_HP9134XV, tag, owner, clock, DEVICE_INPUT_DEFAULTS_NAME(j3_9133xv), false) { }
};

class hp9133xv010_device : public hp9133xv_unit_device_base
{
public:
	hp9133xv010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		hp9133xv_unit_device_base(mconfig, GPIB_HP9133XV_010, tag, owner, clock, DEVICE_INPUT_DEFAULTS_NAME(j3_9133xv010), true) { }
};

void hp9133xv_hdd_board(device_slot_interface &device)
{
	device.option_add("hdd", GPIB_HP9133XV_HDD);
}

void hp9133xv_fdd_board(device_slot_interface &device)
{
	device.option_add("fdd", GPIB_HP9133XV_FDD);
}

void hp9133xv_unit_device_base::device_add_mconfig(machine_config &config)
{
	// each board keeps its own 8291A and HP-IB address; both attach to the host's bus
	ieee488_slot_device &hdd = IEEE488_SLOT(config, "hdd", 0, hp9133xv_hdd_board, "hdd");
	hdd.set_fixed(true);
	hdd.set_option_device_input_defaults("hdd", m_j3);
	if (m_floppy)
		IEEE488_SLOT(config, "fdd", 0, hp9133xv_fdd_board, "fdd").set_fixed(true);

	// the next device on the same bus, as a cable stacked on the unit's connector
	IEEE488_SLOT(config, "chain", 0, hp_ieee488_devices, nullptr);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(GPIB_HP9133XV_FDD, device_ieee488_interface, hp9133xv_fdd_device, "hp9133xv_fdd", "HP9133XV disc drive (floppy board)")
DEFINE_DEVICE_TYPE_PRIVATE(GPIB_HP9133XV_HDD, device_ieee488_interface, hp9133xv_hdd_device, "hp9133xv_hdd", "HP9133XV disc drive (Winchester board)")
DEFINE_DEVICE_TYPE_PRIVATE(GPIB_HP9133XV,     device_t, hp9133xv_device,   "hp9133xv",   "HP9133XV disc drive")
DEFINE_DEVICE_TYPE_PRIVATE(GPIB_HP9133V,      device_t, hp9133v_device,    "hp9133v",    "HP9133V disc drive")
DEFINE_DEVICE_TYPE_PRIVATE(GPIB_HP9133V_004,  device_t, hp9133v004_device, "hp9133v004", "HP9133V option 004 disc drive (4 volumes)")
DEFINE_DEVICE_TYPE_PRIVATE(GPIB_HP9134XV,     device_t, hp9134xv_device,   "hp9134xv",   "HP9134XV disc drive")
DEFINE_DEVICE_TYPE_PRIVATE(GPIB_HP9133XV_010, device_t, hp9133xv010_device, "hp9133xv010", "HP9133XV option 010 disc drive (10M)")
