// license:BSD-3-Clause
// copyright-holders:David Shah
/***************************************************************************

  Systema TV Boy Driver

TODO:
- Find, dump and add the other devices (TV Boy, Super TV Boy)
- Add NTSC variant
***************************************************************************/

#include "emu.h"

#include "includes/a2600.h"
#include "machine/bankdev.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

class tvboy_state : public a2600_state
{
public:
	tvboy_state(const machine_config &mconfig, device_type type, const char *tag)
		: a2600_state(mconfig, type, tag)
		, m_crom(*this, "crom")
		, m_rom(*this, "mainrom") { }

	DECLARE_WRITE8_MEMBER(bank_write);

	void tvboyii(machine_config &config);

	void rom_map(address_map &map);
	void tvboy_mem(address_map &map);
private:
	required_device<address_map_bank_device> m_crom;
	required_region_ptr<uint8_t> m_rom;

	virtual void machine_reset() override;
};

void tvboy_state::machine_reset() {
	m_crom->set_bank(0);
	a2600_state::machine_reset();
}

WRITE8_MEMBER(tvboy_state::bank_write) {
	logerror("banking (?) write %04x, %02x\n", offset, data);
	if ((offset & 0xFF00) == 0x0800)
		m_crom->set_bank(data);
}

ADDRESS_MAP_START(tvboy_state::tvboy_mem) // 6507 has 13-bit address space, 0x0000 - 0x1fff
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x0f00) AM_DEVREADWRITE("tia_video", tia_video_device, read, write)
	AM_RANGE(0x0080, 0x00ff) AM_MIRROR(0x0d00) AM_RAM AM_SHARE("riot_ram")
#if USE_NEW_RIOT
	AM_RANGE(0x0280, 0x029f) AM_MIRROR(0x0d00) AM_DEVICE("riot", mos6532_t, io_map)
#else
	AM_RANGE(0x0280, 0x029f) AM_MIRROR(0x0d00) AM_DEVREADWRITE("riot", riot6532_device, read, write)
#endif
	AM_RANGE(0x1000, 0x1fff) AM_WRITE(bank_write)
	AM_RANGE(0x1000, 0x1fff) AM_DEVICE("crom", address_map_bank_device, amap8)
ADDRESS_MAP_END

ADDRESS_MAP_START(tvboy_state::rom_map)
	AM_RANGE(0x00000, 0x7ffff) AM_ROM AM_REGION("mainrom", 0)
ADDRESS_MAP_END

#define MASTER_CLOCK_PAL    3546894

MACHINE_CONFIG_START(tvboy_state::tvboyii)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6507, MASTER_CLOCK_PAL / 3)
	MCFG_CPU_PROGRAM_MAP(tvboy_mem)
	MCFG_M6502_DISABLE_DIRECT()

	MCFG_DEVICE_ADD("crom", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(rom_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDR_WIDTH(19)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x1000)

	/* video hardware */
	MCFG_DEVICE_ADD("tia_video", TIA_PAL_VIDEO, 0)
	MCFG_TIA_READ_INPUT_PORT_CB(READ16(tvboy_state, a2600_read_input_port))
	MCFG_TIA_DATABUS_CONTENTS_CB(READ8(tvboy_state, a2600_get_databus_contents))
	MCFG_TIA_VSYNC_CB(WRITE16(tvboy_state, a2600_tia_vsync_callback_pal))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS( MASTER_CLOCK_PAL, 228, 26, 26 + 160 + 16, 312, 32, 32 + 228 + 31 )
	MCFG_SCREEN_UPDATE_DEVICE("tia_video", tia_video_device, screen_update)
	MCFG_SCREEN_PALETTE("tia_video:palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_TIA_ADD("tia", MASTER_CLOCK_PAL/114)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90)

	/* devices */
#if USE_NEW_RIOT
	MCFG_DEVICE_ADD("riot", MOS6532n, MASTER_CLOCK_PAL / 3)
	MCFG_MOS6530n_IN_PA_CB(READ8(tvboy_state, switch_A_r))
	MCFG_MOS6530n_OUT_PA_CB(WRITE8(tvboy_state, switch_A_w))
	MCFG_MOS6530n_IN_PB_CB(READ8(tvboy_state, riot_input_port_8_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(tvboy_state, switch_B_w))
	MCFG_MOS6530n_IRQ_CB(WRITELINE(tvboy_state, irq_callback))
#else
	MCFG_DEVICE_ADD("riot", RIOT6532, MASTER_CLOCK_PAL / 3)
	MCFG_RIOT6532_IN_PA_CB(READ8(tvboy_state, switch_A_r))
	MCFG_RIOT6532_OUT_PA_CB(WRITE8(tvboy_state, switch_A_w))
	MCFG_RIOT6532_IN_PB_CB(READ8(tvboy_state, riot_input_port_8_r))
	MCFG_RIOT6532_OUT_PB_CB(WRITE8(tvboy_state, switch_B_w))
	MCFG_RIOT6532_IRQ_CB(WRITELINE(tvboy_state, irq_callback))
#endif

	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, "joy")
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, nullptr)
MACHINE_CONFIG_END

static INPUT_PORTS_START( tvboyii )
	PORT_START("SWB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Game") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Select Game") PORT_CODE(KEYCODE_1)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x08, 0x08, "TV Type" ) PORT_CODE(KEYCODE_C) PORT_TOGGLE
	PORT_DIPSETTING(    0x08, "Color" )
	PORT_DIPSETTING(    0x00, "B&W" )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, "Left Diff. Switch" ) PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_DIPSETTING(    0x40, "A" )
	PORT_DIPSETTING(    0x00, "B" )
	PORT_DIPNAME( 0x80, 0x00, "Right Diff. Switch" ) PORT_CODE(KEYCODE_4) PORT_TOGGLE
	PORT_DIPSETTING(    0x80, "A" )
	PORT_DIPSETTING(    0x00, "B" )
INPUT_PORTS_END

ROM_START( tvboyii )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x80000, "mainrom", 0 )
	ROM_LOAD( "HY23400P.bin", 0x00000, 0x80000, CRC(f8485173) SHA1(cafbaa0c5437f192cb4fb49f9a672846aa038870) )
ROM_END


ROM_START( stvboy )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x80000, "mainrom", 0 )
	ROM_LOAD( "supertvboy.bin", 0x00000, 0x80000, CRC(af2e73e8) SHA1(04b9ddc3b30b0e5b81b9f868d455e902a0151491) )
ROM_END


//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    STATE        INIT  COMPANY    FULLNAME
CONS( 199?, tvboyii, 0,      0,      tvboyii, tvboyii, tvboy_state, 0,    "Systema", "TV Boy II (PAL)" ,    MACHINE_SUPPORTS_SAVE )
CONS( 1995, stvboy,  0,      0,      tvboyii, tvboyii, tvboy_state, 0,    "Akor",    "Super TV Boy (PAL)" , MACHINE_SUPPORTS_SAVE )
