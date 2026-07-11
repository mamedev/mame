// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PC-6001 series (c) 1981 NEC

TODO:
- Move MCU HLE simulation in a device;
- Remove the 8255 hacks;
- Proper support for .wav/.p6t file formats used by the cassette interface;
- Make FDC to actually load images, also fix .dsk identification;
- Confirm irq model daisy chain behaviour, and add missing irqs and features
  (namely the irq dispatch for SR mode should really honor I/O $fb and fallback to legacy
   behaviour if masked);
- Several games are decidedly too fast, down to missing waitstates, no screen raw
  parameters, crtkill signal and bus request;
- Merge PC-6001 video emulation with MC6847 (is it really one or rather a M5C6847P-1?);
- Pinpoint what VDG supersets PC-6001mkII and SR models really uses;
- upd7752 voice speech device needs to be properly emulated (device is currently a skeleton),
  pc6001mk2_cass:chrith is a good test case, it's supposed to talk before title screen;

TODO (pc6001mk2):
- refactor memory model to use address_map_bank_device;
- confirm optional FDC used mapped at 0xd0-0xd3
\- PC-6031? It looks like a 5'25 single drive with 8255 protocol, presumably earlier revision
   of PC-80S31 with no dump available;

TODO (pc6601):
- current regression caused by an internal FDC sense interrupt status that expects a
  DIO high that never occurs;
- mon r-0 type games doesn't seem to work at all on this system?
  Update: tries to autoload cassette at startup for some reason.

TODO (pc6601mk2sr):
- Implement MK-2 compatibility mode via view handler(s)
  (it changes the memory map to behave like the older versions);
- Video Telopper (superimposer) & TV tuner functions for later machines;
- pc6001mk2sr/pc6601sr: currently doesn't work without -debug enabled (?), has serious keyboard
  issues, BASIC based programs hangs at a $e6bb check (irq not fired regression? bp 102c,1,{pc+=2;g})

===================================================================================================

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

===================================================================================================

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
irq vector 0x14: same as 2, (A = 0x00, B = 0x01)                                            ;keyboard control irq (function keys)
irq vector 0x16: tests ppi port c, writes the result to $feca.                              ;joystick / sub irq trigger
irq vector 0x18:                                                                            ;TVR (?)
irq vector 0x1a:                                                                            ;Date
irq vector 0x1c:                                                                            ;(unused)
irq vector 0x1e:                                                                            ;(unused)
irq vector 0x20:                                                                            ;uPD7752 voice irq
irq vector 0x22:                                                                            ;VRTC (SR only?)
irq vector 0x24:                                                                            ;(unused)
irq vector 0x26:                                                                            ;(unused)

***************************************************************************************************/

#include "emu.h"
#include "pc6001.h"

#include "softlist_dev.h"

#define LOG_IRQ    (1U << 1)

#define VERBOSE (0)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

#define LOGIRQ(...)     LOGMASKED(LOG_IRQ, __VA_ARGS__)

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
	}
}

// TODO: this is explicitly needed for making all machines to boot
inline void pc6001_state::ppi_control_hack_w(uint8_t data)
{
	if(data & 1)
		m_port_c_8255 |=   1<<((data>>1)&0x07);
	else
		m_port_c_8255 &= ~(1<<((data>>1)&0x07));

#if 0
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

inline void pc6001_state::set_subcpu_irq_vector(u8 vector_num)
{
	// TODO: if anything the new vector must never override an old one
	// sub CPU INT pin is directly connected to the OBFA (PC7) on PPI while WR is connected to INTRA (PC3),
	// former should act as an handshake flag to inhibit further irqs until current is processed.
	// (Also note tight loops in main CPU code that checks against masks 0x28 or 0x88 for I/O $92)
	LOGIRQ("%s: sub CPU new irq vector old: %02x new: %02x\n", machine().describe_context(), m_sub_vector, vector_num);
	m_sub_vector = vector_num;
	set_irq_level(SUB_CPU_IRQ);
}

inline void pc6001_state::set_irq_level(int which)
{
	m_irq_pending |= 1 << which;
	m_maincpu->set_input_line(0, ASSERT_LINE);
	LOGIRQ("%s: assert %d, state %02x\n", machine().describe_context(), which, m_irq_pending);
}

void pc6001_state::system_latch_w(uint8_t data)
{
	static const uint16_t startaddr[] = {0xC000, 0xE000, 0x8000, 0xA000 };

	m_video_base = &m_ram[startaddr[(data >> 1) & 0x03] - 0x8000];

	cassette_latch_control((data & 8) == 8);
	m_sys_latch = data;

	m_timer_enable = !(data & 1);
	set_timer_divider();
}


uint8_t pc6001_state::nec_ppi8255_r(offs_t offset)
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

void pc6001_state::nec_ppi8255_w(offs_t offset, uint8_t data)
{
	if (offset==3)
	{
		ppi_control_hack_w(data);
		//printf("%02x\n",data);

		if ((data & 0x0e) == 0x04)
			m_cart_bank->set_bank(data & 1);

//      if ((data & 0x0f) == 0x05 && m_cart_rom)
//          m_bank1->set_base(m_cart_rom->base() + 0x2000);
//      if ((data & 0x0f) == 0x04)
//          m_bank1->set_base(m_region_gfx1->base());
	}

	m_ppi->write(offset,data);
}

uint8_t pc6001_state::joystick_r()
{
	uint8_t data = m_joymux->output_r();

	// FIXME: bits 6 and 7 are supposed to be nHSYNC and nVSYNC
	// mk2SR vrtc irq expects VSYNC bit to be high (at line 240 essentially),
	// otherwise it refuses to clear the $e6bb blank buffer.
	if (!m_screen->hblank())
		data &= 0xbf;
	else
		data |= 0x40;
	if (!m_screen->vblank())
		data &= 0x7f;
	else
		data |= 0x80;

	return data;
}

uint8_t pc6001_state::joystick_out_r()
{
	return m_joystick_out;
}

void pc6001_state::joystick_out_w(uint8_t data)
{
	// bit 7 is output enable for first part of 74LS367 buffer
	m_joy[1]->pin_6_w(BIT(data, 7) ? 1 : BIT(data, 0));
	m_joy[1]->pin_7_w(BIT(data, 7) ? 1 : BIT(data, 1));
	m_joy[0]->pin_6_w(BIT(data, 7) ? 1 : BIT(data, 2));
	m_joy[0]->pin_7_w(BIT(data, 7) ? 1 : BIT(data, 3));
	m_joy[1]->pin_8_w(BIT(data, 4));
	m_joy[0]->pin_8_w(BIT(data, 5));
	m_joymux->select_w(BIT(data, 6));

	m_joystick_out = data;
}

uint8_t pc6001_state::portc0_r()
{
	// bit 0: RS232 carrier detect
	// bit 1: printer ready
	uint8_t data = 0xfd;
	if (!m_centronics_busy)
		data |= 0x02;

	return data;
}

void pc6001_state::pc6001_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom().nopw();
	map(0x4000, 0x7fff).m(m_cart_bank, FUNC(address_map_bank_device::amap8));
	map(0x8000, 0xffff).ram().share("ram");
}

