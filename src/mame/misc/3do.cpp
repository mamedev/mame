// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Wilbert Pol
/**************************************************************************************************

3do.cpp

Driver file to handle emulation of the 3DO systems

TODO:
- Incomplete XBus/CD drive semantics
\- 3do disks fail to be recognized, reads Avatar structure, decides they aren't worth and moves on;
\- Photo CD bails out at startup with error -8021 (unless A is held on 3do_fz10e & Sanyo based
   romsets, where first picture is loaded then bails out anyway)
\- Audio CD black screen (needs DSPP irq), has plenty of Cel, VDLP and sport issues later
   (reads random port, asks for audio tracks *with* subcode);
- Incomplete DSPP mapping
\- Most notably it should restart on counter reloads (bp 38,1,{pc=0;g} to bypass hang in 3do_fz1)
- Replace ARM7 with ARM60;
- Fix VRAM size (should be 1 MB, but every single BIOS fails to boot with that, wrong ARM type?);
- CEL engine should really halt main CPU when running, paused only when irqs are taken;
- MMU (user programs will need it);
- 3do_fz1: black screen after insert disk screen (needs DSPP irq);
- 3do_hc21 (bios 0): some intermediate garbage on top-left of CELs;
- 3do_gdo101: errors on DSPP semaphore, hacked to make it boot;
- 3do_try, 3do_hc21 (bios 1): throws "QueueSport error on cmd 4: xfer across 1M boundary",
  has issues with layer clearances, never really pings Sport DMA, needs smaller VRAM?
- 3do_fc2: same as above
- 3do_fc1: hangs on OpenDiskFile at PC=2e6dc, path="/rom/system/tasks/shell", will "give up" if
  skipped.

References:
- https://wiki.console5.com/wiki/Panasonic_3DO_FZ-1
- https://github.com/trapexit/portfolio_os

Hardware descriptions:

Processors:
- 32bit 12.5MHZ RISC CPU (ARM60 - ARM6 core)
- Separate BUS for video refresh updates (VRAM is dual ported)
- Super Fast BUS Speed (50 Megabytes per second)
- Math Co-Processor custom designed by NTG for accelerating fixed-point
  matrix operations (_not_ the ARM FPA)
- Multitasking 32-bit operating system

Resolution:
- 640x480 pixel resolution
- 16.7 million colors

Two accelerated video co-processors:
- 25MHZ clock rate (NTSC), 29.5MHZ clock rate (PAL)
- Capable of producing 9-16 million real pixels per second (36-64 Mpix/sec
  interpolated), distorted, scaled, rotated and texture mapped.
- able to map a rectangular bitmap onto any arbitrary 4-point polygon.
- texturemap source bitmaps can be 1, 2, 4, 6, 8, or 16 bits per pixel and
  are RLE compressed for a maximum combination of both high resolution and
  small storage space.
- supports transparency, translucency, and color-shading effects.

Custom 16bit DSP:
- specifically designed for mixing, manipulating, and synthesizing CD quality
  sound.
- can decompress sound 2:1 or 4:1 on the fly saving memory and bus bandwidth.
- 25MHz clock rate.
- pipelined CISC architecture
- 16bit register size
- 17 separate 16bit DMA channels to and from system memory.
- on chip instruction SRAM and register memory.
- 20bit internal processing.
- special filtering capable of creating effects such as 3D sound.

Sound:
- 16bit stereo sound
- 44.1 kHz sound sampling rate
- Fully support Dolby(tm) Surround Sound

Memory:
- 2 megabytes of DRAM
- 1 megabyte of VRAM (also capable of holding/executing code and data)
- 1 megabyte of ROM
- 32KB battery backed SRAM

CD-ROM drive:
- 320ms access time
- double speed 300kbps data transfer
- 32KB RAM buffer

Ports:
- 2 expansion ports:
  - 1 high-speed 68 pin x 1 AV I/O port (for FMV cartridge)
  - 1 high-speed 30 pin x 1 I/O expansion port
- 1 control port, capable of daisy chaining together up to 8 peripherals

Models:
- Panasonic FZ-1 R.E.A.L. 3DO Interactive Multiplayer (Japan, Asia, North America, Europe)
- Panasonic FZ-10 R.E.A.L. 3DO Interactive Multiplayer (Japan, North America, Europe)
- Goldstar 3DO Interactive Multiplayer (South Korea, North America, Europe)
- Goldstar 3DO ALIVE II (South Korea)
- Samsung DMB-800 (South Korea)
- Sanyo TRY 3DO Interactive Multiplayer (Japan)
- Creative 3DO Blaster - PC Card (ISA)
- Panasonic N-1005 "Robo" 3DO (Japan), based on FZ-1 with 5x CD media changer and VCD adapter
  built-in
- a Scientific Atlanta Set Top Terminal, with a Nicky device in BIGTRACE space

===================================================================================================

Part list of Goldstar 3DO Interactive Multiplayer

- X1 = 50.0000 MHz KONY 95-08 50.0000 KCH089C
- X2 = 59.0000 MHz KONY 95-21 59.0000 KCH089C (NTSC would use 49.09MHz)
- IC303 BOB = 3DO BOB ADG 00919-001-IC 517A4611 - 100 pins
- IC1 ANVIL = 3DO Anvil rev4 00745-004-02 521U5L36 - 304 pins
- IC302 DSP = SONY CXD2500BQ 447HE5V - 80 pins
- IC601 ADAC = BB PCM1710U 9436 GG2553 - 28 pins
- X601 16.934MHz = 16.93440 KONY
- IC101/102/103/104 DRAM = Goldstar GM71C4800AJ70 9520 KOREA - 28 pins
- IC105/106/107/108 VRAM = Toshiba TC528267J-70 9513HBK - 40 pins
- IC3 ROM = Goldstar [202M] GM23C8000AFW-325 9524 - 32 pins
- IC4 SRAM = Goldstar GM76C256ALLFW70 - 28 pins
- IC2 ARM = ARM P60ARMCP 9516C - 100 pins
- IC6 = Philips 74HCT14D 974230Q - 14 pins
- IC301 u-COM = MC68HSC 705C8ACFB 3E20T HLAH9446 - 44 pins

**************************************************************************************************/

