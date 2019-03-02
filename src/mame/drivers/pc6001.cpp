// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/************************************************************************************************

    PC-6001 series (c) 1981 NEC

    driver by Angelo Salese

    TODO:
    - Hook up tape loading, images that are floating around the net are already
      ADC'ed, so they should be easy to implement (but not exactly faithful)
    - cassette handling requires a decap of the MCU. It could be possible to
      do some tight synch between the master CPU and a code simulation, but maybe
      it's not worth the effort...
    - Identify and hook-up the FDC device, apparently PC-6001 and PC-6601 doesn't
      even use the same thing;
    - PC-6601: mon r-0 type games doesn't seem to work at all on this system?
    - PC-6001SR: get it to boot, also implement MK-2 compatibility mode (it changes
      the memory map to behave like the older versions)
    - Hookup MC6847 for vanilla PC-6001 and fix video bugs for that device;
    - upd7752 voice speech device needs to be properly emulated (device is currently a skeleton),
      Chrith game is a good test case, it's supposed to talk before title screen;

    TODO (game specific):
    - (several AX* games, namely Galaxy Mission Part 1/2 and others): inputs doesn't work;
    - AX6 - Demo: When AY-based speech talks, other emus emulates the screen drawing to be
       a solid green (plain PC-6001) or solid white (Mk2 version), but according to an
       original video reference, that screen should actually some kind of weird garbage on it;
    - AX6 - Powered Knight: doesn't work too well, according to the asm code it asks the
       player to press either 'B' or 'C' then a number but nothing is shown on screen,
       other emus behaves the same, bad dump?
    - Dawn Patrol (cart): presumably too slow;
    (Mk2 mode 5 games)
    - 3D Golf Simulation Super Version: gameplay / inputs seems broken
    - American Truck: Screen is offset at the loading screen, loading bug?
    - Castle Excellent: copyright text drawing is quite bogus, scans text in vertical
       instead of horizontal?
    - Dezeni Land (ALL versions) / Hurry Fox 1/2: asks you to "load something", can't do it
       with current cassette kludge, also, for Dezeni Land(s) keyboard irqs doesn't seem to
       work too well with halt opcode execution?
    - Dezeni Land 1/4: dies after loading of main program;
    - Dezeni Land 2: dies at the "load something" screen with presumably wrong stack opcodes
    - (MyCom BASIC games with multiple files): most of them refuses to run ... how to load them?
    - Grobda: when "get ready" speech plays, screen should be full white but instead it's all
       black, same issue as AX-6 Demo?
    - Pac-Man / Tiny Xevious 2: gameplay is too fast
    - Salad no Kunino Tomato-Hime: can't start a play
    - Space Harrier: inputs doesn't work properly
    - The Black Onyx: dies when it attempts to save the character, that obviously means saving
       on the tape;
    - Yakyukyo / Punchball Mario: waits for an irq, check which one;

=================================================================================================

    PC-6001 (1981-09):

     * CPU: Z80A @ 4 MHz
     * ROM: 16KB + 4KB (chargen) - no kanji
     * RAM: 16KB, it can be expanded to 32KB
     * Text Mode: 32x16 and 2 colors
     * Graphic Modes: 64x48 (9 colors), 128x192 (4 colors), 256x192 (2 colors)
     * Sound: BEEP + PSG - Optional Voice Synth Cart
     * Keyboard: JIS Keyboard with 5 function keys, control key, TAB key,
            HOME/CLR key, INS key, DEL key, GRAPH key, Japanese syllabary
            key, page key, STOP key, and cursor key (4 directions)
     * 1 cartslot, optional floppy drive, optional serial 232 port, 2
            joystick ports


    PC-6001 mkII (1983-07):

     * CPU: Z80A @ 4 MHz
     * ROM: 32KB + 16KB (chargen) + 32KB (kanji) + 16KB (Voice Synth)
     * RAM: 64KB
     * Text Mode: same as PC-6001 with N60-BASIC; 40x20 and 15 colors with
            N60M-BASIC
     * Graphic Modes: same as PC-6001 with N60-BASIC; 80x40 (15 colors),
            160x200 (15 colors), 320x200 (4 colors) with N60M-BASIC
     * Sound: BEEP + PSG
     * Keyboard: JIS Keyboard with 5 function keys, control key, TAB key,
            HOME/CLR key, INS key, DEL key, CAPS key, GRAPH key, Japanese
            syllabary key, page key, mode key, STOP key, and cursor key (4
            directions)
     * 1 cartslot, floppy drive, optional serial 232 port, 2 joystick ports


    PC-6001 mkIISR (1984-12):

     * CPU: Z80A @ 3.58 MHz
     * ROM: 64KB + 16KB (chargen) + 32KB (kanji) + 32KB (Voice Synth)
     * RAM: 64KB
     * Text Mode: same as PC-6001/PC-6001mkII with N60-BASIC; 40x20, 40x25,
            80x20, 80x25 and 15 colors with N66SR-BASIC
     * Graphic Modes: same as PC-6001/PC-6001mkII with N60-BASIC; 80x40 (15 colors),
            320x200 (15 colors), 640x200 (15 colors) with N66SR-BASIC
     * Sound: BEEP + PSG + FM
     * Keyboard: JIS Keyboard with 5 function keys, control key, TAB key,
            HOME/CLR key, INS key, DEL key, CAPS key, GRAPH key, Japanese
            syllabary key, page key, mode key, STOP key, and cursor key (4
            directions)
     * 1 cartslot, floppy drive, optional serial 232 port, 2 joystick ports


    info from http://www.geocities.jp/retro_zzz/machines/nec/6001/spc60.html

=================================================================================================

PC-6001 irq table:
irq vector 0x00: writes 0x00 to [$fa19]                                                     ;(unused)
irq vector 0x02: (A = 0, B = 0) tests ppi port c, does something with ay ports (plus more?) ;keyboard data ready, no kanji lock, no caps lock
irq vector 0x04:                                                                            ;uart irq
irq vector 0x06: operates with $fa28, $fa2e, $fd1b                                          ;timer irq
irq vector 0x08: tests ppi port c, puts port A to $fa1d,puts 0x02 to [$fa19]                ;tape data ready
irq vector 0x0a: writes 0x00 to [$fa19]                                                     ;(unused)
irq vector 0x0c: writes 0x00 to [$fa19]                                                     ;(unused)
irq vector 0x0e: same as 2, (A = 0x03, B = 0x00)                                            ;keyboard data ready, unknown type
irq vector 0x10: same as 2, (A = 0x03, B = 0x00)                                            ;(unused)
irq vector 0x12: writes 0x10 to [$fa19]                                                     ;end of tape reached
irq vector 0x14: same as 2, (A = 0x00, B = 0x01)                                            ;kanji lock enabled
irq vector 0x16: tests ppi port c, writes the result to $feca.                              ;joystick
irq vector 0x18:                                                                            ;TVR (?)
irq vector 0x1a:                                                                            ;Date
irq vector 0x1c:                                                                            ;(unused)
irq vector 0x1e:                                                                            ;(unused)
irq vector 0x20:                                                                            ;voice
irq vector 0x22:                                                                            ;VRTC (?)
irq vector 0x24:                                                                            ;(unused)
irq vector 0x26:                                                                            ;(unused)

************************************************************************************************/

#include "emu.h"
#include "includes/pc6001.h"

#define IRQ_LOG (0)

inline void pc6001_state::cassette_latch_control(bool new_state)
{
	// 0 -> 1 transition: send PLAY tape cmd to i8049
	if((!(m_sys_latch & 8)) && new_state == true) //PLAY tape cmd
	{
		m_cas_switch = 1;
		//m_cassette->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
		//m_cassette->change_state(CASSETTE_PLAY,CASSETTE_MASK_UISTATE);
	}
	// 1 -> 0 transition: send STOP tape cmd to i8049
	if((m_sys_latch & 8) && new_state == false) //STOP tape cmd
	{
		m_cas_switch = 0;
		//m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
		//m_cassette->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
		//set_maincpu_irq_line(0x00);
	}
}

// TODO: this is explicitly needed for making all machines to boot
inline void pc6001_state::ppi_control_hack_w(uint8_t data)
{
	if(data & 1)
		m_port_c_8255 |=   1<<((data>>1)&0x07);
	else
		m_port_c_8255 &= ~(1<<((data>>1)&0x07));

	#ifdef UNUSED_FUNCTION
	// this switch-case is overwritten below anyway!?
	switch(data)
	{
		case 0x08: m_port_c_8255 |= 0x88; break;
		case 0x09: m_port_c_8255 &= 0xf7; break;
		case 0x0c: m_port_c_8255 |= 0x28; break;
		case 0x0d: m_port_c_8255 &= 0xf7; break;
		default: break;
	}
	#endif

	m_port_c_8255 |= 0xa8;
}

inline void pc6001_state::set_maincpu_irq_line(uint8_t vector_num)
{
	m_irq_vector = vector_num;
	m_maincpu->set_input_line(0, ASSERT_LINE);
}

WRITE8_MEMBER(pc6001_state::system_latch_w)
{
	static const uint16_t startaddr[] = {0xC000, 0xE000, 0x8000, 0xA000 };

	m_video_ram =  m_ram + startaddr[(data >> 1) & 0x03] - 0x8000;

	cassette_latch_control((data & 8) == 8);
	m_sys_latch = data;

	m_timer_irq_mask = data & 1;
	//printf("%02x\n",data);
}


READ8_MEMBER(pc6001_state::nec_ppi8255_r)
{
	if (offset==2)
		return m_port_c_8255;
	else if(offset==0)
	{
		uint8_t res;
		res = m_cur_keycode;
		//m_cur_keycode = 0;
		return res;
	}

	return m_ppi->read(offset);
}

