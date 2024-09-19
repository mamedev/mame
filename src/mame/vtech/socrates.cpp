// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/******************************************************************************
*
*  V-tech Socrates-series devices
*  Copyright (C) 2009-2021 Jonathan Gevaryahu AKA Lord Nightmare
*  with dumping help from Kevin 'kevtris' Horton
*
*  The devices in this driver all use a similar ASIC, presumably produced by
*  Toshiba for Vtech/Yeno, in a QFP package with 100 pins (30+20+30+20)
*  The asic variants seen in the wild are:
*  27-0769 TC17G032AF-0248 (Socrates NTSC)
*  27-0883 1732-8277 (Video Painter and Socrates PAL)
*
TODO (socrates):
    * The speech chip is a Toshiba tc8802AF (which is pin and speech
      compatible with the older Toshiba t6803, but adds vsm rom read mode and
      apparently does away with the melody mode); the chip is running at
      800khz clock/10khz output with between 1 and 4 t6684F vsm roms
      attached; create a sound driver for this!
    * hook up mouse
    * add waitstates for ram access (lack of this causes the system to run
      way too fast)
      This will require some probing with the LA and the fluke to figure out
      how many cycles the waitstates are for for rom/ram/etc access.
    * figure out what bit 6 of the status register actually does; is this an
      ir mcu busy flag?
    * keyboard IR decoder MCU is HLE'd for now, needs decap and cpu core
    * iqunlimz keyboard MCU simulation does not handle key repeats


  Socrates Educational Video System
        FFFF|----------------|
            | RAM (window 1) |
            |                |
        C000|----------------|
            | RAM (window 0) |
            |                |
        8000|----------------|
            | ROM (banked)   |
            | *Cartridge     |
        4000|----------------|
            | ROM (fixed)    |
            |                |
        0000|----------------|

    * cartridge lives in banks 10 onward, see below

        Banked rom area (4000-7fff) bankswitching
        Bankswitching is achieved by writing to I/O port 0 (mirrored on 1-7)
        Bank       ROM_REGION        Contents
        0          0x00000 - 0x03fff System ROM page 0
        1          0x04000 - 0x07fff System ROM page 1
        2          0x08000 - 0x0bfff System ROM page 2
        ... etc ...
        E          0x38000 - 0x38fff System ROM page E
        F          0x3c000 - 0x3ffff System ROM page F
        10         0x40000 - 0x43fff Expansion Cartridge page 0 (cart ROM 0x0000-0x3fff)
        11         0x44000 - 0x47fff Expansion Cartridge page 1 (cart ROM 0x4000-0x7fff)
        ... etc ...

        Banked ram area (z80 0x8000-0xbfff window 0 and z80 0xc000-0xffff window 1)
        Bankswitching is achieved by writing to I/O port 8 (mirrored to 9-F), only low nybble
        byte written: 0b****BBAA
        where BB controls ram window 1 and AA controls ram window 0
        hence:
        Write    [window 0]         [window 1]
        0        0x0000-0x3fff      0x0000-0x3fff
        1        0x4000-0x7fff      0x0000-0x3fff
        2        0x8000-0xbfff      0x0000-0x3fff
        3        0xc000-0xffff      0x0000-0x3fff
        4        0x0000-0x3fff      0x4000-0x7fff
        5        0x4000-0x7fff      0x4000-0x7fff
        6        0x8000-0xbfff      0x4000-0x7fff
        7        0xc000-0xffff      0x4000-0x7fff
        8        0x0000-0x3fff      0x8000-0xbfff
        9        0x4000-0x7fff      0x8000-0xbfff
        A        0x8000-0xbfff      0x8000-0xbfff
        B        0xc000-0xffff      0x8000-0xbfff
        C        0x0000-0x3fff      0xc000-0xffff
        D        0x4000-0x7fff      0xc000-0xffff
        E        0x8000-0xbfff      0xc000-0xffff
        F        0xc000-0xffff      0xc000-0xffff

******************************************************************************/

#include "emu.h"
#include "socrates_a.h"

#include "cpu/z80/z80.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "machine/bankdev.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class socrates_state : public driver_device
{
public:
	socrates_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sound(*this, "soc_snd"),
		m_screen(*this, "screen"),
		m_cart(*this, "cartslot"),
		m_bios_reg(*this, "maincpu"),
		m_vram_reg(*this, "vram"),
		m_rombank1(*this, "rombank1"),
		m_rombank2(*this, "rombank2"),
		m_rambank1(*this, "rambank1"),
		m_rambank2(*this, "rambank2"),
		m_kbdrow(*this, "IN%u", 0)
	{ }

	void socrates(machine_config &config);
	void socrates_pal(machine_config &config);
	void vpainter_pal(machine_config &config);

	void init_socrates();
	void init_iqunlimz();
	void init_vpainter();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void socrates_mem(address_map &map) ATTR_COLD;
	void socrates_io(address_map &map) ATTR_COLD;
	void socrates_rambank_map(address_map &map) ATTR_COLD;
	void socrates_rombank_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<socrates_snd_device> m_sound;
	required_device<screen_device> m_screen;
	optional_device<generic_slot_device> m_cart;
	memory_region *m_cart_reg;
	required_memory_region m_bios_reg;
	required_shared_ptr<u8> m_vram_reg;
	required_device<address_map_bank_device> m_rombank1;
	optional_device<address_map_bank_device> m_rombank2; // iqunlimz only
	required_device<address_map_bank_device> m_rambank1;
	required_device<address_map_bank_device> m_rambank2;
	optional_ioport_array<0xC> m_kbdrow;

	uint8_t m_rom_bank[2];
	uint8_t m_ram_bank;
	uint16_t m_scroll_offset;
	uint16_t m_kb_spi_buffer;
	bool m_kb_spi_request;
	uint8_t m_kbmcu_type; // 0 for socrates, 1 for iqunlimz, 2 for vpainter
	uint16_t m_oldkeyvalue; // previous key pressed
	uint16_t m_keyrepeat_holdoffcounter; // keyrepeat holdoff countdown
	uint8_t m_io40_latch; // what was last written to speech reg (for open bus)?
	uint8_t m_speech_running; // is speech synth talking?
	uint32_t m_speech_address; // address in speech space
	uint8_t m_speech_settings; // speech settings (nybble 0: ? externrom ? ?; nybble 1: ? ? ? ?)
	uint8_t m_speech_dummy_read; // have we done a dummy read yet?
	uint8_t m_speech_load_address_count; // number of times load address has happened
	uint8_t m_speech_load_settings_count; // number of times load settings has happened

	emu_timer *m_kbmcu_sim_timer;
	emu_timer *m_clear_speech_timer;
	emu_timer *m_clear_irq_timer;

	struct
	{
		uint16_t   buffer[8];
		uint8_t    head;
		uint8_t    tail;
		uint8_t    count;
	} m_kb_queue;

	void palette_init(palette_device &palete) const;

	uint8_t common_rom_bank_r(offs_t offset);
	void common_rom_bank_w(offs_t offset, uint8_t data);
	uint8_t common_ram_bank_r();
	void common_ram_bank_w(uint8_t data);
	uint8_t cart_r(offs_t offset);
	uint8_t read_f3();
	void kbmcu_reset(uint8_t data);
	uint8_t status_r();
	void speech_command(uint8_t data);
	uint8_t keyboard_buffer_read(offs_t offset);
	void keyboard_buffer_update(uint8_t data);
	void kbmcu_sim_reset();
	void kbmcu_sim_fifo_enqueue(uint16_t data);
	uint16_t kbmcu_sim_fifo_dequeue();
	uint16_t kbmcu_sim_fifo_peek();
	void kbmcu_sim_fifo_head_clear();
	void reset_speech(uint8_t data);
	void scroll_w(offs_t offset, uint8_t data);
	void sound_w(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(assert_irq);
	TIMER_CALLBACK_MEMBER(kbmcu_sim_cb);
	TIMER_CALLBACK_MEMBER(clear_speech_cb);
	TIMER_CALLBACK_MEMBER(clear_irq_cb);
	static rgb_t create_color(uint8_t color);
};


class iqunlimz_state : public socrates_state
{
public:
	iqunlimz_state(const machine_config &mconfig, device_type type, const char *tag) :
		socrates_state(mconfig, type, tag)
	{ }

	void iqunlimz(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER( send_input );

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void colors_w(offs_t offset, uint8_t data);
	uint8_t video_regs_r(offs_t offset);
	void video_regs_w(offs_t offset, uint8_t data);
	uint8_t status_r();

	void iqunlimz_io(address_map &map) ATTR_COLD;
	void iqunlimz_mem(address_map &map) ATTR_COLD;
	void iqunlimz_rambank_map(address_map &map) ATTR_COLD;
	void iqunlimz_rombank_map(address_map &map) ATTR_COLD;

	int get_color(int index, int y);

	uint8_t   m_colors[8];
	uint8_t   m_video_regs[4];

};

/* Defines */

// number of mcu "full cycles" before key repeat
#define KEYREPEAT_HOLDOFF 0x1f
// number of mcu "full cycles" between key repeats
#define KEYREPEAT_REPEAT 1

/* Components */

/* Devices */

/******************************************************************************

Socrates Keyboard MCU simulation

*******************************************************************************
the tmp50c40 MCU seems to have an 8? word, 12 bit wide internal fifo, which is fed by IR decoded from the
remote keyboard. The socrates reads this data out via SPI? by asserting a /cs? bit when keyboard_buffer_update
is written to.
the bits are returned in two registers, handled by keyboard_buffer_read
bits are read out in the following order:
offset0  offset1
76543210 76543210
HHHHMMMM F000LLLL
where HHHH is the high nybble, MMMM is the middle nybble, LLLL is the low nybble
and F is the fifo status, 1 if the fifo had any data in it at all, and 0 if not
if the fifo is empty, the mcu shifts out nothing, and the spi reg holds 00000000 00000001
******************************************************************************/

/******************************************************************************

IQUnlimited Keyboard MCU simulation

*******************************************************************************
iq unlimited has a different mcu than socrates, which has an 8-word 8-bit wide
fifo instead.
As in socrates, the bits are returned in two registers, handled by keyboard_buffer_read
bits are read out in the following order:
offset0  offset1
76543210 76543210
HHHHLLLL F000KACS
where HHHH is the high nybble, LLLL is the low nybble, KACS is capslock, alt, ctrl and shift,
and F is the fifo status, 1 if the fifo had any data in it at all, and 0 if not.
KACS are NOT from the fifo, but a live status of the key states.
F is also binary ORed with the vblank state???
if the fifo is empty, the mcu shifts out nothing, and the spi reg holds 00000000 F000KACS
******************************************************************************/

void socrates_state::kbmcu_sim_reset()
{
	m_kb_queue.head = m_kb_queue.tail = m_kb_queue.count = 0;
	for (uint8_t i = 0; i < (sizeof(m_kb_queue.buffer)/sizeof(m_kb_queue.buffer[0])); i++)
		m_kb_queue.buffer[i]=0;
}


void socrates_state::kbmcu_sim_fifo_enqueue(uint16_t data)
{
	//logerror("kbmcu_sim_fifo_enqueue called with %02x, fifo count was %d\n", data, m_kb_queue.count);
	if (m_kb_queue.count < 8)
	{
		m_kb_queue.buffer[m_kb_queue.tail] = data;
		m_kb_queue.tail = (m_kb_queue.tail + 1) % (sizeof(m_kb_queue.buffer)/sizeof(m_kb_queue.buffer[0]));
		m_kb_queue.count++;
	}
}

uint16_t socrates_state::kbmcu_sim_fifo_dequeue()
{
	if (m_kb_queue.count == 0) fatalerror("kbmcu_sim_fifo_dequeue called with queue count of zero. This should never happen, contact MAMEDEV!");
	uint16_t retval = m_kb_queue.buffer[m_kb_queue.head];
	m_kb_queue.count--;
	m_kb_queue.head = (m_kb_queue.head + 1) % (sizeof(m_kb_queue.buffer)/sizeof(m_kb_queue.buffer[0]));
	//logerror("kbmcu_sim_fifo_dequeue was called, returning %02x, fifo count is now %d\n", retval, m_kb_queue.count);
	return retval;
}

uint16_t socrates_state::kbmcu_sim_fifo_peek()
{
	if (m_kb_queue.count == 0) fatalerror("kbmcu_sim_fifo_peek called with queue count of zero. This should never happen, contact MAMEDEV!");
	uint16_t retval = m_kb_queue.buffer[m_kb_queue.head];
	//logerror("kbmcu_sim_fifo_peek was called, returning %02x, fifo count is now %d\n", retval, m_kb_queue.count);
	return retval;
}

void socrates_state::kbmcu_sim_fifo_head_clear()
{
	m_kb_queue.buffer[m_kb_queue.head] = 0;
}

void socrates_state::machine_start()
{
	m_kbmcu_sim_timer = timer_alloc(FUNC(socrates_state::kbmcu_sim_cb), this);
	m_kbmcu_sim_timer->adjust(attotime::from_hz((XTAL(21'477'272)/6)/3000)); // timer rate is a massive guess, depends on instructions per loop of mcu
	m_clear_speech_timer = timer_alloc(FUNC(socrates_state::clear_speech_cb), this);
	m_clear_irq_timer = timer_alloc(FUNC(socrates_state::clear_irq_cb), this);
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_scroll_offset));
	save_item(NAME(m_kb_spi_request));
	save_item(NAME(m_kb_spi_buffer));
	save_item(NAME(m_oldkeyvalue));
	save_item(NAME(m_keyrepeat_holdoffcounter));
	save_item(NAME(m_io40_latch));
	save_item(NAME(m_speech_running));
	save_item(NAME(m_speech_address));
	save_item(NAME(m_speech_settings));
	save_item(NAME(m_speech_dummy_read));
	save_item(NAME(m_speech_load_address_count));
	save_item(NAME(m_speech_load_settings_count));
	m_rom_bank[0] = m_rom_bank[1] = 0x0;
	m_rombank1->set_bank(0x0); // actually set semi-randomly on real console but we need to initialize it somewhere...
	m_ram_bank = 0;
	m_rambank1->set_bank(0x0);// the actual console sets it semi randomly on power up, and the bios cleans it up.
	m_rambank2->set_bank(0x0);
	// ASIC SPI buffer starts with garbage in it for one word, simulate this
	m_kb_spi_buffer = 0xFF8F;
}

