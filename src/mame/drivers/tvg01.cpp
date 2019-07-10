// license:BSD-3-Clause
// copyright-holders:AJR, Roberto Fresca
/*******************************************************************************

  The Boat
  1997(c) Hit Gun Co, LTD.
  
  Gambling/Amusing game
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

*******************************************************************************

  Todo:

  - Find the selector for the PPI2 port C multiplexed inputs.
  - Demux the PPI2 port C inputs.


*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/v9938.h"
#include "screen.h"
#include "speaker.h"

class tvg01_state : public driver_device
{
public:
	tvg01_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void boatrace(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void mem_map(address_map &map);
	void io_map(address_map &map);
};

void tvg01_state::machine_start()
{
	membank("gfxbank")->configure_entries(0, 4, memregion("gfx")->base(), 0x4000);
}

void tvg01_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0xc000, 0xffff).bankr("gfxbank");
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

static INPUT_PORTS_START(boatrace)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )       PORT_NAME("P1 - Coin (1 credit)")                // P1 - coin (1 credit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )       PORT_NAME("P2 - Key Up (10 credits)")            // P2 - keyup (10 credits)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK)  PORT_NAME("Analyze / Bookkeeping")               // analyze (bookkeeping)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P2 - Key Out")  PORT_CODE(KEYCODE_W)  // P2 - keyout
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )       PORT_NAME("P1 - Key Up (10 credits)")            // P1 - keyup (10 credits)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Hopper")        PORT_CODE(KEYCODE_H)  // hopper line
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )       PORT_NAME("P2 - Coin (1 credit)")                // P2 - coin (1 credit)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("P1 - Key Out")  PORT_CODE(KEYCODE_Q)  // P1 - keyout

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("12, 16, 26, 45, 56, TS (take score)")  PORT_CODE(KEYCODE_Z)  // Both players: 12, 16, 26, 45, 56, TS (take score)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("13, 23, 34, ST (start), Small")        PORT_CODE(KEYCODE_X)  // Both players: 13, 23, 34, ST (start), Small
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("14, 24, 35, SH, BG (big)")             PORT_CODE(KEYCODE_C)  // Both players: 14, 24, 35, SH, BG (big)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("15, 25, 36, NG (next game)")           PORT_CODE(KEYCODE_V)  // Both players: 15, 25, 36, NG (next game)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("46")     PORT_CODE(KEYCODE_B)  // Both players: 46
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Unk-1")  PORT_CODE(KEYCODE_N)  // unknown
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Unk-2")  PORT_CODE(KEYCODE_M)  // unknown
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Unk-3")  PORT_CODE(KEYCODE_S)  // unknown

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
	PORT_DIPNAME(0x10, 0x10, "Key Out")         PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown))  PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, "Unknown (Should be OFF)")  PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "Test Mode")       PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END

void tvg01_state::boatrace(machine_config &config)
{
	z80_device &maincpu(Z80(config, "maincpu", 21.477272_MHz_XTAL / 6)); // LH0080A
	maincpu.set_addrmap(AS_PROGRAM, &tvg01_state::mem_map);
	maincpu.set_addrmap(AS_IO, &tvg01_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // D449C-2 + battery

	i8255_device &ppi1(I8255(config, "ppi1")); // D8255AC-2
	ppi1.in_pa_callback().set_ioport("IN0");
	ppi1.out_pb_callback().set_membank("gfxbank").mask(0x03);

	i8255_device &ppi2(I8255(config, "ppi2")); // D8255AC-2
	ppi2.in_pc_callback().set_ioport("IN1");

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	v9938_device &vdp(V9938(config, "vdp", 21.477272_MHz_XTAL)); // unknown type (surface-scratched 64-pin SDIP)
	vdp.set_screen_ntsc("screen");
	vdp.set_vram_size(0x20000); // 4x MB81464-15
	vdp.int_cb().set_inputline("maincpu", INPUT_LINE_IRQ0);

	SPEAKER(config, "mono").front_center();

	ay8910_device &psg(AY8910(config, "psg", 21.477272_MHz_XTAL / 12)); // GI AY-3-8910A
	psg.port_a_read_callback().set_ioport("DSW");
	psg.add_route(ALL_OUTPUTS, "mono", 1.0);
}

ROM_START(boatrace)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("1.ic3", 0x0000, 0x4000, CRC(04a0ef56) SHA1(40256f858032efa1375d336f141dbbd08a8802f7))
	ROM_LOAD("2.ic5", 0x4000, 0x4000, CRC(c18d4a61) SHA1(c40a8b7dcaa90ed871be20605fc853949361257e))

	ROM_REGION(0x10000, "gfx", 0)
	ROM_LOAD("3.ic8",  0x8000, 0x4000, CRC(74b44e32) SHA1(b36c90a13511c5bf4aef079ac506605096e39067))
	ROM_LOAD("4.ic11", 0x4000, 0x4000, CRC(4ea36fa3) SHA1(b020a478e8dd72154916c67d71255b5a6a822d6d))
	ROM_LOAD("5.ic14", 0xc000, 0x4000, CRC(7899a587) SHA1(13cbb7e837e14bc49d8b34dbf876b666cdf48979))
	// Empty socket for one more ROM (IC16), 
ROM_END

//   YEAR  NAME      PARENT  MACHINE   INPUT     STATE        INIT        ROT    COMPANY            FULLNAME   FLAGS
GAME(1987, boatrace, 0,      boatrace, boatrace, tvg01_state, empty_init, ROT0, "Hit Gun Co, LTD", "The Boat", MACHINE_NOT_WORKING)
