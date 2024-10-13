// license:BSD-3-Clause
// copyright-holders:AJR, Roberto Fresca
/*******************************************************************************

  The Boat
  1987(c) Hit Gun Co, LTD.

  Gambling/Amusement game
  Selectable through DIP switches.

  Driver by AJR.
  Additional work by Roberto Fresca.

*******************************************************************************

  Hardware specs...

  Seems based on MSX2.
  PCB is marked "TVG01"

  CPU:
  1x Sharp LH0080A (Z80-A).       (IC26)

  I/O:
  2x NEC D8255AC-2 PPI.           (IC23, IC27)

  Sound:
  1x GI AY-3-8910A.               (IC18)
  1x Fujitsu MB3712 (audio amp)   (IC2)

  Video:
  1x Yamaha V9938 VDP (scratched) (IC13)

  RAM:
  4x Fujitsu MB81464-15 (256 Kbit DRAM) (VRAM)  (IC6, IC7, IC9, IC10)
  1x NEC D449C-2 (2K x 8 Static RAM) (WRK RAM)  (IC1)

  ROM:
  2x Mitsubishi M5L27128K for program.  (IC3, IC5)
  3x Mitsubishi M5L27128K for graphics. (IC8, IC11, IC14)
  1x Empty socket for extra data. (*)   (IC16)

  Other:
  1x 21.47727 MHz Xtal.
  1x 8 DIP switches bank.
  1x 2x18 pins edge connector.
  1x 2x10 pins edge connector.


  (*) Note: The CE line of the extra ROM that should be located in the
      empty socket, is tied to PPI port B, D3.

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/ay8910.h"
#include "video/v9938.h"
#include "screen.h"
#include "speaker.h"


namespace {

class tvg01_state : public driver_device
{
public:
	tvg01_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_player_inputs{{*this, "INP1.%u", 0U}, {*this, "INP2.%u", 0U}}
		, m_banked_rom(*this, "banked_rom")
		, m_hopper(*this, "hopper")
	{
	}

	void theboat(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	template <int P> void input_select_w(u8 data);
	u8 player_inputs_r();
	void bank_select_w(u8 data);
	u8 bank_r(offs_t offset);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_ioport_array<6> m_player_inputs[2];
	required_region_ptr<u8> m_banked_rom;
	required_device<ticket_dispenser_device> m_hopper;

	u8 m_input_select[2]{};
	u8 m_bank_select = 0;
};

#define MAIN_CLOCK      XTAL(21'477'272)
#define VDP_CLOCK       MAIN_CLOCK
#define CPU_CLOCK       MAIN_CLOCK / 6
#define PSG_CLOCK       MAIN_CLOCK / 12


void tvg01_state::machine_start()
{
	save_item(NAME(m_input_select));
	save_item(NAME(m_bank_select));
}

template <int P>
void tvg01_state::input_select_w(u8 data)
{
	m_input_select[P] = data;
}

u8 tvg01_state::player_inputs_r()
{
	u8 result = 0xff;

	for (int p = 0; p < 2; p++)
		for (int n = 0; n < 6; n++)
			if (!BIT(m_input_select[p], n))
				result &= m_player_inputs[p][n]->read();

	return result;
}

void tvg01_state::bank_select_w(u8 data)
{
	m_bank_select = data;
	m_hopper->motor_w(BIT(data, 4));
}

u8 tvg01_state::bank_r(offs_t offset)
{
	u8 result = 0xff;

	for (int i = 0; i < 4; i++)
		if (!BIT(m_bank_select, i))
			result &= m_banked_rom[i * 0x4000 + offset];

	return result;
}


void tvg01_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0xc000, 0xffff).r(FUNC(tvg01_state::bank_r));
}

void tvg01_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r("psg", FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w("psg", FUNC(ay8910_device::data_address_w));
	map(0x20, 0x23).rw("vdp", FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0x80, 0x83).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa0, 0xa3).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
}


static INPUT_PORTS_START(theboat)
	PORT_START("INP0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )       PORT_IMPULSE(3)  PORT_NAME("P1 Coin (1 credit)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )       PORT_IMPULSE(10)  PORT_NAME("P2 Key Up (10 credits)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK)  PORT_NAME("Analyze / Bookkeeping")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Pay Out")  PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )       PORT_IMPULSE(10)  PORT_NAME("P1 Key Up (10 credits)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM )      PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )       PORT_IMPULSE(3)  PORT_NAME("P2 Coin (1 credit)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Pay Out")  PORT_CODE(KEYCODE_U)

	PORT_START("INP1.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Bet 1-2") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Bet 1-3") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Bet 1-4") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Bet 1-5") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INP1.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Bet 1-6") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Bet 2-3") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Bet 2-4") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Bet 2-5") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INP1.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Bet 2-6") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Bet 3-4") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Bet 3-5") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Bet 3-6") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INP1.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Bet 4-5") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )      PORT_NAME("P1 Start")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Big") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Next Game") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Bet 4-6") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INP1.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Take Score") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Small") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 SH") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INP1.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 Bet 5-6") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INP2.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Bet 1-2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Bet 1-3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Bet 1-4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Bet 1-5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INP2.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Bet 1-6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Bet 2-3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Bet 2-4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Bet 2-5")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INP2.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Bet 2-6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Bet 3-4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Bet 3-5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Bet 3-6")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INP2.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Bet 4-5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )      PORT_NAME("P2 Start")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Next Game")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Bet 4-6")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INP2.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Take Score")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Small")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 SH")
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INP2.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 Bet 5-6")
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown))  PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown))  PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x0c, 0x0c, DEF_STR(Coinage))  PORT_DIPLOCATION("DSW:6,5")
	PORT_DIPSETTING(0x0c, "1 Coin 1 Credit / 1 Pulse 10 Credits")
	PORT_DIPSETTING(0x08, "1 Coin 2 Credit / 1 Pulse 20 Credits")
	PORT_DIPSETTING(0x04, "1 Coin 5 Credit / 1 Pulse 50 Credits")
	PORT_DIPSETTING(0x00, "1 Coin 10 Credit / 1 Pulse 100 Credits")
	PORT_DIPNAME(0x10, 0x10, "Payment")         PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(0x10, "Pay Out")
	PORT_DIPSETTING(0x00, "Hopper")
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown))  PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, "Clear Memory")    PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "Test Mode")       PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END


void tvg01_state::theboat(machine_config &config)
{
	z80_device &maincpu(Z80(config, "maincpu", CPU_CLOCK));  // LH0080A
	maincpu.set_addrmap(AS_PROGRAM, &tvg01_state::mem_map);
	maincpu.set_addrmap(AS_IO, &tvg01_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);  // D449C-2 + battery

	i8255_device &ppi1(I8255(config, "ppi1"));  // D8255AC-2
	ppi1.in_pa_callback().set_ioport("INP0");
	ppi1.out_pb_callback().set(FUNC(tvg01_state::bank_select_w));

	i8255_device &ppi2(I8255(config, "ppi2"));  // D8255AC-2
	ppi2.out_pa_callback().set(FUNC(tvg01_state::input_select_w<0>));
	ppi2.out_pb_callback().set(FUNC(tvg01_state::input_select_w<1>));
	ppi2.in_pc_callback().set(FUNC(tvg01_state::player_inputs_r));

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	v9938_device &vdp(V9938(config, "vdp", VDP_CLOCK));  // unknown type (surface-scratched 64-pin SDIP)
	vdp.set_screen_ntsc("screen");
	vdp.set_vram_size(0x20000);  // 4x MB81464-15
	vdp.int_cb().set_inputline("maincpu", INPUT_LINE_IRQ0);

	TICKET_DISPENSER(config, "hopper", attotime::from_msec(50));

	SPEAKER(config, "mono").front_center();

	ay8910_device &psg(AY8910(config, "psg", PSG_CLOCK));  // GI AY-3-8910A
	psg.port_a_read_callback().set_ioport("DSW");
	psg.add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START(theboat)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("1.ic3", 0x0000, 0x4000, CRC(04a0ef56) SHA1(40256f858032efa1375d336f141dbbd08a8802f7))
	ROM_LOAD("2.ic5", 0x4000, 0x4000, CRC(c18d4a61) SHA1(c40a8b7dcaa90ed871be20605fc853949361257e))

	ROM_REGION(0x10000, "banked_rom", 0) // contains both code and data
	ROM_LOAD("3.ic8",  0x0000, 0x4000, CRC(74b44e32) SHA1(b36c90a13511c5bf4aef079ac506605096e39067))
	ROM_LOAD("4.ic11", 0x4000, 0x4000, CRC(4ea36fa3) SHA1(b020a478e8dd72154916c67d71255b5a6a822d6d))
	ROM_LOAD("5.ic14", 0x8000, 0x4000, CRC(7899a587) SHA1(13cbb7e837e14bc49d8b34dbf876b666cdf48979))
	// Empty socket for one more ROM (IC16),
ROM_END

} // anonymous namespace


//   YEAR  NAME      PARENT  MACHINE   INPUT     STATE        INIT        ROT    COMPANY            FULLNAME   FLAGS
GAME(1987, theboat,  0,      theboat,  theboat,  tvg01_state, empty_init, ROT0, "Hit Gun Co, LTD", "The Boat", MACHINE_SUPPORTS_SAVE)
