// license:BSD-3-Clause
// copyright-holders:flama12333
/*************************************************************************
Known Games running in this hardware:
Soccer 2002
Soccer 2004
Soccer 2006
Soccer 2010
SuperV 2010

No progression until the internal rom of main cpu is dumped

Soccer 2004
Main board
PCB Labeled M991205-A
Display controller
c1 Altera epm7064lc84-7
c2 tms 27c512-20 - eeprom - display controller
c3 hm6116lp-2 - video ram
c4 p8051ah fujitsu - microcontroller - display controller
C5 W78E52B-40 - with internal rom 8kb. - main system. Protected
c6 tms 27c512-2jl eeprom - main system
c7 hm6116lp-3
c20 um3567
??? DAC0800CN
c21 Altera epm7128elc84-15
c22 M27C801

Led Board
J-3
Labeled CS111P076 At front back
GH054055 Sticker Near of dsp2
3x 16x16 led display matrix scroll multicolor green and red.
5 buttons
1x dip switch 8
??? p8255a
??? File KC8279P

gfx vram
000-0FF -> 64x32 page 1
100-1FF -> 64x32 page 2
200-2FF -> 64x32 page 3
300-3FF -> 64x32 page 4
400-4FF -> 64x32 page 5
500-5FF -> 64x32 page 6
600-6FF -> 64x32 page 7
700-7FF -> 64x32 page 8
*/

#include "emu.h"

#include "cpu/mcs51/i80c51.h"
#include "cpu/mcs51/i80c52.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "sound/dac.h"
#include "sound/ymopl.h"
#include "socc2004.lh"

#include "speaker.h"
#include "screen.h"
#include "emupal.h"

namespace {

class my6_state : public driver_device
{
public:
	my6_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_inputs(*this, { "KEYS1", "KEYS2", "DSW", "PUSHBUTTONS" })
		, m_p1(*this, "P1")
     	, m_vram(*this, "vram")
        , m_screen(*this, "screen")

	{ }

	void my6(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:

	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
	void display_map(address_map &map) ATTR_COLD;
	void display_data_map(address_map &map) ATTR_COLD;
	
	
	void display_7seg_data_w(uint8_t data);
	void multiplex_7seg_w(uint8_t data);

	
	
	void ppi1_porta_w(uint8_t data) ATTR_COLD;
    void ppi1_portb_w(uint8_t data) ATTR_COLD;
    void ppi1_portc_w(uint8_t data) ATTR_COLD;

	uint8_t keyboard_r();
	
	uint8_t p1_port_r();
	void p1_port_w(uint8_t data);

	

	uint8_t m_selected_7seg_module = 0;

	uint8_t m_p1_out = 0xff;

	output_finder<63> m_digits;
	output_finder<72> m_leds;
	required_ioport_array<4> m_inputs;
	required_ioport m_p1;
	required_shared_ptr<uint8_t> m_vram;
    required_device<screen_device> m_screen;

uint32_t screen_update(screen_device &screen,
                       bitmap_rgb32 &bitmap,
                       const rectangle &cliprect);
	
	
	
	
};

uint32_t my6_state::screen_update(screen_device &screen,
                                  bitmap_rgb32 &bitmap,
                                  const rectangle &cliprect)
{
    bitmap.fill(rgb_t::black(), cliprect);

    for (int page = 0; page < 8; page++)
    {
        const uint8_t *src = &m_vram[page * 0x100];

        int sx = (page & 3) * 64;
        int sy = (page >> 2) * 32;

        for (int y = 0; y < 32; y++)
        {
            for (int x = 0; x < 64; x++)
            {
                int offs = (y * 8) + (x >> 3);

                // LSB-left bit order
                if (BIT(src[offs], x & 7))
                    bitmap.pix(sy + y, sx + x) = rgb_t::white();
            }
        }
    }

    return 0;
}

void my6_state::ppi1_porta_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 32] = BIT(~data, i);
}

void my6_state::ppi1_portb_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 40] = BIT(~data, i);
}

void my6_state::ppi1_portc_w(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
	m_leds[i + 48] = BIT(~data, i);
}



void my6_state::display_7seg_data_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // HEF4511BP (7 seg display driver)

	m_digits[2 * m_selected_7seg_module + 0] = patterns[data & 0x0f];
	m_digits[2 * m_selected_7seg_module + 1] = patterns[data >> 4];
}


void my6_state::multiplex_7seg_w(uint8_t data)
{
	m_selected_7seg_module = data;
}

uint8_t my6_state::keyboard_r()
{
	switch (m_selected_7seg_module & 0x07)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		return m_inputs[m_selected_7seg_module & 0x07]->read();
	default:
		return 0x00;
	}
}







uint8_t my6_state::p1_port_r()
{
	// meter feedback is read here. Fails with error 02 if it doesn't get the expected value.
	uint8_t const ioport_val = m_p1->read();
	uint8_t meter_fb = 0x00;

	if (!BIT(m_p1_out, 0))
		meter_fb = (BIT(m_p1_out, 1) << 4) | (BIT(m_p1_out, 2) << 5);

	return (ioport_val & 0xcf) | meter_fb;
}

