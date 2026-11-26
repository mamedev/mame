// license:BSD-3-Clause
// copyright-holders: Peter Wilhelmsen

/*
Banpresto and Takara / Tomy joint medal game system

PCBs are marked BPSC-2005KG for Banpresto and TMSC-2004KG01 for Takara / Tomy.
There's also a newer BPSC-2006KG PCB with a BPROM-2006KG sub-board for bigger storage capability.

The main components are:
- H8S/2324 (for Banpresto) or H8S/2329 (for Tomy) CPU, using external program ROM.
- 24.576 MHz XTAL (near H8S)
- LC35V256ET-70W 256K SRAM (near CPU)
- Axell AG-2 AX51201 GFX chip
- IC42S32400-7TG 128 MBit SDRAM (near GFX chip)
- Oki M9810B (for Banpresto) or M9811 (for Tomy) ADPCM chip
- 4.08KSS2FT XTAL (near ADPCM chip)
- 2x bank of 8 DIP switches

The BPSC-2006KG PCB adds a TPA3004D2 audio amplifier.

TODO:
- Identify the remaining cabinet outputs and sound reset connection.
- Confirm the audio oscillator frequency and MSM9811 differences from the MSM9810B.
- Verify the physical source of the Tomy frame interrupt currently connected to IRQ6.
- Measure dispenser sensor timing and identify the CommuTouch touchscreen controller.
- CommuTouch's optical card reader only implements the threshold handshake; use the no-card path.
- See the AG-2 device for the remaining graphics and timing limitations.


** means not dumped
*** means not dumped nor identified

Banpresto "Chara Medal Island" series:
    BP-01 - Crayon Shin-chan - Arashi o Yobu! Sushitori Daigassen - クレヨンしんちゃん 嵐を呼ぶ！すし取り大合戦
    BP-02 - Pocket Monster - Sonans ga Koronda! - ポケットモンスター ソーナンスがころんだ！
    BP-03 - Dragon Ball Z - Bucchigiri no Chou Kessen - ドラゴンボールZ ぶっちぎりの超決戦
*** BP-04 - ??? - ??? (possibly Eye Shield 21 - Nerae!! Gyakuten Touchdown - アイシールド21 狙え!!逆転タッチダウン)
    BP-05 - Chibi Maruko-chan - Wanage de Asobo!! - ちびまる子ちゃん輪投げで遊ぼ!!
**  BP-06 - Doraemon - SOS! Itazura Nezumi wo Yattsukero!! - ドラえもん SOS!いたずらネズミをやっつけろ!!
    BP-07 - GeGeGe no Kitarou - Taose! Seiyou Youkai Gundan - ゲゲゲの鬼太郎 倒せ！西洋妖怪軍団
    BP-08 - Keroro Gunsou - Hacha Mecha Yakyu Taikai de Arimasu!! - ケロロ軍曹 ハチャメチャ野球大会であります!!
    BP-09 - From TV Animation One Piece Nakama o Tasukero! Dr. Chopper - From TV Animation One Piece 仲間を助けろ！ ドクター Dr. チョッパー
almost surely more

Takara / Tomy "VS medal" series (games show produced by Amodio and Tomy copyright on title screen):
**  TM-01 - Kurohige Kiki Ippatsu - 黒ひげ危機一発
**  TM-02 - Tomica - トミカ
    TM-03 - Zoids - ゾイド
**  TM-04 - Keitei-oh - 海帝王
    TM-05 - Youkai VS Uchuujin - 妖怪ＶＳ宇宙人
    TM-06 - Satoyama Grand Prix - 里山グランプリ
    TM-07 - Ginga Yakyu - 銀河野球
    TM-08 - Yujo Gattai Shishigami-o - 友情合体獣神王
almost surely more

Banpresto CommuTouch series:
    BPCT1P - Anpanman CommuTouch - アンパンマンコミュタッチ
*/


#include "emu.h"

#include "cpu/h8/h8s2329.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/okim9810.h"
#include "video/axell_ag2.h"

#include "diserial.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>
#include <array>

#define LOG_BOARD (1U << 1)
#define LOG_SOUND (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"

