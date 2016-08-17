// license:BSD-3-Clause
// copyright-holders:balrog,Jonathan Gevaryahu,Sandro Ronco
/******************************************************************************
*
*  Wavetek/Digelec model 804/EP804 (eprom programmer) driver
*  By balrog and Jonathan Gevaryahu AKA Lord Nightmare
*  Code adapted from zexall.c
*
*  DONE:
*  figure out z80 address space and hook up roms and rams (done)
*  figure out where 10937 vfd controller lives (port 0x44 bits 7 and 0, POR has not been found yet)
*  figure out how keypresses are detected (/INT?, port 0x43, and port 0x46)
*  figure out how the banked ram works (writes to port 0x43 select a ram bank, fw2.2 supports 64k and fw4.1 supports 256k)
*  tentatively figure out how flow control from ACIA works (/NMI)?
*  hook up the speaker/beeper (port 0x45)
*  hook up vfd controller (done to stderr, no artwork)
*  hook up leds on front panel (done to stderr for now)
*  hook up r6551 serial and attach terminal for sending to ep804
*  /nmi is emulated correctly (i.e. its tied to +5v. that was easy!)
*  hook up keypad via 74C923 and mode buttons via logic gate mess
*
*  TODO:
*  remove ACIA hack in digel804_state::acia_command_w
*  minor finishing touches to i/o map
*  EPROM socket stuff (ports 0x40, 0x41, 0x42 and 0x47)
*  artwork
*
******************************************************************************/

// port 40 read reads eprom socket pins 11-13, 15-19 (i.e. eprom pin D0 to pin D7)

// port 40 write writes eprom socket pins 11-13, 15-19 (i.e. eprom pin D0 to pin D7)

// port 41 write controls eprom socket pins 10 to 3 (d0 to d8 to eprom pins A0 to A7) and SIM pins 7, 20/11, 8/12, 27, 23, and 21 (d0 to d5, corresponding to SIM pins for A9, A10, A11, A12, A13, and A14+A15)
// Note: the v2.0 hardware has bit d6 controlling one of the pins going to the expansion ram socket
// The 2.0 hardware may also have separate SIM controls for A14 and A15, possibly using bit d7 to control sim A15 somehow

// port 42 write controls eprom socket pins 25(d0), 2(d4), 27(d6)

// port 43 read is status/mode
#undef PORT43_R_VERBOSE
// port 43 write is ram banking and ctl1-7; it also clears overload state
#define PORT43_W_VERBOSE 1
// port 44 write is vfd serial/reset, as well as power and z80 control and various enables; it also clears powerfail state
#define PORT44_W_VERBOSE 1
// port 45 write is speaker control
#undef PORT45_W_VERBOSE
// port 46 read is keypad

// port 46 write is LED control
#undef PORT46_W_VERBOSE

// port 47 write is tim0-tim7


/* Core includes */
#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/speaker.h"
#include "machine/roc10937.h"
#include "machine/mos6551.h"
#include "machine/mm74c922.h"
#include "machine/ram.h"
#include "bus/rs232/rs232.h"
#include "digel804.lh"


class digel804_state : public driver_device
{
public:
	digel804_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_acia(*this, "acia"),
		m_vfd(*this, "vfd"),
		m_kb(*this, "74c923"),
		m_ram(*this, RAM_TAG),
		m_rambank(*this, "bankedram")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<mos6551_device> m_acia;
	required_device<roc10937_t> m_vfd;
	required_device<mm74c922_device> m_kb;
	required_device<ram_device> m_ram;
	required_memory_bank m_rambank;

