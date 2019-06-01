// license:BSD-3-Clause
// copyright-holders:hap, Sandro Ronco
/*

The ChessMachine by Tasc

This is the 2nd (1992) version of The ChessMachine, it is used in:
- The ChessMachine DR, PC ISA card
- The ChessMachine EC, PC external module (printer port)
- The ChessMachine EC2, Amiga external module
- Mephisto Risc 1MB/II, chess computer

Unlike the SR model (devices/bus/isa/chessmsr.*), RAM size and CPU type
are the same on every known device.

- VLSI-ARM VY86C010-12QC (ARM2 CPU)
- 1MB RAM (8*Siemens HYB514256B-60)
- 128 bytes PROM (4*DM74S288AN)
- 2 GALs for I/O handling

Only 4 lines for I/O, so that part is much slower than the SR model.

There is no XTAL, it looks like there's a variable resistor for tweaking
CPU speed. It should be around 14-16MHz. The ARM CPU is rated 12MHz, they
probably went for this solution to get optimum possible speed for each module.

TODO:
- is interrupt handling correct?

*/

#include "emu.h"
#include "machine/chessmachine.h"


DEFINE_DEVICE_TYPE(CHESSMACHINE, chessmachine_device, "chessmachine", "Tasc ChessMachine")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

chessmachine_device::chessmachine_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CHESSMACHINE, tag, owner, clock),
	m_maincpu(*this, "maincpu"),
	m_bootstrap(*this, "bootstrap"),
	m_data_out(*this)
{ }



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void chessmachine_device::device_start()
{
	// resolve callbacks
	m_data_out.resolve_safe();

	// zerofill, register for savestates
	m_latch[0] = m_latch[1] = 0;
	save_item(NAME(m_latch));
}



//-------------------------------------------------
//  external handlers
//-------------------------------------------------

void chessmachine_device::sync0_callback(void *ptr, s32 param)
{
	m_latch[0] = (m_latch[0] & 0x80) | param;
}

void chessmachine_device::data0_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(chessmachine_device::sync0_callback), this), state ? 1 : 0);
}

void chessmachine_device::sync1_callback(void *ptr, s32 param)
{
	m_latch[0] = (m_latch[0] & 1) | param;

	// cause interrupt?
	m_maincpu->set_input_line(ARM_FIRQ_LINE, param ? ASSERT_LINE : CLEAR_LINE);
}

void chessmachine_device::data1_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(chessmachine_device::sync1_callback), this), state ? 0x80 : 0);
}

void chessmachine_device::reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);

	if (state)
	{
		// send bootstrap
		for (int i = 0; i < 0x80; i++)
			m_maincpu->space(AS_PROGRAM).write_byte(i, m_bootstrap[i]);
	}
}



//-------------------------------------------------
//  internal
//-------------------------------------------------

void chessmachine_device::main_map(address_map &map)
{
	map(0x00000000, 0x000fffff).ram();
	map(0x00400000, 0x00400000).mirror(0x003ffffc).rw(FUNC(chessmachine_device::internal_r), FUNC(chessmachine_device::internal_w));
	//map(0x01800000, 0x01800003).nopr(); // disconnect bootstrap?
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
	ROM_REGION( 0x80, "bootstrap", 0 )
	ROM_LOAD32_BYTE( "74s288.1", 0x00, 0x20, CRC(284114e2) SHA1(df4037536d505d7240bb1d70dc58f59a34ab77b4) )
	ROM_LOAD32_BYTE( "74s288.2", 0x01, 0x20, CRC(9f239c75) SHA1(aafaf30dac90f36b01f9ee89903649fc4ea0480d) )
	ROM_LOAD32_BYTE( "74s288.3", 0x02, 0x20, CRC(0455360b) SHA1(f1486142330f2c39a4d6c479646030d31443d1c8) )
	ROM_LOAD32_BYTE( "74s288.4", 0x03, 0x20, CRC(c7c9aba8) SHA1(cbb5b12b5917e36679d45bcbc36ea9285223a75d) )
ROM_END

const tiny_rom_entry *chessmachine_device::device_rom_region() const
{
	return ROM_NAME(chessmachine);
}