namespace {

// Elo SmartSet-compatible touchscreen used by CommuTouch.  The controller
// model is not identified.  ROM 001666 configures SCI2 for 9600 8N1;
// 001d52 consumes ten-byte SmartSet packets via the H8 DTC.
// Protocol: Elo SmartSet Technical Reference Manual v2.0 (2002), chapter 6.
class anpanman_touch_device : public device_t, public device_serial_interface
{
public:
	anpanman_touch_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	auto tx_handler() { return m_tx_cb.bind(); }
	void rx_w(int state) { device_serial_interface::rx_w(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_serial_interface implementation
	virtual void rcv_complete() override;
	virtual void tra_complete() override;
	virtual void tra_callback() override { m_tx_cb(transmit_register_get_data_bit()); }

private:
	TIMER_CALLBACK_MEMBER(sample_touch);
	TIMER_CALLBACK_MEMBER(send_reply);
	void defaults();
	void command();
	void packet(const std::array<u8, 8> &data);
	void acknowledge(u8 error = '0');
	void send_next();
	void touch_packet(u8 status);

	devcb_write_line m_tx_cb;
	required_ioport m_touch;
	required_ioport m_x;
	required_ioport m_y;
	emu_timer *m_sample_timer;
	emu_timer *m_reply_timer;
	u8 m_command[10];
	u8 m_command_pos;
	u8 m_tx_packet_pos;
	u8 m_queue[256];
	u16 m_queue_head;
	u16 m_queue_count;
	u8 m_mode[2];
	u8 m_report[2];
	u8 m_filter[7];
	bool m_touch_down;
	u16 m_last_x;
	u16 m_last_y;
	u16 m_report_ticks;
	u16 m_release_ticks;
};


// CommuTouch has a reversible card transport with separate rotation and card
// sensors. ROM 000ed0 counts rotation edges; 000f56-001024 homes, feeds,
// retracts and advances a card, observing the card sensor independently.
// The sensor geometry and 20 ms rotation interval below are provisional.
class anpanman_card_device : public device_t
{
public:
	anpanman_card_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void motor_w(u8 data) { m_motor = data & 0xc0; }
	int rotation_r() { return BIT(m_position, 0); }
	int payout_r() { return !empty_r() && m_position >= 8 && m_position < 12; }
	int empty_r() { return BIT(m_empty->read(), 0); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(advance);

	required_ioport m_empty;
	emu_timer *m_timer;
	u8 m_position;
	u8 m_motor;
};


// Minimal dummy CommuTouch card code checker for the no-card game path.
// SCI1 is 38400 8N1; optical card reading is intentionally not implemented.
// ROM 0042b2 sends s/S for the threshold-set number, 00cf6a parses the
// response as ASCII '1' + index. The startup presence check (0753a6)
// requires this exchange even without a card on the reader.
// Optical card/mark/calibration commands are outside this dummy model.
// The controller model, factory threshold set and exact timing are unknown.
class anpanman_reader_device : public device_t, public device_serial_interface
{
public:
	anpanman_reader_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	auto tx_handler() { return m_tx_cb.bind(); }
	void rx_w(int state) { device_serial_interface::rx_w(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial_interface implementation
	virtual void rcv_complete() override;
	virtual void tra_complete() override;
	virtual void tra_callback() override { m_tx_cb(transmit_register_get_data_bit()); }

private:
	TIMER_CALLBACK_MEMBER(reply);
	void threshold_reply();

	devcb_write_line m_tx_cb;
	emu_timer *m_reply_timer;
	u8 m_threshold;
	u8 m_response[2];
	u8 m_response_pos;
	bool m_set_pending;
};

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ANPANMAN_TOUCH, anpanman_touch_device, anpanman_touch_device, "anpanman_touch", "CommuTouch SmartSet touchscreen")
DEFINE_DEVICE_TYPE_PRIVATE(ANPANMAN_CARD, anpanman_card_device, anpanman_card_device, "anpanman_card", "CommuTouch card dispenser")
DEFINE_DEVICE_TYPE_PRIVATE(ANPANMAN_READER, anpanman_reader_device, anpanman_reader_device, "anpanman_reader", "CommuTouch card code checker")

namespace {

static INPUT_PORTS_START(anpanman_touch)
	PORT_START("TOUCH")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Touch Screen")

	PORT_START("TOUCH_X")
	PORT_BIT(0x0fff, 0x0800, IPT_LIGHTGUN_X) PORT_NAME("Touch Screen X") PORT_MINMAX(0, 4095) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_CROSSHAIR(X, 1.0, 0.0, 0)

	PORT_START("TOUCH_Y")
	PORT_BIT(0x0fff, 0x0800, IPT_LIGHTGUN_Y) PORT_NAME("Touch Screen Y") PORT_MINMAX(0, 4095) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_REVERSE
INPUT_PORTS_END

anpanman_touch_device::anpanman_touch_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, ANPANMAN_TOUCH, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_tx_cb(*this),
	m_touch(*this, "TOUCH"),
	m_x(*this, "TOUCH_X"),
	m_y(*this, "TOUCH_Y"),
	m_sample_timer(nullptr),
	m_reply_timer(nullptr),
	m_command{},
	m_command_pos(0),
	m_tx_packet_pos(0),
	m_queue{},
	m_queue_head(0),
	m_queue_count(0),
	m_mode{},
	m_report{},
	m_filter{},
	m_touch_down(false),
	m_last_x(0),
	m_last_y(0),
	m_report_ticks(0),
	m_release_ticks(0)
{
}

ioport_constructor anpanman_touch_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(anpanman_touch);
}

void anpanman_touch_device::device_start()
{
	m_sample_timer = timer_alloc(FUNC(anpanman_touch_device::sample_touch), this);
	m_reply_timer = timer_alloc(FUNC(anpanman_touch_device::send_reply), this);
	save_item(NAME(m_command));
	save_item(NAME(m_command_pos));
	save_item(NAME(m_tx_packet_pos));
	save_item(NAME(m_queue));
	save_item(NAME(m_queue_head));
	save_item(NAME(m_queue_count));
	save_item(NAME(m_mode));
	save_item(NAME(m_report));
	save_item(NAME(m_filter));
	save_item(NAME(m_touch_down));
	save_item(NAME(m_last_x));
	save_item(NAME(m_last_y));
	save_item(NAME(m_report_ticks));
	save_item(NAME(m_release_ticks));
}

void anpanman_touch_device::defaults()
{
	m_mode[0] = 0x87;
	m_mode[1] = 0;
	m_report[0] = 3;
	m_report[1] = 1;
	// IntelliTouch defaults. Anpanman replaces these with F32 e0 2e 01 02 52.
	static constexpr u8 FILTER[7] = { '2', 0x80, 0x09, 2, 2, 22, 0 };
	std::copy(std::begin(FILTER), std::end(FILTER), std::begin(m_filter));
}

void anpanman_touch_device::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(9600);
	receive_register_reset();
	transmit_register_reset();
	m_command_pos = m_tx_packet_pos = 0;
	m_queue_head = m_queue_count = 0;
	m_touch_down = false;
	m_report_ticks = m_release_ticks = 0;
	m_last_x = m_last_y = 0;
	defaults();
	m_tx_cb(1);
	m_reply_timer->adjust(attotime::never);
	m_sample_timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10));
}

void anpanman_touch_device::packet(const std::array<u8, 8> &data)
{
	if (m_queue_count > std::size(m_queue) - 10)
	{
		logerror("SmartSet output queue full\n");
		return;
	}
	auto append = [this] (u8 value) { m_queue[(m_queue_head + m_queue_count++) & 255] = value; };
	append(0x55);
	u8 checksum = 0xff; // 0xaa + 0x55
	for (u8 value : data)
	{
		append(value);
		checksum += value;
	}
	append(checksum);
	// Serial command processing has a finite turnaround. Keep whole packets
	// contiguous and give the H8 time to rearm its ten-byte receive DTC.
	if (is_transmit_register_empty() && m_reply_timer->remaining().is_never())
		m_reply_timer->adjust(attotime::from_msec(2));
}

void anpanman_touch_device::acknowledge(u8 error)
{
	packet({ 'A', error, '0', '0', '0', 0, 0, 0 });
}

void anpanman_touch_device::send_next()
{
	if (m_queue_count)
	{
		transmit_register_setup(m_queue[m_queue_head]);
		m_queue_head = (m_queue_head + 1) & 255;
		--m_queue_count;
	}
}

TIMER_CALLBACK_MEMBER(anpanman_touch_device::send_reply)
{
	send_next();
}

void anpanman_touch_device::tra_complete()
{
	// Ten bytes exactly fill the firmware's receive DTC. A short packet gap
	// allows its ISR and foreground receiver to rearm before the next packet.
	m_tx_packet_pos = (m_tx_packet_pos + 1) % 10;
	if (!m_tx_packet_pos && m_queue_count)
		m_reply_timer->adjust(attotime::from_msec(2));
	else
		send_next();
}

void anpanman_touch_device::rcv_complete()
{
	receive_register_extract();
	const u8 value = get_received_char();
	if (!m_command_pos && value != 0x55)
		return;
	m_command[m_command_pos++] = value;
	if (m_command_pos == std::size(m_command))
	{
		m_command_pos = 0;
		command();
	}
}

void anpanman_touch_device::command()
{
	// Power-on serial configuration does not require host checksums. Responses
	// always carry the checksum; Anpanman supplies valid checksums regardless.
	switch (m_command[1])
	{
	case 'R':
		m_tx_packet_pos = 0;
		m_queue_head = m_queue_count = 0;
		transmit_register_reset();
		m_reply_timer->adjust(attotime::never);
		if (m_command[2] == '0' || m_command[2] == '2')
		{
			defaults();
			return; // documented hard resets have no acknowledgement
		}
		break;
	case 'M':
		if (m_command[2])
		{
			acknowledge('C');
			return;
		}
		m_mode[0] = m_command[3];
		m_mode[1] = m_command[4];
		break;
	case 'm': packet({ 'M', 0, m_mode[0], m_mode[1], 0, 0, 0, 0 }); break;
	case 'B':
		m_report[0] = m_command[2] & 15;
		m_report[1] = m_command[3];
		break;
	case 'b': packet({ 'B', m_report[0], m_report[1], 0, 0, 0, 0, 0 }); break;
	case 'F':
		std::copy(m_command + 2, m_command + 9, std::begin(m_filter));
		break;
	case 'f': packet({ 'F', m_filter[0], m_filter[1], m_filter[2], m_filter[3], m_filter[4], m_filter[5], m_filter[6] }); break;
	case 'D':
	case 'd': packet({ 'D', 0, 0, 0, 0, 0, 0, 0 }); break;
	case 'p': packet({ 'P', '0', 5, 0, 0, 0, 0, 0 }); break;
	case 'a': break;
	default:
		logerror("Unsupported SmartSet command %02x\n", m_command[1]);
		acknowledge('5');
		return;
	}
	acknowledge();
}

void anpanman_touch_device::touch_packet(u8 status)
{
	// Normal pressure: the game warns about hitting the screen at Z >= 200.
	packet({ 'T', u8(status | (BIT(m_mode[0], 7) ? 0x80 : 0)), u8(m_last_x), u8(m_last_x >> 8), u8(m_last_y), u8(m_last_y >> 8), u8(status == 4 ? 0 : 128), 0 });
}

TIMER_CALLBACK_MEMBER(anpanman_touch_device::sample_touch)
{
	const bool down = BIT(m_touch->read(), 0);
	if (down)
	{
		m_release_ticks = 0;
		m_last_x = m_x->read();
		m_last_y = m_y->read();
		if (!m_touch_down)
		{
			m_touch_down = true;
			m_report_ticks = 0;
			if (BIT(m_mode[0], 0))
				touch_packet(1);
		}
		else if (++m_report_ticks >= std::max<u32>(1, m_report[1]))
		{
			m_report_ticks = 0;
			if (BIT(m_mode[0], 1) && !m_queue_count && is_transmit_register_empty())
				touch_packet(2);
		}
	}
	else if (m_touch_down && ++m_release_ticks >= std::max<u32>(1, m_report[0]))
	{
		m_touch_down = false;
		m_release_ticks = m_report_ticks = 0;
		if (BIT(m_mode[0], 2))
			touch_packet(4);
	}
}


static INPUT_PORTS_START(anpanman_card)
	PORT_START("EMPTY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Card Dispenser Empty") PORT_TOGGLE
INPUT_PORTS_END

anpanman_card_device::anpanman_card_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, ANPANMAN_CARD, tag, owner, clock),
	m_empty(*this, "EMPTY"),
	m_timer(nullptr),
	m_position(0),
	m_motor(0)
{
}

ioport_constructor anpanman_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(anpanman_card);
}

