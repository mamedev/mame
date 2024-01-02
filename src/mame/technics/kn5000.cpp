// license:GPL2+
// copyright-holders:Felipe Sanches
/******************************************************************************

	Technics SX-KN5000 music keyboard driver

******************************************************************************/

#include "emu.h"
#include "bus/technics/hdae5000.h"
#include "cpu/tlcs900/tmp94c241.h"
#include "imagedev/floppy.h"
#include "machine/gen_latch.h"
#include "machine/upd765.h"
#include "screen.h"
#include "sound/beep.h"
#include "speaker.h"
#include "video/pc_vga.h"

class mn89304_vga_device : public svga_device
{
public:
    // construction/destruction
    mn89304_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
    virtual void device_reset() override;

    virtual void palette_update() override;
    virtual void recompute_params() override;
    virtual uint16_t offset() override;
};

DEFINE_DEVICE_TYPE(MN89304_VGA, mn89304_vga_device, "mn89304_vga", "MN89304 VGA")

// TODO: nothing is known about this, configured out of usage in here for now.
mn89304_vga_device::mn89304_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, MN89304_VGA, tag, owner, clock)
{
    // ...
}

void mn89304_vga_device::device_reset()
{
    svga_device::device_reset();
    svga.rgb8_en = 1;
}

// sets up mode 0, by default it will throw 155 Hz, assume divided by 3
void mn89304_vga_device::recompute_params()
{
    u8 xtal_select = (vga.miscellaneous_output & 0x0c) >> 2;
    int xtal;

    switch(xtal_select & 3)
    {
        case 0: xtal = XTAL(25'174'800).value() / 3; break;
        case 1: xtal = XTAL(28'636'363).value() / 3; break;
        case 2:
        default:
            throw emu_fatalerror("MN89304: setup ext. clock select");
    }

    recompute_params_clock(1, xtal);
}


void mn89304_vga_device::palette_update()
{
    // 4bpp RAMDAC
    for (int i = 0; i < 256; i++)
    {
        set_pen_color(
            i,
            pal4bit(vga.dac.color[3*(i & vga.dac.mask) + 0]),
            pal4bit(vga.dac.color[3*(i & vga.dac.mask) + 1]),
            pal4bit(vga.dac.color[3*(i & vga.dac.mask) + 2])
        );
    }
}

uint16_t mn89304_vga_device::offset()
{
    return svga_device::offset() << 3;
}


namespace {

class kn5000_state : public driver_device
{
public:
	kn5000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_fdc(*this, "fdc")
		, m_checking_device_led_cn11(*this, "checking_device_led_cn11")
		, m_extension(*this, "extension")
		, m_extension_view(*this, "extension_view")
	{ }

	void kn5000(machine_config &config);

private:
	required_device<tmp94c241_device> m_maincpu;
	required_device<tmp94c241_device> m_subcpu;
	required_device<upd72067_device> m_fdc;
	required_device<beep_device> m_checking_device_led_cn11;
	required_device<kn5000_extension_device> m_extension;
	memory_view m_extension_view;

