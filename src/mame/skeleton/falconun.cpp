// license:BSD-3-Clause
// copyright-holders: Mark Beckford


#include "emu.h"

#include "cpu/m6800/m6800.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/6821pia.h"
#include "machine/input_merger.h"

#include "screen.h"


/*
Unknown Falcon\Williams host & satellite terminal gambling game.

From a unfunctional set of 3 host PCBs and 5 matching Falcon terminal PCBs
and auxiliary PCBs without any ROM.

Components were stripped & every PCB was missing ROMs and CPUs.
Recovered ROMSs are from a mixture of PCBs and could be incomplete.
*/


namespace {

class falconun_state : public driver_device
{
public:
	falconun_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void falconun(machine_config &config);

private:
	required_device<m6802_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	void memmap(address_map &map) ATTR_COLD;
	void slavemap(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return 0; }
};

static INPUT_PORTS_START( falconun )
INPUT_PORTS_END

void falconun_state::memmap(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x2003).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc000, 0xffff).rom().region("falcon_terminal", 0xc000);
}

void falconun_state::slavemap(address_map &map)
{
	map(0x000, 0xfff).rom().region("slave", 0);
}

void falconun_state::falconun(machine_config &config)
{
	M6802(config, m_maincpu, 40'000'000 / 4); // TODO 10 MHz for a M6802??
	m_maincpu->set_addrmap(AS_PROGRAM, &falconun_state::memmap);
	m_maincpu->set_ram_enable(false);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, M6802_IRQ_LINE);

	pia6821_device &pia(PIA6821(config, "pia"));
	pia.irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));
	pia.irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	i8035_device &slavemcu(I8035(config, "slavemcu", 6'000'000));
	slavemcu.set_addrmap(AS_PROGRAM, &falconun_state::slavemap);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 255-1);
	m_screen->set_screen_update(FUNC(falconun_state::screen_update));
}