void anpanman_card_device::device_start()
{
	m_timer = timer_alloc(FUNC(anpanman_card_device::advance), this);
	save_item(NAME(m_position));
	save_item(NAME(m_motor));
}

void anpanman_card_device::device_reset()
{
	// Reset the drive without teleporting the physical transport.
	m_motor = 0;
	m_timer->adjust(attotime::from_msec(20), 0, attotime::from_msec(20));
}

TIMER_CALLBACK_MEMBER(anpanman_card_device::advance)
{
	// 80 feeds forward, 40 reverses, C0 brakes and 00 switches the motor off.
	if (m_motor == 0x80)
		m_position = (m_position + 1) & 15;
	else if (m_motor == 0x40)
		m_position = (m_position - 1) & 15;
}


anpanman_reader_device::anpanman_reader_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, ANPANMAN_READER, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_tx_cb(*this),
	m_reply_timer(nullptr),
	m_threshold('1'),
	m_response{},
	m_response_pos(0),
	m_set_pending(false)
{
}

void anpanman_reader_device::device_start()
{
	m_reply_timer = timer_alloc(FUNC(anpanman_reader_device::reply), this);
	save_item(NAME(m_threshold));
	save_item(NAME(m_response));
	save_item(NAME(m_response_pos));
	save_item(NAME(m_set_pending));
}

void anpanman_reader_device::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(38400);
	receive_register_reset();
	transmit_register_reset();
	m_threshold = '1';
	m_response_pos = std::size(m_response);
	m_set_pending = false;
	m_reply_timer->adjust(attotime::never);
	m_tx_cb(1);
}

void anpanman_reader_device::threshold_reply()
{
	m_response[0] = m_threshold;
	m_response[1] = 0x0d;
	m_response_pos = 0;
	// Nominal processing delay; allows the host to arm its response DTC.
	m_reply_timer->adjust(attotime::from_msec(2));
}

TIMER_CALLBACK_MEMBER(anpanman_reader_device::reply)
{
	tra_complete();
}

void anpanman_reader_device::tra_complete()
{
	if (m_response_pos < std::size(m_response))
		transmit_register_setup(m_response[m_response_pos++]);
}

