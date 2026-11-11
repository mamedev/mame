// license:BSD-3-Clause
// copyright-holders:Antonio "Willy" Malara

/*
Korg TRITON skeleton driver

CPU: SH2 SH7043 / SH7045
FLASH: 2 * MX29F1610
LCD controller: M66271FP
Floppy disk controller: HD63266
Serial Interface: uPD71051
SCSI controller: MB86604L
Sound: 2 * "TGL96" MB87F1710-PFV-S (proprietary)

This driver runs the embedded firmware application with skeleton devices.

Missing:
- Sound
- Optional SCSI
- Knobs, joystick, ribbon
- Buttons LEDs
- Front panel layout
- Buzzer (it's using the cpu timers, but they're not fully implemented)

Not working:
- Floppy disk controller (it's using te cpu DMA controller, but it's only stubbed yet)
*/


#include "emu.h"
#include "bus/midi/midi.h"
#include "bus/rs232/rs232.h"
#include "cpu/sh/sh7042.h"
#include "imagedev/floppy.h"
#include "machine/eepromser.h"
#include "machine/i8251.h"
#include "machine/intelfsh.h"
#include "machine/upd765.h"
#include "screen.h"

#include <algorithm>
#include <array>

#define LOG_PORTS (1U << 1)
#define LOG_NKS   (1U << 2)
#define LOG_TGL   (1U << 3)
#define LOG_MOSS  (1U << 4)

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

#define LOGNKS(...)   LOGMASKED(LOG_NKS,   __VA_ARGS__)
#define LOGPORTS(...) LOGMASKED(LOG_PORTS, __VA_ARGS__)
#define LOGTGL(...)   LOGMASKED(LOG_TGL,   __VA_ARGS__)
#define LOGMOSS(...)  LOGMASKED(LOG_MOSS,  __VA_ARGS__)

namespace {

// The SCU is the uPD71051 USART between the main CPU and the NKS scan
// controller.  The NKS is an H8/3334 microcontroller that manages the
// buttons and leds on the front panel and communicates with the main
// CPU via the serial interface. We're emulating the NKS at a byte-level
// because we don't have access to its rom.
class triton_nks_scu_device : public i8251_device
{
public:
	triton_nks_scu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void receive(u8 data);

	virtual void write(offs_t offset, u8 data) override;
	virtual u8 read(offs_t offset) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr u16 FIFO_SIZE = 0x100;
	static constexpr unsigned CHARACTER_CLOCKS = 10 * 16;

	void schedule_next();
	TIMER_CALLBACK_MEMBER(deliver_next);

	u8 m_fifo[FIFO_SIZE] = { };
	u8 m_fifo_head = 0;
	u8 m_fifo_tail = 0;
	u16 m_fifo_count = 0;
	bool m_data_pending = false;
	bool m_delivery_scheduled = false;
	emu_timer *m_delivery_timer = nullptr;
};

DEFINE_DEVICE_TYPE_PRIVATE(TRITON_NKS_SCU, triton_nks_scu_device, triton_nks_scu_device, "triton_nks_scu", "Korg Triton NKS SCU");

triton_nks_scu_device::triton_nks_scu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i8251_device(mconfig, TRITON_NKS_SCU, tag, owner, clock)
{
}

void triton_nks_scu_device::receive(u8 data)
{
	// rxrdy_r() is masked while the receiver is disabled, even if its data
	// register is occupied.  Track that register separately so bytes queued
	// before firmware configuration cannot overwrite one another.
	if (!m_data_pending && !m_delivery_scheduled)
	{
		receive_character(data);
		m_data_pending = true;
	}
	else if (m_fifo_count < FIFO_SIZE)
	{
		m_fifo[m_fifo_tail++] = data;
		++m_fifo_count;
	}
	else
	{
		logerror("NKS SCU receive FIFO overflow\n");
	}
}

void triton_nks_scu_device::write(offs_t offset, u8 data)
{
	i8251_device::write(offset, data);

	if (!offset)
	{
		// The NKS controller is modelled at byte level, so outgoing bytes are
		// complete immediately.  Run the transmitter to completion so the
		// i8251 generates the TXRDY transitions used to request the next byte.
		LOGNKS("NKS SCU: transmit %02x\n", data);
		while (!is_transmit_register_empty())
			transmit_clock();

		// The shift register becoming empty is reflected in the status on the
		// following baud clock.  The NKS link uses the x16 asynchronous mode.
		for (unsigned i = 0; i < 16; ++i)
			transmit_clock();
	}
}

u8 triton_nks_scu_device::read(offs_t offset)
{
	const u8 data = i8251_device::read(offset);

	// Reading the data register acknowledges RXRDY.  Present the next
	// NKS byte only after that acknowledgement, as the physical USART
	// has a single receive register.
	if (!offset && !machine().side_effects_disabled())
	{
		m_data_pending = false;
		schedule_next();
	}

	return data;
}

void triton_nks_scu_device::device_start()
{
	i8251_device::device_start();
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_head));
	save_item(NAME(m_fifo_tail));
	save_item(NAME(m_fifo_count));
	save_item(NAME(m_data_pending));
	save_item(NAME(m_delivery_scheduled));

	m_delivery_timer = timer_alloc(FUNC(triton_nks_scu_device::deliver_next), this);
}