WRITE8_MEMBER(pc6001_state::nec_ppi8255_w)
{
	if (offset==3)
	{
		ppi_control_hack_w(data);
		//printf("%02x\n",data);

		if ((data & 0x0f) == 0x05 && m_cart_rom)
			m_bank1->set_base(m_cart_rom->base() + 0x2000);
		if ((data & 0x0f) == 0x04)
			m_bank1->set_base(m_region_gfx1->base());
	}

	m_ppi->write(offset,data);
}

void pc6001_state::pc6001_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom().nopw();
//  AM_RANGE(0x4000, 0x5fff) // mapped by the cartslot
	map(0x6000, 0x7fff).bankr("bank1");
	map(0x8000, 0xffff).ram().share("ram");
}

void pc6001_state::pc6001_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x81).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x90, 0x93).mirror(0x0c).rw(FUNC(pc6001_state::nec_ppi8255_r), FUNC(pc6001_state::nec_ppi8255_w));
	map(0xa0, 0xa0).mirror(0x0c).w("ay8910", FUNC(ay8910_device::address_w));
	map(0xa1, 0xa1).mirror(0x0c).w("ay8910", FUNC(ay8910_device::data_w));
	map(0xa2, 0xa2).mirror(0x0c).r("ay8910", FUNC(ay8910_device::data_r));
	map(0xa3, 0xa3).mirror(0x0c).nopw();
	map(0xb0, 0xb0).mirror(0x0f).w(FUNC(pc6001_state::system_latch_w));
	map(0xd0, 0xd3).mirror(0x0c).noprw(); // disk device
}

/*****************************************
 *
 * PC-6001Mk2 specific i/o
 *
 ****************************************/

/*
    ROM_REGION( 0x28000, "maincpu", ROMREGION_ERASEFF )
    ROM_LOAD( "basicrom.62", 0x10000, 0x8000, CRC(950ac401) SHA1(fbf195ba74a3b0f80b5a756befc96c61c2094182) )
    ROM_LOAD( "voicerom.62", 0x18000, 0x4000, CRC(49b4f917) SHA1(1a2d18f52ef19dc93da3d65f19d3abbd585628af) )
    ROM_LOAD( "cgrom60.62",  0x1c000, 0x2000, CRC(81eb5d95) SHA1(53d8ae9599306ff23bf95208d2f6cc8fed3fc39f) )
    ROM_LOAD( "cgrom60m.62", 0x1e000, 0x2000, CRC(3ce48c33) SHA1(f3b6c63e83a17d80dde63c6e4d86adbc26f84f79) )
    ROM_LOAD( "kanjirom.62", 0x20000, 0x8000, CRC(20c8f3eb) SHA1(4c9f30f0a2ebbe70aa8e697f94eac74d8241cadd) )
*/

#define BASICROM(_v_) \
	0x10000+0x2000*_v_
#define VOICEROM(_v_) \
	0x18000+0x2000*_v_
#define TVROM(_v_) \
	0x1c000+0x2000*_v_
#define KANJIROM(_v_) \
	0x20000+0x2000*_v_
#define WRAM(_v_) \
	0x28000+0x2000*_v_
#define EXWRAM(_v_) \
	0x38000+0x2000*_v_
#define EXROM(_v_) \
	0x48000+0x2000*_v_
#define INVALID(_v_) \
	0x4c000+0x2000*_v_
// TODO: rewrite using bankdev
// TODO: some comments aren't right
static const uint32_t banksw_table_r0[0x10*4][4] = {
	/* 0 */
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  },  //0x00: <invalid setting>
	{ BASICROM(0),  BASICROM(1),    BASICROM(2),    BASICROM(3) },  //0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ TVROM(0),     TVROM(1),       VOICEROM(0),    VOICEROM(1) },  //0x02: tv rom 0 & 1 / voice rom 0 & 1
	{ EXROM(1),     EXROM(1),       EXROM(1),       EXROM(1)    },  //0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),     EXROM(0),       EXROM(0),       EXROM(0)    },  //0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ TVROM(1),     BASICROM(1),    VOICEROM(0),    BASICROM(3) },  //0x05: tv rom 1 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),  TVROM(2),       BASICROM(2),    VOICEROM(1) },  //0x06: basic rom 0 & tv rom 2 / basic rom 2 & voice 1
	{ EXROM(0),     EXROM(1),       EXROM(0),       EXROM(1)    },  //0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),     EXROM(0),       EXROM(1),       EXROM(0)    },  //0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),     BASICROM(1),    EXROM(1),       BASICROM(3) },  //0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),       BASICROM(2),    EXROM(1)    },  //0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),     TVROM(2),       EXROM(0),       VOICEROM(1) },  //0x0b: ex rom 0 & tv rom 2 / ex rom 0 & voice 1
	{ TVROM(1),     EXROM(0),       VOICEROM(0),    EXROM(0)    },  //0x0c: tv rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(0),      WRAM(1),        WRAM(2),        WRAM(3)     },  //0x0d: ram 0 & 1 / ram 2 & 3
	{ EXWRAM(0),    EXWRAM(1),      EXWRAM(2),      EXWRAM(3)   },  //0x0e: exram 0 & 1 / exram 2 & 3
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  },  //0x0f: <invalid setting>
	/* 1 */
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  },  //0x00: <invalid setting>
	{ BASICROM(0),  BASICROM(1),    BASICROM(2),    BASICROM(3) },  //0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ KANJIROM(0),  KANJIROM(1),    KANJIROM(0),    KANJIROM(1) },  //0x02: tv rom 0 & 1 / voice rom 0 & 1
	{ EXROM(1),     EXROM(1),       EXROM(1),       EXROM(1)    },  //0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),     EXROM(0),       EXROM(0),       EXROM(0)    },  //0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ KANJIROM(0),  BASICROM(1),    KANJIROM(0),    BASICROM(3) },  //0x05: tv rom 1 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),  KANJIROM(1),    BASICROM(2),    KANJIROM(1) },  //0x06: basic rom 0 & tv rom 2 / basic rom 2 & voice 1
	{ EXROM(0),     EXROM(1),       EXROM(0),       EXROM(1)    },  //0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),     EXROM(0),       EXROM(1),       EXROM(0)    },  //0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),     BASICROM(1),    EXROM(1),       BASICROM(3) },  //0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),       BASICROM(2),    EXROM(1)    },  //0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),     KANJIROM(1),    EXROM(0),       KANJIROM(1) },  //0x0b: ex rom 0 & tv rom 2 / ex rom 0 & voice 1
	{ KANJIROM(0),  EXROM(0),       KANJIROM(0),    EXROM(0)    },  //0x0c: tv rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(0),      WRAM(1),        WRAM(2),        WRAM(3)     },  //0x0d: ram 0 & 1 / ram 2 & 3
	{ EXWRAM(0),    EXWRAM(1),      EXWRAM(2),      EXWRAM(3)   },  //0x0e: exram 0 & 1 / exram 2 & 3
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  },  //0x0f: <invalid setting>
	/* 2 */
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  },  //0x00: <invalid setting>
	{ BASICROM(0),  BASICROM(1),    BASICROM(2),    BASICROM(3) },  //0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ TVROM(0),     TVROM(1),       VOICEROM(0),    VOICEROM(1) },  //0x02: tv rom 0 & 1 / voice rom 0 & 1
	{ EXROM(1),     EXROM(1),       EXROM(1),       EXROM(1)    },  //0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),     EXROM(0),       EXROM(0),       EXROM(0)    },  //0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ TVROM(1),     BASICROM(1),    VOICEROM(0),    BASICROM(3) },  //0x05: tv rom 1 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),  TVROM(2),       BASICROM(2),    VOICEROM(1) },  //0x06: basic rom 0 & tv rom 2 / basic rom 2 & voice 1
	{ EXROM(0),     EXROM(1),       EXROM(0),       EXROM(1)    },  //0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),     EXROM(0),       EXROM(1),       EXROM(0)    },  //0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),     BASICROM(1),    EXROM(1),       BASICROM(3) },  //0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),       BASICROM(2),    EXROM(1)    },  //0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),     TVROM(2),       EXROM(0),       VOICEROM(1) },  //0x0b: ex rom 0 & tv rom 2 / ex rom 0 & voice 1
	{ TVROM(1),     EXROM(0),       VOICEROM(0),    EXROM(0)    },  //0x0c: tv rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(0),      WRAM(1),        WRAM(2),        WRAM(3)     },  //0x0d: ram 0 & 1 / ram 2 & 3
	{ EXWRAM(0),    EXWRAM(1),      EXWRAM(2),      EXWRAM(3)   },  //0x0e: exram 0 & 1 / exram 2 & 3
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  },  //0x0f: <invalid setting>
	/* 3 */
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  },  //0x00: <invalid setting>
	{ BASICROM(0),  BASICROM(1),    BASICROM(2),    BASICROM(3) },  //0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ KANJIROM(2),  KANJIROM(3),    KANJIROM(2),    KANJIROM(3) },  //0x02: tv rom 0 & 1 / voice rom 0 & 1
	{ EXROM(1),     EXROM(1),       EXROM(1),       EXROM(1)    },  //0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),     EXROM(0),       EXROM(0),       EXROM(0)    },  //0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ KANJIROM(2),  BASICROM(1),    KANJIROM(2),    BASICROM(3) },  //0x05: tv rom 1 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),  KANJIROM(3),    BASICROM(2),    KANJIROM(3) },  //0x06: basic rom 0 & tv rom 2 / basic rom 2 & voice 1
	{ EXROM(0),     EXROM(1),       EXROM(0),       EXROM(1)    },  //0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),     EXROM(0),       EXROM(1),       EXROM(0)    },  //0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),     BASICROM(1),    EXROM(1),       BASICROM(3) },  //0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),       BASICROM(2),    EXROM(1)    },  //0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),     KANJIROM(3),    EXROM(0),       KANJIROM(3) },  //0x0b: ex rom 0 & tv rom 2 / ex rom 0 & voice 1
	{ KANJIROM(2),  EXROM(0),       KANJIROM(2),    EXROM(0)    },  //0x0c: tv rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(0),      WRAM(1),        WRAM(2),        WRAM(3)     },  //0x0d: ram 0 & 1 / ram 2 & 3
	{ EXWRAM(0),    EXWRAM(1),      EXWRAM(2),      EXWRAM(3)   },  //0x0e: exram 0 & 1 / exram 2 & 3
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  }   //0x0f: <invalid setting>
};