	virtual void machine_reset() override;
	DECLARE_DRIVER_INIT(digel804);
	DECLARE_WRITE8_MEMBER( op00 );
	DECLARE_READ8_MEMBER( ip40 );
	DECLARE_WRITE8_MEMBER( op40 );
	DECLARE_WRITE8_MEMBER( op41 );
	DECLARE_WRITE8_MEMBER( op42 );
	DECLARE_READ8_MEMBER( ip43 );
	DECLARE_WRITE8_MEMBER( op43 );
	DECLARE_WRITE8_MEMBER( op43_1_4 );
	DECLARE_WRITE8_MEMBER( op44 );
	DECLARE_WRITE8_MEMBER( op45 );
	DECLARE_READ8_MEMBER( ip46 );
	DECLARE_WRITE8_MEMBER( op46 );
	DECLARE_WRITE8_MEMBER( op47 );
	DECLARE_READ8_MEMBER( acia_rxd_r );
	DECLARE_WRITE8_MEMBER( acia_txd_w );
	DECLARE_READ8_MEMBER( acia_status_r );
	DECLARE_WRITE8_MEMBER( acia_reset_w );
	DECLARE_READ8_MEMBER( acia_command_r );
	DECLARE_WRITE8_MEMBER( acia_command_w );
	DECLARE_READ8_MEMBER( acia_control_r );
	DECLARE_WRITE8_MEMBER( acia_control_w );
	DECLARE_WRITE_LINE_MEMBER( acia_irq_w );
	DECLARE_WRITE_LINE_MEMBER( ep804_acia_irq_w );
	DECLARE_WRITE_LINE_MEMBER( da_w );
	DECLARE_INPUT_CHANGED_MEMBER(mode_change);
	// current speaker state for port 45
	UINT8 m_speaker_state;
	// ram stuff for banking
	UINT8 m_ram_bank;
	// states
	UINT8 m_acia_intq;
	UINT8 m_overload_state;
	UINT8 m_key_intq;
	UINT8 m_remote_mode;
	UINT8 m_key_mode;
	UINT8 m_sim_mode;
	UINT8 m_powerfail_state;
	UINT8 m_chipinsert_state;
	UINT8 m_keyen_state;
	UINT8 m_op41;
};


enum { MODE_OFF, MODE_KEY, MODE_REM, MODE_SIM };


DRIVER_INIT_MEMBER(digel804_state,digel804)
{
	m_speaker_state = 0;
	//port43_rtn = 0xEE;//0xB6;
	m_acia_intq = 1; // /INT source 1
	m_overload_state = 0; // OVLD
	m_key_intq = 1; // /INT source 2
	m_remote_mode = 1; // /REM
	m_key_mode = 0; // /KEY
	m_sim_mode = 1; // /SIM
	m_powerfail_state = 1; // "O11"
	m_chipinsert_state = 0; // PIN
	m_ram_bank = 0;
	m_op41 = 0;
	m_keyen_state = 1; // /KEYEN

	m_rambank->set_base(m_ram->pointer());
}

void digel804_state::machine_reset()
{
	m_vfd->reset();
}

READ8_MEMBER( digel804_state::ip40 ) // eprom data bus read
{
	// TODO: would be nice to have a 'fake eprom' here
	return 0xFF;
}

WRITE8_MEMBER( digel804_state::op40 ) // eprom data bus write
{
	// TODO: would be nice to have a 'fake eprom' here
	logerror("Digel804: port 40, eprom databus had %02X written to it!\n", data);
}

WRITE8_MEMBER( digel804_state::op41 ) // eprom address low write AND SIM write, d6 also controls memory map somehow
{
	// TODO: would be nice to have a 'fake eprom' here
	logerror("Digel804: port 41, eprom address low/sim/memorybank had %02X written to it!\n", data);
	//fprintf(stderr,"op41 write of %02X, ram mapper bit is %d\n", data, (data&0x40)?1:0);
	m_op41 = data;
}

WRITE8_MEMBER( digel804_state::op42 ) // eprom address hi and control write
{
	// TODO: would be nice to have a 'fake eprom' here
	logerror("Digel804: port 42, eprom address hi/control had %02X written to it!\n", data);
}

READ8_MEMBER( digel804_state::ip43 )
{
	/* Register 0x43: status/mode register read
	 bits 76543210
	      |||||||\- overload state (0 = not overloaded; 1 = overload detected, led on and power disconnected to ic, writing to R43 sets this to ok)
	      ||||||\-- debug jumper X5 on the board; reads as 1 unless jumper is present
	      |||||\--- /INT status (any key pressed on keypad (0 = one or more pressed, 1 = none pressed) OR ACIA has thrown an int)
	      ||||\---- remote mode selected (0 = selected, 1 = not) \
	      |||\----- key mode selected (0 = selected, 1 = not)     > if all 3 of these are 1, unit is going to standby
	      ||\------ sim mode selected (0 = selected, 1 = not)    /
	      |\------- power failure status (1 = power has failed, 0 = ok; writes to R44 set this to ok)
	      \-------- chip insert detect state 'PIN' (1 = no chip or cmos chip which ammeter cannot detect; 0 = nmos or detectable chip inserted)
	 after power failure (in key mode):
	 0xEE 11101110 when no keypad key pressed
	 0xEA 11101010 when keypad key pressed
	 in key mode:
	 0xAE 10101110 when no keypad key pressed
	 0xAA 10101010 when keypad key pressed
	 in remote mode:
	 0xB6 10110110 when no keypad key pressed
	 0xB2 10110010 when keypad key pressed
	 in sim mode:
	 0x9E 10011110 when no keypad key pressed
	 0x9A 10011010 when keypad key pressed
	 in off mode (before z80 is powered down):
	 0xFE 11111110

	*/

#ifdef PORT43_R_VERBOSE
	logerror("Digel804: returning %02X for port 43 status read\n", port43_rtn);
#endif
	return ((m_overload_state<<0)
		|((ioport("DEBUG")->read()&1)<<1)
		|((m_key_intq&m_acia_intq)<<2)
		|(m_remote_mode<<3)
		|(m_key_mode<<4)
		|(m_sim_mode<<5)
		|(m_powerfail_state<<6)
		|(m_chipinsert_state<<7));

}