void triton_nks_scu_device::device_reset()
{
	i8251_device::device_reset();
	// The link to the NKS scan controller has no flow control; CTS is tied
	// active.  Leaving the i8251 default (inactive) suppresses TXRDY.
	write_cts(0);
	m_fifo_head = 0;
	m_fifo_tail = 0;
	m_fifo_count = 0;
	m_data_pending = false;
	m_delivery_scheduled = false;
	m_delivery_timer->adjust(attotime::never);
}

void triton_nks_scu_device::schedule_next()
{
	if (!m_delivery_scheduled && m_fifo_count)
	{
		// The SCU is configured for a 16x asynchronous clock.  Leave one
		// complete 8-N-1 character time between receive-register loads so
		// RXRDY drops and IRQ1 is acknowledged before its next assertion.
		m_delivery_scheduled = true;
		m_delivery_timer->adjust(attotime::from_ticks(CHARACTER_CLOCKS, clock()));
	}
}

TIMER_CALLBACK_MEMBER(triton_nks_scu_device::deliver_next)
{
	m_delivery_scheduled = false;
	if (!m_data_pending && m_fifo_count)
	{
		const u8 data = m_fifo[m_fifo_head++];
		--m_fifo_count;
		receive_character(data);
		m_data_pending = true;
	}
}


class korgtriton_state : public driver_device
{
public:
	korgtriton_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_flash(*this, "flash%u", 0U)
		, m_ram(*this, "ram")
		, m_lcdcm(*this, "lcdcm")
		, m_screen(*this, "screen")
		, m_scu(*this, "scu")
		, m_eeprom(*this, "eeprom")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
		, m_mdout(*this, "mdout")
		, m_mdin(*this, "mdin")
		, m_serial(*this, "serial")
		, m_x(*this, "X")
		, m_y(*this, "Y")
		, m_buttons(*this, "buttons")
	{ }

	void korgtriton(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(touch_changed);
	DECLARE_INPUT_CHANGED_MEMBER(button_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static inline constexpr unsigned SCREEN_WIDTH = 320;
	static inline constexpr unsigned SCREEN_HEIGHT = 240;

	required_device<sh7043a_device> m_maincpu;
	required_device_array<macronix_29f1610mc_16bit_device, 2> m_flash;
	required_shared_ptr<u32> m_ram;
	required_shared_ptr<u32> m_lcdcm;
	required_device<screen_device> m_screen;
	required_device<triton_nks_scu_device> m_scu;
	required_device<eeprom_serial_93c66_16bit_device> m_eeprom;
	required_device<hd63266f_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_device<midi_port_device> m_mdout;
	required_device<midi_port_device> m_mdin;
	required_device<rs232_port_device> m_serial;

	required_ioport m_x;
	required_ioport m_y;
	required_ioport m_buttons;

	u8 m_lcdcio[SCREEN_WIDTH / 8 * SCREEN_HEIGHT] = { }; // 4 bytes per pixel

	u16 m_tgl[0x1000] = { };

	bool m_pe_hack = false;

	bool m_txrdy = false;
	bool m_rxrdy = false;

	void map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u32 pa_r();
	void pa_w(u32 data);

	u32 pe_r();
	void pe_w(u32 data);

	void scu_txrdy_w(int state);
	void scu_rxrdy_w(int state);
	void update_scu_interrupts();
	void send_touch_packet(u8 header);

	u32 flash_r(offs_t offs);
	void flash_w(offs_t offs, u32 data, u32 mem_mask = ~0U);

	u8 lcdcio_r(offs_t offs);
	void lcdcio_w(offs_t offs, u8 data);

	u8 tgl_r(offs_t offs);
	void tgl_w(offs_t offs, u8 data);

	u8 moss_r(offs_t offs);
	void moss_w(offs_t offs, u8 data);
};

static INPUT_PORTS_START(korgtriton)
	PORT_START("X")
	PORT_BIT(0x00ff, 0, IPT_LIGHTGUN_X) PORT_NAME("Touch X") PORT_MINMAX(0x000, 0x0ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::touch_changed), 0)