	virtual void machine_start() override;
	virtual void machine_reset() override;

//	void maincpu_portb_w(offs_t offset, uint8_t data);
//	void maincpu_port5_w(offs_t offset, uint8_t data);
//	void maincpu_port6_w(offs_t offset, uint8_t data);
//	void maincpu_port7_w(offs_t offset, uint8_t data);
	void maincpu_mem(address_map &map);
	void subcpu_mem(address_map &map);
//	uint16_t tone_generator_r(offs_t offset);
//	void tone_generator_w(offs_t offset, uint16_t data);
};

/*
MSAR0: 0x1e MAMR0: 0x0f  start: 0x1e0000  mask: 0x01ffff (128kB) SRAM @ IC21
MSAR1: 0x10 MAMR1: 0x3f  start: 0x100000  mask: 0x0fffff (1MByte)
MSAR2: 0xc0 MAMR2: 0x7f  start: 0xc00000  mask: 0x3fffff (4MByte) table data ROM (also boot?)
MSAR3: 0x00 MAMR3: 0x1f  start: 0x000000  mask: 0x0fffff (1MByte) DRAM ?
MSAR4: 0x80 MAMR4: 0xff  start: 0x800000  mask: 0x7fffff (8MByte)
MSAR5: 0x00 MAMR5: 0xff  start: 0x000000  mask: 0x7fffff (8MByte)
              CS5: optional HSRAM at A22=0 A21=X A20=0 A19=0 (0x200000) (512k)
              CS5: optional HSROM at A22=0 A21=X A20=0 A19=1 (0x280000) (512k)
              CS5: custom data at A22=0 A21=X A20=1 (0x300000) (1MB)
              CS5: rhythm data at A22=1 (0x400000) (4MB)
*/

void kn5000_state::maincpu_mem(address_map &map)
{
	map(0x000000, 0x0fffff).ram(); // 1Mbyte = 2 * 4Mbit DRAMs @ IC9, IC10 (CS3)
	//FIXME: map(0x110000, 0x11ffff).m(m_fdc, FUNC(upd765a_device::map)); // Floppy Controller @ IC208
	//FIXME: map(0x120000, 0x12ffff).w(m_fdc, FUNC(upd765a_device::dack_w)); // Floppy DMA Acknowledge
	map(0x140000, 0x14ffff).r("to_maincpu_latch", FUNC(generic_latch_8_device::read)); // @ IC23
	map(0x140000, 0x14ffff).w("to_subcpu_latch", FUNC(generic_latch_8_device::write)); // @ IC22
	map(0x1703b0, 0x1703df).m("vga", FUNC(mn89304_vga_device::io_map)); // LCD controller @ IC206
	map(0x1a0000, 0x1bffff).rw("vga", FUNC(mn89304_vga_device::mem_linear_r), FUNC(mn89304_vga_device::mem_linear_w));
	map(0x1e0000, 0x1fffff).ram(); // 1Mbit SRAM @ IC21 (CS0)  Note: I think this is the message "ERROR in back-up SRAM"
	map(0x200000, 0x2fffff).view(m_extension_view);
	m_extension_view[0](0x200000, 0x27ffff).ram(); //optional hsram: 2 * 256k bytes Static RAM @ IC5, IC6 (CS5)
	m_extension_view[0](0x280000, 0x2fffff).rom(); // 512k bytes FLASH ROM @ IC4 (CS5)
	map(0x300000, 0x3fffff).rom().region("custom_data", 0); // 8MBit FLASH ROM @ IC19 (CS5)
	map(0x400000, 0x7fffff).rom().region("rhythm_data", 0); // 32MBit ROM @ IC14 (A22=1 and CS5)
	map(0xc00000, 0xdfffff).mirror(0x200000).rom().region("table_data", 0);//2 * 8MBit ROMs @ IC1, IC3 (CS2)
	map(0xe00000, 0xffffff).mask(0x1fffff).rom().region("program", 0); //2 * 8MBit FLASH ROMs @ IC4, IC6
}

void kn5000_state::subcpu_mem(address_map &map)
{
	//map(0x??0000, 0x??ffff).r("to_subcpu_latch", FUNC(generic_latch_8_device::read)); // @ IC22
	//map(0x??0000, 0x??ffff).w("to_maincpu_latch", FUNC(generic_latch_8_device::write)); // @ IC23
	//map(0x??????, 0x??????).rw(FUNC(kn5000_state::tone_generator_r), FUNC(kn5000_state::tone_generator_w)); // @ IC303
	//map(0x??????, 0x??????).rw(FUNC(kn5000_state::dsp1_r), FUNC(kn5000_state::dsp1_w)); // @ IC311
	//map(0x?00000, 0x?fffff).ram(); // 1Mbyte = 2 * 4Mbit DRAMs @ IC28, IC29
	//map(0x??0000, 0x?1ffff).rom().region("mask", 0); // 1Mbit MASK ROM @ IC30

	// This is not necessarily correct.
	// Just silencing oslog messages for the subcpu while we don't have a proper ROM dump.
	map(0xfe0000, 0xffffff).rom().region("mask", 0);

	//Note:
	// DSP2 @ IC302 uses a serial bus
}

static void kn5000_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

static INPUT_PORTS_START(kn5000)
	PORT_START("CN11")
	PORT_DIPNAME(0x01, 0x01, "Main CPU Checking Device")
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_DIPSETTING(   0x01, DEF_STR(Off))

