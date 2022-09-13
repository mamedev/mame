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

WRITE_LINE_MEMBER( namco_52xx_device::reset )
{
	// The incoming signal is active low
	m_cpu->set_input_line(INPUT_LINE_RESET, !state);
}

uint8_t namco_52xx_device::K_r()
{
	return m_latched_cmd & 0x0f;
}

READ_LINE_MEMBER( namco_52xx_device::SI_r )
{
	return m_si(0) ? 1 : 0;
}

uint8_t namco_52xx_device::R0_r()
{
	return m_romread(m_address) & 0x0f;
}

uint8_t namco_52xx_device::R1_r()
{
	return m_romread(m_address) >> 4;
}


void namco_52xx_device::P_w(uint8_t data)
{
	m_discrete->write(NAMCO_52XX_P_DATA(m_basenode), data & 0x0f);
}

void namco_52xx_device::R2_w(uint8_t data)
{
	m_address = (m_address & 0xfff0) | ((data & 0xf) << 0);
}

void namco_52xx_device::R3_w(uint8_t data)
{
	m_address = (m_address & 0xff0f) | ((data & 0xf) << 4);
}

void namco_52xx_device::O_w(uint8_t data)
{
	if (data & 0x10)
		m_address = (m_address & 0x0fff) | ((data & 0xf) << 12);
	else
		m_address = (m_address & 0xf0ff) | ((data & 0xf) << 8);
}


void namco_52xx_device::write(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(namco_52xx_device::write_sync),this), data);
}

TIMER_CALLBACK_MEMBER( namco_52xx_device::write_sync )
{
	m_latched_cmd = param;
}

WRITE_LINE_MEMBER( namco_52xx_device::chip_select )
{
	m_cpu->set_input_line(0, state);
}

TIMER_CALLBACK_MEMBER( namco_52xx_device::external_clock_pulse )
{
	m_cpu->clock_w(ASSERT_LINE);
	m_cpu->clock_w(CLEAR_LINE);
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

ROM_START( namco_52xx )
	ROM_REGION( 0x400, "mcu", 0 )
	ROM_LOAD( "52xx.bin",     0x0000, 0x0400, CRC(3257d11e) SHA1(4883b2fdbc99eb7b9906357fcc53915842c2c186) )
ROM_END


DEFINE_DEVICE_TYPE(NAMCO_52XX, namco_52xx_device, "namco52", "Namco 52xx")

namco_52xx_device::namco_52xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NAMCO_52XX, tag, owner, clock),
	m_cpu(*this, "mcu"),
	m_discrete(*this, finder_base::DUMMY_TAG),
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
	{
		m_extclock_pulse_timer = timer_alloc(FUNC(namco_52xx_device::external_clock_pulse), this);
		m_extclock_pulse_timer->adjust(attotime(0, m_extclock), 0, attotime(0, m_extclock));
	}

	save_item(NAME(m_latched_cmd));
	save_item(NAME(m_address));
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void namco_52xx_device::device_add_mconfig(machine_config &config)
{
	MB8843(config, m_cpu, DERIVED_CLOCK(1,1));     /* parent clock, internally divided by 6 */
	m_cpu->read_k().set(FUNC(namco_52xx_device::K_r));
	m_cpu->write_o().set(FUNC(namco_52xx_device::O_w));
	m_cpu->write_p().set(FUNC(namco_52xx_device::P_w));
	m_cpu->read_si().set(FUNC(namco_52xx_device::SI_r));
	m_cpu->read_r<0>().set(FUNC(namco_52xx_device::R0_r));
	m_cpu->read_r<1>().set(FUNC(namco_52xx_device::R1_r));
	m_cpu->write_r<2>().set(FUNC(namco_52xx_device::R2_w));
	m_cpu->write_r<3>().set(FUNC(namco_52xx_device::R3_w));
}

//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  the device's ROM definitions
//-------------------------------------------------

const tiny_rom_entry *namco_52xx_device::device_rom_region() const
{
	return ROM_NAME(namco_52xx );
}