void pc6001_state::pc6001_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x81).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x90, 0x93).mirror(0x0c).rw(FUNC(pc6001_state::nec_ppi8255_r), FUNC(pc6001_state::nec_ppi8255_w));
	map(0xa0, 0xa0).mirror(0x0c).w(m_ay, FUNC(ay8910_device::address_w));
	map(0xa1, 0xa1).mirror(0x0c).w(m_ay, FUNC(ay8910_device::data_w));
	map(0xa2, 0xa2).mirror(0x0c).r(m_ay, FUNC(ay8910_device::data_r));
	map(0xa3, 0xa3).mirror(0x0c).nopw();
	map(0xb0, 0xb0).mirror(0x0f).w(FUNC(pc6001_state::system_latch_w));
	map(0xc0, 0xc0).mirror(0x0f).r(FUNC(pc6001_state::portc0_r));
}

// TODO: sharrier (at least) pretends to use this as writable work RAM
// it also pretends to act as a mk2 machine, judging at $f0 writes.
// Does it fail an identifier check?
// There's really a work RAM when cart not inserted, or it actually wants a RAM cart?
void pc6001_state::cart_map(address_map &map)
{
	map(0x2000, 0x3fff).rom().region("gfx1", 0);
	map(0x4000, 0x7fff).r(m_cart, FUNC(generic_slot_device::read_rom));
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

void pc6001mk2_state::mk2_bank_r0_w(uint8_t data)
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

void pc6001mk2_state::mk2_bank_r1_w(uint8_t data)
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

void pc6001mk2_state::mk2_bank_w0_w(uint8_t data)
{
	m_bank_w = data;
}

void pc6001mk2_state::mk2_opt_bank_w(uint8_t data)
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

void pc6001mk2_state::mk2_work_ram0_w(offs_t offset, uint8_t data)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x01) ? WRAM(0) : EXWRAM(0))] = data;
}

void pc6001mk2_state::mk2_work_ram1_w(offs_t offset, uint8_t data)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x01) ? WRAM(1) : EXWRAM(1))] = data;
}

void pc6001mk2_state::mk2_work_ram2_w(offs_t offset, uint8_t data)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x04) ? WRAM(2) : EXWRAM(2))] = data;
}

void pc6001mk2_state::mk2_work_ram3_w(offs_t offset, uint8_t data)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x04) ? WRAM(3) : EXWRAM(3))] = data;
}

void pc6001mk2_state::mk2_work_ram4_w(offs_t offset, uint8_t data)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x10) ? WRAM(4) : EXWRAM(4))] = data;
}

void pc6001mk2_state::mk2_work_ram5_w(offs_t offset, uint8_t data)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x10) ? WRAM(5) : EXWRAM(5))] = data;
}

void pc6001mk2_state::mk2_work_ram6_w(offs_t offset, uint8_t data)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x40) ? WRAM(6) : EXWRAM(6))] = data;
}

void pc6001mk2_state::mk2_work_ram7_w(offs_t offset, uint8_t data)
{
	uint8_t *ROM = m_region_maincpu->base();
	ROM[offset+((m_bank_w & 0x40) ? WRAM(7) : EXWRAM(7))] = data;
}


void pc6001mk2_state::necmk2_ppi8255_w(offs_t offset, uint8_t data)
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

void pc6001mk2_state::mk2_system_latch_w(uint8_t data)
{
	cassette_latch_control((data & 8) == 8);
	m_sys_latch = data;

	m_timer_enable = !(data & 1);
	set_timer_divider();

	vram_bank_change((m_ex_vram_bank & 0x06) | ((m_sys_latch & 0x06) << 4));
}

inline void pc6001mk2_state::refresh_crtc_params()
{
	/* Apparently bitmap modes changes the screen res to 320 x 200 */
	rectangle visarea = m_screen->visible_area();
	int y_height;

	// TODO: probably needs the *actual* mode used to fix hudson3 bottom part
	y_height = (m_exgfx_bitmap_mode || m_exgfx_2bpp_mode) ? 200 : 240;

	visarea.set(0, (320) - 1, 0, (y_height) - 1);

	m_screen->configure(m_screen->width(), m_screen->height(), visarea, m_screen->frame_period().attoseconds());
}

void pc6001mk2_state::mk2_vram_bank_w(uint8_t data)
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

//  m_video_base = work_ram + startaddr[(data >> 1) & 0x03];
}

void pc6001mk2_state::mk2_col_bank_w(uint8_t data)
{
	m_bgcol_bank = (data & 7);
}


void pc6001mk2_state::mk2_0xf3_w(uint8_t data)
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
	m_timer_irq_mask = BIT(data, 2);
}

inline u8 pc6001_state::get_timer_base_divider()
{
	return 4;
}

inline u8 pc6001mk2sr_state::get_timer_base_divider()
{
//  if (sr_mode == false)
//      return pc6001mk2_state::get_timer_base_divider();
	return 0x80;
}

inline void pc6001_state::set_timer_divider()
{
	if (m_timer_enable == false)
	{
		m_timer_irq_timer->adjust(attotime::never);
		return;
	}
	attotime period = attotime::from_hz((487.5 * get_timer_base_divider()) / (m_timer_hz_div+1));
	m_timer_irq_timer->adjust(period,  0, period);
}

void pc6001mk2_state::mk2_timer_adj_w(uint8_t data)
{
	m_timer_hz_div = data;
	set_timer_divider();
}

void pc6001mk2_state::mk2_timer_irqv_w(uint8_t data)
{
	m_timer_irq_vector = data;
}

uint8_t pc6001mk2_state::mk2_bank_r0_r()
{
	return m_bank_r0;
}

uint8_t pc6001mk2_state::mk2_bank_r1_r()
{
	return m_bank_r1;
}