void socrates_state::machine_reset()
{
	m_cart_reg = m_cart ? memregion(util::string_format("%s%s", m_cart->tag(), GENERIC_ROM_REGION_TAG).c_str()) : nullptr;
	kbmcu_sim_reset();
	m_kb_spi_request = true;
	m_oldkeyvalue = 0;
	m_keyrepeat_holdoffcounter = KEYREPEAT_HOLDOFF;
	m_io40_latch = 0;
	m_speech_running = 0;
	m_speech_address = 0;
	m_speech_settings = 0;
	m_speech_dummy_read = 0;
	m_speech_load_address_count = 0;
	m_speech_load_settings_count = 0;
}

void socrates_state::init_socrates()
{
	/* fill vram with its init powerup bit pattern, so startup has the checkerboard screen */
	for (int i = 0; i < 0x10000; i++)
		m_vram_reg[i] = (((i&0x1)?0x00:0xFF)^((i&0x100)?0x00:0xff));
	m_maincpu->set_clock_scale(0.45f); /// TODO: RAM access waitstates etc. aren't emulated - slow the CPU to compensate
	m_kbmcu_type = 0;
}

void socrates_state::init_iqunlimz()
{
	/* fill vram with its init powerup bit pattern, so startup has the checkerboard screen... is this even right for the iqunlimz? */
	for (int i = 0; i < 0x20000; i++)
		m_vram_reg[i] = (((i&0x1)?0x00:0xFF)^((i&0x100)?0x00:0xff));
	//m_maincpu->set_clock_scale(0.45f); /// TODO: RAM access waitstates etc. aren't emulated - slow the CPU to compensate
	m_kbmcu_type = 1;
}

void socrates_state::init_vpainter()
{
	/* fill vram with its init powerup bit pattern, so startup has the checkerboard screen */
	for (int i = 0; i < 0x10000; i++)
		m_vram_reg[i] = (((i&0x1)?0x00:0xFF)^((i&0x100)?0x00:0xff));
	m_maincpu->set_clock_scale(0.45f); /// TODO: RAM access waitstates etc. aren't emulated - slow the CPU to compensate
	m_kbmcu_type = 2;
}

uint8_t socrates_state::common_rom_bank_r(offs_t offset)
{
	return m_rom_bank[offset];
}

void socrates_state::common_rom_bank_w(offs_t offset, uint8_t data)
{
	m_rom_bank[offset] = data;
	if (offset && m_rombank2)
		m_rombank2->set_bank(data);
	else
		m_rombank1->set_bank(data);
}

uint8_t socrates_state::common_ram_bank_r()
{
	return m_ram_bank;
}

void socrates_state::common_ram_bank_w(uint8_t data)
{
	m_ram_bank = data;
	m_rambank1->set_bank(((data>>2) & 0x0c) | ((data>>0) & 0x03));
	m_rambank2->set_bank(((data>>4) & 0x0c) | ((data>>2) & 0x03));
}

uint8_t socrates_state::cart_r(offs_t offset)
{
	///TODO: do m_rombank->space(AS_PROGRAM).install_write_handler(0x0002, 0x0002, write8_delegate(FUNC(dac_byte_interface::data_w), (dac_byte_interface *)m_dac)); style stuff
	// demangle the offset, offset passed is bits 11111111 11111111 00000000 00000000
	// where . is 0                               EDCBA987 65432.10 FEDCBA98 76543210
	offset = ((offset&0x3FFFF)|((offset&0xF80000)>>1));
	if (m_cart_reg)
	{
		assert(m_cart);
		offset &= m_cart->get_rom_size()-1;
		return *(m_cart_reg->base()+offset);
	}
	else
		return 0xF3;
}

uint8_t socrates_state::read_f3()// used for read-only i/o ports as mame/mess doesn't have a way to set the unmapped area to read as 0xF3
{
	return 0xF3;
}

void socrates_state::kbmcu_reset(uint8_t data) // reset the keyboard MCU, clear its fifo
{
	//logerror("0x%04X: kbmcu written with %02X!\n", m_maincpu->pc(), data); //if (m_maincpu->pc() != 0x31D)
	kbmcu_sim_reset();
}