void anpanman_reader_device::rcv_complete()
{
	receive_register_extract();
	const u8 data = get_received_char();
	if (m_set_pending)
	{
		m_set_pending = false;
		if (data >= '1' && data <= '9')
		{
			m_threshold = data;
			threshold_reply();
		}
		return;
	}
	switch (data)
	{
	case 's': threshold_reply(); break;
	case 'S': m_set_pending = true; break;

	default: logerror("Unimplemented card code checker command %02x\n", data); break;
	}
}

class banpresto_tomy_h8s_state : public driver_device
{
protected:
	banpresto_tomy_h8s_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ag2(*this, "ag2"),
		m_oki(*this, "oki"),
		m_dsw(*this, "DSW%u", 1U)
	{
	}

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void common(machine_config &config) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;

	void coin_lockout_w(u8 data);

	required_device<h8s2320_device> m_maincpu;
	required_device<ax51201_device> m_ag2;
	required_device<okim9810_device> m_oki;

private:
	required_ioport_array<2> m_dsw;

	u8 m_dsw_select = 0x0c;
	u8 m_sound_data = 0;
	u8 m_sound_control = 0x0f;

	u8 unknown_r(offs_t offset);
	void unknown_w(offs_t offset, u8 data);
	void sound_data_w(u8 data);
	void sound_control_w(u8 data);
	u8 dsw_r();
	void dsw_select_w(u8 data);
};


class banpresto_state : public banpresto_tomy_h8s_state
{
public:
	banpresto_state(const machine_config &mconfig, device_type type, const char *tag) :
		banpresto_tomy_h8s_state(mconfig, type, tag),
		m_hopper(*this, "hopper"),
		m_lamps(*this, "lamp%u", 1U),
		m_win_lamps(*this, "winlamp%u", 1U)
	{
	}

	void banpresto(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<hopper_device> m_hopper;
	output_finder<2> m_lamps;
	output_finder<4> m_win_lamps;

	void banpresto_map(address_map &map) ATTR_COLD;
	void outputs_w(u8 data);
	void motor_w(u8 data);
};


class tomy_state : public banpresto_tomy_h8s_state
{
public:
	tomy_state(const machine_config &mconfig, device_type type, const char *tag) :
		banpresto_tomy_h8s_state(mconfig, type, tag),
		m_hopper(*this, "hopper")
	{
	}

	void tomy(machine_config &config) ATTR_COLD;

private:
	required_device<hopper_device> m_hopper;

	void tomy_map(address_map &map) ATTR_COLD;
	void outputs_w(u8 data);
	void motor_w(u8 data);
};


class anpanman_state : public banpresto_tomy_h8s_state
{
public:
	anpanman_state(const machine_config &mconfig, device_type type, const char *tag) :
		banpresto_tomy_h8s_state(mconfig, type, tag),
		m_card(*this, "card")
	{
	}

	void commutouch(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<anpanman_card_device> m_card;

	void commutouch_map(address_map &map) ATTR_COLD;
	void outputs_w(u8 data);
	void motor_w(u8 data);
};


static INPUT_PORTS_START( base )
	PORT_START("P1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Medal Jam Sensor")
	// These are independent positive pulses, not a two-sensor coin sequence.
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Medal") PORT_IMPULSE(2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("100 Yen") PORT_IMPULSE(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	// The I/O test calls the four panel buttons SW1-SW4 and lights LAMP1/LAMP2 for SW1/SW2.
	PORT_START("P4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Start / Action")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1)
	PORT_SERVICE_NO_TOGGLE(0x20, IP_ACTIVE_HIGH)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN)

	// Both banks share P2. P5 bits 3/2 select DSW1/DSW2 respectively.
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW2:8")
INPUT_PORTS_END


static INPUT_PORTS_START( banpresto )
	PORT_INCLUDE(base)

	// Shared settings decoder in all six Banpresto medal programs.  The
	// jackpot settings are named as on the GAME INFORMATION-2 test screen
	// (JP PAYOUT, JP MODE with the values NC and MJ).
	PORT_MODIFY("DSW1")
	PORT_DIPNAME(0x03, 0x03, "JP Payout") PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(0x00, "3%")
	PORT_DIPSETTING(0x01, "6%")
	PORT_DIPSETTING(0x02, "9%")
	PORT_DIPSETTING(0x03, "12%")
	PORT_DIPNAME(0x04, 0x04, "JP Mode") PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(0x00, "MJ")
	PORT_DIPSETTING(0x04, "NC")
	PORT_DIPNAME(0xc0, 0xc0, DEF_STR(Demo_Sounds)) PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(0xc0, DEF_STR(Off))
	PORT_DIPSETTING(0x80, "Every Attract Cycle")
	PORT_DIPSETTING(0x40, "Every Other Attract Cycle")
	PORT_DIPSETTING(0x00, "Every Fourth Attract Cycle")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME(0x07, 0x07, "Payout Rate") PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(0x00, "55%")
	PORT_DIPSETTING(0x01, "60%")
	PORT_DIPSETTING(0x02, "65%")
	PORT_DIPSETTING(0x03, "70%")
	PORT_DIPSETTING(0x04, "75%")
	PORT_DIPSETTING(0x05, "80%")
	PORT_DIPSETTING(0x06, "85%")
	PORT_DIPSETTING(0x07, "90%")
	PORT_DIPNAME(0x08, 0x08, "Win Wave") PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(0x00, "Small")
	PORT_DIPSETTING(0x08, "Big")
	PORT_DIPNAME(0xf0, 0xf0, "Medals per 100 Yen") PORT_DIPLOCATION("DSW2:5,6,7,8")
	PORT_DIPSETTING(0x00, "5")
	PORT_DIPSETTING(0x10, "6")
	PORT_DIPSETTING(0x20, "7")
	PORT_DIPSETTING(0x30, "8")
	PORT_DIPSETTING(0x40, "9")
	PORT_DIPSETTING(0x50, "10")
	PORT_DIPSETTING(0x60, "11")
	PORT_DIPSETTING(0x70, "12")
	PORT_DIPSETTING(0x80, "13")
	PORT_DIPSETTING(0x90, "14")
	PORT_DIPSETTING(0xa0, "15")
	PORT_DIPSETTING(0xb0, "16")
	PORT_DIPSETTING(0xc0, "17")
	PORT_DIPSETTING(0xd0, "18")
	PORT_DIPSETTING(0xe0, "19")
	PORT_DIPSETTING(0xf0, "20")
INPUT_PORTS_END


