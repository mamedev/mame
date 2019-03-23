// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*************************************************************************************

    Yamaha MU-80 and MU-100 : 32-voice polyphonic/multitimbral General MIDI/GS/XG tone modules
    Preliminary driver by R. Belmont and O. Galibert

    MU100 CPU: Hitachi H8S/2655 (HD6432655F), strapped for mode 4 (24-bit address, 16-bit data, no internal ROM)
    Sound ASIC: Yamaha XS725A0/SWP30
    RAM: 1 MSM51008 (1 meg * 1 bit = 128KBytes)

    MU80 CPU: Hitachi H8/3002 (HD6413D02F16), strapped for mode 4, with a 12 MHz oscillator
    Sound ASICs: 2x Yamaha YMM275-F/SWP20 + 2x YMM279-F/SWD wave decoders + HD62908 "MEG" effects processor

    I/O ports from service manual:

    Port 1 (MU100) / Port B (MU80)
        0 - LCD data, SW data, LED 1
        1 - LCD data, SW data, LED 2
        2 - LCD data, SW data, LED 3
        3 - LCD data, SW data, LED 4
        4 - LCD data, SW data, LED 5
        5 - LCD data, SW strobe data
        6 - LCD data, SW strobe data
        7 - LCD data, SW data, LED 6

    Port 2:
        0 - (out) LCD control RS
        1 - (out) LCD control R/W
        2 - (out) LCD control E
        3 - (out) LCD contrast A
        4 - (out) LCD contrast B
        5 - (out) LCD contrast C
        6 - (out) 1 MHz clock for serial
        7 - NC

    Port 3:
        4 - (out) A/D gain control 1
        5 - (out) A/D gain control 2

    Port 5:
        3 - (out) Reset signal for rotary encoder

    Port 6:
        1 - NC
        2 - (out) PB select (SW1)
        3 - (out) PB select (SW2)
        4 - (out) reset PB
        5 - (out) reset SWP30 (sound chip)
        6 - NC
        7 - (in) Plug detection for A/D input

    Port A (MU100):
        5 - (in) Off Line Detection
        6 - (out) Signal for rotary encoder (REB)
        7 - (out) Signal for rotary encoder (REA)

    Port A (MU80):
        0 -
        1 - LCD control RS
        2 -
        3 - (same as sws on MU100) LED,SW Strobe data latch
        4 - (same as swd on MU100) SW data read control
        5 - LCD control E
        6 - LCD control RW
        7 -

    Port F:
        0 - (out) (sws) LED,SW Strobe data latch
        1 - (out) (swd) SW data read control
        2 - (out) PB select (SW4)

    Port G:
        0 - (out) PB select (SW3)

    Analog input channels:
        0 - level input R
        2 - level output L
        4 - host SW type switch position
        6 - battery voltage
        7 - model check (0 for MU100, 0.5 for OEM, 1 for MU100R)

    Switch map at the connector (17=ground)
        09 8 play
        10 8 edit
        11 8 mute/solo
        12 8 part -
        13 8 part +
        14 8 util
        15 8 effect
        16 8 enter
        12 7 select <
        13 7 select >
        16 7 mode
        15 7 eq
        14 7 exit
        10 7 value -
        11 7 value +
           2 led play
           3 led edit
           4 led util
           5 led effect
           6 led mode
           1 led eq

     IC32:
        1 p10 c.2
        2 p11 c.3
        3 p12 c.4
        4 p13 c.5
        5 p14 c.6
        6 p15 c.7
        7 p16 c.8
        8 p17 c.1
        g sws

     IC33
        1 p17 c.09
        2 p16 c.10
        3 p15 c.11
        4 p14 c.12
        5 p13 c.13
        6 p12 c.14
        7 p11 c.15
        8 p10 c.16
        g swd

**************************************************************************************/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/h8/h83002.h"
#include "cpu/h8/h83003.h"
#include "cpu/h8/h8s2655.h"
#include "video/hd44780.h"
#include "sound/swp30.h"
#include "sound/meg.h"

#include "debugger.h"
#include "screen.h"
#include "speaker.h"


static INPUT_PORTS_START( mu100 )
	PORT_START("P7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter")     PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Effect")    PORT_CODE(KEYCODE_F)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Util")      PORT_CODE(KEYCODE_U)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part +")    PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part -")    PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mute/Solo") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit")      PORT_CODE(KEYCODE_E)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Play")      PORT_CODE(KEYCODE_A)

	PORT_START("P8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mode")      PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Eq")        PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Exit")      PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select >")  PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select <")  PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value +")   PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value -")   PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( vl70 )
	PORT_START("B0")
	PORT_BIT(0x83, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Play")      PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Effect")    PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Midi/WX")   PORT_CODE(KEYCODE_X)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter")     PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Exit")      PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("B1")
	PORT_BIT(0x83, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit")      PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mode")      PORT_CODE(KEYCODE_M)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part -")    PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select <")  PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part +")    PORT_CODE(KEYCODE_CLOSEBRACE)

	PORT_START("B2")
	PORT_BIT(0x83, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Util")      PORT_CODE(KEYCODE_U)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Breath")    PORT_CODE(KEYCODE_B)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value -")   PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select >")  PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value +")   PORT_CODE(KEYCODE_EQUALS)
INPUT_PORTS_END

class mu100_state : public driver_device
{
public:
	mu100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mu80cpu(*this, "mu80cpu")
		, m_vl70cpu(*this, "vl70cpu")
		, m_swp30(*this, "swp30")
		, m_meg(*this, "meg")
		, m_lcd(*this, "lcd")
		, m_ioport_p7(*this, "P7")
		, m_ioport_p8(*this, "P8")
		, m_ioport_b0(*this, "B0")
		, m_ioport_b1(*this, "B1")
		, m_ioport_b2(*this, "B2")
	{ }

	void vl70(machine_config &config);
	void mu80(machine_config &config);
	void mu100(machine_config &config);

	void regs_s1_write_tap(offs_t address, u16 data, u16 mem_mask);
	void regs_s2_write_tap(offs_t address, u16 data, u16 mem_mask);
	void regs_s3_write_tap(offs_t address, u16 data, u16 mem_mask);
	void regs_s4a_write_tap(offs_t address, u16 data, u16 mem_mask);
	void regs_s4b_write_tap(offs_t address, u16 data, u16 mem_mask);
	void regs_s4c_write_tap(offs_t address, u16 data, u16 mem_mask);
	void regs_lfo_write_tap(offs_t address, u16 data, u16 mem_mask);
	void regs_s6_write_tap(offs_t address, u16 data, u16 mem_mask);
	void regs_fp_read_tap(offs_t address, u16 data, u16 mem_mask);
	void regs_fp_write_tap(offs_t address, u16 data, u16 mem_mask);
	void regs_int_read_tap(offs_t address, u16 data, u16 mem_mask);
	void regs_int_write_tap(offs_t address, u16 data, u16 mem_mask);
	void voice_write_tap(offs_t address, u16 data, u16 mem_mask);
	void chan_write_tap(offs_t address, u16 data, u16 mem_mask);
	void prg_write_tap(offs_t address, u16 data, u16 mem_mask);

	virtual void machine_reset() override {
		if(0)
		m_maincpu->space(0).install_write_tap(0x214cb8, 0x214cbf, "prg select", [this](offs_t offset, u16 &data, u16 mem_mask) {
												   prg_write_tap(offset, data, mem_mask);
											   });
		if(0)
		m_maincpu->space(0).install_write_tap(0x20cb10, 0x20cb10 + 0x122*0x22 - 1, "chan debug", [this](offs_t offset, u16 &data, u16 mem_mask) {
												   chan_write_tap(offset, data, mem_mask);
											   });
		if(0)
		m_maincpu->space(0).install_write_tap(0x20f03e, 0x20f03e + 0x92*0x40 - 1, "voice debug", [this](offs_t offset, u16 &data, u16 mem_mask) {
												  voice_write_tap(offset, data, mem_mask);
											   });
		if(0)
		m_maincpu->space(0).install_readwrite_tap(0x214ca2+0x20, 0x214ca2+0x320-1, "regs fp",
												  [this](offs_t offset, u16 &data, u16 mem_mask) {
													  regs_fp_read_tap(offset, data, mem_mask);
												  },
												  [this](offs_t offset, u16 &data, u16 mem_mask) {
													  regs_fp_write_tap(offset, data, mem_mask);
												  });
		if(0)
		m_maincpu->space(0).install_readwrite_tap(0x214ca2+0x320, 0x214ca2+0x420-1, "regs int",
												  [this](offs_t offset, u16 &data, u16 mem_mask) {
													  regs_int_read_tap(offset, data, mem_mask);
												  },
												  [this](offs_t offset, u16 &data, u16 mem_mask) {
													  regs_int_write_tap(offset, data, mem_mask);
												  });
		if(0)
		m_maincpu->space(0).install_write_tap(0x214ca2+0x420, 0x214ca2+0x440-1, "regs s1", [this](offs_t offset, u16 &data, u16 mem_mask) {
												  regs_s1_write_tap(offset, data, mem_mask);
											   });
		if(0)
		m_maincpu->space(0).install_write_tap(0x214ca2+0x440, 0x214ca2+0x460-1, "regs s2", [this](offs_t offset, u16 &data, u16 mem_mask) {
												  regs_s2_write_tap(offset, data, mem_mask);
											   });
		if(0)
		m_maincpu->space(0).install_write_tap(0x214ca2+0x460, 0x214ca2+0x480-1, "regs s3", [this](offs_t offset, u16 &data, u16 mem_mask) {
												  regs_s3_write_tap(offset, data, mem_mask);
											   });
		if(0)
		m_maincpu->space(0).install_write_tap(0x214ca2+0x480, 0x214ca2+0x4a0-1, "regs s4a", [this](offs_t offset, u16 &data, u16 mem_mask) {
												  regs_s4a_write_tap(offset, data, mem_mask);
											   });
		if(0)
		m_maincpu->space(0).install_write_tap(0x214ca2+0x4a0, 0x214ca2+0x4c0-1, "regs s4b", [this](offs_t offset, u16 &data, u16 mem_mask) {
												  regs_s4b_write_tap(offset, data, mem_mask);
											   });
		if(0)
		m_maincpu->space(0).install_write_tap(0x214ca2+0x4c0, 0x214ca2+0x4e0-1, "regs s4c", [this](offs_t offset, u16 &data, u16 mem_mask) {
												  regs_s4c_write_tap(offset, data, mem_mask);
											   });
		if(0)
		m_maincpu->space(0).install_write_tap(0x214ca2+0x4e0, 0x214ca2+0x510-1, "regs lfo", [this](offs_t offset, u16 &data, u16 mem_mask) {
												  regs_lfo_write_tap(offset, data, mem_mask);
											   });
		if(0)
		m_maincpu->space(0).install_write_tap(0x214ca2+0x510, 0x214ca2+0x520-1, "regs s6", [this](offs_t offset, u16 &data, u16 mem_mask) {
												  regs_s6_write_tap(offset, data, mem_mask);
											   });
	}

protected:
	virtual u16 adc_type_r();

private:
	enum {
		P2_LCD_RS     = 0x01,
		P2_LCD_RW     = 0x02,
		P2_LCD_ENABLE = 0x04
	};

	enum {
		P6_LCD_RS     = 0x04,
		P6_LCD_RW     = 0x02,
		P6_LCD_ENABLE = 0x01
	};

