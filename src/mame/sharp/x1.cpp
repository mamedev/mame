// license:LGPL-2.1+
// copyright-holders: Angelo Salese, Barry Rodewald
// thanks-to: Dirk Best
/************************************************************************************************

Sharp X1 (c) 1983 Sharp Corporation

References:
- http://www.x1center.org/
- http://ematei.s602.xrea.com/kenkyu/x1syasin.htm
- http://www2s.biglobe.ne.jp/~ITTO/x1/x1menu.html
- https://eaw.app/sharpx1-manuals/
- https://www.leadedsolder.com/tag/sharp-x1-turbo
- http://takeda-toshiya.my.coocan.jp/x1twin/index.html
- https://monochromeeffect.org/JVCC/2019/05/01/sharp-x1-turbo-z/
- https://monochromeeffect.org/JVCC/2019/06/24/sharp-x1-d/

TODO:
- clean-up QA, is ugly and outdated;
- clean-ups, split components into devices if necessary and maybe separate turbo/turboz features into specific file(s);
- refactor base video into a true scanline renderer, expect it to break 6845 drawing delegation support badly;
- support extended x1turboz video features (need more test cases?);
- Rewrite keyboard input hook-up and decap/dump the keyboard MCU if possible;
- Fix the 0xe80/0xe83 kanji ROM readback;
- x1turbo keyboard inputs are currently broken, use x1turbo40 for now;
- Hook-up remaining .tap image formats;
- Implement APSS tape commands;
- Sort out / redump the BIOS gfx roms, and understand if TurboZ really have same BIOS as
    vanilla Turbo like Jp emulators seems to suggest;
- X1Turbo: Implement SIO.
- Implement true 400 lines mode (i.e. Chatnoir no Mahjong v2.1, Casablanca)
- Implement SASI HDD interface;
- Driver Configuration switches:
    - OPN for X1
    - EMM, and hook-up for X1 too
    - RAM size for EMM
    - specific x1turboz features?

per-game/program specific TODO (to be moved to hash file):
- Might & Magic: uses 0xe80-3 kanji ports, should be a good test case for that;
- Saziri: doesn't re-initialize the tilemap attribute vram when you start a play, making it to have missing colors if you don't start a play in time;
- Super Billiards (X1 Pack 14): has a slight PCG timing bug, that happens randomly;
- Trivia-Q: dunno what to do on the selection screen, missing inputs?
- Ys 2: crashes after the disclaimer screen;
- Ys 3: missing user disk, to hack it (and play with x1turboz features): bp 81ca,pc += 2
- Ys 3: never uploads a valid 4096 palette, probably related to the fact that we don't have an user disk

Notes:
- An interesting feature of the Sharp X-1 is the extended i/o bank. When the ppi port c bit 5
    does a 1->0 transition, any write to the i/o space accesses 2 or 3 banks gradients of the bitmap RAM
    with a single write (generally used for layer clearances and bitmap-style sprites).
    Any i/o read disables this extended bitmap ram.
- ROM format header (TODO: document, it's a full on expansion slot *inside* the machine, BASIC variant known to exist):
    [0x00] ROM identifier, must be 0x01 / SOH
    [0x01 to 0x0d] ROM header, i.e. title for the loader
    [0x12 -  0x13] initial copy size
    [0x14 -  0x15] destination address start address
    [0x16 to 0x17] start boot jump vector
    [0x1d to 0x1f] start boot data vector
- Maidum: you need to load BOTH disk with write protection disabled, otherwise it refuses to run. (btanb)
- Marvelous: needs write protection disabled (btanb)
- Chack'n Pop: to load this game, do a files command on the "Jodan Dos" prompt then move the cursor up at the "Chack'n Pop" file.
    Substitute bin with load and press enter. Finally, do a run once that it loaded correctly.
- Faeries Residence: to load this game, put a basic v2.0 in drive 0, then execute a NEWON command. Load game disks into drive 0 and 1 then
    type run"START" (case sensitive)
- POPLEMON: same as above, but you need to type run"POP"

=================================================================================================

    X1 (CZ-800C) - November, 1982
     * CPU: z80A @ 4MHz, 80C49 x 2 (one for key scan, the other for TV & Cas Ctrl)
     * ROM: IPL (4KB) + chargen (2KB)
     * RAM: Main memory (64KB) + VRAM (4KB) + RAM for PCG (6KB) + GRAM (48KB, Option)
     * Text Mode: 80x25 or 40x25
     * Graphic Mode: 640x200 or 320x200, 8 colors
     * Sound: PSG 8 octave
     * I/O Ports: Centronic ports, 2 Joystick ports, Cassette port (2700 baud)

    X1C (CZ-801C) - October, 1983
     * same but only 48KB GRAM

    X1D (CZ-802C) - October, 1983
     * same as X1C but with a 3" floppy drive (notice: 3" not 3" 1/2!!)

    X1Cs (CZ-803C) - June, 1984
     * two expansion I/O ports

    X1Ck (CZ-804C) - June, 1984
     * same as X1Cs
     * ROM: IPL (4KB) + chargen (2KB) + Kanji 1st level

    X1F Model 10 (CZ-811C) - July, 1985
     * Re-designed
     * ROM: IPL (4KB) + chargen (2KB)

    X1F Model 20 (CZ-812C) - July, 1985
     * Re-designed (same as Model 10)
     * ROM: IPL (4KB) + chargen (2KB) + Kanji
     * Built Tape drive plus a 5" floppy drive was available

    X1G Model 10 (CZ-820C) - July, 1986
     * Re-designed again
     * ROM: IPL (4KB) + chargen (2KB)

    X1G Model 30 (CZ-822C) - July, 1986
     * Re-designed again (same as Model 10)
     * ROM: IPL (4KB) + chargen (2KB) + Kanji
     * Built Tape drive plus a 5" floppy drive was available

    X1twin (CZ-830C) - December, 1986
     * Re-designed again (same as Model 10)
     * ROM: IPL (4KB) + chargen (2KB) + Kanji
     * Built Tape drive plus a 5" floppy drive was available
     * It contains a PC-Engine

    =============  X1 Turbo series  =============

    X1turbo Model 30 (CZ-852C) - October, 1984
     * CPU: z80A @ 4MHz, 80C49 x 2 (one for key scan, the other for TV & Cas Ctrl)
     * ROM: IPL (32KB) + chargen (8KB) + Kanji (128KB)
     * RAM: Main memory (64KB) + VRAM (6KB) + RAM for PCG (6KB) + GRAM (96KB)
     * Text Mode: 80xCh or 40xCh with Ch = 10, 12, 20, 25 (same for Japanese display)
     * Graphic Mode: 640x200 or 320x200, 8 colors
     * Sound: PSG 8 octave
     * I/O Ports: Centronic ports, 2 Joystick ports, built-in Cassette interface,
        2 Floppy drive for 5" disks, two expansion I/O ports

    X1turbo Model 20 (CZ-851C) - October, 1984
     * same as Model 30, but only 1 Floppy drive is included

    X1turbo Model 10 (CZ-850C) - October, 1984
     * same as Model 30, but Floppy drive is optional and GRAM is 48KB (it can
        be expanded to 96KB however)

    X1turbo Model 40 (CZ-862C) - July, 1985
     * same as Model 30, but uses tv screen (you could watch television with this)

    X1turboII (CZ-856C) - November, 1985
     * same as Model 30, but restyled, cheaper and sold with utility software

    X1turboIII (CZ-870C) - November, 1986
     * with two High Density Floppy driver

    X1turboZ (CZ-880C) - December, 1986
     * CPU: z80A @ 4MHz, 80C49 x 2 (one for key scan, the other for TV & Cas Ctrl)
     * ROM: IPL (32KB) + chargen (8KB) + Kanji 1st & 2nd level
     * RAM: Main memory (64KB) + VRAM (6KB) + RAM for PCG (6KB) + GRAM (96KB)
     * Text Mode: 80xCh or 40xCh with Ch = 10, 12, 20, 25 (same for Japanese display)
     * Graphic Mode: 640x200 or 320x200, 8 colors [in compatibility mode],
        640x400, 8 colors (out of 4096); 320x400, 64 colors (out of 4096);
        320x200, 4096 colors [in multimode],
     * Sound: PSG 8 octave + FM 8 octave
     * I/O Ports: Centronic ports, 2 Joystick ports, built-in Cassette interface,
        2 Floppy drive for HD 5" disks, two expansion I/O ports

    X1turboZII (CZ-881C) - December, 1987
     * same as turboZ, but added 64KB expansion RAM

    X1turboZIII (CZ-888C) - December, 1988
     * same as turboZII, but no more built-in cassette drive

    BASIC has to be loaded from external media (tape or disk), the
    computer only has an Initial Program Loader (IPL)

=================================================================================================

    x1turbo specs (courtesy of Yasuhiro Ogawa):

    upper board: Z80A-CPU
                 Z80A-DMA
                 Z80A-SIO(O)
                 Z80A-CTC
                 uPD8255AC
                 LH5357(28pin mask ROM. for IPL?)
                 YM2149F
                 16.000MHz(X1)

    lower board: IX0526CE(HN61364) (28pin mask ROM. for ANK font?)
                 MB83256x4 (Kanji ROMs)
                 HD46505SP (VDP)
                 M80C49-277 (MCU)
                 uPD8255AC
                 uPD1990 (RTC) + battery
                 6.000MHz(X2)
                 42.9545MHz(X3)

    FDD I/O board: MB8877A (FDC)
                   MB4107 (VFO)

    RAM banks:
    upper board: MB8265A-15 x8 (main memory)
    lower board: MB8416A-12 x3 (VRAM)
                 MB8416A-15 x3 (PCG RAM)
                 MB81416-10 x12 (GRAM)

************************************************************************************************/

#include "emu.h"
#include "x1.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "formats/2d_dsk.h"


constexpr XTAL MAIN_CLOCK   = 16_MHz_XTAL;
constexpr XTAL VDP_CLOCK    = 42.954'545_MHz_XTAL;
//constexpr XTAL MCU_CLOCK    = 6_MHz_XTAL;

/*************************************
 *
 *  Keyboard MCU simulation
 *
 *************************************/


