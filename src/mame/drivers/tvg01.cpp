// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for gambling game on "TVG01" PCB.

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
	PORT_START("DSW")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown))
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown))
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown))
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown))
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown))
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown))
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown))
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown))
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
	ppi1.out_pb_callback().set_membank("gfxbank").mask(0x03);

	i8255_device &ppi2(I8255(config, "ppi2")); // D8255AC-2
	ppi2.in_pc_callback().set_constant(0xff);

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	v9938_device &vdp(V9938(config, "vdp", 21.477272_MHz_XTAL)); // unknown type (surface-scratched 64-pin SDIP)
	vdp.set_screen_ntsc("screen");
	vdp.set_vram_size(0x40000); // 4x MB81464-15
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
	// Empty socket for one more ROM (IC16)
ROM_END

GAME(19??, boatrace, 0, boatrace, boatrace, tvg01_state, empty_init, ROT0, "<unknown>", "All-Japan Boat Race", MACHINE_NOT_WORKING) // title is tentative