	PORT_START("CN12")
	PORT_DIPNAME(0x01, 0x01, "Sub CPU Checking Device")
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_DIPSETTING(   0x01, DEF_STR(Off))
INPUT_PORTS_END

//void kn5000_state::maincpu_portb_w(offs_t offset, uint8_t data)
//{
//}

//void kn5000_state::maincpu_port5_w(offs_t offset, uint8_t data)
//{
//}

//void kn5000_state::maincpu_port6_w(offs_t offset, uint8_t data)
//{
//}

//void kn5000_state::maincpu_port7_w(offs_t offset, uint8_t data)
//{
//}

//uint16_t kn5000_state::tone_generator_r(offs_t offset){
//	// TODO: Implement-me!
//	return 0;
//}

//void kn5000_state::tone_generator_w(offs_t offset, uint16_t data){
//	// TODO: Implement-me!
//}

void kn5000_state::machine_start()
{
	if(m_extension)
	{
		m_extension->rom_map(m_extension_view[0], 0x280000, 0x2fffff);
		m_extension_view.select(0);
	}
	else
	{
		m_extension_view.disable();
	}
}

void kn5000_state::machine_reset()
{
	/* Setup beep */
	m_checking_device_led_cn11->set_state(0);
}

void kn5000_state::kn5000(machine_config &config)
{
	// Note: The CPU has an internal clock doubler
	TMP94C241(config, m_maincpu, 2 * 8_MHz_XTAL); // TMP94C241F @ IC5
	// Address bus is set to 32 bits by the pins AM1=+5v and AM0=GND
	m_maincpu->set_addrmap(AS_PROGRAM, &kn5000_state::maincpu_mem);
	// Interrupt 0: CLK on "to_maincpu_latch"
	// Interrupt 4: FDCINT
	// Interrupt 5: FDCIRQ
	// Interrupt 6: FDC.H/D
	// Interrupt 7: FDC.I/O
	// Interrupt 9: HDDINT
	// Interrupt A <edge>: ~CPSCK "Control Panel Serial Clock"
	// ~NMI: SNS
	// TC0: FDCTC
	//
	// m_maincpu->port?_write().set(FUNC(kn5000_state::maincpu_port?_w));
	//

	// PORT 7:
	//   bit 5 (~BUSRQ pin): RY/~BY pin of maincpu ROMs
	m_maincpu->port7_read().set([] { return (1 << 5); }); // checked at EF3735 (v10 ROM)

	// PORT 8:
	//   bit 6 (~WAIT pin) (input): Something involving VGA.RDY, FDC.DMAACK
	//                              and shift-register @ IC18

	// PORT A:
	//   bit 0: sub_cpu ~RESET / SRST

	// PORT C:
	//   bit 0 (input) = "check terminal" switch
	//   bit 1 (output) = "check terminal" LED
	m_maincpu->portc_read().set([this] { return ioport("CN11")->read(); });
	m_maincpu->portc_write().set([this] (u8 data) { m_checking_device_led_cn11->set_state(BIT(data, 1) == 0); });

	// PORT D:
	//   bit 0 (output) = FDCRST
	//   bit 6 (input) = FD.I/O
	m_maincpu->portd_write().set([this] (u8 data) { m_fdc->reset_w(BIT(data, 0)); });
	// TODO: bit 6!


	// PORT E:
	//   bit 0 (input) = +5v
	//   bit 2 (input) = HDDRDY
	//   bit 4 (?) = MICSNS
	m_maincpu->porte_read().set([] { return 1; }); //checked at EF05A6 (v10 ROM)
	                                                   // FIXME: Bit 0 should only be 1 if the optional hard-drive extension board is disabled

	// PORT F:
	//   bit 2 (OUTPUT) = Something related to "RESET CONTROL" circuits?

	// PORT G:
	//   bit 2 (input) = FS1  (Foot Switches and Foot Controler ?)
	//   bit 3 (input) = FS2
	//   bit 4 (input) = FC1
	//   bit 5 (input) = FC2
	//   bit 6 (input) = FC3
	//   bit 7 (input) = FC4

	// PORT H:
	//   bit 1 = TC1 Terminal count - microDMA
	m_maincpu->porth_read().set([] { return 2; }); // area/region detection: checked at EF083E (v10 ROM)
	                                                   // FIXME: These are resistors on the pcb,
	                                                   //        but could be declared in the driver as a 2 bit DIP-Switch for area/region selection.

	// PORT Z:
	//   bit 0 = MSTAT0
	//   bit 1 = MSTAT1
	//   bit 2 = SSTAT0
	//   bit 3 = SSTAT1
	//   bit 4 = COM.PC2
	//   bit 5 = COM.PC1
	//   bit 6 = COM.MAC
	//   bit 7 = COM.MIDI

	// RX0/TX0 = MRXD/MTXD
	// RX1/TX1 = CPDATA
	// SCLK1   = CPSCK

	// AN0 = EXP (expression pedal?)
	// AN1 = AFT

	// Note: The CPU has an internal clock doubler
	TMP94C241(config, m_subcpu, 2*10_MHz_XTAL); // TMP94C241F @ IC27
	// Address bus is set to 8 bits by the pins AM1=GND and AM0=GND
	m_subcpu->set_addrmap(AS_PROGRAM, &kn5000_state::subcpu_mem);
	// Interrupt 0: CLK on "to_subcpu_latch"

	GENERIC_LATCH_8(config, "to_maincpu_latch"); // @ IC23
	GENERIC_LATCH_8(config, "to_subcpu_latch"); //  @ IC22

	UPD72067(config, m_fdc, 32'000'000); // actual controller is UPD72068GF-3B9 at IC208
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, TLCS900_INT4);
	m_fdc->drq_wr_callback().set_inputline(m_maincpu, TLCS900_INT5);
	m_fdc->hdl_wr_callback().set_inputline(m_maincpu, TLCS900_INT6);
	//m_fdc->??_wr_callback().set_inputline(m_maincpu, TLCS900_INT7);
	//FIXME:
	// Interrupt 4: FDCINT
	// Interrupt 5: FDCIRQ
	// Interrupt 6: FDC.H/D
	// Interrupt 7: FDC.I/O