	PORT_START("Y")
	PORT_BIT(0x00ff, 0, IPT_LIGHTGUN_Y) PORT_NAME("Touch Y") PORT_MINMAX(0x000, 0x0ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::touch_changed), 0)

	PORT_START("buttons")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Touch Button") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::touch_changed), 1)

	PORT_START("nks_buttons")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Numeric 1")   PORT_CODE(KEYCODE_1_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x44)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Numeric 2")   PORT_CODE(KEYCODE_2_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x4c)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Numeric 3")   PORT_CODE(KEYCODE_3_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x54)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Numeric 4")   PORT_CODE(KEYCODE_4_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x04)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Numeric 5")   PORT_CODE(KEYCODE_5_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x0c)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Numeric 6")   PORT_CODE(KEYCODE_6_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x14)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Numeric 7")   PORT_CODE(KEYCODE_7_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x43)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Numeric 8")   PORT_CODE(KEYCODE_8_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x4b)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Numeric 9")   PORT_CODE(KEYCODE_9_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x53)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Numeric 0")   PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x05)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Numeric -")   PORT_CODE(KEYCODE_MINUS) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x13)

	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Enter")       PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x15)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Exit")        PORT_CODE(KEYCODE_BACKSPACE) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x03)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("10s Hold")    PORT_CODE(KEYCODE_2)         PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x0d)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Menu")        PORT_CODE(KEYCODE_1)         PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x0b)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Value -1")    PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x50)
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Value +1")    PORT_CODE(KEYCODE_PLUS_PAD)  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x51)

	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Combi")       PORT_CODE(KEYCODE_Q) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x12)
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Program")     PORT_CODE(KEYCODE_W) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x42)
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Sequencer")   PORT_CODE(KEYCODE_E) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x11)
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Sampling")    PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x41)
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Song Play")   PORT_CODE(KEYCODE_T) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x49)
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Global")      PORT_CODE(KEYCODE_Y) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x40)
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Disk")        PORT_CODE(KEYCODE_U) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x10)
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Compare")     PORT_CODE(KEYCODE_I) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x48)

	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bank A")      PORT_CODE(KEYCODE_A) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x47)
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bank B")      PORT_CODE(KEYCODE_S) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x4f)
	PORT_BIT(0x08000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bank C")      PORT_CODE(KEYCODE_D) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x06)
	PORT_BIT(0x10000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bank D")      PORT_CODE(KEYCODE_F) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x0e)
	PORT_BIT(0x20000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bank F")      PORT_CODE(KEYCODE_G) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x46)
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bank E")      PORT_CODE(KEYCODE_H) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x16)
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bank G")      PORT_CODE(KEYCODE_J) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x4e)

	PORT_START("nks_buttons_1")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Pause")             PORT_CODE(KEYCODE_Z)  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x07)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Rewind")            PORT_CODE(KEYCODE_X)  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x0f)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Fast Forward")      PORT_CODE(KEYCODE_C)  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0xd7)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Locate")            PORT_CODE(KEYCODE_V)  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x45)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Rec/Write")         PORT_CODE(KEYCODE_B)  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x4d)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Start/Stop")        PORT_CODE(KEYCODE_N)  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x55)

	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SW1")               PORT_CODE(KEYCODE_M)     PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x56)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SW2")               PORT_CODE(KEYCODE_COMMA) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x4a)

	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Realtime Controls") PORT_CODE(KEYCODE_STOP)  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x00)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Arpeggiator")       PORT_CODE(KEYCODE_SLASH) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(korgtriton_state::button_changed), 0x01)