WRITE8_MEMBER( digel804_state::op00 )
{
	m_ram_bank = data;
	m_rambank->set_base(m_ram->pointer() + ((m_ram_bank * 0x8000) & m_ram->mask()));
}

WRITE8_MEMBER( digel804_state::op43 )
{
	/* writes to 0x43 control the ram banking on firmware which supports it
	 * bits:76543210
	 *      |||||\\\- select ram bank for 4000-bfff area based on these bits (2.0 hardware)
	 *      \\\\\\\-- CTL lines

	 * all writes to port 43 will reset the overload state unless the ammeter detects the overload is ongoing
	 */
	m_overload_state = 0; // writes to port 43 clear overload state
#ifdef PORT43_W_VERBOSE
	logerror("Digel804: port 0x43 ram bank had %02x written to it!\n", data);
	logerror("          op41 bit 6 is %d\n", (m_op41 & 0x40)?1:0);
#endif
	m_ram_bank = data&7;
	if ((data&0xF8)!=0)
		logerror("Digel804: port 0x43 ram bank had unexpected data %02x written to it!\n", data);

	m_rambank->set_base(m_ram->pointer() + ((m_ram_bank * 0x8000) & m_ram->mask()));
}

WRITE8_MEMBER( digel804_state::op43_1_4 )
{
	m_overload_state = 0; // writes to port 43 clear overload state
}

WRITE8_MEMBER( digel804_state::op44 ) // state write
{
	/* writes to 0x44 control the 10937 vfd chip, z80 power/busrq, eprom driving and some eprom power ctl lines
	 * bits:76543210
	 *      |||||||\- 10937 VFDC '/SCK' serial clock '/CDIS'
	 *      ||||||\-- controls '/KEYEN' (which enables the four mode buttons when active)
	 *      |||||\--- z80 and system power control (0 = power on, 1 = power off/standby), also controls '/MEMEN' which secondarily controls POR(power on/reset) for the VFDC chip
	 *      ||||\---- controls the z80 /BUSRQ line (0 = idle/high, 1 = asserted/low) '/BRQ'
	 *      |||\----- when 1, enables the output drivers of op40 to the rom data pins
	 *      ||\------ controls 'CTL8'
	 *      |\------- controls 'CTL9'
	 *      \-------- 10937 VFDC 'DATA' serial data '/DDIS'

	 * all writes to port 44 will reset the powerfail state
	 */
	m_powerfail_state = 0; // writes to port 44 clear powerfail state
	m_keyen_state = BIT(data, 1);
#ifdef PORT44_W_VERBOSE
	logerror("Digel804: port 0x44 vfd/state control had %02x written to it!\n", data);
#endif
	m_vfd->por(!(data&0x04));
	m_vfd->data(data&0x80);
	m_vfd->sclk(data&1);
}

WRITE8_MEMBER( digel804_state::op45 ) // speaker write
{
	// all writes to here invert the speaker state, verified from schematics
#ifdef PORT45_W_VERBOSE
	logerror("Digel804: port 0x45 speaker had %02x written to it!\n", data);
#endif
	m_speaker_state ^= 0xFF;
	m_speaker->level_w(m_speaker_state);
}

READ8_MEMBER( digel804_state::ip46 ) // keypad read
{
	/* reads E* for a keypad number 0-F
	 * reads F0 for enter
	 * reads F4 for next
	 * reads F8 for rept
	 * reads FC for clear
	 * F* takes precedence over E*
	 * higher numbers take precedence over lower ones
	 * this value auto-latches on a key press and remains through multiple reads
	 * this is done by a 74C923 integrated circuit
	*/
	UINT8 kbd = m_kb->read();
#ifdef PORT46_R_VERBOSE
	logerror("Digel804: returning %02X for port 46 keypad read\n", kbd);
#endif

	return BITSWAP8(kbd,7,6,5,4,1,0,3,2);   // verified from schematics
}