uint16_t x1_state::check_keyboard_press()
{
	static const char *const portnames[3] = { "key1","key2","key3" };
	uint8_t keymod = ioport("key_modifiers")->read() & 0x1f;
	uint32_t pad = ioport("tenkey")->read();
	uint32_t f_key = ioport("f_keys")->read();

	static const uint8_t kanatable[52][3] = {
		// normal, kana, kana + shift
		{0x2c,0xc8,0xa4}, // , / ne / japanese comma
		{0x2d,0xce,0x00}, // - / ho
		{0x2e,0xd9,0xa1}, // . / ru / japanese period
		{0x2f,0xd2,0xa5}, // / / me / nakaguro
		{0x30,0xdc,0xa6}, // 0 / wa / wo
		{0x31,0xc7,0x00}, // 1 / nu
		{0x32,0xcc,0x00}, // 2 / fu
		{0x33,0xb1,0xa7}, // 3 / a / small a
		{0x34,0xb3,0xa9}, // 4 / u / small u
		{0x35,0xb4,0xaa}, // 5 / e / small e
		{0x36,0xb5,0xab}, // 6 / o / small o
		{0x37,0xd4,0xac}, // 7 / ya / small ya
		{0x38,0xd5,0xad}, // 8 / yu / small yu
		{0x39,0xd6,0xae}, // 9 / yo / small yo
		{0x3a,0xb9,0x00}, // : / ke
		{0x3b,0xda,0x00}, // ; / re
		{0x3c,0x00,0x00},
		{0x3d,0x00,0x00},
		{0x3e,0x00,0x00},
		{0x3f,0x00,0x00},
		{0x40,0xde,0x00}, // @ / dakuten
		{0x41,0xc1,0x00}, // A / chi
		{0x42,0xba,0x00}, // B / ko
		{0x43,0xbf,0x00}, // C / so
		{0x44,0xbc,0x00}, // D / shi
		{0x45,0xb2,0xa8}, // E / i / small i
		{0x46,0xca,0x00}, // F / ha
		{0x47,0xb7,0x00}, // G / ki
		{0x48,0xb8,0x00}, // H / ku
		{0x49,0xc6,0x00}, // I / ni
		{0x4a,0xcf,0x00}, // J / ma
		{0x4b,0xc9,0x00}, // K / no
		{0x4c,0xd8,0x00}, // L / ri
		{0x4d,0xd3,0x00}, // M / mo
		{0x4e,0xd0,0x00}, // N / mi
		{0x4f,0xd7,0x00}, // O / ra
		{0x50,0xbe,0x00}, // P / se
		{0x51,0xc0,0x00}, // Q / ta
		{0x52,0xbd,0x00}, // R / su
		{0x53,0xc4,0x00}, // S / to
		{0x54,0xb6,0x00}, // T / ka
		{0x55,0xc5,0x00}, // U / na
		{0x56,0xcb,0x00}, // V / hi
		{0x57,0xc3,0x00}, // W / te
		{0x58,0xbb,0x00}, // X / sa
		{0x59,0xdd,0x00}, // Y / n
		{0x5a,0xc2,0xaf}, // Z / tsu / small tsu
		{0x5b,0xdf,0xa2}, // [ / handakuten / opening quotation mark
		{0x5c,0xb0,0x00}, // yen symbol / long vowel mark
		{0x5d,0xd1,0xa3}, // ] / mu / closing quotation mark
		{0x5e,0xcd,0x00}, // ^ / he
		{0x5f,0xdb,0x00}  // _ / ro
	};

	for(u8 port_i=0; port_i<3; port_i++)
	{
		for(u8 i=0; i<32; i++)
		{
			u8 scancode = port_i * 32 + i;
			if(BIT(ioport(portnames[port_i])->read(), i))
			{
				switch (keymod & 6)
				{
					case 0: // kana on, shift on
						if (scancode >= 0x2c && scancode <= 0x5f)
							if (kanatable[scancode - 0x2c][2])
								scancode = kanatable[scancode - 0x2c][2];
						break;

					case 2: // kana on, shift off
						if (scancode >= 0x2c && scancode <= 0x5f)
							if (kanatable[scancode - 0x2c][1])
								scancode = kanatable[scancode - 0x2c][1];
						break;

					case 4: // kana off, shift on
						if (scancode == 0x40 || (scancode >= 0x5b && scancode <= 0x5f))
							scancode += 0x20;
						else // numbers, special chars
						if ((scancode >= 0x2c && scancode <= 0x3b))
							scancode ^= 0x10;
						break;

					default: // kana off, shift off
						// Control key
						if (scancode >= 0x41 && scancode <= 0x5f)
							if (!BIT(keymod, 0))
								scancode -= 0x40;
						// If nothing pressed, default to lower case
						if (scancode >= 0x41 && scancode <= 0x5a)
							scancode += 0x20;
						break;
				}

				if(!BIT(keymod, 3)) // capslock
					if ((scancode >= 0x41 && scancode <= 0x5a) || (scancode >= 0x61 && scancode <= 0x7a))
						scancode ^= 0x20;

				if(!BIT(keymod, 4)) // graph on
					scancode |= 0x80;

				return scancode;
			}
		}
	}

	// check numpad
	for(u8 i=0; i<10; i++)
		if(BIT(pad, i))
			return i + 0x120;

	// check function keys
	for(u8 i=0; i<5; i++)
		if(BIT(f_key, i))
			return i + 0x171 + (BIT(keymod, 1) ? 0 : 5);

	return 0;
}

uint8_t x1_state::check_keyboard_shift()
{
	uint8_t val = 0xe0;
	/*
	all of those are active low
	x--- ---- TEN: Numpad, Function key, special input key
	-x-- ---- KIN: Valid key
	--x- ---- REP: Key repeat
	---x ---- GRAPH key ON
	---- x--- CAPS lock ON
	---- -x-- KANA lock ON
	---- --x- SHIFT ON
	---- ---x CTRL ON
	*/

	val |= ioport("key_modifiers")->read() & 0x1f;

	if(check_keyboard_press() != 0)
		val &= ~0x40;

	if(check_keyboard_press() & 0x100) //function keys
		val &= ~0x80;

	return val;
}

uint8_t x1_state::get_game_key(uint8_t port)
{
	// key status returned by sub CPU function 0xE3.
	// in order from bit 7 to 0:
	// port 0: Q,W,E,A,D,Z,X,C
	// port 1: numpad 7,4,1,8,2,9,6,3
	// port 2: ESC,1,[-],[+],[*],TAB,SPC,RET ([] = numpad)
	// bits are active high
	uint8_t ret = 0;

	if (port == 0)
	{
		uint32_t key3 = ioport("key3")->read();
		if(key3 & 0x00020000) ret |= 0x80;  // Q
		if(key3 & 0x00800000) ret |= 0x40;  // W
		if(key3 & 0x00000020) ret |= 0x20;  // E
		if(key3 & 0x00000002) ret |= 0x10;  // A
		if(key3 & 0x00000010) ret |= 0x08;  // D
		if(key3 & 0x04000000) ret |= 0x04;  // Z
		if(key3 & 0x01000000) ret |= 0x02;  // X
		if(key3 & 0x00000008) ret |= 0x01;  // C
	}
	else
	if (port == 1)
	{
		uint32_t pad = ioport("tenkey")->read();
		if(pad & 0x00000080) ret |= 0x80;  // Tenkey 7
		if(pad & 0x00000010) ret |= 0x40;  // Tenkey 4
		if(pad & 0x00000002) ret |= 0x20;  // Tenkey 1
		if(pad & 0x00000100) ret |= 0x10;  // Tenkey 8
		if(pad & 0x00000004) ret |= 0x08;  // Tenkey 2
		if(pad & 0x00000200) ret |= 0x04;  // Tenkey 9
		if(pad & 0x00000040) ret |= 0x02;  // Tenkey 6
		if(pad & 0x00000008) ret |= 0x01;  // Tenkey 3
	}
	else
	if (port == 2)
	{
		uint32_t key1 = ioport("key1")->read();
		uint32_t key2 = ioport("key2")->read();
		uint32_t pad = ioport("tenkey")->read();
		if(key1 & 0x08000000) ret |= 0x80;  // ESC
		if(key2 & 0x00020000) ret |= 0x40;  // 1
		if(pad & 0x00000400) ret |= 0x20;  // Tenkey -
		if(pad & 0x00000800) ret |= 0x10;  // Tenkey +
		if(pad & 0x00001000) ret |= 0x08;  // Tenkey *
		if(key1 & 0x00000200) ret |= 0x04;  // TAB
		if(key2 & 0x00000001) ret |= 0x02;  // SPC
		if(key1 & 0x00002000) ret |= 0x01;  // RET
	}

	return ret;
}

uint8_t x1_state::sub_io_r()
{
	uint8_t ret,bus_res;

	/* Looks like that the HW retains the latest data putted on the bus here, behaviour confirmed by Rally-X */
	if(m_sub_obf)
	{
		bus_res = m_sub_val[m_key_i];
		/* FIXME: likely to be different here. */
		m_key_i++;
		if(m_key_i >= 2)
			m_key_i = 0;

		return bus_res;
	}

#if 0
	if(key_flag == 1)
	{
		key_flag = 0;
		return 0x82; //TODO: this is for shift/ctrl/kana lock etc.
	}
#endif

	m_sub_cmd_length--;
	m_sub_obf = (m_sub_cmd_length) ? 0x00 : 0x20;

	ret = m_sub_val[m_sub_val_ptr];

	m_sub_val_ptr++;
	if(m_sub_cmd_length <= 0)
		m_sub_val_ptr = 0;

	return ret;
}

void x1_state::cmt_command( uint8_t cmd )
{
	// CMT deck control command (E9 xx)
	// E9 00 - Eject
	// E9 01 - Stop
	// E9 02 - Play
	// E9 03 - Fast Forward
	// E9 04 - Rewind
	// E9 05 - APSS Fast Forward
	// E9 06 - APSS Rewind
	// E9 0A - Record
	/*
	APSS is a Sharp invention and stands for Automatic Program Search System, it scans the tape for silent parts that are bigger than 4 seconds.
	It's basically used for audio tapes in order to jump over the next/previous "track".
	*/
	m_cmt_current_cmd = cmd;

	if(m_cassette->get_image() == nullptr) //avoid a crash if a disk game tries to access this
		return;

	switch(cmd)
	{
		case 0x01:  // Stop
			m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
			m_cassette->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
			m_cmt_test = 1;
			popmessage("CMT: Stop");
			break;
		case 0x02:  // Play
			m_cassette->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
			m_cassette->change_state(CASSETTE_PLAY,CASSETTE_MASK_UISTATE);
			popmessage("CMT: Play");
			break;
		case 0x03:  // Fast Forward
			m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
			m_cassette->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
			popmessage("CMT: Fast Forward");
			break;
		case 0x04:  // Rewind
			m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
			m_cassette->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
			popmessage("CMT: Rewind");
			break;
		case 0x05:  // APSS Fast Forward
			m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
			m_cassette->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
			popmessage("CMT: APSS Fast Forward");
			break;
		case 0x06:  // APSS Rewind
			m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
			m_cassette->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
			popmessage("CMT: APSS Rewind");
			break;
		case 0x0a:  // Record
			m_cassette->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
			m_cassette->change_state(CASSETTE_RECORD,CASSETTE_MASK_UISTATE);
			popmessage("CMT: Record");
			break;
		default:
			logerror("Unimplemented or invalid CMT command (0x%02x)\n",cmd);
	}
	logerror("CMT: Command 0xe9-0x%02x received.\n",cmd);
}