uint8_t pc6001mk2_state::mk2_bank_w0_r()
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

	map(0xa0, 0xa0).mirror(0x0c).w(m_ay, FUNC(ay8910_device::address_w));
	map(0xa1, 0xa1).mirror(0x0c).w(m_ay, FUNC(ay8910_device::data_w));
	map(0xa2, 0xa2).mirror(0x0c).r(m_ay, FUNC(ay8910_device::data_r));
	map(0xa3, 0xa3).mirror(0x0c).noprw();

	map(0xb0, 0xb0).mirror(0x0f).w(FUNC(pc6001mk2_state::mk2_system_latch_w));

	map(0xc0, 0xc0).mirror(0x0f).r(FUNC(pc6001_state::portc0_r));
	map(0xc0, 0xc0).w(FUNC(pc6001mk2_state::mk2_col_bank_w));
	map(0xc1, 0xc1).w(FUNC(pc6001mk2_state::mk2_vram_bank_w));
	map(0xc2, 0xc2).w(FUNC(pc6001mk2_state::mk2_opt_bank_w));

	map(0xe0, 0xe3).mirror(0x0c).rw("upd7752", FUNC(upd7752_device::read), FUNC(upd7752_device::write));

	map(0xf0, 0xf0).rw(FUNC(pc6001mk2_state::mk2_bank_r0_r), FUNC(pc6001mk2_state::mk2_bank_r0_w));
	map(0xf1, 0xf1).rw(FUNC(pc6001mk2_state::mk2_bank_r1_r), FUNC(pc6001mk2_state::mk2_bank_r1_w));
	map(0xf2, 0xf2).rw(FUNC(pc6001mk2_state::mk2_bank_w0_r), FUNC(pc6001mk2_state::mk2_bank_w0_w));
	map(0xf3, 0xf3).w(FUNC(pc6001mk2_state::mk2_0xf3_w));
//  map(0xf4
//  map(0xf5
	map(0xf6, 0xf6).w(FUNC(pc6001mk2_state::mk2_timer_adj_w));
	map(0xf7, 0xf7).w(FUNC(pc6001mk2_state::mk2_timer_irqv_w));
}

/*****************************************
 *
 * PC-6601 specific I/O
 *
 ****************************************/

// disk device I/F
void pc6601_state::fdc_sel_w(uint8_t data)
{
	// bit 2 selects between internal (0) and external (1) FDC interfaces
	// other bits unknown purpose
	m_fdc_intf_view.select((data & 4) >> 2);
}

u8 pc6601_state::fdc_mon_r()
{
	// bit 0 reads the motor line status (active low)
	return 0;
}

void pc6601_state::fdc_mon_w(u8 data)
{
	m_floppy->get_device()->mon_w(!BIT(data,0));
}

// TODO: 0xd0, 0xd3 FIFO data buffer from/to FDC (external DMA?)
// PC-6001mk2SR / PC-6601SR tests these FIFO ports first,
// ditching attempt to floppy load if reading doesn't match 0x55aa previous writes.
// PC-6601 ignores this.

// TODO: hangs at first internal FDC command sense irq status in PC-6601 (and later machines if data buffer is enabled)
// It recalibrate and expects that uPD765 returns a DIO high that never happens.

void pc6601_state::pc6601_fdc_io(address_map &map)
{
	map(0xb1, 0xb1).mirror(0x4).w(FUNC(pc6601_state::fdc_sel_w));
	map(0xb2, 0xb2).lr8([this]() { return m_fdc->get_irq() ? 1 : 0; }, "FDCINT");
	//map(0xb3, 0xb3).w  b3 is written when b2 is read
	map(0xd0, 0xdf).view(m_fdc_intf_view);
	m_fdc_intf_view[0](0xd4, 0xd4).r(FUNC(pc6601_state::fdc_mon_r));
	m_fdc_intf_view[0](0xd6, 0xd6).w(FUNC(pc6601_state::fdc_mon_w));
	m_fdc_intf_view[0](0xdc, 0xdd).m(m_fdc, FUNC(upd765a_device::map));

	m_fdc_intf_view[1](0xd0, 0xd3).mirror(0x4).m(m_pc80s31, FUNC(pc80s31_device::host_map));
}

void pc6601_state::pc6601_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	pc6001mk2_io(map);

	pc6601_fdc_io(map);
}

/*****************************************
 *
 * PC-6001 SR specific i/o
 *
 ****************************************/

u8 pc6001mk2sr_state::sr_bank_reg_r(offs_t offset)
{
	return m_sr_bank_reg[offset];
}

// offset & 8 maps to bank writes
void pc6001mk2sr_state::sr_bank_reg_w(offs_t offset, u8 data)
{
	// TODO: is bit 0 truly a NOP?
	m_sr_bank_reg[offset] = data & 0xfe;
	m_sr_bank[offset]->set_bank(m_sr_bank_reg[offset] >> 1);
}

void pc6001mk2sr_state::sr_bitmap_yoffs_w(u8 data)
{
	m_bitmap_yoffs = data;
}

void pc6001mk2sr_state::sr_bitmap_xoffs_w(u8 data)
{
	m_bitmap_xoffs = data;

	if (data)
		popmessage("xoffs write %02x", data);
}

u8 pc6001mk2sr_state::work_ram_r(offs_t offset)
{
//  if (m_sr_text_mode == false && (offset & 0xe000) == 0) && m_sr_mode)
	if (m_sr_text_mode == false && (offset & 0xe000) == 0)
		return sr_gvram_r(offset);

	return m_ram[offset];
}

void pc6001mk2sr_state::work_ram_w(offs_t offset, u8 data)
{
//  if (m_sr_text_mode == false && (offset & 0xe000) == 0) && m_sr_mode)
	if (m_sr_text_mode == false && (offset & 0xe000) == 0)
	{
		sr_gvram_w(offset, data);
		return;
	}

	m_ram[offset] = data;
}

// TODO: does this maps to the work RAM in an alt fashion?
// Games that uses GVRAM never writes outside 0-0x13f, and xoffs is unconfirmed.
// Games also expects that GVRAM is read-back as 4-bit, otherwise fill issues happens
// By logic it should be a 320x204 8-bit area -> 0x7f80
// The interface is otherwise pretty inconvenient,
// and it is unconfirmed if raster effect over SR text mode is even possible (which would halve the time required for drawing)
// or if double buffering is possible (that would require EXRAM installed)
u8 pc6001mk2sr_state::sr_gvram_r(offs_t offset)
{
	uint32_t real_offs = (m_bitmap_xoffs*16+m_bitmap_yoffs)*320;
	real_offs += offset;

	return m_gvram[real_offs];
}

void pc6001mk2sr_state::sr_gvram_w(offs_t offset, u8 data)
{
	uint32_t real_offs = (m_bitmap_xoffs*16+m_bitmap_yoffs)*320;
	real_offs += offset;

	m_gvram[real_offs] = data;
}

void pc6001mk2sr_state::sr_mode_w(u8 data)
{
	// if 1 text mode else bitmap mode
	m_sr_text_mode = bool(BIT(data, 3));
	// in theory we need a view,
	// in practice this approach doesn't work cause we can't mix views with address banks
//  m_gvram_view.select(m_sr_text_mode == false);

	m_sr_text_rows = data & 4 ? 20 : 25;
	refresh_crtc_params();

	// bit 1: bus request
	// bit 4: VRAM bank select
//  if (data & 0x10)
//      popmessage("VRAM bank select enabled");

	if(data & 1)
		throw emu_fatalerror("PC-6601SR in Mk-2 compatibility mode not yet supported!");
}