#include "emu.h"
#include "3do.h"

#include "cpu/arm/arm.h"
#include "cpu/arm7/arm7.h"
#include "imagedev/cdromimg.h"

#include "softlist_dev.h"
#include "speaker.h"


#define DIAG_ENABLE     0

#define X2_CLOCK_PAL    59000000
#define X2_CLOCK_NTSC   49090000
#define X601_CLOCK      XTAL(16'934'400)


void _3do_state::main_mem(address_map &map)
{
	map(0x0000'0000, 0x001F'FFFF).ram();
	map(0x0000'0000, 0x001F'FFFF).view(m_overlay_view);
	m_overlay_view[0](0x0000'0000, 0x001F'FFFF).rom().region("bios", 0).lw8(NAME([this] (offs_t offset) { m_overlay_view.disable(); }));
	map(0x0020'0000, 0x003F'FFFF).ram().share(m_vram);                                   /* VRAM */
	map(0x0300'0000, 0x030F'FFFF).m(m_bankdev, FUNC(address_map_bank_device::amap32));   /* BIOS */
	// slow bus
	map(0x0310'0000, 0x0313'FFFF).ram();                                                 /* Brooktree? */
	map(0x0314'0000, 0x0315'FFFF).mirror(0x20000).rw(FUNC(_3do_state::nvarea_r), FUNC(_3do_state::nvarea_w)).umask32(0x000000ff);                /* NVRAM */
	map(0x0318'0000, 0x031B'FFFF).rw(FUNC(_3do_state::slow2_r), FUNC(_3do_state::slow2_w));               /* Slow bus - additional expansion */
	// Sport
	map(0x0320'0000, 0x0320'FFFF).rw(FUNC(_3do_state::svf_r), FUNC(_3do_state::svf_w));                   /* special vram access1 */
	map(0x0330'0000, 0x0330'07FF).m(m_madam, FUNC(madam_device::map));              /* address decoder */
	map(0x0340'0000, 0x0340'3FFF).m(m_clio, FUNC(clio_device::map));                /* io controller */
	map(0x0340'C000, 0x0340'FFFF).m(*this, FUNC(_3do_state::uncle_map));
//  map(0x0360'0000, 0X037F'FFFF) trace
//      map(0x0370'0000, 0X037E'FFFF) SRAM
//      map(0X037F'FF00, 0X037F'FF0B) link data/address/FIFO
//      map(0X037F'FF0C, 0X037F'FF0F) joysticks
//      map(0x037F'0000, 0x0373'FFFF) debug ROM
//  map(0x0380'0000, 0x03??'????) trace big RAM
}