WRITE8_MEMBER( digel804_state::op46 )
{
	/* writes to 0x46 control the LEDS on the front panel
	 * bits:76543210
	 *      ||||\\\\- these four bits choose which of the 16 function leds is lit; the number is INVERTED first
	 *      |||\----- if this bit is 1, the function leds are disabled
	 *      ||\------ this bit controls the 'error' led; 1 = on
	 *      |\------- this bit controls the 'busy' led; 1 = on
	 *      \-------- this bit controls the 'input' led; 1 = on
	 */
#ifdef PORT46_W_VERBOSE
		logerror("Digel804: port 0x46 LED control had %02x written to it!\n", data);
#endif
		//popmessage("LEDS: %s %s %s Func: %s%d\n", (data&0x80)?"INPUT":"-----", (data&0x40)?"BUSY":"----", (data&0x20)?"ERROR":"-----", (data&0x10)?"None":"", (data&0x10)?-1:(~data&0xF));
		//fprintf("LEDS: %s %s %s Func: %s%d\n", (data&0x80)?"INPUT":"-----", (data&0x40)?"BUSY":"----", (data&0x20)?"ERROR":"-----", (data&0x10)?"None":"", (data&0x10)?-1:(~data&0xF));

	output().set_value("input_led", BIT(data,7));
	output().set_value("busy_led",  BIT(data,6));
	output().set_value("error_led", BIT(data,5));

	for(int i=0; i<16; i++)
		output().set_indexed_value("func_led", i, (!(data & 0x10) && ((~data & 0x0f) == i)) ? 1 : 0);
}

WRITE8_MEMBER( digel804_state::op47 ) // eprom timing/power and control write
{
	// TODO: would be nice to have a 'fake eprom' here
	logerror("Digel804: port 47, eprom timing/power and control had %02X written to it!\n", data);
}

INPUT_CHANGED_MEMBER( digel804_state::mode_change )
{
	if (!newval && !m_keyen_state)
	{
		switch ((int)(FPTR)param)
		{
			case MODE_OFF:
				m_key_mode = m_remote_mode = m_sim_mode = 1;
				break;
			case MODE_KEY:
				m_remote_mode = m_sim_mode = 1;
				m_key_mode = 0;
				break;
			case MODE_REM:
				m_key_mode = m_sim_mode = 1;
				m_remote_mode = 0;
				break;
			case MODE_SIM:
				m_remote_mode = m_key_mode = 1;
				m_sim_mode = 0;
				break;
		}

		m_acia->reset();
	}

	// press one of those keys reset the Z80
	m_maincpu->set_input_line(INPUT_LINE_RESET, (!newval && !m_keyen_state) ? ASSERT_LINE : CLEAR_LINE);
}

/* ACIA Trampolines */
READ8_MEMBER( digel804_state::acia_rxd_r )
{
	return m_acia->read(space, 0);
}

WRITE8_MEMBER( digel804_state::acia_txd_w )
{
	m_acia->write(space, 0, data);
}

READ8_MEMBER( digel804_state::acia_status_r )
{
	return m_acia->read(space, 1);
}

WRITE8_MEMBER( digel804_state::acia_reset_w )
{
	m_acia->write(space, 1, data);
}

READ8_MEMBER( digel804_state::acia_command_r )
{
	return m_acia->read(space, 2);
}

WRITE8_MEMBER( digel804_state::acia_command_w )
{
	data |= 0x08;   // HACK for ep804 remote mode

	m_acia->write(space, 2, data);
}

READ8_MEMBER( digel804_state::acia_control_r )
{
	return m_acia->read(space, 3);
}

WRITE8_MEMBER( digel804_state::acia_control_w )
{
	m_acia->write(space, 3, data);
}


/******************************************************************************
 Address Maps
******************************************************************************/