uint8_t socrates_state::status_r()// read 0x4x, some sort of status reg
{
// bit 7 - speech status: high when speech is playing, low when it is not (or when speech cart is not present)
// bit 6 - unknown, usually set, possibly mcu ready state?
// bit 5 - vblank status, high when not in vblank
// bit 4 - hblank status, high when not in hblank
// bit 3 - speech chip bit 3
// bit 2 - speech chip bit 2
// bit 1 - speech chip bit 1
// bit 0 - speech chip bit 0
	uint8_t *speechromint = memregion("speechint")->base();
	uint8_t *speechromext = memregion("speechext")->base();
	int temp = 0;
	temp |= (m_speech_running)?0x80:0;
	temp |= (1)?0x40:0; // unknown, possibly IR mcu ready?
	temp |= (m_screen->vblank())?0:0x20;
	temp |= (m_screen->hblank())?0:0x10;
	switch(m_io40_latch&0xF0) // what was last opcode sent?
	{
		case 0x60: case 0xE0:// speech status 'read' register
			if(m_speech_settings&0x04) // external speech roms (outside of speech ic but still in cart) enabled
			{
			logerror("reading external speech rom nybble from nybble address %x (byte address %x)\n",m_speech_address, m_speech_address>>1);
			temp |= ((speechromext[((m_speech_address>>1)&0xffff)]>>((m_speech_address&1)*4))&0xF);
			}
			else
			{
			logerror("reading internal speech rom nybble from nybble address %x (byte address %x)\n",m_speech_address, m_speech_address>>1);
			temp |= ((speechromint[((m_speech_address>>1)&0x1fff)]>>((m_speech_address&1)*4))&0xF);
			}
			if (m_speech_dummy_read == 0) // if we havent done the dummy read yet, do so now
			{
				m_speech_dummy_read++;
			}
			else
			{
				m_speech_address++;
			}
			break;
		default:
			temp |= m_io40_latch&0xF; // read open bus
			break;
	}
	logerror("read from i/o 0x4x of %x\n", temp);
	return temp;
}

TIMER_CALLBACK_MEMBER(socrates_state::clear_speech_cb)
{
	m_speech_running = 0;
	m_speech_load_address_count = 0; // should this be here or in the write functuon subpart which is speak command?
	m_speech_load_settings_count = 0;
}

void socrates_state::speech_command(uint8_t data) // write 0x4x
{
	/*
	 * 76543210
	 * |||||||\-- SEL0
	 * ||||||\--- SEL1
	 * |||||\---- SEL2
	 * ||||\----- SEL3
	 * |||\------ SEL4
	 * ||\------- SEL5
	 * |\-------- 64UP (if LOW the chip is in 'dumb' mode and can only play 64 fixed phrases; if HIGH the chip is in 'cpu controlled' mode and commands work) (not 100% sure about this but suspect it is correct)
	 * \--------- START (in cpu mode must be toggled from low->high for a cpu command to 'take'; in 'dumb' mode must be toggled low->high to start that 'word')
	 */
	logerror("write to i/o 0x4x of %x\n", data);
/*
// old readback test, probably get rid of this junk:
// 00-0f: readback: 70-7f
// 10-1f: readback: 70-7f
// 20-2f: readback: 70-7f
// 30-3f: readback: 70-7f
// 40-5f: readback: 70-7f
// 50-5f: readback: 70-7f
// 60-6f: readback: ALL 7f
// 70-7f: readback: 50, 71-7f (force vblank?)
// 80-8f: 80 starts speech reads as f0, rest read as 71-7f
// 90-9f: all 70-7f
// a0-af: 70-7f
// b0-bf: 70-7f
// c0-cf: 70-7f
// d0-df: 70-7f
// e0-ef: readback ALL 76
// f0-ff: 70-7f
*/
/* The speech chip is a toshiba tc8802, it is NOT a t6803 since the t6803
   does not have the rom read mode, which the socrates definitely uses! */
/* Commands (tc8802):
SEL 5 4 3 2 1 0
    0 0 n n n n -  ADLD - ADdress LoaD - writes one nybble to the address
                          of vsm rom selected (internal or external) starting
                          from the low 4 bits; each subsequent write writes to
                          the next higher 4 bits; resetting the chip resets
                          the position
    0 1 n n n n -  CNDT - CoNDiTion (aka 'setup') - writes one nybble to the
                          setting register, starting from the low 4 bits; the
                          subsequent write will be to the high 4 bits;
                          resetting the chip will reset the position

                          CNDT bits (? are from T6803 datasheet, which could be wrong on the tc8802AF here):
                           * 76543210
                           * |||||||\-- ?bits/frame bit 1 (0 = see bit 2; 1 = 98bits/frame (these may be backwards))
                           * ||||||\--- ?melody mode select if high
                           * |||||\---- speech rom select; 0 is internal 8k mask, 1 is external vsm bus
                           * ||||\----- ?voiced source select (0 = A; 1 = B)
                           * |||\------ ?filter stages (0 = lpc-10; 1 = lpc-8)
                           * ||\------- ?always 0
                           * |\-------- ?frame length (0 = 20ms; 1 = 10ms)
                           * \--------- ?bits/frame bit 2 (if bit 1 is not set: 0 = 56 bits/frame; 1 = 50 bits/frame)
                           *
    1 0 x x x x -  DTRD - DaTa ReaD - reads a nybble from the address as loaded
                          and increments the address by one nybble. The first
                          DTRD command after any other commands is a 'dummy
                          read' which serves to reset the bus direction of the
                          low 4 SEL lines, identical to the way the TMS5100
                          does things. it doesn't affect the bus direction of
                          the high two SEL lines.
    1 1 0 x x x - START - Starts speech at the address as loaded.
    1 1 1 x x x - RESET - Resets the internal chip state (clears shifters and
                          counters) and forces speech to end/silence
                          immediately.
*/
	switch(data&0xF8)
	{
		case 0x80: case 0x88: case 0x90: case 0x98: case 0xA0: case 0xA8: case 0xB0: case 0xB8:
			/* 'dumb' mode speech command: write me: start talking */
			m_speech_running = 1;
			m_clear_speech_timer->adjust(attotime::from_seconds(4)); // hack
			break;
		case 0xC0: case 0xC8: // ADLD: load address to vsm
			m_speech_address |= (((int)data&0xF)<<(m_speech_load_address_count*4))<<1;
			m_speech_load_address_count++;
			logerror("loaded address nybble %X, byte address is currently %5X with %d nybbles loaded\n", data&0xF, m_speech_address>>1, m_speech_load_address_count);
			break;
		case 0xD0: case 0xD8: // CNDT: load settings
			m_speech_settings |= ((data&0xF)<<(m_speech_load_settings_count*4));
			m_speech_load_settings_count++;
			break;
		case 0xE0: case 0xE8: // DTRD: read byte, handled elsewhere
			break;
		case 0xF0: // SPEAK
			m_speech_running = 1;
			m_clear_speech_timer->adjust(attotime::from_seconds(4)); // hack
			break;
		case 0xF8: // RESET
			m_speech_running = 0;
			m_speech_address = 0;
			m_speech_settings = 0;
			m_speech_dummy_read = 0;
			m_speech_load_address_count = 0;
			m_speech_load_settings_count = 0;
			m_io40_latch &= 0x0f; // set last command to 0 to prevent problems
			break;
		default: // 00 through 70 are packets without the write bit set, ignore them
			break;
	}
	m_io40_latch = data;
}

uint8_t socrates_state::keyboard_buffer_read(offs_t offset)
{
	if (m_kbmcu_type == 0)
	{
		if (offset == 1) return m_kb_spi_buffer&0xFF;
		else return (m_kb_spi_buffer&0xFF00)>>8;
	}
	else // if ( (m_kbmcu_type == 1) || (m_kbmcu_type == 2) ) // iqunlimz hack, the system won't work without this?!?!
	{
		if (offset == 1) return (m_screen->vblank()?0x80:0)|(m_kbdrow[0]->read()&0xf);
		else return (m_kb_spi_buffer&0xFF00)>>8;
	}
}

void socrates_state::keyboard_buffer_update(uint8_t data)
{
	m_kb_spi_request = true;
	m_kb_spi_buffer = 0x0001;
}

void socrates_state::reset_speech(uint8_t data)// i/o 60: reset speech synth
{
	m_speech_running = 0;
	m_speech_address = 0;
	m_speech_settings = 0;
	m_speech_dummy_read = 0;
	m_speech_load_address_count = 0;
	m_speech_load_settings_count = 0;
	m_io40_latch &= 0x0f; // set last command to 0 to prevent problems
logerror("write to i/o 0x60 of %x\n",data);
}

/* stuff below belongs in video/socrates.c */
/* graphics section:
    0x20 - W - lsb offset of screen display
    0x21 - W - msb offset of screen display
    resulting screen line is one of 512 total offsets on 128-byte boundaries in the whole 64k ram
    */
void socrates_state::scroll_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	m_scroll_offset = (m_scroll_offset&0x100) | data;
	else
	m_scroll_offset = (m_scroll_offset&0xFF) | ((data&1)<<8);
}

/* NTSC-based Palette stuff */
// max for I and Q
#define M_I 0.5957
#define M_Q 0.5226
	/* luma amplitudes, measured on scope */