TIMER_DEVICE_CALLBACK_MEMBER(x1_state::cmt_seek_cb)
{
	if(m_cassette->get_image() == nullptr) //avoid a crash if a disk game tries to access this
		return;

	switch(m_cmt_current_cmd)
	{
		case 0x03:
		case 0x05:  // Fast Forwarding tape
			m_cassette->seek(1,SEEK_CUR);
			if(m_cassette->get_position() >= m_cassette->get_length())  // at end?
				cmt_command(0x01);  // Stop tape
			break;
		case 0x04:
		case 0x06:  // Rewinding tape
			m_cassette->seek(-1,SEEK_CUR);
			if(m_cassette->get_position() <= 0) // at beginning?
				cmt_command(0x01);  // Stop tape
			break;
	}
}

void x1_state::sub_io_w(uint8_t data)
{
	/* sub-routine at $10e sends to these sub-routines when a keyboard input is triggered:
	 $17a -> floppy
	 $094 -> ROM
	 $0c0 -> timer
	 $052 -> cmt
	 $0f5 -> reload sub-routine? */

	if(m_sub_cmd == 0xe4)
	{
		m_key_irq_vector = data;
		logerror("Key vector set to 0x%02x\n",data);
		data = 0;
	}

	if(m_sub_cmd == 0xe9)
	{
		cmt_command(data);
		data = 0;
	}

	if((data & 0xf0) == 0xd0) //reads here tv recording timer data. (Timer set (0xd0) / Timer readout (0xd8))
	{
		/*
		    xx-- ---- mode
		    --xx xxxx interval
		*/
		m_sub_val[0] = 0;
		/*
		    xxxx xxxx command code:
		    00 timer disabled
		    01 TV command
		    10 interrupt
		    11 Cassette deck
		*/
		m_sub_val[1] = 0;
		/*
		    ---x xxxx minute
		*/
		m_sub_val[2] = 0;
		/*
		    ---- xxxx hour
		*/
		m_sub_val[3] = 0;
		/*
		    xxxx ---- month
		    ---- -xxx day of the week
		*/
		m_sub_val[4] = 0;
		/*
		    --xx xxxx day
		*/
		m_sub_val[5] = 0;
		m_sub_cmd_length = 6;
	}

	switch(data)
	{
		case 0xe3: //game key obtaining
			m_sub_cmd_length = 3;
			m_sub_val[0] = get_game_key(0);
			m_sub_val[1] = get_game_key(1);
			m_sub_val[2] = get_game_key(2);
			break;
		case 0xe4: //irq vector setting
			break;
		//case 0xe5: //timer irq clear
		//  break;
		case 0xe6: //key data readout
			m_sub_val[0] = check_keyboard_shift() & 0xff;
			m_sub_val[1] = check_keyboard_press() & 0xff;
			m_sub_cmd_length = 2;
			break;
//      case 0xe7: // TV Control
//          break;
		case 0xe8: // TV Control read-out
			m_sub_val[0] = m_sub_cmd;
			m_sub_cmd_length = 1;
			break;
		case 0xe9: // CMT Control
			break;
		case 0xea:  // CMT Control status
			m_sub_val[0] = m_cmt_current_cmd;
			m_sub_cmd_length = 1;
			logerror("CMT: Command 0xEA received, returning 0x%02x.\n",m_sub_val[0]);
			break;
		case 0xeb:  // CMT Tape status
					// bit 0 = tape end (0=end of tape)
					// bit 1 = tape inserted
					// bit 2 = record status (1=OK, 0=write protect)
			m_sub_val[0] = 0x05;
			if(m_cassette->get_image() != nullptr)
				m_sub_val[0] |= 0x02;
			m_sub_cmd_length = 1;
			logerror("CMT: Command 0xEB received, returning 0x%02x.\n",m_sub_val[0]);
			break;
//      case 0xec: //set date
//          break;
		case 0xed: //get date
			m_sub_val[0] = m_rtc.day;
			m_sub_val[1] = (m_rtc.month<<4) | (m_rtc.wday & 0xf);
			m_sub_val[2] = m_rtc.year;
			m_sub_cmd_length = 3;
			break;
//      case 0xee: //set time
//          break;
		case 0xef: //get time
			m_sub_val[0] = m_rtc.hour;
			m_sub_val[1] = m_rtc.min;
			m_sub_val[2] = m_rtc.sec;
			m_sub_cmd_length = 3;
			break;
	}

	m_sub_cmd = data;

	m_sub_obf = (m_sub_cmd_length) ? 0x00 : 0x20;

	if(data != 0xe6)
		logerror("SUB: Command byte 0x%02x\n",data);
}

/*************************************
 *
 *  ROM Image / Banking Handling
 *
 *************************************/


uint8_t x1_state::rom_r()
{
//  logerror("%06x\n",m_rom_index[0]<<16|m_rom_index[1]<<8|m_rom_index[2]<<0);
	if (m_cart->exists())
		return m_cart->read_rom((m_rom_index[0] << 16) | (m_rom_index[1] << 8) | (m_rom_index[2] << 0));
	else
		return 0;
}

void x1_state::rom_w(offs_t offset, uint8_t data)
{
	m_rom_index[offset] = data;
}

void x1_state::rom_bank_0_w(uint8_t data)
{
	m_ram_bank = 0x10;
}

void x1_state::rom_bank_1_w(uint8_t data)
{
	m_ram_bank = 0x00;
}

/*************************************
 *
 *  MB8877A FDC (wd17XX compatible)
 *
 *************************************/

uint8_t x1_state::fdc_r(offs_t offset)
{
	//uint8_t ret = 0;

	switch(offset+0xff8)
	{
		case 0x0ff8:
			return m_fdc->status_r();
		case 0x0ff9:
			return m_fdc->track_r();
		case 0x0ffa:
			return m_fdc->sector_r();
		case 0x0ffb:
			return m_fdc->data_r();
		case 0x0ffc:
			if (!machine().side_effects_disabled())
			{
				logerror("FDC: read FM type\n");
				m_fdc->dden_w(1);
			}
			return 0xff;
		case 0x0ffd:
			if (!machine().side_effects_disabled())
			{
				logerror("FDC: read MFM type\n");
				m_fdc->dden_w(0);
			}
			return 0xff;
		case 0x0ffe:
			if (!machine().side_effects_disabled())
				logerror("FDC: read 1.6M type\n");
			return 0xff;
		case 0x0fff:
			if (!machine().side_effects_disabled())
				logerror("FDC: switching between 500k/1M\n");
			return 0xff;
	}

	return 0x00;
}

void x1_state::fdc_w(offs_t offset, uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	switch(offset+0xff8)
	{
		case 0x0ff8:
			m_fdc->cmd_w(data);
			break;
		case 0x0ff9:
			m_fdc->track_w(data);
			break;
		case 0x0ffa:
			m_fdc->sector_w(data);
			break;
		case 0x0ffb:
			m_fdc->data_w(data);
			break;

		case 0x0ffc:
			floppy = m_floppy[data & 0x03]->get_device();

			m_fdc->set_floppy(floppy);

			if (floppy)
			{
				floppy->ss_w(BIT(data, 4));
				if(BIT(m_fdc_ctrl, 7) && !BIT(data, 7))
					m_motor_timer->adjust(attotime::from_seconds(1.2));
				else if(BIT(data, 7))
					floppy->mon_w(0);
			}
			m_fdc_ctrl = data;
			break;

		case 0x0ffd:
		case 0x0ffe:
		case 0x0fff:
			logerror("FDC: undefined write to %04x = %02x\n",offset+0xff8,data);
			break;
	}
}

TIMER_CALLBACK_MEMBER(x1_state::fdc_motor_off_cb)
{
	if(!BIT(m_fdc_ctrl, 7))
	{
		floppy_image_device *floppy = m_floppy[m_fdc_ctrl & 0x03]->get_device();
		if(floppy)
			floppy->mon_w(1);
	}
}

void x1turbo_state::fdc_drq_w(int state)
{
	m_dma->rdy_w(state ^ 1);
}

/*************************************
 *
 *  Programmable Character Generator
 *
 *************************************/

uint16_t x1_state::check_pcg_addr()
{
	if(m_avram[0x7ff] & 0x20) return 0x7ff;
	if(m_avram[0x3ff] & 0x20) return 0x3ff;
	if(m_avram[0x5ff] & 0x20) return 0x5ff;
	if(m_avram[0x1ff] & 0x20) return 0x1ff;

	return 0x3ff;
}

uint16_t x1_state::check_chr_addr()
{
	if(!(m_avram[0x7ff] & 0x20)) return 0x7ff;
	if(!(m_avram[0x3ff] & 0x20)) return 0x3ff;
	if(!(m_avram[0x5ff] & 0x20)) return 0x5ff;
	if(!(m_avram[0x1ff] & 0x20)) return 0x1ff;

	return 0x3ff;
}

uint16_t x1_state::get_pcg_addr( uint16_t width, uint8_t y_char_size )
{
	int hbeam = m_screen->hpos() >> 3;
	int vbeam = m_screen->vpos() / y_char_size;
	uint16_t pcg_offset = ((hbeam + vbeam*width) + (((m_crtc_vreg[0x0c]<<8) & 0x3f00) | (m_crtc_vreg[0x0d] & 0xff))) & 0x7ff;

	//logerror("%08x %d %d %d %d\n",(hbeam+vbeam*width),hbeam,vbeam,m_screen->vpos() & 7,width);

	return pcg_offset;
}

uint8_t x1_state::pcg_r(offs_t offset)
{
	int addr;
	int pcg_offset;
	uint8_t res;
	uint8_t *gfx_data;

	addr = (offset & 0x300) >> 8;

	if(addr == 0 && m_scrn_reg.pcg_mode) // Kanji ROM read, X1Turbo only
	{
		gfx_data = m_kanji_rom;
		pcg_offset = (m_tvram[check_chr_addr()]+(m_kvram[check_chr_addr()]<<8)) & 0xfff;
		pcg_offset*=0x20;
		pcg_offset+=(offset & 0x0f);
		pcg_offset+=(m_kvram[check_chr_addr()] & 0x40) >> 2; //left-right check

		res = gfx_data[pcg_offset];
	}
	else
	{
		uint8_t y_char_size;

		/* addr == 0 reads from the ANK rom */
		gfx_data = addr == 0 ? m_cg_rom : m_pcg_ram.get();
		y_char_size = ((m_crtc_vreg[9]+1) > 8) ? 8 : m_crtc_vreg[9]+1;
		if(y_char_size == 0) { y_char_size = 1; }
		pcg_offset = m_tvram[get_pcg_addr(m_crtc_vreg[1], y_char_size)]*8;
		pcg_offset+= m_screen->vpos() & (y_char_size-1);
		if(addr) { pcg_offset+= ((addr-1)*0x800); }
		res = gfx_data[pcg_offset];
	}

	return res;
}

