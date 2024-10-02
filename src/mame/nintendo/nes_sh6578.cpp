// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    SH6578 NES clone hardware
    enhanced NES, different to VT / OneBus systems

    "UMC 1997.2 A35551S" on CPU die (maxx6in1)

    video rendering is changed significantly compared to NES so not using NES PPU device
    has 256x256 pixel pages, attributes are stored next to tile numbers (not in their own table after them) etc.

*/

#include "emu.h"
#include "video/ppu2c0x_sh6578.h"
#include "cpu/m6502/m6502.h"
#include "sound/nes_apu.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "machine/bankdev.h"
#include "machine/timer.h"

#define LOG_DMA       (1U << 1)
#define LOG_PPU       (1U << 2)

//#define VERBOSE             (LOG_PPU)
#define VERBOSE             (0)

#include "logmacro.h"


namespace {

class nes_sh6578_state : public driver_device
{
public:
	nes_sh6578_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_bank(*this, "cartbank"),
		m_maincpu(*this, "maincpu"),
		m_ppu(*this, "ppu"),
		m_fullrom(*this, "fullrom"),
		m_screen(*this, "screen"),
		m_apu(*this, "nesapu"),
		m_timer(*this, "timer"),
		m_in(*this, "IN%u", 0U)
	{ }

	void nes_sh6578(machine_config& config);
	void nes_sh6578_pal(machine_config& config);

	void init_nes_sh6578();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	virtual void io_w(uint8_t data);
	virtual void extio_w(uint8_t data);
	bool m_isbanked;
	required_memory_bank m_bank;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ppu_sh6578_device> m_ppu;
	required_device<address_map_bank_device> m_fullrom;
	required_device<screen_device> m_screen;
	required_device<nesapu_device> m_apu;
	required_device<timer_device> m_timer;

	uint8_t bankswitch_r(offs_t offset);
	void bankswitch_w(offs_t offset, uint8_t data);

	uint8_t dma_r(offs_t offset);
	void dma_w(offs_t offset, uint8_t data);

	uint8_t bank_r(int bank, uint16_t offset);
	void bank_w(int bank, uint16_t offset, uint8_t data);

	uint8_t bank0_r(offs_t offset) { return bank_r(0, offset); }
	void bank0_w(offs_t offset, uint8_t data) { bank_w(0, offset, data); }
	uint8_t bank1_r(offs_t offset) { return bank_r(1, offset); }
	void bank1_w(offs_t offset, uint8_t data) { bank_w(1, offset, data); }
	uint8_t bank2_r(offs_t offset) { return bank_r(2, offset); }
	void bank2_w(offs_t offset, uint8_t data) { bank_w(2, offset, data); }
	uint8_t bank3_r(offs_t offset) { return bank_r(3, offset); }
	void bank3_w(offs_t offset, uint8_t data) { bank_w(3, offset, data); }
	uint8_t bank4_r(offs_t offset) { return bank_r(4, offset); }
	void bank4_w(offs_t offset, uint8_t data) { bank_w(4, offset, data); }
	uint8_t bank5_r(offs_t offset) { return bank_r(5, offset); }
	void bank5_w(offs_t offset, uint8_t data) { bank_w(5, offset, data); }
	uint8_t bank6_r(offs_t offset) { return bank_r(6, offset); }
	void bank6_w(offs_t offset, uint8_t data) { bank_w(6, offset, data); }
	uint8_t bank7_r(offs_t offset) { return bank_r(7, offset); }
	void bank7_w(offs_t offset, uint8_t data) { bank_w(7, offset, data); }

	void timing_setting_control_w(uint8_t data);
	void initial_startup_w(uint8_t data);
	uint8_t irq_status_r();
	void irq_mask_w(uint8_t data);
	void timer_config_w(uint8_t data);
	void timer_value_w(uint8_t data);

	uint8_t io0_r();
	uint8_t io1_r();

	uint8_t apu_read_mem(offs_t offset);

	void apu_irq(int state);

	int m_initial_startup_state;

	uint8_t m_bankswitch[8];

	uint8_t m_dma_control;
	uint8_t m_dma_bank;
	uint8_t m_dma_source[2];
	uint8_t m_dma_dest[2];
	uint8_t m_dma_length[2];

	uint8_t m_irqmask;