#define LUMAMAX 1.420
#define LUMA_COL_0 0.355, 0.139, 0.205, 0, 0.569, 0.355, 0.419, 0.205, 0.502, 0.288, 0.358, 0.142, 0.720, 0.502, 0.571, 0.358,
#define LUMA_COL_COMMON 0.52, 0.52, 0.52, 0.52, 0.734, 0.734, 0.734, 0.734, 0.667, 0.667, 0.667, 0.667, 0.885, 0.885, 0.885, 0.885,
#define LUMA_COL_2 0.574, 0.6565, 0.625, 0.71, 0.792, 0.87, 0.8425, 0.925, 0.724, 0.8055, 0.7825, 0.865, 0.94275, 1.0225, 0.99555, 1.07525,
#define LUMA_COL_5 0.4585, 0.382, 0.4065, 0.337, 0.6715, 0.5975, 0.6205, 0.5465, 0.6075, 0.531, 0.5555, 0.45, 0.8255, 0.7455, 0.774, 0.6985,
#define LUMA_COL_F 0.690, 0.904, 0.830, 1.053, 0.910, 1.120, 1.053, 1.270, 0.840, 1.053, 0.990, 1.202, 1.053, 1.270, 1.202, 1.420
	/* chroma amplitudes, measured on scope */
#define CHROMAMAX 0.42075
#define CHROMA_COL_COMMON 0.148, 0.3125, 0.26475, 0.42075, 0.148, 0.3125, 0.26475, 0.42075, 0.148, 0.3125, 0.26475, 0.42075, 0.148, 0.3125, 0.26475, 0.42075,
#define CHROMA_COL_2 0.125125, 0.27525, 0.230225, 0.384875, 0.125125, 0.27525, 0.230225, 0.384875, 0.125125, 0.27525, 0.230225, 0.384875, 0.125125, 0.27525, 0.230225, 0.384875,
#define CHROMA_COL_5 0.1235, 0.2695, 0.22625, 0.378, 0.1235, 0.2695, 0.22625, 0.378, 0.1235, 0.2695, 0.22625, 0.378, 0.1235, 0.2695, 0.22625, 0.378,
// gamma: this needs to be messed with... may differ on different systems... attach to slider somehow?
#define GAMMA 1.5

rgb_t socrates_state::create_color(uint8_t color)
{
	static constexpr double lumatable[256] = {
			LUMA_COL_0
			LUMA_COL_COMMON
			LUMA_COL_2
			LUMA_COL_COMMON
			LUMA_COL_COMMON
			LUMA_COL_5
			LUMA_COL_COMMON
			LUMA_COL_COMMON
			LUMA_COL_COMMON
			LUMA_COL_COMMON
			LUMA_COL_COMMON
			LUMA_COL_COMMON
			LUMA_COL_COMMON
			LUMA_COL_COMMON
			LUMA_COL_COMMON
			LUMA_COL_F };
	static constexpr double chromaintensity[256] = {
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			CHROMA_COL_COMMON
			CHROMA_COL_2
			CHROMA_COL_COMMON
			CHROMA_COL_COMMON
			CHROMA_COL_5
			CHROMA_COL_COMMON
			CHROMA_COL_COMMON
			CHROMA_COL_COMMON
			CHROMA_COL_COMMON
			CHROMA_COL_COMMON
			CHROMA_COL_COMMON
			CHROMA_COL_COMMON
			CHROMA_COL_COMMON
			CHROMA_COL_COMMON
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	/* chroma colors and phases:
	 0: black-through-grey (0 assumed chroma)
	 1: purple (90 chroma seems correct)
	 2: light blue/green (210 or 240 chroma, 210 seems slightly closer)
	 3: bright blue (150 seems correct)
	 4: green (270 seems correct)
	 5: red (30 seems correct, does have some blue in it)
	 6: orange (0 seems correct, does have some red in it)
	 7: yellow/gold (330 is closest but conflicts with color C, hence 315 seems close, and must have its own delay line separate from the other phases which use a standard 12 phase scheme)
	 8: blue with a hint of green in it (180 seems correct)
	 9: blue-green (210 seems correct)
	 A: forest green (240 seems correct)
	 B: yellow-green (300 seems correct)
	 C: yellow-orange (330 is close but this conflicts with color 7, and is not quite the same; color 7 has more green in it than color C)
	 D: magenta (60 is closest)
	 E: blue-purple (more blue than color 1, 120 is closest)
	 F: grey-through-white (0 assumed chroma)
	*/
	static constexpr double phaseangle[16] = { 0, 90, 220, 150, 270, 40, 0, 315, 180, 210, 240, 300, 330, 60, 120, 0 }; // note: these are guessed, not measured yet!
	int const chromaindex = color&0x0F;
	int const swappedcolor = ((color&0xf0)>>4)|((color&0x0f)<<4);
	double finalY = (1/LUMAMAX) * lumatable[swappedcolor];
	double const finalI = (M_I * (cos((phaseangle[chromaindex]/180)*M_PI)))* ((1/CHROMAMAX)*chromaintensity[swappedcolor]);
	double const finalQ = (M_Q * (sin((phaseangle[chromaindex]/180)*M_PI)))* ((1/CHROMAMAX)*chromaintensity[swappedcolor]);
	if (finalY > 1) finalY = 1; // clamp luma
	// calculate the R, G and B values here, neato matrix math
	double finalR = (finalY*1)+(finalI*0.9563)+(finalQ*0.6210);
	double finalG = (finalY*1)+(finalI*-0.2721)+(finalQ*-0.6474);
	double finalB = (finalY*1)+(finalI*-1.1070)+(finalQ*1.7046);
	// scale/clamp to 0-255 range
	if (finalR<0) finalR = 0;
	if (finalR>1) finalR = 1;
	if (finalG<0) finalG = 0;
	if (finalG>1) finalG = 1;
	if (finalB<0) finalB = 0;
	if (finalB>1) finalB = 1;
	// gamma correction: 1.0 to GAMMA:
	finalR = pow(finalR, 1/GAMMA)*255;
	finalG = pow(finalG, 1/GAMMA)*255;
	finalB = pow(finalB, 1/GAMMA)*255;
	return rgb_t((int)finalR,(int)finalG,(int)finalB);
}


void socrates_state::palette_init(palette_device &palette) const
{
	for (int i = 0; i < 256; i++)
		palette.set_pen_color(i, create_color(i));
}

void socrates_state::video_start()
{
	m_scroll_offset = 0;
}

uint32_t socrates_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const uint8_t fixedcolors[8] = {
			0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0xF7 };
	uint8_t const *const videoram = m_vram_reg;
	int lineoffset = 0; // if display ever tries to display data at 0xfxxx, offset line displayed by 0x1000
	for (int y = 0; y < 228; y++)
	{
		if ((((y+m_scroll_offset)*128)&0xffff) >= 0xf000) lineoffset = 0x1000; // see comment above
		for (int x = 0; x < 264; x++)
		{
			int color;
			if (x < 256)
			{
				int colidx =videoram[(((y+m_scroll_offset)*128)+(x>>1)+lineoffset)&0xffff];
				if (x&1) colidx >>=4;
				colidx &= 0xF;
				if (colidx > 7) color=videoram[0xF000+(colidx<<8)+((y+m_scroll_offset)&0xFF)];
				else color=fixedcolors[colidx];
			}
			else
			{
				int colidx = videoram[(((y+m_scroll_offset)*128)+(127)+lineoffset)&0xffff];
				colidx >>=4;
				colidx &= 0xF;
				if (colidx > 7) color=videoram[0xF000+(colidx<<8)+((y+m_scroll_offset)&0xFF)];
				else color=fixedcolors[colidx];
			}
			bitmap.pix(y, x) = color;
		}
	}
	return 0;
}

/* below belongs in audio/socrates.cpp */

void socrates_state::sound_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0:
		m_sound->reg0_w(data);
		break;
		case 1:
		m_sound->reg1_w(data);
		break;
		case 2:
		m_sound->reg2_w(data);
		break;
		case 3:
		m_sound->reg3_w(data);
		break;
		case 4: case 5: case 6: case 7: default:
		m_sound->reg4_w(data);
		break;
	}
}



//-----------------------------------------------------------------------------
//
//        IQ Unlimited
//
//-----------------------------------------------------------------------------

int iqunlimz_state::get_color(int index, int y)
{
	if (index < 8)
		return m_colors[index];
	else
		return m_vram_reg[0xf000 + ((index & 0x0f) << 8) + ((m_scroll_offset + y + 1) & 0xff)];
}