	enum {
		PA_LCD_RS     = 0x02,
		PA_LCD_ENABLE = 0x20,
		PA_LCD_RW     = 0x40
	};

	optional_device<h8s2655_device> m_maincpu;
	optional_device<h83002_device> m_mu80cpu;
	optional_device<h83003_device> m_vl70cpu;
	optional_device<swp30_device> m_swp30;
	optional_device<meg_device> m_meg;
	required_device<hd44780_device> m_lcd;
	optional_ioport m_ioport_p7;
	optional_ioport m_ioport_p8;
	optional_ioport m_ioport_b0;
	optional_ioport m_ioport_b1;
	optional_ioport m_ioport_b2;

	u8 cur_p1, cur_p2, cur_p3, cur_p5, cur_p6, cur_pa, cur_pb, cur_pc, cur_pf, cur_pg;
	u8 cur_ic32, cur_leds;
	float contrast;

	u16 adc_zero_r();
	u16 adc_ar_r();
	u16 adc_al_r();
	u16 adc_midisw_r();
	u16 adc_battery_r();
	u16 adc_breath_r();

	void p1_w(u16 data);
	u16 p1_r();
	void p2_w(u16 data);
	void p3_w(u16 data);
	void p5_w(u16 data);
	void p6_w(u16 data);
	u16 p6_r();
	void pa_w(u16 data);
	u16 pa_r();
	void pb_w(u16 data);
	u16 pb_r();
	void pa_w_mu80(u16 data);
	u16 pa_r_mu80();
	void pb_w_mu80(u16 data);
	u16 pb_r_mu80();
	void p6_w_vl70(u16 data);
	u16 p6_r_vl70();
	void pa_w_vl70(u16 data);
	u16 pa_r_vl70();
	void pb_w_vl70(u16 data);
	void pc_w_vl70(u16 data);
	u16 pc_r_vl70();
	void pf_w(u16 data);
	void pg_w(u16 data);

	float lightlevel(const u8 *src, const u8 *render);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual void machine_start() override;
	void mu100_iomap(address_map &map);
	void mu100_map(address_map &map);
	void mu80_iomap(address_map &map);
	void mu80_map(address_map &map);
	void vl70_iomap(address_map &map);
	void vl70_map(address_map &map);
	void swp30_map(address_map &map);
};

class mu100r_state : public mu100_state {
public:
	mu100r_state(const machine_config &mconfig, device_type type, const char *tag)
		: mu100_state(mconfig, type, tag)
	{ }

private:
	virtual u16 adc_type_r() override;
};

void mu100_state::prg_write_tap(offs_t address, u16 data, u16 mem_mask)
{
	if(mem_mask == 0x00ff) {
		static const char *names[4] = { "chorus", "variation", "insertion1", "insertion2" };
		logerror("prg_select %s %d\n", names[(address - 0x214cb8)/2], data);
	}
}

void mu100_state::regs_s1_write_tap(offs_t address, u16 data, u16 mem_mask)
{
	offs_t pc = m_maincpu->pc();
	offs_t reg = (address - 0x214ca2-0x420)/2;
	if(pc != 0x72912)
		logerror("regs_s1_w %03x, %04x @ %04x (%06x)\n", reg, data, mem_mask, pc);
}

void mu100_state::regs_s2_write_tap(offs_t address, u16 data, u16 mem_mask)
{
	offs_t pc = m_maincpu->pc();
	offs_t reg = (address - 0x214ca2-0x440)/2;
	if(pc != 0x72912)
		logerror("regs_s2_w %03x, %04x @ %04x (%06x)\n", reg, data, mem_mask, pc);
}

void mu100_state::regs_s3_write_tap(offs_t address, u16 data, u16 mem_mask)
{
	offs_t pc = m_maincpu->pc();
	offs_t reg = (address - 0x214ca2-0x460)/2;
	if(pc != 0x72912)
		logerror("regs_s3_w %03x, %04x @ %04x (%06x)\n", reg, data, mem_mask, pc);
}

void mu100_state::regs_s4a_write_tap(offs_t address, u16 data, u16 mem_mask)
{
	offs_t pc = m_maincpu->pc();
	offs_t reg = (address - 0x214ca2-0x480)/2;
	if(pc != 0x72912)
		logerror("regs_s4a_w %03x, %04x @ %04x (%06x)\n", reg, data, mem_mask, pc);
}

void mu100_state::regs_s4b_write_tap(offs_t address, u16 data, u16 mem_mask)
{
	offs_t pc = m_maincpu->pc();
	offs_t reg = (address - 0x214ca2-0x4a0)/2;
	if(pc != 0x72912)
		logerror("regs_s4b_w %03x, %04x @ %04x (%06x)\n", reg, data, mem_mask, pc);
}

void mu100_state::regs_s4c_write_tap(offs_t address, u16 data, u16 mem_mask)
{
	offs_t pc = m_maincpu->pc();
	offs_t reg = (address - 0x214ca2-0x4c0)/2;
	if(pc != 0x72912)
		logerror("regs_s4c_w %03x, %04x @ %04x (%06x)\n", reg, data, mem_mask, pc);
}

void mu100_state::regs_lfo_write_tap(offs_t address, u16 data, u16 mem_mask)
{
	offs_t pc = m_maincpu->pc();
	offs_t reg = (address - 0x214ca2-0x4e0)/2;
	if(pc != 0x72912)
		logerror("regs_lfo_w %03x, %04x @ %04x (%06x)\n", reg, data, mem_mask, pc);
}

void mu100_state::regs_s6_write_tap(offs_t address, u16 data, u16 mem_mask)
{
	offs_t pc = m_maincpu->pc();
	offs_t reg = (address - 0x214ca2-0x510)/2;
	if(pc != 0x72912)
		logerror("regs_s6_w %03x, %04x @ %04x (%06x)\n", reg, data, mem_mask, pc);
}

void mu100_state::regs_fp_read_tap(offs_t address, u16 data, u16 mem_mask)
{
	offs_t pc = m_maincpu->pc();
	offs_t reg = (address - 0x214ca2-0x20)/2;
	if(pc != 0x72912)
		logerror("regs_fp_r %03x, %04x @ %04x (%06x)\n", reg, data, mem_mask, pc);
}

void mu100_state::regs_fp_write_tap(offs_t address, u16 data, u16 mem_mask)
{
	offs_t pc = m_maincpu->pc();
	offs_t reg = (address - 0x214ca2-0x20)/2;
	logerror("regs_fp_w %03x, %04x @ %04x (%06x)\n", reg, data, mem_mask, pc);
}

void mu100_state::regs_int_read_tap(offs_t address, u16 data, u16 mem_mask)
{
	offs_t pc = m_maincpu->pc();
	offs_t reg = (address - 0x214ca2-0x320)/2;
	if(pc != 0x729c6)
		logerror("regs_int_r %03x, %04x @ %04x (%06x)\n", reg, data, mem_mask, pc);
}

void mu100_state::regs_int_write_tap(offs_t address, u16 data, u16 mem_mask)
{
	offs_t pc = m_maincpu->pc();
	offs_t reg = (address - 0x214ca2-0x320)/2;
	logerror("regs_int_w %03x, %04x @ %04x (%06x)\n", reg, data, mem_mask, pc);
}

void mu100_state::voice_write_tap(offs_t address, u16 data, u16 mem_mask)
{
	offs_t pc = m_maincpu->pc();
	offs_t off = address - 0x20f03e;
	int voice = off / 0x92;
	int slot = off % 0x92;
	if(mem_mask == 0xffff) {
		logerror("voice_w %02x:%02x, %04x (%06x)\n", voice, slot, data, pc);
	} else {
		if(mem_mask == 0xff00)
			data >>= 8;
		else
			slot++;
		logerror("voice_w %02x:%02x, %02x (%06x)\n", voice, slot, data, pc);
	}
}

void mu100_state::chan_write_tap(offs_t address, u16 data, u16 mem_mask)
{
	offs_t pc = m_maincpu->pc();
	offs_t off = address - 0x20cb10;
	int voice = off / 0x112;
	int slot = off % 0x112;
	if(mem_mask == 0xffff) {
		if(slot == 0x102 && data == 0)
			return;
		if(slot == 0x100 && data == 0)
			return;
		if(slot == 0x0fe && data == 0)
			return;
		logerror("chan_w %02x:%03x, %04x (%06x)\n", voice, slot, data, pc);
	} else {
		if(mem_mask == 0xff00)
			data >>= 8;
		else
			slot++;
		if(slot == 0x106 && data == 0)
			return;
		if(slot == 0x108 && data == 0)
			return;
		if(slot == 0x105) // volume
			return;
		if(slot == 0x109 && data == 0)
			return;
		if(slot == 0x0e7 && data == 0)
			return;
		if(slot == 0x0e5 && data == 0)
			return;
		if(slot == 0x111 && data == 0x40)
			return;
		logerror("chan_w %02x:%03x, %02x (%06x)\n", voice, slot, data, pc);
	}
}

#include "../drivers/ymmu100.hxx"

void mu100_state::machine_start()
{
	cur_p1 = cur_p2 = cur_p3 = cur_p5 = cur_p6 = cur_pa = cur_pc = cur_pf = cur_pg = cur_ic32 = 0xff;
	cur_leds = 0x00;
	contrast = 1.0;
}

float mu100_state::lightlevel(const u8 *src, const u8 *render)
{
	u8 l = *src;
	if(l == 0)
		return 1.0;
	int slot = (src[1] << 8) | src[2];
	if(slot >= 0xff00)
		return (255-l)/255.0;

	int bit = slot & 7;
	int adr = (slot >> 3);
	if(render[adr] & (1 << bit))
		return 1-(1-(255-l)/255.0f)*contrast;
	return 0.95f;
}

u32 mu100_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const u8 *render = m_lcd->render();
	const u8 *src = ymmu100_bkg + 15;

	for(int y=0; y<241; y++) {
		u32 *pix = reinterpret_cast<u32 *>(bitmap.raw_pixptr(y));
		for(int x=0; x<800; x++) {
			float light = lightlevel(src, render);
			u32 col = (int(0xef*light) << 16) | (int(0xf5*light) << 8);
			*pix++ = col;
			src += 3;
		}
		for(int x=800; x<900; x++)
			*pix++ = 0;
	}

	for(int i=0; i<6; i++)
		if(cur_leds & (1 << i)) {
			int x = 830 + 40*(i & 1);
			int y = 55 + 65*(i >> 1);
			for(int yy=-9; yy <= 9; yy++) {
				int dx = int(sqrt((float)(99-yy*yy)));
				u32 *pix = reinterpret_cast<u32 *>(bitmap.raw_pixptr(y+yy)) + (x-dx);
				for(int xx=0; xx<2*dx+1; xx++)
					*pix++ = 0x00ff00;
			}
		}
	return 0;
}

void mu100_state::mu80_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("mu80cpu", 0);
	map(0x200000, 0x20ffff).ram(); // 64K work RAM
	map(0x440000, 0x44001f).m(m_meg, FUNC(meg_device::map));
}

void mu100_state::vl70_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("vl70cpu", 0);
	map(0x200000, 0x20ffff).ram(); // 64K work RAM
	map(0x600000, 0x60001f).m(m_meg, FUNC(meg_device::map));
}

void mu100_state::mu100_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
	map(0x200000, 0x21ffff).ram(); // 128K work RAM
	map(0x400000, 0x401fff).m(m_swp30, FUNC(swp30_device::map));
}

// Grounded adc input
u16 mu100_state::adc_zero_r()
{
	return 0;
}

// Analog input right (also sent to the swp)
u16 mu100_state::adc_ar_r()
{
	return 0;
}

// Analog input left (also sent to the swp)
u16 mu100_state::adc_al_r()
{
	return 0;
}

// Put the host switch to pure midi
u16 mu100_state::adc_midisw_r()
{
	return 0;
}

// Battery level
u16 mu100_state::adc_battery_r()
{
	return 0x200;
}

// Breath controller
u16 mu100_state::adc_breath_r()
{
	return 0x000;
}

// model detect.  pulled to GND (0) on MU100, to 0.5Vcc on the card version, to Vcc on MU100R
u16 mu100_state::adc_type_r()
{
	return 0;
}

u16 mu100r_state::adc_type_r()
{
	return 0x3ff;
}

void mu100_state::p1_w(u16 data)
{
	cur_p1 = data;
}

u16 mu100_state::p1_r()
{
	if((cur_p2 & P2_LCD_ENABLE)) {
		if(cur_p2 & P2_LCD_RW) {
			if(cur_p2 & P2_LCD_RS)
				return m_lcd->data_read();
			else
				return m_lcd->control_read();
		} else
			return 0x00;
	}

	if(!(cur_pf & 0x02)) {
		u8 val = 0xff;
		if(!(cur_ic32 & 0x20))
			val &= m_ioport_p7->read();
		if(!(cur_ic32 & 0x40))
			val &= m_ioport_p8->read();
		return val;
	}

	return 0xff;
}

void mu100_state::p2_w(u16 data)
{
	// LCB enable edge
	if(!(cur_p2 & P2_LCD_ENABLE) && (data & P2_LCD_ENABLE)) {
		if(!(cur_p2 & P2_LCD_RW)) {
			if(cur_p2 & P2_LCD_RS)
				m_lcd->data_write(cur_p1);
			else
				m_lcd->control_write(cur_p1);
		}
	}
	contrast = (8 - ((cur_p2 >> 3) & 7))/8.0;
	cur_p2 = data;
}

void mu100_state::p3_w(u16 data)
{
	cur_p3 = data;
	logerror("A/D gain control %d\n", (data >> 4) & 3);
}

void mu100_state::p5_w(u16 data)
{
	cur_p5 = data;
	logerror("Rotary reset %d\n", (data >> 3) & 1);
}

void mu100_state::p6_w(u16 data)
{
	cur_p6 = data;
	logerror("pbsel %d pbreset %d soundreset %d\n", (data >> 2) & 3, (data >> 4) & 1, (data >> 5) & 1);
}

u16 mu100_state::p6_r()
{
	//  logerror("plug in detect read\n");
	return 0x00;
}

void mu100_state::pa_w(u16 data)
{
	cur_pa = data;
	logerror("rotary encoder %d\n", (data >> 6) & 3);
}

u16 mu100_state::pa_r()
{
	logerror("offline detect read\n");
	return 0x00;
}

void mu100_state::pf_w(u16 data)
{
	if(!(cur_pf & 0x01) && (data & 0x01)) {
		cur_ic32 = cur_p1;
		cur_leds = (cur_p1 & 0x1f) | ((cur_p1 & 0x80) >> 2);
	}
	cur_pf = data;
}

void mu100_state::pg_w(u16 data)
{
	cur_pg = data;
	logerror("pbsel3 %d\n", data & 1);
}

void mu100_state::pb_w_mu80(u16 data)
{
	cur_pb = data;
}

u16 mu100_state::pb_r_mu80()
{
	if((cur_pa & PA_LCD_ENABLE)) {
		if(cur_pa & PA_LCD_RW)
		{
			if(cur_pa & PA_LCD_RS)
				return m_lcd->data_read();
			else
				return m_lcd->control_read();
		} else
			return 0x00;
	}

	if(!(cur_pa & 0x10)) {
		u8 val = 0xff;
		if(!(cur_ic32 & 0x20))
			val &= m_ioport_p7->read();
		if(!(cur_ic32 & 0x40))
			val &= m_ioport_p8->read();
		return val;
	}

	return cur_pa;
}

void mu100_state::pa_w_mu80(u16 data)
{
	data ^= PA_LCD_ENABLE;
	if(!(cur_pa & PA_LCD_ENABLE) && (data & PA_LCD_ENABLE)) {
	if(!(cur_pa & PA_LCD_RW)) {
		if(cur_pa & PA_LCD_RS)
			m_lcd->data_write(cur_pb);
		else
			m_lcd->control_write(cur_pb);
		}
	}

	if(!(cur_pa & 0x08) && (data & 0x08))
		cur_ic32 = cur_pb;

	cur_pa = data;
}

u16 mu100_state::pa_r_mu80()
{
	return cur_pa;
}

void mu100_state::p6_w_vl70(u16 data)
{
	if(!(cur_p6 & P6_LCD_ENABLE) && (data & P6_LCD_ENABLE)) {
	if(!(cur_p6 & P6_LCD_RW)) {
		if(cur_p6 & P6_LCD_RS)
			m_lcd->data_write(cur_pa);
		else
			m_lcd->control_write(cur_pa);
		}
	}

//  if(!(cur_pa9 & 0x08) && (data & 0x08))
//      cur_ic32 = cur_pa;

	cur_p6 = data;
}

void mu100_state::pb_w_vl70(u16 data)
{
	cur_leds = bitswap<6>((data >> 2) ^ 0x3f, 5, 3, 1, 4, 2, 0);
}

void mu100_state::pc_w_vl70(u16 data)
{
	cur_pc = data;
}

