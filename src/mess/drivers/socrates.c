/******************************************************************************
*
*  V-tech Socrates Driver
*  By Jonathan Gevaryahu AKA Lord Nightmare
*  with dumping help from Kevin 'kevtris' Horton
*
*  (driver structure copied from vtech1.c)
TODO:
    fix glitches with keyboard input (double keys still don't work, super painter letter entry still doesn't work)
    hook up hblank
    hook up mouse
    add waitstates for ram access (lack of this causes the system to run way too fast)
    find and hook up any timers/interrupt controls
    switch cartridges over to a CART system rather than abusing BIOS
    keyboard IR decoder MCU is HLE'd for now, needs decap and cpu core (it is rather tms1000 or CIC-like)


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
       10          0x40000 - 0x43fff Expansion Cartridge page 0 (cart ROM 0x0000-0x3fff)
       11          0x44000 - 0x47fff Expansion Cartridge page 1 (cart ROM 0x4000-0x7fff)
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

/* Core includes */
#include "emu.h"
#include "cpu/z80/z80.h"
#include "audio/socrates.h"


class socrates_state : public driver_device
{
public:
	socrates_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_sound(*this, "soc_snd")
		{ }
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<device_t> m_sound;

	rgb_t m_palette[256];

	UINT8 m_data[8];
	UINT8 m_rom_bank;
	UINT8 m_ram_bank;
	UINT16 m_scroll_offset;
	UINT8* m_videoram;
	UINT8 m_kb_latch_low[2];
	UINT8 m_kb_latch_high[2];
	UINT8 m_kb_latch_mouse;
	UINT8 m_kbmcu_rscount; // how many pokes the kbmcu has taken in the last frame
	UINT8 m_io40_latch; // what was last written to speech reg (for open bus)?
	UINT8 m_hblankstate; // are we in hblank?
	UINT8 m_vblankstate; // are we in vblank?
	UINT8 m_speech_running; // is speech synth talking?
	UINT32 m_speech_address; // address in speech space
	UINT8 m_speech_settings; // speech settings (nybble 0: ? externrom ? ?; nybble 1: ? ? ? ?)
	UINT8 m_speech_dummy_read; // have we done a dummy read yet?
	UINT8 m_speech_load_address_count; // number of times load address has happened
	UINT8 m_speech_load_settings_count; // number of times load settings has happened
	DECLARE_READ8_MEMBER(socrates_rom_bank_r);
	DECLARE_WRITE8_MEMBER(socrates_rom_bank_w);
	DECLARE_READ8_MEMBER(socrates_ram_bank_r);
	DECLARE_WRITE8_MEMBER(socrates_ram_bank_w);
	DECLARE_READ8_MEMBER(read_f3);
	DECLARE_WRITE8_MEMBER(kbmcu_strobe);
	DECLARE_READ8_MEMBER(status_and_speech);
	DECLARE_WRITE8_MEMBER(speech_command);
	DECLARE_READ8_MEMBER(socrates_keyboard_low_r);
	DECLARE_READ8_MEMBER(socrates_keyboard_high_r);
	DECLARE_WRITE8_MEMBER(socrates_keyboard_clear);
	DECLARE_WRITE8_MEMBER(reset_speech);
	DECLARE_WRITE8_MEMBER(socrates_scroll_w);
	DECLARE_WRITE8_MEMBER(socrates_sound_w);
	DECLARE_DRIVER_INIT(socrates);
};


/* Defines */

/* Components */

/* Devices */

static void socrates_set_rom_bank( running_machine &machine )
{
	socrates_state *state = machine.driver_data<socrates_state>();
	state->membank( "bank1" )->set_base( state->memregion("maincpu")->base() + ( state->m_rom_bank * 0x4000 ));
}

static void socrates_set_ram_bank( running_machine &machine )
{
	socrates_state *state = machine.driver_data<socrates_state>();
	state->membank( "bank2" )->set_base( machine.root_device().memregion("vram")->base() + ( (state->m_ram_bank&0x3) * 0x4000 )); // window 0
	state->membank( "bank3" )->set_base( state->memregion("vram")->base() + ( ((state->m_ram_bank&0xC)>>2) * 0x4000 )); // window 1
}

static void socrates_update_kb( running_machine &machine )
{
	socrates_state *state = machine.driver_data<socrates_state>();
	static const char *const rownames[] = { "keyboard_40", "keyboard_41", "keyboard_42", "keyboard_43", "keyboard_44" };
	int row, keyvalue, powerof2;
	int shift = 0;
	// first check that the kb latch[1] is clear; if it isn't, don't touch it!
	if ((state->m_kb_latch_low[1] != 0) || (state->m_kb_latch_high[1] != 1)) return;
	// next check for joypad buttons
	keyvalue = machine.root_device().ioport("keyboard_jp")->read();
	if (keyvalue != 0)
	{
		state->m_kb_latch_low[1] = (keyvalue & 0xFF0)>>4;
		state->m_kb_latch_high[1] = 0x80 | (keyvalue & 0xF);
		return; // get out of this function; due to the way key priorities work, we're done here.
	}
	// next check for mouse movement.
	// this isn't written yet, so write me please!
	// next check if shift is down
	shift = machine.root_device().ioport("keyboard_50")->read();
	// find key low and high byte ok keyboard section
	for (row = 4; row>=0; row--)
	{
		keyvalue = machine.root_device().ioport(rownames[row])->read();
		if (keyvalue != 0)
		{
			for (powerof2 = 9; powerof2 >= 0; powerof2--)
			{
				if ((keyvalue&(1<<powerof2)) == (1<<powerof2))
				{
					state->m_kb_latch_low[1] = (shift?0x50:0x40)+row;
					state->m_kb_latch_high[1] = (0x80 | powerof2);
					return; // get out of the for loop; due to the way key priorities work, we're done here.
				}
			}
		}
	}
	// no key was pressed... check if shift was hit then?
	if (shift != 0)
	{
		state->m_kb_latch_low[1] = 0x50;
		state->m_kb_latch_high[1] = 0x80;
	}
}