void x1_state::pcg_w(offs_t offset, uint8_t data)
{
	int addr,pcg_offset;

	addr = (offset & 0x300) >> 8;

	if(addr == 0)
	{
		/* NOP */
		logerror("Warning: write to the ANK area! %04x %02x\n",offset,data);
	}
	else
	{
		if(m_scrn_reg.pcg_mode) // Hi-Speed Mode, X1Turbo only
		{
			pcg_offset = m_tvram[check_pcg_addr()]*8;
			pcg_offset+= (offset & 0xe) >> 1;
			pcg_offset+=((addr-1)*0x800);
			m_pcg_ram[pcg_offset] = data;

			pcg_offset &= 0x7ff;

			m_gfxdecode->gfx(3)->mark_dirty(pcg_offset >> 3);
		}
		else // Compatible Mode
		{
			uint8_t y_char_size;

			/* TODO: Brain Breaker doesn't work with this arrangement in high resolution mode, check out why */
			y_char_size = (m_crtc_vreg[9]+1) > 8 ? (m_crtc_vreg[9]+1)-8 : m_crtc_vreg[9]+1;
			if(y_char_size == 0) { y_char_size = 1; }
			pcg_offset = m_tvram[get_pcg_addr(m_crtc_vreg[1], y_char_size)]*8;
			pcg_offset+= m_screen->vpos() & (y_char_size-1);
			pcg_offset+= ((addr-1)*0x800);

			m_pcg_ram[pcg_offset] = data;

			pcg_offset &= 0x7ff;

			m_gfxdecode->gfx(3)->mark_dirty(pcg_offset >> 3);
		}
	}
}

/*************************************
 *
 *  Other Video-related functions
 *
 *************************************/

/* for bitmap mode */
void x1_state::set_current_palette()
{
	uint8_t addr,r,g,b;

	for(addr=0;addr<8;addr++)
	{
		r = ((m_x_r)>>(addr)) & 1;
		g = ((m_x_g)>>(addr)) & 1;
		b = ((m_x_b)>>(addr)) & 1;

		m_palette->set_pen_color(addr|8, pal1bit(r), pal1bit(g), pal1bit(b));
	}

	// TODO: disabled for now, causes issues with Thunder Force. x1fdemo changes palette dynamically during initial logo.
	//       Likely it needs a video rewrite in order to make this to work correctly.
	//  m_screen->update_partial(m_screen->vpos());
}

/* Note: docs claims that reading the palette ports makes the value to change somehow in X1 mode ...
         In 4096 color mode, it's used for reading the value back. */
void x1_state::pal_r_w(uint8_t data)
{
	m_x_r = data;
	set_current_palette();
}

void x1_state::pal_g_w(uint8_t data)
{
	m_x_g = data;
	set_current_palette();
}

void x1_state::pal_b_w(uint8_t data)
{
	m_x_b = data;
	set_current_palette();
}

void x1_state::x1turboz_4096_palette_w(offs_t offset, uint8_t data)
{
	if (m_turbo_reg.pal & 0x80) // AEN bit, Turbo Z
	{
		if (m_turbo_reg.gfx_pal & 0x80) // APEN bit
		{
			if (m_turbo_reg.gfx_pal & 0x08) // APRD bit
			{
				// TODO: writing here on APRD condition just fetch offset index that reads back on this I/O
				popmessage("APRD enabled, contact MAMEdev");
				return;
			}
			// TODO: unlike normal operation this cannot do mid-frame scanline update
			// (-> bus request signal when accessing this on non-vblank time)
			uint32_t pal_entry = ((offset & 0xff) << 4) | ((data & 0xf0) >> 4);
			// TODO: more complex condition
			if ((m_turbo_reg.pal & 0x10) == 0) // C64 bit
			{
				pal_entry &= 0xccc;
				pal_entry |= pal_entry >> 2;
			}

			m_pal_4096[pal_entry+((offset & 0x300)<<4)] = data & 0xf;

			uint8_t const r = m_pal_4096[pal_entry+(1<<12)];
			uint8_t const g = m_pal_4096[pal_entry+(2<<12)];
			uint8_t const b = m_pal_4096[pal_entry+(0<<12)];

			m_palette->set_pen_color(pal_entry+16, pal4bit(r), pal4bit(g), pal4bit(b));
		}
	}
	else //compatible mode
	{
		switch (offset & 0x0300)
		{
		case 0x0000:
			pal_b_w(data);
			break;
		case 0x0100:
			pal_r_w(data);
			break;
		case 0x0200:
			pal_g_w(data);
			break;
		}
	}
}

uint8_t x1_state::ex_gfxram_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		m_iobank->set_bank(0); // any read disables the extended mode
		return m_iobank->read8(offset);
	}
	else
	{
		return 0xff;
	}
}

void x1_state::ex_gfxram_w(offs_t offset, uint8_t data)
{
	uint8_t ex_mask;

	if     (                    offset <= 0x3fff)   { ex_mask = 7; }
	else if(offset >= 0x4000 && offset <= 0x7fff)   { ex_mask = 6; }
	else if(offset >= 0x8000 && offset <= 0xbfff)   { ex_mask = 5; }
	else                                            { ex_mask = 3; }

	uint8_t *const ptr = reinterpret_cast<uint8_t *>(m_bitmapbank->base());
	if(ex_mask & 1) ptr[(offset & 0x3fff)+0x0000] = data;
	if(ex_mask & 2) ptr[(offset & 0x3fff)+0x4000] = data;
	if(ex_mask & 4) ptr[(offset & 0x3fff)+0x8000] = data;
}

/*
    SCRN flags

    d0(01) = 0:low resolution (15KHz) 1: high resolution (24KHz)
    d1(02) = 0:1 raster / pixel       1:2 raster / pixel
    d2(04) = 0:8 rasters / CHR        1:16 rasters / CHR
    d3(08) = 0:bank 0                 0:bank 1    <- display
    d4(10) = 0:bank 0                 0:bank 1    <- access
    d5(20) = 0:compatibility          1:high speed  <- define PCG mode
    d6(40) = 0:8-raster graphics      1:16-raster graphics
    d7(80) = 0:don't display          1:display  <- underline (when 1, graphics are not displayed)
*/
void x1_state::scrn_w(uint8_t data)
{
	m_scrn_reg.video_mode = data & 0xc7;
	m_scrn_reg.pcg_mode = BIT(data, 5);
	m_bitmapbank->set_entry(BIT(data, 4));
	m_scrn_reg.disp_bank = BIT(data, 3);
	m_scrn_reg.ank_sel = BIT(data, 2);
	m_scrn_reg.v400_mode = ((data & 0x03) == 3) ? 1 : 0;

	if(data & 0x80)
		logerror("SCRN = %02x\n",data & 0x80);
	if((data & 0x03) == 1)
		logerror("SCRN sets true 400 lines mode\n");
}

void x1_state::pri_w(uint8_t data)
{
	m_scrn_reg.pri = data;
//  logerror("PRI = %02x\n",data);
}

uint8_t x1_state::x1turboz_blackclip_r()
{
	/*  TODO: this returns only on x1turboz */
	return m_scrn_reg.blackclip;
}

void x1_state::x1turbo_blackclip_w(uint8_t data)
{
	/*
	-x-- ---- replace blanking duration with black
	--x- ---- replace bitmap palette 1 with black
	---x ---- replace bitmap palette 0 with black
	---- x--- enable text blackclip
	---- -xxx palette color number for text black
	*/
	m_scrn_reg.blackclip = data;
	if(data & 0x40)
		logerror("Blackclip data access %02x\n",data);
}

uint8_t x1_state::x1turbo_pal_r()
{
	return m_turbo_reg.pal;
}

uint8_t x1_state::x1turbo_txpal_r(offs_t offset)
{
	return m_turbo_reg.txt_pal[offset];
}

uint8_t x1_state::x1turbo_txdisp_r()
{
	return m_turbo_reg.txt_disp;
}

uint8_t x1_state::x1turbo_gfxpal_r()
{
	return m_turbo_reg.gfx_pal;
}

void x1_state::x1turbo_pal_w(uint8_t data)
{
	logerror("TURBO PAL %02x\n",data);
	m_turbo_reg.pal = data;
}

void x1_state::x1turbo_txpal_w(offs_t offset, uint8_t data)
{
	int r,g,b;

	logerror("TURBO TEXT PAL %02x %02x\n",data,offset);
	m_turbo_reg.txt_pal[offset] = data;

	if(m_turbo_reg.pal & 0x80)
	{
		r = (data & 0x0c) >> 2;
		g = (data & 0x30) >> 4;
		b = (data & 0x03) >> 0;

		m_palette->set_pen_color(offset, pal2bit(r), pal2bit(g), pal2bit(b));
	}
}

void x1_state::x1turbo_txdisp_w(uint8_t data)
{
	logerror("TURBO TEXT DISPLAY %02x\n",data);
	m_turbo_reg.txt_disp = data;
}

void x1_state::x1turbo_gfxpal_w(uint8_t data)
{
	logerror("TURBO GFX PAL %02x\n",data);
	m_turbo_reg.gfx_pal = data;
}


/*
 *  FIXME: bit-wise this doesn't make any sense, I guess that it uses the lv 2 kanji roms
 *         Test cases for this port so far are Hyper Olympics '84 disk version and Might & Magic.
 */
uint16_t x1_state::jis_convert(int kanji_addr)
{
	if(kanji_addr >= 0x0e00 && kanji_addr <= 0x0e9f) { kanji_addr -= 0x0e00; kanji_addr &= 0x0ff; return ((0x0e0) + (kanji_addr >> 3)) << 4; } // numbers
	if(kanji_addr >= 0x0f00 && kanji_addr <= 0x109f) { kanji_addr -= 0x0f00; kanji_addr &= 0x1ff; return ((0x4c0) + (kanji_addr >> 3)) << 4; } // lower case chars
	if(kanji_addr >= 0x1100 && kanji_addr <= 0x129f) { kanji_addr -= 0x1100; kanji_addr &= 0x1ff; return ((0x2c0) + (kanji_addr >> 3)) << 4; } // upper case chars
	if(kanji_addr >= 0x0100 && kanji_addr <= 0x01ff) { kanji_addr -= 0x0100; kanji_addr &= 0x0ff; return ((0x040) + (kanji_addr >> 3)) << 4; } // grammar symbols
	if(kanji_addr >= 0x0500 && kanji_addr <= 0x06ff) { kanji_addr -= 0x0500; kanji_addr &= 0x1ff; return ((0x240) + (kanji_addr >> 3)) << 4; } // math symbols
	if(kanji_addr >= 0x0300 && kanji_addr <= 0x04ff) { kanji_addr -= 0x0300; kanji_addr &= 0x1ff; return ((0x440) + (kanji_addr >> 3)) << 4; } // parentesis

	if(kanji_addr != 0x0720 && kanji_addr != 0x0730)
		logerror("%08x\n",kanji_addr);

	return 0x0000;
}