u16 mu100_state::pc_r_vl70()
{
	u8 r = 0xff;
	if(!(cur_pc & 0x01))
		r &= m_ioport_b0->read();
	if(!(cur_pc & 0x02))
		r &= m_ioport_b1->read();
	if(!(cur_pc & 0x80))
		r &= m_ioport_b2->read();
	return r;
}

u16 mu100_state::p6_r_vl70()
{
	return cur_p6;
}

void mu100_state::pa_w_vl70(u16 data)
{
	cur_pa = data;
}

u16 mu100_state::pa_r_vl70()
{
	if((cur_p6 & P6_LCD_ENABLE)) {
		if(cur_p6 & P6_LCD_RW)
		{
			if(cur_p6 & P6_LCD_RS)
				return m_lcd->data_read();
			else
				return m_lcd->control_read();
		} else
			return 0x00;
	}

	return cur_pa;
}

void mu100_state::mu80_iomap(address_map &map)
{
	map(h8_device::PORT_A, h8_device::PORT_A).rw(FUNC(mu100_state::pa_r_mu80), FUNC(mu100_state::pa_w_mu80));
	map(h8_device::PORT_B, h8_device::PORT_B).rw(FUNC(mu100_state::pb_r_mu80), FUNC(mu100_state::pb_w_mu80));
	map(h8_device::ADC_0, h8_device::ADC_0).r(FUNC(mu100_state::adc_ar_r));
	map(h8_device::ADC_1, h8_device::ADC_1).r(FUNC(mu100_state::adc_zero_r));
	map(h8_device::ADC_2, h8_device::ADC_2).r(FUNC(mu100_state::adc_al_r));
	map(h8_device::ADC_3, h8_device::ADC_3).r(FUNC(mu100_state::adc_zero_r));
	map(h8_device::ADC_4, h8_device::ADC_4).r(FUNC(mu100_state::adc_midisw_r));
	map(h8_device::ADC_5, h8_device::ADC_6).r(FUNC(mu100_state::adc_zero_r));
	map(h8_device::ADC_6, h8_device::ADC_6).r(FUNC(mu100_state::adc_battery_r));
	map(h8_device::ADC_7, h8_device::ADC_7).r(FUNC(mu100_state::adc_zero_r)); // inputmod from the gate array
}

void mu100_state::vl70_iomap(address_map &map)
{
	map(h8_device::PORT_6, h8_device::PORT_6).rw(FUNC(mu100_state::p6_r_vl70), FUNC(mu100_state::p6_w_vl70));
	map(h8_device::PORT_A, h8_device::PORT_A).rw(FUNC(mu100_state::pa_r_vl70), FUNC(mu100_state::pa_w_vl70));
	map(h8_device::PORT_B, h8_device::PORT_B).w(FUNC(mu100_state::pb_w_vl70));
	map(h8_device::PORT_C, h8_device::PORT_C).rw(FUNC(mu100_state::pc_r_vl70), FUNC(mu100_state::pc_w_vl70));
	map(h8_device::ADC_0, h8_device::ADC_0).r(FUNC(mu100_state::adc_breath_r));
	map(h8_device::ADC_1, h8_device::ADC_6).r(FUNC(mu100_state::adc_zero_r));
	map(h8_device::ADC_2, h8_device::ADC_2).r(FUNC(mu100_state::adc_midisw_r));
	map(h8_device::ADC_3, h8_device::ADC_6).r(FUNC(mu100_state::adc_zero_r));
	map(h8_device::ADC_4, h8_device::ADC_4).r(FUNC(mu100_state::adc_battery_r));
	map(h8_device::ADC_5, h8_device::ADC_6).r(FUNC(mu100_state::adc_zero_r));
	map(h8_device::ADC_6, h8_device::ADC_6).r(FUNC(mu100_state::adc_zero_r));
	map(h8_device::ADC_7, h8_device::ADC_7).r(FUNC(mu100_state::adc_zero_r));
}

void mu100_state::mu100_iomap(address_map &map)
{
	map(h8_device::PORT_1, h8_device::PORT_1).rw(FUNC(mu100_state::p1_r), FUNC(mu100_state::p1_w));
	map(h8_device::PORT_2, h8_device::PORT_2).w(FUNC(mu100_state::p2_w));
	map(h8_device::PORT_3, h8_device::PORT_3).w(FUNC(mu100_state::p3_w));
	map(h8_device::PORT_5, h8_device::PORT_5).w(FUNC(mu100_state::p5_w));
	map(h8_device::PORT_6, h8_device::PORT_6).rw(FUNC(mu100_state::p6_r), FUNC(mu100_state::p6_w));
	map(h8_device::PORT_A, h8_device::PORT_A).rw(FUNC(mu100_state::pa_r), FUNC(mu100_state::pa_w));
	map(h8_device::PORT_F, h8_device::PORT_F).w(FUNC(mu100_state::pf_w));
	map(h8_device::PORT_G, h8_device::PORT_G).w(FUNC(mu100_state::pg_w));
	map(h8_device::ADC_0, h8_device::ADC_0).r(FUNC(mu100_state::adc_ar_r));
	map(h8_device::ADC_1, h8_device::ADC_1).r(FUNC(mu100_state::adc_zero_r));
	map(h8_device::ADC_2, h8_device::ADC_2).r(FUNC(mu100_state::adc_al_r));
	map(h8_device::ADC_1, h8_device::ADC_3).r(FUNC(mu100_state::adc_zero_r));
	map(h8_device::ADC_4, h8_device::ADC_4).r(FUNC(mu100_state::adc_midisw_r));
	map(h8_device::ADC_5, h8_device::ADC_5).r(FUNC(mu100_state::adc_zero_r));
	map(h8_device::ADC_6, h8_device::ADC_6).r(FUNC(mu100_state::adc_battery_r));
	map(h8_device::ADC_7, h8_device::ADC_7).r(FUNC(mu100_state::adc_type_r));
}