static INPUT_PORTS_START( tomy )
	PORT_INCLUDE(base)

	PORT_MODIFY("P1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	// Only Zoids debounces this third pulse channel; no consumer is known.
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN)

	// Later Tomy games have one control-panel button. The other three
	// P4 button inputs used by the Banpresto library are not sampled.
	PORT_MODIFY("P4")
	PORT_BIT(0x0e, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


static INPUT_PORTS_START( commutouch )
	PORT_INCLUDE(base)

	PORT_MODIFY("P1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coin Jam Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("card", FUNC(anpanman_card_device::empty_r))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("100 Yen") PORT_IMPULSE(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("card", FUNC(anpanman_card_device::rotation_r))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("card", FUNC(anpanman_card_device::payout_r))

	PORT_MODIFY("P4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Card Read Switch") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x0e, IP_ACTIVE_HIGH, IPT_UNUSED)

	// ROM 0752e2/001392 sets the coin threshold to (DSW1 & 3) + 1.
	PORT_MODIFY("DSW1")
	PORT_DIPNAME(0x03, 0x03, DEF_STR(Coinage)) PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(0x03, DEF_STR(4C_1C))
	PORT_DIPSETTING(0x02, DEF_STR(3C_1C))
	PORT_DIPSETTING(0x01, DEF_STR(2C_1C))
	PORT_DIPSETTING(0x00, DEF_STR(1C_1C))
INPUT_PORTS_END


void banpresto_tomy_h8s_state::program_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0xc00000, 0xc07fff).ram().share("nvram");
	map(0xc40000, 0xcbffff).rw(FUNC(banpresto_tomy_h8s_state::unknown_r), FUNC(banpresto_tomy_h8s_state::unknown_w));
	map(0xc40000, 0xc40000).w(FUNC(banpresto_tomy_h8s_state::sound_data_w));
	map(0xc60000, 0xc60000).w(FUNC(banpresto_tomy_h8s_state::sound_control_w));
}

void banpresto_state::banpresto_map(address_map &map)
{
	program_map(map);
	// CPU code identifies a 128 KiB host aperture, with registers at +1fc00.
	map(0x200000, 0x21ffff).m(m_ag2, FUNC(ax51201_device::map));
	map(0xc80000, 0xc80000).w(FUNC(banpresto_state::outputs_w));
	map(0xca0000, 0xca0000).w(FUNC(banpresto_state::motor_w));
}

void tomy_state::tomy_map(address_map &map)
{
	program_map(map);
	map(0x200000, 0x21ffff).m(m_ag2, FUNC(ax51201_device::map));
	map(0xc80000, 0xc80000).w(FUNC(tomy_state::outputs_w));
	map(0xca0000, 0xca0000).w(FUNC(tomy_state::motor_w));
}

void anpanman_state::commutouch_map(address_map &map)
{
	program_map(map);
	// Anpanman code uses 41fc00, unlike the other BPSC/TMSC programs.
	map(0x400000, 0x41ffff).m(m_ag2, FUNC(ax51201_device::map));
	map(0xc80000, 0xc80000).w(FUNC(anpanman_state::outputs_w));
	map(0xca0000, 0xca0000).w(FUNC(anpanman_state::motor_w));
}


u8 banpresto_tomy_h8s_state::unknown_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_BOARD, "BOARD %s R8 address=%06x data=ff unimplemented\n", machine().describe_context(), 0xc40000 + offset);
	return 0xff;
}

void banpresto_tomy_h8s_state::unknown_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_BOARD, "BOARD %s W8 address=%06x data=%02x unimplemented\n", machine().describe_context(), 0xc40000 + offset, data);
}

void banpresto_tomy_h8s_state::sound_data_w(u8 data)
{
	m_sound_data = data;
}

void banpresto_tomy_h8s_state::sound_control_w(u8 data)
{
	// Parallel OKI bus: bit 0 CMD, bit 1 /RD, bit 2 /WR, bit 3 /CS.
	// Latch on /WR rising: Tomy programs put data on the bus after
	// asserting /WR (e.g. Youkai 0036a6), unlike the Banpresto library.
	if (!BIT(m_sound_control, 2) && BIT(data, 2) && !BIT(data, 3))
	{
		LOGMASKED(LOG_SOUND, "SOUND %s %s %02x\n", machine().describe_context(), BIT(data, 0) ? "parameter" : "command", m_sound_data);
		if (BIT(data, 0))
			m_oki->tmp_register_w(m_sound_data);
		else
			m_oki->write(m_sound_data);
	}
	m_sound_control = data;
}

void banpresto_tomy_h8s_state::coin_lockout_w(u8 data)
{
	// The firmware drops these enables when the credit limit is
	// reached, during payout and in the operator menus.
	machine().bookkeeping().coin_lockout_w(0, !BIT(data, 2));
	machine().bookkeeping().coin_lockout_w(1, !BIT(data, 0));
}

// Physical accounting meters are pulsed by the firmware, separately
// from the credit and payout totals in battery-backed RAM.
void banpresto_state::outputs_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 4)); // medal in
	machine().bookkeeping().coin_counter_w(1, BIT(data, 3)); // 100 yen
	machine().bookkeeping().coin_counter_w(2, BIT(data, 5)); // medal out
	coin_lockout_w(data);

	m_win_lamps[2] = BIT(data, 6);
	m_win_lamps[3] = BIT(data, 7);
}

void tomy_state::outputs_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 5)); // medal in
	machine().bookkeeping().coin_counter_w(1, BIT(data, 3)); // 100 yen
	machine().bookkeeping().coin_counter_w(2, BIT(data, 6)); // medal out
	coin_lockout_w(data);
}

void anpanman_state::outputs_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 3)); // 100 yen
	machine().bookkeeping().coin_lockout_w(0, !BIT(data, 0));
}

void banpresto_state::motor_w(u8 data)
{
	// Two motor-direction outputs: 01/10 run, while 00/11 stop/brake
	// (cscaoysd 021c20/021e38).
	m_hopper->motor_w(BIT(data, 6) != BIT(data, 7));

	// I/O test: SW1/SW2 -> LAMP1/LAMP2. Holding payout cycles the four
	// WIN LAMP outputs (cscaoysd 0237e6).
	m_lamps[0] = BIT(data, 0);
	m_lamps[1] = BIT(data, 1);
	m_win_lamps[0] = BIT(data, 2);
	m_win_lamps[1] = BIT(data, 3);
}

