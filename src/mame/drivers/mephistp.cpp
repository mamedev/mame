// license:BSD-3-Clause
// copyright-holders:Robbbert,AJR
/*****************************************************************************************

  8088-based pinball games by Unidesa/Stargame:
  - Mephisto
  - Cirsa Sport 2000

  Serial communication with the sound board is handled by a 8256 MUART (not emulated yet).

******************************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/i8155.h"
//#include "machine/i8256.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "sound/3812intf.h"
#include "speaker.h"

// mephisto_state was also defined in mess/drivers/mephisto.c
class mephisto_pinball_state : public driver_device
{
public:
	mephisto_pinball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_aysnd(*this, "aysnd")
		, m_soundbank(*this, "soundbank")
	{ }

	void mephisto(machine_config &config);
	void sport2k(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(shift_load_w);
	DECLARE_READ8_MEMBER(ay8910_read);
	DECLARE_WRITE8_MEMBER(ay8910_write);
	DECLARE_WRITE8_MEMBER(t0_t1_w);
	DECLARE_WRITE8_MEMBER(ay8910_columns_w);
	DECLARE_READ8_MEMBER(ay8910_inputs_r);
	DECLARE_WRITE8_MEMBER(sound_rombank_w);

	void mephisto_8051_io(address_map &map);
	void mephisto_8051_map(address_map &map);
	void mephisto_map(address_map &map);
	void sport2k_8051_io(address_map &map);

	u8 m_ay8910_data;
	bool m_ay8910_bdir;
	bool m_ay8910_bc1;
	void ay8910_update();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<ay8910_device> m_aysnd;
	required_memory_bank m_soundbank;
};


WRITE8_MEMBER(mephisto_pinball_state::shift_load_w)
{
}

READ8_MEMBER(mephisto_pinball_state::ay8910_read)
{
	return m_ay8910_data;
}

WRITE8_MEMBER(mephisto_pinball_state::ay8910_write)
{
	m_ay8910_data = data;
	ay8910_update();
}

WRITE8_MEMBER(mephisto_pinball_state::t0_t1_w)
{
	m_ay8910_bdir = BIT(data, 4); // T0
	m_ay8910_bc1 = BIT(data, 5); // T1
	ay8910_update();
}

void mephisto_pinball_state::ay8910_update()
{
	if (m_ay8910_bdir)
		m_aysnd->data_address_w(m_ay8910_bc1, m_ay8910_data);
	else if (m_ay8910_bc1)
		m_ay8910_data = m_aysnd->data_r();
}

WRITE8_MEMBER(mephisto_pinball_state::ay8910_columns_w)
{
}

READ8_MEMBER(mephisto_pinball_state::ay8910_inputs_r)
{
	return 0xff;
}

WRITE8_MEMBER(mephisto_pinball_state::sound_rombank_w)
{
	m_soundbank->set_entry(data & 0xf);
}

void mephisto_pinball_state::mephisto_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("maincpu", 0);
	map(0x10000, 0x107ff).ram().share("nvram");
	map(0x12000, 0x1201f).noprw(); //AM_DEVREADWRITE("muart", i8256_device, read, write)
	map(0x13000, 0x130ff).rw("ic20", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x13800, 0x13807).rw("ic20", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x14000, 0x140ff).rw("ic9", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x14800, 0x14807).rw("ic9", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x16000, 0x16000).w(FUNC(mephisto_pinball_state::shift_load_w));
	map(0x17000, 0x17001).nopw(); //???
	map(0xf0000, 0xfffff).rom().region("maincpu", 0);
}

void mephisto_pinball_state::mephisto_8051_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xffff).bankr("soundbank");
}

void mephisto_pinball_state::mephisto_8051_io(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0800).w(FUNC(mephisto_pinball_state::sound_rombank_w));
	map(0x1000, 0x1000).w("dac", FUNC(dac08_device::data_w));
}

void mephisto_pinball_state::sport2k_8051_io(address_map &map)
{
	mephisto_8051_io(map);
	map(0x1800, 0x1801).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
}


static INPUT_PORTS_START( mephisto )
INPUT_PORTS_END

void mephisto_pinball_state::machine_start()
{
	m_soundbank->configure_entries(0, 16, memregion("sound1")->base(), 0x8000);
	m_soundbank->set_entry(0);

	m_ay8910_data = 0;
	m_ay8910_bdir = 1;
	m_ay8910_bc1 = 1;
	save_item(NAME(m_ay8910_data));
	save_item(NAME(m_ay8910_bdir));
	save_item(NAME(m_ay8910_bc1));
}

void mephisto_pinball_state::machine_reset()
{
}

void mephisto_pinball_state::mephisto(machine_config &config)
{
	/* basic machine hardware */
	I8088(config, m_maincpu, XTAL(18'000'000)/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_pinball_state::mephisto_map);
	//m_maincpu->set_irq_acknowledge_callback("muart", FUNC(i8256_device::inta_cb));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	//i8256_device &muart(I8256(config, "muart", XTAL(18'000'000)/3));
	//muart.irq_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	//muart.txd_handler().set_inputline("audiocpu", MCS51_RX_LINE);

	I8155(config, "ic20", XTAL(18'000'000)/6);
	//i8155_device &i8155_1(I8155(config, "ic20", XTAL(18'000'000)/6));
	//i8155_1.out_to_callback().set("muart", FUNC(i8256_device::write_txc));

	I8155(config, "ic9", XTAL(18'000'000)/6);
	//i8155_device &i8155_2(I8155(config, "ic9", XTAL(18'000'000)/6));
	//i8155_2.out_to_callback().set(FUNC(mephisto_pinball_state::clk_shift_w));

	i8051_device &soundcpu(I8051(config, "soundcpu", XTAL(12'000'000)));
	soundcpu.set_addrmap(AS_PROGRAM, &mephisto_pinball_state::mephisto_8051_map); // EA tied high for external program ROM
	soundcpu.set_addrmap(AS_IO, &mephisto_pinball_state::mephisto_8051_io);
	soundcpu.port_in_cb<1>().set(FUNC(mephisto_pinball_state::ay8910_read));
	soundcpu.port_out_cb<1>().set(FUNC(mephisto_pinball_state::ay8910_write));
	soundcpu.port_out_cb<3>().set(FUNC(mephisto_pinball_state::t0_t1_w));
	soundcpu.serial_rx_cb().set_constant(0); // from MUART

	SPEAKER(config, "mono").front_center();

	AY8910(config, m_aysnd, XTAL(12'000'000)/8);
	m_aysnd->port_a_write_callback().set(FUNC(mephisto_pinball_state::ay8910_columns_w));
	m_aysnd->port_b_read_callback().set(FUNC(mephisto_pinball_state::ay8910_inputs_r));
	m_aysnd->add_route(ALL_OUTPUTS, "mono", 0.5);

	DAC08(config, "dac", 0).add_route(ALL_OUTPUTS, "mono", 0.5);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}