uint32_t iqunlimz_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *videoram = m_vram_reg;

	// bitmap layer
	for (int y=0; y<224; y++)
	{
		if (y >= m_video_regs[0x03])    break;
		for (int x=0; x<128; x++)
		{
			uint8_t data = videoram[(m_scroll_offset + y) * 0x80 + x];

			for(int b=0; b<2; b++)
			{
				bitmap.pix(y, x*2 + b) = get_color(data & 0x0f, y);
				data >>= 4;
			}
		}
	}

	// text layer
	bool mode_80 = m_video_regs[0x02] & 0x02;   // 80 chars mode
	int line_len = mode_80 ? 80 : 40;
	for (int y=0; y<28; y++)
	{
		for (int x=0; x<line_len; x++)
		{
			uint8_t c = videoram[0x8400 + (y - 1) * (mode_80 ? 0x80 : 0x40) + x];
			uint8_t *gfx = &videoram[0x8000 + (c & 0x7f) * 8];

			for (int cy=0; cy<8; cy++)
			{
				int py = y * 8 + cy;
				if ((m_video_regs[0x02] & 0x01) || py >= m_video_regs[0x03])
				{
					uint8_t col0 = get_color(0x0e, py);
					uint8_t col1 = get_color(0x0f, py);
					uint8_t data = 0;

					if (BIT(m_video_regs[0x02],4) && m_video_regs[0x00] == x && m_video_regs[0x01] == ((y - 1) + (0x400 / (mode_80 ? 0x80 : 0x40)))) // cursor position start at 0x8000
						data = 0xff;
					else if (y > 0 && y < 26)
						data = gfx[cy] ^ (c & 0x80 ? 0xff : 0);

					if (x == 0) bitmap.plot_box(0, py, 8 + line_len*6 + 8, 1, col0);
					for (int cx=0; cx<6; cx++)
					{
						int px = 8 + x*6 + cx;
						bitmap.pix(py, px) = BIT(data, 7) ? col1 : col0;
						data <<= 1;
					}
				}
			}
		}
	}

	return 0;
}

uint8_t iqunlimz_state::status_r()
{
	// ---x ----    main battery status
	// --x- ----    backup battery status

	return 0x30;
}

uint8_t iqunlimz_state::video_regs_r(offs_t offset)
{
	return m_video_regs[offset];
}

void iqunlimz_state::video_regs_w(offs_t offset, uint8_t data)
{
	if (offset == 2 && ((m_video_regs[offset] ^ data) & 0x02))
	{
		rectangle visarea = m_screen->visible_area();
		visarea.set(0, (data & 0x02 ? 496 : 256) - 1, 0, 224 - 1);
		m_screen->configure(data & 0x02 ? 496 : 256 , 224, visarea, m_screen->frame_period().attoseconds());
	}

	m_video_regs[offset] = data;
}

void iqunlimz_state::machine_reset()
{
	socrates_state::machine_reset();

	memset(m_colors, 0, 8);
	memset(m_video_regs, 0, 4);
	kbmcu_sim_reset();
}

void iqunlimz_state::colors_w(offs_t offset, uint8_t data)
{
	m_colors[offset] = data;
}


// IQ Unlimited keyboard MCU simulation

INPUT_CHANGED_MEMBER( iqunlimz_state::send_input )
{
	uint8_t data = (uint8_t)param;

	if (newval)
	{
		kbmcu_sim_fifo_enqueue(data<<4);
	}
}

/******************************************************************************
 Address Maps
******************************************************************************/

void socrates_state::socrates_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom(); /* system rom, bank 0 (fixed) */
	map(0x4000, 0x7fff).m(m_rombank1, FUNC(address_map_bank_device::amap8)); /* banked rom space; system rom is banks 0 through F, cartridge rom is banks 10 onward, usually banks 10 through 17. area past the end of the cartridge, and the whole 10-ff area when no cartridge is inserted, reads as 0xF3 */
	map(0x8000, 0xbfff).m(m_rambank1, FUNC(address_map_bank_device::amap8)); /* banked ram 'window' 0 */
	map(0xc000, 0xffff).m(m_rambank2, FUNC(address_map_bank_device::amap8)); /* banked ram 'window' 1 */
}

void socrates_state::socrates_rombank_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x03ffff).rom().region("maincpu", 0).mirror(0xF00000);      // xxxx 00** **** **** **** ****
	map(0x040000, 0x07ffff).r(FUNC(socrates_state::cart_r)).select(0xF80000);            // **** *1** **** **** **** ****
	map(0x080000, 0x0bffff).r(FUNC(socrates_state::read_f3));                                        // xxxx 10** **** **** **** ****
}

void socrates_state::socrates_rambank_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).ram().share("vram").mirror(0x30000);
}

void socrates_state::socrates_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(socrates_state::common_rom_bank_r), FUNC(socrates_state::common_rom_bank_w)).mirror(0x7); /* rom bank select - RW - 8 bits */
	map(0x08, 0x08).rw(FUNC(socrates_state::common_ram_bank_r), FUNC(socrates_state::common_ram_bank_w)).mirror(0x7); /* ram banks select - RW - 4 low bits; Format: 0b****HHLL where LL controls whether window 0 points at ram area: 0b00: 0x0000-0x3fff; 0b01: 0x4000-0x7fff; 0b10: 0x8000-0xbfff; 0b11: 0xc000-0xffff. HH controls the same thing for window 1 */
	map(0x10, 0x17).rw(FUNC(socrates_state::read_f3), FUNC(socrates_state::sound_w)).mirror(0x8); /* sound section:
	0x10 - W - frequency control for channel 1 (louder channel) - 01=high pitch, ff=low; time between 1->0/0->1 transitions = (XTAL(21'477'272)/(512+256) / (freq_reg+1)) (note that this is double the actual frequency since each full low and high squarewave pulse is two transitions)
	0x11 - W - frequency control for channel 2 (softer channel) - 01=high pitch, ff=low; same equation as above
	0x12 - W - 0b***EVVVV enable, volume control for channel 1
	0x13 - W - 0b***EVVVV enable, volume control for channel 2
	0x14-0x17 - 0bE??????? enable, unknown for channel 3; produces well defined dmc waves when bit 7 is set, and one or two other bits
	This may be some sort of debug register for serial-dmc banging out some internal rom from the asic, maybe color data?
	No writes to ram seem to change the waveforms produced, in my limited testing.
	0x80 produces about a very very quiet 1/8 duty cycle wave at 60hz or so
	0xC0 produces a DMC wave read from an unknown address at around 342hz
	<todo: test the others, maybe take samples?>
	*/
	map(0x20, 0x21).rw(FUNC(socrates_state::read_f3), FUNC(socrates_state::scroll_w)).mirror(0xE);
	map(0x30, 0x30).rw(FUNC(socrates_state::read_f3), FUNC(socrates_state::kbmcu_reset)).mirror(0xF); /* resets the keyboard IR decoder MCU */
	map(0x40, 0x40).rw(FUNC(socrates_state::status_r), FUNC(socrates_state::speech_command)).mirror(0xF); /* reads status register for vblank/hblank/speech, also reads and writes speech module */
	map(0x50, 0x51).rw(FUNC(socrates_state::keyboard_buffer_read), FUNC(socrates_state::keyboard_buffer_update)).mirror(0xE); /* Keyboard fifo read, pop fifo on write */
	map(0x60, 0x60).rw(FUNC(socrates_state::read_f3), FUNC(socrates_state::reset_speech)).mirror(0xF); /* reset the speech module, or perhaps fire an NMI?  */
	map(0x70, 0xFF).r(FUNC(socrates_state::read_f3)); // nothing mapped here afaik
}

void iqunlimz_state::iqunlimz_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).m(m_rombank2, FUNC(address_map_bank_device::amap8));
	map(0x4000, 0x7fff).m(m_rombank1, FUNC(address_map_bank_device::amap8));
	map(0x8000, 0xbfff).m(m_rambank1, FUNC(address_map_bank_device::amap8));
	map(0xc000, 0xffff).m(m_rambank2, FUNC(address_map_bank_device::amap8));
}

void iqunlimz_state::iqunlimz_rombank_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x03ffff).rom().region("maincpu", 0).mirror(0xF00000);      // xxxx 00** **** **** **** ****
	map(0x040000, 0x07ffff).r(FUNC(iqunlimz_state::cart_r)).select(0xF80000);            // **** *1** **** **** **** ****
	map(0x080000, 0x0bffff).rom().region("maincpu", 0x40000).mirror(0xF00000);// xxxx 10** **** **** **** ****
}

void iqunlimz_state::iqunlimz_rambank_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3ffff).ram().share("vram");
}

void iqunlimz_state::iqunlimz_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x01).rw(FUNC(iqunlimz_state::common_rom_bank_r), FUNC(iqunlimz_state::common_rom_bank_w)).mirror(0x06);
	map(0x08, 0x08).rw(FUNC(iqunlimz_state::common_ram_bank_r), FUNC(iqunlimz_state::common_ram_bank_w)).mirror(0x07);
	map(0x10, 0x17).w(FUNC(iqunlimz_state::sound_w)).mirror(0x08);
	map(0x20, 0x21).w(FUNC(iqunlimz_state::scroll_w)).mirror(0x0E);
	// 30: writes an incrementing value here, once per keypress?
	// 40: some sort of serial select/reset or enable, related to 0x60
	map(0x50, 0x51).rw(FUNC(iqunlimz_state::keyboard_buffer_read), FUNC(iqunlimz_state::keyboard_buffer_update)).mirror(0xE);
	// 60: some sort of serial read/write port, related to 0x40
	map(0x70, 0x73).rw(FUNC(iqunlimz_state::video_regs_r), FUNC(iqunlimz_state::video_regs_w)).mirror(0x0C);
	map(0x80, 0x81).nopw(); // LCD
	map(0xb1, 0xb1).nopw();
	map(0xa0, 0xa0).r(FUNC(iqunlimz_state::status_r)).mirror(0x0F);
	map(0xe0, 0xe7).w(FUNC(iqunlimz_state::colors_w)).mirror(0x08);
}



