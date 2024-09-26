// license:BSD-3-Clause
// copyright-holders:Olivier Galibert,Jonathan Gevaryahu
/*************************************************************************************************

    Roland MT-32/CM32L driver

    Driver by Olivier Galibert and Jonathan Gevaryahu

    The Roland MT32 is an expander (synthesizer without the keyboard)
    from 1987.  It had a small 20-char LCD display and 10 control
    buttons (part 1-5, rhythm, sound/sound group, volume/master
    volume), a rotating knob and a midi message led.  It uses a weird
    kind of synthesis called "Linear Additive".

    The CM32L versions is a simpler/smaller version without the lcd,
    buttons or knob but keeping the led.  It also has a different
    firmware and more partials rom, adding "game sounds".  "CM" means
    "Computer Music".

    The LAPC-I (I as in Intel) is a ISA board variant of the CM32L
    with a MPU-401 frontend added to communicate with the synth.  All
    the roms are identical with the CM32L.

    Some special tricks:
    - pressing 3+V (Part 3 + Volume) at boot time starts the test mode

    - predding 4+R+M (Part 4 + Rhythm + Master Volume) at boot time
      shows the firmware version

    - the CM32L drives the lcd as if it was there and looks at the
      buttons at boot time for test mode and firmware version



Roland MT-32 rev00 pcb (early 1.0.7 and all earlier firmware, with ic26 and ic27 socketed):

PCB Layout
----------

MT-32 MAIN BOARD ASSY 79377310 00
|---------------------------------|------------------------|-----------|
|  JK1   JK2            IR3M03    |          JK3           |JK4   SW1  |
|                                 |                        |           |
|5218  072D 5218  072D            |------------------------|           |
|                          IR3M03     PC910       74HC04               |
|                                                                      |
|072D       4051                                                       |
|                            ROM.IC21                     |----|       |
|   M5238                                                 |    |       |
|                                     ROM.IC26    6264    |    |       |
| |---------| 74LS05 74HC27  ROM.IC22                     |    |  12MHz|
| | PCM54HP |        74HC00                       6264    |    |       |
| |---------|                                             |8095|       |
|                            |------|                     |    |       |
|                            |      | ROM.IC27    6264    |    |       |
| 4416  4416                 |  *2  |                     |    |       |
|                            |      |             6264    |----|       |
| 4416  4416                 |      |                                  |
|                            |------|                    |-----|       |
|             |-------|                                  |     |       |
| ROM.IC13    |       |       32.768kHz                  |  *1 |       |
|             |  *3   |                         74HC04   |     |       |
| CN1  CN2    |-------|      CN3  CN4  74HC02   74HC00   |-----|    CN5|
|----------------------------------------------------------------------|
Notes: (All IC's listed for completeness)
      8095   - Intel C8095-90 16-Bit Microcontroller, Clock Input 12.000MHz (DIP48)
      6264   - Hitachi HM6264ALSP-15 8K x8 SRAM (DIP28)
      4416   - Mitsubishi M5M4416P-12 16K x4 DRAM (DIP18)
      PC910  - Sharp PC910 Opto-Isolator (DIP8)
      IR3M03 - Sharp IR3M03A (DIP8)
      072D   - New Japan Radio Co., Ltd JRC072D Dual J-FET Input Operational Amplifier (DIP8)
      5218   - Mitsubishi 5218 Operational Amplifier (DIP8)
      M5238  - Mitsubishi M5238 LCD Display Controller (SIP8)
      CN1    - 6 Pin Connector
      CN2    - 6 Pin Connector
      CN3    - 2 Pin Connector
      CN4    - 3 Pin Connector
      CN5    - 10 Pin Connector
      SW1    - Power On/Off Switch
      JK1    - 1/4" Stereo Jack
      JK2    - 1/4" Stereo Jack
      JK3    - MIDI In, MIDI Out and Midi Thru DIN5 Jacks
      JK4    - Mini 1/8th" Power Input Jack
      *1     - R15229865 HG61H15B59F (QFP80, stamped 'Roland')
      *2     - R15229851 MB87136A LA32 (PGA144?, stamped 'Roland')
      *3     - R15229863 HG61H20R36F (QFP100, stamped 'BOSS')
      PCM54HP- Burr-Brown PCM54HP 16-Bit Monolithic Digital-to-Analog Converter (DIP28)
      & Various logic ICs - 4051, 74HC27, 74HC00, 74HC02, 74HC04, 74LS05

      ROMs -  Filename          Device Type
              ------------------------------------------------------------------------
              ROM.IC26.106      Mitsubishi M5M27C256 (labeled 1.0.6)   read as 27C256
              ROM.IC27.106      Mitsubishi M5M27C256 (labeled 1.0.6)   read as 27C256
              ROM.IC13.200      Mitsubishi M5M27C128 (labeled 2.0.0)   read as 27C128
              ROM.IC21          Toshiba TC532000P (-7471)              read as TC572000
              ROM.IC22          Toshiba TC532000P (-7472)              read as TC572000



Newer version

PCB Layout
----------

MT-32 MAIN BOARD ASSY 79377310 01
|---------------------------------|------------------------|-----------|
|  JK1   JK2            IR3M03    |          JK3           |JK4   SW1  |
|                                 |                        |           |
|5218  072D 5218  072D            |------------------------|           |
|                          IR3M03     PC910       74HC04               |
|                                                                      |
|072D       4051                                                       |
|                            ROM.IC37                     |----|       |
|   M5238                                                 |    |       |
|                                     ROM.IC26    4364    |    |       |
| |---------| 74LS05 74HC27                               |    |  12MHz|
| | PCM54HP |        74HC00                       4364    |    |       |
| |---------|                                             |8095|       |
|                                                         |    |       |
|                              |-----|ROM.IC27    4364    |    |       |
| 81416  81416                 |     |                    |    |       |
|                              |  *2 |            4364    |----|       |
| 81416  81416                 |     |                                 |
|                              |-----|                   |-----|       |
|             |-------|                                  |     |       |
| ROM.IC13    |       |       32.768kHz                  |  *1 |       |
|             |  *3   |                         74HC04   |     |       |
| CN1  CN2    |-------|      CN3  CN4  74HC02   74HC00   |-----|    CN5|
|----------------------------------------------------------------------|
Notes: (All IC's listed for completeness)
      8095   - Intel C8095-90 16-Bit Microcontroller, Clock Input 12.000MHz (DIP48)
      4364   - NEC D4364CX-15L 8K x8 SRAM (DIP28)
      81416  - Fujitsu MB81416 16K x4 DRAM (DIP18)
      PC910  - Sharp PC910 Opto-Isolator (DIP8)
      IR3M03 - Sharp IR3M03A (DIP8)
      072D   - New Japan Radio Co., Ltd JRC072D Dual J-FET Input Operational Amplifier (DIP8)
      5218   - Mitsubishi 5218 Operational Amplifier (DIP8)
      M5238  - Mitsubishi M5238 LCD Display Controller (SIP8)
      CN1    - 6 Pin Connector
      CN2    - 6 Pin Connector
      CN3    - 2 Pin Connector
      CN4    - 3 Pin Connector
      CN5    - 10 Pin Connector
      SW1    - Power On/Off Switch
      JK1    - 1/4" Stereo Jack
      JK2    - 1/4" Stereo Jack
      JK3    - MIDI In, MIDI Out and Midi Thru DIN5 Jacks
      JK4    - Mini 1/8th" Power Input Jack
      *1     - R15229865 HG61H15B59F (QFP80, stamped 'Roland')
      *2     - R15229896 LA32 (QFP100, stamped 'Roland')
      *3     - R15229863 HG61H20R36F (QFP100, stamped 'BOSS')
      PCM54HP- Burr-Brown PCM54HP 16-Bit Monolithic Digital-to-Analog Converter (DIP28)
      & Various logic ICs - 4051, 74HC27, 74HC00, 74HC02, 74HC04, 74LS05

      ROMs -  Filename     Device Type
              ----------------------------------------------------
              ROM.IC26     Hitachi HN623258PH26   read as 27C256
              ROM.IC27     Hitachi HN623258PH26   read as 27C256
              ROM.IC13     Hitachi HN623257PC21   read as 27C256
              ROM.IC37     Toshiba TC534000P      read as TC574000
*/

