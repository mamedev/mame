// license:BSD-3-Clause
// copyright-holders:Mike Harris, Aaron Giles
/***************************************************************************

    Namco 51XX

    This custom chip is a Fujitsu MB8843 MCU programmed to act as an I/O
    device with built-in coin management. It is also apparently used as a
    protection device. It keeps track of the players scores, and checks
    if a high score has been obtained or bonus lives should be awarded.
    The main CPU has a range of commands to increment/decrement the score
    by various fixed amounts.

    The device is used to its full potential only by Bosconian; Xevious
    uses it too, but only to do a protection check on startup.

    CMD = command from main CPU
    ANS = answer to main CPU

    The chip reads/writes the I/O ports when the /IRQ is pulled down.
    Pin 41 determines whether a read or write should happen (1=R, 0=W).

               +------+
             EX|1   42|Vcc
              X|2   41|K3
         /RESET|3   40|K2
           /IRQ|4   39|K1
             SO|5   38|K0
             SI|6   37|R15
        /SC /TO|7   36|R14
            /TC|8   35|R13
             P0|9   34|R12
             P1|10  33|R11
             P2|11  32|R10
             P3|12  31|R9
             O0|13  30|R8
             O1|14  29|R7
             O2|15  28|R6
             O3|16  27|R5
             O4|17  26|R4
             O5|18  25|R3
             O6|19  24|R2
             O7|20  23|R1
            GND|21  22|R0
               +------+

    commands:
    00: nop
    01 + 4 arguments: set coinage (xevious, possibly because of a bug, is different)
    02: go in "credit" mode and enable start buttons
    03: disable joystick remapping
    04: enable joystick remapping
    05: go in "switch" mode
    06: nop
    07: nop

***************************************************************************/

#include "emu.h"
#include "namco51.h"
#include "screen.h"


void namco_51xx_device::reset(int state)
{
	// Reset line is active low.
	m_cpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
}

void namco_51xx_device::vblank(int state)
{
	// The timer is active on falling edges.
	m_cpu->set_input_line(MB88XX_TC_LINE, state ? CLEAR_LINE : ASSERT_LINE);
}

void namco_51xx_device::rw(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(namco_51xx_device::rw_sync),this), state);
}

TIMER_CALLBACK_MEMBER(namco_51xx_device::rw_sync)
{
	m_rw = param;
}

void namco_51xx_device::chip_select(int state)
{
	m_cpu->set_input_line(MB88XX_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t namco_51xx_device::read()
{
	return m_portO;
}

void namco_51xx_device::write(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(namco_51xx_device::write_sync),this), data);
}

TIMER_CALLBACK_MEMBER(namco_51xx_device::write_sync)
{
	m_portO = param;
}

uint8_t namco_51xx_device::K_r()
{
	return (m_rw << 3) | (m_portO & 0x07);
}

template<int N>
uint8_t namco_51xx_device::R_r()
{
	return m_in[N]();
}

void namco_51xx_device::O_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(namco_51xx_device::O_w_sync),this), data);
}

TIMER_CALLBACK_MEMBER(namco_51xx_device::O_w_sync)
{
	m_portO = param;
}

void namco_51xx_device::P_w(uint8_t data)
{
	m_out(data);
}

/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

ROM_START( namco_51xx )
	ROM_REGION( 0x400, "mcu", 0 )
	ROM_LOAD( "51xx.bin", 0x0000, 0x0400, CRC(c2f57ef8) SHA1(50de79e0d6a76bda95ffb02fcce369a79e6abfec) )
ROM_END

DEFINE_DEVICE_TYPE(NAMCO_51XX, namco_51xx_device, "namco51", "Namco 51xx")

namco_51xx_device::namco_51xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NAMCO_51XX, tag, owner, clock)
	, m_cpu(*this, "mcu")
	, m_portO(0)
	, m_rw(0)
	, m_in(*this, 0)
	, m_out(*this)
	, m_lockout(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_51xx_device::device_start()
{
	save_item(NAME(m_portO));
	save_item(NAME(m_rw));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void namco_51xx_device::device_add_mconfig(machine_config &config)
{
	MB8843(config, m_cpu, DERIVED_CLOCK(1,1)); // parent clock, internally divided by 6
	m_cpu->read_k().set(FUNC(namco_51xx_device::K_r));
	m_cpu->read_r<0>().set(FUNC(namco_51xx_device::R_r<0>));
	m_cpu->read_r<1>().set(FUNC(namco_51xx_device::R_r<1>));
	m_cpu->read_r<2>().set(FUNC(namco_51xx_device::R_r<2>));
	m_cpu->read_r<3>().set(FUNC(namco_51xx_device::R_r<3>));
	m_cpu->write_o().set(FUNC(namco_51xx_device::O_w));
	m_cpu->write_p().set(FUNC(namco_51xx_device::P_w));
}

//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  the device's ROM definitions
//-------------------------------------------------

const tiny_rom_entry *namco_51xx_device::device_rom_region() const
{
	return ROM_NAME(namco_51xx );
}