void tomy_state::motor_w(u8 data)
{
	m_hopper->motor_w(BIT(data, 5));
}

void anpanman_state::motor_w(u8 data)
{
	m_card->motor_w(data);

	// CommuTouch 002de4 stops the active voices before selecting
	// one of the two 16 MiB sound ROM banks via 0015a4.
	m_oki->set_rom_bank(BIT(data, 3));
}


void banpresto_tomy_h8s_state::machine_start()
{
	save_item(NAME(m_dsw_select));
	save_item(NAME(m_sound_data));
	save_item(NAME(m_sound_control));
}

void banpresto_tomy_h8s_state::machine_reset()
{
	m_dsw_select = 0x0c;
	m_sound_data = 0;
	m_sound_control = 0x0f;
}

void banpresto_state::machine_reset()
{
	banpresto_tomy_h8s_state::machine_reset();

	for (auto &lamp : m_lamps)
		lamp = 0;

	for (auto &lamp : m_win_lamps)
		lamp = 0;
}

void anpanman_state::machine_reset()
{
	banpresto_tomy_h8s_state::machine_reset();

	m_oki->set_rom_bank(0);
}

u8 banpresto_tomy_h8s_state::dsw_r()
{
	u8 data = 0xff;

	if (!BIT(m_dsw_select, 3))
		data &= m_dsw[0]->read();

	if (!BIT(m_dsw_select, 2))
		data &= m_dsw[1]->read();

	// P2 shares the DIP switch buffers and the OKI's parallel status bus.
	// The firmware deasserts /RD before selecting either DIP bank.
	if (!BIT(m_sound_control, 3) && !BIT(m_sound_control, 1))
		data &= m_oki->read();

	return data;
}

void banpresto_tomy_h8s_state::dsw_select_w(u8 data)
{
	m_dsw_select = data;
}


void banpresto_tomy_h8s_state::common(machine_config &config)
{
	// P1 carries medal sensors; P4 carries active-high buttons/service inputs.
	// P5 also selects the DIP banks on P2. Bit 0 is the board-ready input.
	m_maincpu->read_port1().set_ioport("P1");
	m_maincpu->read_port2().set(FUNC(banpresto_tomy_h8s_state::dsw_r));
	m_maincpu->read_port4().set_ioport("P4");
	m_maincpu->read_port5().set_constant(0xff);
	m_maincpu->write_port5().set(FUNC(banpresto_tomy_h8s_state::dsw_select_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	AX51201(config, m_ag2, 24.576_MHz_XTAL); // TODO: verify, assumed to share the CPU XTAL
	m_ag2->set_screen("screen");

	screen_device &screen(SCREEN(config, "screen")); // TODO
	screen.set_refresh_hz(60);
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479);
	screen.set_screen_update(m_ag2, FUNC(ax51201_device::screen_update));

	SPEAKER(config, "speaker", 2).front();

	okim9810_device &oki(OKIM9810(config, m_oki, 4.096_MHz_XTAL)); // M9810B or M9811
	oki.add_route(0, "speaker", 0.80, 0);
	oki.add_route(1, "speaker", 0.80, 1);
}

void banpresto_state::banpresto(machine_config &config)
{
	H8S2324(config, m_maincpu, 24.576_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &banpresto_state::banpresto_map);

	common(config);

	// cscaoysd vectors 17 and 21 acknowledge status bits 6 and 4.
	m_ag2->event_cb<3>().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
	m_ag2->event_cb<2>().set_inputline(m_maincpu, INPUT_LINE_IRQ5);

	// Nominal sensor transition interval; exact mechanical timing is unmeasured.
	HOPPER(config, m_hopper, attotime::from_msec(100));
}

void tomy_state::tomy(machine_config &config)
{
	H8S2329(config, m_maincpu, 24.576_MHz_XTAL);
	// Expanded mode without internal ROM: reset and exception vectors are
	// supplied by the external program ROM, as are all observed routines.
	m_maincpu->set_mode(4);
	m_maincpu->set_addrmap(AS_PROGRAM, &tomy_state::tomy_map);

	common(config);

	m_ag2->event_cb<3>().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
	m_ag2->event_cb<2>().set_inputline(m_maincpu, INPUT_LINE_IRQ5);

	// Later Tomy programs use IRQ6 as their frame clock and to service
	// the accounting meters (Youkai 000432/005644). The exact board-level
	// source of this pulse is unverified; it is independent of AG-2 status.
	subdevice<screen_device>("screen")->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_IRQ6);

	// Nominal sensor transition interval; exact mechanical timing is unmeasured.
	HOPPER(config, m_hopper, attotime::from_msec(100));
}

void anpanman_state::commutouch(machine_config &config)
{
	H8S2324(config, m_maincpu, 24.576_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &anpanman_state::commutouch_map);

	common(config);

	// CommuTouch vectors 16/17/22 acknowledge AG-2 bits 6/4/2.
	m_ag2->event_cb<3>().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ag2->event_cb<2>().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
	m_ag2->event_cb<1>().set_inputline(m_maincpu, INPUT_LINE_IRQ6);

	ANPANMAN_CARD(config, m_card, 0);

	ANPANMAN_TOUCH(config, "touch", 0).tx_handler().set(m_maincpu, FUNC(h8s2320_device::sci_rx_w<2>));
	m_maincpu->write_sci_tx<2>().set("touch", FUNC(anpanman_touch_device::rx_w));

	ANPANMAN_READER(config, "reader", 0).tx_handler().set(m_maincpu, FUNC(h8s2320_device::sci_rx_w<1>));
	m_maincpu->write_sci_tx<1>().set("reader", FUNC(anpanman_reader_device::rx_w));
}


/***************************************************************************

  ROM definition(s)

***************************************************************************/

ROM_START( cscaoysd )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "bp01-p_ver1.1.ic5", 0x000000, 0x080000, CRC(daa443b1) SHA1(f2a133a9a8cc9678f81a4e7e10f4d15af99ab530) )

	ROM_REGION( 0x400000, "ag2", 0 )
	ROM_LOAD16_WORD_SWAP( "bp01-g_ver1.0.ic6", 0x000000, 0x400000, CRC(716ae57c) SHA1(e1bc89e0dd126574742496cd32d3586f6e584307) ) // x1xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "bp01-s_ver1.0.ic4", 0x000000, 0x100000, CRC(40e957a5) SHA1(444d141b54d5f08520521bd89edc1d0bc10486c0) )