#include "emu.h"
#include "cpu/mcs96/i8x9x.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "video/sed1200.h"
#include "emupal.h"
#include "screen.h"


namespace {

static INPUT_PORTS_START( mt32 )
	PORT_START("SC0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Sound group") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Volume") PORT_CODE(KEYCODE_V)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SC1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Rhythm") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Sound") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Master volume") PORT_CODE(KEYCODE_M)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("A7")
	PORT_BIT(0x03ff, 0x0000, IPT_DIAL) PORT_NAME("Knob") PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_CODE_DEC(KEYCODE_DOWN) PORT_CODE_INC(KEYCODE_UP)
INPUT_PORTS_END

class mt32_state : public driver_device
{
public:
	mt32_state(const machine_config &mconfig, device_type type, const char *tag);

	void mt32(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<i8x9x_device> cpu;
	required_device<ram_device> ram;
	optional_device<sed1200d0a_device> lcd;
	required_device<timer_device> midi_timer;

	void mt32_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bank_w(uint8_t data);
	void so_w(uint8_t data);
	void midi_w(uint16_t data);

	uint8_t lcd_ctrl_r();
	void lcd_ctrl_w(uint8_t data);
	void lcd_data_w(uint8_t data);
	uint16_t port0_r();

	TIMER_DEVICE_CALLBACK_MEMBER(midi_timer_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(samples_timer_cb);

	void mt32_map(address_map &map) ATTR_COLD;

	uint8_t lcd_data_buffer[256]{};
	int lcd_data_buffer_pos = 0;
	uint8_t midi = 0;
	int midi_pos = 0;
	uint8_t port0 = 0;
};

mt32_state::mt32_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	cpu(*this, "maincpu"),
	ram(*this, "ram"),
	lcd(*this, "lcd"),
	midi_timer(*this, "midi_timer")
{
}


uint32_t mt32_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0);
	const uint8_t *data = lcd->render();
	for(int c=0; c<20; c++)
		for(int y=0; y<8; y++) {
			uint8_t v = data[c*8+y];
			for(int x=0; x<5; x++)
				bitmap.pix(y == 7 ? 8 : y, c*6+x) = v & (0x10 >> x) ? 1 : 0;
		}
	return 0;
}

void mt32_state::machine_start()
{
	membank("bank")->configure_entries(0, 4, memregion("maincpu")->base(), 0x4000);
	membank("bank")->configure_entries(0x10, 2, ram->pointer(), 0x4000); // 0x10 doesn't seem used though, but that ram is accessible at c000+
	membank("fixed")->set_base(ram->pointer());

	lcd_data_buffer_pos = 0;
}

void mt32_state::machine_reset()
{
	midi_timer->adjust(attotime::from_hz(1));
	midi_pos = 0;
	port0 = 0;
}

void mt32_state::lcd_ctrl_w(uint8_t data)
{
	lcd->cs_w(0);
	lcd->control_w(data >> 4);
	lcd->control_w(data);
	for(int i=0; i != lcd_data_buffer_pos; i++) {
		lcd->data_w(lcd_data_buffer[i] >> 4);
		lcd->data_w(lcd_data_buffer[i]);
	}
	lcd->cs_w(1);
	lcd_data_buffer_pos = 0;
}

uint8_t mt32_state::lcd_ctrl_r()
{
	// Note that this does not read from the actual LCD unit (whose /RD line is pulled high)
	return 0;
}

void mt32_state::lcd_data_w(uint8_t data)
{
	lcd_data_buffer[lcd_data_buffer_pos++] = data;
}

void mt32_state::bank_w(uint8_t data)
{
	membank("bank")->set_entry(data);
}

void mt32_state::midi_w(uint16_t data)
{
	logerror("midi_out %02x\n", data);
	midi = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(mt32_state::midi_timer_cb)
{
	const static uint8_t midi_data[3] = { 0x91, 0x40, 0x7f };
	midi = midi_data[midi_pos++];
	logerror("midi_in %02x\n", midi);
	cpu->serial_w(midi);
	if(midi_pos < sizeof(midi_data))
		midi_timer->adjust(attotime::from_hz(1250));
}

uint16_t mt32_state::port0_r()
{
	return port0;
}

TIMER_DEVICE_CALLBACK_MEMBER(mt32_state::samples_timer_cb)
{
	port0 ^= 0x10;
}

void mt32_state::so_w(uint8_t data)
{
	// bit 0   = led
	// bit 1-2 = reverb program a13/a14
	// bit 3-4 = nc
	// bit 5   = boss x1
	// bit 6   = lcd cs, also handled internally by the gate array
	// bit 7   = lcd clk, also handled internally by the gate array
	//  logerror("so: x1=%d bank=%d led=%d\n", (data >> 5) & 1, (data >> 1) & 3, data & 1);
}

void mt32_state::mt32_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0, 0, 0));
	palette.set_pen_color(1, rgb_t(0, 255, 0));
}

void mt32_state::mt32_map(address_map &map)
{
	map(0x0100, 0x0100).w(FUNC(mt32_state::bank_w));
	map(0x0200, 0x0200).w(FUNC(mt32_state::so_w));
	map(0x021a, 0x021a).portr("SC0");
	map(0x021c, 0x021c).portr("SC1");
	map(0x0300, 0x0300).w(FUNC(mt32_state::lcd_data_w));
	map(0x0380, 0x0380).rw(FUNC(mt32_state::lcd_ctrl_r), FUNC(mt32_state::lcd_ctrl_w));
	map(0x1000, 0x7fff).rom().region("maincpu", 0x1000);
	map(0x8000, 0xbfff).bankrw("bank");
	map(0xc000, 0xffff).bankrw("fixed");
}