	FLOPPY_CONNECTOR(config, "fdc:0", kn5000_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

    /* extension port */
	KN5000_EXTENSION(config, m_extension, 16_MHz_XTAL / 16); //FIXME: which clock signal should be passed to the extension port?
//	m_extension->firq_callback().set("mainfirq", FUNC(input_merger_device::in_w<3>)); // FIXME: do we need something like this?
//	m_extension->irq_callback().set("mainirq", FUNC(input_merger_device::in_w<3>)); // FIXME: do we need something like this?
	m_extension->option_add("hdae5000", HDAE5000);

	/* sound hardware */
	//SPEAKER(config, "lspeaker").front_left();
	//SPEAKER(config, "rspeaker").front_right();

	/* video hardware */
	// LCD Controller MN89304 @ IC206 24_MHz_XTAL
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_raw(XTAL(40'000'000)/6, 424, 0, 320, 262, 0, 240);
	screen.set_screen_update("vga", FUNC(mn89304_vga_device::screen_update));

	mn89304_vga_device &vga(MN89304_VGA(config, "vga", 0));
	vga.set_screen("screen");
	vga.set_vram_size(0x100000);

	// This is a quick hack to beep whenever the checking device LED is on
	// (just because I find it more convenient to listen to the beeps while debugging the driver)
	SPEAKER(config, "mono").front_center();
	BEEP(config, "checking_device_led_cn11", 12_MHz_XTAL / 3200).add_route(ALL_OUTPUTS, "mono", 0.05);
}

ROM_START(kn5000)
	ROM_REGION16_LE(0x200000, "program" , 0) // main cpu
	ROM_DEFAULT_BIOS("v10")

	// FIXME: These are actually stored in a couple flash rom chips IC6 (even) and IC4 (odd)
	//
	// Note: These ROMs were extracted from the system update floppies which were
	//       compressed using a variant of LZSS. There may still be problems with the
	//       unpacking of the ROM contents. Especially in the lowest 4kbyte window.
	//       Once we get a proper dump of the boot rom, we may have a better idea of
	//       the decompression algorithm and then we may need to update this ROM set.
	//
	//       More info at:
	//       https://github.com/felipesanches/kn5000_homebrew/blob/main/kn5000_extract.py

