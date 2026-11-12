// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Game Arts Dual Joyport / ゲームアーツ デュアル ジョイポート

Released for pc88va:famista, splits a single DE-9 MSX port into 2 ports.
Protocol expects pin 3~4 (left/right) to be pulled down (0) as ID, then clocks pins 6~7,
read pin 0 for port 1 and pin 1 for port 2.
Should be PC-8801, MSX and X68000 compatible.

TODO:
- pins 6~7 may be reversed (game just sync both via 3 -> 0 strobes)
- any other SW that supports this, potentially for 4 players?
- famista manual, mentioned on the back of the box;

**************************************************************************************************/

#include "emu.h"
#include "dual_joyport.h"

#include "input.h"

namespace {

class gamearts_dual_joyport_device : public device_t, public device_msx_general_purpose_port_interface
{
public:
	gamearts_dual_joyport_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read() override;
	virtual void pin_6_w(int state) override;
	virtual void pin_7_w(int state) override;
	virtual void pin_8_w(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device_array<msx_general_purpose_port_device, 2> m_port;

	int m_pin6;
	int m_pin7;
	int m_pin8;
	u8 m_clk[2];
	u8 m_in[2];
};

gamearts_dual_joyport_device::gamearts_dual_joyport_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_DUAL_JOYPORT, tag, owner, clock)
	, device_msx_general_purpose_port_interface(mconfig, *this)
	, m_port(*this, "ctrl_%u", 1U)
{
}

void gamearts_dual_joyport_device::device_start()
{
	save_item(NAME(m_pin6));
	save_item(NAME(m_pin7));
	save_item(NAME(m_pin8));
	save_item(NAME(m_clk));
	save_item(NAME(m_in));
}

void gamearts_dual_joyport_device::device_add_mconfig(machine_config &config)
{
	MSX_GENERAL_PURPOSE_PORT(config, m_port[0], msx_general_purpose_port_devices, "joystick");
	MSX_GENERAL_PURPOSE_PORT(config, m_port[1], msx_general_purpose_port_devices, "joystick");
}

u8 gamearts_dual_joyport_device::read()
{
	u8 res = 0x30;

	res |= m_clk[0] >= 8 ? 1 : BIT(m_in[0], 7 - m_clk[0]) << 0;
	res |= m_clk[1] >= 8 ? 2 : BIT(m_in[1], 7 - m_clk[1]) << 1;

	return res;
}

void gamearts_dual_joyport_device::pin_6_w(int state)
{
	if (m_pin6 != state && !state)
	{
		m_clk[0] ++;
	}
	m_pin6 = state;

}

void gamearts_dual_joyport_device::pin_7_w(int state)
{
	if (m_pin7 != state && !state)
	{
		m_clk[1] ++;
	}
	m_pin7 = state;
}

void gamearts_dual_joyport_device::pin_8_w(int state)
{
	if (m_pin8 != state && !state)
	{
		m_clk[0] = m_clk[1] = 0;
		for (int i = 0; i < 2; i++)
		{
			m_in[i] = m_port[i]->read() & 0xf;
			m_in[i] |= (m_port[i]->read() & 0x30) << 2;
			m_in[i] |= 0x30; // +5V and GND?
		}
	}
	m_pin8 = state;
}


} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_DUAL_JOYPORT, device_msx_general_purpose_port_interface, gamearts_dual_joyport_device, "msx_dual_joyport", "MSX Game Arts Dual Joyport Adapter")
