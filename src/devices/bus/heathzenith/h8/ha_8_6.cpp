// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit HA-8-6 Z80 CPU board


  Onboard Jumpers

  Common   To   Installed  Emulated  Description (if jumper is closed)
  -------------------------------------------------------------------------
    E2     E1     Yes       Fixed    On board clock
           E3     No        No       External clock

    E4     E5     Yes       Fixed    0k to 4k ROM select
          Open    No        No       0k to 8k ROM select

    E7     E6     Yes       Fixed    Normal bus O2
           E8     No        No       Offset bus O2
    E10    E9     Yes       Fixed    Normal bus O2
           E8     No        No       Offset bus O2


    E14    E13    Yes       Fixed    On board reset
           E12    No        No       External reset

    E15    E16    Yes       No       CS2 to CS1
           E17    No        No       CS2 to +5V

    E20    E18    No        No       External O2
           E19    Yes       No       On board

    E23    E21    Yes       No       Reset to bus
           E22    No        No       Reset from bus

    E24    E25    No        No       CS2 to CS1
           E26    No        No       CS2 to +5V

    E28    E29    Yes       No       /INTA
           E27    No        No       /INTA option

    E32    E31    No        No       NMI Option
           E30    No        No       NMI Option

    E33    E34    No        No       INT 0 Option

    E35     -     No        No       Halt Option

    E36     -     No        No       INTA Option

    E38     -     No        No       Bus Pin 18

    E39     -     No        No       Bus Pin 9

    E40     -     No        No       Refresh Option

    E41     -     No        Yes      Int 20 Option

    E42     -     No        No       External Disable

    E43     -     No        No       Int 0 Option

    E44     -     No        No       Bus Pin 8

    E45     -     No        Yes      Int 10 Option

    E46     -     No        No       Side Select Option

    E47     -     No        No       NMI Option

    E48    E50    No        No       Normal Write

    E49    E51    Yes       No       Early Write

****************************************************************************/


#include "emu.h"

#include "ha_8_6.h"

#include "bus/heathzenith/intr_cntrl/intr_cntrl.h"
#include "cpu/z80/z80.h"

#define LOG_ORG0 (1U << 1)    // Shows register setup

#define VERBOSE (0)

#include "logmacro.h"

#define LOGORG0(...)        LOGMASKED(LOG_ORG0, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


namespace {

class ha_8_6_device : public device_t
					, public device_h8bus_card_interface
					, public device_p201_p2_card_interface
{
public:

	ha_8_6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void int1_w(int state) override;
	virtual void int2_w(int state) override;
	virtual void int3_w(int state) override;
	virtual void int4_w(int state) override;
	virtual void int5_w(int state) override;
	virtual void int6_w(int state) override;
	virtual void int7_w(int state) override;
	virtual void rom_disable_w(int state) override;

	virtual void p201_reset_w(int state) override;
	virtual void p201_int1_w(int state) override;
	virtual void p201_int2_w(int state) override;

protected:

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void map_io(address_space_installer & space) override ATTR_COLD;

	void handle_int1();
	void handle_int2();

	u8 sys_rom_r(offs_t offset);

	u8 portf2_r();
	void portf2_w(u8 data);

	void update_gpp(u8 data);

	void map_fetch(address_map &map);
	u8 m1_r(offs_t offset);

	bool m_installed;
	u8   m_gpp;

	bool m_m1_state;
	bool m_allow_bus_int1;
	bool m_allow_bus_int2;
	bool m_p201_int1;
	bool m_p201_int2;
	bool m_bus_int1;
	bool m_bus_int2;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device>        m_maincpu;
	required_device<heath_intr_socket> m_intr_socket;
	required_region_ptr<u8>            m_sys_rom;
	memory_view                        m_mem_view;
	required_ioport                    m_sw1;
	required_ioport                    m_config;

	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_mem;
};

ha_8_6_device::ha_8_6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, H8BUS_HA_8_6, tag, owner, 0)
	, device_h8bus_card_interface(mconfig, *this)
	, device_p201_p2_card_interface(*this, tag)
	, m_maincpu(*this, "maincpu")
	, m_intr_socket(*this, "intr_socket")
	, m_sys_rom(*this, "maincpu")
	, m_mem_view(*this, "mem")
	, m_sw1(*this, "SW1")
	, m_config(*this, "CONFIG")
{
}

