// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion Series 3a/3c/3mx

    TODO:
    - sound devices, replace fake Psion Codec device with M7542 and M7702-03
    - serial ports
    - fix RAM detection for 3mx
    - 3mx speed switch using CTRL+Diamond
    - 3c/3mx backlight using Psion+Space
    - 3c/3mx IrDA

******************************************************************************/

#include "emu.h"
#include "machine/nvram.h"
#include "machine/psion_asic9.h"
#include "machine/psion_condor.h"
#include "machine/psion_ssd.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "bus/psion/honda/slot.h"
#include "bus/psion/sibo/slot.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


class psion3a_codec_device : public device_t, public device_sound_interface
{
public:
	psion3a_codec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void pcm_in(uint8_t data);

protected:
	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	sound_stream *m_stream;
	int16_t m_audio_out;
};

DEFINE_DEVICE_TYPE(PSION_S3A_CODEC, psion3a_codec_device, "psion3a_codec", "Series 3a A-law Codec")

psion3a_codec_device::psion3a_codec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_S3A_CODEC, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_audio_out(0)
{
}

void psion3a_codec_device::device_start()
{
	m_stream = stream_alloc(0, 1, 8000);
}

void psion3a_codec_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	outputs[0].fill(stream_buffer::sample_t(m_audio_out) * (1.0 / 4096.0));
}

void psion3a_codec_device::pcm_in(uint8_t data)
{
	m_stream->update();

	// Expand 8-bit signed compressed number to 16-bit 2's complement integer (13-bit magnitude) using A-law
	data ^= 0x55;

	uint8_t seg = (data & 0x70) >> 4;
	m_audio_out = 0;
	if (seg)
	{
		m_audio_out = 0x10;
		seg--;
	}
	m_audio_out = (((m_audio_out + (data & 0x0f)) << 1) + 1) << seg;

	m_audio_out *= (data & 0x80) ? -1.0 : 1.0;
}


namespace {

class psion3a_base_state : public driver_device
{
public:
	psion3a_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_asic9(*this, "asic9")
		, m_ram(*this, "ram")
		, m_nvram(*this, "nvram")
		, m_palette(*this, "palette")
		, m_keyboard(*this, "COL%u", 0U)
		, m_speaker(*this, "speaker")
		, m_codec(*this, "codec")
		, m_ssd(*this, "ssd%u", 1U)
	{ }

	void psion_asic9(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(wakeup);

protected:
	virtual void machine_start() override ATTR_COLD;

	required_device<psion_asic9_device> m_asic9;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_device<palette_device> m_palette;
	required_ioport_array<8> m_keyboard;
	required_device<speaker_sound_device> m_speaker;
	required_device<psion3a_codec_device> m_codec;
	required_device_array<psion_ssd_device, 2> m_ssd;

private:
	void palette_init(palette_device &palette);

	uint16_t kbd_r();

	uint8_t m_key_col = 0;
};

class psion3a_state : public psion3a_base_state
{
public:
	psion3a_state(const machine_config &mconfig, device_type type, const char *tag)
		: psion3a_base_state(mconfig, type, tag)
		, m_sibo(*this, "sibo")
	{ }

	void psion3a(machine_config &config);
	void psion3a2(machine_config &config);

private:
	required_device<psion_sibo_slot_device> m_sibo;
};

class psion3c_state : public psion3a_base_state
{
public:
	psion3c_state(const machine_config &mconfig, device_type type, const char *tag)
		: psion3a_base_state(mconfig, type, tag)
		, m_condor(*this, "condor")
		, m_honda(*this, "honda")
	{ }

	void psion3c(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<psion_condor_device> m_condor;
	required_device<psion_honda_slot_device> m_honda;
};

class psion3mx_state : public psion3a_base_state
{
public:
	psion3mx_state(const machine_config &mconfig, device_type type, const char *tag)
		: psion3a_base_state(mconfig, type, tag)
		, m_honda(*this, "honda")
	{ }