	ROM_SYSTEM_BIOS(0, "v10", "Version 10 - August 2nd, 1999")
	ROMX_LOAD("kn5000_v10_program.rom", 0x00000, 0x200000, CRC(8f53027e) SHA1(57ebaa13ea6b3d5c67456b16335f06465a29fb0c), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v9", "Version 9 - January 26th, 1999")
	ROMX_LOAD("kn5000_v9_program.rom", 0x00000, 0x200000, CRC(48f2e11d) SHA1(17440a451eb8d756acd224d6b8335fdbdace93e4), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "v8", "Version 8 - November 13th, 1998")
	ROMX_LOAD("kn5000_v8_program.rom", 0x00000, 0x200000, CRC(c9d7843a) SHA1(0496dc46fec5b13ef8ded0fd7930ea81258c462d), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "v7", "Version 7 - June 26th, 1998")
	ROMX_LOAD("kn5000_v7_program.rom", 0x00000, 0x200000, CRC(2ac168c8) SHA1(280a07fe8981d63ea1aa3dfe751184134599eb11), ROM_BIOS(3))

	ROM_SYSTEM_BIOS(4, "v6", "Version 6 - January 16th, 1998") // sometimes refered to as "update6v0"
	ROMX_LOAD("kn5000_v6_program.rom", 0x00000, 0x200000, CRC(8d66ed48) SHA1(f7f10a65aa654795e942c1863dc97aac8ab7ea8b), ROM_BIOS(4))

	ROM_SYSTEM_BIOS(5, "v5", "Version 5 - November 12th, 1997") // sometimes refered to as "update5v0"
	ROMX_LOAD("kn5000_v5_program.rom", 0x00000, 0x200000, CRC(b6100cf0) SHA1(a416fe4ed6084eecc155de3453216b99e305d9c6), ROM_BIOS(5))


	ROM_REGION16_LE(0x20000, "mask", 0) // subcpu (boot rom?)
	ROM_LOAD("kn5000_mask_rom.ic30", 0x00000, 0x20000, NO_DUMP)
	// hack to keep the CPU from touching SFRs arbitrarily while we do not have a proper ROM dump:
	ROM_FILL(0x000000, 1, 0x68) // 68 fe = infinite loop
	ROM_FILL(0x000001, 1, 0xfe)
	ROM_FILL(0x01ff00, 1, 0x00) // RESET vector = 0x00fe0000
	ROM_FILL(0x01ff01, 1, 0x00)
	ROM_FILL(0x01ff02, 1, 0xfe)
	ROM_FILL(0x01ff03, 1, 0x00)

	ROM_REGION16_LE(0x200000, "table_data", 0)
	ROM_LOAD16_BYTE("kn5000_table_data_rom_even.ic3", 0x000000, 0x100000, NO_DUMP)
	ROM_LOAD16_BYTE("kn5000_table_data_rom_odd.ic1", 0x000001, 0x100000, CRC(cd907eac) SHA1(bedf09d606d476f3e6d03e590709715304cf7ea5))

	ROM_REGION16_LE(0x100000, "custom_data", 0)
	ROM_LOAD("kn5000_custom_data_rom.ic19", 0x000000, 0x100000, CRC(5de11a6b) SHA1(4709f815d3d03ce749c51f4af78c62bf4a5e3d94))
	// IC19 is a flash ROM. The contents here were dumped from a system that had it already programmed by the initial data disk.
	// Maybe it could also be declared as NVRAM here?

	ROM_REGION16_LE(0x400000, "rhythm_data", 0)
	ROM_LOAD("kn5000_rhythm_data_rom.ic14", 0x000000, 0x400000, CRC(76d11a5e) SHA1(e4b572d318c9fe7ba00e5b44ea783e89da9c68bd))

	ROM_REGION16_LE(0x1000000, "waveform", 0)
	ROM_LOAD("kn5000_waveform_rom.ic304", 0x000000, 0x400000, NO_DUMP)
	ROM_LOAD("kn5000_waveform_rom.ic305", 0x400000, 0x400000, NO_DUMP)
	ROM_LOAD("kn5000_waveform_rom.ic306", 0x800000, 0x400000, NO_DUMP)
	ROM_LOAD("kn5000_waveform_rom.ic307", 0xc00000, 0x400000, CRC(20ff4629) SHA1(4b511bff6625f4655cabd96a263bf548d2ef4bf7))
ROM_END

} // anonymous namespace

//   YEAR  NAME   PARENT  COMPAT  MACHINE INPUT   STATE         INIT        COMPANY      FULLNAME             FLAGS
CONS(199?, kn5000,    0,       0, kn5000, kn5000, kn5000_state, empty_init, "Technics", "SX-KN5000 Keyboard", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