uint8_t x1_state::kanji_r(offs_t offset)
{
	uint8_t res;

	res = m_kanji_rom[jis_convert(m_kanji_addr & 0xfff0)+(offset*0x10)+(m_kanji_addr & 0xf)];

	if(offset == 1)
		m_kanji_addr_latch++;

	return res;
}

void x1_state::kanji_w(offs_t offset, uint8_t data)
{
//  if(offset < 2)

	switch(offset)
	{
		case 0: m_kanji_addr_latch = (data & 0xff)|(m_kanji_addr_latch&0xff00); break;
		case 1: m_kanji_addr_latch = (data<<8)|(m_kanji_addr_latch&0x00ff);
			//if(m_kanji_addr_latch != 0x720 && m_kanji_addr_latch != 0x730)
			//  logerror("%08x\n",m_kanji_addr_latch);
			break;
		case 2:
		{
			/* 0 -> selects Expanded EEPROM */
			/* 1 -> selects Kanji ROM */
			/* 0 -> 1 -> latches Kanji ROM data */

			if(((m_kanji_eksel & 1) == 0) && ((data & 1) == 1))
			{
				m_kanji_addr = (m_kanji_addr_latch);
				//m_kanji_addr &= 0x3fff; //<- temp kludge until the rom is redumped.
				//logerror("%08x\n",m_kanji_addr);
				//m_kanji_addr+= m_kanji_count;
			}
			m_kanji_eksel = data & 1;
		}
		break;
	}
}

uint8_t x1_state::emm_r(offs_t offset)
{
	uint8_t res;

	if(offset & ~3)
	{
		logerror("Warning: read EMM BASIC area [%02x]\n",offset & 0xff);
		return 0xff;
	}

	if(offset != 3)
		logerror("Warning: read EMM address [%02x]\n",offset);

	res = 0xff;

	if(offset == 3)
	{
		res = m_emm_ram[m_emm_addr];
		m_emm_addr++;
	}

	return res;
}

void x1_state::emm_w(offs_t offset, uint8_t data)
{
	if(offset & ~3)
	{
		logerror("Warning: write EMM BASIC area [%02x] %02x\n",offset & 0xff,data);
		return;
	}

	switch(offset)
	{
		case 0: m_emm_addr = (m_emm_addr & 0xffff00) | (data & 0xff); break;
		case 1: m_emm_addr = (m_emm_addr & 0xff00ff) | (data << 8);   break;
		case 2: m_emm_addr = (m_emm_addr & 0x00ffff) | (data << 16);  break; //TODO: this has a max size limit, check exactly how much
		case 3:
			m_emm_ram[m_emm_addr] = data;
			m_emm_addr++;
			break;
	}
}

/*
    CZ-141SF, CZ-127MF, X1turboZII, X1turboZ3 boards
*/
uint8_t x1_state::x1turbo_bank_r()
{
//  logerror("BANK access read\n");
	return m_ex_bank & 0x3f;
}

void x1_state::x1turbo_bank_w(uint8_t data)
{
	//uint8_t *RAM = memregion("x1_cpu")->base();
	/*
	--x- ---- BML5: latch bit (doesn't have any real function)
	---x ---- BMCS: select bank RAM, active low
	---- xxxx BMNO: Bank memory ID
	*/

	m_ex_bank = data & 0x3f;
//  logerror("BANK access write %02x\n",data);
}

/* TODO: waitstate penalties */
uint8_t x1_state::mem_r(offs_t offset)
{
	if((offset & 0x8000) == 0 && (m_ram_bank == 0))
	{
		return m_ipl_rom[offset]; //ROM
	}

	return m_work_ram[offset]; //RAM
}

void x1_state::mem_w(offs_t offset, uint8_t data)
{
	m_work_ram[offset] = data; //RAM
}

uint8_t x1turbo_state::x1turbo_mem_r(offs_t offset)
{
	if((m_ex_bank & 0x10) == 0)
		return m_work_ram[offset+((m_ex_bank & 0xf)*0x10000)];

	return mem_r(offset);
}

void x1turbo_state::x1turbo_mem_w(offs_t offset, uint8_t data)
{
	if((m_ex_bank & 0x10) == 0)
		m_work_ram[offset+((m_ex_bank & 0xf)*0x10000)] = data; //RAM
	else
		mem_w(offset,data);
}

/*************************************
 *
 *  Memory maps
 *
 *************************************/

void x1_state::x1_io_banks_common(address_map &map)
{
	map.unmap_value_high();

	map(0x0e00, 0x0e02).w(FUNC(x1_state::rom_w));
	map(0x0e03, 0x0e03).r(FUNC(x1_state::rom_r));

	map(0x0ff8, 0x0fff).rw(FUNC(x1_state::fdc_r), FUNC(x1_state::fdc_w));

	map(0x1300, 0x1300).mirror(0x00ff).w(FUNC(x1_state::pri_w));
	map(0x1400, 0x17ff).rw(FUNC(x1_state::pcg_r), FUNC(x1_state::pcg_w));

	// TODO: verify if also readable
	map(0x1800, 0x1800).lw8(
		NAME([this](offs_t offset, u8 data) {
			m_crtc_index = data & 31;
			m_crtc->address_w(data);
		})
	);
	map(0x1801, 0x1801).lw8(
		NAME([this](offs_t offset, u8 data) {
			m_crtc_vreg[m_crtc_index] = data;
			m_crtc->register_w(data);
		})
	);

	map(0x1900, 0x1900).mirror(0x00ff).rw(FUNC(x1_state::sub_io_r), FUNC(x1_state::sub_io_w));
	map(0x1a00, 0x1a03).mirror(0x00fc).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x1b00, 0x1b00).mirror(0x00ff).rw("ay", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x1c00, 0x1c00).mirror(0x00ff).w("ay", FUNC(ay8910_device::address_w));
	map(0x1d00, 0x1d00).mirror(0x00ff).w(FUNC(x1_state::rom_bank_1_w));
	map(0x1e00, 0x1e00).mirror(0x00ff).w(FUNC(x1_state::rom_bank_0_w));

	map(0x1fa0, 0x1fa3).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x1fa8, 0x1fab).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));

	map(0x2000, 0x27ff).mirror(0x0800).ram().share("avram");

	map(0x4000, 0xffff).bankrw("bitmapbank");

	map(0x10000, 0x1ffff).rw(FUNC(x1_state::ex_gfxram_r), FUNC(x1_state::ex_gfxram_w));
}


void x1_state::x1_io_banks(address_map &map)
{
	x1_io_banks_common(map);

//  map(0x0700, 0x0701) TODO: user could install ym2151 on plain X1 too

	map(0x1000, 0x1000).mirror(0x00ff).w(FUNC(x1_state::pal_b_w));
	map(0x1100, 0x1100).mirror(0x00ff).w(FUNC(x1_state::pal_r_w));
	map(0x1200, 0x1200).mirror(0x00ff).w(FUNC(x1_state::pal_g_w));

	// Ys checks if it's a x1/x1turbo machine by checking if this area is a mirror
	map(0x3000, 0x37ff).mirror(0x0800).ram().share("tvram");
}


void x1turbo_state::x1turbo_io_banks(address_map &map)
{
	x1_io_banks_common(map);

	// TODO: a ** at head states devices used on plain X1 too, as option board

/**/map(0x0700, 0x0701).r(FUNC(x1_state::ym_r)).w("ym", FUNC(ym2151_device::write));
	// 0x704 is FM sound detection port on X1 turboZ
	map(0x0704, 0x0707).rw(m_ctc_ym, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));

/**/map(0x0800, 0x0800).w(FUNC(x1_state::color_board_w));
/**/map(0x0801, 0x0801).r(FUNC(x1_state::color_board_r));
/**/map(0x0802, 0x0802).w(FUNC(x1_state::color_board_2_w));
/**/map(0x0803, 0x0803).r(FUNC(x1_state::color_board_2_r));
/**/map(0x0a00, 0x0a07).rw(FUNC(x1_state::stereo_board_r), FUNC(x1_state::stereo_board_w));
	map(0x0b00, 0x0b00).rw(FUNC(x1_state::x1turbo_bank_r), FUNC(x1_state::x1turbo_bank_w));
/**/map(0x0c00, 0x0cff).rw(FUNC(x1_state::rs232_r), FUNC(x1_state::rs232_w));
/**/map(0x0d00, 0x0dff).rw(FUNC(x1_state::emm_r), FUNC(x1_state::emm_w));
	map(0x0e80, 0x0e81).r(FUNC(x1_state::kanji_r));
	map(0x0e80, 0x0e83).w(FUNC(x1_state::kanji_w));
/**/map(0x0fd0, 0x0fd3).rw(FUNC(x1_state::sasi_r), FUNC(x1_state::sasi_w));
/**/map(0x0fe8, 0x0fef).rw(FUNC(x1_state::fdd8_r), FUNC(x1_state::fdd8_w));

	map(0x1000, 0x12ff).w(FUNC(x1_state::x1turboz_4096_palette_w));

	map(0x1f80, 0x1f80).mirror(0x000f).rw(m_dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x1f90, 0x1f93).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x1f98, 0x1f9f).rw(FUNC(x1_state::ext_sio_ctc_r), FUNC(x1_state::ext_sio_ctc_w));
	// FIXME: identify disks with Turbo Z capabilities, potentially move to subclass
	map(0x1fb0, 0x1fb0).rw(FUNC(x1_state::x1turbo_pal_r), FUNC(x1_state::x1turbo_pal_w));       // Z only!
	map(0x1fb8, 0x1fbf).rw(FUNC(x1_state::x1turbo_txpal_r), FUNC(x1_state::x1turbo_txpal_w));   // Z only!
	map(0x1fc0, 0x1fc0).rw(FUNC(x1_state::x1turbo_txdisp_r), FUNC(x1_state::x1turbo_txdisp_w)); // Z only!
	map(0x1fc1, 0x1fc1).w(FUNC(x1_state::z_img_cap_w));                            // Z only!
	map(0x1fc2, 0x1fc2).w(FUNC(x1_state::z_mosaic_w));                             // Z only!
	map(0x1fc3, 0x1fc3).w(FUNC(x1_state::z_chroma_key_w));                         // Z only!
	map(0x1fc4, 0x1fc4).w(FUNC(x1_state::z_extra_scroll_w));                       // Z only!
	map(0x1fc5, 0x1fc5).rw(FUNC(x1_state::x1turbo_gfxpal_r), FUNC(x1_state::x1turbo_gfxpal_w)); // Z only!
