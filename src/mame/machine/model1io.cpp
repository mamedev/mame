// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sega Model 1/2 I/O Board


    I/O PCB
    -------

    837-8950-01 (C) SEGA 1992
    |-------------------------------------------|
    | CN6                           J3   J2     |
    |                                        CN5|
    |                      DSW3       LED1      |
    |                                           |
    | SW7    |---------|                        |
    |  32MHz |SEGA     |   DSW1                 |
    | SW6    |315-5338A|                        |
    |        |         |                        |
    | SW5    |---------|   DSW2                 |
    |                                        CN1|
    | SW4      MB8464                           |
    |          14869.25                         |
    |   3771                                    |
    |          Z80                              |
    |   93C45                                   |
    |                               PC910 PC910 |
    |   LED2                           J1       |
    |      M6253                                |
    | CN3              CN2          CN4     TL1 |
    |-------------------------------------------|
    Notes:
          315-5338A - Sega Custom (QFP100)
          Z80       - Zilog Z0840004PSC Z80 CPU, running at 4.000MHz (DIP40, clock 32 / 8)
          14869.25  - ST Microelectronics M27C512 64k x8 EPROM (DIP28, labelled 'EPR-14869')
                      There is an alternative revision B 'EPR-14869B' also
          MB8464    - Fujitsu MB8464 8k x8 SRAM (DIP28)
          93C45     - 128bytes x8 EEPROM (DIP8)
          M6253     - OKI M6253 (DIP18)
          3771      - Fujitsu MB3771 Master Reset IC (DIP8)
          PC910     - Sharp PC910 opto-isolator (x2, DIP8)
          DSW1/2/3  - 8-position Dip Switch (x3)
          J1        - Jumper, set to 2-3
          J2, J3    - Jumper, both set to 1-2
          CN1       - 50 pin connector (joins to control panel assembly)
          CN2       - 26 pin connector (joins to foot pedal assembly)
          CN3       - 10 pin connector for power input
          CN4       - 6 pin connector (joins to sound PCB -> CN2, used for sound communication from Main PCB to Sound PCB)
          CN5       - 12 pin connector for input/output controls
          CN6       - 12 pin connector (joins to Motor PCB)
          TL1       - Connector for network optical cable link
          SW7       - Push Button Service Switch
          SW6       - Push Button Test Switch
          SW5, SW4  - Push Button Switches (purpose unknown)

***************************************************************************/

#include "emu.h"
#include "model1io.h"
#include "cpu/z80/z80.h"
#include "machine/msm6253.h"
#include "machine/315_5338a.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SEGA_MODEL1IO, model1io_device, "model1io", "Sega Model 1 I/O Board")

//-------------------------------------------------
// mem_map - z80 memory map
//-------------------------------------------------

void model1io_device::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x5fff).ram();
	map(0x8000, 0x800f).rw("io", FUNC(sega_315_5338a_device::read), FUNC(sega_315_5338a_device::write));
	map(0xc000, 0xc003).rw("adc", FUNC(msm6253_device::d0_r), FUNC(msm6253_device::address_w));
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( model1io )
	PORT_START("buttons")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Board 0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Board 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Board 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Board 3")
INPUT_PORTS_END

ioport_constructor model1io_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(model1io);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( model1io )
	ROM_REGION(0x10000, "iocpu", 0)
	ROM_DEFAULT_BIOS("epr14869")

	// Virtua Racing (837-8950-01)
	ROM_SYSTEM_BIOS(0, "epr14869", "EPR-14869")
	ROMX_LOAD("epr-14869.25", 0x0000, 0x10000, CRC(6187cd7a) SHA1(b65fdd0ad31794a565a0ca4dc67a3f16b329fd71), ROM_BIOS(1))

	// Virtua Fighter (837-8936)
	ROM_SYSTEM_BIOS(1, "epr14869b", "EPR-14869B")
	ROMX_LOAD("epr-14869b.25", 0x0000, 0x10000, BAD_DUMP CRC(b410f22b) SHA1(75c5009ca4d21ebb53d54d4e3fb8aa55a4c74a07), ROM_BIOS(2)) // stray FFs at xx49, xx5F, xxC9, xxDF

	// Daytona USA (837-10539)
	ROM_SYSTEM_BIOS(2, "epr14869c", "EPR-14869C")
	ROMX_LOAD("epr-14869c.25", 0x0000, 0x10000, CRC(24b68e64) SHA1(c19d044d4c2fe551474492aa51922587394dd371), ROM_BIOS(3))
ROM_END