void mu100_state::swp30_map(address_map &map)
{
	map(0x000000*4, 0x200000*4-1).rom().region("swp30",         0).mirror(4*0x200000);
	map(0x400000*4, 0x500000*4-1).rom().region("swp30",  0x800000).mirror(4*0x300000);
	map(0x800000*4, 0xa00000*4-1).rom().region("swp30", 0x1000000).mirror(4*0x200000);
}

void mu100_state::mu100(machine_config &config)
{
	H8S2655(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mu100_state::mu100_map);
	m_maincpu->set_addrmap(AS_IO, &mu100_state::mu100_iomap);

	HD44780(config, m_lcd);
	m_lcd->set_lcd_size(4, 20);

	auto &screen = SCREEN(config, "screen", SCREEN_TYPE_LCD);
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate, asynchronous updating anyway */
	screen.set_screen_update(FUNC(mu100_state::screen_update));
	screen.set_size(900, 241);
	screen.set_visarea(0, 899, 0, 240);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SWP30(config, m_swp30);
	m_swp30->set_addrmap(0, &mu100_state::swp30_map);
	m_swp30->add_route(0, "lspeaker", 1.0);
	m_swp30->add_route(1, "rspeaker", 1.0);

	auto &mdin_a(MIDI_PORT(config, "mdin_a"));
	midiin_slot(mdin_a);
	mdin_a.rxd_handler().set("maincpu:sci1", FUNC(h8_sci_device::rx_w));

	auto &mdin_b(MIDI_PORT(config, "mdin_b"));
	midiin_slot(mdin_b);
	mdin_b.rxd_handler().set("maincpu:sci0", FUNC(h8_sci_device::rx_w));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_maincpu->subdevice<h8_sci_device>("sci0")->tx_handler().set(mdout, FUNC(midi_port_device::write_txd));
}

void mu100_state::mu80(machine_config &config)
{
	H83002(config, m_mu80cpu, 12_MHz_XTAL);
	m_mu80cpu->set_addrmap(AS_PROGRAM, &mu100_state::mu80_map);
	m_mu80cpu->set_addrmap(AS_IO, &mu100_state::mu80_iomap);

	HD44780(config, m_lcd);
	m_lcd->set_lcd_size(4, 20);

	auto &screen = SCREEN(config, "screen", SCREEN_TYPE_LCD);
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate, asynchronous updating anyway */
	screen.set_screen_update(FUNC(mu100_state::screen_update));
	screen.set_size(900, 241);
	screen.set_visarea(0, 899, 0, 240);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	// In truth, dual swp-20
	SWP30(config, m_swp30);
	m_swp30->set_addrmap(0, &mu100_state::swp30_map);
	m_swp30->add_route(0, "lspeaker", 1.0);
	m_swp30->add_route(1, "rspeaker", 1.0);

	MEG(config, m_meg);

	auto &mdin_a(MIDI_PORT(config, "mdin_a"));
	midiin_slot(mdin_a);
	mdin_a.rxd_handler().set("mu80cpu:sci1", FUNC(h8_sci_device::rx_w));

	auto &mdin_b(MIDI_PORT(config, "mdin_b"));
	midiin_slot(mdin_b);
	mdin_b.rxd_handler().set("mu80cpu:sci0", FUNC(h8_sci_device::rx_w));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_mu80cpu->subdevice<h8_sci_device>("sci0")->tx_handler().set(mdout, FUNC(midi_port_device::write_txd));
}

void mu100_state::vl70(machine_config &config)
{
	H83003(config, m_vl70cpu, 10_MHz_XTAL);
	m_vl70cpu->set_addrmap(AS_PROGRAM, &mu100_state::vl70_map);
	m_vl70cpu->set_addrmap(AS_IO, &mu100_state::vl70_iomap);

	HD44780(config, m_lcd);
	m_lcd->set_lcd_size(4, 20);

	auto &screen = SCREEN(config, "screen", SCREEN_TYPE_LCD);
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate, asynchronous updating anyway */
	screen.set_screen_update(FUNC(mu100_state::screen_update));
	screen.set_size(900, 241);
	screen.set_visarea(0, 899, 0, 240);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MEG(config, m_meg);

	auto &mdin_a(MIDI_PORT(config, "mdin_a"));
	midiin_slot(mdin_a);
	mdin_a.rxd_handler().set("vl70cpu:sci1", FUNC(h8_sci_device::rx_w));

	auto &mdin_b(MIDI_PORT(config, "mdin_b"));
	midiin_slot(mdin_b);
	mdin_b.rxd_handler().set("vl70cpu:sci0", FUNC(h8_sci_device::rx_w));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_vl70cpu->subdevice<h8_sci_device>("sci0")->tx_handler().set(mdout, FUNC(midi_port_device::write_txd));
}

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios))