/******************************************************************************
 Input Ports
******************************************************************************/

/* socrates keyboard codes:
keycode low
|   keycode high
|   |   key name
00  01  No key pressed
// pads on the sides of the kb; this acts like a giant bitfield, both dpads/buttons can send data at once
00  81  left dpad right
00  82  left dpad up
00  84  left dpad left
00  88  left dpad down
01  80  right dpad down
02  80  right dpad left
04  80  right dpad up
08  80  right dpad right
10  80  left red button
20  80  right red button
// top row (right to left)
44  82  ENTER
44  83  MENU
44  84  ANSWER
44  85  HELP
44  86  ERASE
44  87  divide_sign
44  88  multiply_sign
44  89  minus_sign
44  80  plus_sign
//second row (right to left)
43  81  0
43  82  9
43  83  8
43  84  7
43  85  6
43  86  5
43  87  4
43  88  3
43  89  2
43  80  1
// third row (right to left)
42  82  I/La
42  83  H/So
42  84  G/Fa
42  85  F/Mi
42  86  E/Re
42  87  D/Do
42  88  C/Ti.
42  89  B/La.
42  80  A/So.
42  81  hyphen/period
// fourth row (right to left)
41  81  S
41  82  R
41  83  Q/NEW
41  84  P/PLAY
41  85  O/PAUSE
41  86  N/Fa`
41  87  M/Mi`
41  88  L/Re`
41  89  K/Do`
41  80  J/Ti
// fifth row (right to left)
40  82  SPACE
40  83  Z
40  84  Y
40  85  X
40  86  W
40  87  V
40  88  U
40  89  T
50  80  SHIFT
// socrates mouse pad (separate from keyboard)
8x  8y  mouse movement
x: down = 1 (small) through 7 (large), up = 8 (small) through F (large)
y: right = 1 (small) through 7 (large), left = 8 (small) through F (large)
90  80  right click
A0  80  left click
B0  80  both buttons click
90  81  right click (mouse movement in queue, will be in regs after next latch clear)
A0  81  left click (mouse movement in queue, will be in regs after next latch clear)
B0  81  both buttons click (mouse movement in queue, will be in regs after next latch clear)
// socrates touch pad
// unknown yet, but probably uses the 60/70/c0/d0/e0/f0 low reg vals
*/
static INPUT_PORTS_START( socrates )

	PORT_START("IN5") // joypad keys
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_NAME("Left D-pad Right") // 00 81
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_NAME("Left D-pad Up") // 00 82
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_NAME("Left D-pad Left") // 00 84
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_NAME("Left D-pad Down") // 00 88
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME("Right D-pad Down") // 01 80
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME("Right D-pad Left") // 02 80
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_NAME("Right D-pad Up") // 04 80
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME("Right D-pad Right") // 08 80
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_NAME("Left D-pad Button") // 10 80
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT)) PORT_NAME("Right D-pad Button") // 20 80
	PORT_BIT(0xfc00, IP_ACTIVE_HIGH, IPT_UNUSED)
	/* alt w/left and right keypad keys swapped
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME("Left D-pad Right") // 00 81
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_NAME("Left D-pad Up") // 00 82
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME("Left D-pad Left") // 00 84
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME("Left D-pad Down") // 00 88
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_NAME("Right D-pad Down") // 01 80
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_NAME("Right D-pad Left") // 02 80
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_NAME("Right D-pad Up") // 04 80
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_NAME("Right D-pad Right") // 08 80
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT)) PORT_NAME("Left D-pad Button") // 10 80
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_NAME("Right D-pad Button") // 20 80
	PORT_BIT(0xfc00, IP_ACTIVE_HIGH, IPT_UNUSED)
	*/

	PORT_START("IN6") // lowest 'row' (technically the shift key is on the 5th row but it has its own keycode)
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("SHIFT") // 5x xx
	PORT_BIT(0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN0") // 5th row
	PORT_BIT(0x0003, IP_ACTIVE_HIGH, IPT_UNUSED) // 40 80 and 40 81
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_NAME("SPACE") // 40 82
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_NAME("Z") // 40 83
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_NAME("Y") // 40 84
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_NAME("X") // 40 85
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_NAME("W") // 40 86
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_NAME("V") // 40 87
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_NAME("U") // 40 88
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_NAME("T") // 40 89
	PORT_BIT(0xfc00, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN1") // 4th row
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_NAME("J/Ti") // 41 80
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_NAME("S") // 41 81
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_NAME("R") // 41 82
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_NAME("Q/NEW") // 41 83
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_NAME("P/PLAY") // 41 84
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_NAME("O/PAUSE") // 41 85
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_NAME("N/Fa'") // 41 86
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_NAME("M/Mi'") // 41 87
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_NAME("L/Re'") // 41 88
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_NAME("K/Do'") // 41 89
	PORT_BIT(0xfc00, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN2") // 3rd row
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_NAME("A/So.") // 42 80
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('-') PORT_NAME("-/.") // 42 81
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_NAME("I/La") // 42 82
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_NAME("H/So") // 42 83
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_NAME("G/Fa") // 42 84
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_NAME("F/Mi") // 42 85
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_NAME("E/Re") // 42 86
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_NAME("D/Do") // 42 87
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_NAME("C/Ti.") // 42 88
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_NAME("B/La.") // 42 89
	PORT_BIT(0xfc00, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN3") // 2nd row
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_NAME("1") // 43 80
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_NAME("0") // 43 81
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_NAME("9") // 43 82
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_NAME("8") // 43 83
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_NAME("7") // 43 84
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_NAME("6") // 43 85
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_NAME("5") // 43 86
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_NAME("4") // 43 87
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_NAME("3") // 43 88
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_NAME("2") // 43 89
	PORT_BIT(0xfc00, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN4") // 1st row
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('+') PORT_NAME("+") // 44 80
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNUSED) // 44 81
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("ENTER") // 44 82
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_NAME("MENU") // 44 83
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(PGUP)) PORT_NAME("ANSWER") // 44 84
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(PGDN)) PORT_NAME("HELP") // 44 85
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("ERASE") // 44 86
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_NAME("/") // 44 87
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*') PORT_NAME("*") // 44 88
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_NAME("-") // 44 89
	PORT_BIT(0xfc00, IP_ACTIVE_HIGH, IPT_UNUSED)

	// mouse goes here
INPUT_PORTS_END


static INPUT_PORTS_START( iqunlimz )
	PORT_START( "IN0" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_LSHIFT )  PORT_CODE( KEYCODE_RSHIFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_LCONTROL )  PORT_CODE( KEYCODE_RCONTROL )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_LALT )  PORT_CODE( KEYCODE_RALT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_CAPSLOCK )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START( "IN1" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_N ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x10 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_M ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x11 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_COMMA ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x12 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_B ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x13 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_C ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x14 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_Z ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x15 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_V ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x16 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_X ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x17 )

	PORT_START( "IN2" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x20 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_2_PAD ) PORT_CODE( KEYCODE_DOWN ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x21 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_5_PAD ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x22 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_DEL_PAD ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x23 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_SLASH_PAD ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x24 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_8_PAD ) PORT_CODE( KEYCODE_UP ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x25 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_ASTERISK ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x26 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_7_PAD ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x27 )

	PORT_START( "IN3" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x30 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x31 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x32 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x33 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_F1 ) PORT_NAME( "Help" ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x34 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_9 ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x35 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_BACKSPACE ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x36 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_0 ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x37 )

	PORT_START( "IN4" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_H ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x40 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_J ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x41 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_K ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x42 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_G ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x43 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_D ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x44 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_A ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x45 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_F ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x46 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_S ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x47 )

	PORT_START( "IN5" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x50 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x51 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x52 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x53 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_EQUALS ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x54 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_O ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x55 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_NUMLOCK ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x56 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_P ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x57 )

	PORT_START( "IN6" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_Y ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x60 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_U ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x61 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_I ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x62 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_T ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x63 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_E ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x64 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_Q ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x65 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_R ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x66 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_W ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x67 )

	PORT_START( "IN7" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_MINUS ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x70 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_SLASH ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x71 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x72 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_STOP ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x73 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_QUOTE ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x74 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_L ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x75 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_ENTER ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x76 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_COLON ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x77 )

	PORT_START( "IN8" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_6 ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x80 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_7 ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x81 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_8 ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x82 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_5 ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x83 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_3 ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x84 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_1 ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x85 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_4 ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x86 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_2 ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x87 )

	PORT_START( "IN9" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_4_PAD ) PORT_CODE( KEYCODE_LEFT ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x90 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_3_PAD ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x91 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_6_PAD ) PORT_CODE( KEYCODE_RIGHT ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x92 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_PLUS_PAD ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x93 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_0_PAD ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x94 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_9_PAD ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x95 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_MINUS_PAD ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x96 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_1_PAD ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0x97 )

	PORT_START( "INA" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0xa0 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0xa1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0xa2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0xa3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_F2 ) PORT_NAME("Answer") PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0xa4 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_ESC ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0xa5 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_SPACE ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0xa6 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_TAB ) PORT_CHANGED_MEMBER( DEVICE_SELF, iqunlimz_state, send_input, 0xa7 )
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/
TIMER_CALLBACK_MEMBER(socrates_state::clear_irq_cb)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(socrates_state::assert_irq)
{
	device.execute().set_input_line(0, ASSERT_LINE);
	m_clear_irq_timer->adjust(m_maincpu->cycles_to_attotime(44));
	// 44 is a complete and total guess, need to properly measure how many clocks/microseconds the int line is high for.
}