ROM_END

ROM_START( pmsgk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "ic5", 0x000000, 0x400000, CRC(8683fecb) SHA1(520f71c701d1ebf90656ac7656387ef558a8effb) ) // x11xxxxxxxxxxxxxxxxxxx = 0xFF, no label

	ROM_REGION( 0x400000, "ag2", 0 )
	ROM_LOAD16_WORD_SWAP( "ic6", 0x000000, 0x400000, CRC(b59f9ec6) SHA1(b3fe0dec5587344ea734110eb0508173d6df28bc) ) // 11xxxxxxxxxxxxxxxxxxxx = 0xFF, no label

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "bp02-s_ver1.0.ic4", 0x000000, 0x100000, CRC(01c6d366) SHA1(d8fb30875565ca9c9201be273e3946e383fb5bea) )
ROM_END

ROM_START( dbzbgck )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "bp03-p_ver1.1.ic5", 0x000000, 0x400000, CRC(6f9c1658) SHA1(c175781d6090468c20b7fb4ab118a41a675873c2) ) // 111xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "ag2", 0 )
	ROM_LOAD16_WORD_SWAP( "bp03-g_ver1.0.ic6", 0x000000, 0x400000, CRC(ae9d18b4) SHA1(e4fd9f118da9aa0b9e7395c6d4428844bcc6e2d8) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "bp03-s_ver1.0.ic4", 0x000000, 0x100000, CRC(0cbf97b4) SHA1(c39106e4414cf55bd3b9aaf7d3c37de7fc14b48f) )
ROM_END

ROM_START( dbzbgck10 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "ic5", 0x000000, 0x400000, CRC(267c21ac) SHA1(bde66cb5837f8a93812d54fcb3e857290ff3fed6) ) // 111xxxxxxxxxxxxxxxxxxx = 0xFF, no label

	ROM_REGION( 0x400000, "ag2", 0 )
	ROM_LOAD16_WORD_SWAP( "bp03-g_ver1.0.ic6", 0x000000, 0x400000, CRC(ae9d18b4) SHA1(e4fd9f118da9aa0b9e7395c6d4428844bcc6e2d8) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "bp03-s_ver1.0.ic4", 0x000000, 0x100000, CRC(0cbf97b4) SHA1(c39106e4414cf55bd3b9aaf7d3c37de7fc14b48f) )
ROM_END

ROM_START( cmcwa )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "bp05-p_ver1.1.ic5", 0x000000, 0x400000, CRC(f5efd28c) SHA1(a565de1bbb88d949af20150c93db850523ac3c85) ) // 111xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "ag2", 0 )
	ROM_LOAD16_WORD_SWAP( "bp05-g_ver1.0.ic6", 0x000000, 0x400000, CRC(cef0dab1) SHA1(57e4e62801dcc77e4dbb3ace9788f62fed67cb0a) ) // 11xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "bp05-s_ver1.0.ic4", 0x000000, 0x100000, CRC(5ee334cc) SHA1(6cbc237980d2aadb28970bd5eecec3c2cf9f873f) )
ROM_END

ROM_START( gkitarou )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "bp07-p_ver1.0.ic5", 0x000000, 0x400000, CRC(ac96d432) SHA1(671fc31ecb1e6cfbd6bdc6c4d3084c8e665d5aa6) ) // 111xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "ag2", 0 )
	ROM_LOAD16_WORD_SWAP( "bp07-g_ver1.0.ic6", 0x000000, 0x400000, CRC(dc43236f) SHA1(98b27141d215b490216f5685886c30840e4b1983) ) // 1xxxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "bp07-s_ver1.0.ic4", 0x000000, 0x100000, CRC(56bbede1) SHA1(c68a21ede8f8379ad8f497d3df049cb05131ecc6) )
ROM_END

ROM_START( kghmytda )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "bp08-p_ver1.0.ic5", 0x000000, 0x400000, CRC(d8d49df1) SHA1(7060094305bea6ee9f58ff42c23c64961fc825a9) ) // 111xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "ag2", 0 )
	ROM_LOAD16_WORD_SWAP( "bp08-g_ver1.0.ic6", 0x000000, 0x400000, CRC(8a3a3b1c) SHA1(b3c635e0d61a41c3b492b604e77d8bb492ac39e6) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "bp08-s_ver1.0.ic4", 0x000000, 0x100000, CRC(b30b9f44) SHA1(98c60021c70524dcf9d55bb0d55be78071f85969) )
ROM_END

ROM_START( dchopper )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "bp09-p_ver1.0.ic5", 0x000000, 0x400000, CRC(24c336c5) SHA1(4e0c6614cf7cbbc3509943001ad5abfc39a1991a) ) // 2ND HALF = xxFF

	ROM_REGION( 0x400000, "ag2", 0 )
	ROM_LOAD16_WORD_SWAP( "bp09-g_ver1.0.ic6", 0x000000, 0x400000, CRC(73883d87) SHA1(34be7184888a2ffe5c51cf80785bb77ce4de4d39) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "bp09-s_ver1.0.ic4", 0x000000, 0x100000, CRC(8e82fa09) SHA1(4255b8d508ba983857079f28008a94faa6fe1e49) )
ROM_END

ROM_START( zoids )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "tm03-p_ver1.0.ic5", 0x000000, 0x080000, CRC(becd0060) SHA1(68b72a3e7270f3416935db018b6feeaaa8e4aeae) )

	ROM_REGION( 0x400000, "ag2", 0 )
	ROM_LOAD16_WORD_SWAP( "tm03-g_ver1.0.ic6", 0x000000, 0x400000, CRC(1dc85b79) SHA1(f144d085c77efa8b4ff23b75f01b71a3d2c9a3b0) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "tm03-s_ver1.0.ic4", 0x000000, 0x040000, CRC(d0a24b61) SHA1(1e12a20caefcaa220fa83bb60453ea970ee51ac1) )
ROM_END

ROM_START( youkai )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "tm05-p_ver1.0.ic5", 0x000000, 0x100000, CRC(dfec7575) SHA1(f707be1818103834f36772be93b162b2b32fd0e2) ) // 11xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "ag2", 0 )
	ROM_LOAD16_WORD_SWAP( "tm05-g_ver1.0.ic6", 0x000000, 0x400000, CRC(4c999232) SHA1(ca2498753247fe27c7285680df7b23bf65aa648c) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "tm05-s_ver1.0.ic4", 0x000000, 0x100000, CRC(f2534577) SHA1(9ef308c3fc6c04b04a3282417b2660545d8f154e) ) // 11xxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