void pc6001mk2sr_state::sr_vram_bank_w(u8 data)
{
	m_video_base = &m_ram[(data & 0x0f) * 0x1000];

//  set_videoram_bank(0x70000 + ((data & 0x0f)*0x1000));
}

void pc6001mk2sr_state::sr_system_latch_w(u8 data)
{
	cassette_latch_control((data & 8) == 8);
	m_sys_latch = data;

	m_timer_enable = !(data & 1);
	set_timer_divider();
	//vram_bank_change((m_ex_vram_bank & 0x06) | ((m_sys_latch & 0x06) << 4));

	//printf("%02x B0\n",data);
}

void pc6001mk2sr_state::necsr_ppi8255_w(offs_t offset, u8 data)
{
	if (offset==3)
	{
		ppi_control_hack_w(data);

#if 0
		{
			//printf("%02x\n",data);

			if ((data & 0x0f) == 0x05 && m_cart_rom)
				m_bank1->set_base(m_cart_rom->base() + 0x2000);
			if ((data & 0x0f) == 0x04)
				m_bank1->set_base(m_region_gfx1->base());
		}
#endif
	}

	m_ppi->write(offset,data);
}

u8 pc6001mk2sr_state::hw_rev_r()
{
	// bit 1 is active for pc6601sr (and shows the "PC6601SR World" screen in place of the "PC6001mkIISR World"),
	// causes a direct jump to "video telopper" for pc6001mk2sr
	// bit 0 is related to FDC irq status
	return 0 | 1;
}

u8 pc6601sr_state::hw_rev_r()
{
	return 2 | 1;
}

void pc6001mk2sr_state::crt_mode_w(u8 data)
{
//  m_bgcol_bank = (data & 8) ^ 8;
	m_width80 = !BIT(data, 1);
	refresh_crtc_params();
}

inline void pc6001mk2sr_state::refresh_crtc_params()
{
	/* Apparently bitmap modes changes the screen res to 320 x 200 */
	rectangle visarea = m_screen->visible_area();
	const int y_height = (m_sr_text_mode) ? 240 : 200;
	const int x_width = (m_sr_text_mode) ? (m_width80 ? 640 : 320) : 320;

	visarea.set(0, (x_width) - 1, 0, (y_height) - 1);

	m_screen->configure(m_screen->width(), m_screen->height(), visarea, m_screen->frame_period().attoseconds());
}

void pc6001mk2sr_state::pc6001mk2sr_map(address_map &map)
{
	map.unmap_value_high();
	for (int bank = 0; bank < 8; bank++)
	{
		map(bank << 13, (bank << 13) | 0x1fff).r(m_sr_bank[bank], FUNC(address_map_bank_device::read8));
		map(bank << 13, (bank << 13) | 0x1fff).w(m_sr_bank[bank+8], FUNC(address_map_bank_device::write8));
	}
}

void pc6001mk2sr_state::sr_banked_map(address_map &map)
{
//  map(0x00000, 0x0ffff).view(m_gvram_view);
//  m_gvram_view[0](0x0000, 0xffff).ram();
//  m_gvram_view[1](0x0000, 0x1fff).rw(FUNC(pc6001mk2sr_state::sr_gvram_r), FUNC(pc6001mk2sr_state::sr_gvram_w));
	map(0x00000, 0x0ffff).rw(FUNC(pc6001mk2sr_state::work_ram_r), FUNC(pc6001mk2sr_state::work_ram_w)).share("ram");

	// exram0
	map(0x20000, 0x2ffff).ram();
	// exrom0
	map(0xb4000, 0xb7fff).r(m_cart, FUNC(generic_slot_device::read_rom));
	// exrom1
//  map(0xc0000, 0xcffff).rom();
	// cgrom 1
	map(0xd0000, 0xd3fff).rom().region("cgrom", 0);
	// sysrom 1 / 2
	map(0xe0000, 0xfffff).rom().region("sr_sysrom", 0);
}

void pc6001mk2sr_state::pc6001mk2sr_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x40, 0x43).lw8(NAME([this] (offs_t offset, u8 data) {
		// CLUT used by text mode for N66SR BASIC (default [0~3]) vs.
		// "PC-6*01 World" (sets [0x0, 0xd, 0xa, 0xc]) screens.
		// Use pc6601sr for both.
		const u8 clut_entry = bitswap<4>(0xf - offset, 3, 0, 2, 1);
		const u8 color_entry = bitswap<4>(0xf - (data & 0xf), 3, 0, 2, 1);
		m_sr_clut[clut_entry] = color_entry;
	}));
	map(0x60, 0x6f).rw(FUNC(pc6001mk2sr_state::sr_bank_reg_r), FUNC(pc6001mk2sr_state::sr_bank_reg_w));

	map(0x80, 0x81).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));

	map(0x90, 0x93).mirror(0x0c).rw(FUNC(pc6001mk2sr_state::nec_ppi8255_r), FUNC(pc6001mk2sr_state::necsr_ppi8255_w));

	map(0xa0, 0xa0).mirror(0x0c).w(m_ym, FUNC(ym2203_device::address_w));
	map(0xa1, 0xa1).mirror(0x0c).w(m_ym, FUNC(ym2203_device::data_w));
	map(0xa2, 0xa2).mirror(0x0c).r(m_ym, FUNC(ym2203_device::data_r));
	map(0xa3, 0xa3).mirror(0x0c).r(m_ym, FUNC(ym2203_device::status_r));

	map(0xb0, 0xb0).w(FUNC(pc6001mk2sr_state::sr_system_latch_w));

	pc6601_fdc_io(map);
	map(0xb2, 0xb2).r(FUNC(pc6001mk2sr_state::hw_rev_r));

	map(0xb8, 0xbf).ram().share("irq_vectors");
	map(0xc0, 0xc0).r(FUNC(pc6001_state::portc0_r));
//  map(0xc0, 0xc0).w(FUNC(pc6001mk2sr_state::mk2_col_bank_w));
	map(0xc1, 0xc1).w(FUNC(pc6001mk2sr_state::crt_mode_w));
//  map(0xc2, 0xc2).w(FUNC(pc6001mk2sr_state::opt_bank_w));

	map(0xc8, 0xc8).w(FUNC(pc6001mk2sr_state::sr_mode_w));
	map(0xc9, 0xc9).w(FUNC(pc6001mk2sr_state::sr_vram_bank_w));
	// TODO: confirm readback
	map(0xca, 0xcb).ram().share("sr_scrollx");
	map(0xcc, 0xcc).ram().share("sr_scrolly");
	map(0xce, 0xce).w(FUNC(pc6001mk2sr_state::sr_bitmap_yoffs_w));
	map(0xcf, 0xcf).w(FUNC(pc6001mk2sr_state::sr_bitmap_xoffs_w));

	map(0xe0, 0xe3).mirror(0x0c).rw("upd7752", FUNC(upd7752_device::read), FUNC(upd7752_device::write));

