// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Namco 52XX

    This instance of the Fujitsu MB8843 MCU is programmed to act as a
    sample player. It is used by just a few games, most notably Bosconian
    and Pole Position.

    A0-A15 = address to read from sample ROMs
    D0-D7 = data from sample ROMs
    CMD0-CMD3 = command from CPU (sample to play, 0 = none)
    OUT0-OUT3 = sound output

                  +------+
                EX|1   42|Vcc
                 X|2   41|K3 (CMD3)
            /RESET|3   40|K2 (CMD2)
              /IRQ|4   39|K1 (CMD1)
         (n.c.) SO|5   38|K0 (CMD0)
            [1] SI|6   37|R15 (A7)
     (n.c.) /SC/TO|7   36|R14 (A6)
           [2] /TC|8   35|R13 (A5)
         (OUT0) P0|9   34|R12 (A4)
         (OUT1) P1|10  33|R11 (A3)
         (OUT2) P2|11  32|R10 (A2)
         (OUT3) P3|12  31|R9 (A1)
           (A8) O0|13  30|R8 (A0)
           (A9) O1|14  29|R7 (D7)
          (A10) O2|15  28|R6 (D6)
          (A11) O3|16  27|R5 (D5)
          (A12) O4|17  26|R4 (D4)
          (A13) O5|18  25|R3 (D3)
          (A14) O6|19  24|R2 (D2)
          (A15) O7|20  23|R1 (D1)
               GND|21  22|R0 (D0)
                  +------+

    [1] in polepos, +5V; in bosco, GND
        this value controls the ROM addressing mode:
           if 0 (GND), A12-A15 are direct active-low chip enables
           if 1 (Vcc), A12-A15 are address lines

    [2] in polepos, GND; in bosco, output from a 555 timer
        this value is an external timer, which is used for some samples

***************************************************************************/

#include "emu.h"
#include "namco52.h"

TIMER_CALLBACK_MEMBER( namco_52xx_device::latch_callback )
{
	m_latched_cmd = param;
}

READ8_MEMBER( namco_52xx_device::K_r )
{
	return m_latched_cmd & 0x0f;
}

READ8_MEMBER( namco_52xx_device::SI_r )
{
	return m_si(0) ? 1 : 0;
}

READ8_MEMBER( namco_52xx_device::R0_r )
{
	return m_romread(m_address) & 0x0f;
}

READ8_MEMBER( namco_52xx_device::R1_r )
{
	return m_romread(m_address) >> 4;
}


WRITE8_MEMBER( namco_52xx_device::P_w )
{
	m_discrete->write(space, NAMCO_52XX_P_DATA(m_basenode), data & 0x0f);
}

WRITE8_MEMBER( namco_52xx_device::R2_w )
{
	m_address = (m_address & 0xfff0) | ((data & 0xf) << 0);
}

WRITE8_MEMBER( namco_52xx_device::R3_w )
{
	m_address = (m_address & 0xff0f) | ((data & 0xf) << 4);
}

WRITE8_MEMBER( namco_52xx_device::O_w )
{
	if (data & 0x10)
		m_address = (m_address & 0x0fff) | ((data & 0xf) << 12);
	else
		m_address = (m_address & 0xf0ff) | ((data & 0xf) << 8);
}

TIMER_CALLBACK_MEMBER( namco_52xx_device::irq_clear )
{
	m_cpu->set_input_line(0, CLEAR_LINE);
}

WRITE8_MEMBER( namco_52xx_device::write )
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(namco_52xx_device::latch_callback),this), data);

	m_cpu->set_input_line(0, ASSERT_LINE);

	// The execution time of one instruction is ~4us, so we must make sure to
	// give the cpu time to poll the /IRQ input before we clear it.
	// The input clock to the 06XX interface chip is 64H, that is
	// 18432000/6/64 = 48kHz, so it makes sense for the irq line to be
	// asserted for one clock cycle ~= 21us.

	/* the 52xx uses TSTI to check for an interrupt; it also may be handling
	   a timer interrupt, so we need to ensure the IRQ line is held long enough */
	machine().scheduler().timer_set(attotime::from_usec(5*21), timer_expired_delegate(FUNC(namco_52xx_device::irq_clear),this), 0);
}


TIMER_CALLBACK_MEMBER( namco_52xx_device::external_clock_pulse )
{
	m_cpu->clock_w(ASSERT_LINE);
	m_cpu->clock_w(CLEAR_LINE);
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

static ADDRESS_MAP_START( namco_52xx_map_io, AS_IO, 8, namco_52xx_device )
	AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ(K_r)
	AM_RANGE(MB88_PORTO,  MB88_PORTO)  AM_WRITE(O_w)
	AM_RANGE(MB88_PORTP,  MB88_PORTP)  AM_WRITE(P_w)
	AM_RANGE(MB88_PORTSI, MB88_PORTSI) AM_READ(SI_r)
	AM_RANGE(MB88_PORTR0, MB88_PORTR0) AM_READ(R0_r)
	AM_RANGE(MB88_PORTR1, MB88_PORTR1) AM_READ(R1_r)
	AM_RANGE(MB88_PORTR2, MB88_PORTR2) AM_WRITE(R2_w)
	AM_RANGE(MB88_PORTR3, MB88_PORTR3) AM_WRITE(R3_w)
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( namco_52xx )
	MCFG_CPU_ADD("mcu", MB8843, DERIVED_CLOCK(1,1))     /* parent clock, internally divided by 6 */
	MCFG_CPU_IO_MAP(namco_52xx_map_io)
MACHINE_CONFIG_END


ROM_START( namco_52xx )
	ROM_REGION( 0x400, "mcu", 0 )
	ROM_LOAD( "52xx.bin",     0x0000, 0x0400, CRC(3257d11e) SHA1(4883b2fdbc99eb7b9906357fcc53915842c2c186) )
ROM_END


const device_type NAMCO_52XX = &device_creator<namco_52xx_device>;

namco_52xx_device::namco_52xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NAMCO_52XX, "Namco 52xx", tag, owner, clock, "namco52", __FILE__),
	m_cpu(*this, "mcu"),
	m_discrete(*this),
	m_basenode(0),
	m_extclock(0),
	m_romread(*this),
	m_si(*this),
	m_latched_cmd(0),
	m_address(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_52xx_device::device_start()
{
	/* resolve our read/write callbacks */
	m_romread.resolve_safe(0);
	m_si.resolve_safe(0);

	/* start the external clock */
	if (m_extclock != 0)
		machine().scheduler().timer_pulse(attotime(0, m_extclock), timer_expired_delegate(FUNC(namco_52xx_device::external_clock_pulse),this), 0);
}

//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor namco_52xx_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( namco_52xx  );
}

//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  the device's ROM definitions
//-------------------------------------------------

const rom_entry *namco_52xx_device::device_rom_region() const
{
	return ROM_NAME(namco_52xx );
}