static const uint32_t banksw_table_r1[0x10*4][4] = {
	/* 0 */
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  },  //0x00: <invalid setting>
	{ BASICROM(0),  BASICROM(1),    BASICROM(2),    BASICROM(3) },  //0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ VOICEROM(0),  VOICEROM(1),    VOICEROM(0),    VOICEROM(1) },  //0x02: voice rom 0 & 1 / voice rom 0 & 1
	{ EXROM(1),     EXROM(1),       EXROM(1),       EXROM(1)    },  //0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),     EXROM(0),       EXROM(0),       EXROM(0)    },  //0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ VOICEROM(0),  BASICROM(1),    VOICEROM(0),    BASICROM(3) },  //0x05: voice rom 0 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),  VOICEROM(1),    BASICROM(2),    VOICEROM(1) },  //0x06: basic rom 0 & voice rom 1 / basic rom 2 & voice 1
	{ EXROM(0),     EXROM(1),       EXROM(0),       EXROM(1)    },  //0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),     EXROM(0),       EXROM(1),       EXROM(0)    },  //0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),     BASICROM(1),    EXROM(1),       BASICROM(3) },  //0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),       BASICROM(2),    EXROM(1)    },  //0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),     VOICEROM(1),    EXROM(0),       VOICEROM(1) },  //0x0b: ex rom 0 & voice rom 1 / ex rom 0 & voice 1
	{ VOICEROM(0),  EXROM(0),       VOICEROM(0),    EXROM(0)    },  //0x0c: voice rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(4),      WRAM(5),        WRAM(6),        WRAM(7)     },  //0x0d: ram 4 & 5 / ram 6 & 7
	{ EXWRAM(4),    EXWRAM(5),      EXWRAM(6),      EXWRAM(7)   },  //0x0e: exram 4 & 5 / exram 6 & 7
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  },  //0x0f: <invalid setting>
	/* 1 */
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  },  //0x00: <invalid setting>
	{ BASICROM(0),  BASICROM(1),    BASICROM(2),    BASICROM(3) },  //0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ KANJIROM(0),  KANJIROM(1),    KANJIROM(0),    KANJIROM(1) },  //0x02: kanji rom 0 & 1 / kanji rom 0 & 1
	{ EXROM(1),     EXROM(1),       EXROM(1),       EXROM(1)    },  //0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),     EXROM(0),       EXROM(0),       EXROM(0)    },  //0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ KANJIROM(0),  BASICROM(1),    KANJIROM(0),    BASICROM(3) },  //0x05: voice rom 0 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),  KANJIROM(1),    BASICROM(2),    KANJIROM(1) },  //0x06: basic rom 0 & voice rom 1 / basic rom 2 & voice 1
	{ EXROM(0),     EXROM(1),       EXROM(0),       EXROM(1)    },  //0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),     EXROM(0),       EXROM(1),       EXROM(0)    },  //0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),     BASICROM(1),    EXROM(1),       BASICROM(3) },  //0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),       BASICROM(2),    EXROM(1)    },  //0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),     KANJIROM(1),    EXROM(0),       KANJIROM(1) },  //0x0b: ex rom 0 & voice rom 1 / ex rom 0 & voice 1
	{ KANJIROM(0),  EXROM(0),       KANJIROM(0),    EXROM(0)    },  //0x0c: voice rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(4),      WRAM(5),        WRAM(6),        WRAM(7)     },  //0x0d: ram 4 & 5 / ram 6 & 7
	{ EXWRAM(4),    EXWRAM(5),      EXWRAM(6),      EXWRAM(7)   },  //0x0e: exram 4 & 5 / exram 6 & 7
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  },  //0x0f: <invalid setting>
	/* 2 */
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  },  //0x00: <invalid setting>
	{ BASICROM(0),  BASICROM(1),    BASICROM(2),    BASICROM(3) },  //0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ VOICEROM(0),  VOICEROM(1),    VOICEROM(0),    VOICEROM(1) },  //0x02: voice rom 0 & 1 / voice rom 0 & 1
	{ EXROM(1),     EXROM(1),       EXROM(1),       EXROM(1)    },  //0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),     EXROM(0),       EXROM(0),       EXROM(0)    },  //0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ VOICEROM(0),  BASICROM(1),    VOICEROM(0),    BASICROM(3) },  //0x05: voice rom 0 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),  VOICEROM(1),    BASICROM(2),    VOICEROM(1) },  //0x06: basic rom 0 & voice rom 1 / basic rom 2 & voice 1
	{ EXROM(0),     EXROM(1),       EXROM(0),       EXROM(1)    },  //0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),     EXROM(0),       EXROM(1),       EXROM(0)    },  //0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),     BASICROM(1),    EXROM(1),       BASICROM(3) },  //0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),       BASICROM(2),    EXROM(1)    },  //0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),     VOICEROM(1),    EXROM(0),       VOICEROM(1) },  //0x0b: ex rom 0 & voice rom 1 / ex rom 0 & voice 1
	{ VOICEROM(0),  EXROM(0),       VOICEROM(0),    EXROM(0)    },  //0x0c: voice rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(4),      WRAM(5),        WRAM(6),        WRAM(7)     },  //0x0d: ram 4 & 5 / ram 6 & 7
	{ EXWRAM(4),    EXWRAM(5),      EXWRAM(6),      EXWRAM(7)   },  //0x0e: exram 4 & 5 / exram 6 & 7
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  },  //0x0f: <invalid setting>
	/* 3 */
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  },  //0x00: <invalid setting>
	{ BASICROM(0),  BASICROM(1),    BASICROM(2),    BASICROM(3) },  //0x01: basic rom 0 & 1 / basic rom 2 & 3
	{ KANJIROM(2),  KANJIROM(3),    KANJIROM(2),    KANJIROM(3) },  //0x02: kanji rom 0 & 1 / kanji rom 0 & 1
	{ EXROM(1),     EXROM(1),       EXROM(1),       EXROM(1)    },  //0x03: ex rom 1 & 1 / ex rom 1 & 1
	{ EXROM(0),     EXROM(0),       EXROM(0),       EXROM(0)    },  //0x04: ex rom 0 & 0 / ex rom 0 & 0
	{ KANJIROM(2),  BASICROM(1),    KANJIROM(2),    BASICROM(3) },  //0x05: voice rom 0 & basic rom 1 / voice rom 0 & basic 3
	{ BASICROM(0),  KANJIROM(3),    BASICROM(2),    KANJIROM(3) },  //0x06: basic rom 0 & voice rom 1 / basic rom 2 & voice 1
	{ EXROM(0),     EXROM(1),       EXROM(0),       EXROM(1)    },  //0x07: ex rom 0 & ex rom 1 / ex rom 0 & ex rom 1
	{ EXROM(1),     EXROM(0),       EXROM(1),       EXROM(0)    },  //0x08: ex rom 1 & ex rom 0 / ex rom 1 & ex rom 0
	{ EXROM(1),     BASICROM(1),    EXROM(1),       BASICROM(3) },  //0x09: ex rom 1 & basic rom 1 / ex rom 1 & basic 3
	{ BASICROM(0),  EXROM(1),       BASICROM(2),    EXROM(1)    },  //0x0a: basic rom 0 & ex rom 1 / basic rom 2 & ex rom 1
	{ EXROM(0),     KANJIROM(3),    EXROM(0),       KANJIROM(3) },  //0x0b: ex rom 0 & voice rom 1 / ex rom 0 & voice 1
	{ KANJIROM(2),  EXROM(0),       KANJIROM(2),    EXROM(0)    },  //0x0c: voice rom 1 & ex rom 0 / voice rom 0 & ex rom 0
	{ WRAM(4),      WRAM(5),        WRAM(6),        WRAM(7)     },  //0x0d: ram 4 & 5 / ram 6 & 7
	{ EXWRAM(4),    EXWRAM(5),      EXWRAM(6),      EXWRAM(7)   },  //0x0e: exram 4 & 5 / exram 6 & 7
	{ INVALID(0),   INVALID(0),     INVALID(0),     INVALID(0)  }   //0x0f: <invalid setting>
};

