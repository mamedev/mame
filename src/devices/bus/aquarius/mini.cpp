// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Aquarius Mini Expander

**********************************************************************/


#include "emu.h"
#include "mini.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AQUARIUS_MINI, aquarius_mini_device, "aquarius_mini", "Aquarius Mini Expander")


//-------------------------------------------------
//  INPUT_PORTS( mini )
//-------------------------------------------------

static INPUT_PORTS_START(mini)
	PORT_START("RIGHT")
	PORT_BIT( 0x001000, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(1) PORT_NAME("P1 12:00 (Up)") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1fb)
	PORT_BIT( 0x002000, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(1) PORT_NAME("P1 01:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1eb)
	PORT_BIT( 0x004000, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(1) PORT_NAME("P1 01:30") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1e9)
	PORT_BIT( 0x008000, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(1) PORT_NAME("P1 02:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1f9)
	PORT_BIT( 0x000001, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(1) PORT_NAME("P1 03:00 (Right)") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1fd)
	PORT_BIT( 0x000002, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(1) PORT_NAME("P1 04:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1ed)
	PORT_BIT( 0x000004, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(1) PORT_NAME("P1 04:30") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1ec)
	PORT_BIT( 0x000008, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(1) PORT_NAME("P1 05:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1fc)
	PORT_BIT( 0x000010, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(1) PORT_NAME("P1 06:00 (Down)") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1fe)
	PORT_BIT( 0x000020, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(1) PORT_NAME("P1 06:30") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1ee)
	PORT_BIT( 0x000040, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(1) PORT_NAME("P1 07:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1e6)
	PORT_BIT( 0x000080, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(1) PORT_NAME("P1 08:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1f6)
	PORT_BIT( 0x000100, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(1) PORT_NAME("P1 09:00 (Left)") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1f7)
	PORT_BIT( 0x000200, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(1) PORT_NAME("P1 09:30") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1e7)
	PORT_BIT( 0x000400, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(1) PORT_NAME("P1 10:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1e3)
	PORT_BIT( 0x000800, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(1) PORT_NAME("P1 11:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1f3)
	PORT_BIT( 0x010000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1bf)
	PORT_BIT( 0x020000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x17b)
	PORT_BIT( 0x040000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x15f)
	PORT_BIT( 0x080000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x1df)
	PORT_BIT( 0x100000, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x17d)
	PORT_BIT( 0x200000, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x17e)

	PORT_START("LEFT")
	PORT_BIT( 0x001000, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(2) PORT_NAME("P2 12:00 (Up)") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0fb)
	PORT_BIT( 0x002000, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(2) PORT_NAME("P2 01:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0eb)
	PORT_BIT( 0x004000, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(2) PORT_NAME("P2 01:30") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0e9)
	PORT_BIT( 0x008000, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(2) PORT_NAME("P2 02:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0f9)
	PORT_BIT( 0x000001, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(2) PORT_NAME("P2 03:00 (Right)") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0fd)
	PORT_BIT( 0x000002, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(2) PORT_NAME("P2 04:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0ed)
	PORT_BIT( 0x000004, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(2) PORT_NAME("P2 04:30") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0ec)
	PORT_BIT( 0x000008, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(2) PORT_NAME("P2 05:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0fc)
	PORT_BIT( 0x000010, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(2) PORT_NAME("P2 06:00 (Down)") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0fe)
	PORT_BIT( 0x000020, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(2) PORT_NAME("P2 06:30") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0ee)
	PORT_BIT( 0x000040, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(2) PORT_NAME("P2 07:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0e6)
	PORT_BIT( 0x000080, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(2) PORT_NAME("P2 08:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0f6)
	PORT_BIT( 0x000100, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(2) PORT_NAME("P2 09:00 (Left)") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0f7)
	PORT_BIT( 0x000200, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(2) PORT_NAME("P2 09:30") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0e7)
	PORT_BIT( 0x000400, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(2) PORT_NAME("P2 10:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0e3)
	PORT_BIT( 0x000800, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(2) PORT_NAME("P2 11:00") PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0f3)
	PORT_BIT( 0x010000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0bf)
	PORT_BIT( 0x020000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x07b)
	PORT_BIT( 0x040000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x05f)
	PORT_BIT( 0x080000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x0df)
	PORT_BIT( 0x100000, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x07d)
	PORT_BIT( 0x200000, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aquarius_mini_device::input_changed), 0x07e)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor aquarius_mini_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mini);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void aquarius_mini_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set(DEVICE_SELF_OWNER, FUNC(aquarius_cartridge_slot_device::irq_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay, DERIVED_CLOCK(1, 2));
	m_ay->port_a_read_callback().set([this] () { return m_ctrl_input[0]; });
	m_ay->port_b_read_callback().set([this] () { return m_ctrl_input[1]; });
	m_ay->add_route(ALL_OUTPUTS, "mono", 0.25);

	/* cartridge */
	AQUARIUS_CARTRIDGE_SLOT(config, m_exp[0], DERIVED_CLOCK(1,1), aquarius_cartridge_devices, nullptr);
	m_exp[0]->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));
	m_exp[0]->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(aquarius_cartridge_slot_device::nmi_w));

	AQUARIUS_CARTRIDGE_SLOT(config, m_exp[1], DERIVED_CLOCK(1,1), aquarius_cartridge_devices, nullptr);
	m_exp[1]->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_exp[1]->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(aquarius_cartridge_slot_device::nmi_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  aquarius_mini_device - constructor
//-------------------------------------------------

aquarius_mini_device::aquarius_mini_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AQUARIUS_MINI, tag, owner, clock)
	, device_aquarius_cartridge_interface(mconfig, *this)
	, m_irqs(*this, "irqs")
	, m_exp(*this, "exp%u", 1U)
	, m_ay(*this, "ay8910")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void aquarius_mini_device::device_start()
{
	m_ctrl_input[0] = 0xff;
	m_ctrl_input[1] = 0xff;

	save_item(NAME(m_ctrl_input));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

INPUT_CHANGED_MEMBER(aquarius_mini_device::input_changed)
{
	if (newval)
		m_ctrl_input[BIT(param, 8)] &= param;
	else
		m_ctrl_input[BIT(param, 8)] |= ~param;
}

uint8_t aquarius_mini_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	data &= m_exp[0]->mreq_r(offset);
	data &= m_exp[1]->mreq_r(offset);

	return data;
}

void aquarius_mini_device::mreq_w(offs_t offset, uint8_t data)
{
	m_exp[0]->mreq_w(offset, data);
	m_exp[1]->mreq_w(offset, data);
}


uint8_t aquarius_mini_device::mreq_ce_r(offs_t offset)
{
	uint8_t data = 0xff;

	data &= m_exp[0]->mreq_ce_r(offset);
	data &= m_exp[1]->mreq_ce_r(offset);

	return data;
}

void aquarius_mini_device::mreq_ce_w(offs_t offset, uint8_t data)
{
	m_exp[0]->mreq_ce_w(offset, data);
	m_exp[1]->mreq_ce_w(offset, data);
}


uint8_t aquarius_mini_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 0xff)
	{
	case 0xf6:
		data &= m_ay->data_r();
		break;
	case 0xf7:
		break;
	}

	data &= m_exp[0]->iorq_r(offset);
	data &= m_exp[1]->iorq_r(offset);

	return data;
}

void aquarius_mini_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xff)
	{
	case 0xf6:
		m_ay->data_w(data);
		break;
	case 0xf7:
		m_ay->address_w(data);
		break;
	}

	m_exp[0]->iorq_w(offset, data);
	m_exp[1]->iorq_w(offset, data);
}
