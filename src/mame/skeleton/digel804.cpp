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

/* Core includes */
#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/mm74c922.h"
#include "machine/mos6551.h"
#include "machine/ram.h"
#include "machine/roc10937.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#include "digel804.lh"


namespace {

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


class digel804_state : public driver_device
{
public:
	digel804_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_ram(*this, RAM_TAG),
		m_maincpu(*this, "maincpu"),
		m_acia(*this, "acia"),
		m_speaker(*this, "speaker"),
		m_vfd(*this, "vfd"),
		m_kb(*this, "74c923"),
		m_rambank(*this, "bankedram"),
		m_func_leds(*this, "func_led%u", 0U),
		m_input_led(*this, "input_led"),
		m_busy_led(*this, "busy_led"),
		m_error_led(*this, "error_led")
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(mode_change);

	void digel804(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void op00(uint8_t data);
	uint8_t ip40();
	void op40(uint8_t data);
	void op41(uint8_t data);
	void op42(uint8_t data);
	uint8_t ip43();
	void op43(uint8_t data);
	void op43_1_4(uint8_t data);
	void op44(uint8_t data);
	void op45(uint8_t data);
	uint8_t ip46();
	void op46(uint8_t data);
	void op47(uint8_t data);
	uint8_t acia_rxd_r();
	void acia_txd_w(uint8_t data);
	uint8_t acia_status_r();
	void acia_reset_w(uint8_t data);
	uint8_t acia_command_r();
	void acia_command_w(uint8_t data);
	uint8_t acia_control_r();
	void acia_control_w(uint8_t data);
	void acia_irq_w(int state);
	void da_w(int state);

	void z80_mem_804_1_4(address_map &map) ATTR_COLD;
	void z80_io_1_4(address_map &map) ATTR_COLD;

	required_device<ram_device> m_ram;
	required_device<cpu_device> m_maincpu;
	required_device<mos6551_device> m_acia;

private:
	required_device<speaker_sound_device> m_speaker;
	required_device<roc10937_device> m_vfd;
	required_device<mm74c922_device> m_kb;
	required_memory_bank m_rambank;

	output_finder<16> m_func_leds;
	output_finder<> m_input_led;
	output_finder<> m_busy_led;
	output_finder<> m_error_led;

	// current speaker state for port 45
	uint8_t m_speaker_state;
	// ram stuff for banking
	uint8_t m_ram_bank;
	// states
	uint8_t m_acia_intq;
	uint8_t m_overload_state;
	uint8_t m_key_intq;
	uint8_t m_remote_mode;
	uint8_t m_key_mode;
	uint8_t m_sim_mode;
	uint8_t m_powerfail_state;
	uint8_t m_chipinsert_state;
	uint8_t m_keyen_state;
	uint8_t m_op41;
};


class ep804_state : public digel804_state
{
public:
	using digel804_state::digel804_state;

	void ep804(machine_config &config);

protected:
	void ep804_acia_irq_w(int state);

