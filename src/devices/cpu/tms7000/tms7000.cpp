// license:BSD-3-Clause
// copyright-holders:hap, Tim Lindner
/*****************************************************************************

  Texas Instruments TMS7000

  TODO:
  - dump CROM and emulate cpu at microinstruction level
  - memory modes with IOCNT0, currently always running in faked full expansion mode
  - timer event counter mode (timer control register, bit 6)
  - TMS70x1/2 serial port and timer 3
  - TMS70C46 DOCK-BUS comms with external pins
  - TMS70C46 external memory mode is via "E" bus instead of configuring IOCNT0
  - TMS70C46 clock divider
  - TMS70C46 INT3 on keypress
  - when they're needed, add TMS70Cx2, TMS7742, TMS77C82, SE70xxx

*****************************************************************************/

#include "tms7000.h"

// TMS7000 is the most basic one, 128 bytes internal RAM and no internal ROM.
// TMS7020 and TMS7040 are same, but with 2KB and 4KB internal ROM respectively.
const device_type TMS7000 = &device_creator<tms7000_device>;
const device_type TMS7020 = &device_creator<tms7020_device>;
const device_type TMS7040 = &device_creator<tms7040_device>;

// Exelvision (spinoff of TI) TMS7020 added one custom opcode.
const device_type TMS7020_EXL = &device_creator<tms7020_exl_device>;

// CMOS devices biggest difference in a 'real world' setting is that the power
// requirements are much lower. This obviously has no use in software emulation.
const device_type TMS70C00 = &device_creator<tms70c00_device>;
const device_type TMS70C20 = &device_creator<tms70c20_device>;
const device_type TMS70C40 = &device_creator<tms70c40_device>;

// TMS70x1 features more peripheral I/O, the main addition being a serial port.
// TMS70x2 is the same, just with twice more RAM (256 bytes)
const device_type TMS7001 = &device_creator<tms7001_device>;
const device_type TMS7041 = &device_creator<tms7041_device>;
const device_type TMS7002 = &device_creator<tms7002_device>;
const device_type TMS7042 = &device_creator<tms7042_device>;

// TMS70C46 is literally a shell around a TMS70C40, with support for external
// memory bus, auto external clock divider on slow memory, and wake-up on keypress.
const device_type TMS70C46 = &device_creator<tms70c46_device>;

// TMS70Cx2 is an update to TMS70x2 with some extra features. Due to some changes
// in peripheral file I/O, it is not backward compatible to TMS70x2.


// flag helpers
#define SR_C        0x80 /* Carry */
#define SR_N        0x40 /* Negative */
#define SR_Z        0x20 /* Zero */
#define SR_I        0x10 /* Interrupt */

#define GET_C()     (m_sr >> 7 & 1)
#define SET_C(x)    m_sr = (m_sr & 0x7f) | ((x) >> 1 & 0x80)
#define SET_NZ(x)   m_sr = (m_sr & 0x9f) | ((x) >> 1 & 0x40) | (((x) & 0xff) ? 0 : 0x20)
#define SET_CNZ(x)  m_sr = (m_sr & 0x1f) | ((x) >> 1 & 0xc0) | (((x) & 0xff) ? 0 : 0x20)


// internal memory maps
static ADDRESS_MAP_START(tms7000_io, AS_IO, 8, tms7000_device)
	AM_RANGE(TMS7000_PORTB, TMS7000_PORTB) AM_READNOP
ADDRESS_MAP_END