	void do_dma();

	void rom_map(address_map &map) ATTR_COLD;
	void nes_sh6578_map(address_map &map) ATTR_COLD;

	//uint16_t get_tileaddress(uint8_t x, uint8_t y, bool ishigh);

	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(timer_expired);
	int m_timerval;

	// this might be game specific
	uint8_t m_previo;
	uint8_t m_iolatch[2];
	required_ioport_array<2> m_in;
};

class nes_sh6578_abl_wikid_state : public nes_sh6578_state
{
public:
	nes_sh6578_abl_wikid_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_sh6578_state(mconfig, type, tag)
	{ }

protected:
	virtual void io_w(uint8_t data) override;
};

class nes_sh6578_max10in1_state : public nes_sh6578_state
{
public:
	nes_sh6578_max10in1_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_sh6578_state(mconfig, type, tag)
	{ }

protected:
	virtual void extio_w(uint8_t data) override;
	virtual void machine_reset() override ATTR_COLD;
};

uint8_t nes_sh6578_state::bank_r(int bank, uint16_t offset)
{
	uint32_t address;
	address = offset & 0x00fff;                   // 0x00fff part of address
	address |= (m_bankswitch[bank] & 0xff) << 12; // 0xff000 part of address
	return m_fullrom->read8(address);
}

void nes_sh6578_state::bank_w(int bank, uint16_t offset, uint8_t data)
{
	uint32_t address;
	address = offset & 0x00fff;                   // 0x00fff part of address
	address |= (m_bankswitch[bank] & 0xff) << 12; // 0xff000 part of address
	m_fullrom->write8(address, data);
}

uint8_t nes_sh6578_state::bankswitch_r(offs_t offset)
{
	return m_bankswitch[offset];
}

void nes_sh6578_state::bankswitch_w(offs_t offset, uint8_t data)
{
	m_bankswitch[offset] = data;
}

uint8_t nes_sh6578_state::dma_r(offs_t offset)
{
	switch (offset)
	{
	case 0x00: LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_r offset %01x : DMA Control : %02x\n", machine().describe_context(), offset, m_dma_control); return m_dma_control & 0x7f;
	case 0x01: LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_r offset %01x : DMA Bank Select\n", machine().describe_context(), offset); return m_dma_bank;
	case 0x02: LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_r offset %01x : DMA Source Address Low\n", machine().describe_context(), offset); return m_dma_source[0];
	case 0x03: LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_r offset %01x : DMA Source Address High\n", machine().describe_context(), offset); return m_dma_source[1];
	case 0x04: LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_r offset %01x : DMA Destination Address Low\n", machine().describe_context(), offset); return m_dma_dest[0];
	case 0x05: LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_r offset %01x : DMA Destination Address High\n", machine().describe_context(), offset); return m_dma_dest[1];
	case 0x06: LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_r offset %01x : DMA Length Low\n", machine().describe_context(), offset); return m_dma_length[0];
	case 0x07: LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_r offset %01x : DMA Length High\n", machine().describe_context(), offset); return m_dma_length[1];
	}
	return 0x00;
}

void nes_sh6578_state::do_dma()
{

	if (m_dma_control & 0x80)
	{
		uint16_t dma_source = m_dma_source[0] | (m_dma_source[1] << 8);
		uint16_t dma_dest = m_dma_dest[0] | (m_dma_dest[1] << 8);
		uint16_t dma_length = m_dma_length[0] | (m_dma_length[1] << 8);

		LOGMASKED(LOG_DMA, "Doing DMA :%02x bank:%02x: source:%04x dest:%04x length:%04x\n", m_dma_control, m_dma_bank, dma_source, dma_dest, dma_length);

		uint16_t realsourceaddress = dma_source;
		uint16_t realdestaddress = dma_dest;

		for (int i = 0; i <= dma_length; i++)
		{
			uint8_t readdat = 0x00;
			if (realsourceaddress & 0x8000)
			{
				// reading from ROM?
				uint32_t trueaddress = (realsourceaddress & 0x7fff) | ((m_dma_bank & 0x1f) * 0x8000);

				readdat = m_fullrom->read8(trueaddress);
			}
			else
			{
				//logerror("reading from system area %04x\n", realsourceaddress);
				uint32_t trueaddress = (realsourceaddress & 0x7fff);
				readdat = m_maincpu->space(AS_PROGRAM).read_byte(trueaddress);
			}

			if (m_dma_control & 0x20)
			{
				//logerror("writing to WORK RAM %04x %02x\n", realdestaddress, readdat);
				m_maincpu->space(AS_PROGRAM).write_byte(realdestaddress, readdat);
			}
			else
			{
				m_ppu->space(AS_PROGRAM).write_byte(realdestaddress, readdat);
			}

			realsourceaddress++;
			realdestaddress++;
		}
	}

	// but games seem to be making quite a few DMA writes with lengths that seem too large? buggy code?
	//m_dma_length[0] = 0;
	//m_dma_length[1] = 0;
}

void nes_sh6578_state::dma_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x0:
		LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_w offset %01x : DMA Control : %02x\n", machine().describe_context(), offset, data);
		m_dma_control = data;
		do_dma();
		break;