void _3do_state::bios_mem(address_map &map)
{
	map(0x0000'0000, 0x000F'FFFF).rom().region("bios", 0);
	map(0x0010'0000, 0x001F'FFFF).rom().region("kanji", 0);
}

static INPUT_PORTS_START( 3do )
	PORT_START("P1.0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // ID
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 A") PORT_PLAYER(1)

	PORT_START("P1.1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 B") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 C") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME(u8"P1 P \u23f5/\u23f8") // Play/Pause
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME(u8"P1 X \u23f9") PORT_PLAYER(1) // Stop
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 RT") // Right Trigger
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 LT") // Left Trigger
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void _3do_state::machine_start()
{
	m_nvram->set_base(&m_nvmem, sizeof(m_nvmem));

	m_slow2_init();
	m_uncle.rev = 0x03800000;

	save_item(NAME(m_svf.sport));
}

void _3do_state::machine_reset()
{
	// start with overlay enabled, and bank pointing at BIOS
	m_overlay_view.select(0);
	m_bankdev->set_bank(0);
}


// TODO: clocks (doubled vs. ARM?)
void _3do_state::green_config(machine_config &config)
{
	MADAM(config, m_madam, XTAL(50'000'000)/4);
	m_madam->diag_cb().set([] (u8 data) {
		// Logic Analyser (sic) ping, supposedly repeated in ZSIO debug port
		if (DIAG_ENABLE)
		{
			if(data == 0x0a)
				printf("\n");
			else
				printf("%c", data & 0xff);
		}
	});
	m_madam->dma8_read_cb().set([this] (offs_t offset) {
		address_space &space = m_maincpu->space();
		u8 ret = space.read_byte(offset);
		return ret;
	});
	m_madam->dma32_read_cb().set([this] (offs_t offset) {
		address_space &space = m_maincpu->space();
		u32 ret = space.read_dword(offset, 0xffff'ffff);
		return ret;
	});
	m_madam->dma32_write_cb().set([this] (offs_t offset, u32 data) {
		address_space &space = m_maincpu->space();
		space.write_dword(offset, data, 0xffff'ffff);
	});
	// TODO: disregard enable and cmd, those needs to be from xbus
	m_madam->dma_exp_read_cb().set([this] () {
		// ... in particular, 3do_fz1j and audio CD player will deselect during a DMA transfer (?)
		m_cdrom->enable_w(0);
		m_cdrom->cmd_w(1);
		u8 res = m_cdrom->read();
		m_cdrom->cmd_w(0);
		return res;
	});
	m_madam->arm_ctl_cb().set(m_clio, FUNC(clio_device::arm_ctl_w));
	m_madam->irq_dexp_cb().set(m_clio, FUNC(clio_device::dexp_w));
	m_madam->playerbus_read_cb().set([this] (offs_t offset) -> u32 {
		if (offset == 0)
			return (m_p1_r[0]->read() << 24) | (m_p1_r[1]->read() << 16);

		return 0;
	});
	m_madam->irq_dply_cb().set(m_clio, FUNC(clio_device::dply_w));
	m_madam->set_amy_tag("amy");

	CLIO(config, m_clio, XTAL(50'000'000)/4);
	m_clio->firq_cb().set([this] (int state) {
		m_maincpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
		//if (state)
		//  m_maincpu->pulse_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, m_maincpu->minimum_quantum_time());
	});
	m_clio->set_screen_tag("screen");
	m_clio->xbus_sel_cb().set([this] (u8 data) {
		m_cdrom->enable_w(data != 0);
	});
	m_clio->xbus_read_cb().set([this] (offs_t offset) -> u8 {
		if (offset == 0)
		{
			return m_cdrom->read();
		}

		return 0;
	});
	m_clio->xbus_write_cb().set([this] (offs_t offset, u8 data) {
		if (offset == 0)
		{
			m_cdrom->cmd_w(0);
			m_cdrom->write(data);
			return;
		}
		m_cdrom->cmd_w(1);
	});
	m_clio->exp_dma_enable_cb().set(m_madam, FUNC(madam_device::exp_dma_req_w));
	m_clio->vsync_cb().set(m_madam, FUNC(madam_device::vdlp_start_w));
	m_clio->hsync_cb().set(m_madam, FUNC(madam_device::vdlp_continue_w));
	m_clio->adb_out_cb<2>().set([this] (int state) { m_bankdev->set_bank(state & 1); });

	ADDRESS_MAP_BANK(config, m_bankdev).set_map(&_3do_state::bios_mem).set_options(ENDIANNESS_BIG, 32, (20 + 1), 0x100000);

	AMY(config, m_amy, XTAL(50'000'000)/4);
	m_amy->set_screen("screen");

	CR560B(config, m_cdrom, 0);
	m_cdrom->add_route(0, "speaker", 1.0, 0);
	m_cdrom->add_route(1, "speaker", 1.0, 1);
	m_cdrom->set_interface("cdrom");
//  m_cdrom->scor_cb().set(m_clio, FUNC(clio_device::xbus...)).invert();
//  m_cdrom->stch_cb().set(m_clio, FUNC(clio_device::xbus...)).invert();
//  m_cdrom->sten_cb().set(m_clio, FUNC(clio_device::xbus_rdy_w)).invert();
	m_cdrom->sten_cb().set(m_clio, FUNC(clio_device::xbus_int_w)).invert();
	m_cdrom->drq_cb().set(m_clio, FUNC(clio_device::xbus_wr_w));

	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac[0], 0).add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac[1], 0).add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	m_clio->dacl_cb().set(m_dac[0], FUNC(dac_16bit_r2r_twos_complement_device::write));
	m_clio->dacr_cb().set(m_dac[1], FUNC(dac_16bit_r2r_twos_complement_device::write));
}

void _3do_state::_3do(machine_config &config)
{
	/* Basic machine hardware */
	ARM7_BE(config, m_maincpu, XTAL(50'000'000)/4); // DA86C06020XV
	m_maincpu->set_addrmap(AS_PROGRAM, &_3do_state::main_mem);
	m_maincpu->set_dasm_override(std::function(&portfolio_dasm_override), "portfolio_dasm_override");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	green_config(config);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// TODO: proper params (mostly running in interlace mode)
	// htotal=1592 according to page 36 of HW spec, this is off wrt 15.734 kHz spec
	// (half clocks during HSync?)
	m_screen->set_raw(X2_CLOCK_NTSC / 2, 1560, 254, 1534, 263, 22, 262);
	m_screen->set_screen_update(m_amy, FUNC(amy_device::screen_update));

	SPEAKER(config, "speaker", 2).front();

	SOFTWARE_LIST(config, "cdrom_list").set_original("3do");
	SOFTWARE_LIST(config, "photocd_list").set_compatible("photo_cd");
}

void _3do_state::_3do_pal(machine_config &config)
{
	/* Basic machine hardware */
	ARM7_BE(config, m_maincpu, XTAL(50'000'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &_3do_state::main_mem);
	m_maincpu->set_dasm_override(std::function(&portfolio_dasm_override), "portfolio_dasm_override");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	green_config(config);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// TODO: as above, actual params are unknown
	// assumed 15.625 kHz as per PAL spec, display range looks a bit off
	m_screen->set_raw(X2_CLOCK_PAL / 2, 1888, 254, 1790, 313, 22, 312);
	m_screen->set_screen_update(m_amy, FUNC(amy_device::screen_update));
	m_amy->set_is_pal(true);
	m_madam->set_is_pal(true);

	SPEAKER(config, "speaker", 2).front();

	SOFTWARE_LIST(config, "cdrom_list").set_original("3do");
	SOFTWARE_LIST(config, "photocd_list").set_compatible("photo_cd");
}

void _3do_state::arcade_ntsc(machine_config &config)
{
	_3do(config);
	m_cdrom->add_region("cdimage");
}



ROM_START(3do_fz1)
	ROM_REGION32_BE( 0x200000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "retail", "Retail FZ-1 USA" )
	ROMX_LOAD( "panafz1.bin", 0x000000, 0x100000, CRC(c8c8ff89) SHA1(34bf189111295f74d7b7dfc1f304d98b8d36325a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "dev", "Development FZ-1 USA (v0.9)" )
	ROMX_LOAD( "panafz1_dev_0.9.bin", 0x000000, 0x100000, CRC(b5a5d0a8) SHA1(de3c55490733e6c69724d87e149b52ed955638ed), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "deva", "Development FZ-1 USA (later)" )
	ROMX_LOAD( "panafz1_dev.bin", 0x000000, 0x100000, CRC(e8eba9dd) SHA1(4cb4ee36e0f5bc0995d34992b4f241c420d49b2e), ROM_BIOS(2) )

	ROM_REGION32_BE( 0x100000, "kanji", ROMREGION_ERASEFF )
ROM_END

ROM_START(3do_fz1e)
	ROM_REGION32_BE( 0x200000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "retail", "Retail FZ-1 Europe" )
	ROMX_LOAD( "panafz1e.bin", 0x000000, 0x100000, CRC(a191e1aa) SHA1(1d0db81e171ebc1d07cefc8ce8ab082306186e56), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "unencrypted", "Unencrypted FZ-1 Europe" )
	ROMX_LOAD( "panafz1e-unencrypted.bin", 0x000000, 0x100000, CRC(d3d345df) SHA1(4696951e492e5526772a860ea2c0f35411a80927), ROM_BIOS(1) )

	ROM_REGION32_BE( 0x100000, "kanji", ROMREGION_ERASEFF )
ROM_END

ROM_START(3do_fz1j)
	ROM_REGION32_BE( 0x200000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "retail", "Retail FZ-1 Japan" )
	ROMX_LOAD( "panafz1j.bin", 0x000000, 0x100000, CRC(d9493adc) SHA1(ec7ec62d60ec0459a14ed56ebc66761ef3c80efc), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "norsa", "FZ-1 Japan with disabled RSA" )
	ROMX_LOAD( "panafz1j-norsa.bin", 0x000000, 0x100000, CRC(82ce67c6) SHA1(a417587ae3b0b8ef00c830920c21af8bee88e419), ROM_BIOS(1) )

	ROM_REGION32_BE( 0x100000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "panafz1j-kanji.bin", 0x000000, 0x100000, CRC(45f478b1) SHA1(884515605ee243577ab20767ef8c1a7368e4e407) )
ROM_END

ROM_START(3do_fz10)
	ROM_REGION32_BE( 0x200000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "retail", "Retail FZ-10 USA" )
	ROMX_LOAD( "panafz10.bin", 0x000000, 0x100000, CRC(58242cee) SHA1(3c912300775d1ad730dc35757e279c274c0acaad), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "norsa", "FZ-10 USA with disabled RSA" )
	ROMX_LOAD( "panafz10-norsa.bin", 0x000000, 0x100000, CRC(230e6feb) SHA1(f05e642322c03694f06a809c0b90fc27ac73c002), ROM_BIOS(1) )

	ROM_REGION32_BE( 0x100000, "kanji", ROMREGION_ERASEFF )
ROM_END

ROM_START(3do_fz10e)
	ROM_REGION32_BE( 0x200000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "retail", "Retail FZ-10 Europe" )
	ROMX_LOAD( "panafz10e-anvil.bin", 0x000000, 0x100000, CRC(2495c500) SHA1(a900371f0cdcdc03f79557f11d406fd71251a5fd), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "norsa", "FZ-10 Europe with disabled RSA" )
	ROMX_LOAD( "panafz10e-anvil-norsa.bin", 0x000000, 0x100000, CRC(9a186221) SHA1(2765c7b4557cc838b32567d2428d088980295159), ROM_BIOS(1) )

	ROM_REGION32_BE( 0x100000, "kanji", ROMREGION_ERASEFF )
ROM_END

// TODO: supposedly this is a pre-Anvil model (kanji ROM may not fit)
ROM_START(3do_fz10j)
	ROM_REGION32_BE( 0x200000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "retail", "Retail FZ-10 Japan" )
	ROMX_LOAD( "panafz10j.bin", 0x000000, 0x100000, CRC(07b50015) SHA1(fe7f9c9c6a98910013bf13f2cf798de9fea52acd), ROM_BIOS(0) )

	ROM_REGION32_BE( 0x100000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "panafz10ja-anvil-kanji.bin", 0x000000, 0x100000, CRC(ff7393de) SHA1(2e857b957803d0331fd229328df01f3ffab69eee) )
ROM_END

// TODO: was labeled "GDO-101P", mistake or it's the PAL version?
// TODO: may need renaming when the "Alive" South Korean version ever surfaces
ROM_START(3do_gdo101)
	ROM_REGION32_BE( 0x200000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "gdo101m", "Retail GDO-101M" )
	ROMX_LOAD( "goldstar.bin", 0x000000, 0x100000, CRC(b6f5028b) SHA1(c4a2e5336f77fb5f743de1eea2cda43675ee2de7), ROM_BIOS(0) )

	ROM_REGION32_BE( 0x100000, "kanji", ROMREGION_ERASEFF )
ROM_END

// NOTE: prints an extra "goldstar-fc1 encrypted" on screen, doesn't ping the logic analyzer
// Launch BIOS
ROM_START(3do_fc1)
	ROM_REGION32_BE( 0x200000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "fc1", "FC-1 (encrypted, retail?)" )
	ROMX_LOAD( "goldstar_fc1_enc.bin", 0x000000, 0x100000, CRC(5c5b4f98) SHA1(8ef7503c948314d242da47b7fdc272f68dac2aee), ROM_BIOS(0) )

	ROM_REGION32_BE( 0x100000, "kanji", ROMREGION_ERASEFF )
ROM_END

// devstation unit, prints "3DO-NTSC-1.0fc2 encrypted" and Logic Analyzer stuff directly OSD.
// "they're labeled as `3DO Station` units, or if has ethernet capability it'd be `3DO Network Station`
//  they're very large motherboards housed inside a standard PC case"
ROM_START(3do_fc2)
	ROM_REGION32_BE( 0x200000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "fc2", "FC-2 (1.0 dev kit)" )
	ROMX_LOAD( "3do_devkit_1.0fc2.bin", 0x000000, 0x100000, CRC(cdb23167) SHA1(bd325c869e1dde8a3872fc21565e0646a3d5b525), ROM_BIOS(0) )

	ROM_REGION32_BE( 0x100000, "kanji", ROMREGION_ERASEFF )
ROM_END

ROM_START(3do_try)
	ROM_REGION32_BE( 0x200000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "retail", "Retail IMP-21J TRY Japan" )
	ROMX_LOAD( "sanyotry.bin", 0x000000, 0x100000, CRC(d5cbc509) SHA1(b01c53da256dde43ffec4ad3fc3adfa8d635e943), ROM_BIOS(0) )

	// baddump: the actual kanji ROM for this model needs dumping
	ROM_REGION32_BE( 0x100000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "panafz1j-kanji.bin", 0x000000, 0x100000, BAD_DUMP CRC(45f478b1) SHA1(884515605ee243577ab20767ef8c1a7368e4e407) )
ROM_END

// model number "MPHC2100USA"
ROM_START(3do_hc21)
	ROM_REGION32_BE( 0x200000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "alpha", "alpha 21 March 94" )
	ROMX_LOAD( "sanyo_hc21_alpha.bin", 0x000000, 0x100000, CRC(a9f2f749) SHA1(29c40515dc1174ff13975baa59eb532083e4a3d3), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "b3", "b3 unencrypted" )
	ROMX_LOAD( "sanyo_hc21_b3_unenc.bin", 0x000000, 0x100000, CRC(c4c3db01) SHA1(c389af32bcadf0d86826927dc3d20b7072f90069), ROM_BIOS(1) )

	ROM_REGION32_BE( 0x100000, "kanji", ROMREGION_ERASEFF )
ROM_END


// Arcade section
// TODO: still using the old BIOS scheme, determine what they actually used for Orbatak
#define NTSC_BIOS \
	ROM_REGION32_BE( 0x200000, "bios", 0 ) \
	ROM_SYSTEM_BIOS( 0, "panafz10", "Panasonic FZ-10 R.E.A.L. 3DO Interactive Multiplayer" ) \
	ROMX_LOAD( "panafz10.bin", 0x000000, 0x100000, CRC(58242cee) SHA1(3c912300775d1ad730dc35757e279c274c0acaad), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "goldstar", "Goldstar 3DO Interactive Multiplayer v1.01m" ) \
	ROMX_LOAD( "goldstar.bin", 0x000000, 0x100000, CRC(b6f5028b) SHA1(c4a2e5336f77fb5f743de1eea2cda43675ee2de7), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS( 2, "panafz1", "Panasonic FZ-1 R.E.A.L. 3DO Interactive Multiplayer" ) \
	ROMX_LOAD( "panafz1.bin", 0x000000, 0x100000, CRC(c8c8ff89) SHA1(34bf189111295f74d7b7dfc1f304d98b8d36325a), ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS( 3, "sanyotry", "Sanyo TRY 3DO Interactive Multiplayer" ) \
	ROMX_LOAD( "sanyotry.bin", 0x000000, 0x100000, CRC(d5cbc509) SHA1(b01c53da256dde43ffec4ad3fc3adfa8d635e943), ROM_BIOS(3) ) \
	ROM_REGION32_BE( 0x100000, "kanji", ROMREGION_ERASEFF )


ROM_START(3dobios)
	NTSC_BIOS
ROM_END


ROM_START(orbatak)
	NTSC_BIOS

	DISK_REGION( "cdimage" )
	DISK_IMAGE_READONLY( "orbatak", 0, SHA1(25cb3b889cf09dbe5faf2b0ca4aae5e03453da00) )
ROM_END

#define ALG_BIOS \
	ROM_REGION32_BE( 0x200000, "bios", 0 ) \
	/* TC544000AF-150, 1xxxxxxxxxxxxxxxxxx = 0xFF */ \
	ROM_LOAD( "saot_rom2.bin", 0x000000, 0x80000, CRC(b832da9a) SHA1(520d3d1b5897800af47f92efd2444a26b7a7dead) )  \
	ROM_REGION32_BE( 0x100000, "kanji", ROMREGION_ERASEFF )

ROM_START(alg3do)
	ALG_BIOS
ROM_END


ROM_START(md23do)
	ALG_BIOS

	DISK_REGION( "cdimage" )
	DISK_IMAGE_READONLY( "mad dog ii", 0, SHA1(0117c1fd279f42e942648ca55fa75dd45da37a4f) )
ROM_END

ROM_START(sht3do)
	ALG_BIOS

	DISK_REGION( "cdimage" )
	DISK_IMAGE_READONLY( "shootout at old tucson", 0, SHA1(bd42213c6b460b5b6153a8b2b41d0a114171e86e) )
ROM_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

// console section
// Panasonic
CONS( 1993, 3do_fz1,    0,          0,       _3do,       3do,    _3do_state, empty_init, "Panasonic", "3DO FZ-1 R.E.A.L. Interactive Multiplayer (USA)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING )
CONS( 1993, 3do_fz1e,   3do_fz1,    0,       _3do_pal,   3do,    _3do_state, empty_init, "Panasonic", "3DO FZ-1 R.E.A.L. Interactive Multiplayer (Europe)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING )
CONS( 1994, 3do_fz1j,   3do_fz1,    0,       _3do,       3do,    _3do_state, empty_init, "Panasonic", "3DO FZ-1 R.E.A.L. Interactive Multiplayer (Japan)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING )
CONS( 1994, 3do_fz10,   0,          0,       _3do,       3do,    _3do_state, empty_init, "Panasonic", "3DO FZ-10 R.E.A.L. Interactive Multiplayer (USA)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING )
CONS( 1994, 3do_fz10e,  3do_fz10,   0,       _3do_pal,   3do,    _3do_state, empty_init, "Panasonic", "3DO FZ-10 R.E.A.L. Interactive Multiplayer (Europe, Anvil chipset)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING )
CONS( 1994, 3do_fz10j,  3do_fz10,   0,       _3do,       3do,    _3do_state, empty_init, "Panasonic", "3DO FZ-10 R.E.A.L. Interactive Multiplayer (Japan)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING )
// Goldstar
CONS( 1994, 3do_gdo101, 0,          0,       _3do,       3do,    _3do_state, empty_init, "Goldstar",  "3DO GDO-101M Interactive Multiplayer (USA?)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING )
CONS( 1994?,3do_fc1,    3do_gdo101, 0,       _3do,       3do,    _3do_state, empty_init, "Goldstar",  "3DO FC-1 Interactive Multiplayer (USA)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING )
CONS( 1994?,3do_fc2,    3do_gdo101, 0,       _3do,       3do,    _3do_state, empty_init, "Goldstar?", "3DO FC-2 Interactive Multiplayer (dev kit)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING )
// Sanyo
CONS( 1995, 3do_try,    0,          0,       _3do,       3do,    _3do_state, empty_init, "Sanyo", "3DO IMP-21J TRY Interactive Multiplayer (Japan)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING )
CONS( 1994, 3do_hc21,   3do_try,    0,       _3do,       3do,    _3do_state, empty_init, "Sanyo", "3DO HC-21 Interactive Multiplayer (USA, prototype)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING )


// Arcade section
GAME( 1993, 3dobios, 0,       _3do,           3do,   _3do_state, empty_init, ROT0,     "The 3DO Company",      "3DO BIOS",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING | MACHINE_IS_BIOS_ROOT )

GAME( 1995, orbatak, 3dobios, arcade_ntsc,    3do,   _3do_state, empty_init, ROT0,     "American Laser Games", "Orbatak (prototype)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING )
// Beavis and Butthead (prototype), with "proprietary" CD drive according to pitch deck
// (likely not Jaguar CD derived because seems to work with stock 3do drive anyway)


// American Laser Games uses its own BIOS (with additional "FKr-Severe-System-extended-RSA failed in CreateTask")
GAME( 1993, alg3do, 0,       _3do,           3do,   _3do_state, empty_init, ROT0,     "American Laser Games / The 3DO Company", "ALG 3DO BIOS",            MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING | MACHINE_IS_BIOS_ROOT )

GAME( 199?, md23do,  alg3do, arcade_ntsc,    3do,   _3do_state, empty_init, ROT0,     "American Laser Games", "Mad Dog II: The Lost Gold (3DO hardware)", MACHINE_NOT_WORKING  | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING )
GAME( 1994, sht3do,  alg3do, arcade_ntsc,    3do,   _3do_state, empty_init, ROT0,     "American Laser Games", "Shootout at Old Tucson (3DO hardware)", MACHINE_NOT_WORKING  | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING )

