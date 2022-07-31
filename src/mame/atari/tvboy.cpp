// license:BSD-3-Clause
// copyright-holders:David Shah
/***************************************************************************

  Systema TV Boy Driver

TODO:
- Find, dump and add the other devices (TV Boy, Super TV Boy)
- Add NTSC variant
***************************************************************************/

#include "emu.h"

#include "a2600.h"
#include "machine/bankdev.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

class tvboy_state : public a2600_base_state
{
public:
	tvboy_state(const machine_config &mconfig, device_type type, const char *tag)
		: a2600_base_state(mconfig, type, tag)
		, m_crom(*this, "crom")
		, m_rom(*this, "mainrom")
	{ }

	void tvboyii(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void bank_write(offs_t offset, uint8_t data);

	void rom_map(address_map &map);
	void tvboy_mem(address_map &map);

	required_memory_bank m_crom;
	required_region_ptr<uint8_t> m_rom;
};

void tvboy_state::machine_start()
{
	a2600_base_state::machine_start();
	m_crom->configure_entries(0, m_rom.bytes() / 0x1000, &m_rom[0], 0x1000);
}

void tvboy_state::machine_reset()
{
	m_crom->set_entry(0);
	a2600_base_state::machine_reset();
}

void tvboy_state::bank_write(offs_t offset, uint8_t data)
{
	logerror("banking (?) write %04x, %02x\n", offset, data);
	if ((offset & 0xff00) == 0x0800)
		m_crom->set_entry(data);
}

void tvboy_state::tvboy_mem(address_map &map)
{ // 6507 has 13-bit address space, 0x0000 - 0x1fff
	map(0x0000, 0x007f).mirror(0x0f00).rw("tia_video", FUNC(tia_video_device::read), FUNC(tia_video_device::write));
	map(0x0080, 0x00ff).mirror(0x0d00).ram().share("riot_ram");
#if USE_NEW_RIOT
	map(0x0280, 0x029f).mirror(0x0d00).m("riot", FUNC(mos6532_new_device::io_map));
#else
	map(0x0280, 0x029f).mirror(0x0d00).rw("riot", FUNC(riot6532_device::read), FUNC(riot6532_device::write));
#endif
	map(0x1000, 0x1fff).w(FUNC(tvboy_state::bank_write));
	map(0x1000, 0x1fff).bankr(m_crom);
}

#define MASTER_CLOCK_PAL    3546894

void tvboy_state::tvboyii(machine_config &config)
{
	/* basic machine hardware */
	M6507(config, m_maincpu, MASTER_CLOCK_PAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &tvboy_state::tvboy_mem);

	/* video hardware */
	TIA_PAL_VIDEO(config, m_tia, 0, "tia");
	m_tia->read_input_port_callback().set(FUNC(tvboy_state::a2600_read_input_port));
	m_tia->databus_contents_callback().set(FUNC(tvboy_state::a2600_get_databus_contents));
	m_tia->vsync_callback().set(FUNC(tvboy_state::a2600_tia_vsync_callback_pal));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK_PAL, 228, 26, 26 + 160 + 16, 312, 32, 32 + 228 + 31);
	m_screen->set_screen_update("tia_video", FUNC(tia_video_device::screen_update));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	TIA(config, "tia", MASTER_CLOCK_PAL/114).add_route(ALL_OUTPUTS, "mono", 0.90);

	/* devices */
#if USE_NEW_RIOT
	MOS6532_NEW(config, m_riot, MASTER_CLOCK_PAL / 3);
	m_riot->pa_rd_callback().set(FUNC(tvboy_state::switch_A_r));
	m_riot->pa_wr_callback().set(FUNC(tvboy_state::switch_A_w));
	m_riot->pb_rd_callback().set_ioport("SWB");
	m_riot->pb_wr_callback().set(FUNC(tvboy_state::switch_B_w));
	m_riot->irq_wr_callback().set(FUNC(tvboy_state::irq_callback));
#else
	RIOT6532(config, m_riot, MASTER_CLOCK_PAL / 3);
	m_riot->in_pa_callback().set(FUNC(tvboy_state::switch_A_r));
	m_riot->out_pa_callback().set(FUNC(tvboy_state::switch_A_w));
	m_riot->in_pb_callback().set_ioport("SWB");
	m_riot->out_pb_callback().set(FUNC(tvboy_state::switch_B_w));
	m_riot->irq_callback().set(FUNC(tvboy_state::irq_callback));
#endif

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, "joy");
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, nullptr);
}

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
	ROM_LOAD( "hy23400p.bin", 0x00000, 0x80000, CRC(f8485173) SHA1(cafbaa0c5437f192cb4fb49f9a672846aa038870) )
ROM_END


ROM_START( stvboy )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x80000, "mainrom", 0 )
	ROM_LOAD( "supertvboy.bin", 0x00000, 0x80000, CRC(af2e73e8) SHA1(04b9ddc3b30b0e5b81b9f868d455e902a0151491) )
ROM_END


//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS        INIT        COMPANY    FULLNAME
CONS( 199?, tvboyii, 0,      0,      tvboyii, tvboyii, tvboy_state, empty_init, "Systema", "TV Boy II (PAL)" ,    MACHINE_SUPPORTS_SAVE )
CONS( 1995, stvboy,  0,      0,      tvboyii, tvboyii, tvboy_state, empty_init, "Akor",    "Super TV Boy (PAL)" , MACHINE_SUPPORTS_SAVE )
