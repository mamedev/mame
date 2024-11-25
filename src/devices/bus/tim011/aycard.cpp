// license:BSD-3-Clause
// copyright-holders:Marko Solajic, Miodrag Milanovic
/**********************************************************************

    TIM-011 AY Card

    Card created by Zoran Urosevic.

**********************************************************************/

#include "emu.h"
#include "aycard.h"
#include "speaker.h"
#include "sound/ay8910.h"

namespace {

static INPUT_PORTS_START(joystick)
	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON2)
INPUT_PORTS_END

class tim011_aycard_device :
	public device_t,
	public bus::tim011::device_exp_interface
{
public:
	// construction/destruction
	tim011_aycard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(joystick); }

private:
	required_device<ay8910_device> m_psg;
};


tim011_aycard_device::tim011_aycard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TIM011_AYCARD, tag, owner, clock)
	, bus::tim011::device_exp_interface(mconfig, *this)
	, m_psg(*this, "ay8912")
{
}

/*----------------------------------
  device_t implementation
----------------------------------*/

void tim011_aycard_device::device_start()
{
}

void tim011_aycard_device::device_reset()
{
	m_slot->m_io->install_write_handler(0x00f4, 0x00f4, emu::rw_delegate(m_psg, FUNC(ay8910_device::data_w)));
	m_slot->m_io->install_read_handler (0x00fc, 0x00fc, emu::rw_delegate(m_psg, FUNC(ay8910_device::data_r)));
	m_slot->m_io->install_write_handler(0x00fc, 0x00fc, emu::rw_delegate(m_psg, FUNC(ay8910_device::address_w)));
}

void tim011_aycard_device::device_add_mconfig(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8912(config, m_psg, 1.8432_MHz_XTAL);
	m_psg->add_route(ALL_OUTPUTS, "mono", 0.25);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(TIM011_AYCARD, bus::tim011::device_exp_interface, tim011_aycard_device, "ay", "AY card")