ROM_START( falconun )
	ROM_REGION( 0x10000, "falcon_terminal", 0 )
	// STICKER FALCON 1985 FOR MC6802P
	ROM_LOAD( "falcon-83", 0x0000, 0x8000, CRC(49bf0180) SHA1(40ea3049dd35dc9b306fc516c7b814be025cd39f) ) // 1 PCB ONLY X2
	ROM_LOAD( "falcon-85", 0x8000, 0x8000, CRC(c34704ce) SHA1(853434087df664ed17e7043804fa9e961c5fe97f) ) // 1 PCB ONLY X2

	ROM_REGION( 0x40, "falcon_prom", 0 )
	ROM_LOAD( "falcon_rg", 0x00, 0x20, CRC(c9bc35f1) SHA1(78e76cbe72157b775cd24f36e1814e312d56af30) ) // 1 PCB ONLY
	ROM_LOAD( "falcon_b",  0x20, 0x20, CRC(3fb02480) SHA1(ace4eeb3a9b81d0890d2eb5c4e3cc2e874cfa88c) ) // 1 PCB ONLY

	ROM_REGION( 0x1000, "slave", 0 )
	// AUXILIARY SC - STICKER IO SLAVE CONTROL
	ROM_LOAD( "io_slave", 0x0000, 0x1000, CRC(ccd186b0) SHA1(822602f5141e326df8ed18e0436a0ec585b5cb9e) )

	ROM_REGION( 0x80000, "host", ROMREGION_ERASEFF )
	// UV WINDOWS EXPOSED!!! "WILLIAMS ELECTRONICS" PRINT NEAR EMPTY 68000? SOCKET
	ROM_LOAD16_BYTE( "host-201", 0x00000, 0x08000, CRC(2ea40e11) SHA1(73512af40c6739c4061409ee12cf47b7b03b683f) )
	ROM_LOAD16_BYTE( "host-202", 0x00001, 0x08000, CRC(8deefb05) SHA1(0c1fbf9e1e846dc9bc68a0f8ad505bdec9c08df5) )
	ROM_LOAD16_BYTE( "host-203", 0x10001, 0x08000, CRC(9982e4bd) SHA1(a2bbd9883e21bbf8e6d614d4e1ef60d66892236d) ) // MATCH ON 2 PCBs FF FILL ON 1
	ROM_LOAD16_BYTE( "host-204", 0x10000, 0x08000, CRC(0a24750a) SHA1(b40237c0b5165ed7fa30c40bfd42711441c1b8c7) )
	ROM_LOAD16_BYTE( "host-216", 0x20001, 0x08000, CRC(8714876b) SHA1(239ad5f28c45c2a2a995e5ed1712fd4da90bb9dc) ) // 1 PCB ONLY
	ROM_LOAD16_BYTE( "host-217", 0x20000, 0x08000, CRC(5ed16dc8) SHA1(d50550d7d6e51c0c81972829f4d0a7e4cc20878d) ) // 1 PCB ONLY
	ROM_LOAD16_BYTE( "host-219", 0x30001, 0x08000, CRC(752c326b) SHA1(99afbc96c0984e0964937c753efcacf2ff2ade32) ) // 1 PCB ONLY
	ROM_LOAD16_BYTE( "host-220", 0x30000, 0x08000, CRC(e1371d17) SHA1(f7c3e9aeb35192d00d83c886a9bf8dc1ec2a5c15) ) // 1 PCB ONLY, 1xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "host-221", 0x40001, 0x08000, CRC(ce8d42ed) SHA1(7bc014fdfe64c7648ce550e969b23c46aaabace0) ) // FIXED BITS (xxxxxxx0)
	ROM_LOAD16_BYTE( "host-222", 0x40000, 0x08000, CRC(b795c4c2) SHA1(7a6528516f6e28670f25068a293a12f360dc78a1) ) // MATCH ON 2 PCBs FF FILL ON 1
	ROM_LOAD16_BYTE( "host-223", 0x50001, 0x08000, CRC(9b66c1ca) SHA1(d70464fc457671682711eb47523c595ece5cf488) ) // FIXED BITS (xxxxx0x0)
	ROM_LOAD16_BYTE( "host-224", 0x50000, 0x08000, CRC(bdba0ed9) SHA1(066f3ee60c1922b6b5f75a9d6083eddf7b673c9b) )
	ROM_LOAD16_BYTE( "host-230", 0x60000, 0x08000, CRC(3828b6bc) SHA1(db985977f5d17b45f61c67e8f190581ddf8fee70) ) // MATCH ON 2 PCBs BROKEN PIN (SENSE ERROR) ON 1

	ROM_REGION( 0x800, "hostprom", 0 )
	// 1 PCB ONLY
	ROM_LOAD( "host-prom1", 0x000, 0x200, CRC(8368a869) SHA1(86b2562106d894b63a529ae11c6d7a27347d3654) )
	ROM_LOAD( "host-prom2", 0x200, 0x200, CRC(a5005f9a) SHA1(b33ccdc433d299fe3495f5fb2a5e604d8712881f) )
	ROM_LOAD( "host-prom3", 0x400, 0x200, CRC(700b24c1) SHA1(790aa04717e86828974afaa2ac743f37afa11021) )
	ROM_LOAD( "host-prom4", 0x600, 0x200, CRC(835d733a) SHA1(2ea0fe5698a34206e8fc0166e2af4e066d864866) )

	ROM_REGION( 0x200, "timing", 0 )
	ROM_LOAD( "prom", 0x000, 0x200, CRC(f8aeaa23) SHA1(f1245705a65795be1c15423fda99b3cf2c09200a) ) // X2
ROM_END

} // anonymous namespace

GAME( 198?, falconun, 0, falconun, falconun, falconun_state, empty_init, ROT0, "Falcon", "unknown Falcon gambling machine", MACHINE_IS_SKELETON )