INPUT_PORTS_END

void korgtriton_state::machine_start()
{
	save_item(NAME(m_pe_hack));
	save_item(NAME(m_txrdy));
	save_item(NAME(m_rxrdy));
}

void korgtriton_state::machine_reset()
{
	// Initial NKS response: the handshake acknowledgement, 50 configuration
	// records, and the marker read after the host terminates enumeration.
	static const auto initial_packet_size = 1 + 50 * 2 + 1;
	for (unsigned i = 0; i < initial_packet_size; ++i)
		m_scu->receive(BIT(i, 0) ? 0x31 : 0x66);

	m_pe_hack = false;

	m_txrdy = false;
	m_rxrdy = false;

	std::fill(std::begin(m_tgl), std::end(m_tgl), 0);
	std::fill(std::begin(m_lcdcio), std::end(m_lcdcio), 0);
}

void korgtriton_state::map(address_map &map)
{
	// on chip rom
	map(0x000000, 0x03ffff).rom().region("maincpu", 0);

	// cs0 space (32 bits access)
	// to SAMPLING interface

	// cs1 space (32 bits access)
	map(0x400000, 0x7fffff).rw(FUNC(korgtriton_state::flash_r), FUNC(korgtriton_state::flash_w)); // 2 x MX29F1610

	// cs2 space (16 bits access)
	// map(0x800000, 0x8fffff).ram(); // SPC
	map(0x900000, 0x9fffff).ram().share(m_lcdcm);
	map(0xa00000, 0xafffff).rw(FUNC(korgtriton_state::lcdcio_r), FUNC(korgtriton_state::lcdcio_w));
	map(0xb00000, 0xbfffff).rw(FUNC(korgtriton_state::tgl_r), FUNC(korgtriton_state::tgl_w));
	map(0xd00000, 0xdfffff).rw(FUNC(korgtriton_state::moss_r), FUNC(korgtriton_state::moss_w));
	map(0xe00000, 0xe00003).m(m_fdc, FUNC(hd63266f_device::map));
	map(0xf00000, 0xf00001).rw(m_scu, FUNC(triton_nks_scu_device::read), FUNC(triton_nks_scu_device::write));

	// System DRAM
	map(0x01000000, 0x01ffffff).ram().share(m_ram);
}

void korgtriton_state::korgtriton(machine_config &config)
{
	SH7043A(config, m_maincpu, 7_MHz_XTAL * 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &korgtriton_state::map);

	m_maincpu->read_porta().set(FUNC(korgtriton_state::pa_r));
	m_maincpu->write_porta().set(FUNC(korgtriton_state::pa_w));
	m_maincpu->read_porte().set(FUNC(korgtriton_state::pe_r));
	m_maincpu->write_porte().set(FUNC(korgtriton_state::pe_w));
	m_maincpu->write_sci_tx<0>().set(m_mdout, FUNC(midi_port_device::write_txd));
	m_maincpu->write_sci_tx<1>().set(m_serial, FUNC(rs232_port_device::write_txd));

	MACRONIX_29F1610MC_16BIT(config, m_flash[0]);
	MACRONIX_29F1610MC_16BIT(config, m_flash[1]);

	EEPROM_93C66_16BIT(config, m_eeprom);

	HD63266F(config, m_fdc, 16_MHz_XTAL);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_fdc->inp_rd_callback().set([this]() { return m_floppy->get_device()->dskchg_r(); });

	FLOPPY_CONNECTOR(config, m_floppy, "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats);

	// 320x240 screen, should be a simple framebuffer at LCDCIO
	SCREEN(config, m_screen).set_lcd();
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(SCREEN_WIDTH, SCREEN_HEIGHT);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(korgtriton_state::screen_update));

	TRITON_NKS_SCU(config, m_scu, 1021800);
	m_scu->txrdy_handler().set(FUNC(korgtriton_state::scu_txrdy_w));
	m_scu->rxrdy_handler().set(FUNC(korgtriton_state::scu_rxrdy_w));

	MIDI_PORT(config, m_mdout, midiout_slot, "midiout");

	MIDI_PORT(config, m_mdin, midiin_slot, "midiin");
	m_mdin->rxd_handler().set(m_maincpu, FUNC(sh7042_device::sci_rx_w<0>));

	RS232_PORT(config, m_serial, default_rs232_devices, nullptr);
	m_serial->rxd_handler().set(m_maincpu, FUNC(sh7042_device::sci_rx_w<1>));
}

