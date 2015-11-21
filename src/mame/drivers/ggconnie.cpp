// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/****************************************************************************

    Go! Go! Connie chan Jaka Jaka Janken

    Driver by Mariusz Wojcieszek

    EC9601

    Hudson Chip
    CPU  :Hu6280
    Video:Hu6202,Hu6260,Hu6270

    OSC  :21.47727MHz
    Other:XILINX XC7336-15,OKI M6295


****************************************************************************/

#include "emu.h"
#include "machine/pcecommn.h"
#include "video/huc6270.h"
#include "video/huc6260.h"
#include "video/huc6202.h"
#include "cpu/h6280/h6280.h"
#include "sound/c6280.h"
#include "sound/okim6295.h"
#include "machine/msm6242.h"


class ggconnie_state : public pce_common_state
{
public:
	ggconnie_state(const machine_config &mconfig, device_type type, const char *tag)
		: pce_common_state(mconfig, type, tag),
		m_rtc(*this, "rtc"),
		m_oki(*this, "oki")
		{ }

	required_device <msm6242_device> m_rtc;
	required_device <okim6295_device> m_oki;
	DECLARE_WRITE8_MEMBER(lamp_w);
	DECLARE_WRITE8_MEMBER(output_w);
	DECLARE_READ8_MEMBER(rtc_r);
	DECLARE_WRITE8_MEMBER(rtc_w);
	DECLARE_WRITE8_MEMBER(oki_bank_w);
};

WRITE8_MEMBER(ggconnie_state::lamp_w)
{
	output_set_value("lamp", !BIT(data,0));
}

WRITE8_MEMBER(ggconnie_state::output_w)
{
	// written in "Output Test" in test mode
}

/* TODO: banking not understood (is the ROM dumped correctly btw?) */
WRITE8_MEMBER(ggconnie_state::oki_bank_w)
{
	m_oki->set_bank_base((data) ? 0x40000 : 0x00000);
}