	case 0x1:
		LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_w offset %01x : DMA Bank Select : %02x\n", machine().describe_context(), offset, data);
		m_dma_bank = data;
		break;

	case 0x2:
		LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_w offset %01x : DMA Source Address Low : %02x\n", machine().describe_context(), offset, data);
		m_dma_source[0] = data;
		break;

	case 0x3:
		LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_w offset %01x : DMA Source Address High : %02x\n", machine().describe_context(), offset, data);
		m_dma_source[1] = data;
		break;

	case 0x4:
		LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_w offset %01x : DMA Destination Address Low : %02x\n", machine().describe_context(), offset, data);
		m_dma_dest[0] = data;
		break;

	case 0x5:
		LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_w offset %01x : DMA Destination Address High : %02x\n", machine().describe_context(), offset, data);
		m_dma_dest[1] = data;
		break;

	case 0x6:
		LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_w offset %01x : DMA Length Low : %02x\n", machine().describe_context(), offset, data);
		m_dma_length[0] = data;
		break;

	case 0x7:
		LOGMASKED(LOG_DMA, "%s: nes_sh6578_state::dma_w offset %01x : DMA Length High : %02x\n", machine().describe_context(), offset, data);
		m_dma_length[1] = data;
		break;
	}
}


void nes_sh6578_state::initial_startup_w(uint8_t data)
{
	// there is also a timeframe in which this must happen
	// if the writes are not correct the system does not operate
	if (m_initial_startup_state == 0)
	{
		if (data == 0x65)
		{
			logerror("initial_startup_w VALID first write (0x65)\n");
			m_initial_startup_state = 1;
		}
		else
		{
			logerror("initial_startup_w invalid first write (not 0x65)\n");
			m_initial_startup_state = -1;
		}
	}
	else if (m_initial_startup_state == 1)
	{
		if (data == 0x76)
		{
			logerror("initial_startup_w VALID second write (0x76)\n");
			m_initial_startup_state = 2;
		}
		else
		{
			logerror("initial_startup_w invalid second write (not 0x76)\n");
			m_initial_startup_state = -1;
		}
	}
	else if (m_initial_startup_state == 2)
	{
		logerror("initial_startup_w invalid write (already passed) (%02x)\n", data);
	}
	else if (m_initial_startup_state == -1)
	{
		logerror("initial_startup_w invalid write (already failed) (%02x)\n", data);
	}
}

uint8_t nes_sh6578_state::irq_status_r()
{
	logerror("%s: nes_sh6578_state::irq_status_r\n", machine().describe_context());
	return machine().rand();
}