//  map(0x1fd0, 0x1fdf).r(FUNC(x1_state::x1_scrn_r));                               // Z only!
	map(0x1fd0, 0x1fd0).mirror(0x000f).w(FUNC(x1_state::scrn_w));
	map(0x1fe0, 0x1fe0).rw(FUNC(x1_state::x1turboz_blackclip_r), FUNC(x1_state::x1turbo_blackclip_w));
	map(0x1ff0, 0x1ff0).portr("X1TURBO_DSW");

	map(0x3000, 0x37ff).ram().share("tvram");
	map(0x3800, 0x3fff).ram().share("kvram");
}


void x1_state::x1_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(x1_state::mem_r), FUNC(x1_state::mem_w));
}

void x1turbo_state::x1turbo_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(x1turbo_state::x1turbo_mem_r), FUNC(x1turbo_state::x1turbo_mem_w));
}

void x1_state::x1_io(address_map &map)
{
	map(0x0000, 0xffff).m(m_iobank, FUNC(address_map_bank_device::amap8));
}

/*************************************
 *
 *  PPI8255
 *
 *************************************/

uint8_t x1_state::x1_porta_r()
{
	logerror("PPI Port A read\n");
	return 0xff;
}

/*
x--- ---- V-DISP
-x-- ---- "sub cpu ibf"
--x- ---- "sub cpu obf"
---x ---- IPL RESET (0=ROM, 1=RAM)
---- x--- "busy" <- allow printer data output
---- -x-- CV-SYNC "v sync"
---- --x- READ DATA "cmt read"
---- ---x -BREAK "cmt test" (active low) <- actually this is "Sub CPU detected BREAK"
*/
uint8_t x1_state::x1_portb_r()
{
	//logerror("PPI Port B read\n");
	uint8_t res = 0;
	// TODO: ys3 is unhappy about V-DISP
	// NOTE: all PCG games actively reads from here, touching this uncarefully *will* break stuff
	int vblank_line = m_crtc_vreg[6] * (m_crtc_vreg[9]+1);
	int vsync_line = m_crtc_vreg[7] * (m_crtc_vreg[9]+1);
	m_vdisp = (m_screen->vpos() < vblank_line) ? 0x80 : 0x00;
	m_vsync = (m_screen->vpos() < vsync_line) ? 0x00 : 0x04;

//  popmessage("%d",vsync_line);
//  popmessage("%d",vblank_line);

	res = m_ram_bank | m_sub_obf | m_vsync | m_vdisp;

	if(m_cassette->input() > 0.03)
		res |= 0x02;

//  if(cassette_get_state(m_cassette) & CASSETTE_MOTOR_DISABLED)
//      res &= ~0x02;  // is zero if not playing

	// CMT test bit is set low when the CMT Stop command is issued, and becomes
	// high again when this bit is read.
	res |= 0x01;
	if(m_cmt_test != 0)
	{
		m_cmt_test = 0;
		res &= ~0x01;
	}

	return res;
}

/* I/O system port */
uint8_t x1_state::x1_portc_r()
{
	//logerror("PPI Port C read\n");
	/*
	x--- ---- Printer port output
	-x-- ---- 320 mode (r/w), divider for the pixel clock
	--x- ---- i/o mode (r/w)
	---x ---- smooth scroll enabled (?)
	---- ---x cassette output data
	*/
	return (m_io_sys & 0x9f) | m_hres_320 | ~m_io_switch;
}

void x1_state::x1_porta_w(uint8_t data)
{
	//logerror("PPI Port A write %02x\n",data);
}

void x1_state::x1_portb_w(uint8_t data)
{
	//logerror("PPI Port B write %02x\n",data);
}

void x1_state::x1_portc_w(uint8_t data)
{
	m_hres_320 = data & 0x40;

	/* set up the pixel clock according to the above divider */
	m_crtc->set_unscaled_clock(VDP_CLOCK/((m_hres_320) ? 48 : 24));

	if(!BIT(data, 5) && BIT(m_io_switch, 5))
		m_iobank->set_bank(1);

	m_io_switch = data & 0x20;
	m_io_sys = data & 0xff;

	m_cassette->output(BIT(data, 0) ? +1.0 : -1.0);
}

uint8_t x1turbo_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

void x1turbo_state::memory_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

uint8_t x1turbo_state::io_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

void x1turbo_state::io_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.write_byte(offset, data);
}

uint8_t x1_state::ym_r(offs_t offset)
{
	uint8_t result = m_ym->read(offset);
	// TODO: kingkngt x1turbo expects this to be high when in OPM mode
	// is it just covering some tight OPM busy flag?
	if (!BIT(offset, 0))
		result = (result & 0x7f) | (m_sound_sw->read() & 0x80);
	return result;
}

/*************************************
 *
 *  Placeholders
 *
 *************************************/

uint8_t x1_state::color_board_r(address_space &space)
{
	logerror("Color image board read\n");
	return space.unmap();
}

void x1_state::color_board_w(uint8_t data)
{
	logerror("Color image board write %02x\n", data);
}

uint8_t x1_state::color_board_2_r(address_space &space)
{
	logerror("Color image board 2 read\n");
	return space.unmap();
}

void x1_state::color_board_2_w(uint8_t data)
{
	logerror("Color image board 2 write %02x\n", data);
}

uint8_t x1_state::stereo_board_r(address_space &space, offs_t offset)
{
	logerror("Stereoscopic board read %04x\n", offset);
	return space.unmap();
}

void x1_state::stereo_board_w(offs_t offset, uint8_t data)
{
	logerror("Stereoscopic board write %04x %02x\n", offset, data);
}

uint8_t x1_state::rs232_r(offs_t offset)
{
	logerror("RS-232C read %04x\n", offset);
	return 0;
}

void x1_state::rs232_w(offs_t offset, uint8_t data)
{
	logerror("RS-232C write %04x %02x\n", offset, data);
}

uint8_t x1_state::sasi_r(address_space &space, offs_t offset)
{
	//logerror("SASI HDD read %04x\n",offset);
	return space.unmap();
}

void x1_state::sasi_w(offs_t offset, uint8_t data)
{
	logerror("SASI HDD write %04x %02x\n", offset, data);
}

uint8_t x1_state::fdd8_r(address_space &space, offs_t offset)
{
	logerror("8-inch FD read %04x\n", offset);
	return space.unmap();
}

void x1_state::fdd8_w(offs_t offset, uint8_t data)
{
	logerror("8-inch FD write %04x %02x\n", offset, data);
}

uint8_t x1_state::ext_sio_ctc_r(address_space &space, offs_t offset)
{
	logerror("Extended SIO/CTC read %04x\n", offset);
	return space.unmap();
}

void x1_state::ext_sio_ctc_w(offs_t offset, uint8_t data)
{
	logerror("Extended SIO/CTC write %04x %02x\n", offset, data);
}

void x1_state::z_img_cap_w(uint8_t data)
{
	logerror("Z image capturing access %02x\n", data);
}

void x1_state::z_mosaic_w(uint8_t data)
{
	logerror("Z mosaic effect access %02x\n", data);
}

void x1_state::z_chroma_key_w(uint8_t data)
{
	logerror("Z Chroma key access %02x\n", data);
}

void x1_state::z_extra_scroll_w(uint8_t data)
{
	logerror("Z Extra scroll config access %02x\n", data);
}


/*************************************
 *
 *  Inputs
 *
 *************************************/

INPUT_CHANGED_MEMBER(x1_state::ipl_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);

	m_ram_bank = 0x00;
	if(m_is_turbo) { m_ex_bank = 0x10; }
	//anything else?
}

// on 177 this makes the game to reset, on other games sending a NMI signal just causes a jump to la-la-land (including the Konami ones)
INPUT_CHANGED_MEMBER(x1_state::nmi_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_PORTS_START( x1 )
	PORT_START("FP_SYS") //front panel buttons, hard-wired with the soft reset/NMI lines
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, x1_state, ipl_reset,0) PORT_NAME("IPL reset")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, x1_state, nmi_reset,0) PORT_NAME("NMI reset")

	PORT_START("SOUND_SW")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IOSYS")
	// TODO: route front-panel DIP-SW here
	PORT_DIPNAME( 0x01, 0x01, "IOSYS" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Sound Setting?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// TODO: move me to x1_keyboard_device
	PORT_START("key1") //0x00-0x1f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_UNUSED) //0x00 null
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-2") PORT_CHAR(1)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-3") PORT_CHAR(2)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_END) PORT_CHAR(3)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-5") PORT_CHAR(4)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-6") PORT_CHAR(5)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-7") PORT_CHAR(6)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-8") PORT_CHAR(7)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DEL INS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HTab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-3") PORT_CHAR(10)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-4") PORT_CHAR(11)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-5") PORT_CHAR(12)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-7") PORT_CHAR(14)
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-8") PORT_CHAR(15)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-1") PORT_CHAR(16)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-2") PORT_CHAR(17)
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-3") PORT_CHAR(18)
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-4") PORT_CHAR(19)
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-5") PORT_CHAR(20)
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-6") PORT_CHAR(21)
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-7") PORT_CHAR(22)
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-8") PORT_CHAR(23)
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-1") PORT_CHAR(24)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-2") PORT_CHAR(25)
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-3") PORT_CHAR(26)
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

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
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2a *
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2b +
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3c <
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3d =
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3e >
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3f ?

	PORT_START("key3") //0x40-0x5f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(u8"¥") PORT_CHAR(U'¥') PORT_CHAR('|')
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("_") PORT_CHAR('_')

	PORT_START("f_keys")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR(UCHAR_MAMEKEY(F10))

	PORT_START("tenkey")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey -") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey *") PORT_CODE(KEYCODE_ASTERISK)
	// TODO: add other numpad keys (comma, period, equals, enter, HOME/CLR)

	PORT_START("key_modifiers")
	PORT_BIT(0x00000001,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x00000002,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x00000004,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("KANA") PORT_CODE(KEYCODE_RCONTROL) PORT_TOGGLE
	PORT_BIT(0x00000008,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x00000010,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_LALT)
INPUT_PORTS_END

INPUT_PORTS_START( x1turbo )
	PORT_INCLUDE( x1 )
	// TODO: add other keys (ROLL UP, ROLL DOWN, HELP, COPY, XFER)

	PORT_MODIFY("SOUND_SW")
	PORT_DIPNAME( 0x80, 0x80, "OPM Sound Setting?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("X1TURBO_DSW")
	PORT_DIPNAME( 0x01, 0x01, "Interlace mode" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x00, "Default Auto-boot Device" ) // this selects what kind of device is loaded at start-up
	PORT_DIPSETTING(    0x00, "5/3-inch 2D" )
	PORT_DIPSETTING(    0x02, "5/3-inch 2DD" )
	PORT_DIPSETTING(    0x04, "5/3-inch 2HD" )
	PORT_DIPSETTING(    0x06, "5/3-inch 2DD (IBM)" )
	PORT_DIPSETTING(    0x08, "8-inch 2D256" )
	PORT_DIPSETTING(    0x0a, "8-inch 2D256 (IBM)" )
	PORT_DIPSETTING(    0x0c, "8-inch 1S128 (IBM)" )
	PORT_DIPSETTING(    0x0e, "SASI HDD" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) //this is a port conditional of some sort ...
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/*************************************
 *
 *  GFX decoding
 *
 *************************************/

static const gfx_layout x1_chars_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout x1_chars_8x16 =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	8*16
};

static const gfx_layout x1_chars_16x16 =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};