static ADDRESS_MAP_START( sgx_mem , AS_PROGRAM, 8, ggconnie_state )
	AM_RANGE( 0x000000, 0x0fffff) AM_ROM
	AM_RANGE( 0x110000, 0x1edfff) AM_NOP
	AM_RANGE( 0x1ee800, 0x1effff) AM_NOP
	AM_RANGE( 0x1f0000, 0x1f5fff) AM_RAM
	AM_RANGE( 0x1f7000, 0x1f7000) AM_READ_PORT("SWA")
	AM_RANGE( 0x1f7100, 0x1f7100) AM_READ_PORT("SWB")
	AM_RANGE( 0x1f7200, 0x1f7200) AM_READ_PORT("SWC")
	AM_RANGE( 0x1f7700, 0x1f7700) AM_READ_PORT("IN1")
	AM_RANGE( 0x1f7800, 0x1f7800) AM_WRITE(output_w)
	AM_RANGE( 0x1fe000, 0x1fe007) AM_DEVREADWRITE( "huc6270_0", huc6270_device, read, write ) AM_MIRROR(0x03E0)
	AM_RANGE( 0x1fe008, 0x1fe00f) AM_DEVREADWRITE( "huc6202", huc6202_device, read, write ) AM_MIRROR(0x03E0)
	AM_RANGE( 0x1fe010, 0x1fe017) AM_DEVREADWRITE( "huc6270_1", huc6270_device, read, write ) AM_MIRROR(0x03E0)
	AM_RANGE( 0x1fe400, 0x1fe7ff) AM_DEVREADWRITE( "huc6260", huc6260_device, read, write )
	AM_RANGE( 0x1fe800, 0x1febff) AM_DEVREADWRITE("c6280", c6280_device, c6280_r, c6280_w)
	AM_RANGE( 0x1fec00, 0x1fefff) AM_DEVREADWRITE("maincpu", h6280_device, timer_r, timer_w)
	AM_RANGE( 0x1f7300, 0x1f7300) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE( 0x1f7400, 0x1f74ff) AM_WRITE(oki_bank_w)
	AM_RANGE( 0x1f7500, 0x1f750f) AM_DEVREADWRITE("rtc", msm6242_device, read, write)
	AM_RANGE( 0x1ff000, 0x1ff000) AM_READ_PORT("IN0") AM_WRITE(lamp_w)
	AM_RANGE( 0x1ff400, 0x1ff7ff) AM_DEVREADWRITE("maincpu", h6280_device, irq_status_r, irq_status_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( sgx_io , AS_IO, 8, ggconnie_state )
	AM_RANGE( 0x00, 0x03) AM_DEVREADWRITE( "huc6202", huc6202_device, io_read, io_write )
ADDRESS_MAP_END


static INPUT_PORTS_START(ggconnie)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME( "Medal" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) /* 100 Yen */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) /* 10 Yen */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) /* run */
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)  PORT_DIPLOCATION("SWC:8")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hopper")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SWA")
	PORT_DIPNAME(0x03, 0x03, "Coin Set")  PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(0x03, DEF_STR(1C_1C) )
	PORT_DIPSETTING(0x02, "2")
	PORT_DIPSETTING(0x01, "1")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPNAME(0x1c, 0x1c, "100 Yen -> Coin" )  PORT_DIPLOCATION("SWA:3,4,5")
	PORT_DIPSETTING(0x08, "0 Coin")
	PORT_DIPSETTING(0x0c, "5 Coin")
	PORT_DIPSETTING(0x10, "6 Coin")
	PORT_DIPSETTING(0x14, "7 Coin")
	PORT_DIPSETTING(0x18, "8 Coin")
	PORT_DIPSETTING(0x1c, "10 Coin")
	PORT_DIPSETTING(0x00, "11 Coin")
	PORT_DIPSETTING(0x04, "12 Coin")
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SWA:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SWA:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SWA:8" )

	PORT_START("SWB")
	PORT_DIPNAME(0x07, 0x07, "Payout")  PORT_DIPLOCATION("SWB:1,2,3")
	PORT_DIPSETTING(0x00, "85%")
	PORT_DIPSETTING(0x01, "90%")
	PORT_DIPSETTING(0x02, "55%")
	PORT_DIPSETTING(0x03, "60%")
	PORT_DIPSETTING(0x04, "65%")
	PORT_DIPSETTING(0x05, "70%")
	PORT_DIPSETTING(0x06, "75%")
	PORT_DIPSETTING(0x07, "80%")
	PORT_DIPNAME(0x18, 0x18, DEF_STR(Difficulty))  PORT_DIPLOCATION("SWB:4,5")
	PORT_DIPSETTING(0x00, DEF_STR(Easy))
	PORT_DIPSETTING(0x08, DEF_STR(Very_Hard))
	PORT_DIPSETTING(0x10, DEF_STR(Hard))
	PORT_DIPSETTING(0x18, DEF_STR(Normal))
	PORT_DIPNAME(0x20, 0x20, "Payout Info")  PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x20, DEF_STR(On))
	PORT_DIPNAME(0xc0, 0xc0, "Rate")  PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(0x00, "Few" )
	PORT_DIPSETTING(0x40, "Most" )
	PORT_DIPSETTING(0x80, "More" )
	PORT_DIPSETTING(0xc0, DEF_STR(Normal))

	PORT_START("SWC")
	PORT_DIPNAME(0x03, 0x03, "Demo Sound" )  PORT_DIPLOCATION("SWC:1,2")
	PORT_DIPSETTING(0x00, DEF_STR(Off) )
	PORT_DIPSETTING(0x01, "3/1" )
	PORT_DIPSETTING(0x02, "2/1" )
	PORT_DIPSETTING(0x03, "1/1" )
	PORT_DIPNAME(0x0c, 0x0c, "Start Time" )  PORT_DIPLOCATION("SWC:3,4")
	PORT_DIPSETTING(0x00, "4 sec" )
	PORT_DIPSETTING(0x04, "8 sec" )
	PORT_DIPSETTING(0x08, "6 sec" )
	PORT_DIPSETTING(0x0c, "5 sec" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SWC:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SWC:6" )
	PORT_DIPNAME(0x40, 0x00, "RAM Clear" )  PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(0x40, DEF_STR(Off) )
	PORT_DIPSETTING(0x00, DEF_STR(On) )
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Service_Mode) )  PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(0x80, DEF_STR(Off) )
	PORT_DIPSETTING(0x00, DEF_STR(On) )
INPUT_PORTS_END

