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
    The Atari schematic for Pole Position lists pin 5 (SO) connected to IOSEL.
    This seems to be a misprint, and it likely goes to to pin 3 (reset) like
    the other Namco custom chips.

***************************************************************************/

#include "emu.h"
#include "namco53.h"


void namco_53xx_device::reset(int state)
{
	// The incoming signal is active low
	m_cpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
}

uint8_t namco_53xx_device::K_r()
{
	return m_k();
}

template<int N>
uint8_t namco_53xx_device::R_r()
{
	return m_in[N]();
}

void namco_53xx_device::O_w(uint8_t data)
{
	m_portO = data;
}

void namco_53xx_device::P_w(uint8_t data)
{
	m_p(data);
}

void namco_53xx_device::chip_select(int state)
{
	m_cpu->set_input_line(MB88XX_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t namco_53xx_device::read()
{
	return m_portO;
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

ROM_START( namco_53xx )
	ROM_REGION( 0x400, "mcu", 0 )
	ROM_LOAD( "53xx.bin", 0x0000, 0x0400, CRC(b326fecb) SHA1(758d8583d658e4f1df93184009d86c3eb8713899) )
ROM_END

DEFINE_DEVICE_TYPE(NAMCO_53XX, namco_53xx_device, "namco53", "Namco 53xx")

namco_53xx_device::namco_53xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NAMCO_53XX, tag, owner, clock),
	m_cpu(*this, "mcu"),
	m_portO(0),
	m_k(*this, 0),
	m_in(*this, 0),
	m_p(*this)
{
}
//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_53xx_device::device_start()
{
	save_item(NAME(m_portO));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void namco_53xx_device::device_add_mconfig(machine_config &config)
{
	MB8843(config, m_cpu, DERIVED_CLOCK(1,1)); // parent clock, internally divided by 6
	m_cpu->read_k().set(FUNC(namco_53xx_device::K_r));
	m_cpu->write_o().set(FUNC(namco_53xx_device::O_w));
	m_cpu->write_p().set(FUNC(namco_53xx_device::P_w));
	m_cpu->read_r<0>().set(FUNC(namco_53xx_device::R_r<0>));
	m_cpu->read_r<1>().set(FUNC(namco_53xx_device::R_r<1>));
	m_cpu->read_r<2>().set(FUNC(namco_53xx_device::R_r<2>));
	m_cpu->read_r<3>().set(FUNC(namco_53xx_device::R_r<3>));
}

//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  the device's ROM definitions
//-------------------------------------------------

const tiny_rom_entry *namco_53xx_device::device_rom_region() const
{
	return ROM_NAME(namco_53xx );
}