void nes_sh6578_state::irq_mask_w(uint8_t data)
{
	m_irqmask = data;

	if (m_irqmask & 0x80)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void nes_sh6578_state::timer_config_w(uint8_t data)
{
	logerror("%s: nes_sh6578_state::timer_config_w : %02x (at pos y: %d x: %d )\n", machine().describe_context(), data, m_screen->vpos(), m_screen->hpos() );

	if ((data & 0x80) && (data & 0x20))
	{
		m_timer->adjust(m_screen->scan_period() * m_timerval);
	}
	else
	{
		m_timer->adjust(attotime::never);
	}
}

void nes_sh6578_state::timer_value_w(uint8_t data)
{
	logerror("%s: nes_sh6578_state::timer_value_w : %02x\n", machine().describe_context(), data);
	m_timerval = data;
}


void nes_sh6578_state::timing_setting_control_w(uint8_t data)
{
	logerror("%s: nes_sh6578_state::timing_setting_control_w : %02x\n", machine().describe_context(), data);
}


uint8_t nes_sh6578_state::io0_r()
{
	uint8_t ret = m_iolatch[0] & 0x01;
	m_iolatch[0] >>= 1;
	return ret;
}

uint8_t nes_sh6578_state::io1_r()
{
	uint8_t ret = m_iolatch[1] & 0x01;
	m_iolatch[1] >>= 1;
	return ret;
}

void nes_sh6578_state::io_w(uint8_t data)
{
	if ((data != 0x00) && (data != 0x01) && (data != 0x02) && (data != 0x03))
		logerror("%s: io_w : unexpected value : %02x\n", machine().describe_context(), data);

	if ((m_previo & 0x01) != (data & 0x01))
	{
		// latch on rising or falling?
		if (!(data & 0x01))
		{
			m_iolatch[0] = m_in[0]->read();
			m_iolatch[1] = m_in[1]->read();
		}
	}

	m_previo = data;
}

void nes_sh6578_abl_wikid_state::io_w(uint8_t data)
{
	nes_sh6578_state::io_w(data);

	if (m_isbanked)
	{
		m_bank->set_entry((data>>1)&1);
	}
}

void nes_sh6578_state::extio_w(uint8_t data)
{
	logerror("%s: extio_w : %02x\n", machine().describe_context(), data);
}

void nes_sh6578_max10in1_state::extio_w(uint8_t data)
{
	logerror("%s: extio_w : %02x (max10in1)\n", machine().describe_context(), data);

	m_bank->set_entry((data & 0x80) >> 7);
}





void nes_sh6578_state::apu_irq(int state)
{
	// unimplemented
}

uint8_t nes_sh6578_state::apu_read_mem(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}



void nes_sh6578_state::nes_sh6578_map(address_map& map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2007).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));
	map(0x2008, 0x2008).rw(m_ppu, FUNC(ppu_sh6578_device::read_extended), FUNC(ppu_sh6578_device::write_extended));

	map(0x2040, 0x207f).rw(m_ppu, FUNC(ppu_sh6578_device::palette_read), FUNC(ppu_sh6578_device::palette_write));

	map(0x4000, 0x4017).w(m_apu, FUNC(nesapu_device::write));
	map(0x4014, 0x4014).w(m_ppu, FUNC(ppu_sh6578_device::spriteram_dma));
	map(0x4015, 0x4015).r(m_apu, FUNC(nesapu_device::status_r));
	map(0x4016, 0x4016).rw(FUNC(nes_sh6578_state::io0_r), FUNC(nes_sh6578_state::io_w));
	map(0x4017, 0x4017).r(FUNC(nes_sh6578_state::io1_r));

	map(0x4020, 0x4020).w(FUNC(nes_sh6578_state::timing_setting_control_w));
	//4021 write keyboard output port
	//4022 read/write keyboard data control
	//4023 read/write joystick,mouse control
	//4024 read - mouse port / write - mouse baud
	//4025 write - Printer Port
	map(0x4026, 0x4026).w(FUNC(nes_sh6578_state::extio_w));
	//4027 read/write - DAC data register

	map(0x4031, 0x4031).w(FUNC(nes_sh6578_state::initial_startup_w));
	map(0x4032, 0x4032).w(FUNC(nes_sh6578_state::irq_mask_w));
	map(0x4033, 0x4033).r(FUNC(nes_sh6578_state::irq_status_r));
	map(0x4034, 0x4034).w(FUNC(nes_sh6578_state::timer_config_w));
	map(0x4035, 0x4035).w(FUNC(nes_sh6578_state::timer_value_w));

	map(0x4040, 0x4047).rw(FUNC(nes_sh6578_state::bankswitch_r), FUNC(nes_sh6578_state::bankswitch_w));

	map(0x4048, 0x404f).rw(FUNC(nes_sh6578_state::dma_r), FUNC(nes_sh6578_state::dma_w));

	map(0x5000, 0x57ff).ram();

	map(0x5800, 0x7fff).ram(); // cpatrolm seems to expect RAM here too?

	map(0x8000, 0x8fff).rw(FUNC(nes_sh6578_state::bank0_r), FUNC(nes_sh6578_state::bank0_w));
	map(0x9000, 0x9fff).rw(FUNC(nes_sh6578_state::bank1_r), FUNC(nes_sh6578_state::bank1_w));
	map(0xa000, 0xafff).rw(FUNC(nes_sh6578_state::bank2_r), FUNC(nes_sh6578_state::bank2_w));
	map(0xb000, 0xbfff).rw(FUNC(nes_sh6578_state::bank3_r), FUNC(nes_sh6578_state::bank3_w));
	map(0xc000, 0xcfff).rw(FUNC(nes_sh6578_state::bank4_r), FUNC(nes_sh6578_state::bank4_w));
	map(0xd000, 0xdfff).rw(FUNC(nes_sh6578_state::bank5_r), FUNC(nes_sh6578_state::bank5_w));
	map(0xe000, 0xefff).rw(FUNC(nes_sh6578_state::bank6_r), FUNC(nes_sh6578_state::bank6_w));
	map(0xf000, 0xffff).rw(FUNC(nes_sh6578_state::bank7_r), FUNC(nes_sh6578_state::bank7_w));
}

