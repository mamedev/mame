// license:BSD-3-Clause
// copyright-holders:hap, Sandro Ronco
/*

The ChessMachine by Tasc

This is the 2nd (1992) version of The ChessMachine, it is used in:
- The ChessMachine DR, PC ISA card
- The ChessMachine EC, PC external module (printer port)
- The ChessMachine EC2, Amiga external module
- Mephisto Risc 1MB/II, chess computer

Hardware notes:
- VLSI-ARM VY86C010-12QC (ARM2 CPU)
- 1MB RAM (8*Siemens HYB514256B-60)
- 128 bytes PROM (4*DM74S288AN)
- 2 GALs for I/O handling

Unlike the SR model (devices/bus/isa/chessmsr.*), RAM size and CPU type
are the same on every known device.

Only 4 lines for I/O, so that part is much slower than the SR model.

There is no XTAL, it looks like there's a variable resistor for tweaking
CPU speed. It should be around 14-16MHz. The ARM CPU is rated 12MHz, they
probably went for this solution to get optimum possible speed for each module.

TODO:
- is interrupt handling correct?
- timer shouldn't be needed for disabling bootrom, real ARM has already read the next opcode

*/

#include "emu.h"
#include "chessmachine.h"


DEFINE_DEVICE_TYPE(CHESSMACHINE, chessmachine_device, "chessmachine", "Tasc ChessMachine")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

chessmachine_device::chessmachine_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CHESSMACHINE, tag, owner, clock),
	m_maincpu(*this, "maincpu"),
	m_boot_view(*this, "boot_view"),
	m_data_out(*this)
{ }



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void chessmachine_device::device_start()
{
	memset(m_latch, 0, sizeof(m_latch));
	m_boot_timer = timer_alloc(FUNC(chessmachine_device::disable_bootrom), this);

	// register for savestates
	save_item(NAME(m_latch));
}



//-------------------------------------------------
//  external handlers
//-------------------------------------------------

void chessmachine_device::data0_w_sync(s32 param)
{
	if ((m_latch[0] & 1) != param)
	{
		machine().scheduler().perfect_quantum(attotime::from_usec(50));
		m_latch[0] = (m_latch[0] & 0x80) | param;
	}
}

void chessmachine_device::data0_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(chessmachine_device::data0_w_sync), this), state ? 1 : 0);
}

void chessmachine_device::data1_w_sync(s32 param)
{
	if ((m_latch[0] & 0x80) != param)
	{
		machine().scheduler().perfect_quantum(attotime::from_usec(50));
		m_latch[0] = (m_latch[0] & 1) | param;

		// cause interrupt?
		m_maincpu->set_input_line(ARM_FIRQ_LINE, param ? ASSERT_LINE : CLEAR_LINE);
	}
}

void chessmachine_device::data1_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(chessmachine_device::data1_w_sync), this), state ? 0x80 : 0);
}

void chessmachine_device::reset_w_sync(s32 param)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, param ? ASSERT_LINE : CLEAR_LINE);

	if (param)
	{
		// enable bootrom
		m_boot_view.select(0);
		m_boot_timer->adjust(attotime::never);
	}
}

void chessmachine_device::reset_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(chessmachine_device::reset_w_sync), this), state ? 1 : 0);
}



//-------------------------------------------------
//  internal
//-------------------------------------------------

u32 chessmachine_device::disable_bootrom_r()
{
	// disconnect bootrom from the bus after next opcode
	if (!machine().side_effects_disabled() && m_boot_timer->remaining().is_never())
		m_boot_timer->adjust(m_maincpu->cycles_to_attotime(5));

	return 0;
}

void chessmachine_device::main_map(address_map &map)
{
	map(0x00000000, 0x000fffff).ram();

	map(0x00000000, 0x0000007f).view(m_boot_view);
	m_boot_view[0](0x00000000, 0x0000007f).rom().region("bootrom", 0);

	map(0x00400000, 0x00400003).mirror(0x003ffffc).rw(FUNC(chessmachine_device::internal_r), FUNC(chessmachine_device::internal_w)).umask32(0x000000ff);
	map(0x01800000, 0x01800003).r(FUNC(chessmachine_device::disable_bootrom_r));
}

void chessmachine_device::device_add_mconfig(machine_config &config)
{
	ARM(config, m_maincpu, DERIVED_CLOCK(1,1));
	m_maincpu->set_addrmap(AS_PROGRAM, &chessmachine_device::main_map);
	m_maincpu->set_copro_type(arm_cpu_device::copro_type::VL86C020);
}



//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( chessmachine )
	ROM_REGION32_LE( 0x80, "bootrom", 0 )
	ROM_LOAD32_BYTE( "74s288.1", 0x00, 0x20, CRC(284114e2) SHA1(df4037536d505d7240bb1d70dc58f59a34ab77b4) )
	ROM_LOAD32_BYTE( "74s288.2", 0x01, 0x20, CRC(9f239c75) SHA1(aafaf30dac90f36b01f9ee89903649fc4ea0480d) )
	ROM_LOAD32_BYTE( "74s288.3", 0x02, 0x20, CRC(0455360b) SHA1(f1486142330f2c39a4d6c479646030d31443d1c8) )
	ROM_LOAD32_BYTE( "74s288.4", 0x03, 0x20, CRC(c7c9aba8) SHA1(cbb5b12b5917e36679d45bcbc36ea9285223a75d) )
ROM_END

const tiny_rom_entry *chessmachine_device::device_rom_region() const
{
	return ROM_NAME(chessmachine);
}