//  map(0xf0, 0xf0).rw(FUNC(pc6001mk2sr_state::mk2_bank_r0_r), FUNC(pc6001mk2sr_state::mk2_bank_r0_w));
//  map(0xf1, 0xf1).rw(FUNC(pc6001mk2sr_state::mk2_bank_r1_r), FUNC(pc6001mk2sr_state::mk2_bank_r1_w));
//  map(0xf2, 0xf2).rw(FUNC(pc6001mk2sr_state::mk2_bank_w0_r), FUNC(pc6001mk2sr_state::mk2_bank_w0_w));
	map(0xf3, 0xf3).w(FUNC(pc6001mk2sr_state::mk2_0xf3_w));
//  map(0xf4
//  map(0xf5
	map(0xf6, 0xf6).w(FUNC(pc6001mk2sr_state::mk2_timer_adj_w));
	map(0xf7, 0xf7).w(FUNC(pc6001mk2sr_state::mk2_timer_irqv_w));
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
INPUT_PORTS_END

TIMER_CALLBACK_MEMBER(pc6001_state::audio_callback)
{
	// TODO: shouldn't really need the cas switch check, different thread
	if(m_cas_switch == 0 && m_timer_irq_mask == false)
		set_irq_level(TIMER_IRQ);
}

INTERRUPT_GEN_MEMBER(pc6001mk2sr_state::sr_vrtc_irq)
{
	set_irq_level(VRTC_IRQ);
}

u8 pc6001_state::sub_ack()
{
	return m_sub_vector;
}

u8 pc6001_state::joystick_ack()
{
	return 0x16;
}

u8 pc6001_state::timer_ack()
{
	return m_timer_irq_vector;
}

u8 pc6001_state::vrtc_ack()
{
	// not present for PC-6001
	throw emu_fatalerror("Vanilla PC-6001 doesn't have VRTC irq ack");
}

u8 pc6001mk2_state::vrtc_ack()
{
	// TODO: currently unimplemented, find and verify any software that needs this
	return 0x22;
}

u8 pc6001mk2sr_state::vrtc_ack()
{
	// TODO: bit 0 of sr_mode_w
//  if (sr_mode == false)
//      return pc6001mk2_state::vrtc_ack();

	return m_sr_irq_vectors[VRTC_IRQ];
}

IRQ_CALLBACK_MEMBER(pc6001_state::irq_callback)
{
	u8 result_vector = 0x00;
	for (int i = 0; i < 8; i++)
	{
		u8 mask = m_irq_pending & (1 << i);
		if (mask)
		{
			// TODO: understand how HW implements daisy chain if at all
			// priority should be right, i.e. lower level gets serviced first
			switch(i)
			{
				case SUB_CPU_IRQ: result_vector = sub_ack(); break;
				case JOYSTICK_IRQ: result_vector = joystick_ack(); break;
				case TIMER_IRQ: result_vector = timer_ack(); break;
				case VRTC_IRQ: result_vector = vrtc_ack(); break;
				default:
					throw emu_fatalerror("Unhandled irq ack %d", i);
			}
			m_irq_pending &= ~mask;
			if (m_irq_pending == 0)
				device.execute().set_input_line(0, CLEAR_LINE);
			LOGIRQ("%s: ack %d, state %02x, vector %02x\n", machine().describe_context(), i, m_irq_pending, result_vector);

			return result_vector;
		}
	}

	// check if this ever happens
	LOGIRQ("%s: spurious IRQ fired!\n", machine().describe_context());
	return 0x06;
}

uint8_t pc6001_state::ppi_porta_r()
{
	return 0;
}

void pc6001_state::ppi_porta_w(uint8_t data)
{
	// sub command
	// [0x06]: trigger a 0x16 irq
	// [0x19/0x39]: Cassette PLAY
	// [0x1a]: Cassette STOP
	// [0x1d/0x3d]: Cassette baud select 600
	// [0x1e/0x3e]: Cassette baud select 1200
	// [0x38]: Cassette RECord

	if (data == 0x06)
	{
		m_kbd->joy_cmd_w(1);
	}
}

uint8_t pc6001_state::ppi_portb_r()
{
	return 0;
}

void pc6001_state::ppi_portb_w(uint8_t data)
{
	m_cent_data_out->write(~data);
}

void pc6001_state::ppi_portc_w(uint8_t data)
{
	//printf("ppi_portc_w %02x\n",data);
	m_centronics->write_strobe(BIT(~data, 0));
}

uint8_t pc6001_state::ppi_portc_r()
{
	return 0x88;
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
				set_subcpu_irq_vector(0x08);
			}
			else
				cas_data_i>>=1;
		#else
			m_cur_keycode = m_cas_data[m_cas_offset++];
			popmessage("%04x %04x", m_cas_offset, m_cas_maxsize);
			if(m_cas_offset > m_cas_maxsize)
			{
				m_cas_offset = 0;
				m_cas_switch = 0;
				// Tape-E
				set_subcpu_irq_vector(0x12);
			}
			else
			{
				// Tape-D
				set_subcpu_irq_vector(0x08);
			}
		#endif
	}
}

void pc6001_state::machine_start()
{
	m_timer_irq_timer = timer_alloc(FUNC(pc6001_state::audio_callback), this);

	save_item(NAME(m_cas_data));
	save_item(NAME(m_cas_offset));
	save_item(NAME(m_cas_maxsize));
}

inline void pc6001_state::set_videoram_bank(uint32_t offs)
{
	m_video_base = m_region_maincpu->base() + offs;
}

void pc6001_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

inline void pc6001_state::default_cartridge_reset()
{
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
}

SNAPSHOT_LOAD_MEMBER(pc6001_state::snapshot_cb)
{
	/* get file size */
	uint64_t snapsize = image.length();

	// biggest file seen in the wild is 0x1478d
	if (snapsize > 0x18000)
		return std::make_pair(image_error::INVALIDLENGTH, std::string());

	auto data = std::make_unique<uint8_t []>(snapsize);
	if (image.fread(data.get(), snapsize) != snapsize)
		return std::make_pair(image_error::UNSPECIFIED, "Internal error loading snapshot");

	memcpy(m_cas_data, &data[0], snapsize);

	m_cas_switch = 0;
	m_cas_offset = 0;
	m_cas_maxsize = snapsize;

	return std::make_pair(std::error_condition(), std::string());
}