TIMER_CALLBACK_MEMBER(socrates_state::kbmcu_sim_cb)
{
	// timer rate is a massive guess; we're assuming the mcu runs at the same speed as the cpu does,
	// and the refresh rate depends on instructions per loop of mcu, which we've randomly guessed is 60 cycles.
	m_kbmcu_sim_timer->adjust(m_maincpu->cycles_to_attotime(3000));
	/// TODO: dump the mcus and get rid of this...
	if (m_kbmcu_type == 0)
	{
		// socrates keyboard MCU simulation: if a keyboard key is pressed, enqueue it into the fifo as needed
		uint16_t keyvalue = 0;
		// check for joypad buttons
		keyvalue = m_kbdrow[5]->read();
		if (keyvalue == 0) // if joypad wasn't pushed...
		{
			// next check for mouse movement.
			/// TODO: this isn't written yet
			// next check for basic keyboard "4x xx" stuff
			// check if shift is down
			keyvalue = (m_kbdrow[6]->read())?0x500:0;
			bool keyfound = false;
			// find what row and bit we're on...
			for (int8_t row = 4; row>=0; row--)
			{
				uint16_t tempkey = m_kbdrow[row]->read();
				if (tempkey != 0)
				{
					for (int8_t powerof2 = 9; ((powerof2 >= 0) && (keyfound == false)); powerof2--) // continue until we find the first key only
					{
						if ((tempkey&(1<<powerof2)) == (1<<powerof2))
						{
							keyvalue |= (0x400 | (row<<4) | powerof2);
							keyfound = true;
						}
					}
				}
			}
		}
		if (keyvalue != 0) // if a key is down...
		{
			if (keyvalue == m_oldkeyvalue) // and its the same key as it was the last run...
			{
				if (m_keyrepeat_holdoffcounter > 0) // and the key repeat holdoff counter is > 0...
				{
					m_keyrepeat_holdoffcounter--; // decrement the holdoff counter
				}
				else // the key repeat holdoff counter is zero
				{
					kbmcu_sim_fifo_enqueue(0x1000|keyvalue); // queue the key with bit 12 set as a flag that this is a repeat
					m_keyrepeat_holdoffcounter += KEYREPEAT_REPEAT; // increment the holdoff counter by the repeat value
				}
			}
			else // it isn't the same key as it was the last run
			{
				kbmcu_sim_fifo_enqueue(keyvalue); // queue the key
				m_keyrepeat_holdoffcounter = KEYREPEAT_HOLDOFF; // reset the holdoff counter
				m_oldkeyvalue = keyvalue; // set this new key to be the 'previous key'
			}
		}
		else // keyvalue is zero, reset the holdoff counter
		{
			m_keyrepeat_holdoffcounter = KEYREPEAT_HOLDOFF;
			m_oldkeyvalue = keyvalue; // set this new key to be the 'previous key'
		}
	}
	//else if (m_kbmcu_type == 1) // this is currently hacked around by the input system so no code here
	//{
	//}

	if ((m_kb_spi_request) && (m_kb_queue.count > 0)) // if the console requested data from the MCU AND the MCU has data to send...
	{
		uint16_t tempbuffer = 0;
		m_kb_spi_request = false;
		tempbuffer = kbmcu_sim_fifo_peek();
		if (tempbuffer&0x1000) // repeat bit was set
		{
			kbmcu_sim_fifo_head_clear(); // clear the head byte of the fifo, but leave it enqueued so it is returned next time. socrates wants this...
		}
		else tempbuffer = kbmcu_sim_fifo_dequeue();
		m_kb_spi_buffer = ((tempbuffer&0xFF0)<<4)|(tempbuffer&0xF)|0x80;
	}
}

void socrates_state::socrates(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(21'477'272)/6);  /* Toshiba TMPZ84C00AP @ 3.579545 MHz, verified, xtal is divided by 6 */
	m_maincpu->set_addrmap(AS_PROGRAM, &socrates_state::socrates_mem);
	m_maincpu->set_addrmap(AS_IO, &socrates_state::socrates_io);
	m_maincpu->set_vblank_int("screen", FUNC(socrates_state::assert_irq));
	config.set_maximum_quantum(attotime::from_hz(60));

	ADDRESS_MAP_BANK(config, "rombank1").set_map(&socrates_state::socrates_rombank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);
	ADDRESS_MAP_BANK(config, "rambank1").set_map(&socrates_state::socrates_rambank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);
	ADDRESS_MAP_BANK(config, "rambank2").set_map(&socrates_state::socrates_rambank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(264, 228); // technically the screen size is 256x228 but super painter abuses what I suspect is a hardware bug to display repeated pixels of the very last pixel beyond this horizontal space, well into hblank
	m_screen->set_visarea(0, 263, 0, 219); // the last few rows are usually cut off by the screen bottom but are indeed displayed if you mess with v-hold
	m_screen->set_screen_update(FUNC(socrates_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(socrates_state::palette_init), 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SOCRATES_SOUND(config, m_sound, XTAL(21'477'272)/(512+256)).add_route(ALL_OUTPUTS, "mono", 0.25); // this is correct, as strange as it sounds.

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "socrates_cart");

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("socrates");
}

void socrates_state::socrates_pal(machine_config &config)
{
	socrates(config);

	m_maincpu->set_clock(XTAL(26'601'712)/8); // XTAL verified, divider NOT verified; this is a later ASIC so the divider may be different

	config.set_maximum_quantum(attotime::from_hz(50));

	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_size(264, 238); // technically the screen size is 256x228 but super painter abuses what I suspect is a hardware bug to display repeated pixels of the very last pixel beyond this horizontal space, well into hblank
	m_screen->set_visarea(0, 263, 0, 229); // the last few rows are usually cut off by the screen bottom but are indeed displayed if you mess with v-hold
	m_screen->set_screen_update(FUNC(socrates_state::screen_update));

	m_sound->set_clock(XTAL(26'601'712)/(512+256)); // this is correct, as strange as it sounds.
}

void socrates_state::vpainter_pal(machine_config &config)
{
	socrates_pal(config);

	config.device_remove("cartslot");
	config.device_remove("cart_list");
}

void iqunlimz_state::iqunlimz(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000)); /* not accurate */
	m_maincpu->set_addrmap(AS_PROGRAM, &iqunlimz_state::iqunlimz_mem);
	m_maincpu->set_addrmap(AS_IO, &iqunlimz_state::iqunlimz_io);
	m_maincpu->set_vblank_int("screen", FUNC(iqunlimz_state::assert_irq));

	ADDRESS_MAP_BANK(config, "rombank1").set_map(&iqunlimz_state::iqunlimz_rombank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);
	ADDRESS_MAP_BANK(config, "rombank2").set_map(&iqunlimz_state::iqunlimz_rombank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);
	ADDRESS_MAP_BANK(config, "rambank1").set_map(&iqunlimz_state::iqunlimz_rambank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);
	ADDRESS_MAP_BANK(config, "rambank2").set_map(&iqunlimz_state::iqunlimz_rambank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_screen_update(FUNC(iqunlimz_state::screen_update));
	m_screen->set_size(256, 224);
	m_screen->set_visarea(0, 256-1, 0, 224-1);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(iqunlimz_state::palette_init), 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SOCRATES_SOUND(config, m_sound, XTAL(21'477'272)/(512+256)).add_route(ALL_OUTPUTS, "mono", 0.25);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, nullptr);
}