WRITE8_MEMBER(pc6001mk2_state::mk2_bank_r0_w)
{
	uint8_t *ROM = m_region_maincpu->base();
	uint8_t *gfx_data = m_region_gfx1->base();

//  bankaddress = 0x10000 + (0x4000 * ((data & 0x40)>>6));
//  membank(1)->set_base(&ROM[bankaddress]);

	m_bank_r0 = data;

//  printf("%02x BANK | %02x\n",data,m_bank_opt);
	m_bank1->set_base(&ROM[banksw_table_r0[(data & 0xf)+(m_bank_opt*0x10)][0]]);
	m_bank2->set_base(&ROM[banksw_table_r0[(data & 0xf)+(m_bank_opt*0x10)][1]]);
	m_bank3->set_base(&ROM[banksw_table_r0[((data & 0xf0)>>4)+(m_bank_opt*0x10)][2]]);
	if(!m_gfx_bank_on)
		m_bank4->set_base(&ROM[banksw_table_r0[((m_bank_r0 & 0xf0)>>4)+(m_bank_opt*0x10)][3]]);
	else
		m_bank4->set_base(&gfx_data[m_cgrom_bank_addr]);
}

WRITE8_MEMBER(pc6001mk2_state::mk2_bank_r1_w)
{
	uint8_t *ROM = m_region_maincpu->base();

//  bankaddress = 0x10000 + (0x4000 * ((data & 0x40)>>6));
//  membank(1)->set_base(&ROM[bankaddress]);

	m_bank_r1 = data;

//  printf("%02x BANK\n",data);
	m_bank5->set_base(&ROM[banksw_table_r1[(data & 0xf)+(m_bank_opt*0x10)][0]]);
	m_bank6->set_base(&ROM[banksw_table_r1[(data & 0xf)+(m_bank_opt*0x10)][1]]);
	m_bank7->set_base(&ROM[banksw_table_r1[((data & 0xf0)>>4)+(m_bank_opt*0x10)][2]]);
	m_bank8->set_base(&ROM[banksw_table_r1[((data & 0xf0)>>4)+(m_bank_opt*0x10)][3]]);
}

WRITE8_MEMBER(pc6001mk2_state::mk2_bank_w0_w)
{
	m_bank_w = data;
}

WRITE8_MEMBER(pc6001mk2_state::mk2_opt_bank_w)
{
	uint8_t *ROM = m_region_maincpu->base();
	uint8_t *gfx_data = m_region_gfx1->base();

	/*
	0 - TVROM / VOICE ROM
	1 - KANJI ROM bank 0
	2 - KANJI ROM bank 1
	3 - TVROM / VOICE ROM
	*/
	m_bank_opt = data & 3;

	m_bank1->set_base(&ROM[banksw_table_r0[(m_bank_r0 & 0xf)+(m_bank_opt*0x10)][0]]);
	m_bank2->set_base(&ROM[banksw_table_r0[(m_bank_r0 & 0xf)+(m_bank_opt*0x10)][1]]);
	m_bank3->set_base(&ROM[banksw_table_r0[((m_bank_r0 & 0xf0)>>4)+(m_bank_opt*0x10)][2]]);
	if(!m_gfx_bank_on)
		m_bank4->set_base(&ROM[banksw_table_r0[((m_bank_r0 & 0xf0)>>4)+(m_bank_opt*0x10)][3]]);
	else
		m_bank4->set_base(&gfx_data[m_cgrom_bank_addr]);
	m_bank4->set_base(&ROM[banksw_table_r0[((m_bank_r0 & 0xf0)>>4)+(m_bank_opt*0x10)][3]]);
	m_bank5->set_base(&ROM[banksw_table_r1[(m_bank_r1 & 0xf)+(m_bank_opt*0x10)][0]]);
	m_bank6->set_base(&ROM[banksw_table_r1[(m_bank_r1 & 0xf)+(m_bank_opt*0x10)][1]]);
	m_bank7->set_base(&ROM[banksw_table_r1[((m_bank_r1 & 0xf0)>>4)+(m_bank_opt*0x10)][2]]);
	m_bank8->set_base(&ROM[banksw_table_r1[((m_bank_r1 & 0xf0)>>4)+(m_bank_opt*0x10)][3]]);

}

WRITE8_MEMBER(pc6001mk2_state::mk2_work_ram0_w)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x01) ? WRAM(0) : EXWRAM(0))] = data;
}

WRITE8_MEMBER(pc6001mk2_state::mk2_work_ram1_w)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x01) ? WRAM(1) : EXWRAM(1))] = data;
}

WRITE8_MEMBER(pc6001mk2_state::mk2_work_ram2_w)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x04) ? WRAM(2) : EXWRAM(2))] = data;
}

WRITE8_MEMBER(pc6001mk2_state::mk2_work_ram3_w)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x04) ? WRAM(3) : EXWRAM(3))] = data;
}

WRITE8_MEMBER(pc6001mk2_state::mk2_work_ram4_w)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x10) ? WRAM(4) : EXWRAM(4))] = data;
}

WRITE8_MEMBER(pc6001mk2_state::mk2_work_ram5_w)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x10) ? WRAM(5) : EXWRAM(5))] = data;
}

WRITE8_MEMBER(pc6001mk2_state::mk2_work_ram6_w)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x40) ? WRAM(6) : EXWRAM(6))] = data;
}

WRITE8_MEMBER(pc6001mk2_state::mk2_work_ram7_w)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x40) ? WRAM(7) : EXWRAM(7))] = data;
}


WRITE8_MEMBER(pc6001mk2_state::necmk2_ppi8255_w)
{
	if (offset==3)
	{
		ppi_control_hack_w(data);

		if((data & 0x0f) == 0x05)
		{
			uint8_t *ROM = m_region_maincpu->base();

			m_gfx_bank_on = 0;
			m_bank4->set_base(&ROM[banksw_table_r0[((m_bank_r0 & 0xf0)>>4)+(m_bank_opt*0x10)][3]]);
		}

		if((data & 0x0f) == 0x04)
		{
			uint8_t *gfx_data = m_region_gfx1->base();

			m_gfx_bank_on = 1;
			m_bank4->set_base(&gfx_data[m_cgrom_bank_addr]);
		}
	}

	m_ppi->write(offset,data);
}

void pc6001mk2_state::vram_bank_change(uint8_t vram_bank)
{
	uint32_t bank_base_values[8] = { 0x8000, 0xc000, 0xc000, 0xe000, 0x0000, 0x8000, 0x4000, 0xa000 };
	uint8_t vram_bank_index = ((vram_bank & 0x60) >> 4) | ((vram_bank & 2) >> 1);
//  uint8_t *work_ram = m_region_maincpu->base();

//  bit 2 of vram_bank sets up 4 color mode
	set_videoram_bank(0x28000 + bank_base_values[vram_bank_index]);

//  popmessage("%02x",vram_bank);
}

WRITE8_MEMBER(pc6001mk2_state::mk2_system_latch_w)
{
	cassette_latch_control((data & 8) == 8);
	m_sys_latch = data;

	m_timer_irq_mask = data & 1;
	vram_bank_change((m_ex_vram_bank & 0x06) | ((m_sys_latch & 0x06) << 4));

	//printf("%02x B0\n",data);
}

inline void pc6001mk2_state::refresh_crtc_params()
{
	/* Apparently bitmap modes changes the screen res to 320 x 200 */
	rectangle visarea = m_screen->visible_area();
	int y_height;

	y_height = (m_exgfx_bitmap_mode || m_exgfx_2bpp_mode) ? 200 : 240;

	visarea.set(0, (320) - 1, 0, (y_height) - 1);

	m_screen->configure(m_screen->width(), m_screen->height(), visarea, m_screen->frame_period().attoseconds());
}

WRITE8_MEMBER(pc6001mk2_state::mk2_vram_bank_w)
{
	//static const uint32_t startaddr[] = {WRAM(6), WRAM(6), WRAM(0), WRAM(4) };

	m_ex_vram_bank = data;
	vram_bank_change((m_ex_vram_bank & 0x06) | ((m_sys_latch & 0x06) << 4));

	m_exgfx_text_mode = ((data & 2) == 0);
	m_cgrom_bank_addr = (data & 2) ? 0x0000 : 0x2000;

	m_exgfx_bitmap_mode = (data & 8);
	m_exgfx_2bpp_mode = ((data & 6) == 0);

	refresh_crtc_params();

//  popmessage("%02x",data);

//  m_video_ram = work_ram + startaddr[(data >> 1) & 0x03];
}

WRITE8_MEMBER(pc6001mk2_state::mk2_col_bank_w)
{
	m_bgcol_bank = (data & 7);
}


WRITE8_MEMBER(pc6001mk2_state::mk2_0xf3_w)
{
	/*
	x--- ---- M1 (?) wait setting
	-x-- ---- ROM wait setting
	--x- ---- RAM wait setting
	---x ---- custom irq 2 address output
	---- x--- custom irq 1 address output
	---- -x-- timer irq mask 2 (mirror?)
	---- --x- custom irq 2 mask
	---- ---x custom irq 1 mask
	*/
	m_timer_irq_mask2 = data & 4;
}

inline void pc6001_state::set_timer_divider(uint8_t data)
{
	m_timer_hz_div = data;
	attotime period = attotime::from_hz((487.5*4)/(m_timer_hz_div+1));
	m_timer_irq_timer->adjust(period,  0, period);
}

WRITE8_MEMBER(pc6001mk2_state::mk2_timer_adj_w)
{
	set_timer_divider(data);
}

WRITE8_MEMBER(pc6001mk2_state::mk2_timer_irqv_w)
{
	m_timer_irq_vector = data;
}

READ8_MEMBER(pc6001mk2_state::mk2_bank_r0_r)
{
	return m_bank_r0;
}

READ8_MEMBER(pc6001mk2_state::mk2_bank_r1_r)
{
	return m_bank_r1;
}

READ8_MEMBER(pc6001mk2_state::mk2_bank_w0_r)
{
	return m_bank_w;
}