u32 korgtriton_state::pa_r()
{
	return 0;
}

void korgtriton_state::pa_w(u32 data)
{
	m_eeprom->cs_write(BIT(data, 16));
	m_eeprom->clk_write(BIT(data, 9));

	LOGPORTS("pa_w: %08x\n", data);
}

u32 korgtriton_state::pe_r()
{
	// The TGL bus signals command completion on PE10.  Until the sound CPUs are
	// emulated, acknowledge each observed wait phase with the required edge.
	if (!machine().side_effects_disabled())
		m_pe_hack = !m_pe_hack;

	u32 pe_hack = m_pe_hack ? 0x0400 : 0x0000;
	u32 eeprom_data = m_eeprom->do_read() ? 0x40000 : 0x0000;

	return pe_hack | eeprom_data;
}

void korgtriton_state::pe_w(u32 data)
{
	m_eeprom->di_write(BIT(data, 13));
	m_fdc->tc_w(BIT(data, 11));

	LOGPORTS("pe_w: %08x\n", data);
}

u32 korgtriton_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const int byte_width = SCREEN_WIDTH / 8;

	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		u32 *const dst = &bitmap.pix(y);
		for (int x_byte = 0; x_byte < byte_width; ++x_byte)
		{
			const u8 byte = m_lcdcio[y * byte_width + x_byte];

			for (int bit = 0; bit < 8; ++bit)
			{
				const int x = x_byte * 8 + (7 - bit); // MSB first
				const u32 color = BIT(byte, bit) ? 0xffffffff : 0xff000000;
				dst[x] = color;
			}
		}
	}
	return 0;
}

u8 korgtriton_state::lcdcio_r(offs_t offs)
{
	return (offs < sizeof(m_lcdcio)) ? m_lcdcio[offs] : 0;
}

void korgtriton_state::lcdcio_w(offs_t offs, u8 data)
{
	if (offs < std::size(m_lcdcio))
		m_lcdcio[offs] = data;
}

void korgtriton_state::scu_txrdy_w(int state)
{
	m_txrdy = state;
	update_scu_interrupts();
}

void korgtriton_state::scu_rxrdy_w(int state)
{
	m_rxrdy = state;
	update_scu_interrupts();
}

void korgtriton_state::update_scu_interrupts()
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ1, (m_txrdy || m_rxrdy) ? ASSERT_LINE : CLEAR_LINE);
}

u32 korgtriton_state::flash_r(offs_t offs)
{
	return (u32(m_flash[1]->read(offs)) << 16) | m_flash[0]->read(offs);
}

void korgtriton_state::flash_w(offs_t offs, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_16_31)
		m_flash[1]->write(offs, data >> 16);

	if (ACCESSING_BITS_0_15)
		m_flash[0]->write(offs, data);
}

u8 korgtriton_state::tgl_r(offs_t offs)
{
	int res = 0;

	if (offs < std::size(m_tgl))
		res = m_tgl[offs];

	if (offs == 0)
		res = 4;

	LOGTGL("tgl_read: %08x -> %02x\n", offs, res);
	return res;
}

void korgtriton_state::tgl_w(offs_t offs, u8 data) {
	LOGTGL("tgl_write: %08x %02x\n", offs, data);

	if (offs == 0x609)
		data = 0;

	if (offs == 0xe09)
		data = 0;

	if (offs < std::size(m_tgl))
		m_tgl[offs] = data;
}

u8 korgtriton_state::moss_r(offs_t offs)
{
	if (offs == 0)
		return 0x80;

	return 0xff;
}

void korgtriton_state::moss_w(offs_t offs, u8 data)
{
	LOGMOSS("%s: moss_w %08x -- %02x\n", machine().describe_context(), offs, data);
}

INPUT_CHANGED_MEMBER(korgtriton_state::touch_changed)
{
	const bool pressed = bool(newval);

	if (param)
		send_touch_packet(pressed ? 0x11 : 0x12);
	else
		send_touch_packet(0x10);
}

