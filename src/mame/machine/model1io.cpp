// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sega Model 1 I/O Board


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

static INPUT_PORTS_START( ioboard_buttons )
	PORT_START("buttons")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Board 0 (SW4)")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Board 1 (SW5)")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Board 2 (SW6)")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Board 3 (SW7)")
INPUT_PORTS_END

ioport_constructor model1io_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ioboard_buttons);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( model1io )
	ROM_REGION(0x10000, "iocpu", 0)
	ROM_DEFAULT_BIOS("epr14869")

	// Virtua Racing (837-8950-01)
	ROM_SYSTEM_BIOS(0, "epr14869", "EPR-14869")
	ROMX_LOAD("epr-14869.25", 0x0000, 0x10000, CRC(6187cd7a) SHA1(b65fdd0ad31794a565a0ca4dc67a3f16b329fd71), ROM_BIOS(0))

	// Virtua Fighter (837-8936), Star Wars Arcade
	ROM_SYSTEM_BIOS(1, "epr14869b", "EPR-14869B")
	ROMX_LOAD("epr-14869b.25", 0x0000, 0x10000, CRC(2d093304) SHA1(af0fe245eb9fa3c3c60e4b685f1e779f83d894f9), ROM_BIOS(1))

	// Daytona USA (837-10539)
	ROM_SYSTEM_BIOS(2, "epr14869c", "EPR-14869C")
	ROMX_LOAD("epr-14869c.25", 0x0000, 0x10000, CRC(24b68e64) SHA1(c19d044d4c2fe551474492aa51922587394dd371), ROM_BIOS(2))
ROM_END

const tiny_rom_entry *model1io_device::device_rom_region() const
{
	return ROM_NAME(model1io);
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void  model1io_device::device_add_mconfig(machine_config &config)
{
	z80_device &iocpu(Z80(config, "iocpu", 32_MHz_XTAL/8));
	iocpu.set_addrmap(AS_PROGRAM, &model1io_device::mem_map);

	EEPROM_93C46_16BIT(config, m_eeprom); // 93C45

	sega_315_5338a_device &io(SEGA_315_5338A(config, "io", 32_MHz_XTAL));
	io.read_callback().set(FUNC(model1io_device::io_r));
	io.write_callback().set(FUNC(model1io_device::io_w));
	io.out_pa_callback().set(FUNC(model1io_device::io_pa_w));
	io.in_pb_callback().set(FUNC(model1io_device::io_pb_r));
	io.in_pc_callback().set(FUNC(model1io_device::io_pc_r));
	io.in_pd_callback().set(FUNC(model1io_device::io_pd_r));
	io.in_pe_callback().set(FUNC(model1io_device::io_pe_r));
	io.out_pe_callback().set(FUNC(model1io_device::io_pe_w));
	io.out_pf_callback().set(FUNC(model1io_device::io_pf_w));
	io.in_pg_callback().set(FUNC(model1io_device::io_pg_r));

	msm6253_device &adc(MSM6253(config, "adc", 0));
	adc.set_input_cb<0>(FUNC(model1io_device::analog0_r));
	adc.set_input_cb<1>(FUNC(model1io_device::analog1_r));
	adc.set_input_cb<2>(FUNC(model1io_device::analog2_r));
	adc.set_input_cb<3>(FUNC(model1io_device::analog3_r));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  model1io_device - constructor
//-------------------------------------------------

model1io_device::model1io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGA_MODEL1IO, tag, owner, clock),
	m_eeprom(*this, "eeprom"),
	m_buttons(*this, "buttons"),
	m_dsw(*this, "dsw%u", 1U),
	m_read_cb(*this), m_write_cb(*this),
	m_in_cb{ {*this}, {*this}, {*this} },
	m_drive_read_cb(*this), m_drive_write_cb(*this),
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

	for (unsigned i = 0; i < 3; i++)
		m_in_cb[i].resolve_safe(0xff);

	m_drive_read_cb.resolve_safe(0xff);
	m_drive_write_cb.resolve_safe();

	for (unsigned i = 0; i < 8; i++)
		m_an_cb[i].resolve_safe(0xff);

	m_output_cb.resolve_safe();

	// register for save states
	save_item(NAME(m_secondary_controls));
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

WRITE8_MEMBER( model1io_device::io_pa_w )
{
	// 7-------  eeprom clk
	// -6------  eeprom cs
	// --5-----  eeprom di
	// ---4----  eeprom pe
	// ----32--  not used
	// ------1-  led2
	// -------0  control switch (0 = first, 1 = second)

	m_eeprom->clk_write(BIT(data, 7) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write(BIT(data, 5));
	m_eeprom->cs_write(BIT(data, 6) ? ASSERT_LINE : CLEAR_LINE);

	m_secondary_controls = bool(BIT(data, 0));
}

READ8_MEMBER( model1io_device::io_pb_r )
{
	return m_secondary_controls ? m_dsw[0]->read() : m_in_cb[0](0);
}

READ8_MEMBER( model1io_device::io_pc_r )
{
	return m_secondary_controls ? m_dsw[1]->read() : m_in_cb[1](0);
}

READ8_MEMBER( model1io_device::io_pd_r )
{
	return m_secondary_controls ? m_dsw[2]->read() : m_in_cb[2](0);
}

READ8_MEMBER( model1io_device::io_pe_r )
{
	return m_drive_read_cb(0);
}

WRITE8_MEMBER( model1io_device::io_pe_w )
{
	m_drive_write_cb(data);
}

WRITE8_MEMBER( model1io_device::io_pf_w )
{
	m_output_cb(data);
}

READ8_MEMBER( model1io_device::io_pg_r )
{
	// 7-------  eeprom do
	// -6------  eeprom nc
	// --54----  not used
	// ----3---  button board 3 (sw7)
	// -----2--  button board 2 (sw6)
	// ------1-  button board 1 (sw5)
	// -------0  button board 0 (sw4)

	uint8_t data = 0;

	data |= m_eeprom->do_read() << 7;
	data |= 0x70;
	data |= m_buttons->read();

	return data;
}

// analog port switching is handled by two 74hc4066 analog switches
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