ROM_START( mu100 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "bios0", "xu50720 (v1.11, Aug. 3, 1999)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "xu50720.ic11", 0x000000, 0x200000, CRC(1126a8a4) SHA1(e90b8bd9d14297da26ba12f4d9a4f2d22cd7d34a) )
	ROM_SYSTEM_BIOS( 1, "bios1", "xt71420 (v1.05, Sep. 19, 1997)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "xt71420.ic11", 0x000000, 0x200000, CRC(0e5b3bae) SHA1(3148c5bd59a3d00809d3ab1921216215fe2582c5) )

	ROM_REGION( 0x1800000, "swp30", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD( "sx518b0.ic34", 0x0000000, 0x400000, CRC(2550d44f) SHA1(fd3cce228c7d389a2fde25c808a5b26080588cba) )
	ROM_LOAD32_WORD( "sx743b0.ic35", 0x0000002, 0x400000, CRC(a9109a6c) SHA1(a67bb49378a38a2d809bd717d286e18bc6496db0) )
	ROM_LOAD32_WORD( "xt445a0-828.ic36", 0x0800000, 0x200000, CRC(225c2280) SHA1(23b5e046fd2e2ac01af3e6dc6357c5c6547b286b) )
	ROM_LOAD32_WORD( "xt461a0-829.ic37", 0x0800002, 0x200000, CRC(a1d138a3) SHA1(46a7a7225cd7e1818ba551325d2af5ac1bf5b2bf) )
	ROM_LOAD32_WORD( "xt462a0.ic39", 0x1000000, 0x400000, CRC(cbf037da) SHA1(37449e741243305de38cb913b17041942ad334cd) )
	ROM_LOAD32_WORD( "xt463a0.ic38", 0x1000002, 0x400000, CRC(cce5f8d3) SHA1(bdca8c5158f452f2b5535c7d658c9b22c6d66048) )

	ROM_REGION( 0x1000, "lcd", 0)
	// Hand made, 3 characters unused
	ROM_LOAD( "mu100-font.bin", 0x0000, 0x1000, BAD_DUMP CRC(a7d6c1d6) SHA1(9f0398d678bdf607cb34d83ee535f3b7fcc97c41) )
ROM_END

// Identical to the mu100
ROM_START( mu100r )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "bios0", "xu50720 (v1.11, Aug. 3, 1999)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "xu50720.ic11", 0x000000, 0x200000, CRC(1126a8a4) SHA1(e90b8bd9d14297da26ba12f4d9a4f2d22cd7d34a) )
	ROM_SYSTEM_BIOS( 1, "bios1", "xt71420 (v1.05, Sep. 19, 1997)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "xt71420.ic11", 0x000000, 0x200000, CRC(0e5b3bae) SHA1(3148c5bd59a3d00809d3ab1921216215fe2582c5) )

	ROM_REGION( 0x1800000, "swp30", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD( "sx518b0.ic34", 0x000000, 0x400000, CRC(2550d44f) SHA1(fd3cce228c7d389a2fde25c808a5b26080588cba) )
	ROM_LOAD32_WORD( "sx743b0.ic35", 0x000002, 0x400000, CRC(a9109a6c) SHA1(a67bb49378a38a2d809bd717d286e18bc6496db0) )
	ROM_LOAD32_WORD( "xt445a0-828.ic36", 0x800000, 0x200000, CRC(225c2280) SHA1(23b5e046fd2e2ac01af3e6dc6357c5c6547b286b) )
	ROM_LOAD32_WORD( "xt461a0-829.ic37", 0x800002, 0x200000, CRC(a1d138a3) SHA1(46a7a7225cd7e1818ba551325d2af5ac1bf5b2bf) )
	ROM_LOAD32_WORD( "xt462a0.ic39", 0x1000000, 0x400000, CRC(cbf037da) SHA1(37449e741243305de38cb913b17041942ad334cd) )
	ROM_LOAD32_WORD( "xt463a0.ic38", 0x1000002, 0x400000, CRC(cce5f8d3) SHA1(bdca8c5158f452f2b5535c7d658c9b22c6d66048) )

	ROM_REGION( 0x1000, "lcd", 0)
	// Hand made, 3 characters unused
	ROM_LOAD( "mu100-font.bin", 0x0000, 0x1000, BAD_DUMP CRC(a7d6c1d6) SHA1(9f0398d678bdf607cb34d83ee535f3b7fcc97c41) )
ROM_END

ROM_START( mu80 )
	ROM_REGION( 0x80000, "mu80cpu", 0 )
	ROM_LOAD16_WORD_SWAP( "yamaha_mu80.bin", 0x000000, 0x080000, CRC(c31074c0) SHA1(a11bd4523cd8ff1e1744078c3b4c18112b73c61e) )

	ROM_REGION( 0x1800000, "swp30", ROMREGION_ERASE00 )

	ROM_REGION( 0x1000, "lcd", 0)
	// Hand made, 3 characters unused
	ROM_LOAD( "mu100-font.bin", 0x0000, 0x1000, BAD_DUMP CRC(a7d6c1d6) SHA1(9f0398d678bdf607cb34d83ee535f3b7fcc97c41) )
ROM_END

ROM_START( vl70 )
	ROM_REGION( 0x200000, "vl70cpu", 0 )
	ROM_LOAD16_WORD_SWAP( "vl70m_v111_27c160.bin", 0x000000, 0x200000, CRC(efdba9f0) SHA1(cfa9fb7d2a991e4752393c9677e4ddcbe10866c7) )

	ROM_REGION( 0x1000, "lcd", 0)
	// Hand made, 3 characters unused
	ROM_LOAD( "mu100-font.bin", 0x0000, 0x1000, BAD_DUMP CRC(a7d6c1d6) SHA1(9f0398d678bdf607cb34d83ee535f3b7fcc97c41) )
ROM_END

CONS( 1997, mu100,  0,     0, mu100, mu100, mu100_state,  empty_init, "Yamaha", "MU100",                  MACHINE_NOT_WORKING )
CONS( 1997, mu100r, mu100, 0, mu100, mu100, mu100r_state, empty_init, "Yamaha", "MU100 Rackable version", MACHINE_NOT_WORKING )
CONS( 1994, mu80,   mu100, 0, mu80,  mu100, mu100_state,  empty_init, "Yamaha", "MU80",                   MACHINE_NOT_WORKING )
CONS( 1996, vl70,   mu100, 0, vl70,  vl70,  mu100_state,  empty_init, "Yamaha", "VL70-m",                 MACHINE_NOT_WORKING )