void mt32_state::mt32(machine_config &config)
{
	i8x9x_device &maincpu(P8098(config, cpu, 12_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &mt32_state::mt32_map);
	maincpu.ach7_cb().set_ioport("A7");
	maincpu.serial_tx_cb().set(FUNC(mt32_state::midi_w));
	maincpu.in_p0_cb().set(FUNC(mt32_state::port0_r));

	RAM(config, ram).set_default_size("32K");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update(FUNC(mt32_state::screen_update));
//  screen.set_size(20*6-1, 9);
	screen.set_size(20*6-1, (20*6-1)*3/4);
	screen.set_visarea(0, 20*6-2, 0, (20*6-1)*3/4-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(mt32_state::mt32_palette), 2);

	SED1200D0A(config, lcd, 0);

	TIMER(config, midi_timer).configure_generic(FUNC(mt32_state::midi_timer_cb));

	TIMER(config, "samples_timer").configure_periodic(FUNC(mt32_state::samples_timer_cb), attotime::from_hz(32000*2));
}

ROM_START( mt32 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_DEFAULT_BIOS( "107" )

	ROM_SYSTEM_BIOS( 0, "104", "Firmware 1.0.4" )
	ROMX_LOAD(       "mt32_1.0.4.ic27.bin",          0,   0x8000, CRC(a93b65f2) SHA1(9cd4858014c4e8a9dff96053f784bfaac1092a2e), ROM_BIOS(0) | ROM_SKIP(1) )
	ROMX_LOAD(       "mt32_1.0.4.ic26.bin",          1,   0x8000, CRC(b5ee2192) SHA1(fe8db469b5bfeb37edb269fd47e3ce6d91014652), ROM_BIOS(0) | ROM_SKIP(1) )

	ROM_SYSTEM_BIOS( 1, "105", "Firmware 1.0.5" )
	ROMX_LOAD(       "mt32_1.0.5.ic27.bin",          0,   0x8000, CRC(3281216c) SHA1(57a09d80d2f7ca5b9734edbe9645e6e700f83701), ROM_BIOS(1) | ROM_SKIP(1) )
	ROMX_LOAD(       "mt32_1.0.5.ic26.bin",          1,   0x8000, CRC(e06d8020) SHA1(52e3c6666db9ef962591a8ee99be0cde17f3a6b6), ROM_BIOS(1) | ROM_SKIP(1) )

	ROM_SYSTEM_BIOS( 2, "106", "Firmware 1.0.6" )
	ROMX_LOAD(       "mt32_1.0.6.ic27.bin",          0,   0x8000, CRC(29369ae1) SHA1(cc83bf23cee533097fb4c7e2c116e43b50ebacc8), ROM_BIOS(2) | ROM_SKIP(1) )
	ROMX_LOAD(       "mt32_1.0.6.ic26.bin",          1,   0x8000, CRC(4d495d98) SHA1(bf4f15666bc46679579498386704893b630c1171), ROM_BIOS(2) | ROM_SKIP(1) )

	ROM_SYSTEM_BIOS( 3, "107", "Firmware 1.0.7" )
	ROMX_LOAD(       "mt32_1.0.7.ic27.bin",          0,   0x8000, CRC(67fd8968) SHA1(13f06b38f0d9e0fc050b6503ab777bb938603260), ROM_BIOS(3) | ROM_SKIP(1) ) // also exists as a Mask ROM "R15449122 // HN623258PH26" @IC27
	ROMX_LOAD(       "mt32_1.0.7.ic26.bin",          1,   0x8000, CRC(60f45882) SHA1(c55e165487d71fa88bd8c5e9c083bc456c1a89aa), ROM_BIOS(3) | ROM_SKIP(1) ) // also exists as a Mask ROM "R15449123 // HN623258PH27" @IC26

	ROM_SYSTEM_BIOS( 4, "br", "Blue Ridge enhanced firmware" )
	ROMX_LOAD(       "blue_ridge__mt32b.bin",        1,   0x8000, CRC(5816476f) SHA1(e0934320d7cbb5edfaa29e0d01ae835ef620085b), ROM_BIOS(4) | ROM_SKIP(1) )
	ROMX_LOAD(       "blue_ridge__mt32a.bin",        0,   0x8000, CRC(d75fc3d9) SHA1(11a6ae5d8b6ee328b371af7f1e40b82125aa6b4d), ROM_BIOS(4) | ROM_SKIP(1) )

	ROM_SYSTEM_BIOS( 5, "m9", "M9 enhanced firmware" )
	ROMX_LOAD(       "a__m-9.27c256.ic27.bin",       0,   0x8000, CRC(c078ab00) SHA1(381e4208c0211a9a24a3a1b06a36760a1940ea6b), ROM_BIOS(5) | ROM_SKIP(1) )
	ROMX_LOAD(       "b__m-9.27c256.ic26.bin",       1,   0x8000, CRC(e9c439c4) SHA1(36fece02eddd84230a7cf32f931c94dd14adbf2c), ROM_BIOS(5) | ROM_SKIP(1) )

	// Dumped from "new" board revision single 128K x 8 ROM
	ROM_SYSTEM_BIOS( 6, "204", "Firmware 2.0.4" )
	ROMX_LOAD(       "mt32_2.0.4.ic28",              0,   0x10000, CRC(59a49d5c) SHA1(2c16432b6c73dd2a3947cba950a0f4c19d6180eb), ROM_BIOS(6) )
	ROM_IGNORE(0x10000)  // banking needs to be implemented

	ROM_SYSTEM_BIOS( 7, "207", "Firmware 2.0.7" )
	ROMX_LOAD(       "mt32_2.0.7.ic28",              0,   0x10000, CRC(a016b607) SHA1(47b52adefedaec475c925e54340e37673c11707c), ROM_BIOS(7) )
	ROM_IGNORE(0x10000)  // banking needs to be implemented

// We need a bios-like selection for these too
	ROM_REGION( 0x80000, "la32", 0 )
	// separate PCM Mask ROMs from the MT-32 1.x older PCB
	ROM_LOAD(        "r15179844.ic21.bin",           0,  0x40000, CRC(dd9deac3) SHA1(3a1e19b0cd4036623fd1d1d11f5f25995585962b) )
	ROM_LOAD(        "r15179845.ic22.bin",     0x40000,  0x40000, CRC(4ee6506c) SHA1(2cadb99d21a6a4a6f5b61b6218d16e9b43f61d01) )

	// combined PCM Mask ROM from the MT-32 1.x newer PCB; same contents as the two ROMs above, concatenated.
	ROM_LOAD(        "r15449121.ic37.bin",           0,  0x80000, CRC(573e31cc) SHA1(f6b1eebc4b2d200ec6d3d21d51325d5b48c60252) )


	ROM_REGION( 0x8000, "boss", 0 )
	ROM_LOAD(        "r15179857.ic13.bin",           0,   0x8000, CRC(cb219d85) SHA1(c2933cb7ad86e51904aa1c3bc12fa234e73a337f) )
ROM_END

ROM_START( cm32l )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_DEFAULT_BIOS( "102" )

	ROM_SYSTEM_BIOS( 0, "100", "Firmware 1.00" )
	ROMX_LOAD(       "lapc-i.v1.0.0.ic3.bin",        0,  0x10000, CRC(ee62022f) SHA1(73683d585cd6948cc19547942ca0e14a0319456d), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "102", "Firmware 1.02" )
	ROMX_LOAD(       "cm32l_control.rom",            0,  0x10000, CRC(b998047e) SHA1(a439fbb390da38cada95a7cbb1d6ca199cd66ef8), ROM_BIOS(1) )

	ROM_REGION( 0x100000, "la32", 0 )
// We need a bios-like selection for these too
	// separate PCM Mask ROMS from the LAPC-I and CM-32L
	ROM_LOAD(        "r15449121.ic9.bin",            0,  0x80000, CRC(573e31cc) SHA1(f6b1eebc4b2d200ec6d3d21d51325d5b48c60252) ) // shared with MT-32 1.x newer pcb, RA-50 and others
	ROM_LOAD(        "r15179945.ic8.bin",      0x80000,  0x80000, CRC(8e9ea06e) SHA1(3ad889fde5db5b6437cbc2eb6e305312fec3df93) ) // extended CM-32L specific sounds

	// combined PCM Mask ROM; same contents as the two roms above, concatenated; this may match the contents of the LAPC-N rom "R15209324" but needs verification
	ROM_LOAD(        "cm32l_pcm.rom",                0, 0x100000, CRC(04204baa) SHA1(f2a10225b0c191a10fbf068f1320c91b35c1c3f2) )

	ROM_REGION( 0x8000, "boss", 0 )
	ROM_LOAD(        "r15179917.ic19.bin",           0,   0x8000, CRC(236c87a6) SHA1(e1c03905c46e962d1deb15eeed92eb61b42bba4a) ) // shared with RA-50 and others
ROM_END

} // anonymous namespace


CONS( 1987, mt32,  0, 0, mt32, mt32, mt32_state, empty_init, "Roland", "MT-32",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 1989, cm32l, 0, 0, mt32, mt32, mt32_state, empty_init, "Roland", "CM-32L", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