ROM_START( satoyama )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "tm06-p_ver1.0.ic5", 0x000000, 0x100000, CRC(f02cddba) SHA1(27162dbfdf364e16a490b7dfb3db0b19bcc54d41) ) // 11xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "ag2", 0 )
	ROM_LOAD16_WORD_SWAP( "tm06-g_ver1.0.ic6", 0x000000, 0x400000, CRC(b36fd1ee) SHA1(b576219273d25f6bfcc2e2f6f81e7c4a208fc70b) ) // another dump had 0x2f4322 as 0x1c instead of 0x0c

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "tm06-s_ver1.0.ic4", 0x000000, 0x100000, CRC(133c4dc8) SHA1(9af3ec284212268302b15f89831816a42ba3f353) )
ROM_END

ROM_START( gyakyu )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "tm07-p_ver1.0.ic5", 0x000000, 0x100000, CRC(ca3f2d4b) SHA1(36df59092af114b419d147d78eb4672fe1200ae4) ) // 11xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "ag2", 0 )
	ROM_LOAD16_WORD_SWAP( "tm07-g_ver1.0.ic6", 0x000000, 0x400000, CRC(9ff53c1e) SHA1(3edc38496a54a94df42b0b5ba5c7a888e9c2527c) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "tm07-s_ver1.0.ic4", 0x000000, 0x100000, CRC(05276f20) SHA1(501ce4fc14593e3f5e8ed7e08b8b7006d65212c6) )
ROM_END

ROM_START( yujogs )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "tm08-p_ver1.0.ic5", 0x000000, 0x100000, CRC(8c393893) SHA1(f8fad4f831c08bdf49400dc790a6693718408e15) ) // 1xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "ag2", 0 )
	ROM_LOAD16_WORD_SWAP( "tm08-g_ver1.0.ic6", 0x000000, 0x400000, CRC(6b08c87c) SHA1(afa4cc64722fadac49fc62bb071c4b86311239f9) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "tm08-s_ver1.0.ic4", 0x000000, 0x100000, CRC(51632db2) SHA1(acc85dfc5250384775f58c66619ec0b701d0b218) )
ROM_END

ROM_START( anpanmct ) // uses touch screen
	ROM_REGION( 0x400000, "maincpu", 0 )
	// Dump is circularly displaced by 0x2e bytes. Its reset vector is at
	// file 0x3fffd2 and points to 0x500 (code at file 0x4d2); the
	// interrupt vectors and absolute subroutine addresses agree.
	ROM_LOAD16_WORD_SWAP( "bpct1p_ver1.0.rom1", 0x00002e, 0x3fffd2, BAD_DUMP CRC(2a0326cf) SHA1(623617d1ffb686673caee3c345b6052b4c5d4af7) )
	ROM_CONTINUE(0x000000, 0x00002e)

	ROM_REGION( 0x2000000, "ag2", 0 )
	ROM_LOAD16_WORD_SWAP( "bp-cmtc1-g_ver1.0.rom3", 0x0000000, 0x2000000, BAD_DUMP CRC(7e482971) SHA1(1e5b0801515292731ac639b6e0d53b406af1138d) ) // flagged bad when dumped, not verified since

	ROM_REGION( 0x2000000, "oki", 0 )
	ROM_LOAD( "bp-cmtc1-s_ver1.0.rom2", 0x0000000, 0x2000000, CRC(337d688a) SHA1(276466924e6a23f531a7100d25390fdaae4fecfb) )
ROM_END

} // anonymous namespace


GAME( 2006, cscaoysd,  0,       banpresto,  banpresto,  banpresto_state, empty_init, ROT0, "Banpresto",     "Crayon Shin-chan - Arashi o Yobu! Sushitori Daigassen",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2006, pmsgk,     0,       banpresto,  banpresto,  banpresto_state, empty_init, ROT0, "Banpresto",     "Pocket Monster - Sonans ga Koronda!",                          MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2006, dbzbgck,   0,       banpresto,  banpresto,  banpresto_state, empty_init, ROT0, "Banpresto",     "Dragon Ball Z - Bucchigiri no Chou Kessen (ver. 1.1)",         MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2006, dbzbgck10, dbzbgck, banpresto,  banpresto,  banpresto_state, empty_init, ROT0, "Banpresto",     "Dragon Ball Z - Bucchigiri no Chou Kessen (ver. 1.0)",         MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2007, cmcwa,     0,       banpresto,  banpresto,  banpresto_state, empty_init, ROT0, "Banpresto",     "Chibi Maruko-chan - Wanage de Asobo!!",                        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2007, gkitarou,  0,       banpresto,  banpresto,  banpresto_state, empty_init, ROT0, "Banpresto",     "GeGeGe no Kitarou - Taose! Seiyou Youkai Gundan",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2007, kghmytda,  0,       banpresto,  banpresto,  banpresto_state, empty_init, ROT0, "Banpresto",     "Keroro Gunsou - Hacha Mecha Yakyu Taikai de Arimasu!!",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2008, dchopper,  0,       banpresto,  banpresto,  banpresto_state, empty_init, ROT0, "Banpresto",     "From TV Animation One Piece - Nakama o Tasukero! Dr. Chopper", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2005, zoids,     0,       tomy,       tomy,       tomy_state,      empty_init, ROT0, "Amodio / Tomy", "Zoids",                                                        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // title to be verified
GAME( 2007, youkai,    0,       tomy,       tomy,       tomy_state,      empty_init, ROT0, "Amodio / Tomy", "Youkai VS Uchuujin",                                           MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2007, satoyama,  0,       tomy,       tomy,       tomy_state,      empty_init, ROT0, "Amodio / Tomy", "Satoyama Grand Prix",                                          MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2007, gyakyu,    0,       tomy,       tomy,       tomy_state,      empty_init, ROT0, "Amodio / Tomy", "Ginga Yakyu",                                                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2007, yujogs,    0,       tomy,       tomy,       tomy_state,      empty_init, ROT0, "Amodio / Tomy", "Yujo Gattai Shishigami-o",                                     MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // title to be verified, inferred from some strings related to characters
GAME( 2007, anpanmct,  0,       commutouch, commutouch, anpanman_state,  empty_init, ROT0, "Banpresto",     "Anpanman CommuTouch",                                          MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
