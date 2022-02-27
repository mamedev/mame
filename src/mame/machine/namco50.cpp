// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Namco 50XX

    This custom chip is a Fujitsu MB8842 MCU programmed to act as a
    protection device. It keeps track of the players scores, and checks if
    a high score has been obtained or bonus lives should be awarded. The
    main CPU has a range of commands to increment/decrement the score by
    various fixed amounts.

    The device is used to its full potential only by Bosconian; Xevious
    uses it too, but only to do a protection check on startup.

    CMD = command from main CPU
    ANS = answer to main CPU

    The chip reads/writes the I/O ports when the /IRQ is pulled down. Pin 21
    determines whether a read or write should happen (1=R, 0=W).

                  +------+
                EX|1   28|Vcc
                 X|2   27|K3 (CMD7)
            /RESET|3   26|K2 (CMD6)
         (ANS0) O0|4   25|K1 (CMD5)
         (ANS1) O1|5   24|K0 (CMD4)
         (ANS2) O2|6   23|R10/IRQ
         (ANS3) O3|7   22|R9/TC (n.c.)
         (ANS4) O4|8   21|R8 (R/W)
         (ANS5) O5|9   20|R7 (n.c.)
         (ANS6) O6|10  19|R6 (n.c.)
         (ANS7) O7|11  18|R5 (n.c.)
         (CMD0) R0|12  17|R4 (n.c.)
         (CMD1) R1|13  16|R3 (CMD3)
               GND|14  15|R2 (CMD2)
                  +------+



Commands:

0x = nop

1x = reset scores

2x = set first bonus score (followed by 3 bytes)

3x = set interval bonus score (followed by 3 bytes)

4x = ?

5x = set high score (followed by 3 bytes)

60 = switch to player 1
68 = switch to player 2

70 = switch to increment score
7x = switch to decrement score

score increments/decrements:

80 =    5
81 =   10
82 =   15
83 =   20
84 =   25
85 =   30
86 =   40
87 =   50
88 =   60
89 =   70
8A =   80
8B =   90
8C =  100
8D =  200
8E =  300
8F =  500

9x same as 8x but *10
Ax same as 8x but *100

B0h =   10
B1h =   20
B2h =   30
B3h =   40
B4h =   50
B5h =   60
B6h =   80
B7h =  100
B8h =  120
B9h =  140
BAh =  160
BBh =  180
BCh =  200
BDh =  400
BEh =  600
BFh = 1000

Cx same as Bx but *10
Dx same as Bx but *100

E0 =   15
E1 =   30
E2 =   45
E3 =   60
E4 =   75
E5 =   90
E6 =  120
E7 =  150
E8 =  180
E9 =  210
EA =  240
EB =  270
EC =  300
ED =  600
EE =  900
EF = 1500

Fx same as Ex but *10


When reading, the score for the currently selected player is returned. The first
byte also contains flags.

Byte 0: BCD Score (fs------) and flags
Byte 1: BCD Score (--ss----)
Byte 2: BCD Score (----ss--)
Byte 3: BCD Score (------ss)

Flags: 80=high score, 40=first bonus, 20=interval bonus, 10=?

***************************************************************************/

#include "emu.h"
#include "namco50.h"

WRITE_LINE_MEMBER( namco_50xx_device::reset )
{
	// The incoming signal is active low
	m_cpu->set_input_line(INPUT_LINE_RESET, !state);
}


uint8_t namco_50xx_device::K_r()
{
	return m_cmd >> 4;
}

uint8_t namco_50xx_device::R0_r()
{
	return m_cmd & 0x0f;
}

uint8_t namco_50xx_device::R2_r()
{
	return m_rw & 1;
}

void namco_50xx_device::O_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(namco_50xx_device::O_w_sync),this), data);
}

TIMER_CALLBACK_MEMBER( namco_50xx_device::O_w_sync )
{
	uint8_t out = (param & 0x0f);
	if (param & 0x10)
		m_portO = (m_portO & 0x0f) | (out << 4);
	else
		m_portO = (m_portO & 0xf0) | (out);
}

WRITE_LINE_MEMBER( namco_50xx_device::rw )
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(namco_50xx_device::rw_sync),this), state);
}

TIMER_CALLBACK_MEMBER( namco_50xx_device::rw_sync )
{
	m_rw = param;
}

WRITE_LINE_MEMBER( namco_50xx_device::chip_select )
{
	m_cpu->set_input_line(0, state);
}

void namco_50xx_device::write(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(namco_50xx_device::write_sync),this), data);
}

TIMER_CALLBACK_MEMBER( namco_50xx_device::write_sync )
{
	m_cmd = param;
}

uint8_t namco_50xx_device::read()
{
	return m_portO;
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

ROM_START( namco_50xx )
	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "50xx.bin",     0x0000, 0x0800, CRC(a0acbaf7) SHA1(f03c79451e73b3a93c1591cdb27fedc9f130508d) )
ROM_END


DEFINE_DEVICE_TYPE(NAMCO_50XX, namco_50xx_device, "namco50", "Namco 50xx")

namco_50xx_device::namco_50xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NAMCO_50XX, tag, owner, clock),
	m_cpu(*this, "mcu"),
	m_rw(0),
	m_cmd(0),
	m_portO(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_50xx_device::device_start()
{
	save_item(NAME(m_rw));
	save_item(NAME(m_cmd));
	save_item(NAME(m_portO));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void namco_50xx_device::device_add_mconfig(machine_config &config)
{
	MB8842(config, m_cpu, DERIVED_CLOCK(1,1)); /* parent clock, internally divided by 6 */
	m_cpu->read_k().set(FUNC(namco_50xx_device::K_r));
	m_cpu->read_r<0>().set(FUNC(namco_50xx_device::R0_r));
	m_cpu->read_r<2>().set(FUNC(namco_50xx_device::R2_r));
	m_cpu->write_o().set(FUNC(namco_50xx_device::O_w));
}

//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  the device's ROM definitions
//-------------------------------------------------

const tiny_rom_entry *namco_50xx_device::device_rom_region() const
{
	return ROM_NAME(namco_50xx );
}