static ADDRESS_MAP_START(z80_mem_804_1_4, AS_PROGRAM, 8, digel804_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM // 3f in mapper = rom J3
	//AM_RANGE(0x4000, 0x5fff) AM_RAM AM_SHARE("main_ram") // 6f in mapper = RAM D43 (6164)
	//AM_RANGE(0x6000, 0x7fff) AM_RAM AM_SHARE("main_ram") // 77 in mapper = RAM D44 (6164)
	//AM_RANGE(0x8000, 0x9fff) AM_RAM AM_SHARE("main_ram") // 7b in mapper = RAM D45 (6164)
	//AM_RANGE(0xa000, 0xbfff) AM_RAM AM_SHARE("main_ram") // 7d in mapper = RAM D46 (6164)
	AM_RANGE(0x4000,0xbfff) AM_RAMBANK("bankedram")
	// c000-cfff is open bus in mapper, 7f
	AM_RANGE(0xd000, 0xd7ff) AM_RAM // 7e in mapper = RAM P3 (6116)
	AM_RANGE(0xd800, 0xf7ff) AM_ROM // 5f in mapper = rom K3
	// f800-ffff is open bus in mapper, 7f
ADDRESS_MAP_END

static ADDRESS_MAP_START(z80_mem_804_1_2, AS_PROGRAM, 8, digel804_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM // 3f in mapper = rom D41
	AM_RANGE(0x2000, 0x3fff) AM_ROM // 5f in mapper = rom D42
	//AM_RANGE(0x4000, 0x5fff) AM_RAM AM_SHARE("main_ram") // 6f in mapper = RAM D43 (6164)
	//AM_RANGE(0x6000, 0x7fff) AM_RAM AM_SHARE("main_ram") // 77 in mapper = RAM D44 (6164)
	//AM_RANGE(0x8000, 0x9fff) AM_RAM AM_SHARE("main_ram") // 7b in mapper = RAM D45 (6164)
	//AM_RANGE(0xa000, 0xbfff) AM_RAM AM_SHARE("main_ram") // 7d in mapper = RAM D46 (6164)
	AM_RANGE(0x4000,0xbfff) AM_RAMBANK("bankedram")
	// c000-cfff is open bus in mapper, 7f
	//AM_RANGE(0xc000, 0xc7ff) AM_RAM // hack for now to test, since sometimes it writes to c3ff
	AM_RANGE(0xd000, 0xd7ff) AM_RAM // 7e in mapper = RAM D47 (6116)
	// d800-ffff is open bus in mapper, 7f
ADDRESS_MAP_END

static ADDRESS_MAP_START(z80_io_1_4, AS_IO, 8, digel804_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	// io bits: x 1 x x x * * *
	// writes to 47, 4e, 57, 5e, 67, 6e, 77, 7e, c7, ce, d7, de, e7, ee, f7, fe all go to 47, same with reads
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x38) AM_WRITE(op00) // W, banked ram
	AM_RANGE(0x40, 0x40) AM_MIRROR(0xB8) AM_READWRITE(ip40, op40) // RW, eprom socket data bus input/output value
	AM_RANGE(0x41, 0x41) AM_MIRROR(0xB8) AM_WRITE(op41) // W, eprom socket address low out
	AM_RANGE(0x42, 0x42) AM_MIRROR(0xB8) AM_WRITE(op42) // W, eprom socket address hi/control out
	AM_RANGE(0x43, 0x43) AM_MIRROR(0xB8) AM_READWRITE(ip43, op43_1_4) // RW, mode and status register, also checks if keypad is pressed; write is unknown
	AM_RANGE(0x44, 0x44) AM_MIRROR(0xB8) AM_WRITE(op44) // W, 10937 vfd controller, z80 power and state control reg
	AM_RANGE(0x45, 0x45) AM_MIRROR(0xB8) AM_WRITE(op45) // W, speaker bit; every write inverts state
	AM_RANGE(0x46, 0x46) AM_MIRROR(0xB8) AM_READWRITE(ip46, op46) // RW, reads keypad, writes controls the front panel leds.
	AM_RANGE(0x47, 0x47) AM_MIRROR(0xB8) AM_WRITE(op47) // W eprom socket power and timing control
	// io bits: 1 0 x x x * * *
	// writes to 80, 88, 90, 98, a0, a8, b0, b8 all go to 80, same with reads
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x38) AM_WRITE(acia_txd_w) // (ACIA xmit reg)
	AM_RANGE(0x81, 0x81) AM_MIRROR(0x38) AM_READ(acia_rxd_r) // (ACIA rcv reg)
	AM_RANGE(0x82, 0x82) AM_MIRROR(0x38) AM_WRITE(acia_reset_w) // (ACIA reset reg)
	AM_RANGE(0x83, 0x83) AM_MIRROR(0x38) AM_READ(acia_status_r) // (ACIA status reg)
	AM_RANGE(0x84, 0x84) AM_MIRROR(0x38) AM_WRITE(acia_command_w) // (ACIA command reg)
	AM_RANGE(0x85, 0x85) AM_MIRROR(0x38) AM_READ(acia_command_r) // (ACIA command reg)
	AM_RANGE(0x86, 0x86) AM_MIRROR(0x38) AM_WRITE(acia_control_w) // (ACIA control reg)
	AM_RANGE(0x87, 0x87) AM_MIRROR(0x38) AM_READ(acia_control_r) // (ACIA control reg)
	//AM_RANGE(0x80,0x87) AM_MIRROR(0x38) AM_SHIFT(-1) AM_DEVREADWRITE("acia", mos6551_device, read, write) // this doesn't work since we lack an AM_SHIFT command

ADDRESS_MAP_END

static ADDRESS_MAP_START(z80_io_1_2, AS_IO, 8, digel804_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	// io bits: x 1 x x x * * *
	// writes to 47, 4e, 57, 5e, 67, 6e, 77, 7e, c7, ce, d7, de, e7, ee, f7, fe all go to 47, same with reads
	AM_RANGE(0x40, 0x40) AM_MIRROR(0xB8) AM_READWRITE(ip40, op40) // RW, eprom socket data bus input/output value
	AM_RANGE(0x41, 0x41) AM_MIRROR(0xB8) AM_WRITE(op41) // W, eprom socket address low out
	AM_RANGE(0x42, 0x42) AM_MIRROR(0xB8) AM_WRITE(op42) // W, eprom socket address hi/control out
	AM_RANGE(0x43, 0x43) AM_MIRROR(0xB8) AM_READWRITE(ip43, op43) // RW, mode and status register, also checks if keypad is pressed; write is unknown
	AM_RANGE(0x44, 0x44) AM_MIRROR(0xB8) AM_WRITE(op44) // W, 10937 vfd controller, z80 power and state control reg
	AM_RANGE(0x45, 0x45) AM_MIRROR(0xB8) AM_WRITE(op45) // W, speaker bit; every write inverts state
	AM_RANGE(0x46, 0x46) AM_MIRROR(0xB8) AM_READWRITE(ip46, op46) // RW, reads keypad, writes controls the front panel leds.
	AM_RANGE(0x47, 0x47) AM_MIRROR(0xB8) AM_WRITE(op47) // W eprom socket power and timing control
	// io bits: 1 0 x x x * * *
	// writes to 80, 88, 90, 98, a0, a8, b0, b8 all go to 80, same with reads
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x38) AM_WRITE(acia_txd_w) // (ACIA xmit reg)
	AM_RANGE(0x81, 0x81) AM_MIRROR(0x38) AM_READ(acia_rxd_r) // (ACIA rcv reg)
	AM_RANGE(0x82, 0x82) AM_MIRROR(0x38) AM_WRITE(acia_reset_w) // (ACIA reset reg)
	AM_RANGE(0x83, 0x83) AM_MIRROR(0x38) AM_READ(acia_status_r) // (ACIA status reg)
	AM_RANGE(0x84, 0x84) AM_MIRROR(0x38) AM_WRITE(acia_command_w) // (ACIA command reg)
	AM_RANGE(0x85, 0x85) AM_MIRROR(0x38) AM_READ(acia_command_r) // (ACIA command reg)
	AM_RANGE(0x86, 0x86) AM_MIRROR(0x38) AM_WRITE(acia_control_w) // (ACIA control reg)
	AM_RANGE(0x87, 0x87) AM_MIRROR(0x38) AM_READ(acia_control_r) // (ACIA control reg)
	//AM_RANGE(0x80,0x87) AM_MIRROR(0x38) AM_SHIFT(-1) AM_DEVREADWRITE("acia", mos6551_device, read, write) // this doesn't work since we lack an AM_SHIFT command

ADDRESS_MAP_END


/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( digel804 )
	PORT_START("LINE0") /* KEY ROW 0, 'X1' */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTR/INCR") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('^')

	PORT_START("LINE1") /* KEY ROW 1, 'X2' */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NEXT/DECR") PORT_CODE(KEYCODE_DOWN)  PORT_CHAR('V')

	PORT_START("LINE2") /* KEY ROW 2, 'X3' */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REPT") PORT_CODE(KEYCODE_X)  PORT_CHAR('X')

	PORT_START("LINE3") /* KEY ROW 3, 'X4' */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLR") PORT_CODE(KEYCODE_MINUS)   PORT_CHAR('-')

	PORT_START("MODE") // TODO, connects entirely separately from the keypad through some complicated latching logic
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KEY") PORT_CODE(KEYCODE_K)   PORT_CHANGED_MEMBER( DEVICE_SELF, digel804_state, mode_change, MODE_KEY )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REM") PORT_CODE(KEYCODE_R)   PORT_CHANGED_MEMBER( DEVICE_SELF, digel804_state, mode_change, MODE_REM )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SIM") PORT_CODE(KEYCODE_S)   PORT_CHANGED_MEMBER( DEVICE_SELF, digel804_state, mode_change, MODE_SIM )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("OFF") PORT_CODE(KEYCODE_O)   PORT_CHANGED_MEMBER( DEVICE_SELF, digel804_state, mode_change, MODE_OFF )

	PORT_START("DEBUG") // debug jumper on the board
	PORT_DIPNAME( 0x01, 0x01, "Debug Mode" )
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( digel804_rs232_defaults )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_ODD )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