const tiny_rom_entry *model1io_device::device_rom_region() const
{
	return ROM_NAME(model1io);
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START( model1io_device::device_add_mconfig )
	MCFG_CPU_ADD("iocpu", Z80, 32_MHz_XTAL/8)
	MCFG_CPU_PROGRAM_MAP(mem_map)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom") // 93C45

	MCFG_DEVICE_ADD("io", SEGA_315_5338A, 0)
	MCFG_315_5338A_READ_CB(READ8(model1io_device, io_r))
	MCFG_315_5338A_WRITE_CB(WRITE8(model1io_device, io_w))
	MCFG_315_5338A_OUT0_CB(WRITE8(model1io_device, out0_w))
	MCFG_315_5338A_IN1_CB(READ8(model1io_device, in1_r))
	MCFG_315_5338A_IN2_CB(READ8(model1io_device, in2_r))
	MCFG_315_5338A_IN3_CB(READ8(model1io_device, in3_r))
	MCFG_315_5338A_OUT5_CB(WRITE8(model1io_device, out5_w))
	MCFG_315_5338A_IN6_CB(READ8(model1io_device, in6_r))

	MCFG_DEVICE_ADD("adc", MSM6253, 0)
	MCFG_MSM6253_IN0_ANALOG_READ(model1io_device, analog0_r)
	MCFG_MSM6253_IN1_ANALOG_READ(model1io_device, analog1_r)
	MCFG_MSM6253_IN2_ANALOG_READ(model1io_device, analog2_r)
	MCFG_MSM6253_IN3_ANALOG_READ(model1io_device, analog3_r)
MACHINE_CONFIG_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m1io_device - constructor
//-------------------------------------------------

model1io_device::model1io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGA_MODEL1IO, tag, owner, clock),
	m_eeprom(*this, "eeprom"),
	m_buttons(*this, "buttons"),
	m_read_cb(*this), m_write_cb(*this),
	m_in_cb{ {*this}, {*this}, {*this}, {*this}, {*this}, {*this} },
	m_an_cb{ {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this} },
	m_output_cb(*this),
	m_secondary_controls(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void model1io_device::device_start()
{
	// resolve callbacks
	m_read_cb.resolve_safe(0xff);
	m_write_cb.resolve_safe();

	for (unsigned i = 0; i < 6; i++)
		m_in_cb[i].resolve_safe(0xff);

	for (unsigned i = 0; i < 8; i++)
		m_an_cb[i].resolve_safe(0xff);

	m_output_cb.resolve_safe();
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

READ8_MEMBER( model1io_device::io_r )
{
	return m_read_cb(offset);
}

WRITE8_MEMBER( model1io_device::io_w )
{
	m_write_cb(offset, data, 0xff);
}

WRITE8_MEMBER( model1io_device::out0_w )
{
	// 7-------  eeprom clk
	// -6------  eeprom cs
	// --5-----  eeprom di
	// ---4----  eeprom related (0 on reads, 1 on writes)
	// ----32--  unknown (not used?)
	// ------1-  led? set to 1 in startup, after eeprom written to ram
	// -------0  control panel switch (0 = first, 1 = second)

	m_eeprom->clk_write(BIT(data, 7) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write(BIT(data, 5));
	m_eeprom->cs_write(BIT(data, 6) ? ASSERT_LINE : CLEAR_LINE);

	m_secondary_controls = bool(BIT(data, 0));
}

READ8_MEMBER( model1io_device::in1_r )
{
	return m_secondary_controls ? m_in_cb[3](0) : m_in_cb[0](0);
}

READ8_MEMBER( model1io_device::in2_r )
{
	return m_secondary_controls ? m_in_cb[4](0) : m_in_cb[1](0);
}

READ8_MEMBER( model1io_device::in3_r )
{
	return m_secondary_controls ? m_in_cb[5](0) : m_in_cb[2](0);
}

WRITE8_MEMBER( model1io_device::out5_w )
{
	m_output_cb(data);
}

READ8_MEMBER( model1io_device::in6_r )
{
	// 7-------  eeprom do
	// -654----  unknown
	// ----3---  button board 3
	// -----2--  button board 2
	// ------1-  button board 1
	// -------0  button board 0

	uint8_t data = 0;

	data |= m_eeprom->do_read() << 7;
	data |= 0x70;
	data |= m_buttons->read();

	return data;
}

ioport_value model1io_device::analog0_r()
{
	return m_secondary_controls ? m_an_cb[4](0) : m_an_cb[0](0);
}

ioport_value model1io_device::analog1_r()
{
	return m_secondary_controls ? m_an_cb[5](0) : m_an_cb[1](0);
}

ioport_value model1io_device::analog2_r()
{
	return m_secondary_controls ? m_an_cb[6](0) : m_an_cb[2](0);
}

ioport_value model1io_device::analog3_r()
{
	return m_secondary_controls ? m_an_cb[7](0) : m_an_cb[3](0);
}