void pc6001_state::irq_reset(u8 timer_default_setting)
{
	m_timer_enable = false;
	m_timer_irq_mask = false;
	// timer irq vector is fixed in plain PC-6001
	m_timer_irq_vector = 0x06;
	m_irq_pending = 0;
	m_timer_hz_div = timer_default_setting;
	set_timer_divider();
}

void pc6001_state::machine_reset()
{
	set_videoram_bank(0xc000);

	default_cartridge_reset();
	//default_cassette_hack_reset();
	m_cas_offset = 0;
	irq_reset(3);
	m_port_c_8255 = 0;
}

void pc6001mk2_state::machine_reset()
{
//  pc6001_state::machine_reset();
	set_videoram_bank(0xc000 + 0x28000);

	default_cartridge_reset();
	// TODO: hackish way to simplify bankswitch handling
	if (m_cart->exists())
		memcpy(m_region_maincpu->base() + 0x48000, m_cart_rom->base(), 0x4000);

	//default_cassette_hack_reset();
	m_cas_offset = 0;
	irq_reset(3);
	m_port_c_8255 = 0;

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

		m_bgcol_bank = 0;
	}

	m_timer_irq_mask = false;
//  refresh_crtc_params();
}

void pc6601_state::machine_start()
{
	pc6001mk2_state::machine_start();

	m_fdc->set_rate(250000);
	m_floppy->get_device()->set_rpm(300);
}

void pc6001mk2sr_state::machine_reset()
{
//  pc6001_state::machine_reset();

//  set_videoram_bank(0x70000);
	m_video_base = &m_ram[0];

	default_cartridge_reset();
	// TODO: checkout where cart actually maps in SR model
	// should be mirrored into the EXROM regions?
	// hard to tell without an actual SR cart dump
//  std::string region_tag;
//  m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	//default_cassette_hack_reset();
	irq_reset(0x7f);
	m_port_c_8255 = 0;
	m_cas_offset = 0;

	// default to text mode
	m_sr_text_mode = true;
	m_sr_text_rows = 20;
	m_width80 = 0;

	/* set default bankswitch */
	{
		// TODO: confirm this arrangement
		u8 default_banks[16] = {
			// read default:
			// 0x0000 - 0x3fff sysrom 1 0x8000 - 0xbfff
			// 0x4000 - 0x5fff exrom 1 0x0000 - 0x1fff
			// 0x6000 - 0x7fff exrom 0 0x0000 - 0x1fff
			// 0x8000 - 0xffff RAM 0x8000 - 0xffff
			0xf8, 0xfa, 0xc0, 0xb0, 0x08, 0x0a, 0x0c, 0x0e,
			// enable all work RAM writes
			0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e
		};

		for (int i = 0; i < 16; i++)
		{
			m_sr_bank_reg[i] = default_banks[i] & 0xfe;
			m_sr_bank[i]->set_bank(m_sr_bank_reg[i] >> 1);
		}
//      m_gfx_bank_on = 0;
	}

	// jewels and satan (at least) definitely expects timer irq implicitly enabled
	m_timer_irq_mask = false;
}

// debug only, not for drawing

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
	GFXDECODE_ENTRY( "gfx1", 0x0000, char_layout,  0x10, 1 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, kanji_layout, 0x10, 1 )
GFXDECODE_END

// TODO: same as PC-88 / PC-98 31'948'800 ?
#define PC6001_MAIN_CLOCK 7987200

void pc6001_state::pc6001(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, PC6001_MAIN_CLOCK / 2); // PD 780C-1, ~4 Mhz
	m_maincpu->set_addrmap(AS_PROGRAM, &pc6001_state::pc6001_map);
	m_maincpu->set_addrmap(AS_IO, &pc6001_state::pc6001_io);
//  m_maincpu->set_vblank_int("screen", FUNC(pc6001_state::vrtc_irq));
	m_maincpu->set_irq_acknowledge_callback(FUNC(pc6001_state::irq_callback));

//  I8049(config, "subcpu", 7987200);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_pc6001m2);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_screen_update(FUNC(pc6001_state::screen_update));
	// FIXME: actual parameters, particularly for later iterations
	m_screen->set_size(320, 25+192+26);
	m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(pc6001_state::pc6001_palette), 16+4);

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(pc6001_state::ppi_porta_r));
	m_ppi->out_pa_callback().set(FUNC(pc6001_state::ppi_porta_w));
	m_ppi->in_pb_callback().set(FUNC(pc6001_state::ppi_portb_r));
	m_ppi->out_pb_callback().set(FUNC(pc6001_state::ppi_portb_w));
	m_ppi->in_pc_callback().set(FUNC(pc6001_state::ppi_portc_r));
	m_ppi->out_pc_callback().set(FUNC(pc6001_state::ppi_portc_w));

	ADDRESS_MAP_BANK(config, m_cart_bank).set_map(&pc6001_state::cart_map).set_options(ENDIANNESS_LITTLE, 8, 13 + 2, 0x4000);

	/* uart */
	I8251(config, "uart");

	MSX_GENERAL_PURPOSE_PORT(config, m_joy[0], msx_general_purpose_port_devices, "joystick");
	MSX_GENERAL_PURPOSE_PORT(config, m_joy[1], msx_general_purpose_port_devices, "joystick");

	LS157_X2(config, m_joymux);
	m_joymux->a_in_callback().set(m_joy[1], FUNC(msx_general_purpose_port_device::read));
	m_joymux->b_in_callback().set(m_joy[0], FUNC(msx_general_purpose_port_device::read));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "pc6001_cart");

//  CASSETTE(config, m_cassette);
//  m_cassette->set_formats(pc6001_cassette_formats);
//  m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
//  m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	// cas and p6 are raw binary files with no tape markers
	snapshot_image_device &snapshot(SNAPSHOT(config, "snapshot", "cas,p6"));
	snapshot.set_interface("pc6001_snap");
	snapshot.set_load_callback(FUNC(pc6001_state::snapshot_cb));

	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay, PC6001_MAIN_CLOCK/4);
	m_ay->port_a_read_callback().set(FUNC(pc6001_state::joystick_r));
	m_ay->port_b_read_callback().set(FUNC(pc6001_state::joystick_out_r));
	m_ay->port_b_write_callback().set(FUNC(pc6001_state::joystick_out_w));
	m_ay->add_route(ALL_OUTPUTS, "mono", 1.00);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(pc6001_state::write_centronics_busy));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	PC6001_KBD(config, m_kbd);
	m_kbd->key_irq_cb().set([this] (int state) {
		if (state && !m_cas_switch)
		{
			m_cur_keycode = m_kbd->read_key_press();
			set_subcpu_irq_vector(0x02);
		}
	});
	m_kbd->keyfn_irq_cb().set([this] (int state) {
		if (state)
		{
			const u8 key_press = m_kbd->read_key_press();
			// pc6001 specifically wants STOP at 0xfa
			if (key_press == 0xfa || m_cas_switch == 0)
			{
				m_cur_keycode = key_press;
				m_cas_switch = 0;
				//m_cas_offset = 0;
				set_subcpu_irq_vector(0x14);
			}
		}
	});
	m_kbd->joy_irq_cb().set([this] (int state) {
		if (state && !m_cas_switch)
		{
			m_cur_keycode = m_kbd->read_joy_press();
			set_irq_level(JOYSTICK_IRQ);
		}
	});

	// TODO: move this as a dedicated device, get rid of timer_device
	TIMER(config, "cassette_timer").configure_periodic(FUNC(pc6001_state::cassette_callback), attotime::from_hz(1200/12));

	SOFTWARE_LIST(config, "cart_list").set_original("pc6001_cart");
	SOFTWARE_LIST(config, "cass_list").set_original("pc6001_cass");
}