	void psion3mx(machine_config &config);

private:
	required_device<psion_honda_slot_device> m_honda;
};


void psion3a_base_state::machine_start()
{
	m_nvram->set_base(m_ram->pointer(), m_ram->size());
}

void psion3c_state::machine_reset()
{
	m_asic9->io_space().install_readwrite_handler(0x0100, 0x011f, read8sm_delegate(*m_condor, FUNC(psion_condor_device::read)), write8sm_delegate(*m_condor, FUNC(psion_condor_device::write)), 0x00ff);
}


static INPUT_PORTS_START( psion3a )
	PORT_START("COL0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                                   PORT_NAME("Enter")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))                 PORT_NAME(u8"\u2192 End") // U+2192 = →
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(0x09)                                 PORT_NAME("Tab")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))                  PORT_NAME(u8"\u2190 Home") // U+2190 = ←
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))                  PORT_NAME(u8"\u2193 Pg Dn") // U+2193 = ↓
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')  PORT_CHAR('N')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_SHIFT_2)                        PORT_NAME("Psion")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))                    PORT_NAME("Sheet")           PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))                    PORT_NAME("Time")            PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))                    PORT_NAME("Data")            PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)

	PORT_START("COL1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR(';')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('+')  PORT_CHAR('=')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR(')')  PORT_CHAR(']')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR('*')  PORT_CHAR(':')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)                        PORT_NAME("Shift (L)")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))                    PORT_NAME("Calc")            PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))                    PORT_NAME("Agenda")          PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))                    PORT_NAME("System")          PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)

	PORT_START("COL2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(0x08)                                 PORT_NAME("Delete")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('?')  PORT_CHAR('}')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR('(')  PORT_CHAR('[')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))              PORT_NAME("Control")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))                    PORT_NAME("World")           PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))                    PORT_NAME("Word")            PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)

	PORT_START("COL3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))                   PORT_NAME("Help Dial")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('&')  PORT_CHAR('{')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)                        PORT_NAME("Shift (R)")

	PORT_START("COL4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR('$')  PORT_CHAR('~')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('%')  PORT_CHAR('\'')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))              PORT_NAME("\xe2\x97\x86 Caps")

	PORT_START("COL5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR(0xa3) PORT_CHAR('\\')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))                   PORT_NAME("Menu")

	PORT_START("COL6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')

	PORT_START("COL7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')                  PORT_NAME("1 ! Off")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')  PORT_CHAR('"')  PORT_CHAR('#')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('^')  PORT_CHAR('@')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))                    PORT_NAME(u8"\u2191 Pg Up") // U+2191 = ↑
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))                   PORT_NAME("Esc On")          PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
INPUT_PORTS_END


static INPUT_PORTS_START( psion3a_de )
	PORT_INCLUDE(psion3a)

	PORT_MODIFY("COL0")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))                 PORT_NAME(u8"\u2192 Ende") // U+2192 = →
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))                  PORT_NAME(u8"\u2190 Pos1") // U+2190 = ←
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))                  PORT_NAME(u8"\u2193 Bild Dn") // U+2193 = ↓
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))                    PORT_NAME("Tabelle")         PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))                    PORT_NAME("Uhr")             PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))                    PORT_NAME("Daten")           PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)

	PORT_MODIFY("COL1")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR(0xdf)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('+')  PORT_CHAR('<')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR('=')  PORT_CHAR('}')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR('*')  PORT_CHAR('>')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))                    PORT_NAME("Rechner")         PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)

	PORT_MODIFY("COL2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(0x08)                                 PORT_NAME("Entf")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')  PORT_CHAR('[')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR(')')  PORT_CHAR(']')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))              PORT_NAME("Strg")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))                    PORT_NAME("Welt")            PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)

	PORT_MODIFY("COL3")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR(';')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))                   PORT_NAME("Hilfe Wahl")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('?')  PORT_CHAR('{')

	PORT_MODIFY("COL4")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR('$')  PORT_CHAR('@')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('%')  PORT_CHAR('\'')

	PORT_MODIFY("COL5")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR(0xa7) PORT_CHAR('\\')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))                   PORT_NAME(u8"Menü")

	PORT_MODIFY("COL6")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('y')  PORT_CHAR('Y')

	PORT_MODIFY("COL7")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')                  PORT_NAME("1 ! Aus")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('&')  PORT_CHAR(0xba)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR(':')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))                    PORT_NAME(u8"\u2191 Bild Up") // U+2191 = ↑
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))                   PORT_NAME("Esc Ein")         PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
INPUT_PORTS_END