WRITE_LINE_MEMBER( digel804_state::da_w )
{
	m_key_intq = state ? 0 : 1;
	m_maincpu->set_input_line(0, (m_key_intq & m_acia_intq) ? CLEAR_LINE : ASSERT_LINE);
}

WRITE_LINE_MEMBER( digel804_state::acia_irq_w )
{
	m_acia_intq = state ? 0 : 1;
	m_maincpu->set_input_line(0, (m_key_intq & m_acia_intq) ? CLEAR_LINE : ASSERT_LINE);
}

WRITE_LINE_MEMBER( digel804_state::ep804_acia_irq_w )
{
}

static MACHINE_CONFIG_START( digel804, digel804_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_3_6864MHz/2) /* Z80A, X1(aka E0 on schematics): 3.6864Mhz */
	MCFG_CPU_PROGRAM_MAP(z80_mem_804_1_4)
	MCFG_CPU_IO_MAP(z80_io_1_4)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_ROC10937_ADD("vfd",0) // RIGHT_TO_LEFT

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_digel804)

	MCFG_DEVICE_ADD("74c923", MM74C923, 0)
	MCFG_MM74C922_DA_CALLBACK(WRITELINE(digel804_state, da_w))
	MCFG_MM74C922_X1_CALLBACK(IOPORT("LINE0"))
	MCFG_MM74C922_X2_CALLBACK(IOPORT("LINE1"))
	MCFG_MM74C922_X3_CALLBACK(IOPORT("LINE2"))
	MCFG_MM74C922_X4_CALLBACK(IOPORT("LINE3"))

	/* acia */
	MCFG_DEVICE_ADD("acia", MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_3_6864MHz/2)
	MCFG_MOS6551_IRQ_HANDLER(WRITELINE(digel804_state, acia_irq_w))
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_MOS6551_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_MOS6551_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "null_modem")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia", mos6551_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("acia", mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("acia", mos6551_device, write_cts))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("null_modem", digel804_rs232_defaults)
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", digel804_rs232_defaults)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256K")
	MCFG_RAM_EXTRA_OPTIONS("32K,64K,128K")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ep804, digel804 )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")  /* Z80, X1(aka E0 on schematics): 3.6864Mhz */
	MCFG_CPU_PROGRAM_MAP(z80_mem_804_1_2)
	MCFG_CPU_IO_MAP(z80_io_1_2)

	MCFG_DEVICE_MODIFY("acia")
	MCFG_MOS6551_IRQ_HANDLER(WRITELINE(digel804_state, ep804_acia_irq_w))

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_EXTRA_OPTIONS("64K")
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