void mephisto_pinball_state::sport2k(machine_config &config)
{
	mephisto(config);
	subdevice<i8051_device>("soundcpu")->set_addrmap(AS_IO, &mephisto_pinball_state::sport2k_8051_io);

	YM3812(config, "ymsnd", XTAL(14'318'181)/4).add_route(ALL_OUTPUTS, "mono", 0.5);
}

/*-------------------------------------------------------------------
/ Mephisto
/-------------------------------------------------------------------*/
ROM_START(mephistp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_ver1.2", 0x00000, 0x8000, CRC(845c8eb4) SHA1(2a705629990950d4e2d3a66a95e9516cf112cc88))
	ROM_RELOAD(0x08000, 0x8000)

	ROM_REGION(0x08000, "soundcpu", 0)
	ROM_LOAD("ic15_02", 0x00000, 0x8000, CRC(2accd446) SHA1(7297e4825c33e7cf23f86fe39a0242e74874b1e2))

	ROM_REGION(0x80000, "sound1", 0)
	ROM_LOAD("ic14_s0", 0x40000, 0x8000, CRC(7cea3018) SHA1(724fe7a4456cbf2ac01466d946668ee86f4410ae))
	ROM_LOAD("ic13_s1", 0x48000, 0x8000, CRC(5a9e0f1d) SHA1(dbfd307706c51f8809f4867a199b4b62beb64379))
	ROM_LOAD("ic12_s2", 0x50000, 0x8000, CRC(b3cc962a) SHA1(521376cab7e917a5d5f5f183bccb21bd13327c48))
	ROM_LOAD("ic11_s3", 0x58000, 0x8000, CRC(8aaa21ec) SHA1(29f17249cac62128fd8b0eee415ce399ee2ec672))
	ROM_LOAD("ic16_c", 0x60000, 0x8000, CRC(5f12b4f4) SHA1(73fbdb57fca0dbc918e6665a6cb949e741f2720a))
	ROM_LOAD("ic17_d", 0x68000, 0x8000, CRC(d17e18a8) SHA1(372eaf209ea5d26f3c096aadd7d028ef68bfb68e))
	ROM_LOAD("ic18_e", 0x70000, 0x8000, CRC(eac6dbba) SHA1(f4971c8b0aa3a72c396b943a0ee3094afb902ec1))
	ROM_LOAD("ic19_f", 0x78000, 0x8000, CRC(cc4bb629) SHA1(db46be2a8034bbd106b7dd80f50988c339684b5e))
ROM_END

ROM_START(mephistp1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_ver1.1", 0x00000, 0x8000, CRC(ce584902) SHA1(dd05d008bbd9b6588cb204e8d901537ffe7ddd43))
	ROM_RELOAD(0x08000, 0x8000)

	ROM_REGION(0x08000, "soundcpu", 0)
	ROM_LOAD("ic15_02", 0x00000, 0x8000, CRC(2accd446) SHA1(7297e4825c33e7cf23f86fe39a0242e74874b1e2))

	ROM_REGION(0x80000, "sound1", 0)
	ROM_LOAD("ic14_s0", 0x40000, 0x8000, CRC(7cea3018) SHA1(724fe7a4456cbf2ac01466d946668ee86f4410ae))
	ROM_LOAD("ic13_s1", 0x48000, 0x8000, CRC(5a9e0f1d) SHA1(dbfd307706c51f8809f4867a199b4b62beb64379))
	ROM_LOAD("ic12_s2", 0x50000, 0x8000, CRC(b3cc962a) SHA1(521376cab7e917a5d5f5f183bccb21bd13327c48))
	ROM_LOAD("ic11_s3", 0x58000, 0x8000, CRC(8aaa21ec) SHA1(29f17249cac62128fd8b0eee415ce399ee2ec672))
	ROM_LOAD("ic16_c", 0x60000, 0x8000, CRC(5f12b4f4) SHA1(73fbdb57fca0dbc918e6665a6cb949e741f2720a))
	ROM_LOAD("ic17_d", 0x68000, 0x8000, CRC(d17e18a8) SHA1(372eaf209ea5d26f3c096aadd7d028ef68bfb68e))
	ROM_LOAD("ic18_e", 0x70000, 0x8000, CRC(eac6dbba) SHA1(f4971c8b0aa3a72c396b943a0ee3094afb902ec1))
	ROM_LOAD("ic19_f", 0x78000, 0x8000, CRC(cc4bb629) SHA1(db46be2a8034bbd106b7dd80f50988c339684b5e))
ROM_END

ROM_START(sport2k)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u1_256.bin", 0x00000, 0x8000, CRC(403f9000) SHA1(376dc17355c9569bd1ed9b19dbc322bfd69bf938))
	ROM_LOAD("u2_256.bin", 0x08000, 0x8000, CRC(4a88cc10) SHA1(591568dc60c40cc058f45a144c098faccb4e970c))

	ROM_REGION(0x08000, "soundcpu", 0)
	ROM_LOAD("c541_256.bin", 0x00000, 0x8000, CRC(7ca4a952) SHA1(6b01f7f79fa88c4ae71a6a19341760fa256b9958))

	ROM_REGION(0x50000, "sound1", 0)
	ROM_LOAD("s117_512.bin", 0x00000, 0x10000, CRC(035d302e) SHA1(f207ea239e5a34839366cc19a569ab5f3d1e1a60))
	ROM_LOAD("s211_512.bin", 0x10000, 0x10000, CRC(61cf84f9) SHA1(4c5680fbf48f30fbe0e15f4194ab708955df7721))
	ROM_LOAD("s311_512.bin", 0x20000, 0x10000, CRC(162cd1ff) SHA1(4d9ad7a839cc16e74abfc77c92674608ccba8cc3))
	ROM_LOAD("s411_512.bin", 0x30000, 0x10000, CRC(4deffaa0) SHA1(98a20a01437ea060ac5c6fb52f4da892fee1fb75))
	ROM_LOAD("s511_512.bin", 0x40000, 0x10000, CRC(ca9afa80) SHA1(6f219bdc1ad06e340b2930610897b70369a43684))
ROM_END

GAME(1987,  mephistp,   0,         mephisto,  mephisto, mephisto_pinball_state, empty_init, ROT0,  "Stargame",    "Mephisto (Stargame) (rev. 1.2)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987,  mephistp1,  mephistp,  mephisto,  mephisto, mephisto_pinball_state, empty_init, ROT0,  "Stargame",    "Mephisto (Stargame) (rev. 1.1)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1988,  sport2k,    0,         sport2k,   mephisto, mephisto_pinball_state, empty_init, ROT0,  "Cirsa",       "Sport 2000",              MACHINE_IS_SKELETON_MECHANICAL)