static void socrates_check_kb_latch( running_machine &machine ) // if kb[1] is full and kb[0] is not, shift [1] to [0] and clear [1]
{
	socrates_state *state = machine.driver_data<socrates_state>();
	if (((state->m_kb_latch_low[1] != 0) || (state->m_kb_latch_high[1] != 1)) &&
	((state->m_kb_latch_low[0] == 0) && (state->m_kb_latch_high[0] == 1)))
	{
		state->m_kb_latch_low[0] = state->m_kb_latch_low[1];
		state->m_kb_latch_low[1] = 0;
		state->m_kb_latch_high[0] = state->m_kb_latch_high[1];
		state->m_kb_latch_high[1] = 1;
	}
}

static MACHINE_RESET( socrates )
{
	socrates_state *state = machine.driver_data<socrates_state>();
 state->m_rom_bank = 0xF3; // actually set semi-randomly on real console but we need to initialize it somewhere...
 socrates_set_rom_bank( machine );
 state->m_ram_bank = 0;  // the actual console sets it semi randomly on power up, and the bios cleans it up.
 socrates_set_ram_bank( machine );
 state->m_kb_latch_low[0] = 0xFF;
 state->m_kb_latch_high[0] = 0x8F;
 state->m_kb_latch_low[1] = 0x00;
 state->m_kb_latch_high[1] = 0x01;
 state->m_kb_latch_mouse = 0;
 state->m_kbmcu_rscount = 0;
 state->m_io40_latch = 0;
 state->m_hblankstate = 0;
 state->m_vblankstate = 0;
 state->m_speech_running = 0;
 state->m_speech_address = 0;
 state->m_speech_settings = 0;
 state->m_speech_dummy_read = 0;
 state->m_speech_load_address_count = 0;
 state->m_speech_load_settings_count = 0;
}

DRIVER_INIT_MEMBER(socrates_state,socrates)
{
	UINT8 *gfx = machine().root_device().memregion("vram")->base();
	int i;
    /* fill vram with its init powerup bit pattern, so startup has the checkerboard screen */
    for (i = 0; i < 0x10000; i++)
        gfx[i] = (((i&0x1)?0x00:0xFF)^((i&0x100)?0x00:0xff));
// init sound channels to both be on lowest pitch and max volume
    machine().device("maincpu")->set_clock_scale(0.45f); /* RAM access waitstates etc. aren't emulated - slow the CPU to compensate */
}

READ8_MEMBER(socrates_state::socrates_rom_bank_r)
{
 return m_rom_bank;
}

WRITE8_MEMBER(socrates_state::socrates_rom_bank_w)
{
 m_rom_bank = data;
 socrates_set_rom_bank(machine());
}

READ8_MEMBER(socrates_state::socrates_ram_bank_r)
{
 return m_ram_bank;
}

WRITE8_MEMBER(socrates_state::socrates_ram_bank_w)
{
 m_ram_bank = data&0xF;
 socrates_set_ram_bank(machine());
}

READ8_MEMBER(socrates_state::read_f3)// used for read-only i/o ports as mame/mess doesn't have a way to set the unmapped area to read as 0xF3
{
 return 0xF3;
}

WRITE8_MEMBER(socrates_state::kbmcu_strobe) // strobe the keyboard MCU
{
	//logerror("0x%04X: kbmcu written with %02X!\n", m_maincpu->pc(), data); //if (m_maincpu->pc() != 0x31D)
	// if two writes happen within one frame, reset the keyboard latches
	m_kbmcu_rscount++;
	if (m_kbmcu_rscount > 1)
	{
		m_kb_latch_low[0] = 0x00;
		m_kb_latch_high[0] = 0x01;
		m_kb_latch_low[1] = 0x00;
		m_kb_latch_high[1] = 0x01;
		m_kb_latch_mouse = 0;
	}
}

