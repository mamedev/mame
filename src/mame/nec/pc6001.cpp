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
- specific behaviour for CRTKILL to video;
- Use the actual M5C6847P device for vanilla pc6001;
- Pinpoint what VDG supersets PC-6001mkII and SR models really use;
- irq system needs improving, particularly for later machines (where it may really warrant exposing
  as device);

TODO (pc6001mk2):
- confirm optional FDC use mapped at 0xd0-0xd3
\- PC-6031? It looks like a 5'25 single drive with 8255 protocol, presumably earlier revision
   of PC-80S31 with no dump available;
- upd7752 voice speech device needs to be properly emulated (device is currently a skeleton),
  pc6001mk2_cass:chrith is a good test case, it's supposed to talk before title screen;

TODO (pc6601):
- current regression caused by an internal FDC sense interrupt status that expects a
  DIO high that never occurs;
- mon r-0 type games doesn't seem to work at all on this system?
  Update: tries to autoload cassette at startup for some reason.

TODO (pc6601mk2sr):
- check if there are more registers for mkII compatibility mode that are actually substituted
  or unavailable (SR mode probably locked out for instance);
- Video Telopper (superimposer) & TV tuner functions for later machines;

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
irq vector 0x04:                                                                            ;RS-232C irq
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

#define LOG_IRQ     (1U << 1)
#define LOG_SUB_CMD (1U << 2)

#define VERBOSE (LOG_GENERAL | LOG_SUB_CMD)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

#define LOGIRQ(...)      LOGMASKED(LOG_IRQ, __VA_ARGS__)
#define LOGSUB_CMD(...)  LOGMASKED(LOG_SUB_CMD, __VA_ARGS__)

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

