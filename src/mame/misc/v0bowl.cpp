// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    "Fancy Bowling" (c) 200x A1 Amusement One

    hybrid bowling / video game

    VRender0+ SOC
    512MB compact flash (PCBs with ROMs instead of CF also exist, undumped)
    W29X040 boot ROM
    Atmel PLD (undumped)
    XTALs 25.175 mhz (VGA dot clock) and 1.8432 mhz (UART UCLK), plus the
    14.318 mhz the PLL takes as its reference.
    2x bank of 8 switches
    VGA and CGA on switch

    Reference videos:
    https://www.youtube.com/watch?v=Rp7XpDtvrPo <-- only video part
    https://www.youtube.com/watch?v=mTuqN-8WUz0 <-- also shows physical part

    TODO:
    - inputs / outputs

***************************************************************************/

#include "emu.h"

#include "bus/ata/atadev.h"
#include "bus/ata/ataintf.h"
#include "bus/rs232/rs232.h"
#include "cpu/se3208/se3208.h"
#include "machine/ds1307.h"
#include "machine/vrender0.h"
#include "sound/vrender0.h"
#include "video/vrender0.h"

#include "speaker.h"


namespace {

class v0bowl_state : public driver_device
{
public:
	v0bowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vr0soc(*this, "vr0soc")
		, m_rtc(*this, "rtc")
	{}

	void v0bowl(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void v0bowl_map(address_map &map) ATTR_COLD;

	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;
	required_device<i2c_ds1307_device> m_rtc;

	static constexpr int I2C_SCL_BIT = 28;
	static constexpr int I2C_SDA_BIT = 29;

	u32 m_pio_ddr = 0xffffffff;
	u32 m_pio_dat = 0;
	int m_rtc_sda = 1;

	void rtc_sda_w(int state) { m_rtc_sda = state; }

	void update_i2c()
	{
		m_rtc->scl_write(BIT(m_pio_ddr, I2C_SCL_BIT) ? 1 : BIT(m_pio_dat, I2C_SCL_BIT));
		m_rtc->sda_write(BIT(m_pio_ddr, I2C_SDA_BIT) ? 1 : BIT(m_pio_dat, I2C_SDA_BIT));
	}

	u32 piolddr_r();
	void piolddr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pioldat_r();
	void pioldat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pioedat_r();
};


u32 v0bowl_state::piolddr_r()
{
	return m_pio_ddr;
}

void v0bowl_state::piolddr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pio_ddr);
	update_i2c();
}

u32 v0bowl_state::pioldat_r()
{
	return m_pio_dat;
}

void v0bowl_state::pioldat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pio_dat);
	update_i2c();
}

u32 v0bowl_state::pioedat_r()
{
	u32 data = m_pio_dat | m_pio_ddr;

	if (!m_rtc_sda)
		data &= ~(1 << I2C_SDA_BIT);

	return data;
}


void v0bowl_state::v0bowl_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom();

	map(0x01500000, 0x01500003).portr("IN0");
	map(0x01500004, 0x01500007).portr("IN1");
	map(0x01500008, 0x0150000b).portr("IN2");

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
	map(0x01802000, 0x01802003).rw(FUNC(v0bowl_state::piolddr_r), FUNC(v0bowl_state::piolddr_w));
	map(0x01802004, 0x01802007).rw(FUNC(v0bowl_state::pioldat_r), FUNC(v0bowl_state::pioldat_w));
	map(0x01802008, 0x0180200b).r(FUNC(v0bowl_state::pioedat_r));

	map(0x02000000, 0x027fffff).ram().share("workram");

	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));

	map(0x05000000, 0x0500000f).rw("ata", FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w));
}


static INPUT_PORTS_START( v0bowl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(2)
	PORT_BIT( 0xffffff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(3)
	PORT_BIT( 0xffffff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(4)
	PORT_BIT( 0xffffff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )


	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void v0bowl_state::machine_start()
{
	save_item(NAME(m_pio_ddr));
	save_item(NAME(m_pio_dat));
	save_item(NAME(m_rtc_sda));
}

void v0bowl_state::machine_reset()
{
	m_pio_ddr = 0xffffffff;
	m_pio_dat = 0;
	m_rtc_sda = 1;
	update_i2c();
}


void v0bowl_state::v0bowl(machine_config &config)
{
	// 80 MHz out of the PLL, not a crystal.  Beware that the SE3208 core retires
	//  one instruction per cycle while the real part averages about five, so the
	// emulated CPU does roughly five times the work per second that the board does.
	// Nothing here has been shown to care yet.
	SE3208(config, m_maincpu, 80'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &v0bowl_state::v0bowl_map);
	m_maincpu->iackx_cb().set(m_vr0soc, FUNC(vrender0soc_device::irq_callback));

	VRENDER0_SOC(config, m_vr0soc, 80'000'000);
	m_vr0soc->set_host_space_tag(m_maincpu, AS_PROGRAM);
	m_vr0soc->int_callback().set_inputline(m_maincpu, se3208_device::SE3208_INT);
	m_vr0soc->set_external_vclk(25.175_MHz_XTAL);
	m_vr0soc->set_uart_external_clock(1.8432_MHz_XTAL);

	rs232_port_device &uart0(RS232_PORT(config, "uart0", default_rs232_devices, nullptr));
	uart0.rxd_handler().set(m_vr0soc, FUNC(vrender0soc_device::rx_w<0>));
	m_vr0soc->tx_callback<0>().set("uart0", FUNC(rs232_port_device::write_txd));

	// set to 38400 baud to see the boot messages
	rs232_port_device &uart1(RS232_PORT(config, "uart1", default_rs232_devices, nullptr));
	uart1.rxd_handler().set(m_vr0soc, FUNC(vrender0soc_device::rx_w<1>));
	m_vr0soc->tx_callback<1>().set("uart1", FUNC(rs232_port_device::write_txd));

	I2C_DS1307(config, m_rtc, 32'768);
	m_rtc->sda_callback().set(FUNC(v0bowl_state::rtc_sda_w));

	ATA_INTERFACE(config, "ata").options(ata_devices, "cf", nullptr, true);

	SPEAKER(config, "speaker", 2).front();
	m_vr0soc->add_route(0, "speaker", 1.0, 0);
	m_vr0soc->add_route(1, "speaker", 1.0, 1);
}


/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( v0bowl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bootrom.u16", 0x00000, 0x80000, CRC(f8f4cf22) SHA1(881d8eeb6f50b2e7933e7c3c93adcdd0c1e93e77) )

	DISK_REGION( "ata:0:cf" )
	DISK_IMAGE( "bowling.001", 0, SHA1(434cb2c264066f7279b06e4ea940091be5f70870) BAD_DUMP ) // identify bytes not dumped, but AI-recreated to match expected serial
ROM_END

} // anonymous namespace


GAME( 200?, v0bowl, 0, v0bowl, v0bowl, v0bowl_state, empty_init, ROT270, "A1 Amusement One", "Fancy Bowling", MACHINE_NOT_WORKING ) // Return Bowl?