	void z80_mem_804_1_2(address_map &map) ATTR_COLD;
	void z80_io_1_2(address_map &map) ATTR_COLD;
};


enum { MODE_OFF, MODE_KEY, MODE_REM, MODE_SIM };


void digel804_state::machine_start()
{
	m_func_leds.resolve();
	m_input_led.resolve();
	m_busy_led.resolve();
	m_error_led.resolve();

	save_item(NAME(m_speaker_state));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_acia_intq));
	save_item(NAME(m_overload_state));
	save_item(NAME(m_key_intq));
	save_item(NAME(m_remote_mode));
	save_item(NAME(m_key_mode));
	save_item(NAME(m_sim_mode));
	save_item(NAME(m_powerfail_state));
	save_item(NAME(m_chipinsert_state));
	save_item(NAME(m_keyen_state));
	save_item(NAME(m_op41));

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

uint8_t digel804_state::ip40() // eprom data bus read
{
	// TODO: would be nice to have a 'fake eprom' here
	return 0xFF;
}

void digel804_state::op40(uint8_t data) // eprom data bus write
{
	// TODO: would be nice to have a 'fake eprom' here
	logerror("Digel804: port 40, eprom databus had %02X written to it!\n", data);
}

void digel804_state::op41(uint8_t data) // eprom address low write AND SIM write, d6 also controls memory map somehow
{
	// TODO: would be nice to have a 'fake eprom' here
	logerror("Digel804: port 41, eprom address low/sim/memorybank had %02X written to it!\n", data);
	//fprintf(stderr,"op41 write of %02X, ram mapper bit is %d\n", data, (data&0x40)?1:0);
	m_op41 = data;
}

void digel804_state::op42(uint8_t data) // eprom address hi and control write
{
	// TODO: would be nice to have a 'fake eprom' here
	logerror("Digel804: port 42, eprom address hi/control had %02X written to it!\n", data);
}

uint8_t digel804_state::ip43()
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

void digel804_state::op00(uint8_t data)
{
	m_ram_bank = data;
	m_rambank->set_base(m_ram->pointer() + ((m_ram_bank * 0x8000) & m_ram->mask()));
}

void digel804_state::op43(uint8_t data)
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

void digel804_state::op43_1_4(uint8_t data)
{
	m_overload_state = 0; // writes to port 43 clear overload state
}

void digel804_state::op44(uint8_t data) // state write
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

void digel804_state::op45(uint8_t data) // speaker write
{
	// all writes to here invert the speaker state, verified from schematics
#ifdef PORT45_W_VERBOSE
	logerror("Digel804: port 0x45 speaker had %02x written to it!\n", data);
#endif
	m_speaker_state ^= 0xFF;
	m_speaker->level_w(m_speaker_state);
}

uint8_t digel804_state::ip46() // keypad read
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
	uint8_t kbd = m_kb->read();
#ifdef PORT46_R_VERBOSE
	logerror("Digel804: returning %02X for port 46 keypad read\n", kbd);
#endif

	return bitswap<8>(kbd,7,6,5,4,1,0,3,2);   // verified from schematics
}

void digel804_state::op46(uint8_t data)
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

	m_input_led = BIT(data,7);
	m_busy_led  = BIT(data,6);
	m_error_led = BIT(data,5);

	for (int i = 0; i < 16; i++)
		m_func_leds[i] = (!(data & 0x10) && ((~data & 0x0f) == i)) ? 1 : 0;
}

void digel804_state::op47(uint8_t data) // eprom timing/power and control write
{
	// TODO: would be nice to have a 'fake eprom' here
	logerror("Digel804: port 47, eprom timing/power and control had %02X written to it!\n", data);
}

INPUT_CHANGED_MEMBER( digel804_state::mode_change )
{
	if (!newval && !m_keyen_state)
	{
		switch (param)
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
uint8_t digel804_state::acia_rxd_r()
{
	return m_acia->read(0);
}

void digel804_state::acia_txd_w(uint8_t data)
{
	m_acia->write(0, data);
}

uint8_t digel804_state::acia_status_r()
{
	return m_acia->read(1);
}

void digel804_state::acia_reset_w(uint8_t data)
{
	m_acia->write(1, data);
}

uint8_t digel804_state::acia_command_r()
{
	return m_acia->read(2);
}

void digel804_state::acia_command_w(uint8_t data)
{
	data |= 0x08;   // HACK for ep804 remote mode

	m_acia->write(2, data);
}

uint8_t digel804_state::acia_control_r()
{
	return m_acia->read(3);
}

void digel804_state::acia_control_w(uint8_t data)
{
	m_acia->write(3, data);
}


/******************************************************************************
 Address Maps
******************************************************************************/

void digel804_state::z80_mem_804_1_4(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom(); // 3f in mapper = rom J3
	//map(0x4000, 0x5fff).ram().share("main_ram"); // 6f in mapper = RAM D43 (6164)
	//map(0x6000, 0x7fff).ram().share("main_ram"); // 77 in mapper = RAM D44 (6164)
	//map(0x8000, 0x9fff).ram().share("main_ram"); // 7b in mapper = RAM D45 (6164)
	//map(0xa000, 0xbfff).ram().share("main_ram"); // 7d in mapper = RAM D46 (6164)
	map(0x4000, 0xbfff).bankrw("bankedram");
	// c000-cfff is open bus in mapper, 7f
	map(0xd000, 0xd7ff).ram(); // 7e in mapper = RAM P3 (6116)
	map(0xd800, 0xf7ff).rom(); // 5f in mapper = rom K3
	// f800-ffff is open bus in mapper, 7f
}

void ep804_state::z80_mem_804_1_2(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom(); // 3f in mapper = rom D41
	map(0x2000, 0x3fff).rom(); // 5f in mapper = rom D42
	//map(0x4000, 0x5fff).ram().share("main_ram"); // 6f in mapper = RAM D43 (6164)
	//map(0x6000, 0x7fff).ram().share("main_ram"); // 77 in mapper = RAM D44 (6164)
	//map(0x8000, 0x9fff).ram().share("main_ram"); // 7b in mapper = RAM D45 (6164)
	//map(0xa000, 0xbfff).ram().share("main_ram"); // 7d in mapper = RAM D46 (6164)
	map(0x4000, 0xbfff).bankrw("bankedram");
	// c000-cfff is open bus in mapper, 7f
	//map(0xc000, 0xc7ff).ram(); // hack for now to test, since sometimes it writes to c3ff
	map(0xd000, 0xd7ff).ram(); // 7e in mapper = RAM D47 (6116)
	// d800-ffff is open bus in mapper, 7f
}

void digel804_state::z80_io_1_4(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	// io bits: x 1 x x x * * *
	// writes to 47, 4e, 57, 5e, 67, 6e, 77, 7e, c7, ce, d7, de, e7, ee, f7, fe all go to 47, same with reads
	map(0x00, 0x00).mirror(0x38).w(FUNC(digel804_state::op00)); // W, banked ram
	map(0x40, 0x40).mirror(0xB8).rw(FUNC(digel804_state::ip40), FUNC(digel804_state::op40)); // RW, eprom socket data bus input/output value
	map(0x41, 0x41).mirror(0xB8).w(FUNC(digel804_state::op41)); // W, eprom socket address low out
	map(0x42, 0x42).mirror(0xB8).w(FUNC(digel804_state::op42)); // W, eprom socket address hi/control out
	map(0x43, 0x43).mirror(0xB8).rw(FUNC(digel804_state::ip43), FUNC(digel804_state::op43_1_4)); // RW, mode and status register, also checks if keypad is pressed; write is unknown
	map(0x44, 0x44).mirror(0xB8).w(FUNC(digel804_state::op44)); // W, 10937 vfd controller, z80 power and state control reg
	map(0x45, 0x45).mirror(0xB8).w(FUNC(digel804_state::op45)); // W, speaker bit; every write inverts state
	map(0x46, 0x46).mirror(0xB8).rw(FUNC(digel804_state::ip46), FUNC(digel804_state::op46)); // RW, reads keypad, writes controls the front panel leds.
	map(0x47, 0x47).mirror(0xB8).w(FUNC(digel804_state::op47)); // W eprom socket power and timing control
	// io bits: 1 0 x x x * * *
	// writes to 80, 88, 90, 98, a0, a8, b0, b8 all go to 80, same with reads
	map(0x80, 0x80).mirror(0x38).w(FUNC(digel804_state::acia_txd_w)); // (ACIA xmit reg)
	map(0x81, 0x81).mirror(0x38).r(FUNC(digel804_state::acia_rxd_r)); // (ACIA rcv reg)
	map(0x82, 0x82).mirror(0x38).w(FUNC(digel804_state::acia_reset_w)); // (ACIA reset reg)
	map(0x83, 0x83).mirror(0x38).r(FUNC(digel804_state::acia_status_r)); // (ACIA status reg)
	map(0x84, 0x84).mirror(0x38).w(FUNC(digel804_state::acia_command_w)); // (ACIA command reg)
	map(0x85, 0x85).mirror(0x38).r(FUNC(digel804_state::acia_command_r)); // (ACIA command reg)
	map(0x86, 0x86).mirror(0x38).w(FUNC(digel804_state::acia_control_w)); // (ACIA control reg)
	map(0x87, 0x87).mirror(0x38).r(FUNC(digel804_state::acia_control_r)); // (ACIA control reg)
	//map(0x80,0x87).mirror(0x38).shift(-1).rw("acia", FUNC(mos6551_device::read, FUNC(mos6551_device::write)); // this doesn't work since we lack a shift() command

}

void ep804_state::z80_io_1_2(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	// io bits: x 1 x x x * * *
	// writes to 47, 4e, 57, 5e, 67, 6e, 77, 7e, c7, ce, d7, de, e7, ee, f7, fe all go to 47, same with reads
	map(0x40, 0x40).mirror(0xB8).rw(FUNC(ep804_state::ip40), FUNC(ep804_state::op40)); // RW, eprom socket data bus input/output value
	map(0x41, 0x41).mirror(0xB8).w(FUNC(ep804_state::op41)); // W, eprom socket address low out
	map(0x42, 0x42).mirror(0xB8).w(FUNC(ep804_state::op42)); // W, eprom socket address hi/control out
	map(0x43, 0x43).mirror(0xB8).rw(FUNC(ep804_state::ip43), FUNC(ep804_state::op43)); // RW, mode and status register, also checks if keypad is pressed; write is unknown
	map(0x44, 0x44).mirror(0xB8).w(FUNC(ep804_state::op44)); // W, 10937 vfd controller, z80 power and state control reg
	map(0x45, 0x45).mirror(0xB8).w(FUNC(ep804_state::op45)); // W, speaker bit; every write inverts state
	map(0x46, 0x46).mirror(0xB8).rw(FUNC(ep804_state::ip46), FUNC(ep804_state::op46)); // RW, reads keypad, writes controls the front panel leds.
	map(0x47, 0x47).mirror(0xB8).w(FUNC(ep804_state::op47)); // W eprom socket power and timing control
	// io bits: 1 0 x x x * * *
	// writes to 80, 88, 90, 98, a0, a8, b0, b8 all go to 80, same with reads
	map(0x80, 0x80).mirror(0x38).w(FUNC(ep804_state::acia_txd_w)); // (ACIA xmit reg)
	map(0x81, 0x81).mirror(0x38).r(FUNC(ep804_state::acia_rxd_r)); // (ACIA rcv reg)
	map(0x82, 0x82).mirror(0x38).w(FUNC(ep804_state::acia_reset_w)); // (ACIA reset reg)
	map(0x83, 0x83).mirror(0x38).r(FUNC(ep804_state::acia_status_r)); // (ACIA status reg)
	map(0x84, 0x84).mirror(0x38).w(FUNC(ep804_state::acia_command_w)); // (ACIA command reg)
	map(0x85, 0x85).mirror(0x38).r(FUNC(ep804_state::acia_command_r)); // (ACIA command reg)
	map(0x86, 0x86).mirror(0x38).w(FUNC(ep804_state::acia_control_w)); // (ACIA control reg)
	map(0x87, 0x87).mirror(0x38).r(FUNC(ep804_state::acia_control_r)); // (ACIA control reg)
	//map(0x80,0x87).mirror(0x38).shift(-1).rw("acia", FUNC(mos6551_device::read, FUNC(mos6551_device::write)); // this doesn't work since we lack a shift() command

}


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
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_ODD )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

void digel804_state::da_w(int state)
{
	m_key_intq = state ? 0 : 1;
	m_maincpu->set_input_line(0, (m_key_intq & m_acia_intq) ? CLEAR_LINE : ASSERT_LINE);
}

void digel804_state::acia_irq_w(int state)
{
	m_acia_intq = state ? 0 : 1;
	m_maincpu->set_input_line(0, (m_key_intq & m_acia_intq) ? CLEAR_LINE : ASSERT_LINE);
}

void ep804_state::ep804_acia_irq_w(int state)
{
}

void digel804_state::digel804(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3.6864_MHz_XTAL/2); /* Z80A, X1(aka E0 on schematics): 3.6864Mhz */
	m_maincpu->set_addrmap(AS_PROGRAM, &digel804_state::z80_mem_804_1_4);
	m_maincpu->set_addrmap(AS_IO, &digel804_state::z80_io_1_4);
	config.set_maximum_quantum(attotime::from_hz(60));

	ROC10937(config, m_vfd); // RIGHT_TO_LEFT

	/* video hardware */
	config.set_default_layout(layout_digel804);

	MM74C923(config, m_kb, 0);
	m_kb->da_wr_callback().set(FUNC(digel804_state::da_w));
	m_kb->x1_rd_callback().set_ioport("LINE0");
	m_kb->x2_rd_callback().set_ioport("LINE1");
	m_kb->x3_rd_callback().set_ioport("LINE2");
	m_kb->x4_rd_callback().set_ioport("LINE3");

	/* acia */
	MOS6551(config, m_acia, 0);
	m_acia->set_xtal(3.6864_MHz_XTAL/2);
	m_acia->irq_handler().set(FUNC(digel804_state::acia_irq_w));
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_acia->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "null_modem"));
	rs232.rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	rs232.dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));
	rs232.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(digel804_rs232_defaults));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(digel804_rs232_defaults));

	RAM(config, m_ram).set_default_size("256K").set_extra_options("32K,64K,128K");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

void ep804_state::ep804(machine_config &config)
{
	digel804(config);

	/* basic machine hardware */
	/* Z80, X1(aka E0 on schematics): 3.6864Mhz */
	m_maincpu->set_addrmap(AS_PROGRAM, &ep804_state::z80_mem_804_1_2);
	m_maincpu->set_addrmap(AS_IO, &ep804_state::z80_io_1_2);

	m_acia->irq_handler().set(FUNC(ep804_state::ep804_acia_irq_w));

	m_ram->set_default_size("32K").set_extra_options("64K");
}



/******************************************************************************
 ROM Definitions
******************************************************************************/

/*
"Hardware Revisions"
For pcb 1.0, there are at least 6 hardware revisions (i.e. small changes/component changes/greenwire fixes).
The hardware revision is shown on the sticker on the bottom.

Known features:

1.0 - ?
1.1 - does not support driving pin 1 of the socket, i.e. max size is 27256; pin 1 is pulled high; several components unpopulated
1.2 - ?
1.3 - ?
1.4 - ?
1.5 - does support driving pin 1
1.6 - does support driving pin 1

*/

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

ROM_START(digel804) // pcb v2.0; address mapper 804-1-4
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("1-04__76f1.27128.d41", 0x0000, 0x4000, CRC(61b50b61) SHA1(ad717fcbf3387b0a8fb0546025d3c527792eb3f0))
	// the second rom here is loaded bizarrely: the first 3/4 appears at e000-f7ff and the last 1/4 appears at d800-dfff
	ROM_LOAD("2-04__d6cc.2764a.d42", 0xe000, 0x1800, CRC(098cb008) SHA1(9f04e12489ab5f714d2fd4af8158969421557e75))
	ROM_CONTINUE(0xd800, 0x800)
	ROM_REGION(0x20, "proms", 0)
	ROM_LOAD("804-1-4.82s23a.d30", 0x0000, 0x0020, CRC(f961beb1) SHA1(f2ec89375e656eeabc30246d42741cf718fb0f91)) // Address mapper prom, 82s23/mmi6330/tbp18sa030 equivalent 32x8 open collector