void pc6001mk2_state::pc6001mk2_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).bankr("bank1").w(FUNC(pc6001mk2_state::mk2_work_ram0_w));
	map(0x2000, 0x3fff).bankr("bank2").w(FUNC(pc6001mk2_state::mk2_work_ram1_w));
	map(0x4000, 0x5fff).bankr("bank3").w(FUNC(pc6001mk2_state::mk2_work_ram2_w));
	map(0x6000, 0x7fff).bankr("bank4").w(FUNC(pc6001mk2_state::mk2_work_ram3_w));
	map(0x8000, 0x9fff).bankr("bank5").w(FUNC(pc6001mk2_state::mk2_work_ram4_w));
	map(0xa000, 0xbfff).bankr("bank6").w(FUNC(pc6001mk2_state::mk2_work_ram5_w));
	map(0xc000, 0xdfff).bankr("bank7").w(FUNC(pc6001mk2_state::mk2_work_ram6_w));
	map(0xe000, 0xffff).bankr("bank8").w(FUNC(pc6001mk2_state::mk2_work_ram7_w));
}

void pc6001mk2_state::pc6001mk2_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x81).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));

	map(0x90, 0x93).mirror(0x0c).rw(FUNC(pc6001mk2_state::nec_ppi8255_r), FUNC(pc6001mk2_state::necmk2_ppi8255_w));

	map(0xa0, 0xa0).mirror(0x0c).w("ay8910", FUNC(ay8910_device::address_w));
	map(0xa1, 0xa1).mirror(0x0c).w("ay8910", FUNC(ay8910_device::data_w));
	map(0xa2, 0xa2).mirror(0x0c).r("ay8910", FUNC(ay8910_device::data_r));
	map(0xa3, 0xa3).mirror(0x0c).noprw();

	map(0xb0, 0xb0).mirror(0x0f).w(FUNC(pc6001mk2_state::mk2_system_latch_w));

	map(0xc0, 0xc0).w(FUNC(pc6001mk2_state::mk2_col_bank_w));
	map(0xc1, 0xc1).w(FUNC(pc6001mk2_state::mk2_vram_bank_w));
	map(0xc2, 0xc2).w(FUNC(pc6001mk2_state::mk2_opt_bank_w));

	map(0xd0, 0xd3).mirror(0x0c).noprw(); // disk device

	map(0xe0, 0xe3).mirror(0x0c).rw("upd7752", FUNC(upd7752_device::read), FUNC(upd7752_device::write));

	map(0xf0, 0xf0).rw(FUNC(pc6001mk2_state::mk2_bank_r0_r), FUNC(pc6001mk2_state::mk2_bank_r0_w));
	map(0xf1, 0xf1).rw(FUNC(pc6001mk2_state::mk2_bank_r1_r), FUNC(pc6001mk2_state::mk2_bank_r1_w));
	map(0xf2, 0xf2).rw(FUNC(pc6001mk2_state::mk2_bank_w0_r), FUNC(pc6001mk2_state::mk2_bank_w0_w));
	map(0xf3, 0xf3).w(FUNC(pc6001mk2_state::mk2_0xf3_w));
//  AM_RANGE(0xf4
//  AM_RANGE(0xf5
	map(0xf6, 0xf6).w(FUNC(pc6001mk2_state::mk2_timer_adj_w));
	map(0xf7, 0xf7).w(FUNC(pc6001mk2_state::mk2_timer_irqv_w));
}

/*****************************************
 *
 * PC-6601 specific i/o
 *
 ****************************************/

// disk device placeholder
// TODO: identify & hook-up this FDC
READ8_MEMBER(pc6601_state::fdc_r)
{
	return machine().rand();
}

WRITE8_MEMBER(pc6601_state::fdc_w)
{
}

void pc6601_state::pc6601_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	pc6001mk2_io(map);

	// these are disk related
//  AM_RANGE(0xb1
//  AM_RANGE(0xb2
//  AM_RANGE(0xb3

	map(0xd0, 0xdf).rw(FUNC(pc6601_state::fdc_r), FUNC(pc6601_state::fdc_w));
}

/*****************************************
 *
 * PC-6001 SR specific i/o
 *
 ****************************************/

#define SR_SYSROM_1(_v_) \
	0x10000+(0x1000*_v_)
#define SR_SYSROM_2(_v_) \
	0x20000+(0x1000*_v_)
#define SR_CGROM1(_v_) \
	0x30000+(0x1000*_v_)
#define SR_EXROM0(_v_) \
	0x40000+(0x1000*_v_)
#define SR_EXROM1(_v_) \
	0x50000+(0x1000*_v_)
#define SR_EXRAM0(_v_) \
	0x60000+(0x1000*_v_)
#define SR_WRAM0(_v_) \
	0x70000+(0x1000*_v_)
#define SR_NULL(_v_) \
	0x80000+(0x1000*_v_)

READ8_MEMBER(pc6001sr_state::sr_bank_rn_r)
{
	return m_sr_bank_r[offset];
}

WRITE8_MEMBER(pc6001sr_state::sr_bank_rn_w)
{
	memory_bank *bank[8] = { m_bank1, m_bank2, m_bank3, m_bank4, m_bank5, m_bank6, m_bank7, m_bank8 };
	uint8_t *ROM = m_region_maincpu->base();
	uint8_t bank_num;

	m_sr_bank_r[offset] = data;
	bank_num = data & 0x0f;

	switch(data & 0xf0)
	{
		case 0xf0: bank[offset]->set_base(&ROM[SR_SYSROM_1(bank_num)]); break;
		case 0xe0: bank[offset]->set_base(&ROM[SR_SYSROM_2(bank_num)]); break;
		case 0xd0: bank[offset]->set_base(&ROM[SR_CGROM1(bank_num)]); break;
		case 0xc0: bank[offset]->set_base(&ROM[SR_EXROM1(bank_num)]); break;
		case 0xb0: bank[offset]->set_base(&ROM[SR_EXROM0(bank_num)]); break;
		case 0x20: bank[offset]->set_base(&ROM[SR_EXRAM0(bank_num)]); break;
		case 0x00: bank[offset]->set_base(&ROM[SR_WRAM0(bank_num)]); break;
		default:   bank[offset]->set_base(&ROM[SR_NULL(bank_num)]); break;
	}
}

READ8_MEMBER(pc6001sr_state::sr_bank_wn_r)
{
	return m_sr_bank_w[offset];
}

WRITE8_MEMBER(pc6001sr_state::sr_bank_wn_w)
{
	m_sr_bank_w[offset] = data;
}

WRITE8_MEMBER(pc6001sr_state::sr_bitmap_yoffs_w)
{
	m_bitmap_yoffs = data;
}

WRITE8_MEMBER(pc6001sr_state::sr_bitmap_xoffs_w)
{
	m_bitmap_xoffs = data;
}

#define SR_WRAM_BANK_W(_v_) \
{ \
	uint8_t *ROM = m_region_maincpu->base(); \
	uint8_t bank_num; \
	bank_num = m_sr_bank_w[_v_] & 0x0e; \
	if((m_sr_bank_w[_v_] & 0xf0) == 0x00) \
		ROM[offset+(SR_WRAM0(bank_num))] = data; \
	else if((m_sr_bank_w[_v_] & 0xf0) == 0x20) \
		ROM[offset+(SR_EXRAM0(bank_num))] = data; \
}

WRITE8_MEMBER(pc6001sr_state::sr_work_ram0_w)
{
	// TODO: not entirely correct
	if(m_sr_text_mode == false && m_sr_bank_w[0] == 0)
	{
		uint32_t real_offs = (m_bitmap_xoffs*16+m_bitmap_yoffs)*256;
		real_offs += offset;

		m_gvram[real_offs] = data;
		return;
	}

	SR_WRAM_BANK_W(0);
}
WRITE8_MEMBER(pc6001sr_state::sr_work_ram1_w){ SR_WRAM_BANK_W(1); }
WRITE8_MEMBER(pc6001sr_state::sr_work_ram2_w){ SR_WRAM_BANK_W(2); }
WRITE8_MEMBER(pc6001sr_state::sr_work_ram3_w){ SR_WRAM_BANK_W(3); }
WRITE8_MEMBER(pc6001sr_state::sr_work_ram4_w){ SR_WRAM_BANK_W(4); }
WRITE8_MEMBER(pc6001sr_state::sr_work_ram5_w){ SR_WRAM_BANK_W(5); }
WRITE8_MEMBER(pc6001sr_state::sr_work_ram6_w){ SR_WRAM_BANK_W(6); }
WRITE8_MEMBER(pc6001sr_state::sr_work_ram7_w){ SR_WRAM_BANK_W(7); }

WRITE8_MEMBER(pc6001sr_state::sr_mode_w)
{
	// if 1 text mode else bitmap mode
	m_sr_text_mode = bool(BIT(data,3));
	m_sr_text_rows = data & 4 ? 20 : 25;
	// bit 1: bus request

	if(data & 1)
		assert("PC-6001SR in Mk-2 compatibility mode not yet supported!\n");
}

WRITE8_MEMBER(pc6001sr_state::sr_vram_bank_w)
{
	set_videoram_bank(0x70000 + ((data & 0x0f)*0x1000));
}

WRITE8_MEMBER(pc6001sr_state::sr_system_latch_w)
{
	cassette_latch_control((data & 8) == 8);
	m_sys_latch = data;

	m_timer_irq_mask = data & 1;
	//vram_bank_change((m_ex_vram_bank & 0x06) | ((m_sys_latch & 0x06) << 4));

	//printf("%02x B0\n",data);
}