/* decoded for debugging purpose, this will be nuked in the end... */
static GFXDECODE_START( gfx_x1 )
	GFXDECODE_ENTRY( "cgrom",   0x00000, x1_chars_8x8,    0, 1 )
	GFXDECODE_ENTRY( "font",    0x00000, x1_chars_8x16,   0, 1 )
	GFXDECODE_ENTRY( "kanji",   0x00000, x1_chars_16x16,  0, 1 )
//  GFXDECODE_ENTRY( "pcg",     0x00000, x1_pcg_8x8,      0, 1 )
GFXDECODE_END

static const z80_daisy_config x1_daisy[] =
{
	{ "x1kb" },
	{ "ctc" },
	{ nullptr }
};

// TODO: verify order, suppose ctc_ym really goes as generic ext pin instead.
static const z80_daisy_config x1turbo_daisy[] =
{
	{ "x1kb" },
	{ "ctc_ym" },
	{ "ctc" },
	{ "dma" },
	{ "sio" },
	{ nullptr }
};

/*************************************
 *
 *  Machine Functions
 *
 *************************************/

#ifdef UNUSED_FUNCTION
IRQ_CALLBACK_MEMBER(x1_state::x1_irq_callback)
{
	if(m_ctc_irq_flag != 0)
	{
		m_ctc_irq_flag = 0;
		if(m_key_irq_flag == 0)  // if no other devices are pulling the IRQ line high
			device.execute().set_input_line(0, CLEAR_LINE);
		return m_irq_vector;
	}
	if(m_key_irq_flag != 0)
	{
		m_key_irq_flag = 0;
		if(m_ctc_irq_flag == 0)  // if no other devices are pulling the IRQ line high
			device.execute().set_input_line(0, CLEAR_LINE);
		return m_key_irq_vector;
	}
	return m_irq_vector;
}
#endif

TIMER_DEVICE_CALLBACK_MEMBER(x1_state::sub_keyboard_cb)
{
	uint32_t key1 = ioport("key1")->read();
	uint32_t key2 = ioport("key2")->read();
	uint32_t key3 = ioport("key3")->read();
	uint32_t key4 = ioport("tenkey")->read();
	uint32_t f_key = ioport("f_keys")->read();

	if(m_key_irq_vector)
	{
		//if(key1 == 0 && key2 == 0 && key3 == 0 && key4 == 0 && f_key == 0)
		//  return;

		if((key1 != m_old_key1) || (key2 != m_old_key2) || (key3 != m_old_key3) || (key4 != m_old_key4) || (f_key != m_old_fkey))
		{
			// generate keyboard IRQ
			sub_io_w(0xe6);
			m_irq_vector = m_key_irq_vector;
			m_key_irq_flag = 1;
			m_maincpu->set_input_line(0,ASSERT_LINE);
			m_old_key1 = key1;
			m_old_key2 = key2;
			m_old_key3 = key3;
			m_old_key4 = key4;
			m_old_fkey = f_key;
		}
	}
}

TIMER_CALLBACK_MEMBER(x1_state::rtc_tick_cb)
{
	static const uint8_t dpm[12] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };

	m_rtc.sec++;

	if((m_rtc.sec & 0x0f) >= 0x0a)              { m_rtc.sec+=0x10; m_rtc.sec&=0xf0; }
	if((m_rtc.sec & 0xf0) >= 0x60)              { m_rtc.min++; m_rtc.sec = 0; }
	if((m_rtc.min & 0x0f) >= 0x0a)              { m_rtc.min+=0x10; m_rtc.min&=0xf0; }
	if((m_rtc.min & 0xf0) >= 0x60)              { m_rtc.hour++; m_rtc.min = 0; }
	if((m_rtc.hour & 0x0f) >= 0x0a)             { m_rtc.hour+=0x10; m_rtc.hour&=0xf0; }
	if((m_rtc.hour & 0xff) >= 0x24)             { m_rtc.day++; m_rtc.wday++; m_rtc.hour = 0; }
	if((m_rtc.wday & 0x0f) >= 0x07)             { m_rtc.wday = 0; }
	if((m_rtc.day & 0x0f) >= 0x0a)              { m_rtc.day+=0x10; m_rtc.day&=0xf0; }
	/* FIXME: very crude leap year support (i.e. it treats the RTC to be with a 2000-2099 timeline), dunno how the real x1 supports this,
	   maybe it just have a 1980-1999 timeline since year 0x00 shows as a XX on display */
	if(((m_rtc.year % 4) == 0) && m_rtc.month == 2)
	{
		if((m_rtc.day & 0xff) >= dpm[m_rtc.month-1]+1+1)
			{ m_rtc.month++; m_rtc.day = 0x01; }
	}
	else if((m_rtc.day & 0xff) >= dpm[m_rtc.month-1]+1){ m_rtc.month++; m_rtc.day = 0x01; }
	if(m_rtc.month > 12)                            { m_rtc.year++;  m_rtc.month = 0x01; }
	if((m_rtc.year & 0x0f) >= 0x0a)             { m_rtc.year+=0x10; m_rtc.year&=0xf0; }
	if((m_rtc.year & 0xf0) >= 0xa0)             { m_rtc.year = 0; } //roll over
}

void x1_state::machine_reset()
{
	//uint8_t *ROM = memregion("x1_cpu")->base();
	int i;

	memset(m_gfx_bitmap_ram.get(),0x00,0xc000*2);

	for(i=0;i<0x1800;i++)
	{
		m_pcg_ram[i] = 0;
		m_gfxdecode->gfx(3)->mark_dirty(i >> 3);
	}

	m_is_turbo = 0;

	m_iobank->set_bank(0);

	//m_x1_cpu->set_irq_acknowledge_callback(device_irq_acknowledge_delegate(FUNC(x1_state::x1_irq_callback),this));

	m_cmt_current_cmd = 0;
	m_cmt_test = 0;
	m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);

	m_key_irq_flag = m_ctc_irq_flag = 0;
	m_sub_cmd = 0;
	m_key_irq_vector = 0;
	m_sub_cmd_length = 0;
	m_sub_val[0] = 0;
	m_sub_val[1] = 0;
	m_sub_val[2] = 0;
	m_sub_val[3] = 0;
	m_sub_val[4] = 0;
	m_sub_obf = (m_sub_cmd_length) ? 0x00 : 0x20;
	m_sub_val_ptr = 0;
	m_key_i = 0;
	m_scrn_reg.v400_mode = 0;
	m_scrn_reg.ank_sel = 0;

	m_rtc_timer->adjust(attotime::zero, 0, attotime::from_seconds(1));

	/* Reinitialize palette here if there's a soft reset for the Turbo PAL stuff*/
	for(i=0;i<0x10;i++)
		m_palette->set_pen_color(i, pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));

	m_ram_bank = 0;
//  m_old_vpos = -1;

	m_fdc->dden_w(0);
}

void x1turbo_state::machine_reset()
{
	x1_state::machine_reset();
	m_is_turbo = 1;
	m_ex_bank = 0x10;

	m_scrn_reg.blackclip = 0;
}

static const gfx_layout x1_pcg_8x8 =
{
	8,8,
	0x100,
	3,
	{ 0x1000*8,0x800*8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

void x1_state::machine_start()
{
	/* set up RTC */
	{
		system_time systime;
		machine().base_datetime(systime);

		m_rtc.day = ((systime.local_time.mday / 10)<<4) | ((systime.local_time.mday % 10) & 0xf);
		m_rtc.month = ((systime.local_time.month+1));
		m_rtc.wday = ((systime.local_time.weekday % 10) & 0xf);
		m_rtc.year = (((systime.local_time.year % 100)/10)<<4) | ((systime.local_time.year % 10) & 0xf);
		m_rtc.hour = ((systime.local_time.hour / 10)<<4) | ((systime.local_time.hour % 10) & 0xf);
		m_rtc.min = ((systime.local_time.minute / 10)<<4) | ((systime.local_time.minute % 10) & 0xf);
		m_rtc.sec = ((systime.local_time.second / 10)<<4) | ((systime.local_time.second % 10) & 0xf);

		m_rtc_timer = timer_alloc(FUNC(x1_state::rtc_tick_cb), this);
	}

	m_motor_timer = timer_alloc(FUNC(x1_state::fdc_motor_off_cb), this);
	m_work_ram = make_unique_clear<uint8_t[]>(0x10000*0x10);
	m_emm_ram = make_unique_clear<uint8_t[]>(0x1000000);
	m_pcg_ram = make_unique_clear<uint8_t[]>(0x1800);

	save_pointer(NAME(m_work_ram), 0x10000*0x10);
	save_pointer(NAME(m_emm_ram), 0x1000000);
	save_pointer(NAME(m_pcg_ram), 0x1800);
	save_item(STRUCT_MEMBER(m_scrn_reg, video_mode));

	m_gfxdecode->set_gfx(3, std::make_unique<gfx_element>(m_palette, x1_pcg_8x8, m_pcg_ram.get(), 0, 1, 0));
}

void x1_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_2D_FORMAT);
}

static void x1_floppies(device_slot_interface &device)
{
	// TODO: 3" (!?) and 8" options, verify if vanilla X1 has them all
	device.option_add("525dd", FLOPPY_525_DD);
//  device.option_add("525hd", FLOPPY_525_HD);
}