inline u8 pc6001_state::get_timer_base_divider()
{
	return 4;
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

void pc6001_state::system_latch_w(uint8_t data)
{
	// NOTE: swapped in memory map
	// static const uint16_t startaddr[] = { 0xC000, 0xE000, 0x8000, 0xA000 };

	// make sure anything doesn't try mapping out of bounds
	// (that would either cause havoc in VDG or system just do this anyway)
	const u32 ram_mask = ((m_ram->size() == 32 * 1024) << 1) | 0x01;

	m_video_base = &m_ram->pointer()[((data >> 1) & ram_mask) << 13];

	cassette_motor_control((data & 8) == 8);
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
	map(0x8000, 0xbfff).lrw8(
		NAME([this] (offs_t offset) -> u8 {
			if (m_ram->size() == 32 * 1024)
				return m_ram->pointer()[offset | 0x4000];
			return 0xff;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (m_ram->size() == 32 * 1024)
				m_ram->pointer()[offset | 0x4000] = data;
		})
	);
	map(0xc000, 0xffff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));
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

// TODO: use a memory_view like later iterations, eventually can mount a RAM cart.
void pc6001_state::cart_map(address_map &map)
{
	map(0x2000, 0x3fff).rom().region("gfx1", 0);
	map(0x4000, 0x7fff).r(m_cart, FUNC(generic_slot_device::read_rom));
}

/*****************************************
 *
 * PC-6001mkII specific I/O
 *
 ****************************************/

// bank window 0x0000 - 0x3fff uses TV and Kanji ROMs on relevant pages
// TODO: verify the use of TVROM(2)
// is it really overflowing in kanji ROM space or it was just a legacy mistake?
template <unsigned TV_BASE> u8 pc6001mk2_state::tv_kanji_r(offs_t offset)
{
	if (m_bank_opt == 0 || m_bank_opt == 2)
	{
		return m_tv_rom->base()[(offset | TV_BASE) & 0x7fff];
	}
	const u32 kanji_bank_base = BIT(m_bank_opt, 1) * 0x4000;
	// on all pages but 0x2 shift down the tv base for our kanji calc
	// this is again legacy cruft, unconfirmed on HW
	const u32 kanji_offset_base = (TV_BASE >> 1) & 0x2000;

	return m_kanji_rom->base()[((offset + kanji_offset_base) & 0x3fff) | kanji_bank_base];
}

// all other banks uses Voice and Kanji ROMs
template <unsigned VOICE_BASE> u8 pc6001mk2_state::voice_kanji_r(offs_t offset)
{
	if (m_bank_opt == 0 || m_bank_opt == 2)
		return m_voice_rom->base()[(offset | VOICE_BASE) & 0x3fff];
	const u32 kanji_bank_base = BIT(m_bank_opt, 1) * 0x4000;

	return m_kanji_rom->base()[((offset | VOICE_BASE) & 0x3fff) | kanji_bank_base];
}

// TODO: how writing works on mk2 mode?
template <unsigned CART_BASE> u8 pc6001mk2_state::cart_mk2_r(offs_t offset)
{
	return m_cart->read_rom(offset + CART_BASE);
}

// TODO: it was marking stuff with TVROM(2), which is outside the size of the actual ROM?
void pc6001mk2_state::mk2_tv_map(address_map &map)
{
	map.unmap_value_high();
	// 00: <invalid setting>
	map(0x00000, 0x03fff).unmapr();
	// 01: basic ROM
	map(0x04000, 0x07fff).rom().region("basic_rom", 0);
	// 02: TV or kanji ROM
	map(0x08000, 0x0bfff).r(FUNC(pc6001mk2_state::tv_kanji_r<0x0000>));
	// 03: ex ROM 1
	map(0x0c000, 0x0dfff).mirror(0x2000).r(FUNC(pc6001mk2_state::cart_mk2_r<0x2000>));
	// 04: ex ROM 0
	map(0x10000, 0x11fff).mirror(0x2000).r(FUNC(pc6001mk2_state::cart_mk2_r<0x0000>));
	// 05: TV ROM 1 & basic ROM 1
	map(0x14000, 0x15fff).r(FUNC(pc6001mk2_state::tv_kanji_r<0x2000>));
	map(0x16000, 0x17fff).rom().region("basic_rom", 0x2000);
	// 06: basic ROM 0 & TV ROM 2 (?)
	map(0x18000, 0x19fff).rom().region("basic_rom", 0);
	map(0x1a000, 0x1bfff).r(FUNC(pc6001mk2_state::tv_kanji_r<0x4000>));
	// 07: ex ROM 0 & 1
	map(0x1c000, 0x1ffff).r(FUNC(pc6001mk2_state::cart_mk2_r<0x0000>));
	// 08: ex ROM 1 & 0
	map(0x20000, 0x21fff).r(FUNC(pc6001mk2_state::cart_mk2_r<0x2000>));
	map(0x22000, 0x23fff).r(FUNC(pc6001mk2_state::cart_mk2_r<0x0000>));
	// 09: ex ROM 1 & basic ROM 1
	map(0x24000, 0x25fff).r(FUNC(pc6001mk2_state::cart_mk2_r<0x2000>));
	map(0x26000, 0x27fff).rom().region("basic_rom", 0x2000);
	// 0a: basic rom 0 & ex rom 1
	map(0x28000, 0x29fff).rom().region("basic_rom", 0);
	map(0x2a000, 0x2bfff).r(FUNC(pc6001mk2_state::cart_mk2_r<0x2000>));
	// 0b: ex ROM 0 & TV ROM 2 (?)
	map(0x2c000, 0x2dfff).r(FUNC(pc6001mk2_state::cart_mk2_r<0x0000>));
	map(0x2e000, 0x2ffff).r(FUNC(pc6001mk2_state::tv_kanji_r<0x4000>));
	// 0c: TV ROM 1 & ex ROM 0
	map(0x30000, 0x31fff).r(FUNC(pc6001mk2_state::tv_kanji_r<0x2000>));
	map(0x32000, 0x33fff).r(FUNC(pc6001mk2_state::cart_mk2_r<0x0000>));
	// 0d: RAM 0 & 1
	map(0x34000, 0x37fff).lr8(NAME([this] (offs_t offset) { return m_ram->pointer()[offset]; }));
	// 0e: EXRAM 0 & 1
	map(0x38000, 0x3bfff).lr8(NAME([this] (offs_t offset) { return m_mk2_exram[offset]; }));
	// 0f: <invalid setting>
	map(0x3c000, 0x3ffff).unmapr();
}

template <unsigned BASIC_BASE, unsigned WORK_BASE> void pc6001mk2_state::mk2_voice_map(address_map &map)
{
	map.unmap_value_high();
	// 00: <invalid setting>
	map(0x00000, 0x03fff).unmapr();
	// 01: basic ROM
	map(0x04000, 0x07fff).rom().region("basic_rom", BASIC_BASE);
	// 02: voice or kanji ROM
	map(0x08000, 0x0bfff).r(FUNC(pc6001mk2_state::voice_kanji_r<0>));
	// 03: ex ROM 1
	map(0x0c000, 0x0dfff).mirror(0x2000).r(FUNC(pc6001mk2_state::cart_mk2_r<0x2000>));
	// 04: ex ROM 0
	map(0x10000, 0x11fff).mirror(0x2000).r(FUNC(pc6001mk2_state::cart_mk2_r<0x0000>));
	// 05: voice ROM 0 & basic ROM 3
	map(0x14000, 0x15fff).r(FUNC(pc6001mk2_state::voice_kanji_r<0>));
	map(0x16000, 0x17fff).rom().region("basic_rom", BASIC_BASE + 0x2000);
	// 06: basic ROM 2 & voice ROM 1
	map(0x18000, 0x19fff).rom().region("basic_rom", BASIC_BASE);
	map(0x1a000, 0x1bfff).r(FUNC(pc6001mk2_state::voice_kanji_r<0x2000>));
	// 07: ex ROM 0 & 1
	map(0x1c000, 0x1ffff).r(FUNC(pc6001mk2_state::cart_mk2_r<0x0000>));
	// 08: ex ROM 1 & 0
	map(0x20000, 0x21fff).r(FUNC(pc6001mk2_state::cart_mk2_r<0x2000>));
	map(0x22000, 0x23fff).r(FUNC(pc6001mk2_state::cart_mk2_r<0x0000>));
	// 09: ex ROM 1 & basic ROM 3
	map(0x24000, 0x25fff).r(FUNC(pc6001mk2_state::cart_mk2_r<0x2000>));
	map(0x26000, 0x27fff).rom().region("basic_rom", BASIC_BASE + 0x2000);
	// 0a: basic rom 2 & ex rom 1
	map(0x28000, 0x29fff).rom().region("basic_rom", BASIC_BASE);
	map(0x2a000, 0x2bfff).r(FUNC(pc6001mk2_state::cart_mk2_r<0x2000>));
	// 0b: ex ROM 0 & voice ROM 1
	map(0x2c000, 0x2dfff).r(FUNC(pc6001mk2_state::cart_mk2_r<0x0000>));
	map(0x2e000, 0x2ffff).r(FUNC(pc6001mk2_state::voice_kanji_r<0x2000>));
	// 0c: voice ROM 0 & ex ROM 0
	map(0x30000, 0x31fff).r(FUNC(pc6001mk2_state::voice_kanji_r<0>));
	map(0x32000, 0x33fff).r(FUNC(pc6001mk2_state::cart_mk2_r<0x0000>));
	// 0d: RAM 6 & 7
	map(0x34000, 0x37fff).lr8(NAME([this] (offs_t offset) { return m_ram->pointer()[offset | WORK_BASE]; }));
	// 0e: EXRAM 6 & 7
	map(0x38000, 0x3bfff).lr8(NAME([this] (offs_t offset) { return m_mk2_exram[offset | WORK_BASE]; }));
	// 0f: <invalid setting>
	map(0x3c000, 0x3ffff).unmapr();
}

void pc6001mk2_state::mk2_bank_r0_w(uint8_t data)
{
	m_bank_r0 = data;

	m_mk2_bank[0]->set_bank(m_bank_r0 & 0xf);
	m_mk2_bank[1]->set_bank(m_bank_r0 >> 4);
}

void pc6001mk2_state::mk2_bank_r1_w(uint8_t data)
{
	m_bank_r1 = data;

	m_mk2_bank[2]->set_bank(m_bank_r1 & 0xf);
	m_mk2_bank[3]->set_bank(m_bank_r1 >> 4);
}

void pc6001mk2_state::mk2_bank_w0_w(uint8_t data)
{
	m_bank_w = data;
}

/*
 * $c2 Option bank
 *
 * chrith uses this on title screen
 *
 * ---- --11 Kanji ROM bank 1
 * ---- --01 Kanji ROM bank 0
 * ---- --x0 TVROM / VOICE ROM
 */
void pc6001mk2_state::mk2_opt_bank_w(uint8_t data)
{
	m_bank_opt = data & 3;
}

void pc6001mk2_state::mk2_work_ram0_w(offs_t offset, uint8_t data)
{
	if (BIT(m_bank_w, 0))
		m_ram->pointer()[offset] = data;
	else
		m_mk2_exram[offset] = data;
}

void pc6001mk2_state::mk2_work_ram1_w(offs_t offset, uint8_t data)
{
	if (BIT(m_bank_w, 2))
		m_ram->pointer()[offset | 0x4000] = data;
	else
		m_mk2_exram[offset | 0x4000] = data;
}

void pc6001mk2_state::mk2_work_ram2_w(offs_t offset, uint8_t data)
{
	if (BIT(m_bank_w, 4))
		m_ram->pointer()[offset | 0x8000] = data;
	else
		m_mk2_exram[offset | 0x8000] = data;
}

void pc6001mk2_state::mk2_work_ram3_w(offs_t offset, uint8_t data)
{
	if (BIT(m_bank_w, 6))
		m_ram->pointer()[offset | 0xc000] = data;
	else
		m_mk2_exram[offset | 0xc000] = data;
}


void pc6001mk2_state::necmk2_ppi8255_w(offs_t offset, uint8_t data)
{
	if (offset==3)
	{
		ppi_control_hack_w(data);

		if ((data & 0x0f) == 0x05)
			m_gfx_view.disable();
		if ((data & 0x0f) == 0x04)
			m_gfx_view.select(0);
	}

	m_ppi->write(offset,data);
}

void pc6001mk2_state::vram_bank_change(uint8_t vram_bank)
{
	uint32_t bank_base_values[8] = { 0x8000, 0xc000, 0xc000, 0xe000, 0x0000, 0x8000, 0x4000, 0xa000 };
	uint8_t vram_bank_index = ((vram_bank & 0x60) >> 4) | ((vram_bank & 2) >> 1);
//  uint8_t *work_ram = m_region_maincpu->base();

//  bit 2 of vram_bank sets up 4 color mode
//  set_videoram_bank(0x28000 + bank_base_values[vram_bank_index]);
	m_video_base = &m_ram->pointer()[bank_base_values[vram_bank_index]];

//  popmessage("%02x",vram_bank);
}

void pc6001mk2_state::mk2_system_latch_w(uint8_t data)
{
	cassette_motor_control((data & 8) == 8);
	m_sys_latch = data;

	m_timer_enable = !(data & 1);
	set_timer_divider();

	vram_bank_change((m_ex_vram_bank & 0x06) | ((m_sys_latch & 0x06) << 4));
}

inline void pc6001mk2_state::refresh_crtc_params()
{
	rectangle visarea = m_screen->visible_area();
	const int y_height = (m_exgfx_bitmap_mode || m_exgfx_2bpp_mode || m_exgfx_text_mode) ? 200 : 240;

	visarea.set(0, (320) - 1, 0, (y_height) - 1);

	const int htotal = 456;
	const int vtotal = 262;
	const XTAL pclock = XTAL(28'636'363) / 4;

	m_screen->configure(htotal, vtotal, visarea, attotime::from_ticks(htotal * vtotal, pclock).as_attoseconds());

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
	m_bgcol_bank = data & 7;
}

/*
 * x--- ---- M1 wait setting
 * -x-- ---- ROM wait setting
 * --x- ---- RAM wait setting
 * ---x ---- joystick irq vector override (mkII mode only)
 * ---- x--- sub CPU irq vector override (mkII mode only)
 * ---- -x-- timer irq mask
 * ---- --x- joystick irq mask
 * ---- ---x sub CPU irq mask
 */
void pc6001mk2_state::mk2_0xf3_w(uint8_t data)
{
	m_timer_irq_mask = BIT(data, 2);
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
	map(0x0000, 0x3fff).r(m_mk2_bank[0], FUNC(address_map_bank_device::read8)).w(FUNC(pc6001mk2_state::mk2_work_ram0_w));
	map(0x4000, 0x7fff).r(m_mk2_bank[1], FUNC(address_map_bank_device::read8)).w(FUNC(pc6001mk2_state::mk2_work_ram1_w));
	map(0x6000, 0x7fff).view(m_gfx_view);
	m_gfx_view[0](0x6000, 0x7fff).lr8(
		NAME([this] (offs_t offset) { return m_tv_rom->base()[offset | m_cgrom_bank_addr]; })
	);
	map(0x8000, 0xbfff).r(m_mk2_bank[2], FUNC(address_map_bank_device::read8)).w(FUNC(pc6001mk2_state::mk2_work_ram2_w));
	map(0xc000, 0xffff).r(m_mk2_bank[3], FUNC(address_map_bank_device::read8)).w(FUNC(pc6001mk2_state::mk2_work_ram3_w));
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
	map(0xc3, 0xc3).lw8(NAME([this] (u8 data) {
		if (data != 0xff)
			LOG("Port $c3: %02x\n", data);
	}));

	map(0xe0, 0xe3).mirror(0x0c).rw("upd7752", FUNC(upd7752_device::read), FUNC(upd7752_device::write));

	map(0xf0, 0xf0).rw(FUNC(pc6001mk2_state::mk2_bank_r0_r), FUNC(pc6001mk2_state::mk2_bank_r0_w));
	map(0xf1, 0xf1).rw(FUNC(pc6001mk2_state::mk2_bank_r1_r), FUNC(pc6001mk2_state::mk2_bank_r1_w));
	map(0xf2, 0xf2).rw(FUNC(pc6001mk2_state::mk2_bank_w0_r), FUNC(pc6001mk2_state::mk2_bank_w0_w));
	map(0xf3, 0xf3).w(FUNC(pc6001mk2_state::mk2_0xf3_w));
//  map(0xf4, 0xf4) sub CPU irq vector override
//  map(0xf5, 0xf5) joystick irq vector override
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
 * PC-6001mkIISR/PC-6601SR specific I/O
 *
 ****************************************/

inline u8 pc6001mk2sr_state::get_timer_base_divider()
{
	if (m_mk2_mode)
		return pc6001mk2_state::get_timer_base_divider();
	return 0x80;
}

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

	return m_ram->pointer()[offset];
}

void pc6001mk2sr_state::work_ram_w(offs_t offset, u8 data)
{
//  if (m_sr_text_mode == false && (offset & 0xe000) == 0) && m_sr_mode)
	if (m_sr_text_mode == false && (offset & 0xe000) == 0)
	{
		sr_gvram_w(offset, data);
		return;
	}

	m_ram->pointer()[offset] = data;
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

	m_mk2_mode = BIT(data, 0);
	// TODO: clock bumps are assumed really
	if(m_mk2_mode)
	{
		m_mk2_view.select(0);
		m_mk2_io_view.select(0);
		m_maincpu->set_unscaled_clock(XTAL(4'000'000));
	}
	else
	{
		m_mk2_view.disable();
		m_mk2_io_view.disable();
		m_maincpu->set_unscaled_clock(XTAL(3'579'545));
	}
}

void pc6001mk2sr_state::sr_vram_bank_w(u8 data)
{
	m_video_base = &m_ram->pointer()[(data & 0x0f) * 0x1000];

//  set_videoram_bank(0x70000 + ((data & 0x0f)*0x1000));
}

void pc6001mk2sr_state::sr_system_latch_w(u8 data)
{
	cassette_motor_control((data & 8) == 8);
	m_sys_latch = data;

	m_timer_enable = !(data & 1);
	set_timer_divider();
//  vram_bank_change((m_ex_vram_bank & 0x06) | ((m_sys_latch & 0x06) << 4));

	//printf("%02x B0\n",data);
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
	if (m_mk2_mode)
	{
		pc6001mk2_state::refresh_crtc_params();
		return;
	}
	/* Apparently bitmap modes changes the screen res to 320 x 200 */
	rectangle visarea = m_screen->visible_area();
	//const int y_height = (m_sr_text_mode) ? 240 : 200;
	const int y_height = 200;
	const int x_width = (m_sr_text_mode) ? (m_width80 ? 640 : 320) : 320;

	visarea.set(0, (x_width) - 1, 0, (y_height) - 1);

	// TODO: guessed
	const int htotal = 456 * (m_width80 + 1);
	const int vtotal = 262;
	const XTAL pclock = XTAL(28'636'363) / (4 >> m_width80);

	m_screen->configure(htotal, vtotal, visarea, attotime::from_ticks(htotal * vtotal, pclock).as_attoseconds());
}

void pc6001mk2sr_state::pc6001mk2sr_map(address_map &map)
{
	map.unmap_value_high();
	for (int bank = 0; bank < 8; bank++)
	{
		map(bank << 13, (bank << 13) | 0x1fff).r(m_sr_bank[bank], FUNC(address_map_bank_device::read8));
		map(bank << 13, (bank << 13) | 0x1fff).w(m_sr_bank[bank+8], FUNC(address_map_bank_device::write8));
	}
	map(0x0000, 0xffff).view(m_mk2_view);
	m_mk2_view[0](0x0000, 0xffff).m(*this, FUNC(pc6001mk2sr_state::pc6001mk2_map));
}

void pc6001mk2sr_state::sr_banked_map(address_map &map)
{
//  map(0x00000, 0x0ffff).view(m_gvram_view);
//  m_gvram_view[0](0x0000, 0xffff).ram();
//  m_gvram_view[1](0x0000, 0x1fff).rw(FUNC(pc6001mk2sr_state::sr_gvram_r), FUNC(pc6001mk2sr_state::sr_gvram_w));
	map(0x00000, 0x0ffff).rw(FUNC(pc6001mk2sr_state::work_ram_r), FUNC(pc6001mk2sr_state::work_ram_w));

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

	map(0x90, 0x93).mirror(0x0c).rw(FUNC(pc6001mk2sr_state::nec_ppi8255_r), FUNC(pc6001mk2sr_state::necmk2_ppi8255_w));

	map(0xa0, 0xa0).mirror(0x0c).w(m_ym, FUNC(ym2203_device::address_w));
	map(0xa1, 0xa1).mirror(0x0c).w(m_ym, FUNC(ym2203_device::data_w));
	map(0xa2, 0xa2).mirror(0x0c).r(m_ym, FUNC(ym2203_device::data_r));
	map(0xa3, 0xa3).mirror(0x0c).r(m_ym, FUNC(ym2203_device::status_r));

	map(0xb0, 0xb0).w(FUNC(pc6001mk2sr_state::sr_system_latch_w));

	pc6601_fdc_io(map);
	map(0xb2, 0xb2).r(FUNC(pc6001mk2sr_state::hw_rev_r));

	map(0xb8, 0xbf).ram().share("irq_vectors");
	map(0xc0, 0xc0).r(FUNC(pc6001_state::portc0_r));
	map(0xc0, 0xc0).w(FUNC(pc6001mk2sr_state::mk2_col_bank_w));
	map(0xc1, 0xc1).w(FUNC(pc6001mk2sr_state::crt_mode_w));
	map(0xc2, 0xc2).w(FUNC(pc6001mk2sr_state::mk2_opt_bank_w));

	map(0xc8, 0xc8).w(FUNC(pc6001mk2sr_state::sr_mode_w));
	map(0xc9, 0xc9).w(FUNC(pc6001mk2sr_state::sr_vram_bank_w));
	// TODO: confirm readback
	map(0xca, 0xcb).ram().share("sr_scrollx");
	map(0xcc, 0xcc).ram().share("sr_scrolly");
	map(0xce, 0xce).w(FUNC(pc6001mk2sr_state::sr_bitmap_yoffs_w));
	map(0xcf, 0xcf).w(FUNC(pc6001mk2sr_state::sr_bitmap_xoffs_w));

	map(0xe0, 0xe3).mirror(0x0c).rw("upd7752", FUNC(upd7752_device::read), FUNC(upd7752_device::write));

	map(0xf0, 0xf0).rw(FUNC(pc6001mk2sr_state::mk2_bank_r0_r), FUNC(pc6001mk2sr_state::mk2_bank_r0_w));
	map(0xf1, 0xf1).rw(FUNC(pc6001mk2sr_state::mk2_bank_r1_r), FUNC(pc6001mk2sr_state::mk2_bank_r1_w));
	map(0xf2, 0xf2).rw(FUNC(pc6001mk2sr_state::mk2_bank_w0_r), FUNC(pc6001mk2sr_state::mk2_bank_w0_w));
	map(0xf3, 0xf3).w(FUNC(pc6001mk2sr_state::mk2_0xf3_w));
//  map(0xf4, 0xf4) sub CPU irq vector override
//  map(0xf5, 0xf5) joystick irq vector override
	map(0xf6, 0xf6).w(FUNC(pc6001mk2sr_state::mk2_timer_adj_w));
	map(0xf7, 0xf7).w(FUNC(pc6001mk2sr_state::mk2_timer_irqv_w));

//  map(0xfa, 0xfa) SR Mode irq mask
//  map(0xfb, 0xfb) SR Mode irq vector override

	// TODO: likely more registers are concealed in compatible modes
	map(0x00, 0xff).view(m_mk2_io_view);
	m_mk2_io_view[0](0xb0, 0xb0).w(FUNC(pc6001mk2sr_state::mk2_system_latch_w));
	m_mk2_io_view[0](0xc1, 0xc1).w(FUNC(pc6001mk2sr_state::mk2_vram_bank_w));
}

/* Input ports */
static INPUT_PORTS_START( pc6001 )
	// TODO: is this really a DSW? bit arrangement is also unknown if so.
	PORT_START("MODE4_DSW")
	PORT_DIPNAME( 0x07, 0x00, "Screen 4 GFX colors" )
	PORT_DIPSETTING(    0x00, "Monochrome" )
	PORT_DIPSETTING(    0x01, "Red/Blue" )
	PORT_DIPSETTING(    0x02, "Blue/Red" )
	PORT_DIPSETTING(    0x03, "Pink/Green" )
	PORT_DIPSETTING(    0x04, "Green/Pink" )
	//5-6-7 is presumably invalid
INPUT_PORTS_END

TIMER_CALLBACK_MEMBER(pc6001_state::timer_irq_cb)
{
	// TODO: shouldn't really need the cas switch check, different thread
	if(m_cas_switch == 0 && m_timer_irq_mask == false)
		set_irq_level(TIMER_IRQ);
}

INTERRUPT_GEN_MEMBER(pc6001mk2sr_state::sr_vrtc_irq)
{
	if (!m_mk2_mode)
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
	// TODO: as above
//  if (m_mk2_mode)
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

void pc6001_state::sub_cmd_w(uint8_t data)
{
	switch(data)
	{
		// [0x06]: trigger a 0x16 irq
		case 0x06:
			m_kbd->joy_cmd_w(1);
			break;
		// [0x19/0x39]: Cassette PLAY
		case 0x19:
		case 0x39:
			LOGSUB_CMD("sub_cmd_w: PLAY command (%02x) at %04x\n", data, m_cas_offset);
			m_cas_switch = 1;
			m_cassette_timer->adjust(attotime::from_hz(m_cas_baud_select / 16));
			break;
		// [0x1a]: Cassette STOP
		// TODO: also 0x3a? (suprball)
		case 0x1a:
			LOGSUB_CMD("sub_cmd_w: STOP command (%02x) at %04x\n", data, m_cas_offset);
			m_cas_switch = 0;
			m_cassette_timer->adjust(attotime::never);
			break;
		// [0x1d/0x3d]: Cassette baud select 600
		// [0x1e/0x3e]: Cassette baud select 1200
		case 0x1d:
		case 0x3d:
		case 0x1e:
		case 0x3e:
			m_cas_baud_select = ((data & 0x1f) == 0x1e) ? 1200 : 600;
			LOGSUB_CMD("sub_cmd_w: baud select command %d (%02x)\n", m_cas_baud_select, data);

			if (m_cas_switch && m_cas_motor)
			{
				m_cassette_timer->reset();
				m_cassette_timer->adjust(attotime::from_hz(m_cas_baud_select / 16));
			}
			break;

		// [0x38]: Cassette RECord
		case 0x38:
			LOG("sub_cmd_w: Unhandled SUB REC command (%02x)\n", data);
			break;
		default:
			LOG("sub_cmd_w: Unhandled SUB command %02x\n", data);
			break;
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
	m_centronics->write_strobe(BIT(~data, 0));
	m_crtkill = BIT(~data, 1);
	if (m_crtkill)
		m_maincpu->set_input_line(Z80_INPUT_LINE_BUSREQ, CLEAR_LINE);
}

uint8_t pc6001_state::ppi_portc_r()
{
	return 0x88;
}

// this controls the motor relay for the tape deck (directly, not from sub CPU?)
inline void pc6001_state::cassette_motor_control(bool new_state)
{
	// 0 -> 1 transition: send PLAY tape cmd to i8049
	if((!(m_sys_latch & 8)) && new_state == true) //PLAY tape cmd
	{
		m_cas_motor = true;
		//m_cassette->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
	}
	// 1 -> 0 transition: send STOP tape cmd to i8049
	if((m_sys_latch & 8) && new_state == false) //STOP tape cmd
	{
		m_cas_motor = false;
		//m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
	}
}

// TODO: move this as a dedicated device
// was 1200 / 12 pre-bus request
// - check random failures in pc6001mk2 plazmaln, digdug, nfanfun loaders if modified
// - pc6001mk2:zunou uses a STOP command when loading.
// - pc6601 enables motor at startup, but doesn't want actually loading a tape.
TIMER_CALLBACK_MEMBER(pc6001_state::cassette_data_cb)
{
	if (!m_cas_motor)
	{
		if (m_cas_switch)
			m_cassette_timer->adjust(attotime::from_hz(m_cas_baud_select / 16));
		return;
	}

	if (m_cas_switch == 1)
	{
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
			m_cassette_timer->adjust(attotime::from_hz(m_cas_baud_select / 16));
		}
	}
}

void pc6001_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
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
	m_cas_motor = false;
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

bool pc6001_state::screen_blanked()
{
	return m_crtkill;
}

bool pc6001mk2sr_state::screen_blanked()
{
	// TODO: investigate why SR mode is unhappy with bus request line
	// jewels (at very least) will crash with a UL Error during title screen composition
	// 1. perhaps CRTKILL is relocated;
	// or
	// 2. the lowered CPU clock takes care of this in some way;
	if (m_mk2_mode)
		return pc6001mk2_state::screen_blanked();
	return true;
}

rectangle pc6001_state::get_screen_display_area()
{
	// VDG regular display area is 256x192, on border bus request should be off
	// A bunch of pc6001 will otherwise fail at startup:
	// - mysterh2
	// - portopia (on second load after explaination)
	// - suprball
	return rectangle { 32, 320 - 32, 24, 240 - 24 };
}

rectangle pc6001mk2_state::get_screen_display_area()
{
	// pc6001mk2:digdug intro clearly wants timing itself by 240 vertical
	// assume mk2 bitmap mode vertical cutoff is border overscan, not blanking
	// TODO: cache mode
	if (m_exgfx_text_mode || m_exgfx_2bpp_mode || m_exgfx_bitmap_mode)
		return rectangle { 0, 320, 0, 240 };

	return pc6001_state::get_screen_display_area();
}

rectangle pc6001mk2sr_state::get_screen_display_area()
{
	if (m_mk2_mode)
		return pc6001mk2_state::get_screen_display_area();

	const int scr_width = m_screen->visible_area().width();

	return rectangle { 0, scr_width, 0, 240 };
}

TIMER_CALLBACK_MEMBER(pc6001_state::video_sync_cb)
{
	const rectangle visarea = get_screen_display_area();
	int hpos = m_screen->hpos();
	int vpos = m_screen->vpos();
	int hsync = hpos < visarea.min_x || hpos >= visarea.max_x;
	int vsync = vpos < visarea.min_y || vpos >= visarea.max_y;

//	printf("%d %d %d %d (%d %d)\n", hsync, vsync, hpos, vpos, visarea.min_y, visarea.max_y);

	m_maincpu->set_input_line(Z80_INPUT_LINE_BUSREQ, hsync || vsync || screen_blanked() ? CLEAR_LINE : ASSERT_LINE);

	if (vsync)
	{
		m_video_sync_timer->adjust(m_screen->time_until_pos(visarea.min_y, visarea.min_x));
	}
	else
	{
		if (hsync)
			m_video_sync_timer->adjust(m_screen->time_until_pos(vpos + 1, visarea.min_x));
		else
			m_video_sync_timer->adjust(m_screen->time_until_pos(vpos, visarea.max_x));
	}
}

void pc6001_state::machine_start()
{
	m_timer_irq_timer = timer_alloc(FUNC(pc6001_state::timer_irq_cb), this);
	m_video_sync_timer = timer_alloc(FUNC(pc6001_state::video_sync_cb), this);
	m_cassette_timer = timer_alloc(FUNC(pc6001_state::cassette_data_cb), this);

	save_item(NAME(m_cas_data));
	save_item(NAME(m_cas_offset));
	save_item(NAME(m_cas_maxsize));
	save_item(NAME(m_crtkill));
}

void pc6001_state::machine_reset()
{
	m_video_base = &m_ram->pointer()[0xc000 - 0x8000];

	m_cas_switch = 0;
	m_cas_motor = false;
	m_cas_baud_select = 1200;
	m_cas_offset = 0;
	m_cassette_timer->adjust(attotime::never);
	irq_reset(3);
	m_port_c_8255 = 0;

	m_crtkill = false;
	m_video_sync_timer->adjust(m_screen->time_until_pos(0, 0));
}


void pc6001mk2_state::machine_start()
{
	pc6001_state::machine_start();
	m_mk2_exram.resize(0x10000);

	save_item(NAME(m_bank_r0));
	save_item(NAME(m_bank_r1));
	save_item(NAME(m_bank_w));
	save_item(NAME(m_ex_vram_bank));
	save_item(NAME(m_cgrom_bank_addr));
	save_item(NAME(m_bgcol_bank));
}

void pc6001mk2_state::machine_reset()
{
	pc6001_state::machine_reset();
//  set_videoram_bank(0xc000 + 0x28000);
	m_video_base = &m_ram->pointer()[0xc000];

	/* set default bankswitch */
	{
		mk2_bank_r0_w(0x71);
		mk2_bank_r1_w(0xdd);
		m_bank_opt = 0x02; //tv rom
		m_bank_w = 0x50; //enable write to work ram 4,5,6,7
		m_gfx_view.disable();

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
	pc6001mk2_state::machine_reset();

//  set_videoram_bank(0x70000);
	m_video_base = &m_ram->pointer()[0];

	m_mk2_view.disable();
	m_mk2_io_view.disable();
	m_mk2_mode = false;
	m_maincpu->set_unscaled_clock(XTAL(3'579'545));

	// TODO: checkout where cart actually maps in SR model
	// should be mirrored into the EXROM regions?
	// hard to tell without an actual SR cart dump

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

// TODO: all clocks needs to be verified from HWs
// same as PC-88 / PC-98 31'948'800 ?
// different for mkIISR machines
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

	RAM(config, m_ram).set_default_size("32K").set_extra_options("16K");

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(pc6001_state::ppi_porta_r));
	m_ppi->out_pa_callback().set(FUNC(pc6001_state::sub_cmd_w));
	m_ppi->in_pb_callback().set(FUNC(pc6001_state::ppi_portb_r));
	m_ppi->out_pb_callback().set(FUNC(pc6001_state::ppi_portb_w));
	m_ppi->in_pc_callback().set(FUNC(pc6001_state::ppi_portc_r));
	m_ppi->out_pc_callback().set(FUNC(pc6001_state::ppi_portc_w));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_pc6001m2);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(pc6001_state::screen_update));
	// allegedly NTSC clock, PC8801FH_OSC1 equivalent
	m_screen->set_raw(XTAL(28'636'363) / 4, 456, 0, 320, 262, 0, 240);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(pc6001_state::palette_init), 16 + 4);

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

	RAM(config.replace(), m_ram).set_default_size("64K");

	config.device_remove("cart_bank");

	ADDRESS_MAP_BANK(config, m_mk2_bank[0]).set_map(&pc6001mk2_state::mk2_tv_map).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);
	ADDRESS_MAP_BANK(config, m_mk2_bank[1]).set_map(&pc6001mk2_state::mk2_voice_map<0x4000, 0x4000>).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);
	ADDRESS_MAP_BANK(config, m_mk2_bank[2]).set_map(&pc6001mk2_state::mk2_voice_map<0x0000, 0x8000>).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);
	ADDRESS_MAP_BANK(config, m_mk2_bank[3]).set_map(&pc6001mk2_state::mk2_voice_map<0x4000, 0xc000>).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);

	m_screen->set_screen_update(FUNC(pc6001mk2_state::screen_update));

	m_palette->set_entries(16+16);
	m_palette->set_init(FUNC(pc6001mk2_state::mk2_palette_init));

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

	// TODO: move this option to regular mk2
	// (needs mountable option from pc80s31 device)
	SOFTWARE_LIST(config, "flop_list_mk2").set_original("pc6001mk2_flop");
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
	SOFTWARE_LIST(config, "flop_list_mk2").set_original("pc6001mk2_flop");
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
	ROM_REGION( 0x8000, "basic_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "basicrom.62", 0x0000, 0x8000, CRC(950ac401) SHA1(fbf195ba74a3b0f80b5a756befc96c61c2094182) )

	ROM_REGION( 0x4000, "voice_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "voicerom.62", 0x0000, 0x4000, CRC(49b4f917) SHA1(1a2d18f52ef19dc93da3d65f19d3abbd585628af) )

	ROM_REGION( 0x8000, "kanji_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "kanjirom.62", 0x0000, 0x8000, CRC(20c8f3eb) SHA1(4c9f30f0a2ebbe70aa8e697f94eac74d8241cadd) )

	ROM_REGION( 0x8000, "tv_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "cgrom60.62",          0x0000, 0x2000, CRC(81eb5d95) SHA1(53d8ae9599306ff23bf95208d2f6cc8fed3fc39f) )
	ROM_LOAD( "cgrom60m.62",         0x2000, 0x2000, CRC(3ce48c33) SHA1(f3b6c63e83a17d80dde63c6e4d86adbc26f84f79) )
	ROM_COPY( "kanji_rom",   0x0000, 0x4000, 0x2000 ) // TVROM(2)?

	ROM_REGION( 0x800, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i8049", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_COPY( "tv_rom", 0x0000, 0x0000, 0x4000 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "kanji_rom", 0x0000, 0x0000, 0x8000 )
ROM_END

/* Variant of pc6001mk2 */
ROM_START( pc6601 )
	ROM_REGION( 0x8000, "basic_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "basicrom.66", 0x0000, 0x8000, CRC(c0b01772) SHA1(9240bb6b97fe06f5f07b5d65541c4d2f8758cc2a) )

	ROM_REGION( 0x4000, "voice_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "voicerom.66", 0x0000, 0x4000, CRC(91d078c1) SHA1(6a93bd7723ef67f461394530a9feee57c8caf7b7) )

	ROM_REGION( 0x8000, "kanji_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "kanjirom.66", 0x0000, 0x8000, CRC(20c8f3eb) SHA1(4c9f30f0a2ebbe70aa8e697f94eac74d8241cadd) )

	ROM_REGION( 0x8000, "tv_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "cgrom60.66",         0x0000, 0x2000, CRC(d2434f29) SHA1(a56d76f5cbdbcdb8759abe601eab68f01b0a8fe8) )
	ROM_LOAD( "cgrom66.66",         0x2000, 0x2000, CRC(3ce48c33) SHA1(f3b6c63e83a17d80dde63c6e4d86adbc26f84f79) )
	ROM_COPY( "kanji_rom",  0x0000, 0x4000, 0x2000 )  // TVROM(2)?

	ROM_REGION( 0x800, "mcu", ROMREGION_ERASEFF )
	ROM_LOAD( "i8049", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_COPY( "tv_rom", 0x0000, 0x0000, 0x4000 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "kanji_rom", 0x0000, 0x0000, 0x8000 )
ROM_END

ROM_START( pc6001mk2sr )
	ROM_REGION( 0x20000, "sr_sysrom", ROMREGION_ERASEFF )
	ROM_LOAD( "systemrom1.64", 0x10000, 0x10000, CRC(b6fc2db2) SHA1(dd48b1eee60aa34780f153359f5da7f590f8dff4) )
	ROM_LOAD( "systemrom2.64", 0x00000, 0x10000, CRC(55a62a1d) SHA1(3a19855d290fd4ac04e6066fe4a80ecd81dc8dd7) )

	// TODO: mk2sr in mk2 mode aren't dumped, using pc6601sr for now
	ROM_REGION( 0x8000, "basic_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "basicrom.68",  0x0000, 0x8000, CRC(516b1be3) SHA1(e9977fc13f65f009f03d0340b1f1eb9a3e586739) )

	ROM_REGION( 0x8000, "kanji_rom", ROMREGION_ERASEFF )
	ROM_COPY( "sr_sysrom", 0x18000,  0x0000, 0x8000 )

	ROM_REGION( 0x8000, "tv_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "cgrom60.68",           0x0000, 0x2000, CRC(331473a9) SHA1(361836f9758d6d9b5133c9dc7860a7c74f9cf596) )
	ROM_LOAD( "cgrom66.68",           0x2000, 0x2000, CRC(03ba2cf1) SHA1(6fb32a4332b26aba2f28c3d8872cac5606be3998) )
	ROM_COPY( "kanji_rom",   0x00000, 0x4000, 0x2000 )

	ROM_REGION( 0x4000, "voice_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "voicerom.68",  0x0000, 0x4000, CRC(37ff3829) SHA1(f887e95e29d071df8329168b48c07b78e492c837) )

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

	ROM_REGION( 0x8000, "basic_rom", ROMREGION_ERASEFF )
	ROM_COPY( "mk2", 0x00000, 0, 0x8000 )

	ROM_REGION( 0x4000, "voice_rom", ROMREGION_ERASEFF )
	ROM_COPY( "mk2", 0x08000, 0, 0x4000 )

	ROM_REGION( 0x8000, "kanji_rom", ROMREGION_ERASEFF )
	ROM_COPY( "sr_sysrom", 0x18000, 0x00000, 0x8000 )

	ROM_REGION( 0x8000, "tv_rom", ROMREGION_ERASEFF )
	ROM_COPY( "mk2",       0x0c000, 0x0000, 0x4000 )
	ROM_COPY( "kanji_rom", 0x00000, 0x4000, 0x2000 )

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

COMP( 1981, pc6001,       0,           0,        pc6001,      pc6001, pc6001_state,       empty_init, "NEC",   "PC-6001 (Japan)",              MACHINE_NOT_WORKING )
COMP( 1981, pc6001a,      pc6001,      0,        pc6001,      pc6001, pc6001_state,       empty_init, "NEC",   "PC-6001A \"NEC Trek\" (US)",   MACHINE_NOT_WORKING )

// MACHINE_IMPERFECT_SOUND for lack of upd7752 semantics
COMP( 1983, pc6001mk2,    0,           0,        pc6001mk2,   pc6001, pc6001mk2_state,    empty_init, "NEC",   "PC-6001mkII (Japan)",          MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1983, pc6601,       pc6001mk2,   0,        pc6601,      pc6001, pc6601_state,       empty_init, "NEC",   "PC-6601 (Japan)",              MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
// al-Warka PC-6001, official Iraqi mkII equivalent with Arabic charset (allegedly without voice chip)
// prototype English mkII

COMP( 1984, pc6001mk2sr,  0,           0,        pc6001mk2sr, pc6001, pc6001mk2sr_state,  empty_init, "NEC",   "PC-6001mkIISR (Japan)",        MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1984, pc6601sr,     pc6001mk2sr, 0,        pc6601sr,    pc6001, pc6601sr_state,     empty_init, "NEC",   "PC-6601SR \"Mr. PC\" (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
// al-Warka PC-6002, mkIISR equivalent (allegedly with *both* YM and PSG chips)