WRITE8_MEMBER(pc6001sr_state::necsr_ppi8255_w)
{
	if (offset==3)
	{
		ppi_control_hack_w(data);

		if(0)
		{
			//printf("%02x\n",data);

			if ((data & 0x0f) == 0x05 && m_cart_rom)
				m_bank1->set_base(m_cart_rom->base() + 0x2000);
			if ((data & 0x0f) == 0x04)
				m_bank1->set_base(m_region_gfx1->base());
		}
	}

	m_ppi->write(offset,data);
}

READ8_MEMBER(pc6001sr_state::hw_rev_r)
{
	// bit 1 is active for pc6601sr, causes a direct jump to "video telopper" for pc6001mk2sr
	return 0;
}

void pc6001sr_state::pc6001sr_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).bankr("bank1").w(FUNC(pc6001sr_state::sr_work_ram0_w));
	map(0x2000, 0x3fff).bankr("bank2").w(FUNC(pc6001sr_state::sr_work_ram1_w));
	map(0x4000, 0x5fff).bankr("bank3").w(FUNC(pc6001sr_state::sr_work_ram2_w));
	map(0x6000, 0x7fff).bankr("bank4").w(FUNC(pc6001sr_state::sr_work_ram3_w));
	map(0x8000, 0x9fff).bankr("bank5").w(FUNC(pc6001sr_state::sr_work_ram4_w));
	map(0xa000, 0xbfff).bankr("bank6").w(FUNC(pc6001sr_state::sr_work_ram5_w));
	map(0xc000, 0xdfff).bankr("bank7").w(FUNC(pc6001sr_state::sr_work_ram6_w));
	map(0xe000, 0xffff).bankr("bank8").w(FUNC(pc6001sr_state::sr_work_ram7_w));
}

void pc6001sr_state::pc6001sr_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
//  0x40-0x43 palette indexes
	map(0x60, 0x67).rw(FUNC(pc6001sr_state::sr_bank_rn_r), FUNC(pc6001sr_state::sr_bank_rn_w));
	map(0x68, 0x6f).rw(FUNC(pc6001sr_state::sr_bank_wn_r), FUNC(pc6001sr_state::sr_bank_wn_w));
	map(0x80, 0x81).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));

	map(0x90, 0x93).mirror(0x0c).rw(FUNC(pc6001sr_state::nec_ppi8255_r), FUNC(pc6001sr_state::necsr_ppi8255_w));

	map(0xa0, 0xa0).mirror(0x0c).w("ay8910", FUNC(ay8910_device::address_w));
	map(0xa1, 0xa1).mirror(0x0c).w("ay8910", FUNC(ay8910_device::data_w));
	map(0xa2, 0xa2).mirror(0x0c).r("ay8910", FUNC(ay8910_device::data_r));
	map(0xa3, 0xa3).mirror(0x0c).noprw();

	map(0xb0, 0xb0).w(FUNC(pc6001sr_state::sr_system_latch_w));
	/* these are disk related */
//  AM_RANGE(0xb1
	map(0xb2, 0xb2).r(FUNC(pc6001sr_state::hw_rev_r));
//  AM_RANGE(0xb3

	map(0xb8, 0xbf).ram().share("irq_vectors");
//  AM_RANGE(0xc0, 0xc0) AM_WRITE(mk2_col_bank_w)
//  AM_RANGE(0xc1, 0xc1) AM_WRITE(mk2_vram_bank_w)
//  AM_RANGE(0xc2, 0xc2) AM_WRITE(opt_bank_w)

	map(0xc8, 0xc8).w(FUNC(pc6001sr_state::sr_mode_w));
	map(0xc9, 0xc9).w(FUNC(pc6001sr_state::sr_vram_bank_w));
	map(0xce, 0xce).w(FUNC(pc6001sr_state::sr_bitmap_yoffs_w));
	map(0xcf, 0xcf).w(FUNC(pc6001sr_state::sr_bitmap_xoffs_w));

	map(0xd0, 0xdf).rw(FUNC(pc6001sr_state::fdc_r), FUNC(pc6001sr_state::fdc_w)); // disk device

	map(0xe0, 0xe3).mirror(0x0c).rw("upd7752", FUNC(upd7752_device::read), FUNC(upd7752_device::write));

//  AM_RANGE(0xf0, 0xf0) AM_READWRITE(mk2_bank_r0_r, mk2_bank_r0_w)
//  AM_RANGE(0xf1, 0xf1) AM_READWRITE(mk2_bank_r1_r, mk2_bank_r1_w)
//  AM_RANGE(0xf2, 0xf2) AM_READWRITE(mk2_bank_w0_r, mk2_bank_w0_w)
	map(0xf3, 0xf3).w(FUNC(pc6001sr_state::mk2_0xf3_w));
//  AM_RANGE(0xf4
//  AM_RANGE(0xf5
	map(0xf6, 0xf6).w(FUNC(pc6001sr_state::mk2_timer_adj_w));
	map(0xf7, 0xf7).w(FUNC(pc6001sr_state::mk2_timer_irqv_w));
}

/* Input ports */
static INPUT_PORTS_START( pc6001 )
	// TODO: is this really a DSW? bit arrangement is also unknown if so.
	PORT_START("MODE4_DSW")
	PORT_DIPNAME( 0x07, 0x00, "Mode 4 GFX colors" )
	PORT_DIPSETTING(    0x00, "Monochrome" )
	PORT_DIPSETTING(    0x01, "Red/Blue" )
	PORT_DIPSETTING(    0x02, "Blue/Red" )
	PORT_DIPSETTING(    0x03, "Pink/Green" )
	PORT_DIPSETTING(    0x04, "Green/Pink" )
	//5-6-7 is presumably invalid

	/* TODO: these two are unchecked */
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("key1") //0x00-0x1f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_UNUSED) //0x00 null
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_UNUSED) //0x01 soh
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_UNUSED) //0x02 stx
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_UNUSED) //0x03 etx
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_UNUSED) //0x04 etx
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_UNUSED) //0x05 eot
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_UNUSED) //0x06 enq
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_UNUSED) //0x07 ack
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0a
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0b lf
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0c vt
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(27)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0e cr
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x0f so

	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x10 si
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x11 dle
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x12 dc1
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x13 dc2
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x14 dc3
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x15 dc4
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x16 nak
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x17 syn
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x18 etb
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x19 cancel
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1a em
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x1b sub
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("key2") //0x20-0x3f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_UNUSED) //0x21 !
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_UNUSED) //0x22 "
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_UNUSED) //0x23 #
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_UNUSED) //0x24 $
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_UNUSED) //0x25 %
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_UNUSED) //0x26 &
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_UNUSED) //0x27 '
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_UNUSED) //0x28 (
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_UNUSED) //0x29 )
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') //0x2c
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') //0x2e
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') //0x2f

	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3a :
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3b ;
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3c <
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3d =
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3e >
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3f ?

	PORT_START("key3") //0x40-0x5f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']')
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("_")

	PORT_START("key_modifiers")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("KANA") PORT_CODE(KEYCODE_RCONTROL) PORT_TOGGLE
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("GRPH") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("STOP") PORT_CODE(KEYCODE_ESC)
INPUT_PORTS_END

TIMER_CALLBACK_MEMBER(pc6001_state::audio_callback)
{
	if(m_cas_switch == 0 && ((m_timer_irq_mask == 0) || (m_timer_irq_mask2 == 0)))
	{
		if(IRQ_LOG) printf("Timer IRQ called %02x\n",m_timer_irq_vector);
		set_maincpu_irq_line(m_timer_irq_vector);
	}
}

INTERRUPT_GEN_MEMBER(pc6001_state::vrtc_irq)
{
	m_cur_keycode = check_joy_press();
	if(IRQ_LOG) printf("Joystick IRQ called 0x16\n");
	set_maincpu_irq_line(0x16);
}

INTERRUPT_GEN_MEMBER(pc6001sr_state::sr_vrtc_irq)
{
	m_kludge ^= 1;

	// TODO: it is unclear who is responsible of the "Joystick IRQ" vs VRTC
	if(m_kludge)
	{
		m_cur_keycode = check_joy_press();
		if(IRQ_LOG) printf("Joystick IRQ called 0x16\n");
		set_maincpu_irq_line(0x16);
	}
	else
	{
		set_maincpu_irq_line(m_sr_irq_vectors[VRTC_IRQ]);
	}
}

IRQ_CALLBACK_MEMBER(pc6001_state::irq_callback)
{
	device.execute().set_input_line(0, CLEAR_LINE);
	return m_irq_vector;
}

READ8_MEMBER(pc6001_state::ppi_porta_r)
{
	return 0;
}

WRITE8_MEMBER(pc6001_state::ppi_porta_w)
{
//  if(data != 0x06)
//      printf("ppi_porta_w %02x\n",data);
}

READ8_MEMBER(pc6001_state::ppi_portb_r)
{
	return 0;
}

WRITE8_MEMBER(pc6001_state::ppi_portb_w)
{
	//printf("ppi_portb_w %02x\n",data);
}

WRITE8_MEMBER(pc6001_state::ppi_portc_w)
{
	//printf("ppi_portc_w %02x\n",data);
}

READ8_MEMBER(pc6001_state::ppi_portc_r)
{
	return 0x88;
}