static INPUT_PORTS_START( psion3a_fr )
	PORT_INCLUDE(psion3a)

	PORT_MODIFY("COL0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                                   PORT_NAME(u8"Entrée")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))                 PORT_NAME(u8"\u2192 Fin") // U+2192 = →
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))                  PORT_NAME(u8"\u2190 Debut") // U+2190 = ←
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))                  PORT_NAME(u8"\u2193") // U+2193 = ↓
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))                    PORT_NAME("Tableur")         PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))                    PORT_NAME("Heure")           PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))                    PORT_NAME("Fiche")           PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)

	PORT_MODIFY("COL1")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('+')  PORT_CHAR('<')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR(0xe0) PORT_CHAR('0')  PORT_CHAR(0xba)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR('*')  PORT_CHAR('>')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))                    PORT_NAME("Calc")            PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))                    PORT_NAME(u8"Système")       PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)

	PORT_MODIFY("COL2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(0x08)                                 PORT_NAME("Eff")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('!')  PORT_CHAR('8')  PORT_CHAR('>')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR(0xe7) PORT_CHAR('9')  PORT_CHAR(0xf9)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))              PORT_NAME("Ctrl")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))                    PORT_NAME("Monde")           PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)

	PORT_MODIFY("COL3")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR(';')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))                   PORT_NAME("Aide Comp")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR(0xe8) PORT_CHAR('7')  PORT_CHAR('<')

	PORT_MODIFY("COL4")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('\'') PORT_CHAR('4')  PORT_CHAR('%')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('(')  PORT_CHAR('5')  PORT_CHAR('@')

	PORT_MODIFY("COL5")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('"')  PORT_CHAR('3') PORT_CHAR('\\')

	PORT_MODIFY("COL6")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('y')  PORT_CHAR('Y')

	PORT_MODIFY("COL7")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('&')  PORT_CHAR('1')                  PORT_NAME("& 1 Off")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR(0xe9) PORT_CHAR('2')  PORT_CHAR('#')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR(')')  PORT_CHAR('6')  PORT_CHAR('^')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR(':')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))                    PORT_NAME(u8"\u2191") // U+2191 = ↑
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))                   PORT_NAME("Esc On")          PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
INPUT_PORTS_END


static INPUT_PORTS_START( psion3a_it )
	PORT_INCLUDE(psion3a)

	PORT_MODIFY("COL0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                                   PORT_NAME("Invio")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))                 PORT_NAME(u8"\u2192 Fine") // U+2192 = →
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))                  PORT_NAME(u8"\u2190 Inizio") // U+2190 = ←
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))                  PORT_NAME(u8"\u2193 Pag") // U+2193 = ↓
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))                    PORT_NAME("Foglio")          PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))                    PORT_NAME("Ora")             PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))                    PORT_NAME("Archivi")         PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)

	PORT_MODIFY("COL1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))                    PORT_NAME("Calc")            PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))                    PORT_NAME("Agenda")          PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))                    PORT_NAME("Sistema")         PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)

	PORT_MODIFY("COL2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(0x08)                                 PORT_NAME("Canc.")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))              PORT_NAME("Ctrl")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))                    PORT_NAME("Mondo")           PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))                    PORT_NAME("Testi")           PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)

	PORT_MODIFY("COL3")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))                   PORT_NAME("Aiuto Telef.")

	PORT_MODIFY("COL4")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))              PORT_NAME("\xe2\x97\x86 Maiusc.")

	PORT_MODIFY("COL7")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))                    PORT_NAME(u8"\u2191 Pag") // U+2191 = ↑
INPUT_PORTS_END


static INPUT_PORTS_START( psion3c )
	PORT_INCLUDE(psion3a)

	PORT_MODIFY("COL2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))                    PORT_NAME("Jotter")          PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
INPUT_PORTS_END


//static INPUT_PORTS_START( psion3c_de )
//  PORT_INCLUDE(psion3a_de)
//
//  PORT_MODIFY("COL2")
//  PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))                    PORT_NAME("Notiz")           PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
//INPUT_PORTS_END


static INPUT_PORTS_START( psion3c_fr )
	PORT_INCLUDE(psion3a_fr)

	PORT_MODIFY("COL2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))                    PORT_NAME("Calepin")         PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
INPUT_PORTS_END


static INPUT_PORTS_START( pocketbk2 )
	PORT_INCLUDE(psion3a)

	PORT_MODIFY("COL0")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_SHIFT_2)                        PORT_NAME("Acorn")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))                    PORT_NAME("Abacus")          PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))                    PORT_NAME("Time")            PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))                    PORT_NAME("Cards")           PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)

	PORT_MODIFY("COL1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))                    PORT_NAME("Calc")            PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))                    PORT_NAME("Schedule")        PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))                    PORT_NAME("Desktop")         PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)

	PORT_MODIFY("COL2")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))                    PORT_NAME("World")           PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))                    PORT_NAME("Write")           PORT_CHANGED_MEMBER(DEVICE_SELF, psion3a_state, wakeup, 0)