READ8_MEMBER(socrates_state::status_and_speech)// read 0x4x, some sort of status reg
{
// bit 7 - speech status: high when speech is playing, low when it is not (or when speech cart is not present)
// bit 6 - unknown, usually set, may involve the writes to 0x30, possibly some sort of fixed-length timer?
// bit 5 - vblank status, high when not in vblank
// bit 4 - hblank status, high when not in hblank
// bit 3 - speech chip bit 3
// bit 2 - speech chip bit 2
// bit 1 - speech chip bit 1
// bit 0 - speech chip bit 0
UINT8 *speechromint = memregion("speechint")->base();
UINT8 *speechromext = memregion("speechext")->base();
	int temp = 0;
	temp |= (m_speech_running)?0x80:0;
	temp |= 0x40; // unknown, possibly IR mcu busy
	temp |= (m_vblankstate)?0:0x20;
	temp |= (m_hblankstate)?0:0x10;
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
static TIMER_CALLBACK( clear_speech_cb )
{
	socrates_state *state = machine.driver_data<socrates_state>();
	state->m_speech_running = 0;
	state->m_speech_load_address_count = 0; // should this be here or in the write functuon subpart which is speak command?
	state->m_speech_load_settings_count = 0;
}

WRITE8_MEMBER(socrates_state::speech_command)// write 0x4x, some sort of bitfield; speech chip is probably hitachi hd38880 related but not exact, w/4 bit interface
{
	logerror("write to i/o 0x4x of %x\n", data);
/*
// the high 4 bits of the write control which 'register' is written to, the low 4 bits are data (this is based on a readback test)
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
/* all this really tells us is: 0x80 is the speech start command;
   all commands are open bus on readback of 4 bits EXCEPT for 0x60 and 0xE0, which are the CTP active and inactive versions of the same command */
/*  following is hd38880 info:
    microcomputer interface of hd38880 is usually 7 wires:
    FP      frame pulse, involved with READ command somehow
    SYBS1   data line bit 0, bidirectional
    SYBS2   data line bit 1, "
    SYBS3   data line bit 2, "
    SYBS4   data line bit 3, "
    CTP     Command pulse line, sort of a 'write' line
    CMV     More or less the 'command is being written' line which is active whenenever any command sequence is being written and during readback.

    The instructions which the hd38880 can be sent are: (msb first/to the left), blank instructions are invalid
    0000    (nop?, used as a dummy read/write during direction change??? invalid when used alone?)
    0001
    0010    ADSET (Transfer address to "vsm-alike serial roms") <followed by 5 nybbles>
    0011    READ (read nybble) <bus changes from cpu->speechchip to speechchip->sys and you get as many nybbles as you pulse the CTP line for, address auto-increments>
    0100    INT1 (initialize 1) <followed by one nybble>
    0101
    0110    INT2 (initialize 2) <followed by one nybble>
    0111
    1000    SYSPD (set speed) <followed by one nybble>
    1001
    1010    STOP
    1011    CONDT (Read state P1) <bus changes from cpu->speechchip to speechchip->sys and you get one nybble>
    1100    START
    1101
    1110    SSTART (same as start but syspd speed is ignored and forced to be set to 9 (scale = 1.0))
    1111
end hd38880 info.*/
/* the socrates speech chip does not QUITE match the hd38880 though, but is very similar */
	switch(data&0xF0)
	{
		case 0x80:
			if (data==0x80)
			{
				/* write me: start talking */
				m_speech_running = 1;
				machine().scheduler().timer_set(attotime::from_seconds(4), FUNC(clear_speech_cb)); // hack
			}
			break;
		case 0x90: // unknown, one of these is probably read and branch
			break;
		case 0xA0: // unknown
			break;
		case 0xB0: // unknown
			break;
		case 0xC0: // load address to vsm
			m_speech_address |= (((int)data&0xF)<<(m_speech_load_address_count*4))<<1;
			m_speech_load_address_count++;
			logerror("loaded address nybble %X, byte address is currently %5X with %d nybbles loaded\n", data&0xF, m_speech_address>>1, m_speech_load_address_count);
			break;
		case 0xD0: // load settings
			m_speech_settings |= ((data&0xF)<<(m_speech_load_settings_count*4));
			m_speech_load_settings_count++;
			break;
		case 0xE0: // read byte, handled elsewhere
			break;
		case 0xF0: // command: sub 0 is speak, sub 8 is reset
			if ((data&0xF) == 0) // speak
			{
				m_speech_running = 1;
				machine().scheduler().timer_set(attotime::from_seconds(4), FUNC(clear_speech_cb)); // hack
			}
			else if ((data&0xF) == 8) // reset
			{
				m_speech_running = 0;
				m_speech_address = 0;
				m_speech_settings = 0;
				m_speech_dummy_read = 0;
				m_speech_load_address_count = 0;
				m_speech_load_settings_count = 0;
				m_io40_latch &= 0x0f; // set last command to 0 to prevent problems
			}
			else // other
			{
			logerror("speech command 0xF%x is unknown!\n",data&0xF);
			}
			break;
		default: // 00 through 70 are packets without the write bit set, ignore them
			break;
	}
	m_io40_latch = data;
}

READ8_MEMBER(socrates_state::socrates_keyboard_low_r)// keyboard code low
{
 socrates_update_kb(machine());
 socrates_check_kb_latch(machine());
 return m_kb_latch_low[0];
}

READ8_MEMBER(socrates_state::socrates_keyboard_high_r)// keyboard code high
{
 socrates_update_kb(machine());
 socrates_check_kb_latch(machine());
 return m_kb_latch_high[0];
}

WRITE8_MEMBER(socrates_state::socrates_keyboard_clear)// keyboard latch shift/clear
{
	m_kb_latch_low[0] = m_kb_latch_low[1];
	m_kb_latch_high[0] = m_kb_latch_high[1];
	m_kb_latch_low[1] = 0;
	m_kb_latch_high[1] = 1;
}

WRITE8_MEMBER(socrates_state::reset_speech)// i/o 60: reset speech synth
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

/* stuff below belongs in video/nc.c */

WRITE8_MEMBER(socrates_state::socrates_scroll_w)
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

static rgb_t socrates_create_color(UINT8 color)
{
  rgb_t composedcolor;
  static const double lumatable[256] = {
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
    LUMA_COL_F
  };
  static const double chromaintensity[256] = {
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
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  };
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
  static const double phaseangle[16] = { 0, 90, 220, 150, 270, 40, 0, 315, 180, 210, 240, 300, 330, 60, 120, 0 }; // note: these are guessed, not measured yet!
  int chromaindex = color&0x0F;
  int swappedcolor = ((color&0xf0)>>4)|((color&0x0f)<<4);
  double finalY, finalI, finalQ, finalR, finalG, finalB;
  finalY = (1/LUMAMAX) * lumatable[swappedcolor];
  finalI = (M_I * (cos((phaseangle[chromaindex]/180)*3.141592653589793)))* ((1/CHROMAMAX)*chromaintensity[swappedcolor]);
  finalQ = (M_Q * (sin((phaseangle[chromaindex]/180)*3.141592653589793)))* ((1/CHROMAMAX)*chromaintensity[swappedcolor]);
  if (finalY > 1) finalY = 1; // clamp luma
  /* calculate the R, G and B values here, neato matrix math */
  finalR = (finalY*1)+(finalI*0.9563)+(finalQ*0.6210);
  finalG = (finalY*1)+(finalI*-0.2721)+(finalQ*-0.6474);
  finalB = (finalY*1)+(finalI*-1.1070)+(finalQ*1.7046);
  /* scale/clamp to 0-255 range */
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
composedcolor = MAKE_RGB((int)finalR,(int)finalG,(int)finalB);
return composedcolor;
}


static PALETTE_INIT( socrates )
{
	socrates_state *state = machine.driver_data<socrates_state>();
	int i; // iterator
	for (i = 0; i < 256; i++)
	{
	 state->m_palette[i] = socrates_create_color(i);
	}
	palette_set_colors(machine, 0, state->m_palette, ARRAY_LENGTH(state->m_palette));
}

static VIDEO_START( socrates )
{
	socrates_state *state = machine.driver_data<socrates_state>();
	state->m_videoram = state->memregion("vram")->base();
	state->m_scroll_offset = 0;
}

static SCREEN_UPDATE_IND16( socrates )
{
	socrates_state *state = screen.machine().driver_data<socrates_state>();
	static const UINT8 fixedcolors[8] =
	{
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0xF7
	};
	int x, y, colidx, color;
	int lineoffset = 0; // if display ever tries to display data at 0xfxxx, offset line displayed by 0x1000
	for (y = 0; y < 228; y++)
	{
		if ((((y+state->m_scroll_offset)*128)&0xffff) >= 0xf000) lineoffset = 0x1000; // see comment above
		for (x = 0; x < 264; x++)
		{
			if (x < 256)
			{
				colidx = state->m_videoram[(((y+state->m_scroll_offset)*128)+(x>>1)+lineoffset)&0xffff];
				if (x&1) colidx >>=4;
				colidx &= 0xF;
				if (colidx > 7) color=state->m_videoram[0xF000+(colidx<<8)+((y+state->m_scroll_offset)&0xFF)];
				else color=fixedcolors[colidx];
				bitmap.pix16(y, x) = color;
			}
			else
			{
				colidx = state->m_videoram[(((y+state->m_scroll_offset)*128)+(127)+lineoffset)&0xffff];
				colidx >>=4;
				colidx &= 0xF;
				if (colidx > 7) color=state->m_videoram[0xF000+(colidx<<8)+((y+state->m_scroll_offset)&0xFF)];
				else color=fixedcolors[colidx];
				bitmap.pix16(y, x) = color;
			}
		}
	}
	return 0;
}

/* below belongs in sound/nc.c */

WRITE8_MEMBER(socrates_state::socrates_sound_w)
{
	device_t *socr_snd = machine().device("soc_snd");
	switch(offset)
	{
		case 0:
		socrates_snd_reg0_w(socr_snd, data);
		break;
		case 1:
		socrates_snd_reg1_w(socr_snd, data);
		break;
		case 2:
		socrates_snd_reg2_w(socr_snd, data);
		break;
		case 3:
		socrates_snd_reg3_w(socr_snd, data);
		break;
		case 4: case 5: case 6: case 7: default:
		socrates_snd_reg4_w(socr_snd, data);
		break;
	}
}

/******************************************************************************
 Address Maps
******************************************************************************/

static ADDRESS_MAP_START(z80_mem, AS_PROGRAM, 8, socrates_state )
    ADDRESS_MAP_UNMAP_HIGH
    AM_RANGE(0x0000, 0x3fff) AM_ROM /* system rom, bank 0 (fixed) */
    AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1") /* banked rom space; system rom is banks 0 through F, cartridge rom is banks 10 onward, usually banks 10 through 17. area past the end of the cartridge, and the whole 10-ff area when no cartridge is inserted, reads as 0xF3 */
    AM_RANGE(0x8000, 0xbfff) AM_RAMBANK("bank2") /* banked ram 'window' 0 */
    AM_RANGE(0xc000, 0xffff) AM_RAMBANK("bank3") /* banked ram 'window' 1 */
ADDRESS_MAP_END

static ADDRESS_MAP_START(z80_io, AS_IO, 8, socrates_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(socrates_rom_bank_r, socrates_rom_bank_w) AM_MIRROR(0x7) /* rom bank select - RW - 8 bits */
	AM_RANGE(0x08, 0x08) AM_READWRITE(socrates_ram_bank_r, socrates_ram_bank_w) AM_MIRROR(0x7) /* ram banks select - RW - 4 low bits; Format: 0b****HHLL where LL controls whether window 0 points at ram area: 0b00: 0x0000-0x3fff; 0b01: 0x4000-0x7fff; 0b10: 0x8000-0xbfff; 0b11: 0xc000-0xffff. HH controls the same thing for window 1 */
	AM_RANGE(0x10, 0x17) AM_READWRITE(read_f3, socrates_sound_w) AM_MIRROR (0x8) /* sound section:
        0x10 - W - frequency control for channel 1 (louder channel) - 01=high pitch, ff=low; time between 1->0/0->1 transitions = (XTAL_21_4772MHz/(512+256) / (freq_reg+1)) (note that this is double the actual frequency since each full low and high squarewave pulse is two transitions)
    0x11 - W - frequency control for channel 2 (softer channel) - 01=high pitch, ff=low; same equation as above
    0x12 - W - 0b***EVVVV enable, volume control for channel 1
    0x13 - W - 0b***EVVVV enable, volume control for channel 2
    0x14-0x17 - 0bE??????? enable, unknown for channel 3; produces well defined dmc waves when bit 7 is set, and one or two other bits
    This may be some sort of debug register for serial-dmc banging out some internal rom from the asic, maybe color data?
    No writes to ram seem to change the waveforms produced, in my limited testing.
    0x80 produces about a very very quiet 1/8 duty cycle wave at 60hz or so
    0xC0 produces a DMC wave read from an unknown address at around 342hz
    0x
    */
	AM_RANGE(0x20, 0x21) AM_READWRITE(read_f3, socrates_scroll_w) AM_MIRROR (0xe) /* graphics section:
    0x20 - W - lsb offset of screen display
    0x21 - W - msb offset of screen display
    resulting screen line is one of 512 total offsets on 128-byte boundaries in the whole 64k ram
    */
	AM_RANGE(0x30, 0x30) AM_READWRITE(read_f3, kbmcu_strobe) AM_MIRROR (0xf) /* resets the keyboard IR decoder MCU */
	AM_RANGE(0x40, 0x40) AM_READWRITE(status_and_speech, speech_command ) AM_MIRROR(0xf) /* reads status register for vblank/hblank/speech, also reads and writes speech module */
	AM_RANGE(0x50, 0x50) AM_READWRITE(socrates_keyboard_low_r, socrates_keyboard_clear) AM_MIRROR(0xE) /* Keyboard keycode low, latched on keypress, can be unlatched by writing anything here */
	AM_RANGE(0x51, 0x51) AM_READWRITE(socrates_keyboard_high_r, socrates_keyboard_clear) AM_MIRROR(0xE) /* Keyboard keycode high, latched as above, unlatches same as above */
	AM_RANGE(0x60, 0x60) AM_READWRITE(read_f3, reset_speech) AM_MIRROR(0xF) /* reset the speech module, or perhaps fire an NMI?  */
	AM_RANGE(0x70, 0xFF) AM_READ(read_f3) // nothing mapped here afaik
ADDRESS_MAP_END


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

	PORT_START("keyboard_jp") // joypad keys
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_NAME("Left D-pad Right") // 00 81
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_NAME("Left D-pad Up") // 00 82
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_NAME("Left D-pad Left") // 00 84
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_NAME("Left D-pad Down") // 00 88
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME("Right D-pad Down") // 01 80
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME("Right D-pad Left") // 02 80
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_NAME("Right D-pad Up") // 04 80
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME("Right D-pad Right") // 08 80
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_NAME("Left D-pad Button") // 10 80
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT)) PORT_NAME("Right D-pad Button") // 20 80
	PORT_BIT(0xfffffc00, IP_ACTIVE_HIGH, IPT_UNUSED)
	/* alt w/left and right keypad keys swapped
    PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME("Left D-pad Right") // 00 81
    PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_NAME("Left D-pad Up") // 00 82
    PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME("Left D-pad Left") // 00 84
    PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME("Left D-pad Down") // 00 88
    PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_NAME("Right D-pad Down") // 01 80
    PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_NAME("Right D-pad Left") // 02 80
    PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_NAME("Right D-pad Up") // 04 80
    PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_NAME("Right D-pad Right") // 08 80
    PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT)) PORT_NAME("Left D-pad Button") // 10 80
    PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_NAME("Right D-pad Button") // 20 80
    PORT_BIT(0xfffffc00, IP_ACTIVE_HIGH, IPT_UNUSED)
    */

	PORT_START("keyboard_50") // lowest 'row' (technically the shift key is on the 5th row but it has its own keycode)
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("SHIFT") // 5x xx
	PORT_BIT(0xfffffffe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyboard_40") // 5th row
	PORT_BIT(0x00000003, IP_ACTIVE_HIGH, IPT_UNUSED) // 40 80 and 40 81
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_NAME("SPACE") // 40 82
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_NAME("Z") // 40 83
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_NAME("Y") // 40 84
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_NAME("X") // 40 85
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_NAME("W") // 40 86
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_NAME("V") // 40 87
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_NAME("U") // 40 88
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_NAME("T") // 40 89
	PORT_BIT(0xfffffc00, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyboard_41") // 4th row
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_NAME("J/Ti") // 41 80
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_NAME("S") // 41 81
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_NAME("R") // 41 82
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_NAME("Q/NEW") // 41 83
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_NAME("P/PLAY") // 41 84
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_NAME("O/PAUSE") // 41 85
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_NAME("N/Fa'") // 41 86
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_NAME("M/Mi'") // 41 87
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_NAME("L/Re'") // 41 88
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_NAME("K/Do'") // 41 89
	PORT_BIT(0xfffffc00, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyboard_42") // 3rd row
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_NAME("A/So.") // 42 80
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('-') PORT_NAME("-/.") // 42 81
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_NAME("I/La") // 42 82
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_NAME("H/So") // 42 83
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_NAME("G/Fa") // 42 84
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_NAME("F/Mi") // 42 85
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_NAME("E/Re") // 42 86
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_NAME("D/Do") // 42 87
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_NAME("C/Ti.") // 42 88
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_NAME("B/La.") // 42 89
	PORT_BIT(0xfffffc00, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyboard_43") // 2nd row
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_NAME("1") // 43 80
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_NAME("0") // 43 81
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_NAME("9") // 43 82
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_NAME("8") // 43 83
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_NAME("7") // 43 84
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_NAME("6") // 43 85
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_NAME("5") // 43 86
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_NAME("4") // 43 87
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_NAME("3") // 43 88
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_NAME("2") // 43 89
	PORT_BIT(0xfffffc00, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyboard_44") // 1st row
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('+') PORT_NAME("+") // 44 80
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_UNUSED) // 44 81
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("ENTER") // 44 82
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_NAME("MENU") // 44 83
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(PGUP)) PORT_NAME("ANSWER") // 44 84
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(PGDN)) PORT_NAME("HELP") // 44 85
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("ERASE") // 44 86
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_NAME("/") // 44 87
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*') PORT_NAME("*") // 44 88
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_NAME("-") // 44 89
	PORT_BIT(0xfffffc00, IP_ACTIVE_HIGH, IPT_UNUSED)

	// mouse goes here
INPUT_PORTS_END


/******************************************************************************
 Machine Drivers
******************************************************************************/
static TIMER_CALLBACK( clear_irq_cb )
{
	socrates_state *state = machine.driver_data<socrates_state>();
	machine.device("maincpu")->execute().set_input_line(0, CLEAR_LINE);
	state->m_vblankstate = 0;
}

static INTERRUPT_GEN( assert_irq )
{
	socrates_state *state = device->machine().driver_data<socrates_state>();
	device->execute().set_input_line(0, ASSERT_LINE);
	device->machine().scheduler().timer_set(downcast<cpu_device *>(device)->cycles_to_attotime(44), FUNC(clear_irq_cb));
// 44 is a complete and total guess, need to properly measure how many clocks/microseconds the int line is high for.
	state->m_vblankstate = 1;
	state->m_kbmcu_rscount = 0; // clear the mcu poke count
}

static MACHINE_CONFIG_START( socrates, socrates_state )
    /* basic machine hardware */
    MCFG_CPU_ADD("maincpu", Z80, XTAL_21_4772MHz/6)  /* Toshiba TMPZ84C00AP @ 3.579545 MHz, verified, xtal is divided by 6 */
    MCFG_CPU_PROGRAM_MAP(z80_mem)
    MCFG_CPU_IO_MAP(z80_io)
    MCFG_QUANTUM_TIME(attotime::from_hz(60))
    MCFG_CPU_VBLANK_INT("screen", assert_irq)
    //MCFG_MACHINE_START(socrates)
    MCFG_MACHINE_RESET(socrates)

    /* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(264, 228) // technically the screen size is 256x228 but super painter abuses what I suspect is a hardware bug to display repeated pixels of the very last pixel beyond this horizontal space, well into hblank
	MCFG_SCREEN_VISIBLE_AREA(0, 263, 0, 219) // the last few rows are usually cut off by the screen bottom but are indeed displayed if you mess with v-hold
	MCFG_SCREEN_UPDATE_STATIC(socrates)

	MCFG_PALETTE_LENGTH(256)
	MCFG_PALETTE_INIT(socrates)

	MCFG_VIDEO_START(socrates)

    /* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("soc_snd", SOCRATES, XTAL_21_4772MHz/(512+256)) // this is correct, as strange as it sounds.
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(socrates)
    ROM_REGION(0x400000, "maincpu", ROMREGION_ERASEVAL(0xF3)) /* can technically address 4mb of rom via bankswitching; open bus area reads as 0xF3 */
    /* Socrates US NTSC */
	/* all cart roms are 28 pin 23c1000/tc531000 128Kx8 roms */
	/* cart port pinout:
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
    Note that a17 goes to what would be pin 2 if a 32 pin rom were installed, which is not the case. (pins 1, 31 and 32 would be tied to vcc)

    Cartridge check procedure by socrates is, after screen init and check for speech synth,
    bankswitch to bank 0x10 (i.e. first 0x4000 of cart appears at 4000-7fff in z80 space),
    do following tests; if any tests fail, jump to 0x0015 (socrates main menu)
    * read 0x7ff0(0x3ff0 in cart rom) and compare to 0xAA
    * read 0x7ff1(0x3ff1 in cart rom) and compare to 0x55
    * read 0x7ff2(0x3ff2 in cart rom) and compare to 0xE7
    * read 0x7ff3(0x3ff3 in cart rom) and compare to 0x18
    if all tests passed, jump to 0x4000 (0x0000 in cart rom)
    */
    ROM_DEFAULT_BIOS("nocart")
    ROM_LOAD("27-00817-000-000.u1", 0x00000, 0x40000, CRC(80f5aa20) SHA1(4fd1ff7f78b5dd2582d5de6f30633e4e4f34ca8f)) // Label: "(Vtech) 27-00817-000-000 // (C)1987 VIDEO TECHNOLOGY // 8811 D"
    ROM_SYSTEM_BIOS( 0, "nocart", "Socrates w/o cartridge installed")
    ROM_SYSTEM_BIOS( 1, "maze", "Socrates w/Amazing Mazes cartridge installed")
    ROMX_LOAD("27-5050-00.u1", 0x40000, 0x20000, CRC(95B84308) SHA1(32E065E8F48BAF0126C1B9AA111C291EC644E387), ROM_BIOS(2)) // Label: "(Vtech) 27-5050-00 // TC531000CP-L332 // (C)1989 VIDEO TECHNOLOGY // 8931EAI   JAPAN"; Alt label: "(Vtech) LH53101Y // (C)1989 VIDEO TECHNOLOGY // 8934 D"; cart has an orange QC stickse
    ROM_SYSTEM_BIOS( 2, "world", "Socrates w/Around the World cartridge installed")
    ROMX_LOAD("27-5013-00-0.u1", 0x40000, 0x20000, CRC(A1E01C38) SHA1(BEEB2869AE1DDC8BBC9A81749AB9662C14DD47D3), ROM_BIOS(3)) // Label: "(Vtech) 27-5013-00-0 // TC531000CP-L318 // (C)1989 VIDEO TECHNOLOGY // 8918EAI   JAPAN"; cart has an orange QC sticker
    ROM_SYSTEM_BIOS( 3, "fracts", "Socrates w/Facts'N Fractions cartridge installed")
    ROMX_LOAD("27-5001-00-0.u1", 0x40000, 0x20000, CRC(7118617B) SHA1(52268EF0ADB651AD62773FB2EBCB7506759B2686), ROM_BIOS(4)) // Label: "(Vtech) 27-5001-00-0 // TC531000CP-L313 // (C)1988 VIDEO TECHNOLOGY // 8918EAI   JAPAN"; cart has a brown QC sticker
    ROM_SYSTEM_BIOS( 4, "hodge", "Socrates w/Hodge-Podge cartridge installed")
    ROMX_LOAD("27-5014-00-0.u1", 0x40000, 0x20000, CRC(19E1A301) SHA1(649A7791E97BCD0D31AC65A890FACB5753AB04A3), ROM_BIOS(5)) // Label: "(Vtech) 27-5014-00-0 // TC531000CP-L316 // (C)1989 VIDEO TECHNOLOGY // 8913EAI   JAPAN"; cart has a green QC sticker
    ROM_SYSTEM_BIOS( 5, "memoryb", "Socrates w/Memory Mania rev B cartridge installed")
    ROMX_LOAD("27-5002-00-0.u1", 0x40000, 0x20000, CRC(3C7FD651) SHA1(3118F53625553010EC95EA91DA8320CCE3DC7FE4), ROM_BIOS(6)) // Label: "(Vtech) 27-5002-00-0 // TC531000CP-L314 // (C)1988 VIDEO TECHNOLOGY // 8905EAI   JAPAN"; cart has a red QC sticker with a small B sticker on top of it, and the rom has a large B sticker; cart pcb shows signs of resoldering, which leads me to believe this is the B revision of the rom code for this game
    ROM_SYSTEM_BIOS( 6, "state", "Socrates w/State to State cartridge installed")
    ROMX_LOAD("27-5045-00-0.u1", 0x40000, 0x20000, CRC(5848379F) SHA1(961C9CA4F28A9E02AA1D67583B2D2ADF8EE5F10E), ROM_BIOS(7)) // Label: "(Vtech) 27-5045-00-0 // TC531000CP-L333 // (C)1989 VIDEO TECHNOLOGY // 8931EAI   JAPAN"; cart has a brown QC sticker
// a cartridge called 'game master' is supposed to be here
// an international-only? cartridge called 'puzzles' is supposed to be here
// Cad professor mouse is supposed to be here
// the touch pad cartridge is supposed to be here

    ROM_REGION(0x10000, "vram", ROMREGION_ERASEFF) /* fill with ff, driver_init changes this to the 'correct' startup pattern */

    ROM_REGION(0x800, "kbmcu", ROMREGION_ERASEFF)
    ROM_LOAD("tmp42c40p1844.bin", 0x000, 0x200, NO_DUMP) /* keyboard IR decoder MCU */

    /* english speech cart has a green QC sticker */
    ROM_REGION(0x2000, "speechint", ROMREGION_ERASE00) // speech data inside of the speech chip; fill with 00, if no speech cart is present socrates will see this
    ROM_LOAD_OPTIONAL("speech_internal.bin", 0x0000, 0x2000, CRC(edc1fb3f) SHA1(78b4631fc3b1c038e14911047f9edd6c4e8bae58)) // 8k on the speech chip itself

	ROM_REGION(0x10000, "speechext", ROMREGION_ERASE00) // speech serial modules outside of the speech chip but still on speech cart
    ROM_LOAD_OPTIONAL("speech_eng_vsm1.bin", 0x0000, 0x4000, CRC(888e3ddd) SHA1(33AF6A21BA6D826071C9D48557B1C9012752570B)) // 16k in serial rom
    ROM_LOAD_OPTIONAL("speech_eng_vsm2.bin", 0x4000, 0x4000, CRC(de4ac89d) SHA1(3DFA853B02DF756A9B72DEF94A39310992EE11C7)) // 16k in serial rom
    ROM_LOAD_OPTIONAL("speech_eng_vsm3.bin", 0x8000, 0x4000, CRC(972384aa) SHA1(FFCB1D633CA6BFFC7F481EC505DA447E5B847F16)) // 16k in serial rom
    ROM_FILL(0xC000, 0x4000, 0xff) // last vsm isn't present, FF fill
ROM_END

ROM_START(socratfc)
    ROM_REGION(0x80000, "maincpu", ROMREGION_ERASEVAL(0xF3))
    /* Socrates SAITOUT (French Canadian) NTSC */
    ROM_LOAD("27-00884-001-000.u1", 0x00000, 0x40000, CRC(042d9d21) SHA1(9ffc67b2721683b2536727d0592798fbc4d061cb)) // Label: "(Vtech) 27-00884-001-000 // (C)1988 VIDEO TECHNOLOGY // 8911 D"
    ROM_LOAD_OPTIONAL("cartridge.bin", 0x40000, 0x20000, NO_DUMP)

    ROM_REGION(0x10000, "vram", ROMREGION_ERASEFF) /* fill with ff, driver_init changes this to the 'correct' startup pattern */

    ROM_REGION(0x800, "kbmcu", ROMREGION_ERASEFF)
    ROM_LOAD("tmp42c40p1844.bin", 0x000, 0x200, NO_DUMP) /* keyboard IR decoder MCU */

    ROM_REGION(0x2000, "speechint", ROMREGION_ERASE00) // speech data inside of the speech chip; fill with 00, if no speech cart is present socrates will see this
    ROM_LOAD_OPTIONAL("speech_fra_internal.bin", 0x0000, 0x2000, BAD_DUMP CRC(edc1fb3f) SHA1(78b4631fc3b1c038e14911047f9edd6c4e8bae58)) // probably same on french and english speech carts

    ROM_REGION(0x10000, "speechext", ROMREGION_ERASE00) // speech serial modules outside of the speech chip but still on speech cart
    ROM_LOAD_OPTIONAL("speech_fra_vsm1.bin", 0x0000, 0x4000, NO_DUMP) // 16k in serial rom
    ROM_LOAD_OPTIONAL("speech_fra_vsm2.bin", 0x4000, 0x4000, NO_DUMP) // 16k in serial rom
    ROM_LOAD_OPTIONAL("speech_fra_vsm3.bin", 0x8000, 0x4000, NO_DUMP) // 16k in serial rom
    ROM_FILL(0xC000, 0x4000, 0xff) // last vsm isn't present, FF fill
ROM_END



/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT       COMPANY                     FULLNAME                            FLAGS */
COMP( 1988, socrates,   0,          0,      socrates,   socrates, socrates_state, socrates, "Video Technology",        "Socrates Educational Video System", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND ) // English NTSC
COMP( 1988, socratfc,   socrates,   0,      socrates,   socrates, socrates_state, socrates, "Video Technology",        "Socrates SAITOUT", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND ) // French Canandian NTSC
// Yeno Professor Weiss-Alles goes here (german PAL)
// Yeno Professeur Saitout goes here (french SECAM)
// ? goes here (spanish PAL)
