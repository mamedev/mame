// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Namco C148 - CPU Bus Manager

    Does some Memory Decode, Interrupt Handling, 3 bit PIO port, Bus Controller

    Based off implementation from K.Wilkins and Phil Stroffolino

    TODO:
    - hookup C116 device, @see mame/machine/namcoic.h

=============================================================================

Interrupt Controller C148          1C0000-1FFFFF  R/W  D00-D02
    ????????                       1C0XXX
    ????????                       1C2XXX
    Bus Controller?                1C4XXX              D00-D02
    -x- master priority bit?
    Master/Slave IRQ level         1C6XXX              D00-D02
    EXIRQ level                    1C8XXX              D00-D02
    POSIRQ level                   1CAXXX              D00-D02
    SCIRQ level                    1CCXXX              D00-D02
    VBLANK IRQ level               1CEXXX              D00-D02
    xxx irq level for specific irq.

    ????????                       1D0XXX
    ????????                       1D2XXX
    Acknowledge Bus?               1D4XXX
    Acknowlegde Master/Slave IRQ   1D6XXX ack master/slave INT
    Acknowledge EXIRQ              1D8XXX
    Acknowledge POSIRQ             1DAXXX
    Acknowledge SCIRQ              1DCXXX
    Acknowledge VBLANK IRQ         1DEXXX

    EEPROM Ready status            1E0XXX         R    D01
    Sound CPU Reset control        1E2XXX           W  D01
    Slave 68000 & IO CPU Reset     1E4XXX           W  D01
    xxx PIO ports, per-HW / CPU specific
    Watchdog reset kicker          1E6XXX           W
    xxx Unknown at current stage if internal or external to the C148

***************************************************************************/

#include "emu.h"
#include "namco_c148.h"

#define VERBOSE         0
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(NAMCO_C148, namco_c148_device, "namco_c148", "Namco C148 Interrupt Controller")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  namco_c148_device - constructor
//-------------------------------------------------

namco_c148_device::namco_c148_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NAMCO_C148, tag, owner, clock),
	m_out_ext1_cb(*this),
	m_out_ext2_cb(*this),
	m_hostcpu(*this, finder_base::DUMMY_TAG),
	m_linked_c148(*this, finder_base::DUMMY_TAG)
{
}

// (*) denotes master CPU only
void namco_c148_device::map(address_map &map)
{
	map(0x04000, 0x05fff).rw(FUNC(namco_c148_device::bus_ctrl_r), FUNC(namco_c148_device::bus_ctrl_w)).umask16(0x00ff);
	map(0x06000, 0x07fff).rw(FUNC(namco_c148_device::cpu_irq_level_r), FUNC(namco_c148_device::cpu_irq_level_w)).umask16(0x00ff); // CPUIRQ lv
	map(0x08000, 0x09fff).rw(FUNC(namco_c148_device::ex_irq_level_r), FUNC(namco_c148_device::ex_irq_level_w)).umask16(0x00ff); // EXIRQ lv
	map(0x0a000, 0x0bfff).rw(FUNC(namco_c148_device::pos_irq_level_r), FUNC(namco_c148_device::pos_irq_level_w)).umask16(0x00ff); // POSIRQ lv
	map(0x0c000, 0x0dfff).rw(FUNC(namco_c148_device::sci_irq_level_r), FUNC(namco_c148_device::sci_irq_level_w)).umask16(0x00ff); // SCIRQ lv
	map(0x0e000, 0x0ffff).rw(FUNC(namco_c148_device::vblank_irq_level_r), FUNC(namco_c148_device::vblank_irq_level_w)).umask16(0x00ff); // VBlank IRQ lv

	map(0x10000, 0x11fff).w(FUNC(namco_c148_device::cpu_irq_assert_w));
	map(0x14000, 0x15fff).noprw(); // busack
	map(0x16000, 0x17fff).rw(FUNC(namco_c148_device::cpu_irq_ack_r), FUNC(namco_c148_device::cpu_irq_ack_w)); // CPUIRQ ack
	map(0x18000, 0x19fff).rw(FUNC(namco_c148_device::ex_irq_ack_r), FUNC(namco_c148_device::ex_irq_ack_w)); // EXIRQ ack
	map(0x1a000, 0x1bfff).rw(FUNC(namco_c148_device::pos_irq_ack_r), FUNC(namco_c148_device::pos_irq_ack_w)); // POSIRQ ack
	map(0x1c000, 0x1dfff).rw(FUNC(namco_c148_device::sci_irq_ack_r), FUNC(namco_c148_device::sci_irq_ack_w)); // SCIRQ ack
	map(0x1e000, 0x1ffff).rw(FUNC(namco_c148_device::vblank_irq_ack_r), FUNC(namco_c148_device::vblank_irq_ack_w)); // VBlank IRQ ack
	map(0x20000, 0x21fff).r(FUNC(namco_c148_device::ext_r)).umask16(0x00ff); // EEPROM ready status (*)
	map(0x22000, 0x23fff).nopr().w(FUNC(namco_c148_device::ext1_w)).umask16(0x00ff); // sound CPU reset (*)
	map(0x24000, 0x25fff).nopr().w(FUNC(namco_c148_device::ext2_w)).umask16(0x00ff); // slave & i/o reset (*)
	map(0x26000, 0x27fff).noprw(); // watchdog
}