static INPUT_PORTS_START(nes_sh6578)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void nes_sh6578_state::video_start()
{
}

void nes_sh6578_state::machine_reset()
{
	for (int i = 0; i < 8; i++)
		m_bankswitch[i] = i;

	m_initial_startup_state = 0;
	m_bank->set_entry(0);

	m_irqmask = 0xff;
	m_timerval = 0x00;
}

void nes_sh6578_max10in1_state::machine_reset()
{
	nes_sh6578_state::machine_reset();
	m_bank->set_entry(1);
}

void nes_sh6578_state::machine_start()
{
	m_bank->configure_entry(0, memregion("maincpu")->base() + 0x0000000);
	m_bank->set_entry(0);

	if (memregion("maincpu")->bytes() == 0x200000)
	{
		m_isbanked = true;
		m_bank->configure_entry(1, memregion("maincpu")->base() + 0x100000);
	}
	else
	{
		m_isbanked = false;
	}
}

// SH6578 can address 20-bit address space (1MB of ROM)
void nes_sh6578_state::rom_map(address_map& map)
{
	map(0x00000, 0xfffff).bankr("cartbank");
}

TIMER_DEVICE_CALLBACK_MEMBER(nes_sh6578_state::timer_expired)
{
	if (!(m_irqmask & 0x80))
	{
		//printf("timer expired on line %d\n", m_screen->vpos());
		m_maincpu->set_input_line(0, ASSERT_LINE);
	}

	m_timer->adjust(attotime::never);
}


// from rp2a03.h verify that it actually uses these
#define RP2A03_NTSC_XTAL           XTAL(21'477'272)
#define RP2A03_PAL_XTAL            XTAL(26'601'712)
#define NTSC_APU_CLOCK      (RP2A03_NTSC_XTAL/12) /* 1.7897726666... MHz */
#define PAL_APU_CLOCK       (RP2A03_PAL_XTAL/16) /* 1.662607 MHz */
#define PALC_APU_CLOCK      (RP2A03_PAL_XTAL/15) /* 1.77344746666... MHz */

uint32_t nes_sh6578_state::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	return m_ppu->screen_update(screen, bitmap, cliprect);
}