uint8_t pc6001_state::check_keyboard_press()
{
	int i,port_i,scancode;
	uint8_t shift_pressed,caps_lock;
	scancode = 0;

	shift_pressed = (m_io_key_modifiers->read() & 2)>>1;
	caps_lock = (m_io_key_modifiers->read() & 8)>>3;

	for(port_i=0;port_i<3;port_i++)
	{
		for(i=0;i<32;i++)
		{
			if((m_io_keys[port_i]->read()>>i) & 1)
			{
				if((shift_pressed != caps_lock) && scancode >= 0x41 && scancode <= 0x5f)
					scancode+=0x20;

				if(shift_pressed && scancode >= 0x31 && scancode <= 0x39)
					scancode-=0x10;

				if(shift_pressed && scancode == 0x30) // '0' / '='
					scancode = 0x3d;

				if(shift_pressed && scancode == 0x2c) // ',' / ';'
					scancode = 0x3b;

				if(shift_pressed && scancode == 0x2f) // '/' / '?'
					scancode = 0x3f;

				if(shift_pressed && scancode == 0x2e) // '.' / ':'
					scancode = 0x3a;

				return scancode;
			}
			scancode++;
		}
	}

	return 0;
}

uint8_t pc6001_state::check_joy_press()
{
	uint8_t p1_key = m_io_p1->read() ^ 0xff;
	uint8_t shift_key = m_io_key_modifiers->read() & 0x02;
	uint8_t space_key = m_io_keys[1]->read() & 0x01;
	uint8_t joy_press;

		/*
		    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
		    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
		    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
		    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
		    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
		    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
		    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
		    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
		*/

	joy_press = 0;

	switch(p1_key & 0xf)
	{
		case 0x01: joy_press = 0x04; break; //up
		case 0x02: joy_press = 0x08; break; //down
		case 0x04: joy_press = 0x20; break;
		case 0x05: joy_press = 0x24; break; //up-left
		case 0x06: joy_press = 0x28; break; //down-left
		case 0x08: joy_press = 0x10; break; //right
		case 0x09: joy_press = 0x14; break; //up-right
		case 0x0a: joy_press = 0x18; break; //down-right
	}

	if(p1_key & 0x10 || space_key) { joy_press |= 0x80; } //button 1 (space)
	if(p1_key & 0x20 || shift_key) { joy_press |= 0x01; } //button 2 (shift)

	return joy_press;
}

TIMER_DEVICE_CALLBACK_MEMBER(pc6001_state::cassette_callback)
{
	if(m_cas_switch == 1)
	{
		#if 0
			static uint8_t cas_data_i = 0x80,cas_data_poll;
			//m_cur_keycode = gfx_data[m_cas_offset++];
			if(m_cassette->input() > 0.03)
				cas_data_poll|= cas_data_i;
			else
				cas_data_poll&=~cas_data_i;
			if(cas_data_i == 1)
			{
				m_cur_keycode = cas_data_poll;
				cas_data_i = 0x80;
				/* data ready, poll irq */
				set_maincpu_irq_line(0x08);
			}
			else
				cas_data_i>>=1;
		#else
			m_cur_keycode = m_cas_hack->read_rom(m_cas_offset++);
			popmessage("%04x %04x", m_cas_offset, m_cas_maxsize);
			if(m_cas_offset > m_cas_maxsize)
			{
				m_cas_offset = 0;
				m_cas_switch = 0;
				if(IRQ_LOG) printf("Tape-E IRQ 0x12\n");
				set_maincpu_irq_line(0x12);
			}
			else
			{
				if(IRQ_LOG) printf("Tape-D IRQ 0x08\n");
				set_maincpu_irq_line(0x08);
			}
		#endif
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(pc6001_state::keyboard_callback)
{
	uint32_t key1 = m_io_keys[0]->read();
	uint32_t key2 = m_io_keys[1]->read();
	uint32_t key3 = m_io_keys[2]->read();
//  uint8_t p1_key = m_io_p1->read();

	if(m_cas_switch == 0)
	{
		if((key1 != m_old_key1) || (key2 != m_old_key2) || (key3 != m_old_key3))
		{
			m_cur_keycode = check_keyboard_press();
			if(IRQ_LOG) printf("KEY IRQ 0x02\n");
			set_maincpu_irq_line(0x02);
			m_old_key1 = key1;
			m_old_key2 = key2;
			m_old_key3 = key3;
		}
		#if 0
		else /* joypad polling */
		{
			m_cur_keycode = check_joy_press();
			if(m_cur_keycode)
				set_maincpu_irq_line(0x16);
		}
		#endif
	}
}

void pc6001_state::machine_start()
{
	m_timer_irq_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pc6001_state::audio_callback),this));
}

inline void pc6001_state::set_videoram_bank(uint32_t offs)
{
	m_video_ram = m_region_maincpu->base() + offs;
}

void pc6001_state::machine_reset()
{
	set_videoram_bank(0xc000);

	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x4000, 0x5fff, read8sm_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_cart));

	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	m_port_c_8255=0;

	m_cas_switch = 0;
	m_cas_offset = 0;
	m_cas_maxsize = (m_cas_hack->exists()) ? m_cas_hack->get_rom_size() : 0;
	m_timer_irq_mask = 1;
	m_timer_irq_mask2 = 1;
	// timer irq vector is fixed in plain PC-6001
	m_timer_irq_vector = 0x06;
	set_timer_divider(3);
}

void pc6001mk2_state::machine_reset()
{
	pc6001_state::machine_reset();
	set_videoram_bank(0xc000 + 0x28000);

	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	// TODO: hackish way to simplify bankswitch handling
	if (m_cart_rom)
		memcpy(m_region_maincpu->base() + 0x48000, m_cart_rom->base(), 0x4000);

	/* set default bankswitch */
	{
		uint8_t *ROM = m_region_maincpu->base();
		m_bank_r0 = 0x71;
		m_bank1->set_base(&ROM[BASICROM(0)]);
		m_bank2->set_base(&ROM[BASICROM(1)]);
		m_bank3->set_base(&ROM[EXROM(0)]);
		m_bank4->set_base(&ROM[EXROM(1)]);
		m_bank_r1 = 0xdd;
		m_bank5->set_base(&ROM[WRAM(4)]);
		m_bank6->set_base(&ROM[WRAM(5)]);
		m_bank7->set_base(&ROM[WRAM(6)]);
		m_bank8->set_base(&ROM[WRAM(7)]);
		m_bank_opt = 0x02; //tv rom
		m_bank_w = 0x50; //enable write to work ram 4,5,6,7
		m_gfx_bank_on = 0;
	}

//  refresh_crtc_params();
}

void pc6001sr_state::machine_reset()
{
	pc6001_state::machine_reset();
	set_videoram_bank(0x70000);

	// default to bitmap mode
	m_sr_text_mode = false;
	m_sr_text_rows = 20;

	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	// should this be mirrored into the EXROM regions? hard to tell without an actual cart dump...

	/* set default bankswitch */
	{
		uint8_t *ROM = m_region_maincpu->base();
		m_sr_bank_r[0] = 0xf8; m_bank1->set_base(&ROM[SR_SYSROM_1(0x08)]);
		m_sr_bank_r[1] = 0xfa; m_bank2->set_base(&ROM[SR_SYSROM_1(0x0a)]);
		m_sr_bank_r[2] = 0xc0; m_bank3->set_base(&ROM[SR_EXROM1(0x00)]);
		m_sr_bank_r[3] = 0xb0; m_bank4->set_base(&ROM[SR_EXROM0(0x00)]);
		m_sr_bank_r[4] = 0x08; m_bank5->set_base(&ROM[SR_WRAM0(0x08)]);
		m_sr_bank_r[5] = 0x0a; m_bank6->set_base(&ROM[SR_WRAM0(0x0a)]);
		m_sr_bank_r[6] = 0x0c; m_bank7->set_base(&ROM[SR_WRAM0(0x0c)]);
		m_sr_bank_r[7] = 0x0e; m_bank8->set_base(&ROM[SR_WRAM0(0x0e)]);
//      m_bank_opt = 0x02; //tv rom

		/* enable default work RAM writes */
		m_sr_bank_w[0] = 0x00;
		m_sr_bank_w[1] = 0x02;
		m_sr_bank_w[2] = 0x04;
		m_sr_bank_w[3] = 0x06;
		m_sr_bank_w[4] = 0x08;
		m_sr_bank_w[5] = 0x0a;
		m_sr_bank_w[6] = 0x0c;
		m_sr_bank_w[7] = 0x0e;

//      m_gfx_bank_on = 0;
	}
}


static const gfx_layout char_layout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};


static const gfx_layout kanji_layout =
{
	16, 16,
	RGN_FRAC(1,2),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
	0+RGN_FRAC(1,2), 1+RGN_FRAC(1,2), 2+RGN_FRAC(1,2), 3+RGN_FRAC(1,2), 4+RGN_FRAC(1,2), 5+RGN_FRAC(1,2), 6+RGN_FRAC(1,2), 7+RGN_FRAC(1,2)  },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};

static GFXDECODE_START( gfx_pc6001m2 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, char_layout, 2, 1 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, kanji_layout, 2, 1 )
GFXDECODE_END

#define PC6001_MAIN_CLOCK 7987200

void pc6001_state::pc6001(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, PC6001_MAIN_CLOCK / 2); // PD 780C-1, ~4 Mhz
	m_maincpu->set_addrmap(AS_PROGRAM, &pc6001_state::pc6001_map);
	m_maincpu->set_addrmap(AS_IO, &pc6001_state::pc6001_io);
	m_maincpu->set_vblank_int("screen", FUNC(pc6001_state::vrtc_irq));
	m_maincpu->set_irq_acknowledge_callback(FUNC(pc6001_state::irq_callback));