void korgtriton_state::send_touch_packet(u8 header)
{
	m_scu->receive(header);
	m_scu->receive(m_x->read());
	m_scu->receive(m_y->read());
}

INPUT_CHANGED_MEMBER(korgtriton_state::button_changed)
{
	const bool pressed = bool(newval);
	if (!pressed) return;

	const u8 key = param;
	m_scu->receive(0x30);
	m_scu->receive(key);
}

// The following rom images are reconstructed from the Triton OS floppy disks.

ROM_START( korgtriton )
	ROM_REGION( 0x00800000, "maincpu", 0 )
	ROM_LOAD("int01092.710", 0x00000000, 0x00008000, CRC(135bfd09) SHA1(8e57c6d8460801ebcce22310cecf90eb6a807099))
	ROM_LOAD("int13092.710", 0x00008000, 0x00018000, CRC(d1b8ce69) SHA1(7cff969ed4c663c3aca89014dd776e780b23d81e))

	ROM_REGION16_BE(0x200000, "flash0", ROMREGION_ERASEFF)
	ROM_LOAD("ext092.ic7",  0, 0x200000, CRC(595f633f) SHA1(54f9fc3c58ed26a75480de8a39ad7433fce089db))

	ROM_REGION16_BE(0x200000, "flash1", ROMREGION_ERASEFF)
	ROM_LOAD("ext092.ic15", 0, 0x200000, CRC(538e5a31) SHA1(05dceb0c9083b8bfee7f8b84fafdf758ec3d3988))
ROM_END

ROM_START( korgtritona )
	ROM_REGION( 0x00800000, "maincpu", 0 )
	ROM_LOAD("int01088.710", 0x00000000, 0x00008000, CRC(135bfd09) SHA1(8e57c6d8460801ebcce22310cecf90eb6a807099))
	ROM_LOAD("int13088.710", 0x00008000, 0x00018000, CRC(d1b8ce69) SHA1(7cff969ed4c663c3aca89014dd776e780b23d81e))

	ROM_REGION16_BE(0x200000, "flash0", ROMREGION_ERASEFF)
	ROM_LOAD("ext088.ic7",  0, 0x200000, CRC(bf60c16a) SHA1(d80843e366d9c69fb48bf965117f8f6195f9ec0f))

	ROM_REGION16_BE(0x200000, "flash1", ROMREGION_ERASEFF)
	ROM_LOAD("ext088.ic15", 0, 0x200000, CRC(5c17ae50) SHA1(5c1c09b23a442c2ca386669cd8b33f2f9fbac379))
ROM_END

ROM_START( korgtritonb )
	ROM_REGION( 0x00800000, "maincpu", 0 )
	ROM_LOAD("int01080.710", 0x00000000, 0x00008000, CRC(ef0d7b3a) SHA1(5b20814dbd118df746437a5aa0b9bc371b8bada0))
	ROM_LOAD("int13080.710", 0x00008000, 0x00018000, CRC(295bd0f8) SHA1(5f21cbaa95d9622aa0cfcf8b453d8c98b5dc54cd))

	ROM_REGION16_BE(0x200000, "flash0", ROMREGION_ERASEFF)
	ROM_LOAD("ext080.ic7",  0, 0x200000, CRC(20488469) SHA1(00f7aa95a1ce436794a93347f57cfe960e9793b5))

	ROM_REGION16_BE(0x200000, "flash1", ROMREGION_ERASEFF)
	ROM_LOAD("ext080.ic15", 0, 0x200000, CRC(2421dfcb) SHA1(384ba6fb1b6964308e9f3d4cb42e0e4a622a766e))
ROM_END

} // anonymous namespace

template class device_finder<triton_nks_scu_device, false>;
template class device_finder<triton_nks_scu_device, true>;

SYST(1999, korgtriton,  0,          0, korgtriton, korgtriton, korgtriton_state, empty_init, "Korg", "Triton Music Workstation/Sampler (v2.5.3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST(1999, korgtritona, korgtriton, 0, korgtriton, korgtriton, korgtriton_state, empty_init, "Korg", "Triton Music Workstation/Sampler (v2.5.0)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST(1999, korgtritonb, korgtriton, 0, korgtriton, korgtriton, korgtriton_state, empty_init, "Korg", "Triton Music Workstation/Sampler (v2.0.0)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