void pc6001mk2_state::pc6001mk2(machine_config &config)
{
	pc6001(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &pc6001mk2_state::pc6001mk2_map);
	m_maincpu->set_addrmap(AS_IO, &pc6001mk2_state::pc6001mk2_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(pc6001mk2_state::irq_callback));

//  MCFG_MACHINE_RESET_OVERRIDE(pc6001mk2_state,pc6001mk2)

	config.device_remove("cart_bank");

	m_screen->set_screen_update(FUNC(pc6001mk2_state::screen_update));

	m_palette->set_entries(16+16);
	m_palette->set_init(FUNC(pc6001mk2_state::pc6001mk2_palette));

	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_pc6001m2);

	UPD7752(config, "upd7752", PC6001_MAIN_CLOCK / 4).add_route(ALL_OUTPUTS, "mono", 1.00);

	SOFTWARE_LIST(config, "cass_list_mk2").set_original("pc6001mk2_cass");
}

void pc6601_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	// TODO: cannot identify .dsk images anyway
	// (HxC reports them to be MSX based images)
	fr.add(FLOPPY_MSX_FORMAT);
	fr.add(FLOPPY_DSK_FORMAT);
}

static void pc6601_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35ssdd", FLOPPY_35_SSDD);
}

void pc6601_state::pc6601_fdc_config(machine_config &config)
{
	UPD765A(config, m_fdc, 8'000'000, true, true);
	FLOPPY_CONNECTOR(config, m_floppy, pc6601_floppies, "35ssdd", pc6601_state::floppy_formats).enable_sound(true);

	// TODO: slotify external I/F
	// PC-6031 mini disk unit (single 5'25 2D drive)
	PC80S31(config, m_pc80s31, XTAL(4'000'000));
	config.set_perfect_quantum(m_maincpu);
//  config.set_perfect_quantum("pc80s31:fdc_cpu");
}

void pc6601_state::pc6601(machine_config &config)
{
	pc6001mk2(config);

	/* basic machine hardware */
	Z80(config.replace(), m_maincpu, PC6001_MAIN_CLOCK / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc6601_state::pc6001mk2_map);
	m_maincpu->set_addrmap(AS_IO, &pc6601_state::pc6601_io);
//  m_maincpu->set_vblank_int("screen", FUNC(pc6001_state::vrtc_irq));
	m_maincpu->set_irq_acknowledge_callback(FUNC(pc6601_state::irq_callback));

	pc6601_fdc_config(config);

	// TODO: move this option to both regular mk2 and mk2sr
	SOFTWARE_LIST(config, "flop_list_pc6001mk2").set_original("pc6001mk2_flop");
}

void pc6001mk2sr_state::pc6001mk2sr(machine_config &config)
{
	pc6001mk2(config);

	/* basic machine hardware */
	// PC-6001SR clock is actually slower than older models (better waitstates tho?)
	Z80(config.replace(), m_maincpu, XTAL(3'579'545));
	m_maincpu->set_addrmap(AS_PROGRAM, &pc6001mk2sr_state::pc6001mk2sr_map);
	m_maincpu->set_addrmap(AS_IO, &pc6001mk2sr_state::pc6001mk2sr_io);
	m_maincpu->set_vblank_int("screen", FUNC(pc6001mk2sr_state::sr_vrtc_irq));
	m_maincpu->set_irq_acknowledge_callback(FUNC(pc6001mk2sr_state::irq_callback));

	for (auto &bank : m_sr_bank)
	{
		ADDRESS_MAP_BANK(config, bank).set_map(&pc6001mk2sr_state::sr_banked_map).set_options(ENDIANNESS_LITTLE, 8, 20, 0x2000);
	}

//  MCFG_MACHINE_RESET_OVERRIDE(pc6001mk2sr_state,pc6001mk2sr)

	m_screen->set_screen_update(FUNC(pc6001mk2sr_state::screen_update));

	pc6601_fdc_config(config);

	config.device_remove("aysnd");
	YM2203(config, m_ym, 4_MHz_XTAL);
	m_ym->port_a_read_callback().set(FUNC(pc6001mk2sr_state::joystick_r));
	m_ym->port_b_read_callback().set(FUNC(pc6001mk2sr_state::joystick_out_r));
	m_ym->port_b_write_callback().set(FUNC(pc6001mk2sr_state::joystick_out_w));
	m_ym->add_route(ALL_OUTPUTS, "mono", 1.00);

	// TODO: telopper board (system explicitly asks for missing tape dump tho)

	SOFTWARE_LIST(config, "cass_list_mk2sr").set_original("pc6001mk2sr_cass");
	SOFTWARE_LIST(config, "flop_list_mk2sr").set_original("pc6001mk2sr_flop");
}

void pc6601sr_state::pc6601sr(machine_config &config)
{
	pc6001mk2sr(config);

	FLOPPY_CONNECTOR(config.replace(), m_floppy, pc6601_floppies, "35dd", pc6601sr_state::floppy_formats);

	// TODO: IR keyboard (does it have functional differences wrt normal PC-6001?)
	// TODO: TV tuner
}

// TODO: all labels needs to be checked up
/* ROM definition */
ROM_START( pc6001 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basicrom.60", 0x0000, 0x4000, CRC(54c03109) SHA1(c622fefda3cdc2b87a270138f24c05828b5c41d2) )

	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "upd8049.ic17", 0x000, 0x800, CRC(6682ec41) SHA1(ea739be6178c0f2ef48a3a33a3f2a3438ed2ca61) BAD_DUMP ) // about 60% of bytes are bad

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "cgrom60.60", 0x0000, 0x1000, CRC(b0142d32) SHA1(9570495b10af5b1785802681be94b0ea216a1e26) )
	ROM_RELOAD(             0x1000, 0x1000 )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_ERASEFF )
ROM_END