void ha_8_6_device::int1_w(int state)
{
	if (m_allow_bus_int1)
	{
		m_bus_int1 = bool(state);

		handle_int1();
	}
}

void ha_8_6_device::int2_w(int state)
{
	if (m_allow_bus_int2)
	{
		m_bus_int2 = bool(state);

		handle_int2();
	}
}

void ha_8_6_device::int3_w(int state)
{
	m_intr_socket->set_irq_level(3, state);
}

void ha_8_6_device::int4_w(int state)
{
	m_intr_socket->set_irq_level(4, state);
}

void ha_8_6_device::int5_w(int state)
{
	m_intr_socket->set_irq_level(5, state);
}

void ha_8_6_device::int6_w(int state)
{
	m_intr_socket->set_irq_level(6, state);
}

void ha_8_6_device::int7_w(int state)
{
	m_intr_socket->set_irq_level(7, state);
}

void ha_8_6_device::p201_reset_w(int state)
{
	if (state)
	{
		m_maincpu->reset();
	}

	set_slot_reset(state);
}

void ha_8_6_device::p201_int1_w(int state)
{
	m_p201_int1 = bool(state);

	handle_int1();
}

void ha_8_6_device::p201_int2_w(int state)
{
	m_p201_int2 = bool(state);

	handle_int2();
}

void ha_8_6_device::handle_int1()
{
	m_intr_socket->set_irq_level(1, (m_p201_int1 || m_bus_int1) ? ASSERT_LINE : CLEAR_LINE);

}

void ha_8_6_device::handle_int2()
{
	m_intr_socket->set_irq_level(2, (m_p201_int2 || m_bus_int2) ? ASSERT_LINE : CLEAR_LINE);
}


static void intr_ctrl_options(device_slot_interface &device)
{
	device.option_add("original", HEATH_INTR_CNTRL);
}

u8 ha_8_6_device::sys_rom_r(offs_t offset)
{
	return m_sys_rom[offset & 0xfff];
}

u8 ha_8_6_device::portf2_r()
{
	u8 sw1 = m_sw1->read();

	return sw1;
}

void ha_8_6_device::portf2_w(u8 data)
{
	if (data != m_gpp)
	{
		update_gpp(data);
	}
}

void ha_8_6_device::update_gpp(u8 data)
{
	u8 changed_gpp = data ^ m_gpp;

	m_gpp = data;

	if (BIT(changed_gpp, 5))
	{
		int state = BIT(m_gpp, 5);

		LOGORG0("%s: updating gpp: %d\n", FUNCNAME, state);

		set_slot_rom_disable(state);
	}
}

void ha_8_6_device::rom_disable_w(int state)
{
	LOGORG0("%s: mem_view %d\n", FUNCNAME, (state ? 1 : 0));

	m_mem_view.select(state ? 1 : 0);
}

void ha_8_6_device::mem_map(address_map &map)
{
	map.unmap_value_low();

	map(0x0000, 0xffff).view(m_mem_view);
}

void ha_8_6_device::io_map(address_map &map)
{
	map.unmap_value_low();
	map.global_mask(0xff);
}

void ha_8_6_device::map_io(address_space_installer & space)
{
	space.install_readwrite_handler(0xf2, 0xf2,
		read8smo_delegate(*this, FUNC(ha_8_6_device::portf2_r)),
		write8smo_delegate(*this, FUNC(ha_8_6_device::portf2_w))
	);
}

// ROM definition
ROM_START( ha_8_6 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("xcon8")

	ROM_SYSTEM_BIOS(0, "xcon8", "ROM supporting ORG0 for H17 and H47")
	ROMX_LOAD( "2732_444-70_xcon8.rom", 0x0000, 0x1000,  CRC(b04368f4) SHA1(965244277a3a8039a987e4c3593b52196e39b7e7), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "pam37", "ROM supporting H17, H37, H47, and H67")
	ROMX_LOAD( "2732_444-140_pam37.rom", 0x0000, 0x1000, CRC(53a540db) SHA1(90082d02ffb1d27e8172b11fff465bd24343486e), ROM_BIOS(1) )
ROM_END

static INPUT_PORTS_START( ha_8_6_jumpers )

	PORT_START("SW1")
	// Generic definition
	PORT_DIPNAME( 0x01, 0x00, "Switch 0" )                        PORT_DIPLOCATION("SW1:1")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Switch 1" )                        PORT_DIPLOCATION("SW1:2")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Switch 2" )                        PORT_DIPLOCATION("SW1:3")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Switch 3" )                        PORT_DIPLOCATION("SW1:4")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Switch 4" )                        PORT_DIPLOCATION("SW1:5")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Switch 5" )                        PORT_DIPLOCATION("SW1:6")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Switch 6" )                        PORT_DIPLOCATION("SW1:7")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Switch 7" )                        PORT_DIPLOCATION("SW1:8")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_DIPNAME( 0x01, 0x00, "Port 174 device" )                 PORT_DIPLOCATION("SW1:1")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x04)
	PORT_DIPSETTING(    0x00, "H17" )
	PORT_DIPSETTING(    0x01, "H47" )
	PORT_DIPNAME( 0x02, 0x00, "(Set At Zero)" )                   PORT_DIPLOCATION("SW1:2")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x04)
	PORT_DIPSETTING(    0x00, "Undefined" )
	PORT_DIPSETTING(    0x02, "Undefined" )
	PORT_DIPNAME( 0x04, 0x00, "Port 170 device" )                 PORT_DIPLOCATION("SW1:3")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x04)
	PORT_DIPSETTING(    0x00, "Not in use" )
	PORT_DIPSETTING(    0x04, "H47" )
	PORT_DIPNAME( 0x08, 0x00, "(Set At Zero)" )                   PORT_DIPLOCATION("SW1:4")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x04)
	PORT_DIPSETTING(    0x00, "Undefined" )
	PORT_DIPSETTING(    0x08, "Undefined" )
	PORT_DIPNAME( 0x10, 0x10, "Primary Boot from" )               PORT_DIPLOCATION("SW1:5")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x04)
	PORT_DIPSETTING(    0x00, "Boots from device at port 174" )
	PORT_DIPSETTING(    0x10, "Boots from device at port 170" )
	PORT_DIPNAME( 0x20, 0x20, "(Set At Zero)" )                   PORT_DIPLOCATION("SW1:6")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x04)
	PORT_DIPSETTING(    0x00, "Undefined" )
	PORT_DIPSETTING(    0x20, "Undefined" )
	PORT_DIPNAME( 0x40, 0x00, "(Set At Zero)" )                   PORT_DIPLOCATION("SW1:7")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x04)
	PORT_DIPSETTING(    0x00, "Undefined" )
	PORT_DIPSETTING(    0x40, "Undefined" )
	PORT_DIPNAME( 0x80, 0x00, "Boot mode" )                       PORT_DIPLOCATION("SW1:8")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x04)
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, "Auto" )

	PORT_DIPNAME( 0x03, 0x00, "Port 174 device" )                 PORT_DIPLOCATION("SW1:1,2")     PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, "H17" )
	PORT_DIPSETTING(    0x01, "H47" )
	PORT_DIPSETTING(    0x02, "H67" )
	PORT_DIPSETTING(    0x03, "Undefined" )
	PORT_DIPNAME( 0x0c, 0x00, "Port 170 device" )                 PORT_DIPLOCATION("SW1:3,4")     PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, "H37" )
	PORT_DIPSETTING(    0x04, "H47" )
	PORT_DIPSETTING(    0x08, "H67" )
	PORT_DIPSETTING(    0x0c, "Undefined" )
	PORT_DIPNAME( 0x10, 0x10, "Primary Boot from" )               PORT_DIPLOCATION("SW1:5")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, "Boots from device at port 174" )
	PORT_DIPSETTING(    0x10, "Boots from device at port 170" )
	PORT_DIPNAME( 0x20, 0x20, "(Set At Zero)" )                   PORT_DIPLOCATION("SW1:6")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, "Undefined" )
	PORT_DIPSETTING(    0x20, "Undefined" )
	PORT_DIPNAME( 0x40, 0x00, "(Set At Zero)" )                   PORT_DIPLOCATION("SW1:7")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, "Undefined" )
	PORT_DIPSETTING(    0x40, "Undefined" )
	PORT_DIPNAME( 0x80, 0x00, "Boot mode" )                       PORT_DIPLOCATION("SW1:8")       PORT_CONDITION("CONFIG", 0x1c, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, "Auto" )

	PORT_START("CONFIG")
	PORT_CONFNAME(0x01, 0x00, "Allow INT1 signal on BH Bus - Jumper E45" )
	PORT_CONFSETTING(   0x00, DEF_STR( No ) )
	PORT_CONFSETTING(   0x01, DEF_STR( Yes ) )
	PORT_CONFNAME(0x02, 0x00, "Allow INT2 signal on BH Bus - Jumper E41" )
	PORT_CONFSETTING(   0x00, DEF_STR( No ) )
	PORT_CONFSETTING(   0x02, DEF_STR( Yes ) )

	PORT_CONFNAME(0x1c, 0x08, "Switch SW1 Definitions" )
	PORT_CONFSETTING(   0x00, "Generic" )
	PORT_CONFSETTING(   0x04, "Heath XCON8" )
	PORT_CONFSETTING(   0x08, "Heath PAM37" )

INPUT_PORTS_END

ioport_constructor ha_8_6_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ha_8_6_jumpers);
}

const tiny_rom_entry *ha_8_6_device::device_rom_region() const
{
	return ROM_NAME(ha_8_6);
}

void ha_8_6_device::map_fetch(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(ha_8_6_device::m1_r));
}

u8 ha_8_6_device::m1_r(offs_t offset)
{
	u8 data = m_mem.read_byte(offset);

	if (!machine().side_effects_disabled())
	{
		set_slot_m1(ASSERT_LINE);

		if (data == 0xfb) // Check for EI instruction
		{
			m_p201_inte(ASSERT_LINE);
		}
		else if (data == 0xf3) // Check for DI instruction
		{
			m_p201_inte(CLEAR_LINE);
		}

		set_slot_m1(CLEAR_LINE);
	}

	return data;
}

void ha_8_6_device::device_start()
{
	save_item(NAME(m_installed));
	save_item(NAME(m_gpp));
	save_item(NAME(m_m1_state));
	save_item(NAME(m_p201_int1));
	save_item(NAME(m_p201_int2));
	save_item(NAME(m_bus_int1));
	save_item(NAME(m_bus_int2));

	h8bus().set_clock(m_maincpu->clock());
}

void ha_8_6_device::device_reset()
{
	ioport_value const config(m_config->read());

	m_allow_bus_int1 = bool(BIT(config, 0));
	m_allow_bus_int2 = bool(BIT(config, 1));

	m_maincpu->space(AS_PROGRAM).specific(m_mem);

	h8bus().map_mem(m_mem_view[0]);
	h8bus().map_mem(m_mem_view[1]);
	m_mem_view[0].install_read_handler(0x0000, 0x0fff, read8sm_delegate(*this, FUNC(ha_8_6_device::sys_rom_r)));

	m_mem_view.select(0);
	LOGORG0("%s: mem_view 0\n", FUNCNAME);

	h8bus().map_io(m_maincpu->space(AS_IO));


	m_m1_state  = false;
	m_p201_int1 = false;
	m_p201_int2 = false;
	m_bus_int1  = false;
	m_bus_int2  = false;
}

void ha_8_6_device::device_add_mconfig(machine_config &config)
{
	constexpr XTAL H8_CLOCK = XTAL(18'432'000) / 9;  // 2.048 MHz

	Z80(config, m_maincpu, H8_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &ha_8_6_device::mem_map);
	m_maincpu->set_addrmap(AS_IO, & ha_8_6_device::io_map);
	m_maincpu->set_m1_map(&ha_8_6_device::map_fetch);
	m_maincpu->set_irq_acknowledge_callback(m_intr_socket, FUNC(heath_intr_socket::irq_callback));

	HEATH_INTR_SOCKET(config, m_intr_socket, intr_ctrl_options, nullptr);
	m_intr_socket->irq_line_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_intr_socket->set_default_option("original");
	m_intr_socket->set_fixed(true);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_HA_8_6, device_h8bus_card_interface, ha_8_6_device, "h8_ha_8_6", "Heath Z80 CPU board");