/*
For the 82S23A/MMI6330-equivalent mapper PROM at D30:

z80 a11 -> prom a0
z80 a12 -> prom a1
z80 a13 -> prom a2
z80 a14 -> prom a3
z80 a15 -> prom a4

prom d0 -> ram 6116 /CE (pin 18?) at d47
prom d1 -> ram 6116/64 /CE (pin 20) at d46
prom d2 -> ram 6116/64 /CE (pin 20) at d45
prom d3 -> ram 6116/64 /CE (pin 20) at d44
prom d4 -> ram 6116/64 /CE (pin 20) at d43
prom d5 -> rom /CE at d42
prom d6 -> rom /CE at d41
prom d7 -> N/C (unused)


On the v2.0 804-0197B mainboard, the added connector X9 (for the ram expansion daughterboard)
near to the mapper prom at D30 has the pinout:
 ___
|1 10
|2 9|
|3 8|
|4 7|
|5_6|

1 -> pin 15 of CD74HC374E at D23 & eprom socket pin A6 (i.e. op41 bit 6)
2 -> N/C
3 -> ram 6116/64 /CE (pin 20) of D45 & mapper prom D2 (pin 3)
4 -> ram 6116/64 /CE (pin 20) of D43 & mapper prom D4 (pin 5)
5 -> CE2 (pin 26) of all four ram chips (D43 D44 D45 D46)
6 -> +5v
7 -> ram 6116/64 /CE (pin 20) of D44 & mapper prom D3 (pin 4)
8 -> ram 6116/64 /CE (pin 20) of D46 & mapper prom D1 (pin 2)
9 -> N/C
10 -> mapper prom /CE (pin 15)

If the ram expansion is not installed (i.e. only 32k of base ram on the mainboard), there is a jumper present between pins 5 and 6
*/