static ADDRESS_MAP_START(tms7000_mem, AS_PROGRAM, 8, tms7000_device )
	AM_RANGE(0x0000, 0x007f) AM_RAM // 128 bytes internal RAM
	AM_RANGE(0x0080, 0x00ff) AM_READWRITE(tms7000_unmapped_rf_r, tms7000_unmapped_rf_w)
	AM_RANGE(0x0104, 0x0105) AM_WRITENOP // no port A write or ddr
	AM_RANGE(0x0100, 0x010b) AM_READWRITE(tms7000_pf_r, tms7000_pf_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(tms7001_mem, AS_PROGRAM, 8, tms7000_device )
	AM_RANGE(0x0000, 0x007f) AM_RAM // 128 bytes internal RAM
	AM_RANGE(0x0080, 0x00ff) AM_READWRITE(tms7000_unmapped_rf_r, tms7000_unmapped_rf_w)
	AM_RANGE(0x0100, 0x010b) AM_READWRITE(tms7000_pf_r, tms7000_pf_w)
	AM_RANGE(0x0110, 0x0117) AM_READWRITE(tms7002_pf_r, tms7002_pf_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(tms7002_mem, AS_PROGRAM, 8, tms7000_device )
	AM_RANGE(0x0000, 0x00ff) AM_RAM // 256 bytes internal RAM
	AM_RANGE(0x0100, 0x010b) AM_READWRITE(tms7000_pf_r, tms7000_pf_w)
	AM_RANGE(0x0110, 0x0117) AM_READWRITE(tms7002_pf_r, tms7002_pf_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(tms7020_mem, AS_PROGRAM, 8, tms7000_device )
	AM_RANGE(0xf000, 0xffff) AM_ROM // 2kB internal ROM
	AM_IMPORT_FROM( tms7000_mem )
ADDRESS_MAP_END

static ADDRESS_MAP_START(tms7040_mem, AS_PROGRAM, 8, tms7000_device )
	AM_RANGE(0xf000, 0xffff) AM_ROM // 4kB internal ROM
	AM_IMPORT_FROM( tms7000_mem )
ADDRESS_MAP_END

static ADDRESS_MAP_START(tms7041_mem, AS_PROGRAM, 8, tms7000_device )
	AM_RANGE(0xf000, 0xffff) AM_ROM
	AM_IMPORT_FROM( tms7001_mem )
ADDRESS_MAP_END

static ADDRESS_MAP_START(tms7042_mem, AS_PROGRAM, 8, tms7000_device )
	AM_RANGE(0xf000, 0xffff) AM_ROM
	AM_IMPORT_FROM( tms7002_mem )
ADDRESS_MAP_END

static ADDRESS_MAP_START(tms70c46_mem, AS_PROGRAM, 8, tms70c46_device )
	AM_RANGE(0x010c, 0x010c) AM_READWRITE(e_bus_data_r, e_bus_data_w)
	AM_RANGE(0x010d, 0x010d) AM_NOP // ? always writes $FF before checking keyboard... maybe INT3 ack?
	AM_RANGE(0x010e, 0x010e) AM_READWRITE(dockbus_data_r, dockbus_data_w)
	AM_RANGE(0x010f, 0x010f) AM_READWRITE(dockbus_status_r, dockbus_status_w)
	AM_RANGE(0x0118, 0x0118) AM_READWRITE(control_r, control_w)
	AM_IMPORT_FROM( tms7040_mem )
ADDRESS_MAP_END


// device definitions
tms7000_device::tms7000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, TMS7000, "TMS7000", tag, owner, clock, "tms7000", __FILE__),
	m_program_config("program", ENDIANNESS_BIG, 8, 16, 0, ADDRESS_MAP_NAME(tms7000_mem)),
	m_io_config("io", ENDIANNESS_BIG, 8, 8, 0, ADDRESS_MAP_NAME(tms7000_io)),
	m_info_flags(0)
{
}

tms7000_device::tms7000_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, address_map_constructor internal, UINT32 info_flags, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_program_config("program", ENDIANNESS_BIG, 8, 16, 0, internal),
	m_io_config("io", ENDIANNESS_BIG, 8, 8, 0, ADDRESS_MAP_NAME(tms7000_io)),
	m_info_flags(info_flags)
{
}

tms7020_device::tms7020_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms7000_device(mconfig, TMS7020, "TMS7020", tag, owner, clock, ADDRESS_MAP_NAME(tms7020_mem), 0, "tms7020", __FILE__)
{
}

tms7020_exl_device::tms7020_exl_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms7000_device(mconfig, TMS7020_EXL, "TMS7020 (Exelvision)", tag, owner, clock, ADDRESS_MAP_NAME(tms7020_mem), 0, "tms7020_exl", __FILE__)
{
}

tms7040_device::tms7040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms7000_device(mconfig, TMS7040, "TMS7040", tag, owner, clock, ADDRESS_MAP_NAME(tms7040_mem), 0, "tms7040", __FILE__)
{
}

tms70c00_device::tms70c00_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms7000_device(mconfig, TMS70C00, "TMS70C00", tag, owner, clock, ADDRESS_MAP_NAME(tms7000_mem), TMS7000_CHIP_IS_CMOS, "tms70c00", __FILE__)
{
}

tms70c20_device::tms70c20_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms7000_device(mconfig, TMS70C20, "TMS70C20", tag, owner, clock, ADDRESS_MAP_NAME(tms7020_mem), TMS7000_CHIP_IS_CMOS, "tms70c20", __FILE__)
{
}

tms70c40_device::tms70c40_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms7000_device(mconfig, TMS70C40, "TMS70C40", tag, owner, clock, ADDRESS_MAP_NAME(tms7040_mem), TMS7000_CHIP_IS_CMOS, "tms70c40", __FILE__)
{
}

tms7001_device::tms7001_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms7000_device(mconfig, TMS7001, "TMS7001", tag, owner, clock, ADDRESS_MAP_NAME(tms7001_mem), TMS7000_CHIP_FAMILY_70X2, "tms7001", __FILE__)
{
}

tms7041_device::tms7041_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms7000_device(mconfig, TMS7041, "TMS7041", tag, owner, clock, ADDRESS_MAP_NAME(tms7041_mem), TMS7000_CHIP_FAMILY_70X2, "tms7041", __FILE__)
{
}

tms7002_device::tms7002_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms7000_device(mconfig, TMS7002, "TMS7002", tag, owner, clock, ADDRESS_MAP_NAME(tms7002_mem), TMS7000_CHIP_FAMILY_70X2, "tms7002", __FILE__)
{
}

tms7042_device::tms7042_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms7000_device(mconfig, TMS7042, "TMS7042", tag, owner, clock, ADDRESS_MAP_NAME(tms7042_mem), TMS7000_CHIP_FAMILY_70X2, "tms7042", __FILE__)
{
}

tms70c46_device::tms70c46_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms7000_device(mconfig, TMS70C46, "TMS70C46", tag, owner, clock, ADDRESS_MAP_NAME(tms70c46_mem), TMS7000_CHIP_IS_CMOS, "tms70c46", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms7000_device::device_start()
{
	// init/zerofill
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	m_icountptr = &m_icount;

	m_irq_state[TMS7000_INT1_LINE] = false;
	m_irq_state[TMS7000_INT3_LINE] = false;

	m_idle_state = false;
	m_idle_halt = false;
	m_pc = 0;
	m_sp = 0;
	m_sr = 0;
	m_op = 0;

	memset(m_io_control, 0, 3);

	memset(m_port_latch, 0, 4);
	memset(m_port_ddr, 0, 4);
	m_port_ddr[1] = 0xff; // !

	for (int tmr = 0; tmr < 2; tmr++)
	{
		m_timer_handle[tmr] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(tms7000_device::simple_timer_cb), this));
		m_timer_handle[tmr]->adjust(attotime::never, tmr);

		m_timer_data[tmr] = 0;
		m_timer_control[tmr] = 0;
		m_timer_decrementer[tmr] = 0;
		m_timer_prescaler[tmr] = 0;
		m_timer_capture_latch[tmr] = 0;
	}

	// register for savestates
	save_item(NAME(m_irq_state));
	save_item(NAME(m_idle_state));
	save_item(NAME(m_pc));
	save_item(NAME(m_sp));
	save_item(NAME(m_sr));
	save_item(NAME(m_op));

	save_item(NAME(m_io_control));
	save_item(NAME(m_port_latch));
	save_item(NAME(m_port_ddr));
	save_item(NAME(m_timer_data));
	save_item(NAME(m_timer_control));
	save_item(NAME(m_timer_decrementer));
	save_item(NAME(m_timer_prescaler));
	save_item(NAME(m_timer_capture_latch));

	// register for debugger
	state_add(TMS7000_PC, "PC", m_pc).formatstr("%02X");
	state_add(TMS7000_SP, "S", m_sp).formatstr("%02X");
	state_add(TMS7000_ST, "ST", m_sr).formatstr("%02X");

	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("%02X").noshow();
	state_add(STATE_GENSP, "GENSP", m_sp).formatstr("%02X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_sr).formatstr("%8s").noshow();
}

void tms7000_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c%c%c",
				m_sr & 0x80 ? 'C':'c',
				m_sr & 0x40 ? 'N':'n',
				m_sr & 0x20 ? 'Z':'z',
				m_sr & 0x10 ? 'I':'i',
				m_sr & 0x08 ? '?':'.',
				m_sr & 0x04 ? '?':'.',
				m_sr & 0x02 ? '?':'.',
				m_sr & 0x01 ? '?':'.'
			);
			break;

		default: break;
	}
}