void nes_sh6578_state::nes_sh6578(machine_config& config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, NTSC_APU_CLOCK); // regular M6502 core, not RP2A03?
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_sh6578_state::nes_sh6578_map);

	ADDRESS_MAP_BANK(config, m_fullrom).set_map(&nes_sh6578_state::rom_map).set_options(ENDIANNESS_NATIVE, 8, 20, 0x100000);

	PPU_SH6578(config, m_ppu, RP2A03_NTSC_XTAL);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.0988);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66/(NTSC_APU_CLOCK.dvalue()/1000000)) *
							 (ppu2c0x_device::VBLANK_LAST_SCANLINE_NTSC-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 262);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(nes_sh6578_state::screen_update));

	TIMER(config, m_timer).configure_generic(FUNC(nes_sh6578_state::timer_expired));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	// have to add the APU separately due to using M6502
	NES_APU(config, m_apu, NTSC_APU_CLOCK);
	m_apu->irq().set(FUNC(nes_sh6578_state::apu_irq));
	m_apu->mem_read().set(FUNC(nes_sh6578_state::apu_read_mem));
	m_apu->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void nes_sh6578_state::nes_sh6578_pal(machine_config& config)
{
	nes_sh6578(config);

	m_maincpu->set_clock(PALC_APU_CLOCK);
	m_apu->set_clock(PALC_APU_CLOCK);

	PPU_SH6578PAL(config.replace(), m_ppu, RP2A03_PAL_XTAL);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	m_screen->set_refresh_hz(50.0070);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66 / (PALC_APU_CLOCK.dvalue() / 1000000)) *
		(ppu2c0x_device::VBLANK_LAST_SCANLINE_PAL - ppu2c0x_device::VBLANK_FIRST_SCANLINE_PALC + 1 + 2)));
	m_screen->set_size(32 * 8, 312);
	m_screen->set_visarea(0 * 8, 32 * 8 - 1, 0 * 8, 30 * 8 - 1);

}

void nes_sh6578_state::init_nes_sh6578()
{
}



ROM_START( bandgpad )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "gamepad.bin", 0x00000, 0x100000, CRC(e2fbb532) SHA1(e9170a7739a8355acbf263fe2b1d291951dc07f0) )
ROM_END

ROM_START( bandggcn )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "gogoconniechan.bin", 0x00000, 0x100000, CRC(715d66ae) SHA1(9326c227bad86eea85194a90f746c60dc032a323) )
ROM_END



ROM_START( ts_handy11 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tvplaypowercontroller.bin", 0x00000, 0x100000, CRC(9c7fe9ff) SHA1(c872e91ca835b66c9dd3b380e8374b51f12bcae0) ) // 29LV008B
ROM_END

ROM_START( cpatrolm )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "citypatrolman.bin", 0x00000, 0x100000, CRC(4b139c67) SHA1(a5b03f472a94ee879f58bbff201b671fbf4f1ea1) )
ROM_END

ROM_START( ablwikid )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "mx29f1610atc.u2", 0x00000, 0x200000, CRC(f16abf79) SHA1(aeccbb40d7fdd451ba8e5cca20464da2cf116461) )
ROM_END

ROM_START( maxx5in1 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vsmaxxcasino5_e28f008sa_89a2.bin", 0x00000, 0x100000, CRC(e3d8f24f) SHA1(121411e72d53eabe6be927d1db2f871d59a9e08e) )
ROM_END

ROM_START( maxx6in1 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "maxx6in1.bin", 0x00000, 0x100000, CRC(8e582298) SHA1(89892b9095dbd5101cdf2477a66abd2cb11ad8c8) )
ROM_END

ROM_START( max10in1 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "csmaxxcasino10.bin", 0x000000, 0x200000, CRC(2a05e9af) SHA1(fcf591c22ce8773f72e9d0fa0bae545f6a82a063) )
ROM_END

ROM_START( vsmaxx15 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "vsmaxx15n1_e28f008sa_89a2.bin", 0x00000, 0x100000, CRC(713955ce) SHA1(f5ff02055fd4574cedc2f056d19388207a7244c0) )
ROM_END

ROM_START( vsmaxx25 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "vsmaxx25_am29lv160dt_000122c4.bin", 0x00000, 0x200000, CRC(0efd1625) SHA1(34e83f748af3eee475c5b2b24ff03c00c1b5b8ed) )
ROM_END

ROM_START( dgun806 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "dgpnpdgu806as29lv160be_000422c4.bin", 0x00000, 0x200000, CRC(576d6caf) SHA1(fdfa4712e6ed66d2af41ccfbfbf870cd01f7b0f7) )
ROM_END



ROM_START( dancmix3 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "e28f008sa.u5", 0x00000, 0x100000, CRC(faf6480c) SHA1(68bf79910e091443aecc7bf256cd5378a04c550e) )
ROM_END

} // anonymous namespace


CONS( 200?, maxx5in1,  0, 0,  nes_sh6578, nes_sh6578, nes_sh6578_state,  init_nes_sh6578, "Senario / JungleTac", "Vs Maxx 5-in-1 Casino / Senario Card & Casino Games", 0 ) // advertised on box as 'With Solitaire" (was there an even older version without it?)