//-------------------------------------------------
//  device_validity_check - device-specific checks
//-------------------------------------------------

void namco_c148_device::device_validity_check(validity_checker &valid) const
{
	if ((m_linked_c148.finder_tag() != finder_base::DUMMY_TAG) && !m_linked_c148)
		osd_printf_error("Linked C148 configured but not found.\n");
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_c148_device::device_start()
{
	// TODO: link to SCI, EX and the screen device controller devices
	m_bus_reg = 0;

	save_item(NAME(m_irqlevel.cpu));
	save_item(NAME(m_irqlevel.ex));
	save_item(NAME(m_irqlevel.sci));
	save_item(NAME(m_irqlevel.pos));
	save_item(NAME(m_irqlevel.vblank));
	save_item(NAME(m_bus_reg));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void namco_c148_device::device_reset()
{
	vblank_irq_level_w(0);
	pos_irq_level_w(0);
	cpu_irq_level_w(0);
	ex_irq_level_w(0);
	sci_irq_level_w(0);
}


//**************************************************************************
//  IRQ section
//**************************************************************************

uint8_t namco_c148_device::pos_irq_level_r()    { return m_irqlevel.pos & 7; }
uint8_t namco_c148_device::vblank_irq_level_r() { return m_irqlevel.vblank & 7; }
uint8_t namco_c148_device::cpu_irq_level_r()    { return m_irqlevel.cpu & 7; }
uint8_t namco_c148_device::ex_irq_level_r()     { return m_irqlevel.ex & 7; }
uint8_t namco_c148_device::sci_irq_level_r()    { return m_irqlevel.sci & 7; }

void namco_c148_device::vblank_irq_level_w(uint8_t data) { vblank_irq_ack_w(); m_irqlevel.vblank = data & 7; LOG("%s: vblank IRQ level = %02x\n", tag(), data); }
void namco_c148_device::pos_irq_level_w(uint8_t data)    { pos_irq_ack_w();    m_irqlevel.pos = data & 7;    if(data != 0) { LOG("%s: pos IRQ level = %02x\n", tag(), data); } }
void namco_c148_device::cpu_irq_level_w(uint8_t data)    { cpu_irq_ack_w();    m_irqlevel.cpu = data & 7;    LOG("%s: cpu IRQ level = %02x\n", tag(), data); }
void namco_c148_device::ex_irq_level_w(uint8_t data)     { ex_irq_ack_w();     m_irqlevel.ex = data & 7;     LOG("%s: ex IRQ level = %02x\n", tag(), data); }
void namco_c148_device::sci_irq_level_w(uint8_t data)    { sci_irq_ack_w();    m_irqlevel.sci = data & 7;    LOG("%s: sci IRQ level = %02x\n", tag(), data); }

uint16_t namco_c148_device::vblank_irq_ack_r() { if (!machine().side_effects_disabled()) m_hostcpu->set_input_line(m_irqlevel.vblank, CLEAR_LINE); return 0; }
uint16_t namco_c148_device::pos_irq_ack_r()    { if (!machine().side_effects_disabled()) m_hostcpu->set_input_line(m_irqlevel.pos, CLEAR_LINE);    return 0; }
uint16_t namco_c148_device::cpu_irq_ack_r()    { if (!machine().side_effects_disabled()) m_hostcpu->set_input_line(m_irqlevel.cpu, CLEAR_LINE);    return 0; }
uint16_t namco_c148_device::ex_irq_ack_r()     { if (!machine().side_effects_disabled()) m_hostcpu->set_input_line(m_irqlevel.ex, CLEAR_LINE);     return 0; }
uint16_t namco_c148_device::sci_irq_ack_r()    { if (!machine().side_effects_disabled()) m_hostcpu->set_input_line(m_irqlevel.sci, CLEAR_LINE);    return 0; }

void namco_c148_device::vblank_irq_ack_w(uint16_t data) { m_hostcpu->set_input_line(m_irqlevel.vblank, CLEAR_LINE); }
void namco_c148_device::pos_irq_ack_w(uint16_t data)    { m_hostcpu->set_input_line(m_irqlevel.pos, CLEAR_LINE); }
void namco_c148_device::cpu_irq_ack_w(uint16_t data)    { m_hostcpu->set_input_line(m_irqlevel.cpu, CLEAR_LINE); }
void namco_c148_device::ex_irq_ack_w(uint16_t data)     { m_hostcpu->set_input_line(m_irqlevel.ex, CLEAR_LINE); }
void namco_c148_device::sci_irq_ack_w(uint16_t data)    { m_hostcpu->set_input_line(m_irqlevel.sci, CLEAR_LINE); }

void namco_c148_device::vblank_irq_trigger() { m_hostcpu->set_input_line(m_irqlevel.vblank, HOLD_LINE); } // TODO: Phelios doesn't ack the vblank irq at all!
void namco_c148_device::pos_irq_trigger()    { m_hostcpu->set_input_line(m_irqlevel.pos, ASSERT_LINE); }
void namco_c148_device::cpu_irq_trigger()    { m_hostcpu->set_input_line(m_irqlevel.cpu, ASSERT_LINE); }
void namco_c148_device::ex_irq_trigger()     { m_hostcpu->set_input_line(m_irqlevel.ex, ASSERT_LINE); }
void namco_c148_device::sci_irq_trigger()    { m_hostcpu->set_input_line(m_irqlevel.sci, ASSERT_LINE); }


//**************************************************************************
//  Comm ports
//**************************************************************************

uint8_t namco_c148_device::ext_r()
{
	return 0xff; // TODO: bit 0 EEPROM bit ready
}

void namco_c148_device::ext1_w(uint8_t data)
{
	m_out_ext1_cb(data & 7);
}

void namco_c148_device::ext2_w(uint8_t data)
{
	m_out_ext2_cb(data & 7);
	// TODO: bit 1/2 in Winning Run GPU might be irq enable?
}

uint8_t namco_c148_device::bus_ctrl_r()
{
	return m_bus_reg;
}

void namco_c148_device::bus_ctrl_w(uint8_t data)
{
	m_bus_reg = data & 7;
}


void namco_c148_device::cpu_irq_assert_w(uint16_t data)
{
	// TODO: Starblade relies on this for showing large polygons, is it the right place?
	if (m_linked_c148)
		m_linked_c148->cpu_irq_trigger();
}