ROM_END

ROM_START(ep804) // pcb v1.0; address mapper 804-1-2
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_DEFAULT_BIOS("ep804_v1.6")
	ROM_SYSTEM_BIOS( 0, "ep804_v1.6", "Wavetek/Digelec EP804 FWv1.6") // hardware 1.1
	ROMX_LOAD("804-2__rev_1.6__07f7.hn482764g-4.d41", 0x0000, 0x2000, CRC(2d4c334c) SHA1(1be70ed5f4f315a8d2e38a17327a049e00fa174e), ROM_BIOS(0))
	ROMX_LOAD("804-3__rev_1.6__265c.hn482764g-4.d42", 0x2000, 0x2000, CRC(9c14906b) SHA1(41996039e604011c7c0f757397f82b6479ee3f62), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "ep804_v1.4", "Wavetek/Digelec EP804 FWv1.4") // hardware 1.1
	ROMX_LOAD("804-2_rev_1.4__7a7e.hn482764g-4.d41", 0x0000, 0x2000, CRC(fdc0d2e3) SHA1(da1bc1e8c4cb2a2d8cd2273f3e1a9f318ae8cb87), ROM_BIOS(1))
	ROMX_LOAD("804-3_rev_1.4__f240.2732.d42", 0x2000, 0x1000, CRC(29827e29) SHA1(4c7fadf81bcf32349a564d946f5d215de50315c5), ROM_BIOS(1))
	ROMX_LOAD("804-3_rev_1.4__f240.2732.d42", 0x3000, 0x1000, CRC(29827e29) SHA1(4c7fadf81bcf32349a564d946f5d215de50315c5), ROM_BIOS(1)) // load this twice
	ROM_SYSTEM_BIOS( 2, "ep804_v2.21", "Wavetek/Digelec EP804 FWv2.21") // hardware 1.5 NOTE: this may use the address mapper 804-1-3 which is not dumped!
	ROMX_LOAD("804-2_rev2.21__cs_ab50.hn482764g.d41", 0x0000, 0x2000, CRC(ffbc95f6) SHA1(b12aa97e23d546064f1d17aa9b90772017fec5ec), ROM_BIOS(2))
	ROMX_LOAD("804-3_rev2.21__cs_6b98.hn482764g.d42", 0x2000, 0x2000, CRC(a4acb9fe) SHA1(bbc7e3e2e6b3b1abe747380909dcddc985ef8d0d), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "promicron2k_v2.3", "Celectronic Berlin/Digelec promicron 2000 FWv2.3") // hardware 1.6
	ROMX_LOAD("2023__1-03__be7d.m5l2764k.d41", 0x0000, 0x2000, CRC(8e5182f1) SHA1(e8409b6ace80fdaad862e6c06975aeabcf728f97), ROM_BIOS(3))
	ROMX_LOAD("2023__2-03__c73e.m5l2764k.d42", 0x2000, 0x2000, CRC(ff7d959b) SHA1(75718fc1d98969739911cc51b6d5fef74b530e36), ROM_BIOS(3))
	// on the promicron 2000, the bprom at d30 is an unlabeled TB18S030N part, but has the same contents as 804-1-2.mmi_6330-in.d30 below. it is possible the sticker fell off.
	ROM_REGION(0x20, "proms", 0)
	ROM_LOAD("804-1-2.mmi_6330-in.d30", 0x0000, 0x0020, CRC(30dd4721) SHA1(e4b2f5756118be4c8ab56c708dc4f42469c7e51b)) // Address mapper prom, 82s23/mmi6330/tbp18sa030 equivalent 32x8 open collector
ROM_END

} // anonymous namespace


/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY                 FULLNAME                        FLAGS
COMP( 1985, digel804, 0,        0,      digel804, digel804, digel804_state, empty_init, "Digelec, Inc",         "Digelec 804 EPROM Programmer", MACHINE_NOT_WORKING )
COMP( 1982, ep804,    digel804, 0,      ep804,    digel804, ep804_state,    empty_init, "Wavetek/Digelec, Inc", "EP804 EPROM Programmer",       MACHINE_NOT_WORKING )