void x1_state::x1(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MAIN_CLOCK/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &x1_state::x1_mem);
	m_maincpu->set_addrmap(AS_IO, &x1_state::x1_io);
	m_maincpu->set_daisy_config(x1_daisy);

	ADDRESS_MAP_BANK(config, m_iobank).set_map(&x1_state::x1_io_banks).set_options(ENDIANNESS_LITTLE, 8, 17, 0x10000);

	z80ctc_device& ctc(Z80CTC(config, "ctc", MAIN_CLOCK/4));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.zc_callback<0>().set("ctc", FUNC(z80ctc_device::trg3));
	// TODO: clocks for SIO
	ctc.zc_callback<1>().set("ctc", FUNC(z80ctc_device::trg1));
	ctc.zc_callback<2>().set("ctc", FUNC(z80ctc_device::trg2));

	X1_KEYBOARD(config, "x1kb", 0);

	i8255_device &ppi(I8255A(config, "ppi8255_0"));
	ppi.in_pa_callback().set(FUNC(x1_state::x1_porta_r));
	ppi.in_pb_callback().set(FUNC(x1_state::x1_portb_r));
	ppi.in_pc_callback().set(FUNC(x1_state::x1_portc_r));
	ppi.out_pa_callback().set(FUNC(x1_state::x1_porta_w));
	ppi.out_pb_callback().set(FUNC(x1_state::x1_portb_w));
	ppi.out_pc_callback().set(FUNC(x1_state::x1_portc_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 640-1, 0, 480-1);
	m_screen->set_screen_update(FUNC(x1_state::screen_update_x1));

	HD6845S(config, m_crtc, (VDP_CLOCK/48)); //unknown divider
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);

	PALETTE(config, m_palette, palette_device::BLACK, 0x10+0x1000);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_x1);

	MB8877(config, m_fdc, 16_MHz_XTAL / 16); // clocked by SED9421C0B

	FLOPPY_CONNECTOR(config, "fdc:0", x1_floppies, "525dd", x1_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", x1_floppies, "525dd", x1_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", x1_floppies, "525dd", x1_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", x1_floppies, "525dd", x1_state::floppy_formats).enable_sound(true);

	SOFTWARE_LIST(config, "flop_list").set_original("x1_flop");

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "x1_cart", "bin,rom");

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	// TODO: fix thru schematics (formation of resistors tied to ABC outputs)
	ay8910_device &ay(AY8910(config, "ay", MAIN_CLOCK/8));
	ay.port_a_read_callback().set_ioport("P1");
	ay.port_b_read_callback().set_ioport("P2");
	ay.add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	ay.add_route(ALL_OUTPUTS, "rspeaker", 0.25);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(x1_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "lspeaker", 0.25).add_route(ALL_OUTPUTS, "rspeaker", 0.10);
	m_cassette->set_interface("x1_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("x1_cass");

	TIMER(config, "keyboard_timer").configure_periodic(FUNC(x1_state::sub_keyboard_cb), attotime::from_hz(250));
	TIMER(config, "cmt_wind_timer").configure_periodic(FUNC(x1_state::cmt_seek_cb), attotime::from_hz(16));
}

void x1turbo_state::x1turbo(machine_config &config)
{
	x1(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &x1turbo_state::x1turbo_mem);
	m_maincpu->set_daisy_config(x1turbo_daisy);

	m_iobank->set_map(&x1turbo_state::x1turbo_io_banks);

	z80sio_device& sio(Z80SIO(config, "sio", MAIN_CLOCK/4));
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80DMA(config, m_dma, MAIN_CLOCK/4);
	m_dma->out_busreq_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_dma->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dma->in_mreq_callback().set(FUNC(x1turbo_state::memory_read_byte));
	m_dma->out_mreq_callback().set(FUNC(x1turbo_state::memory_write_byte));
	m_dma->in_iorq_callback().set(FUNC(x1turbo_state::io_read_byte));
	m_dma->out_iorq_callback().set(FUNC(x1turbo_state::io_write_byte));

	m_fdc->drq_wr_callback().set(FUNC(x1turbo_state::fdc_drq_w));

	// TODO: as sub-board option, CZ-8BS1
	Z80CTC(config, m_ctc_ym, MAIN_CLOCK/4);
	// FIXME: check intr
	m_ctc_ym->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc_ym->zc_callback<0>().set(m_ctc_ym, FUNC(z80ctc_device::trg3));

	YM2151(config, m_ym, MAIN_CLOCK/8);
	m_ym->add_route(0, "lspeaker", 0.50);
	m_ym->add_route(1, "rspeaker", 0.50);
}

/*************************************
 *
 * ROM definitions
 *
 *************************************/

ROM_START( x1 )
	ROM_REGION( 0x8000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.x1", 0x0000, 0x1000, CRC(7b28d9de) SHA1(c4db9a6e99873808c8022afd1c50fef556a8b44d) )

	ROM_REGION(0x1000, "mcu", ROMREGION_ERASEFF) //MCU for the Keyboard, "sub cpu"
	ROM_LOAD( "80c48", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION(0x2000, "font", 0) //TODO: this contains 8x16 charset only, maybe it's possible that it derivates a 8x8 charset by skipping gfx lines?
	ROM_LOAD( "ank.fnt", 0x0000, 0x2000, BAD_DUMP CRC(19689fbd) SHA1(0d4e072cd6195a24a1a9b68f1d37500caa60e599) )

	ROM_REGION(0x1800, "cgrom", 0)
	ROM_LOAD("fnt0808.x1",  0x00000, 0x00800, CRC(e3995a57) SHA1(1c1a0d8c9f4c446ccd7470516b215ddca5052fb2) )
	ROM_COPY("font",    0x1000, 0x00800, 0x1000 )

	ROM_REGION(0x20000, "kanji", ROMREGION_ERASEFF)

	ROM_REGION(0x20000, "raw_kanji", ROMREGION_ERASEFF)
ROM_END

ROM_START( x1turbo )
	ROM_REGION( 0x8000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.x1t", 0x0000, 0x8000, CRC(2e8b767c) SHA1(44620f57a25f0bcac2b57ca2b0f1ebad3bf305d3) )

	ROM_REGION(0x1000, "mcu", ROMREGION_ERASEFF) //MCU for the Keyboard, "sub cpu"
	ROM_LOAD( "80c48", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION(0x2000, "font", 0) //TODO: this contains 8x16 charset only, maybe it's possible that it derivates a 8x8 charset by skipping gfx lines?
	ROM_LOAD( "ank.fnt", 0x0000, 0x2000, CRC(19689fbd) SHA1(0d4e072cd6195a24a1a9b68f1d37500caa60e599) )

	ROM_REGION(0x4800, "cgrom", 0)
	ROM_LOAD("fnt0808_turbo.x1", 0x00000, 0x00800, CRC(84a47530) SHA1(06c0995adc7a6609d4272417fe3570ca43bd0454) )
	ROM_COPY("font",             0x01000, 0x00800, 0x1000 )

	ROM_REGION(0x20000, "kanji", ROMREGION_ERASEFF)

	ROM_REGION(0x20000, "raw_kanji", ROMREGION_ERASEFF)
	ROM_LOAD("kanji4.rom", 0x00000, 0x8000, CRC(3e39de89) SHA1(d3fd24892bb1948c4697dedf5ff065ff3eaf7562) )
	ROM_LOAD("kanji2.rom", 0x08000, 0x8000, CRC(e710628a) SHA1(103bbe459dc8da27a9400aa45b385255c18fcc75) )
	ROM_LOAD("kanji3.rom", 0x10000, 0x8000, CRC(8cae13ae) SHA1(273f3329c70b332f6a49a3a95e906bbfe3e9f0a1) )
	ROM_LOAD("kanji1.rom", 0x18000, 0x8000, CRC(5874f70b) SHA1(dad7ada1b70c45f1e9db11db273ef7b385ef4f17) )
ROM_END

ROM_START( x1turbo40 )
	ROM_REGION( 0x8000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.bin", 0x0000, 0x8000, CRC(112f80a2) SHA1(646cc3fb5d2d24ff4caa5167b0892a4196e9f843) )

	ROM_REGION(0x1000, "mcu", ROMREGION_ERASEFF) //MCU for the Keyboard, "sub cpu"
	ROM_LOAD( "80c48", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION(0x2000, "font", 0) //TODO: this contains 8x16 charset only, maybe it's possible that it derivates a 8x8 charset by skipping gfx lines?
	ROM_LOAD( "ank.fnt", 0x0000, 0x2000, CRC(19689fbd) SHA1(0d4e072cd6195a24a1a9b68f1d37500caa60e599) )

	ROM_REGION(0x4800, "cgrom", 0)
	ROM_LOAD("fnt0808_turbo.x1",0x00000, 0x0800, CRC(84a47530) SHA1(06c0995adc7a6609d4272417fe3570ca43bd0454) )
	ROM_COPY("font",            0x01000, 0x0800, 0x1000 )

	ROM_REGION(0x20000, "kanji", ROMREGION_ERASEFF)

	ROM_REGION(0x20000, "raw_kanji", ROMREGION_ERASEFF)
	ROM_LOAD("kanji4.rom", 0x00000, 0x8000, CRC(3e39de89) SHA1(d3fd24892bb1948c4697dedf5ff065ff3eaf7562) )
	ROM_LOAD("kanji2.rom", 0x08000, 0x8000, CRC(e710628a) SHA1(103bbe459dc8da27a9400aa45b385255c18fcc75) )
	ROM_LOAD("kanji3.rom", 0x10000, 0x8000, CRC(8cae13ae) SHA1(273f3329c70b332f6a49a3a95e906bbfe3e9f0a1) )
	ROM_LOAD("kanji1.rom", 0x18000, 0x8000, CRC(5874f70b) SHA1(dad7ada1b70c45f1e9db11db273ef7b385ef4f17) )
ROM_END


/* Convert the ROM interleaving into something usable by the write handlers */
void x1_state::init_x1_kanji()
{
	uint8_t *kanji = memregion("kanji")->base();
	uint8_t *raw_kanji = memregion("raw_kanji")->base();

	uint32_t k = 0;
	for (uint32_t l=0; l < 2; l++)
	{
		for (uint32_t j = l*16; j < (l*16) + 0x10000; j += 32)
		{
			for (uint32_t i = 0; i  < 16; i++)
			{
				kanji[j + i] = raw_kanji[k];
				kanji[j + i + 0x10000] = raw_kanji[0x10000 + k];
				k++;
			}
		}
	}
}


COMP( 1982, x1,        0,      0,      x1,      x1,      x1_state,      empty_init,    "Sharp", "X1 (CZ-800C)",       0 )
// x1twin in x1twin.cpp
COMP( 1984, x1turbo,   x1,     0,      x1turbo, x1turbo, x1turbo_state, init_x1_kanji, "Sharp", "X1 Turbo (CZ-850C)", MACHINE_NOT_WORKING ) //model 10
COMP( 1985, x1turbo40, x1,     0,      x1turbo, x1turbo, x1turbo_state, init_x1_kanji, "Sharp", "X1 Turbo (CZ-862C)", 0 ) //model 40
//COMP( 1986, x1turboz,  x1,     0,      x1turbo, x1turbo, x1_state, init_x1_kanji, "Sharp", "X1 TurboZ", MACHINE_NOT_WORKING )