ROM_START( pc6001a )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basicrom.60a", 0x0000, 0x4000, CRC(fa8e88d9) SHA1(c82e30050a837e5c8ffec3e0c8e3702447ffd69c) )

	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "upd8049.ic17", 0x000, 0x800, CRC(6682ec41) SHA1(ea739be6178c0f2ef48a3a33a3f2a3438ed2ca61) BAD_DUMP ) // about 60% of bytes are bad

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "cgrom60.60a", 0x0000, 0x1000, CRC(49c21d08) SHA1(9454d6e2066abcbd051bad9a29a5ca27b12ec897) )
	ROM_RELOAD(              0x1000, 0x1000 )

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

	ROM_REGION( 0x800, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i8049", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_COPY( "maincpu", 0x1c000, 0x00000, 0x4000 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "maincpu", 0x20000, 0x00000, 0x8000 )
ROM_END

/* Variant of pc6001mk2 */
ROM_START( pc6601 )
	ROM_REGION( 0x50000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basicrom.66", 0x10000, 0x8000, CRC(c0b01772) SHA1(9240bb6b97fe06f5f07b5d65541c4d2f8758cc2a) )
	ROM_LOAD( "voicerom.66", 0x18000, 0x4000, CRC(91d078c1) SHA1(6a93bd7723ef67f461394530a9feee57c8caf7b7) )
	ROM_LOAD( "cgrom60.66",  0x1c000, 0x2000, CRC(d2434f29) SHA1(a56d76f5cbdbcdb8759abe601eab68f01b0a8fe8) )
	ROM_LOAD( "cgrom66.66",  0x1e000, 0x2000, CRC(3ce48c33) SHA1(f3b6c63e83a17d80dde63c6e4d86adbc26f84f79) )
	ROM_LOAD( "kanjirom.66", 0x20000, 0x8000, CRC(20c8f3eb) SHA1(4c9f30f0a2ebbe70aa8e697f94eac74d8241cadd) )
	// exrom                 0x48000, 0x4000

	ROM_REGION( 0x800, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i8049", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_COPY( "maincpu", 0x1c000, 0x00000, 0x4000 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "maincpu", 0x20000, 0x00000, 0x8000 )
ROM_END

ROM_START( pc6001mk2sr )
	ROM_REGION( 0x20000, "sr_sysrom", ROMREGION_ERASEFF )
	ROM_LOAD( "systemrom1.64", 0x10000, 0x10000, CRC(b6fc2db2) SHA1(dd48b1eee60aa34780f153359f5da7f590f8dff4) )
	ROM_LOAD( "systemrom2.64", 0x00000, 0x10000, CRC(55a62a1d) SHA1(3a19855d290fd4ac04e6066fe4a80ecd81dc8dd7) )

	ROM_REGION( 0x800, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i8049", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x4000, "cgrom", 0 )
	ROM_LOAD( "cgrom68.64", 0x0000, 0x4000, CRC(73bc3256) SHA1(5f80d62a95331dc39b2fb448a380fd10083947eb) )

	ROM_REGION( 0x4000, "gfx1", 0)
	ROM_COPY( "cgrom", 0, 0, 0x4000 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "sr_sysrom", 0x18000, 0x00000, 0x8000 )
ROM_END

ROM_START( pc6601sr )
	ROM_REGION( 0x20000, "sr_sysrom", ROMREGION_ERASEFF )
	// Note: identical to pc6001mk2sr?
	ROM_LOAD( "systemrom1.68", 0x10000, 0x010000, CRC(b6fc2db2) SHA1(dd48b1eee60aa34780f153359f5da7f590f8dff4) )
	ROM_LOAD( "systemrom2.68", 0x00000, 0x010000, CRC(55a62a1d) SHA1(3a19855d290fd4ac04e6066fe4a80ecd81dc8dd7) )

	// mkII compatible ROMs
	ROM_REGION( 0x20000, "mk2", ROMREGION_ERASEFF )
	ROM_LOAD( "basicrom.68",  0x00000, 0x008000, CRC(516b1be3) SHA1(e9977fc13f65f009f03d0340b1f1eb9a3e586739) )
	ROM_LOAD( "voicerom.68",  0x08000, 0x004000, CRC(37ff3829) SHA1(f887e95e29d071df8329168b48c07b78e492c837) )
	ROM_LOAD( "cgrom60.68",   0x0c000, 0x002000, CRC(331473a9) SHA1(361836f9758d6d9b5133c9dc7860a7c74f9cf596) )
	ROM_LOAD( "cgrom66.68",   0x0e000, 0x002000, CRC(03ba2cf1) SHA1(6fb32a4332b26aba2f28c3d8872cac5606be3998) )
	ROM_LOAD( "sysrom2.68",   0x10000, 0x002000, CRC(07318218) SHA1(061f3e7d6c85a560846856feb55fdc0a1f561548) )

	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "d8049hc-016.bin", 0x000, 0x800, CRC(65394e8d) SHA1(761397cbd812623367ef1df5561c6dddb7ebdab7) )

	ROM_REGION( 0x800, "tim", 0 )
	ROM_LOAD( "d8049hc-025.ic201", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "sub", 0 )
	ROM_LOAD( "d8049hc-246.ic47", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x4000, "cgrom", 0 )
	ROM_LOAD( "cgrom68.68",   0x000000, 0x004000, CRC(73bc3256) SHA1(5f80d62a95331dc39b2fb448a380fd10083947eb) )

	ROM_REGION( 0x4000, "gfx1", 0)
	ROM_COPY( "cgrom", 0, 0, 0x4000 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "sr_sysrom", 0x18000, 0x00000, 0x8000 )
ROM_END

COMP( 1981, pc6001,       0,           0,        pc6001,      pc6001, pc6001_state,       empty_init, "NEC",   "PC-6001 (Japan)",              MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING )
COMP( 1981, pc6001a,      pc6001,      0,        pc6001,      pc6001, pc6001_state,       empty_init, "NEC",   "PC-6001A \"NEC Trek\" (US)",   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING )
COMP( 1983, pc6001mk2,    0,           0,        pc6001mk2,   pc6001, pc6001mk2_state,    empty_init, "NEC",   "PC-6001mkII (Japan)",          MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING )
COMP( 1983, pc6601,       pc6001mk2,   0,        pc6601,      pc6001, pc6601_state,       empty_init, "NEC",   "PC-6601 (Japan)",              MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING )
COMP( 1984, pc6001mk2sr,  0,           0,        pc6001mk2sr, pc6001, pc6001mk2sr_state,  empty_init, "NEC",   "PC-6001mkIISR (Japan)",        MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING )
COMP( 1984, pc6601sr,     pc6001mk2sr, 0,        pc6601sr,    pc6001, pc6601sr_state,     empty_init, "NEC",   "PC-6601SR \"Mr. PC\" (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING )
