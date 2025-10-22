// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Terak monochrome video board

    To do: native keyboard, beeper

***************************************************************************/

#include "emu.h"
#include "terak_v.h"

#include "machine/keyboard.h"
#include "machine/pdp11.h"
#include "machine/z80daisy.h"
#include "sound/dac.h"

#include "screen.h"

#include "emupal.h"
#include "speaker.h"


#define LOG_DBG     (1U << 1)

// #define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

#define LOGDBG(format, ...)   LOGMASKED(LOG_DBG, "%11.6f at %s: " format, machine().time().as_double(), machine().describe_context(), __VA_ARGS__)


namespace {

static constexpr int TVVIR_WR = 0017417;
static constexpr int TVVCR_WR = 0004677;


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> terak_v_device

class terak_v_device : public device_t,
					public device_qbus_card_interface,
					public device_z80daisy_interface
{
public:
	// construction/destruction
	terak_v_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);

	uint16_t emu_read(offs_t offset);
	void emu_write(offs_t offset, uint16_t data);

	uint16_t text_read(offs_t offset);
	void text_write(offs_t offset, uint16_t data, uint16_t mem_mask);

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override {};

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	bool m_installed;

	required_device<screen_device> m_screen;
	required_device<dac_bit_interface> m_dac;

	TIMER_CALLBACK_MEMBER(wait_tick);

	static constexpr uint16_t EMRCSR_RD      = CSR_DONE | CSR_IE;
	static constexpr uint16_t EMRCSR_WR      = CSR_IE;

	static constexpr uint16_t EMTCSR_LTC     = 0004000;
	static constexpr uint16_t EMTCSR_RD      = CSR_DONE | CSR_IE | EMTCSR_LTC;
	static constexpr uint16_t EMTCSR_WR      = CSR_IE;

private:
	std::unique_ptr<uint16_t[]> m_videoram_base;
	std::unique_ptr<uint8_t[]> m_textram_base, m_chargen_base;
	uint16_t *m_videoram;
	uint8_t *m_textram, *m_chargen;

	line_state m_rxrdy, m_txrdy, m_emrdy;
	int m_rxvec, m_txvec, m_emvec;

	uint16_t m_gar, m_vir, m_vcr;
	uint16_t m_ksr, m_kdr, m_esr, m_edb;

	emu_timer *m_timer_wait;

	void kbd_put(uint8_t data);
};


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

enum
{
	EMRCSR = 0,
	EMRBUF,
	EMTCSR,
	EMTBUF
};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  terak_v_device - constructor
//-------------------------------------------------

terak_v_device::terak_v_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TERAK_V, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, device_z80daisy_interface(mconfig, *this)
	, m_installed(false)
	, m_screen(*this, "screen")
	, m_dac(*this, "dac")
	, m_rxvec(060)
	, m_txvec(064)
	, m_emvec(0164)
{
}


uint32_t terak_v_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 240; y++)
	{
		const int g = BIT(m_vcr, 5 - (y / 80));
		const int t = BIT(m_vcr, 2 - (y / 80));

		for (int x = 0; x < 20; x++)
		{
			uint16_t gfx = g ? m_videoram[((m_gar - 020000) >> 1) + (y * 20) + x] : 0; // FIXME wrap
			uint32_t gft = 0;

			if (t)
			{
				const uint16_t s = y + (m_vir & 15);
				const uint16_t a = ((((s / 10) + (m_vir >> 8)) % 25) << 7) + (x << 2);

				uint8_t code = m_textram[a];
				gft  = m_chargen[(code << 4) + (s % 10)];

				code = m_textram[a + 1];
				gft |= m_chargen[(code << 4) + (s % 10)] << 8;

				code = m_textram[a + 2];
				gft |= m_chargen[(code << 4) + (s % 10)] << 16;

				code = m_textram[a + 3];
				gft |= m_chargen[(code << 4) + (s % 10)] << 24;
			}

			for (int b = 0; b < 32; b++)
			{
				bitmap.pix(y, x * 32 + b) = BIT(gfx, b >> 1) | BIT(gft, b);
			}
		}
	}
	return 0;
}