CONS( 200?, maxx6in1,  0, 0,  nes_sh6578, nes_sh6578, nes_sh6578_state,  init_nes_sh6578, "Senario / JungleTac", "Vs Maxx 6-in-1 Casino / Senario Card & Casino Games", 0 ) // advertised on box as "With Texas Hold 'Em" (which is the added game since the 5-in-1)

CONS( 200?, max10in1,  0, 0,  nes_sh6578, nes_sh6578, nes_sh6578_max10in1_state,  init_nes_sh6578, "Senario / JungleTac", "Vs Maxx 10-in-1 Casino / Senario Card & Casino Games", 0 )

// titles below have various issues (DMA, split interrupt timing etc.)

// aka Wik!d Joystick, sometimes also called Air Blaster like the real Air Blaster game.  Wik!d seems a rebranding (a 'gift' company) so did ABL reuse the Air Blaster name here instead?
CONS( 200?, ablwikid,    0,  0,  nes_sh6578_pal, nes_sh6578, nes_sh6578_abl_wikid_state, init_nes_sh6578, "Advance Bright Ltd. / JungleTac", "Wikid Joystick 14-in-1", MACHINE_NOT_WORKING )

CONS( 2001?, ts_handy11,  0,  0,  nes_sh6578,     nes_sh6578, nes_sh6578_state, init_nes_sh6578, "Techno Source / JungleTac", "Handy Boy 11-in-1 (TV Play Power)", MACHINE_NOT_WORKING ) // possibly newer than 2001

// from a blue coloured unit, a yellow one exists, is it the same?
CONS( 2004?, vsmaxx15,    0,  0,  nes_sh6578, nes_sh6578, nes_sh6578_state, init_nes_sh6578, "Senario / JungleTac", "Vs Maxx 15-in-1", MACHINE_NOT_WORKING )

// This is from the blue coloured unit with the 1p/2p slider (does it do anything / get read anywhere?)
// A version of the 25-in-1 on VT hardware also exists, with the downgraded version of Big Racing & removed copyrights etc. (probably the purple tinted version without the 1p/2p slider)
CONS( 2004?, vsmaxx25,    0,  0,  nes_sh6578, nes_sh6578, nes_sh6578_max10in1_state, init_nes_sh6578, "Senario / JungleTac", "Vs Maxx 25-in-1 (SH6578 hardware)", MACHINE_NOT_WORKING )

// DGUN-806 on sticker on battery compartment. DreamGear had some other products with the same pad type but different DGUN numbers on the packaging, is the ROM the same?
CONS( 2004?, dgun806,    0,  0,  nes_sh6578, nes_sh6578, nes_sh6578_max10in1_state, init_nes_sh6578, "dreamGEAR", "Plug 'N' Play 25-in-1 (DGUN-806)", MACHINE_NOT_WORKING )



// titles below need inputs mapping to go further

CONS( 1997, bandgpad,    0,  0,  nes_sh6578,     nes_sh6578, nes_sh6578_state, init_nes_sh6578, "Bandai", "Multi Game Player Gamepad", MACHINE_NOT_WORKING )

CONS( 1997, bandggcn,    0,  0,  nes_sh6578,     nes_sh6578, nes_sh6578_state, init_nes_sh6578, "Bandai", "Go! Go! Connie-chan! Asobou Mouse", MACHINE_NOT_WORKING )

CONS( 200?, cpatrolm,    0,  0,  nes_sh6578_pal, nes_sh6578, nes_sh6578_state, init_nes_sh6578, "TimeTop", "City Patrolman", MACHINE_NOT_WORKING )

// Super Moto 3 https://youtu.be/DR5Y_r6C_qk - has JungleTac copyrights intact, and appears to have the SH6578 versions of the games

// Handy Max 15-in-1 https://youtu.be/jQTUHj1cP-k - has JungleTac copyrights intact, and appears to have the SH6578 versions of the games

// has sampled audio, not supported (make sure it isn't being done by an external device)
CONS( 201?, dancmix3, 0, 0, nes_sh6578_pal, nes_sh6578, nes_sh6578_state, init_nes_sh6578, "Venom", "TV Dance Mat 4 Games in 1 (Mix Party 3 / Dance Mix 3)", MACHINE_NOT_WORKING )
