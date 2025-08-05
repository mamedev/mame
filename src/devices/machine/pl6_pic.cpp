// license:BSD-3-Clause
// copyright-holders: NaokiS

#include "emu.h"

#include "pl6_pic.h"

#define LOG_PIC     (1U << 1)
#define VERBOSE     ( LOG_PIC )

#include "logmacro.h"

#define LOGPIC(...)     LOGMASKED(LOG_PIC,     __VA_ARGS__)

pl6pic_device::pl6pic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HEBER_PLUTO6_PIC, tag, owner, clock),
	i2c_hle_interface(mconfig, *this, 0xc0),
	device_rtc_interface(mconfig, *this),
	m_timer(nullptr),
	m_hb_led(*this, "heartbeat_led"),
	m_dip(*this, "SW%u", 1U),
	m_stake(*this, "Stake-Jackpot"),
	m_perc(*this, "Percentage"),
	m_secsw(*this, "SECSW")
{
}

void pl6pic_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_rtc_minute = convert_to_bcd(minute);
	m_rtc_hour   = convert_to_bcd(hour);
	m_rtc_day    = day;
}

void pl6pic_device::device_start()
{
	// Heartbeat LED flash
	m_timer = timer_alloc(FUNC(pl6pic_device::heartbeat_callback), this);
	m_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));
	save_item(NAME(m_heartbeat));
	m_hb_led.resolve();
}

TIMER_CALLBACK_MEMBER(pl6pic_device::heartbeat_callback)
{
	if(m_heartbeat) m_heartbeat = 0;
	else m_heartbeat = 1;
	m_hb_led = m_heartbeat;
}

INPUT_PORTS_START(pluto6_csd_pic)
//ioport("SW1")->read();
	PORT_START( "SW1" )
	PORT_DIPNAME( 0x00000001, 0x00000001, "SW1: 1" ) PORT_DIPLOCATION( "SW1:1" )
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, "SW1: 2" ) PORT_DIPLOCATION( "SW1:2" )
	PORT_DIPSETTING(          0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, "SW1: 3" ) PORT_DIPLOCATION( "SW1:3" )
	PORT_DIPSETTING(          0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, "SW1: 4" ) PORT_DIPLOCATION( "SW1:4" )
	PORT_DIPSETTING(          0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, "SW1: 5" ) PORT_DIPLOCATION( "SW1:5" )
	PORT_DIPSETTING(          0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, "SW1: 6" ) PORT_DIPLOCATION( "SW1:6" )
	PORT_DIPSETTING(          0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, "SW1: 7" ) PORT_DIPLOCATION( "SW1:7" )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, "SW1: 8" ) PORT_DIPLOCATION( "SW1:8" )
	PORT_DIPSETTING(          0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )

	PORT_START( "SW2" )
	PORT_DIPNAME( 0x00000001, 0x00000001, "SW2: 1" ) PORT_DIPLOCATION( "SW2:1" )
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, "SW2: 2" ) PORT_DIPLOCATION( "SW2:2" )
	PORT_DIPSETTING(          0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, "SW2: 3" ) PORT_DIPLOCATION( "SW2:3" )
	PORT_DIPSETTING(          0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, "SW2: 4" ) PORT_DIPLOCATION( "SW2:4" )
	PORT_DIPSETTING(          0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, "SW2: 5" ) PORT_DIPLOCATION( "SW2:5" )
	PORT_DIPSETTING(          0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, "SW2: 6" ) PORT_DIPLOCATION( "SW2:6" )
	PORT_DIPSETTING(          0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, "SW2: 7" ) PORT_DIPLOCATION( "SW2:7" )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, "SW2: 8" ) PORT_DIPLOCATION( "SW2:8" )
	PORT_DIPSETTING(          0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )

	PORT_START("Stake-Jackpot")
	PORT_DIPNAME( 0x07, 0x06, "Stake" )        PORT_DIPLOCATION("Stake-Jackpot:1,2,3")
	PORT_DIPSETTING(    0x00, "5p" )
	PORT_DIPSETTING(    0x01, "10p" )
	PORT_DIPSETTING(    0x02, "20p" )
	PORT_DIPSETTING(    0x03, "25p" )
	PORT_DIPSETTING(    0x04, "30p" )
	PORT_DIPSETTING(    0x05, "40p" )
	PORT_DIPSETTING(    0x06, "50p" )
	PORT_DIPSETTING(    0x07, "£1" )
	PORT_DIPNAME( 0xf0, 0x30, "Jackpot" )        PORT_DIPLOCATION("Stake-Jackpot:5,6,7,8")
	PORT_DIPSETTING(    0x00, "No key" )
	PORT_DIPSETTING(    0x10, "£3 Cash" )
	PORT_DIPSETTING(    0x20, "£4 Cash" )
	PORT_DIPSETTING(    0x30, "£5 Cash" )
	PORT_DIPSETTING(    0x40, "£6 Cash" )
	PORT_DIPSETTING(    0x50, "£6 Token" )
	PORT_DIPSETTING(    0x60, "£8 Cash" )
	PORT_DIPSETTING(    0x70, "£8 Cash" )
	PORT_DIPSETTING(    0x80, "£10 Cash" )
	PORT_DIPSETTING(    0x90, "£15 Cash" )
	PORT_DIPSETTING(    0xa0, "£25 Cash" )
	PORT_DIPSETTING(    0xb0, "£25 LBO" )
	PORT_DIPSETTING(    0xc0, "£35 Cash" )
	PORT_DIPSETTING(    0xd0, "£70 Cash" )
	PORT_DIPSETTING(    0xe0, "£100 Cash" )
	PORT_DIPSETTING(    0xe0, "Invalid" )

	PORT_START("Percentage")
	PORT_DIPNAME( 0x0f, 0x05, "Percentage" )        PORT_DIPLOCATION("Percentage:1,2,3,4")
	PORT_DIPSETTING(    0x00, "No key" )
	PORT_DIPSETTING(    0x01, "70%" )
	PORT_DIPSETTING(    0x02, "72%" )
	PORT_DIPSETTING(    0x03, "74%" )
	PORT_DIPSETTING(    0x04, "76%" )
	PORT_DIPSETTING(    0x05, "78%" )
	PORT_DIPSETTING(    0x06, "80%" )
	PORT_DIPSETTING(    0x07, "82%" )
	PORT_DIPSETTING(    0x08, "84%" )
	PORT_DIPSETTING(    0x09, "86%" )
	PORT_DIPSETTING(    0x0a, "88%" )
	PORT_DIPSETTING(    0x0b, "90%" )
	PORT_DIPSETTING(    0x0c, "92%" )
	PORT_DIPSETTING(    0x0d, "94%" )
	PORT_DIPSETTING(    0x0e, "96%" )
	PORT_DIPSETTING(    0x0f, "98%" )

	PORT_START("SECSW")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Security A") PORT_TOGGLE
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Security B") PORT_TOGGLE
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Security C") PORT_TOGGLE
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Security D") PORT_TOGGLE
INPUT_PORTS_END

ioport_constructor pl6pic_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(pluto6_csd_pic);
}

DEFINE_DEVICE_TYPE(HEBER_PLUTO6_PIC, pl6pic_device, "pl6_pic", "Heber Pluto 6 Customer Security Device")