INPUT_PORTS_END


INPUT_CHANGED_MEMBER(psion3a_base_state::wakeup)
{
	m_asic9->eint0_w(newval);
}


uint16_t psion3a_base_state::kbd_r()
{
	uint16_t data = 0x00;

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_key_col, i))
			data |= m_keyboard[i]->read();
	}

	return data;
}


void psion3a_base_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(190, 220, 190));
	palette.set_pen_color(1, rgb_t(130, 130, 110));
	palette.set_pen_color(2, rgb_t(190, 210, 180));
}


void psion3a_base_state::psion_asic9(machine_config &config)
{
	PSION_ASIC9(config, m_asic9, 7.68_MHz_XTAL); // V30H
	m_asic9->set_screen("screen");
	m_asic9->set_ram_rom("ram", "rom");
	m_asic9->port_ab_r().set(FUNC(psion3a_base_state::kbd_r));
	m_asic9->buz_cb().set(m_speaker, FUNC(speaker_sound_device::level_w));
	//m_asic9->buzvol_cb().set([this](int state) { m_speaker->set_output_gain(ALL_OUTPUTS, state ? 1.0 : 0.25); });
	m_asic9->col_cb().set([this](uint8_t data) { m_key_col = data; });
	m_asic9->data_r<0>().set(m_ssd[1], FUNC(psion_ssd_device::data_r));      // SSD Pack 2
	m_asic9->data_w<0>().set(m_ssd[1], FUNC(psion_ssd_device::data_w));
	m_asic9->data_r<1>().set(m_ssd[0], FUNC(psion_ssd_device::data_r));      // SSD Pack 1
	m_asic9->data_w<1>().set(m_ssd[0], FUNC(psion_ssd_device::data_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_size(480, 160);
	screen.set_visarea_full();
	screen.set_refresh_hz(66);
	screen.set_screen_update(m_asic9, FUNC(psion_asic9_device::screen_update));
	screen.set_palette(m_palette);
	PALETTE(config, "palette", FUNC(psion3a_base_state::palette_init), 3);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	RAM(config, m_ram);
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	PSION_SSD(config, m_ssd[0]);
	m_ssd[0]->door_cb().set(m_asic9, FUNC(psion_asic9_device::medchng_w));
	PSION_SSD(config, m_ssd[1]);
	m_ssd[1]->door_cb().set(m_asic9, FUNC(psion_asic9_device::medchng_w));

	SOFTWARE_LIST(config, "ssd_list").set_original("psion_ssd").set_filter("S3,S3A");
}

void psion3a_state::psion3a(machine_config &config)
{
	psion_asic9(config);

	m_ram->set_default_size("512K").set_extra_options("256K");

	// SIBO expansion port
	PSION_SIBO_SLOT(config, m_sibo, psion_sibo_devices, nullptr);
	m_sibo->int_cb().set(m_asic9, FUNC(psion_asic9_device::sds_int_w));
	m_asic9->data_r<4>().set(m_sibo, FUNC(psion_sibo_slot_device::data_r));
	m_asic9->data_w<4>().set(m_sibo, FUNC(psion_sibo_slot_device::data_w));

	PSION_S3A_CODEC(config, m_codec).add_route(ALL_OUTPUTS, "mono", 1.00); // TODO: M7542
	m_asic9->pcm_out().set(m_codec, FUNC(psion3a_codec_device::pcm_in));
}

void psion3a_state::psion3a2(machine_config &config)
{
	psion3a(config);

	m_ram->set_default_size("2M").set_extra_options("1M");
}

void psion3c_state::psion3c(machine_config &config)
{
	psion_asic9(config);

	m_ram->set_default_size("2M").set_extra_options("1M");

	PSION_S3A_CODEC(config, m_codec).add_route(ALL_OUTPUTS, "mono", 1.00); // TODO: M7702-03
	m_asic9->pcm_out().set(m_codec, FUNC(psion3a_codec_device::pcm_in));

	PSION_CONDOR(config, m_condor);
	m_condor->txd_handler().set(m_honda, FUNC(psion_honda_slot_device::write_txd));
	m_condor->rts_handler().set(m_honda, FUNC(psion_honda_slot_device::write_rts));
	m_condor->dtr_handler().set(m_honda, FUNC(psion_honda_slot_device::write_dtr));
	m_condor->int_handler().set(m_asic9, FUNC(psion_asic9_device::eint1_w));

	// Honda expansion port
	PSION_HONDA_SLOT(config, m_honda, psion_honda_devices, nullptr);
	m_honda->rxd_handler().set(m_condor, FUNC(psion_condor_device::write_rxd));
	m_honda->dcd_handler().set(m_condor, FUNC(psion_condor_device::write_dcd));
	m_honda->dsr_handler().set(m_condor, FUNC(psion_condor_device::write_dsr));
	m_honda->cts_handler().set(m_condor, FUNC(psion_condor_device::write_cts));
	m_honda->sdoe_handler().set(m_asic9, FUNC(psion_asic9_device::medchng_w)); // TODO: verify input line
	m_asic9->data_r<4>().set(m_honda, FUNC(psion_honda_slot_device::data_r));
	m_asic9->data_w<4>().set(m_honda, FUNC(psion_honda_slot_device::data_w));
}

void psion3mx_state::psion3mx(machine_config &config)
{
	psion_asic9(config);

	m_asic9->set_clock(3.6864_MHz_XTAL * 15 / 2); // V30MX

	m_ram->set_default_size("2M").set_extra_options("");

	PSION_S3A_CODEC(config, m_codec).add_route(ALL_OUTPUTS, "mono", 1.00); // TODO: M7702-03
	m_asic9->pcm_out().set(m_codec, FUNC(psion3a_codec_device::pcm_in));

	// Honda expansion port
	PSION_HONDA_SLOT(config, m_honda, psion_honda_devices, nullptr);
	//m_honda->rxd_handler().set(m_asic9mx, FUNC(psion_condor_device::write_rxd));
	//m_honda->dcd_handler().set(m_asic9mx, FUNC(psion_condor_device::write_dcd));
	//m_honda->dsr_handler().set(m_asic9mx, FUNC(psion_condor_device::write_dsr));
	//m_honda->cts_handler().set(m_asic9mx, FUNC(psion_condor_device::write_cts));
	m_honda->sdoe_handler().set(m_asic9, FUNC(psion_asic9_device::medchng_w)); // TODO: verify input line
	m_asic9->data_r<4>().set(m_honda, FUNC(psion_honda_slot_device::data_r));
	m_asic9->data_w<4>().set(m_honda, FUNC(psion_honda_slot_device::data_w));
}


ROM_START(psion3a)
	// Known versions: English, Belgian, Dutch, German
	ROM_REGION16_LE(0x100000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "322f", "V3.22F/ENG")
	ROMX_LOAD("s3a_v3.22f_eng.bin", 0x000000, 0x100000, CRC(fafa3820) SHA1(c1a320b43280cfdb74fc1cb1363fca88dd187487), ROM_BIOS(0))
ROM_END

ROM_START(psion3a2)
	// Known versions: English, Dutch, French, German, Italian, Russian
	ROM_REGION16_LE(0x200000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "340f", "V3.40F/ENG")
	ROMX_LOAD("s3a_v3.40f_eng.bin", 0x000000, 0x200000, CRC(f0adf12c) SHA1(3eb4e7f1fc5611a4d6e65d27d336969ebae94395), ROM_BIOS(0))
ROM_END

ROM_START(psion3a2_us)
	ROM_REGION16_LE(0x200000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "340f", "V3.40F/USA")
	ROMX_LOAD("s3a_v3.40f_usa.bin", 0x000000, 0x200000, CRC(6028294b) SHA1(9dfcb02af268797a15b070ac62e29689f2d18c86), ROM_BIOS(0))
ROM_END

ROM_START(psion3a2_it)
	ROM_REGION16_LE(0x200000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "340f", "V3.40F/ITA")
	ROMX_LOAD("s3a_v3.40f_ita.bin", 0x000000, 0x200000, CRC(0e54df7b) SHA1(c4ed29db1c799fda53acf909a5ef553b4953dc32), ROM_BIOS(0))
ROM_END

ROM_START(psion3a2_de)
	ROM_REGION16_LE(0x200000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "341f", "V3.41F/DEU")
	ROMX_LOAD("s3a_v3.41f_deu.bin", 0x000000, 0x200000, CRC(1f21cb0a) SHA1(fbb9c3356cf8b1d89b8cf50fc12175568c74ce3e), ROM_BIOS(0))
ROM_END

ROM_START(psion3a2_ru)
	ROM_REGION16_LE(0x200000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "343f", "V3.43F/RUS")
	ROMX_LOAD("s3a_v3.43f_rus.bin", 0x000000, 0x200000, CRC(ada4da36) SHA1(24953197f5175596593e5e4045846812ca9b82e3), ROM_BIOS(0))
ROM_END

ROM_START(psion3c)
	// Known versions: English, French, German, Italian, Flemish and Dutch
	ROM_REGION16_LE(0x200000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "520f", "V5.20F/ENG")
	ROMX_LOAD("oak_v5.20f_eng.bin", 0x000000, 0x200000, CRC(d8e672ca) SHA1(23e7570ddbecbfd50953ce6a6b7ead7128814402), ROM_BIOS(0))
ROM_END

ROM_START(psion3mx)
	// Known versions: English, Dutch, French, German, Italian
	ROM_REGION16_LE(0x200000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "616f", "V6.16F/ENG")
	ROMX_LOAD("maple_v6.16f_uk.bin", 0x000000, 0x200000, CRC(10011d9d) SHA1(8c657414513ed57ccf6beddc65dca1fe5ab600fb), ROM_BIOS(0))
ROM_END

ROM_START(psion3mx_fr)
	ROM_REGION16_LE(0x200000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "620f", "V6.20F/FRE")
	ROMX_LOAD("maple_v6.20f_fre.bin", 0x000000, 0x200000, CRC(b4fc57f4) SHA1(26588937d811adf08b973a0188927707d1f6a6e4), ROM_BIOS(0))
ROM_END

ROM_START(pocketbk2)
	ROM_REGION16_LE(0x200000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "130f", "V1.30F/ACN")
	ROMX_LOAD("pb2_v1.30f_acn.bin", 0x000000, 0x200000, CRC(d7ba3a50) SHA1(2d29a7ac5ec4d6bf63bd7d93cc3763e0f9763136), ROM_BIOS(0))
ROM_END

} // anonymous namespace


//    YEAR  NAME          PARENT    COMPAT  MACHINE    INPUT        CLASS           INIT         COMPANY             FULLNAME                    FLAGS
COMP( 1993, psion3a,      0,        0,      psion3a,   psion3a,     psion3a_state,  empty_init,  "Psion",            "Series 3a",                MACHINE_SUPPORTS_SAVE )
COMP( 1994, pocketbk2,    psion3a,  0,      psion3a,   pocketbk2,   psion3a_state,  empty_init,  "Acorn Computers",  "Pocket Book II",           MACHINE_SUPPORTS_SAVE )
COMP( 1995, psion3a2,     psion3a,  0,      psion3a2,  psion3a,     psion3a_state,  empty_init,  "Psion",            "Series 3a (2M)",           MACHINE_SUPPORTS_SAVE )
COMP( 1995, psion3a2_us,  psion3a,  0,      psion3a2,  psion3a,     psion3a_state,  empty_init,  "Psion",            "Series 3a (2M) (US)",      MACHINE_SUPPORTS_SAVE )
COMP( 1995, psion3a2_it,  psion3a,  0,      psion3a2,  psion3a_it,  psion3a_state,  empty_init,  "Psion",            "Series 3a (2M) (Italian)", MACHINE_SUPPORTS_SAVE )
COMP( 1995, psion3a2_de,  psion3a,  0,      psion3a2,  psion3a_de,  psion3a_state,  empty_init,  "Psion",            "Series 3a (2M) (German)",  MACHINE_SUPPORTS_SAVE )
COMP( 1997, psion3a2_ru,  psion3a,  0,      psion3a2,  psion3a,     psion3a_state,  empty_init,  "Psion",            "Series 3a (2M) (Russian)", MACHINE_SUPPORTS_SAVE )
COMP( 1996, psion3c,      0,        0,      psion3c,   psion3c,     psion3c_state,  empty_init,  "Psion",            "Series 3c",                MACHINE_SUPPORTS_SAVE )
COMP( 1998, psion3mx,     0,        0,      psion3mx,  psion3c,     psion3mx_state, empty_init,  "Psion",            "Series 3mx",               MACHINE_SUPPORTS_SAVE )
COMP( 1998, psion3mx_fr,  psion3mx, 0,      psion3mx,  psion3c_fr,  psion3mx_state, empty_init,  "Psion",            "Series 3mx (French)",      MACHINE_SUPPORTS_SAVE )