//  I8049(config, "subcpu", 7987200);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_pc6001m2);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_screen_update(FUNC(pc6001_state::screen_update_pc6001));
	m_screen->set_size(320, 25+192+26);
	m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(pc6001_state::pc6001_palette), 16+4);

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(pc6001_state::ppi_porta_r));
	m_ppi->out_pa_callback().set(FUNC(pc6001_state::ppi_porta_w));
	m_ppi->in_pb_callback().set(FUNC(pc6001_state::ppi_portb_r));
	m_ppi->out_pb_callback().set(FUNC(pc6001_state::ppi_portb_w));
	m_ppi->in_pc_callback().set(FUNC(pc6001_state::ppi_portc_r));
	m_ppi->out_pc_callback().set(FUNC(pc6001_state::ppi_portc_w));

	/* uart */
	I8251(config, "uart", 0);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "pc6001_cart");

//  CASSETTE(config, m_cassette);
//  m_cassette->set_formats(pc6001_cassette_formats);
//  m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	GENERIC_CARTSLOT(config, m_cas_hack, generic_plain_slot, "pc6001_cass", "cas,p6");

	SPEAKER(config, "mono").front_center();
	ay8910_device &ay8910(AY8910(config, "ay8910", PC6001_MAIN_CLOCK/4));
	ay8910.port_a_read_callback().set_ioport("P1");
	ay8910.port_b_read_callback().set_ioport("P2");
	ay8910.add_route(ALL_OUTPUTS, "mono", 1.00);
//  WAVE(config, "wave", "cassette").add_route(ALL_OUTPUTS, "mono", 0.25);

	/* TODO: accurate timing on this */
	TIMER(config, "keyboard_timer").configure_periodic(FUNC(pc6001_state::keyboard_callback), attotime::from_hz(250));
	TIMER(config, "cassette_timer").configure_periodic(FUNC(pc6001_state::cassette_callback), attotime::from_hz(1200/12));
}



void pc6001mk2_state::pc6001mk2(machine_config &config)
{
	pc6001(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &pc6001mk2_state::pc6001mk2_map);
	m_maincpu->set_addrmap(AS_IO, &pc6001mk2_state::pc6001mk2_io);

//  MCFG_MACHINE_RESET_OVERRIDE(pc6001mk2_state,pc6001mk2)

	m_screen->set_screen_update(FUNC(pc6001mk2_state::screen_update_pc6001mk2));

	m_palette->set_entries(16+16);
	m_palette->set_init(FUNC(pc6001mk2_state::pc6001mk2_palette));

	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_pc6001m2);

	UPD7752(config, "upd7752", PC6001_MAIN_CLOCK/4).add_route(ALL_OUTPUTS, "mono", 1.00);
}

void pc6601_state::pc6601(machine_config &config)
{
	pc6001mk2(config);

	/* basic machine hardware */
	Z80(config.replace(), m_maincpu, PC6001_MAIN_CLOCK / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc6601_state::pc6001mk2_map);
	m_maincpu->set_addrmap(AS_IO, &pc6601_state::pc6601_io);
	m_maincpu->set_vblank_int("screen", FUNC(pc6001_state::vrtc_irq));
	m_maincpu->set_irq_acknowledge_callback(FUNC(pc6001_state::irq_callback));
}

void pc6001sr_state::pc6001sr(machine_config &config)
{
	pc6001mk2(config);

	/* basic machine hardware */
	//*Yes*, PC-6001 SR Z80 CPU is actually slower than older models (better waitstates tho?)
	Z80(config.replace(), m_maincpu, XTAL(3'579'545));
	m_maincpu->set_addrmap(AS_PROGRAM, &pc6001sr_state::pc6001sr_map);
	m_maincpu->set_addrmap(AS_IO, &pc6001sr_state::pc6001sr_io);
	m_maincpu->set_vblank_int("screen", FUNC(pc6001sr_state::sr_vrtc_irq));
	m_maincpu->set_irq_acknowledge_callback(FUNC(pc6001_state::irq_callback));

//  MCFG_MACHINE_RESET_OVERRIDE(pc6001sr_state,pc6001sr)

	m_screen->set_screen_update(FUNC(pc6001sr_state::screen_update_pc6001sr));
}

/* ROM definition */
ROM_START( pc6001 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basicrom.60", 0x0000, 0x4000, CRC(54c03109) SHA1(c622fefda3cdc2b87a270138f24c05828b5c41d2) )

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i8049", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "cgrom60.60", 0x0000, 0x1000, CRC(b0142d32) SHA1(9570495b10af5b1785802681be94b0ea216a1e26) )
	ROM_RELOAD(             0x1000, 0x1000 )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_ERASEFF )
ROM_END

ROM_START( pc6001a )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basicrom.60a", 0x0000, 0x4000, CRC(fa8e88d9) SHA1(c82e30050a837e5c8ffec3e0c8e3702447ffd69c) )

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i8049", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "cgrom60.60a", 0x0000, 0x1000, CRC(49c21d08) SHA1(9454d6e2066abcbd051bad9a29a5ca27b12ec897) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_ERASEFF )
ROM_END

ROM_START( pc6001mk2 )
	ROM_REGION( 0x50000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basicrom.62", 0x10000, 0x8000, CRC(950ac401) SHA1(fbf195ba74a3b0f80b5a756befc96c61c2094182) )
	ROM_LOAD( "voicerom.62", 0x18000, 0x4000, CRC(49b4f917) SHA1(1a2d18f52ef19dc93da3d65f19d3abbd585628af) )
	ROM_LOAD( "cgrom60.62",  0x1c000, 0x2000, CRC(81eb5d95) SHA1(53d8ae9599306ff23bf95208d2f6cc8fed3fc39f) )
	ROM_LOAD( "cgrom60m.62", 0x1e000, 0x2000, CRC(3ce48c33) SHA1(f3b6c63e83a17d80dde63c6e4d86adbc26f84f79) )
	ROM_LOAD( "kanjirom.62", 0x20000, 0x8000, CRC(20c8f3eb) SHA1(4c9f30f0a2ebbe70aa8e697f94eac74d8241cadd) )
	// work ram              0x28000,0x10000
	// extended work ram     0x38000,0x10000
	// exrom                 0x48000, 0x4000
	// <invalid>             0x4c000, 0x4000

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i8049", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_COPY( "maincpu", 0x1c000, 0x00000, 0x4000 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "maincpu", 0x20000, 0x00000, 0x8000 )
ROM_END

ROM_START( pc6601 ) /* Variant of pc6001m2 */
	ROM_REGION( 0x50000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basicrom.66", 0x10000, 0x8000, CRC(c0b01772) SHA1(9240bb6b97fe06f5f07b5d65541c4d2f8758cc2a) )
	ROM_LOAD( "voicerom.66", 0x18000, 0x4000, CRC(91d078c1) SHA1(6a93bd7723ef67f461394530a9feee57c8caf7b7) )
	ROM_LOAD( "cgrom60.66",  0x1c000, 0x2000, CRC(d2434f29) SHA1(a56d76f5cbdbcdb8759abe601eab68f01b0a8fe8) )
	ROM_LOAD( "cgrom66.66",  0x1e000, 0x2000, CRC(3ce48c33) SHA1(f3b6c63e83a17d80dde63c6e4d86adbc26f84f79) )
	ROM_LOAD( "kanjirom.66", 0x20000, 0x8000, CRC(20c8f3eb) SHA1(4c9f30f0a2ebbe70aa8e697f94eac74d8241cadd) )
	// exrom                 0x48000, 0x4000

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i8049", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_COPY( "maincpu", 0x1c000, 0x00000, 0x4000 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "maincpu", 0x20000, 0x00000, 0x8000 )
ROM_END

ROM_START( pc6001sr )
	ROM_REGION( 0x90000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "systemrom1.64", 0x10000, 0x10000, CRC(b6fc2db2) SHA1(dd48b1eee60aa34780f153359f5da7f590f8dff4) )
	ROM_LOAD( "systemrom2.64", 0x20000, 0x10000, CRC(55a62a1d) SHA1(3a19855d290fd4ac04e6066fe4a80ecd81dc8dd7) )
//  cgrom 1                    0x30000, 0x10000
//  exrom 0                    0x40000, 0x10000
//  exrom 1                    0x50000, 0x10000
//  exram 0                    0x60000, 0x10000
//  work ram 0                 0x70000, 0x10000
//  <invalid>                  0x80000, 0x10000

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i8049", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "cgrom68.64", 0x0000, 0x4000, CRC(73bc3256) SHA1(5f80d62a95331dc39b2fb448a380fd10083947eb) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "maincpu", 0x28000, 0x00000, 0x8000 )
ROM_END

//    YEAR  NAME       PARENT  COMPAT MACHINE    INPUT   STATE            INIT        COMPANY  FULLNAME                 FLAGS
COMP( 1981, pc6001,    0,      0,     pc6001,    pc6001, pc6001_state,    empty_init, "NEC",   "PC-6001 (Japan)",       MACHINE_NOT_WORKING )
COMP( 1981, pc6001a,   pc6001, 0,     pc6001,    pc6001, pc6001_state,    empty_init, "NEC",   "PC-6001A (US)",         MACHINE_NOT_WORKING ) // This version is also known as the NEC Trek
COMP( 1983, pc6001mk2, pc6001, 0,     pc6001mk2, pc6001, pc6001mk2_state, empty_init, "NEC",   "PC-6001mkII (Japan)",   MACHINE_NOT_WORKING )
COMP( 1983, pc6601,    pc6001, 0,     pc6601,    pc6001, pc6601_state,    empty_init, "NEC",   "PC-6601 (Japan)",       MACHINE_NOT_WORKING )
COMP( 1984, pc6001sr,  pc6001, 0,     pc6001sr,  pc6001, pc6001sr_state,  empty_init, "NEC",   "PC-6001mkIISR (Japan)", MACHINE_NOT_WORKING )
