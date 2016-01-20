// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Namco 53XX

    This instance of the Fujitsu MB8843 MCU is programmed to act as an I/O
    device. It is used by just two games: Dig Dug and Pole Position.

    MOD0-MOD2 = input mode
    CS0-CS3 = chip select lines used to select 1 of 4 input sources
    OUT0-OUT7 = 8-bit final output data
    P0.0-P0.3 = input port 0 data
    P1.0-P1.3 = input port 1 data
    P2.0-P2.3 = input port 2 data
    P3.0-P3.3 = input port 3 data

                   +------+
                 EX|1   42|Vcc
                  X|2   41|K3 (MOD2)
             /RESET|3   40|K2 (MOD1)
               /IRQ|4   39|K1 (MOD0)
                 SO|5   38|K0
                 SI|6   37|R15 (P3.3)
            /SC /TO|7   36|R14 (P3.2)
                /TC|8   35|R13 (P3.1)
           (CS0) P0|9   34|R12 (P3.0)
           (CS1) P1|10  33|R11 (P2.3)
           (CS2) P2|11  32|R10 (P2.2)
           (CS3) P3|12  31|R9 (P2.1)
          (OUT0) O0|13  30|R8 (P2.0)
          (OUT1) O1|14  29|R7 (P1.3)
          (OUT2) O2|15  28|R6 (P1.2)
          (OUT3) O3|16  27|R5 (P1.1)
          (OUT4) O4|17  26|R4 (P1.0)
          (OUT5) O5|18  25|R3 (P0.3)
          (OUT6) O6|19  24|R2 (P0.2)
          (OUT7) O7|20  23|R1 (P0.1)
                GND|21  22|R0 (P0.0)
                   +------+

    MOD selects one of 8 modes in which the input data is interpreted.

    Pole Position is hard-wired to use mode 0, which reads 4 steering
    inputs and 4 DIP switches (only 1 of each is used). The steering
    inputs are clocked on P0 and direction on P1, 1 bit per analog input.
    The DIP switches are connected to P2 and P3.

    Dig Dug can control which mode to use via the MOD bit latches. It sets
    these values to mode 7 when running.

    Unknowns:
        SO is connected to IOSEL on Pole Position

***************************************************************************/

#include "emu.h"
#include "namco53.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

READ8_MEMBER( namco_53xx_device::K_r )
{
	return m_k(0);
}

READ8_MEMBER( namco_53xx_device::Rx_r )
{
	switch(offset) {
		case 0 : return m_in_0(0);
		case 1 : return m_in_1(0);
		case 2 : return m_in_2(0);
		case 3 : return m_in_3(0);
		default : return 0xff;
	}

}

WRITE8_MEMBER( namco_53xx_device::O_w )
{
	UINT8 out = (data & 0x0f);
	if (data & 0x10)
		m_portO = (m_portO & 0x0f) | (out << 4);
	else
		m_portO = (m_portO & 0xf0) | (out);
}

WRITE8_MEMBER( namco_53xx_device::P_w )
{
	m_p(space, 0, data);
}


TIMER_CALLBACK_MEMBER( namco_53xx_device::irq_clear )
{
	m_cpu->set_input_line(0, CLEAR_LINE);
}

WRITE_LINE_MEMBER(namco_53xx_device::read_request)
{
	m_cpu->set_input_line(0, ASSERT_LINE);

	// The execution time of one instruction is ~4us, so we must make sure to
	// give the cpu time to poll the /IRQ input before we clear it.
	// The input clock to the 06XX interface chip is 64H, that is
	// 18432000/6/64 = 48kHz, so it makes sense for the irq line to be
	// asserted for one clock cycle ~= 21us.
	machine().scheduler().timer_set(attotime::from_usec(21), timer_expired_delegate(FUNC(namco_53xx_device::irq_clear),this), 0);
}

READ8_MEMBER( namco_53xx_device::read )
{
	UINT8 res = m_portO;

	read_request(0);

	return res;
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

static ADDRESS_MAP_START( namco_53xx_map_io, AS_IO, 8,namco_53xx_device )
	AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ(K_r)
	AM_RANGE(MB88_PORTO,  MB88_PORTO)  AM_WRITE(O_w)
	AM_RANGE(MB88_PORTP,  MB88_PORTP)  AM_WRITE(P_w)
	AM_RANGE(MB88_PORTR0, MB88_PORTR3) AM_READ(Rx_r)
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( namco_53xx )
	MCFG_CPU_ADD("mcu", MB8843, DERIVED_CLOCK(1,1))     /* parent clock, internally divided by 6 */
	MCFG_CPU_IO_MAP(namco_53xx_map_io)
MACHINE_CONFIG_END


ROM_START( namco_53xx )
	ROM_REGION( 0x400, "mcu", 0 )
	ROM_LOAD( "53xx.bin",     0x0000, 0x0400, CRC(b326fecb) SHA1(758d8583d658e4f1df93184009d86c3eb8713899) )
ROM_END

const device_type NAMCO_53XX = &device_creator<namco_53xx_device>;

namco_53xx_device::namco_53xx_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NAMCO_53XX, "Namco 53xx", tag, owner, clock, "namco53", __FILE__),
	m_cpu(*this, "mcu"),
	m_portO(0),
	m_k(*this),
	m_in_0(*this),
	m_in_1(*this),
	m_in_2(*this),
	m_in_3(*this),
	m_p(*this)
{
}
//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_53xx_device::device_start()
{
	/* resolve our read/write callbacks */
	m_k.resolve_safe(0);
	m_in_0.resolve_safe(0);
	m_in_1.resolve_safe(0);
	m_in_2.resolve_safe(0);
	m_in_3.resolve_safe(0);
	m_p.resolve_safe();

	save_item(NAME(m_portO));
}

//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor namco_53xx_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( namco_53xx  );
}

//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  the device's ROM definitions
//-------------------------------------------------

const rom_entry *namco_53xx_device::device_rom_region() const
{
	return ROM_NAME(namco_53xx );
}