void terak_v_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_screen_update(FUNC(terak_v_device::screen_update));
	m_screen->set_palette("palette");

	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(640, 240);
	m_screen->set_visarea(0, 640 - 1, 0, 240 - 1);
	m_screen->screen_vblank().set([this](int state) { m_bus->bevnt_w(state); });

	PALETTE(config, "palette", palette_device::MONOCHROME);

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(terak_v_device::kbd_put));

	// built-in piezo
	SPEAKER(config, "mono").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.25);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void terak_v_device::device_start()
{
	// save state
	save_item(NAME(m_installed));
	save_item(NAME(m_gar));
	save_item(NAME(m_vir));
	save_item(NAME(m_vcr));
	save_item(NAME(m_edb));

	m_timer_wait = timer_alloc(FUNC(terak_v_device::wait_tick), this);

	m_videoram_base = std::make_unique<uint16_t[]>(49152 / 2);
	m_videoram = m_videoram_base.get();

	m_textram_base = std::make_unique<uint8_t[]>(4096);
	m_textram = m_textram_base.get();

	m_chargen_base = std::make_unique<uint8_t[]>(4096);
	m_chargen = m_chargen_base.get();

	m_installed = false;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void terak_v_device::device_reset()
{
	if (!m_installed)
	{
		m_bus->program_space().install_ram(0020000, 0157777, m_videoram);
		m_bus->program_space().install_readwrite_handler(0160000, 0167777, emu::rw_delegate(*this, FUNC(terak_v_device::text_read)),
			emu::rw_delegate(*this, FUNC(terak_v_device::text_write)));
		m_bus->install_device(0177560, 0177567, read16sm_delegate(*this, FUNC(terak_v_device::emu_read)),
			write16sm_delegate(*this, FUNC(terak_v_device::emu_write)));
		m_bus->install_device(0177740, 0177747, read16sm_delegate(*this, FUNC(terak_v_device::read)),
			write16sm_delegate(*this, FUNC(terak_v_device::write)));
		m_gar = m_vir = m_vcr = m_edb = 0;

		m_installed = true;
	}
	else
	{
		m_ksr &= ~(CSR_IE | CSR_DONE);
		m_esr &= ~CSR_IE;
		m_esr |= CSR_DONE;
		clear_virq(m_bus->birq4_w, 1, 1, m_rxrdy);
		clear_virq(m_bus->birq4_w, 1, 1, m_txrdy);
		clear_virq(m_bus->birq4_w, 1, 1, m_emrdy);
	}
}

int terak_v_device::z80daisy_irq_state()
{
	if (m_rxrdy == ASSERT_LINE || m_txrdy == ASSERT_LINE || m_emrdy == ASSERT_LINE)
		return Z80_DAISY_INT;
	else
		return 0;
}

int terak_v_device::z80daisy_irq_ack()
{
	int vec = -1;

	if (m_rxrdy == ASSERT_LINE)
	{
		m_rxrdy = CLEAR_LINE;
		vec = m_rxvec;
	}
	else if (m_emrdy == ASSERT_LINE)
	{
		m_emrdy = CLEAR_LINE;
		vec = m_emvec;
	}
	else if (m_txrdy == ASSERT_LINE)
	{
		m_txrdy = CLEAR_LINE;
		vec = m_txvec;
	}

	return vec;
}

TIMER_CALLBACK_MEMBER(terak_v_device::wait_tick)
{
	raise_virq(m_bus->birq4_w, m_esr, CSR_IE, m_txrdy);
}

void terak_v_device::kbd_put(uint8_t data)
{
	m_kdr = data;
	m_ksr |= CSR_DONE;
	raise_virq(m_bus->birq4_w, m_ksr, CSR_IE, m_rxrdy);
}


uint16_t terak_v_device::read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_gar;
		break;

	case 1:
		data = m_vir;
		break;

	case 2:
		data = m_vcr;
		break;

	case 3:
		data = m_edb;
		break;
	}

	return data;
}

void terak_v_device::write(offs_t offset, uint16_t data)
{
	LOGDBG("W %06o <- %06o\n", 0177740 + (offset << 1), data);

	switch (offset)
	{
	case 0:
		m_gar = data & ~1;
		break;

	case 1:
		m_vir = ((m_vir & ~TVVIR_WR) | (data & TVVIR_WR));
		break;

	case 2:
		m_vcr = ((m_vcr & ~TVVCR_WR) | (data & TVVCR_WR));
		m_dac->write(BIT(data, 8));
		break;

	case 3:
		m_edb = data;
		break;
	}
}

uint16_t terak_v_device::emu_read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset)
	{
	case EMRCSR:
		data = m_ksr & EMRCSR_RD;
		break;

	case EMRBUF:
		data = m_kdr;
		if (!machine().side_effects_disabled())
		{
			m_ksr &= ~CSR_DONE;
			clear_virq(m_bus->birq4_w, m_ksr, CSR_IE, m_rxrdy);
		}
		break;

	case EMTCSR:
		data = (m_esr & EMTCSR_RD) | EMTCSR_LTC;
		break;

	case EMTBUF:
		data = m_edb;
		if (!machine().side_effects_disabled())
		{
			m_esr |= CSR_DONE;
			m_timer_wait->adjust(attotime::from_usec(480 + m_screen->frame_number() % 40)); // FIXME
		}
		break;
	}

	return data;
}

void terak_v_device::emu_write(offs_t offset, uint16_t data)
{
	LOG("W %06o <- %06o '%c' emu\n", 0177560 + (offset << 1), data, isprint(data) ? data : ' ');

	switch (offset)
	{
	case EMRCSR:
		if ((data & CSR_IE) == 0)
		{
			clear_virq(m_bus->birq4_w, 1, 1, m_rxrdy);
		}
		else if ((m_ksr & (CSR_DONE + CSR_IE)) == CSR_DONE)
		{
			raise_virq(m_bus->birq4_w, 1, 1, m_rxrdy);
		}
		m_ksr = ((m_ksr & ~EMRCSR_WR) | (data & EMRCSR_WR));
		break;

	case EMTCSR:
		if ((data & CSR_IE) == 0)
		{
			clear_virq(m_bus->birq4_w, 1, 1, m_txrdy);
		}
		else if ((m_esr & (CSR_DONE + CSR_IE)) == CSR_DONE)
		{
			raise_virq(m_bus->birq4_w, 1, 1, m_txrdy);
		}
		m_esr = ((m_esr & ~EMTCSR_WR) | (data & EMTCSR_WR));
		break;

	case EMTBUF:
		m_edb = data;
		m_esr &= ~CSR_DONE;
		clear_virq(m_bus->birq4_w, m_esr, CSR_IE, m_txrdy);
		raise_virq(m_bus->birq4_w, 1, 1, m_emrdy);
		break;
	}
}

uint16_t terak_v_device::text_read(offs_t offset)
{
	uint8_t *p = BIT(m_vcr, 7) ? m_chargen : m_textram;
	return p[offset + offset] | (p[offset + offset + 1] << 8);
}

void terak_v_device::text_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint8_t *p = BIT(m_vcr, 7) ? m_chargen : m_textram;
	if (ACCESSING_BITS_0_7)
	{
		p[offset + offset] = data;
	}
	else
	{
		p[offset + offset + 1] = data >> 8;
	}
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(TERAK_V, device_qbus_card_interface, terak_v_device, "terak_v", "Terak monochrome video")