void my6_state::p1_port_w(uint8_t data)
{

	m_p1_out = data;
}

static INPUT_PORTS_START( socc2004 )
	PORT_START("KEYS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )


	PORT_START("KEYS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSW:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSW:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSW:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSW:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSW:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW:8")

	PORT_START("PUSHBUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

    PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )


	PORT_START("display") // input for test purposes only.
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "display:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "display:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "display:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "display:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "display:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "display:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "display:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "display:8")
INPUT_PORTS_END

void my6_state::program_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0x2000, 0xffff).rom().region("eeprom",  0x2000);
}

void my6_state::data_map(address_map &map)
{
	map(0xc000, 0xc001).rw("i8279_1", FUNC(i8279_device::read), FUNC(i8279_device::write));
    map(0xf000, 0xf7ff).ram();
}

void my6_state::display_map(address_map &map)
{

	map(0x0000, 0x7fff).rom(); // Has two program rom.

}

void my6_state::display_data_map(address_map &map)
{
	map(0xe000, 0xe7ff).ram().share("vram"); // Video ram. 64x64
	map(0xa000, 0xa000).portr("display"); // Input for display controller handled by maincpu

}

void my6_state::machine_start()
{

	save_item(NAME(m_selected_7seg_module));
	save_item(NAME(m_p1_out));
}

void my6_state::my6(machine_config &config)
{
	// basic machine hardware
	i8052_device &maincpu(I8052(config, "maincpu", XTAL(10'738'635)));
	maincpu.set_addrmap(AS_PROGRAM, &my6_state::program_map);
	maincpu.set_addrmap(AS_DATA, &my6_state::data_map);

	maincpu.port_in_cb<1>().set(FUNC(my6_state::p1_port_r));
	maincpu.port_out_cb<1>().set(FUNC(my6_state::p1_port_w));

/* Programmable Peripheral Interface */
   i8255_device &ppi1(I8255A(config, "ppi1"));
	ppi1.out_pa_callback().set(FUNC(my6_state::ppi1_porta_w));
	ppi1.out_pb_callback().set(FUNC(my6_state::ppi1_portb_w));
	ppi1.out_pc_callback().set(FUNC(my6_state::ppi1_portc_w));
	


	// Keyboard & display interface
	i8279_device &kbdc(I8279(config, "i8279_1", XTAL(12'000'000) / 6));
	kbdc.out_sl_callback().set(FUNC(my6_state::multiplex_7seg_w));   // select  block of 7seg modules by multiplexing the SL scan lines
	kbdc.in_rl_callback().set(FUNC(my6_state::keyboard_r));          // keyboard Return Lines
	kbdc.out_disp_callback().set(FUNC(my6_state::display_7seg_data_w));

	/* Video */
     screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
     screen.set_refresh_hz(60);
     screen.set_size(256, 64);
     screen.set_visarea(0, 255, 0, 63);     
	 screen.set_screen_update(FUNC(my6_state::screen_update));
     PALETTE(config, "palette", palette_device::MONOCHROME);
     config.set_default_layout(layout_socc2004);

	// Display Controller
	i8051_device &display(I8051(config, "display", XTAL(10'738'635)));
	display.set_addrmap(AS_PROGRAM, &my6_state::display_map);
	display.set_addrmap(AS_DATA, &my6_state::display_data_map);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2413_device &opll(YM2413(config, "opll", 3.579545_MHz_XTAL));
	opll.add_route(ALL_OUTPUTS, "mono", 1.0);

	DAC0800(config, "snd").add_route(ALL_OUTPUTS, "mono", 1.0);

}

ROM_START( socc2004 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w78e52b.c5", 0x00000, 0x2000,  NO_DUMP ) // Protected. ID Error!! Tried to read two times without success. Contains internal rom code.

	ROM_REGION( 0x10000, "eeprom", 0 )
	ROM_LOAD( "2_tms2c5122jl.c6", 0x00000, 0x10000,  CRC(05EF99CD) SHA1(591c51ced0acc3231c9629a060f9c42a2db9fbe0) ) // Sticker labeled 2. Hex FF filled at 0x0000-0x1fff.

	ROM_REGION( 0x10000, "display", 0 )
	ROM_LOAD( "tms27c512_2jl.c2", 0x0000, 0x10000,   CRC(3FBD0A4A) SHA1(d2b5d09d1f4209411ca884c9fa4a73276846c780) ) //  32kb Two Program Rom code. Soccer 2004 and Soccer 2002 bpp1 gfx.

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD( "m27c801.bin", 0x00000, 0x100000,  CRC(BBF4A74C) SHA1(662aaaea0df23c14c2b802d117a342a9bdf13845) ) // Unsigned 8-bit pcm.
ROM_END

} // anonymous namespace


//    YEAR  NAME         PARENT   MACHINE   INPUT       STATE        INIT          ROT      COMPANY                        FULLNAME              FLAGS
GAME( 2004?, socc2004,   0,       my6,      socc2004,   my6_state,   empty_init,   ROT0,   "Ming-Yang Electronic / TSK",   "Soccer 2004",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS |  MACHINE_MECHANICAL )