static MACHINE_CONFIG_START( ggconnie, ggconnie_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H6280, PCE_MAIN_CLOCK/3)
	MCFG_CPU_PROGRAM_MAP(sgx_mem)
	MCFG_CPU_IO_MAP(sgx_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PCE_MAIN_CLOCK/3, HUC6260_WPF, 64, 64 + 1024 + 64, HUC6260_LPF, 18, 18 + 242)
	MCFG_SCREEN_UPDATE_DRIVER( ggconnie_state, screen_update )
	MCFG_SCREEN_PALETTE("huc6260:palette")

	MCFG_DEVICE_ADD( "huc6260", HUC6260, PCE_MAIN_CLOCK/3 )
	MCFG_HUC6260_NEXT_PIXEL_DATA_CB(DEVREAD16("huc6202", huc6202_device, next_pixel))
	MCFG_HUC6260_TIME_TIL_NEXT_EVENT_CB(DEVREAD16("huc6202", huc6202_device, time_until_next_event))
	MCFG_HUC6260_VSYNC_CHANGED_CB(DEVWRITELINE("huc6202", huc6202_device, vsync_changed))
	MCFG_HUC6260_HSYNC_CHANGED_CB(DEVWRITELINE("huc6202", huc6202_device, hsync_changed))
	MCFG_DEVICE_ADD( "huc6270_0", HUC6270, 0 )
	MCFG_HUC6270_VRAM_SIZE(0x10000)
	MCFG_HUC6270_IRQ_CHANGED_CB(WRITELINE(pce_common_state, pce_irq_changed))
	MCFG_DEVICE_ADD( "huc6270_1", HUC6270, 0 )
	MCFG_HUC6270_VRAM_SIZE(0x10000)
	MCFG_HUC6270_IRQ_CHANGED_CB(WRITELINE(pce_common_state, pce_irq_changed))
	MCFG_DEVICE_ADD( "huc6202", HUC6202, 0 )
	MCFG_HUC6202_NEXT_PIXEL_0_CB(DEVREAD16("huc6270_0", huc6270_device, next_pixel))
	MCFG_HUC6202_TIME_TIL_NEXT_EVENT_0_CB(DEVREAD16("huc6270_0", huc6270_device, time_until_next_event))
	MCFG_HUC6202_VSYNC_CHANGED_0_CB(DEVWRITELINE("huc6270_0", huc6270_device, vsync_changed))
	MCFG_HUC6202_HSYNC_CHANGED_0_CB(DEVWRITELINE("huc6270_0", huc6270_device, hsync_changed))
	MCFG_HUC6202_READ_0_CB(DEVREAD8("huc6270_0", huc6270_device, read))
	MCFG_HUC6202_WRITE_0_CB(DEVWRITE8("huc6270_0", huc6270_device, write))
	MCFG_HUC6202_NEXT_PIXEL_1_CB(DEVREAD16("huc6270_1", huc6270_device, next_pixel))
	MCFG_HUC6202_TIME_TIL_NEXT_EVENT_1_CB(DEVREAD16("huc6270_1", huc6270_device, time_until_next_event))
	MCFG_HUC6202_VSYNC_CHANGED_1_CB(DEVWRITELINE("huc6270_1", huc6270_device, vsync_changed))
	MCFG_HUC6202_HSYNC_CHANGED_1_CB(DEVWRITELINE("huc6270_1", huc6270_device, hsync_changed))
	MCFG_HUC6202_READ_1_CB(DEVREAD8("huc6270_1", huc6270_device, read))
	MCFG_HUC6202_WRITE_1_CB(DEVWRITE8("huc6270_1", huc6270_device, write))

	MCFG_DEVICE_ADD("rtc", MSM6242, XTAL_32_768kHz)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker","rspeaker")
	MCFG_SOUND_ADD("c6280", C6280, PCE_MAIN_CLOCK/6)
	MCFG_C6280_CPU("maincpu")
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)

	MCFG_OKIM6295_ADD("oki", PCE_MAIN_CLOCK/12, OKIM6295_PIN7_HIGH) /* unknown clock / pin 7 */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.00)
MACHINE_CONFIG_END

ROM_START(ggconnie)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "prg0_u3_ver.2.bin", 0x00000, 0x80000, CRC(5e104855) SHA1(3ab2b1ec1fc3aefbb57d9b2ba272e75b34b69383) )
	ROM_LOAD( "prg1_u4.bin", 0x80000, 0x80000, CRC(513f0b18) SHA1(44c61dc1a06bb4c8b4840ea6a372f92114888490) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "adpcm_u31.bin", 0x00000, 0x80000, CRC(de514c2b) SHA1(da73aa825d73646f556f6d4dbb46f43acf7c3357) )
ROM_END

GAME( 1996, ggconnie, 0, ggconnie, ggconnie, pce_common_state, pce_common, ROT0, "Eighting", "Go! Go! Connie chan Jaka Jaka Janken", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