ROM_START(digel804) // address mapper 804-1-4
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("1-04__76f1.27128.d41", 0x0000, 0x4000, CRC(61b50b61) SHA1(ad717fcbf3387b0a8fb0546025d3c527792eb3f0))
	// the second rom here is loaded bizarrely: the first 3/4 appears at e000-f7ff and the last 1/4 appears at d800-dfff
	ROM_LOAD("2-04__d6cc.2764a.d42", 0xe000, 0x1800, CRC(098cb008) SHA1(9f04e12489ab5f714d2fd4af8158969421557e75))
	ROM_CONTINUE(0xd800, 0x800)
	ROM_REGION(0x20, "proms", 0)
	ROM_LOAD("804-1-4.82s23a.d30", 0x0000, 0x0020, CRC(f961beb1) SHA1(f2ec89375e656eeabc30246d42741cf718fb0f91)) // Address mapper prom, 82s23/mmi6330/tbp18sa030 equivalent 32x8 open collector
ROM_END

ROM_START(ep804) // address mapper 804-1-2
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_DEFAULT_BIOS("ep804_v1.6")
	ROM_SYSTEM_BIOS( 0, "ep804_v1.6", "Wavetek/Digelec EP804 FWv1.6") // hardware 1.1
	ROMX_LOAD("804-2__rev_1.6__07f7.hn482764g-4.d41", 0x0000, 0x2000, CRC(2d4c334c) SHA1(1be70ed5f4f315a8d2e38a17327a049e00fa174e), ROM_BIOS(1))
	ROMX_LOAD("804-3__rev_1.6__265c.hn482764g-4.d42", 0x2000, 0x2000, CRC(9c14906b) SHA1(41996039e604011c7c0f757397f82b6479ee3f62), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "ep804_v1.4", "Wavetek/Digelec EP804 FWv1.4") // hardware 1.1
	ROMX_LOAD("804-2_rev_1.4__7a7e.hn482764g-4.d41", 0x0000, 0x2000, CRC(fdc0d2e3) SHA1(da1bc1e8c4cb2a2d8cd2273f3e1a9f318ae8cb87), ROM_BIOS(2))
	ROMX_LOAD("804-3_rev_1.4__f240.2732.d42", 0x2000, 0x1000, CRC(29827e29) SHA1(4c7fadf81bcf32349a564d946f5d215de50315c5), ROM_BIOS(2))
	ROMX_LOAD("804-3_rev_1.4__f240.2732.d42", 0x3000, 0x1000, CRC(29827e29) SHA1(4c7fadf81bcf32349a564d946f5d215de50315c5), ROM_BIOS(2)) // load this twice
	ROM_SYSTEM_BIOS( 2, "ep804_v2.21", "Wavetek/Digelec EP804 FWv2.21") // hardware 1.5 NOTE: this may use the address mapper 804-1-3 which is not dumped!
	ROMX_LOAD("804-2_rev2.21__cs_ab50.hn482764g.d41", 0x0000, 0x2000, CRC(ffbc95f6) SHA1(b12aa97e23d546064f1d17aa9b90772017fec5ec), ROM_BIOS(3))
	ROMX_LOAD("804-3_rev2.21__cs_6b98.hn482764g.d42", 0x2000, 0x2000, CRC(a4acb9fe) SHA1(bbc7e3e2e6b3b1abe747380909dcddc985ef8d0d), ROM_BIOS(3))
	ROM_REGION(0x20, "proms", 0)
	ROM_LOAD("804-1-2.mmi_6330-in.d30", 0x0000, 0x0020, CRC(30dd4721) SHA1(e4b2f5756118be4c8ab56c708dc4f42469c7e51b)) // Address mapper prom, 82s23/mmi6330/tbp18sa030 equivalent 32x8 open collector
ROM_END



/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT      COMPANY                     FULLNAME                                                    FLAGS */
COMP( 1985, digel804,   0,          0,      digel804,   digel804, digel804_state, digel804,      "Digelec, Inc",   "Digelec 804 EPROM Programmer", MACHINE_NOT_WORKING )
COMP( 1982, ep804,   digel804,          0,      ep804,   digel804, digel804_state, digel804,      "Wavetek/Digelec, Inc",   "EP804 EPROM Programmer", MACHINE_NOT_WORKING )