offs_t tms7000_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms7000 );
	return CPU_DISASSEMBLE_NAME(tms7000)(this, buffer, pc, oprom, opram, options);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tms7000_device::device_reset()
{
	if (m_idle_state)
	{
		m_pc++;
		m_idle_state = false;
	}

	// while _RESET is asserted:
	// clear ports
	write_p(0x04, 0xff); // port a
	write_p(0x06, 0xff); // port b

	write_p(0x05, 0x00); // ddr a
	write_p(0x09, 0x00); // ddr c
	write_p(0x0b, 0x00); // ddr d

	if (!chip_is_cmos())
	{
		write_p(0x08, 0xff); // port c
		write_p(0x0a, 0xff); // port d
	}

	// when _RESET goes inactive (0 to 1)
	m_sr = 0;

	write_p(0x00, 0x00); // IOCNT0
	if (chip_is_family_70x2())
		write_p(0x10, 0x00); // IOCNT1

	m_sp = 0xff;
	m_op = 0xff;
	execute_one(m_op);
	m_icount -= 3; // 17 total
}


//-------------------------------------------------
//  interrupts
//-------------------------------------------------

void tms7000_device::execute_set_input(int extline, int state)
{
	if (extline != TMS7000_INT1_LINE && extline != TMS7000_INT3_LINE)
		return;

	bool irqstate = (state == CLEAR_LINE) ? false : true;

	// reverse polarity (TMS70cx2-only)
	if (m_io_control[2] & (0x01 << (4 * extline)))
		irqstate = !irqstate;

	if (m_irq_state[extline] != irqstate)
	{
		m_irq_state[extline] = irqstate;

		// set/clear internal irq flag
		flag_ext_interrupt(extline);

		if (m_irq_state[extline])
		{
			// latch timer 1 on INT3
			if (extline == TMS7000_INT3_LINE)
				m_timer_capture_latch[0] = m_timer_decrementer[0];

			// on TMS70cx2, latch timer 2 on INT1
			if (extline == TMS7000_INT1_LINE && chip_is_family_70cx2())
				m_timer_capture_latch[1] = m_timer_decrementer[1];

			// clear external if it's edge-triggered (TMS70cx2-only)
			if (m_io_control[2] & (0x02 << (4 * extline)))
				m_irq_state[extline] = false;

			check_interrupts();
		}
	}
}