/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(socrates)
	ROM_REGION(0x400000, "maincpu", ROMREGION_ERASEVAL(0xF3)) /* can technically address 4mb of rom via bankswitching; open bus area reads as 0xF3 on fluke */
	/* Socrates English (same ROM for NTSC and PAL) */
	/* All cart ROMs are 28 pin 23c1000/tc531000 128Kx8 ROMs */
	/* Cart port pinout:
	(looking into end of disk-shaped cartridge with label/top side pointing to the right)
	A15 -> 19  18 -- VCC
	A14 -> 20  17 <- A16
	A13 -> 21  16 <- A12
	 A8 -> 22  15 <- A7
	 A9 -> 23  14 <- A6
	A11 -> 24  13 <- A5
	 A3 -> 25  12 <- A4
	 A2 -> 26  11 <- A10
	 D7 <- 27  10 <- A1
	 D6 <- 28  9 <- A0
	 D5 <- 29  8 -> D0
	 D4 <- 30  7 -> D1
	 D3 <- 31  6 -> D2
	  ? ?? 32  5 ?? ?
	A17 -> 33  4 ?? ?
	  ? ?? 34  3 ?? ?
	/CE -> 35  2 ?? ?
	GND -- 36  1 -- GND
	Note that a17 goes to what would be pin 2 on a rom chip if a 32 pin rom were installed, which is not the case. (pins 1, 31 and 32 would be tied to vcc)
	It is likely that at least one of the 6 unknown lines is R/W from the z80, and another may be phi1/m1/clock etc to allow for ram to live in cart space

	Cartridge check procedure by Socrates is, after screen init and check for speech synth,
	bankswitch to bank 0x10 (i.e. first 0x4000 of cart appears at 4000-7fff in z80 space),
	do following tests; if any tests fail, jump to 0x0015 (Socrates main menu)
	* read 0x7ff0(0x3ff0 in cart rom) and compare to 0xAA
	* read 0x7ff1(0x3ff1 in cart rom) and compare to 0x55
	* read 0x7ff2(0x3ff2 in cart rom) and compare to 0xE7
	* read 0x7ff3(0x3ff3 in cart rom) and compare to 0x18
	if all tests passed, jump to 0x4000 (0x0000 in cart rom)
	*/
	ROM_LOAD("27-00817-000-000.u1", 0x00000, 0x40000, CRC(80f5aa20) SHA1(4fd1ff7f78b5dd2582d5de6f30633e4e4f34ca8f)) // Label: "(Vtech) 27-00817-000-000 // (C)1987 VIDEO TECHNOLOGY // 8811 D"

	ROM_REGION(0x800, "kbmcu", ROMREGION_ERASEFF)
	ROM_LOAD("tmp42c40p1844.u6", 0x000, 0x200, NO_DUMP) /* keyboard IR decoder MCU */

	/* english speech cart has a green QC sticker */
	ROM_REGION(0x2000, "speechint", ROMREGION_ERASE00) // speech data inside of the speech chip; fill with 00, if no speech cart is present socrates will see this
	ROM_LOAD_OPTIONAL("speech_eng_internal.bin", 0x0000, 0x2000, CRC(edc1fb3f) SHA1(78b4631fc3b1c038e14911047f9edd6c4e8bae58)) // 8k on the speech chip itself

	ROM_REGION(0x10000, "speechext", ROMREGION_ERASE00) // speech serial modules outside of the speech chip but still on speech cart
	ROM_LOAD_OPTIONAL("speech_eng_vsm1.bin", 0x0000, 0x4000, CRC(888e3ddd) SHA1(33af6a21ba6d826071c9d48557b1c9012752570b)) // 16k in serial rom
	ROM_LOAD_OPTIONAL("speech_eng_vsm2.bin", 0x4000, 0x4000, CRC(de4ac89d) SHA1(3dfa853b02df756a9b72def94a39310992ee11c7)) // 16k in serial rom
	ROM_LOAD_OPTIONAL("speech_eng_vsm3.bin", 0x8000, 0x4000, CRC(972384aa) SHA1(ffcb1d633ca6bffc7f481ec505da447e5b847f16)) // 16k in serial rom
	ROM_FILL(0xC000, 0x4000, 0xff) // last vsm isn't present, FF fill
ROM_END

ROM_START(socratfc)
	ROM_REGION(0x80000, "maincpu", ROMREGION_ERASEVAL(0xF3))
	/* Socrates SAITOUT (French Canadian) NTSC */
	ROM_LOAD("27-00884-001-000.u1", 0x00000, 0x40000, CRC(042d9d21) SHA1(9ffc67b2721683b2536727d0592798fbc4d061cb)) // Label: "(Vtech) 27-00884-001-000 // (C)1988 VIDEO TECHNOLOGY // 8911 D"

	ROM_REGION(0x800, "kbmcu", ROMREGION_ERASEFF)
	ROM_LOAD("tmp42c40p1844.u6", 0x000, 0x200, NO_DUMP) /* keyboard IR decoder MCU */

	ROM_REGION(0x2000, "speechint", ROMREGION_ERASE00) // speech data inside of the speech chip; fill with 00, if no speech cart is present socrates will see this
	ROM_LOAD_OPTIONAL("speech_fra_internal.bin", 0x0000, 0x2000, NO_DUMP)

	ROM_REGION(0x10000, "speechext", ROMREGION_ERASE00) // speech serial modules outside of the speech chip but still on speech cart
	ROM_LOAD_OPTIONAL("speech_fra_vsm1.bin", 0x0000, 0x4000, NO_DUMP) // 16k in serial rom
	ROM_LOAD_OPTIONAL("speech_fra_vsm2.bin", 0x4000, 0x4000, NO_DUMP) // 16k in serial rom
	ROM_LOAD_OPTIONAL("speech_fra_vsm3.bin", 0x8000, 0x4000, NO_DUMP) // 16k in serial rom
	ROM_FILL(0xC000, 0x4000, 0xff) // last vsm isn't present, FF fill
ROM_END

ROM_START(profweis)
	ROM_REGION(0x80000, "maincpu", ROMREGION_ERASEVAL(0xF3))
	/* Yeno Professor Weiss-Alles (German PAL) */
	ROM_SYSTEM_BIOS(0, "89", "1989")
	ROMX_LOAD("lh53216d.u1", 0x00000, 0x40000, CRC(6e801762) SHA1(b80574a3abacf18133dacb9d3a8d9e2916730423), ROM_BIOS(0)) // Label: "(Vtech) LH53216D // (C)1989 VIDEO TECHNOLOGY // 9119 D"
	ROM_SYSTEM_BIOS(1, "88", "1988")
	ROMX_LOAD("27-00885-000-000.u1", 0x00000, 0x40000, CRC(fcaf8850) SHA1(a99011ee6a1ef63461c00d062278951252f117db), ROM_BIOS(1)) // Label: "(Vtech) 27-00885-000-000 // (C)1988 VIDEO TECHNOLOGY // 8844 D"

	ROM_REGION(0x800, "kbmcu", ROMREGION_ERASEFF)
	ROM_LOAD("tmp42c40p1844.u6", 0x000, 0x200, NO_DUMP) /* keyboard IR decoder MCU */

	ROM_REGION(0x2000, "speechint", ROMREGION_ERASE00) // speech data inside of the speech chip; fill with 00, if no speech cart is present socrates will see this
	ROM_LOAD_OPTIONAL("speech_ger_internal.bin", 0x0000, 0x2000, CRC(5ff0fdc6) SHA1(8ef128561a846762a20e3fe9513a4a22aaadc7f6))

	ROM_REGION(0x10000, "speechext", ROMREGION_ERASE00) // speech serial modules outside of the speech chip but still on speech cart
	ROM_LOAD_OPTIONAL("speech_ger_vsm1.bin", 0x0000, 0x4000, CRC(a979a00b) SHA1(0290844619dbdf336757003aaab3ffd0a75b7ee9)) // 16k in serial rom
	ROM_FILL(0x4000, 0xc000, 0xff) // last 3 vsms aren't present, FF fill
ROM_END

ROM_START( iqunlimz )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "vtech.bin", 0x000000, 0x080000, CRC(f100c8a7) SHA1(6ad2a8accae2dd5c5c46ae953eef33cdd1ea3cf9) )

	ROM_REGION(0x1000, "kbmcu", ROMREGION_ERASEFF)
	ROM_LOAD("kbmcu.bin", 0x0000, 0x1000, NO_DUMP) /* keyboard reader MCU */
ROM_END

ROM_START( vpainter )
	ROM_REGION(0x80000, "maincpu", ROMREGION_ERASEVAL(0xF3))
	ROM_LOAD("lh531g02.u1", 0x00000, 0x20000, CRC(898defac) SHA1(8307e00b5ce3675ce71960e7cf2d1334197a1dce)) // Label: "(Vtech)LH531G02 // (C)1991 VIDEO TECHNOLOGY // 9211 D"

	ROM_REGION(0x1000, "kbmcu", ROMREGION_ERASEFF)
	ROM_LOAD("tmp47c241nj408.u6", 0x0000, 0x1000, NO_DUMP) /* key/touchpad reader MCU */

	///TODO: get rid of these regions in the status_read function or make it socrates-specific
	ROM_REGION(0x2000, "speechint", ROMREGION_ERASE00) // doesn't exist? on vpainter, presumably reads as all zeroes
	ROM_REGION(0x10000, "speechext", ROMREGION_ERASE00) // doesn't exist? on vpainter, presumably reads as all zeroes
ROM_END

} // anonymous namespace


/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME      PARENT    COMPAT  MACHINE       INPUT     STATE           INIT           COMPANY                    FULLNAME                             FLAGS
COMP( 1988, socrates, 0,        0,      socrates,     socrates, socrates_state, init_socrates, "Video Technology",        "Socrates Educational Video System", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // English, no title copyright, same ROM for NTSC and PAL
COMP( 1988, socratfc, socrates, 0,      socrates,     socrates, socrates_state, init_socrates, "Video Technology",        "Socrates SAITOUT",                  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // French Canandian NTSC, 1988 title copyright
// Yeno Professeur Saitout (French SECAM) matches the Socrates SAITOUT dump (same ROM 27-00884-001-000)
COMP( 1988, profweis, socrates, 0,      socrates_pal, socrates, socrates_state, init_socrates, "Video Technology / Yeno", "Professor Weiss-Alles",             MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // German PAL, 1988 title copyright
// ? goes here (Spanish PAL)

COMP( 1991, iqunlimz, 0,        0,      iqunlimz,     iqunlimz, iqunlimz_state, init_iqunlimz, "Video Technology",        "IQ Unlimited (Z80)",                MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

COMP( 1991, vpainter, 0,        0,      vpainter_pal, socrates, socrates_state, init_vpainter, "Video Technology",        "Video Painter (PAL)",               MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
// Master Video Painter goes here