void tms7000_device::flag_ext_interrupt(int extline)
{
	if (extline != TMS7000_INT1_LINE && extline != TMS7000_INT3_LINE)
		return;

	// set/clear for pending external interrupt
	if (m_irq_state[extline])
		m_io_control[0] |= (0x02 << (4 * extline));
	else
		m_io_control[0] &= ~(0x02 << (4 * extline));
}

void tms7000_device::check_interrupts()
{
	// global interrupt bit
	if (!(m_sr & SR_I))
		return;

	// check for and handle interrupt
	for (int irqline = 0; irqline < 5; irqline++)
	{
		// INT 1,2,3 are in IOCNT0 d0-d5
		// INT 4,5 are in IOCNT1 d0-d3
		int shift = (irqline > 2) ? irqline * 2 - 6 : irqline * 2;
		if ((m_io_control[irqline > 2] >> shift & 3) == 3)
		{
			// ack
			m_io_control[irqline > 2] &= ~(0x02 << shift);
			if (irqline == 0 || irqline == 2)
				flag_ext_interrupt(irqline / 2);

			do_interrupt(irqline);
			return;
		}
	}
}

void tms7000_device::do_interrupt(int irqline)
{
	if (m_idle_state)
	{
		m_icount -= 17;
		m_pc++;
		m_idle_state = false;
	}
	else
		m_icount -= 19;

	push8(m_sr);
	push16(m_pc);
	m_sr = 0;
	m_pc = read_mem16(0xfffc - irqline * 2);

	standard_irq_callback(irqline);
}


//-------------------------------------------------
//  timers
//-------------------------------------------------

void tms7000_device::timer_run(int tmr)
{
	m_timer_prescaler[tmr] = m_timer_control[tmr] & 0x1f;

	// run automatic timer if source is internal
	if ((m_timer_control[tmr] & 0xe0) == 0x80)
	{
		attotime period = attotime::from_hz(clock()) * 8 * (m_timer_prescaler[tmr] + 1); // fOSC/16 - fOSC is freq _before_ internal clockdivider
		m_timer_handle[tmr]->adjust(period, tmr);
	}
}

void tms7000_device::timer_reload(int tmr)
{
	// stop possible running timer
	m_timer_handle[tmr]->adjust(attotime::never, tmr);

	if (m_timer_control[tmr] & 0x80)
	{
		m_timer_decrementer[tmr] = m_timer_data[tmr];
		timer_run(tmr);
	}
}

void tms7000_device::timer_tick_pre(int tmr)
{
	// timer prescaler underflow
	if (--m_timer_prescaler[tmr] < 0)
	{
		m_timer_prescaler[tmr] = m_timer_control[tmr] & 0x1f;
		timer_tick_low(tmr);
	}
}

void tms7000_device::timer_tick_low(int tmr)
{
	// timer decrementer underflow
	if (--m_timer_decrementer[tmr] < 0)
	{
		timer_reload(tmr);

		// set INT2/INT5
		m_io_control[tmr] |= 0x08;

		// cascaded timer
		if (tmr == 0 && (m_timer_control[1] & 0xa0) == 0xa0)
			timer_tick_pre(tmr + 1);
	}
}

TIMER_CALLBACK_MEMBER(tms7000_device::simple_timer_cb)
{
	int tmr = param;

	// tick and restart timer
	timer_tick_low(tmr);
	timer_run(tmr);
}


//-------------------------------------------------
//  peripheral file - read/write internal ports
//  note: TMS7000 family is from $00 to $0b, TMS7002 family adds $10 to $17
//-------------------------------------------------

READ8_MEMBER(tms7000_device::tms7000_pf_r)
{
	switch (offset)
	{
		// i/o control
		case 0x00: case 0x10:
			return m_io_control[offset >> 4];

		// timer 1/2 data
		case 0x02: case 0x12:
			// current decrementer value
			return m_timer_decrementer[offset >> 4];

		// timer 1 control
		case 0x03:
			// timer capture (latched by INT3)
			return m_timer_capture_latch[0];

		// port data
		case 0x04: case 0x06: case 0x08: case 0x0a:
		{
			// note: port B is write-only, reading it returns the output value as if ddr is 0xff
			int port = offset / 2 - 2;
			if (!space.debugger_access())
				return (m_io->read_byte(port) & ~m_port_ddr[port]) | (m_port_latch[port] & m_port_ddr[port]);
			break;
		}

		// port direction (note: TMS7000 doesn't support it for port A)
		case 0x05: case 0x09: case 0x0b:
			return m_port_ddr[offset / 2 - 2];

		default:
			if (!space.debugger_access())
				logerror("'%s' (%04X): tms7000_pf_r @ $%04x\n", tag(), m_pc, offset);
			break;
	}

	return 0;
}

WRITE8_MEMBER(tms7000_device::tms7000_pf_w)
{
	switch (offset)
	{
		// i/o control (IOCNT0)
		case 0x00:
			// d0,d2,d4: INT1,2,3 enable
			// d1,d3,d5: INT1,2,3 flag (write 1 to clear flag)
			// d6-d7: memory mode (currently not implemented)
			m_io_control[0] = (m_io_control[0] & (~data & 0x2a)) | (data & 0xd5);

			// possibly need to reactivate flags
			if (data & 0x02)
				flag_ext_interrupt(TMS7000_INT1_LINE);
			if (data & 0x20)
				flag_ext_interrupt(TMS7000_INT3_LINE);

			check_interrupts();
			break;

		// i/o control (IOCNT1)
		case 0x10:
			// d0,d2: INT4,5 enable
			// d1,d3: INT4,5 flag (write 1 to clear flag)
			m_io_control[1] = (m_io_control[1] & (~data & 0x0a)) | (data & 0x05);
			check_interrupts();
			break;

		// timer 1/2 data
		case 0x02: case 0x12:
			// decrementer reload value
			m_timer_data[offset >> 4] = data;
			break;

		// timer 1/2 control
		case 0x03:
			// d5: t1: cmos low-power mode when IDLE opcode is used (not emulated)
			// 0(normal), or 1(halt) - indicating it can only wake up with RESET or external interrupt
			if (chip_is_cmos())
			{
				m_idle_halt = (data & 0x20) ? true : false;
				if (m_idle_halt)
					logerror("%s: CMOS low-power halt mode enabled\n", tag());
			}
			data &= ~0x20;
		case 0x13:
			// d0-d4: prescaler reload value
			// d5: t2: cascade from t1
			// d6: source (internal/external)
			// d7: stop/start timer
			m_timer_control[offset >> 4] = data;
			timer_reload(offset >> 4);

			// on cmos chip, clear INT2/INT5 as well
			if (~data & 0x80 && chip_is_cmos())
				m_io_control[offset >> 4] &= ~0x08;

			break;

		// port data (note: TMS7000 doesn't support it for port A)
		case 0x04: case 0x06: case 0x08: case 0x0a:
		{
			// note: in memory expansion modes, some port output pins are used for memory strobes.
			// this is currently ignored, since port writes will always be visible externally on peripheral expansion anyway.
			int port = offset / 2 - 2;
			m_io->write_byte(port, data & m_port_ddr[port]);
			m_port_latch[port] = data;
			break;
		}

		// port direction (note: TMS7000 doesn't support it for port A)
		case 0x05: case 0x09: case 0x0b:
			// note: changing port direction does not change(refresh) the output pins
			m_port_ddr[offset / 2 - 2] = data;
			break;

		default:
			logerror("'%s' (%04X): tms7000_pf_w @ $%04x = $%02x\n", tag(), m_pc, offset, data);
			break;
	}
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

#include "tms70op.inc"

void tms7000_device::execute_run()
{
	check_interrupts();

	do
	{
		debugger_instruction_hook(this, m_pc);

		m_op = m_direct->read_byte(m_pc++);
		execute_one(m_op);
	} while (m_icount > 0);
}

void tms7000_device::execute_one(UINT8 op)
{
	switch (op)
	{
		case 0x00: nop(); break;
		case 0x01: idle(); break;
		case 0x05: eint(); break;
		case 0x06: dint(); break;
		case 0x07: setc(); break;
		case 0x08: pop_st(); break;
		case 0x09: stsp(); break;
		case 0x0a: rets(); break;
		case 0x0b: reti(); break;
		case 0x0d: ldsp(); break;
		case 0x0e: push_st(); break;

		case 0x12: am_r2a(&tms7000_device::op_mov); break;
		case 0x13: am_r2a(&tms7000_device::op_and); break;
		case 0x14: am_r2a(&tms7000_device::op_or); break;
		case 0x15: am_r2a(&tms7000_device::op_xor); break;
		case 0x16: am_r2a(&tms7000_device::op_btjo); break;
		case 0x17: am_r2a(&tms7000_device::op_btjz); break;
		case 0x18: am_r2a(&tms7000_device::op_add); break;
		case 0x19: am_r2a(&tms7000_device::op_adc); break;
		case 0x1a: am_r2a(&tms7000_device::op_sub); break;
		case 0x1b: am_r2a(&tms7000_device::op_sbb); break;
		case 0x1c: am_r2a(&tms7000_device::op_mpy); break;
		case 0x1d: am_r2a(&tms7000_device::op_cmp); break;
		case 0x1e: am_r2a(&tms7000_device::op_dac); break;
		case 0x1f: am_r2a(&tms7000_device::op_dsb); break;

		case 0x22: am_i2a(&tms7000_device::op_mov); break;
		case 0x23: am_i2a(&tms7000_device::op_and); break;
		case 0x24: am_i2a(&tms7000_device::op_or); break;
		case 0x25: am_i2a(&tms7000_device::op_xor); break;
		case 0x26: am_i2a(&tms7000_device::op_btjo); break;
		case 0x27: am_i2a(&tms7000_device::op_btjz); break;
		case 0x28: am_i2a(&tms7000_device::op_add); break;
		case 0x29: am_i2a(&tms7000_device::op_adc); break;
		case 0x2a: am_i2a(&tms7000_device::op_sub); break;
		case 0x2b: am_i2a(&tms7000_device::op_sbb); break;
		case 0x2c: am_i2a(&tms7000_device::op_mpy); break;
		case 0x2d: am_i2a(&tms7000_device::op_cmp); break;
		case 0x2e: am_i2a(&tms7000_device::op_dac); break;
		case 0x2f: am_i2a(&tms7000_device::op_dsb); break;

		case 0x32: am_r2b(&tms7000_device::op_mov); break;
		case 0x33: am_r2b(&tms7000_device::op_and); break;
		case 0x34: am_r2b(&tms7000_device::op_or); break;
		case 0x35: am_r2b(&tms7000_device::op_xor); break;
		case 0x36: am_r2b(&tms7000_device::op_btjo); break;
		case 0x37: am_r2b(&tms7000_device::op_btjz); break;
		case 0x38: am_r2b(&tms7000_device::op_add); break;
		case 0x39: am_r2b(&tms7000_device::op_adc); break;
		case 0x3a: am_r2b(&tms7000_device::op_sub); break;
		case 0x3b: am_r2b(&tms7000_device::op_sbb); break;
		case 0x3c: am_r2b(&tms7000_device::op_mpy); break;
		case 0x3d: am_r2b(&tms7000_device::op_cmp); break;
		case 0x3e: am_r2b(&tms7000_device::op_dac); break;
		case 0x3f: am_r2b(&tms7000_device::op_dsb); break;

		case 0x42: am_r2r(&tms7000_device::op_mov); break;
		case 0x43: am_r2r(&tms7000_device::op_and); break;
		case 0x44: am_r2r(&tms7000_device::op_or); break;
		case 0x45: am_r2r(&tms7000_device::op_xor); break;
		case 0x46: am_r2r(&tms7000_device::op_btjo); break;
		case 0x47: am_r2r(&tms7000_device::op_btjz); break;
		case 0x48: am_r2r(&tms7000_device::op_add); break;
		case 0x49: am_r2r(&tms7000_device::op_adc); break;
		case 0x4a: am_r2r(&tms7000_device::op_sub); break;
		case 0x4b: am_r2r(&tms7000_device::op_sbb); break;
		case 0x4c: am_r2r(&tms7000_device::op_mpy); break;
		case 0x4d: am_r2r(&tms7000_device::op_cmp); break;
		case 0x4e: am_r2r(&tms7000_device::op_dac); break;
		case 0x4f: am_r2r(&tms7000_device::op_dsb); break;

		case 0x52: am_i2b(&tms7000_device::op_mov); break;
		case 0x53: am_i2b(&tms7000_device::op_and); break;
		case 0x54: am_i2b(&tms7000_device::op_or); break;
		case 0x55: am_i2b(&tms7000_device::op_xor); break;
		case 0x56: am_i2b(&tms7000_device::op_btjo); break;
		case 0x57: am_i2b(&tms7000_device::op_btjz); break;
		case 0x58: am_i2b(&tms7000_device::op_add); break;
		case 0x59: am_i2b(&tms7000_device::op_adc); break;
		case 0x5a: am_i2b(&tms7000_device::op_sub); break;
		case 0x5b: am_i2b(&tms7000_device::op_sbb); break;
		case 0x5c: am_i2b(&tms7000_device::op_mpy); break;
		case 0x5d: am_i2b(&tms7000_device::op_cmp); break;
		case 0x5e: am_i2b(&tms7000_device::op_dac); break;
		case 0x5f: am_i2b(&tms7000_device::op_dsb); break;

		case 0x62: am_b2a(&tms7000_device::op_mov); break;
		case 0x63: am_b2a(&tms7000_device::op_and); break;
		case 0x64: am_b2a(&tms7000_device::op_or); break;
		case 0x65: am_b2a(&tms7000_device::op_xor); break;
		case 0x66: am_b2a(&tms7000_device::op_btjo); break;
		case 0x67: am_b2a(&tms7000_device::op_btjz); break;
		case 0x68: am_b2a(&tms7000_device::op_add); break;
		case 0x69: am_b2a(&tms7000_device::op_adc); break;
		case 0x6a: am_b2a(&tms7000_device::op_sub); break;
		case 0x6b: am_b2a(&tms7000_device::op_sbb); break;
		case 0x6c: am_b2a(&tms7000_device::op_mpy); break;
		case 0x6d: am_b2a(&tms7000_device::op_cmp); break;
		case 0x6e: am_b2a(&tms7000_device::op_dac); break;
		case 0x6f: am_b2a(&tms7000_device::op_dsb); break;

		case 0x72: am_i2r(&tms7000_device::op_mov); break;
		case 0x73: am_i2r(&tms7000_device::op_and); break;
		case 0x74: am_i2r(&tms7000_device::op_or); break;
		case 0x75: am_i2r(&tms7000_device::op_xor); break;
		case 0x76: am_i2r(&tms7000_device::op_btjo); break;
		case 0x77: am_i2r(&tms7000_device::op_btjz); break;
		case 0x78: am_i2r(&tms7000_device::op_add); break;
		case 0x79: am_i2r(&tms7000_device::op_adc); break;
		case 0x7a: am_i2r(&tms7000_device::op_sub); break;
		case 0x7b: am_i2r(&tms7000_device::op_sbb); break;
		case 0x7c: am_i2r(&tms7000_device::op_mpy); break;
		case 0x7d: am_i2r(&tms7000_device::op_cmp); break;
		case 0x7e: am_i2r(&tms7000_device::op_dac); break;
		case 0x7f: am_i2r(&tms7000_device::op_dsb); break;

		case 0x80: am_p2a(&tms7000_device::op_mov); break;
		case 0x82: am_a2p(&tms7000_device::op_mov); break;
		case 0x83: am_a2p(&tms7000_device::op_and); break;
		case 0x84: am_a2p(&tms7000_device::op_or); break;
		case 0x85: am_a2p(&tms7000_device::op_xor); break;
		case 0x86: am_a2p(&tms7000_device::op_btjo); break;
		case 0x87: am_a2p(&tms7000_device::op_btjz); break;
		case 0x88: movd_dir(); break;
		case 0x8a: lda_dir(); break;
		case 0x8b: sta_dir(); break;
		case 0x8c: br_dir(); break;
		case 0x8d: cmpa_dir(); break;
		case 0x8e: call_dir(); break;

		case 0x91: am_p2b(&tms7000_device::op_mov); break;
		case 0x92: am_b2p(&tms7000_device::op_mov); break;
		case 0x93: am_b2p(&tms7000_device::op_and); break;
		case 0x94: am_b2p(&tms7000_device::op_or); break;
		case 0x95: am_b2p(&tms7000_device::op_xor); break;
		case 0x96: am_b2p(&tms7000_device::op_btjo); break;
		case 0x97: am_b2p(&tms7000_device::op_btjz); break;
		case 0x98: movd_ind(); break;
		case 0x9a: lda_ind(); break;
		case 0x9b: sta_ind(); break;
		case 0x9c: br_ind(); break;
		case 0x9d: cmpa_ind(); break;
		case 0x9e: call_ind(); break;

		case 0xa2: am_i2p(&tms7000_device::op_mov); break;
		case 0xa3: am_i2p(&tms7000_device::op_and); break;
		case 0xa4: am_i2p(&tms7000_device::op_or); break;
		case 0xa5: am_i2p(&tms7000_device::op_xor); break;
		case 0xa6: am_i2p(&tms7000_device::op_btjo); break;
		case 0xa7: am_i2p(&tms7000_device::op_btjz); break;
		case 0xa8: movd_inx(); break;
		case 0xaa: lda_inx(); break;
		case 0xab: sta_inx(); break;
		case 0xac: br_inx(); break;
		case 0xad: cmpa_inx(); break;
		case 0xae: call_inx(); break;

		case 0xb0: am_a2a(&tms7000_device::op_mov); break; // aka clrc/tsta
		case 0xb1: am_b2a(&tms7000_device::op_mov); break; // undocumented
		case 0xb2: am_a(&tms7000_device::op_dec); break;
		case 0xb3: am_a(&tms7000_device::op_inc); break;
		case 0xb4: am_a(&tms7000_device::op_inv); break;
		case 0xb5: am_a(&tms7000_device::op_clr); break;
		case 0xb6: am_a(&tms7000_device::op_xchb); break;
		case 0xb7: am_a(&tms7000_device::op_swap); break;
		case 0xb8: push_a(); break;
		case 0xb9: pop_a(); break;
		case 0xba: am_a(&tms7000_device::op_djnz); break;
		case 0xbb: decd_a(); break;
		case 0xbc: am_a(&tms7000_device::op_rr); break;
		case 0xbd: am_a(&tms7000_device::op_rrc); break;
		case 0xbe: am_a(&tms7000_device::op_rl); break;
		case 0xbf: am_a(&tms7000_device::op_rlc); break;

		case 0xc0: am_a2b(&tms7000_device::op_mov); break;
		case 0xc1: am_b2b(&tms7000_device::op_mov); break; // aka tstb
		case 0xc2: am_b(&tms7000_device::op_dec); break;
		case 0xc3: am_b(&tms7000_device::op_inc); break;
		case 0xc4: am_b(&tms7000_device::op_inv); break;
		case 0xc5: am_b(&tms7000_device::op_clr); break;
		case 0xc6: am_b(&tms7000_device::op_xchb); break; // result equivalent to tstb
		case 0xc7: am_b(&tms7000_device::op_swap); break;
		case 0xc8: push_b(); break;
		case 0xc9: pop_b(); break;
		case 0xca: am_b(&tms7000_device::op_djnz); break;
		case 0xcb: decd_b(); break;
		case 0xcc: am_b(&tms7000_device::op_rr); break;
		case 0xcd: am_b(&tms7000_device::op_rrc); break;
		case 0xce: am_b(&tms7000_device::op_rl); break;
		case 0xcf: am_b(&tms7000_device::op_rlc); break;

		case 0xd0: am_a2r(&tms7000_device::op_mov); break;
		case 0xd1: am_b2r(&tms7000_device::op_mov); break;
		case 0xd2: am_r(&tms7000_device::op_dec); break;
		case 0xd3: am_r(&tms7000_device::op_inc); break;
		case 0xd4: am_r(&tms7000_device::op_inv); break;
		case 0xd5: am_r(&tms7000_device::op_clr); break;
		case 0xd6: am_r(&tms7000_device::op_xchb); break;
		case 0xd7: am_r(&tms7000_device::op_swap); break;
		case 0xd8: push_r(); break;
		case 0xd9: pop_r(); break;
		case 0xda: am_r(&tms7000_device::op_djnz); break;
		case 0xdb: decd_r(); break;
		case 0xdc: am_r(&tms7000_device::op_rr); break;
		case 0xdd: am_r(&tms7000_device::op_rrc); break;
		case 0xde: am_r(&tms7000_device::op_rl); break;
		case 0xdf: am_r(&tms7000_device::op_rlc); break;

		case 0xe0: jmp(true); break;
		case 0xe1: jmp(m_sr & SR_N); break; // jn/jlt
		case 0xe2: jmp(m_sr & SR_Z); break; // jz/jeq
		case 0xe3: jmp(m_sr & SR_C); break; // jc/jhs
		case 0xe4: jmp(!(m_sr & (SR_Z | SR_N))); break; // jp/jgt
		case 0xe5: jmp(!(m_sr & SR_N)); break; // jpz/jge - note: error in TI official documentation
		case 0xe6: jmp(!(m_sr & SR_Z)); break; // jnz/jne
		case 0xe7: jmp(!(m_sr & SR_C)); break; // jnc/jl

		case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
		case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
		case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
			trap(op << 1); break;

		default: illegal(op); break;
	}
}

void tms7020_exl_device::execute_one(UINT8 op)
{
	// TMS7020 Exelvision EXL 100 custom opcode(s)
	if (op == 0xd7)
		lvdp();
	else
		tms7000_device::execute_one(op);
}


//-------------------------------------------------
//  TMS70C46 specifics
//-------------------------------------------------

void tms70c46_device::device_start()
{
	// init/zerofill
	m_control = 0;

	// register for savestates
	save_item(NAME(m_control));

	tms7000_device::device_start();
}

void tms70c46_device::device_reset()
{
	m_control = 0;
	m_io->write_byte(TMS7000_PORTE, 0xff);

	tms7000_device::device_reset();
}

READ8_MEMBER(tms70c46_device::control_r)
{
	return m_control;
}

WRITE8_MEMBER(tms70c46_device::control_w)
{
	// d5: enable external databus
	if (~m_control & data & 0x20)
		m_io->write_byte(TMS7000_PORTE, 0xff); // go into high impedance

	// d4: enable clock divider when accessing slow memory (not emulated)
	// known fast memory areas: internal ROM/RAM, system RAM
	// known slow memory areas: system ROM, cartridge ROM/RAM

	// d0-d3(all bits?): clock divider when d4 is set and addressbus is in slow memory area
	// needs to be measured, i just know that $30 is full speed, and $38 is about 4 times slower
	m_control = data;
}

// DOCK-BUS: TODO..
// right now pretend that nothing is connected
// external pins are HD0-HD3(data), HSK(handshake), BAV(bus available)

READ8_MEMBER(tms70c46_device::dockbus_status_r)
{
	// d0: slave _HSK
	// d1: slave _BAV
	// d2: unused?
	// d3: IRQ active
	return 0;
}

WRITE8_MEMBER(tms70c46_device::dockbus_status_w)
{
	// d0: master _HSK (setting it low(write 1) also clears IRQ)
	// d1: master _BAV
	// other bits: unused?
}

READ8_MEMBER(tms70c46_device::dockbus_data_r)
{
	return 0xff;
}

WRITE8_MEMBER(tms70c46_device::dockbus_data_w)
{
}
