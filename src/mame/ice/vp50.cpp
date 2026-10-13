// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Play Mechanix / Right Hand Tech "VP50" platform
    Emulation by R. Belmont

    Games:
        - Rhythm Nation
        - Zoofari

    This is the "cheap and cheerful" version of the VP10x.
    - Toshiba TX4925 MIPS-based SoC
    - 256 color dumb framebuffer
    - 22 kHz stereo audio DAC

***************************************************************************/

#include "emu.h"

#include "bus/ata/ataintf.h"
#include "cpu/mips/mips3.h"
#include "machine/eepromser.h"
#include "sound/dmadac.h"

#include "screen.h"
#include "speaker.h"

#include <algorithm>
#include <iterator>
#include <string>

#define LOG_TTY     (1U << 1)   // the boot ROM's console

#define VERBOSE (0)
#include "logmacro.h"


namespace {

class vp50_state : public driver_device
{
public:
	void vp50(machine_config &config) ATTR_COLD;

protected:
	vp50_state(const machine_config &mconfig, device_type type, const char *tag, uint8_t pic_product)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainram(*this, "mainram")
		, m_ata(*this, "ata")
		, m_dmadac(*this, "dac%u", 0U)
		, m_eeprom(*this, "eeprom")
		, m_latches(*this, "LATCH%u", 1U)
		, m_outputs(*this, "output%u", 0U)
		, m_clut{}
		, m_video{}
		, m_fpga_ctrl(0)
		, m_fpga_configured(false)
		, m_dmac{}
		, m_pio{}
		, m_audio_control(0)
		, m_audio_request(false)
		, m_audio_block{}
		, m_audio_block_frames(0)
		, m_audio_timer(nullptr)
		, m_irc_den(0)
		, m_irc_dm{}
		, m_irc_lvl{}
		, m_irc_msk(0)
		, m_irc_request{}
		, m_irc_lines(0)
		, m_irc_edges(0)
		, m_irc_cs(0)
		, m_ata_irq_timer(nullptr)
		, m_spi_control{}
		, m_spi_data(0)
		, m_spi_ready(false)
		, m_pic_cmd(0)
		, m_pic_index(0)
		, m_pic_product(pic_product)
	{
	}

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static constexpr uint32_t AUDIO_SAMPLE_RATE = 22050;
	static constexpr uint32_t AUDIO_BLOCK_FRAMES = 224;

	// FPGA
	uint32_t fpga_reg0_r();
	void fpga_reg0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t fpga_ctrl_r();
	void fpga_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t clut_r(offs_t offset);
	void clut_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t video_r(offs_t offset);
	void video_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	// TX4925 on-chip peripherals
	uint32_t dmac_r(offs_t offset);
	void dmac_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t sio_r(offs_t offset);
	void sio_w(offs_t offset, uint32_t data);
	uint32_t pio_r(offs_t offset);
	void pio_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t irc_r(offs_t offset);
	void irc_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t irc_request_r(offs_t offset);
	void irc_request_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t irc_mode(int source) const;
	uint32_t irc_pending() const;
	void irc_set_input(int source, int state);
	void irc_update();
	uint32_t spi_r(offs_t offset);
	void spi_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	// the PIC security chip and the EEPROM on the SPI
	uint8_t pic_transfer(uint8_t data);
	uint8_t eeprom_transfer(uint8_t data);

	void ata_irq_w(int state);
	TIMER_CALLBACK_MEMBER(ata_irq_ready);
	void vblank_w(int state);

	// the FPGA's audio output
	void audio_control_w(uint32_t data, uint32_t mem_mask);
	void audio_data_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void audio_reset();
	void audio_flush_block();
	void audio_update_irq();
	TIMER_CALLBACK_MEMBER(audio_request);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;

	required_device<tx4925le_device> m_maincpu;
	required_shared_ptr<uint32_t> m_mainram;
	required_device<ata_interface_device> m_ata;
	required_device_array<dmadac_sound_device, 2> m_dmadac;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_ioport_array<6> m_latches;
	output_finder<8> m_outputs;

	uint32_t m_clut[256];           // 0x00BBGGRR
	uint32_t m_video[0x40];         // timing registers at 0x1f401000
	uint32_t m_fpga_ctrl;
	bool m_fpga_configured;
	uint32_t m_dmac[8];             // channel 0: CHAR, SAR, DAR, CNTR, SAIR, DAIR, CCR, CSR
	uint32_t m_pio[4];

	// audio
	uint32_t m_audio_control;
	bool m_audio_request;
	int16_t m_audio_block[AUDIO_BLOCK_FRAMES * 2];  // left, right
	uint32_t m_audio_block_frames;
	emu_timer *m_audio_timer;

	// interrupt controller
	uint32_t m_irc_den;             // detection enable
	uint32_t m_irc_dm[2];           // detection mode, 2 bits a source
	uint32_t m_irc_lvl[8];          // levels: sources 2n, 2n+1, 2n+16, 2n+17 in each register
	uint32_t m_irc_msk;             // mask level
	uint32_t m_irc_request[6];      // request flag registers (unused by the games)
	uint32_t m_irc_lines;           // request inputs, bit per source
	uint32_t m_irc_edges;           // latched edge detections
	uint32_t m_irc_cs;              // current status
	emu_timer *m_ata_irq_timer;

	// SPI and the PIC behind it
	uint32_t m_spi_control[4];      // SPMCR, SPCR0, SPCR1, SPFS
	uint32_t m_spi_data;            // last byte received
	bool m_spi_ready;               // receive buffer holds it
	uint8_t m_pic_cmd;
	uint8_t m_pic_index;
	const uint8_t m_pic_product;    // byte 5 of the reply to command 0x22

	std::string m_tty;              // the console line being assembled
};

class rhnation_state : public vp50_state
{
public:
	rhnation_state(const machine_config &mconfig, device_type type, const char *tag)
		: vp50_state(mconfig, type, tag, 1)
	{
	}
};

class zoofari_state : public vp50_state
{
public:
	zoofari_state(const machine_config &mconfig, device_type type, const char *tag)
		: vp50_state(mconfig, type, tag, 5)
	{
	}
};


void vp50_state::machine_start()
{
	m_ata_irq_timer = timer_alloc(FUNC(vp50_state::ata_irq_ready), this);
	m_audio_timer = timer_alloc(FUNC(vp50_state::audio_request), this);

	for (auto &dac : m_dmadac)
	{
		dac->set_frequency(AUDIO_SAMPLE_RATE);
	}

	save_item(NAME(m_clut));
	save_item(NAME(m_video));
	save_item(NAME(m_fpga_ctrl));
	save_item(NAME(m_fpga_configured));
	save_item(NAME(m_audio_control));
	save_item(NAME(m_audio_request));
	save_item(NAME(m_audio_block));
	save_item(NAME(m_audio_block_frames));
	save_item(NAME(m_dmac));
	save_item(NAME(m_pio));
	save_item(NAME(m_irc_den));
	save_item(NAME(m_irc_dm));
	save_item(NAME(m_irc_lvl));
	save_item(NAME(m_irc_msk));
	save_item(NAME(m_irc_request));
	save_item(NAME(m_irc_lines));
	save_item(NAME(m_irc_edges));
	save_item(NAME(m_irc_cs));
	save_item(NAME(m_spi_control));
	save_item(NAME(m_spi_data));
	save_item(NAME(m_spi_ready));
	save_item(NAME(m_pic_cmd));
	save_item(NAME(m_pic_index));
}

void vp50_state::machine_reset()
{
	// the FPGA is reconfigured from the boot ROM on every reset, which implies all state goes away
	std::fill(std::begin(m_clut), std::end(m_clut), 0);
	std::fill(std::begin(m_video), std::end(m_video), 0);
	m_fpga_ctrl = 0;
	m_fpga_configured = false;
	std::fill(std::begin(m_dmac), std::end(m_dmac), 0);
	std::fill(std::begin(m_pio), std::end(m_pio), 0);

	m_irc_den = 0;
	std::fill(std::begin(m_irc_dm), std::end(m_irc_dm), 0);
	std::fill(std::begin(m_irc_lvl), std::end(m_irc_lvl), 0);
	m_irc_msk = 0;
	std::fill(std::begin(m_irc_request), std::end(m_irc_request), 0);
	m_irc_edges = 0;
	// INT pins other than the drive's idle high
	m_irc_lines = 0x3fc & ~(1U << 5);
	m_ata_irq_timer->adjust(attotime::never);
	irc_update();

	audio_reset();

	std::fill(std::begin(m_spi_control), std::end(m_spi_control), 0);
	m_spi_data = 0;
	m_spi_ready = false;
	m_pic_cmd = 0;
	m_pic_index = 0;
}


/***************************************************************************
    FPGA
***************************************************************************/

// the bitstream port until the FPGA is configured, then the audio control register
uint32_t vp50_state::fpga_reg0_r()
{
	if (!m_fpga_configured)
	{
		return 0;
	}
	return (m_audio_control & ~uint32_t(1U << 2)) | (m_audio_request ? (1U << 2) : 0);
}

void vp50_state::fpga_reg0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_fpga_configured)
	{
		audio_control_w(data, mem_mask);
	}
}

uint32_t vp50_state::fpga_ctrl_r()
{
	return m_fpga_ctrl;
}

void vp50_state::fpga_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: fpga_ctrl_w: %08x\n", machine().describe_context(), data);
	COMBINE_DATA(&m_fpga_ctrl);

	// bit 8 is VBL ack
	if (BIT(m_fpga_ctrl, 8))
	{
		irc_set_input(2, 1);
	}
}

void vp50_state::vblank_w(int state)
{
	if (state && BIT(m_fpga_ctrl, 8))
	{
		irc_set_input(2, 0);
	}
}


/***************************************************************************
    Audio support
***************************************************************************/

void vp50_state::audio_reset()
{
	m_audio_control = 0;
	m_audio_request = false;
	m_audio_block_frames = 0;
	m_audio_timer->adjust(attotime::never);
	for (auto &dac : m_dmadac)
	{
		dac->enable(0);
	}
	audio_update_irq();
}

// bit 0 starts playback, bit 1 routes the request to INT[1], bit 2 reads the request back
void vp50_state::audio_control_w(uint32_t data, uint32_t mem_mask)
{
	const uint32_t previous = m_audio_control;
	COMBINE_DATA(&m_audio_control);
	LOG("%s: audio_control_w: %08x\n", machine().describe_context(), m_audio_control);

	if (BIT(previous ^ m_audio_control, 0))
	{
		for (auto &dac : m_dmadac)
		{
			dac->enable(BIT(m_audio_control, 0));
		}
		if (BIT(m_audio_control, 0))
		{
			const attotime period = attotime::from_ticks(AUDIO_BLOCK_FRAMES, AUDIO_SAMPLE_RATE);
			m_audio_timer->adjust(period, 0, period);
		}
		else
		{
			m_audio_timer->adjust(attotime::never);
			m_audio_request = false;
			m_audio_block_frames = 0;
		}
	}
	audio_update_irq();
}

void vp50_state::audio_data_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!ACCESSING_BITS_0_15)
	{
		return;
	}

	m_audio_block[m_audio_block_frames * 2] = int16_t(data);
	m_audio_block[(m_audio_block_frames * 2) + 1] = int16_t(data >> 16);
	if (++m_audio_block_frames == AUDIO_BLOCK_FRAMES)
	{
		audio_flush_block();
	}

	m_audio_request = false;
	audio_update_irq();
}

void vp50_state::audio_flush_block()
{
	if (!m_audio_block_frames)
	{
		return;
	}

	for (int channel = 0; channel < 2; channel++)
	{
		m_dmadac[channel]->flush();
		m_dmadac[channel]->transfer(channel, 1, 2, m_audio_block_frames, m_audio_block);
	}
	m_audio_block_frames = 0;
}

// signal the CPU that we need a new block of audio data
TIMER_CALLBACK_MEMBER(vp50_state::audio_request)
{
	audio_flush_block();
	m_audio_request = true;
	audio_update_irq();
}

void vp50_state::audio_update_irq()
{
	irc_set_input(3, (BIT(m_audio_control, 1) && m_audio_request) ? 0 : 1);
}

uint32_t vp50_state::clut_r(offs_t offset)
{
	return m_clut[offset];
}

void vp50_state::clut_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_clut[offset]);
}

uint32_t vp50_state::video_r(offs_t offset)
{
	return m_video[offset];
}

void vp50_state::video_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: video_w: %03x = %08x\n", machine().describe_context(), offset * 4, data);
	COMBINE_DATA(&m_video[offset]);
}


/***************************************************************************
    TX4925 on-chip peripherals (TODO: move to dedicated TX4925 device)
***************************************************************************/

// DMA channel 0 carries the frame buffer to the FPGA's pixel port
uint32_t vp50_state::dmac_r(offs_t offset)
{
	return m_dmac[offset];
}

void vp50_state::dmac_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: dmac_w: %02x = %08x\n", machine().describe_context(), offset * 4, data);
	COMBINE_DATA(&m_dmac[offset]);
}

// serial port: the boot ROM's console
uint32_t vp50_state::sio_r(offs_t offset)
{
	if (offset == 0x0c / 4)
	{
		return 0x2;     // transmitter ready
	}
	return 0;
}

void vp50_state::sio_w(offs_t offset, uint32_t data)
{
	if (offset == 0x1c / 4)
	{
		if (data == 0x0d || m_tty.length() >= 256)
		{
			LOGMASKED(LOG_TTY, "tty: %s\n", m_tty);
			m_tty.clear();
		}
		if (data >= 0x20 || data == 0x09)
		{
			m_tty += char(data);
		}
	}
}

// GPIO: the boot ROM watches the FPGA's DONE pin on input bit 12
uint32_t vp50_state::pio_r(offs_t offset)
{
	if (offset == 0x04 / 4)
	{
		return 0x1000;
	}
	return m_pio[offset];
}

// output bit 20 is the EEPROM's CS, bit 12 the FPGA's PROG, bit 24 set once DONE is up
void vp50_state::pio_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const uint32_t previous = m_pio[offset];
	COMBINE_DATA(&m_pio[offset]);
	if (offset == 0 && BIT(previous ^ m_pio[0], 20))
	{
		m_eeprom->cs_write(BIT(m_pio[0], 20));
	}
	if (offset == 0 && BIT(~previous & m_pio[0], 24))
	{
		m_fpga_configured = true;
	}
	if (offset == 0x08 / 4 && BIT(m_pio[2], 12) && !BIT(m_pio[0], 12) && m_fpga_configured)
	{
		m_fpga_configured = false;
		audio_reset();
	}
}

/***************************************************************************
    TX4925 interrupt controller
***************************************************************************/

// INT[7:0] are sources 2-9; the highest level above the mask goes to IP[2], its number to IP[6:3]

// a source's detection mode: two bits, sixteen sources a register
uint32_t vp50_state::irc_mode(int source) const
{
	return (m_irc_dm[source >> 4] >> ((source & 15) * 2)) & 3;
}

// the sources requesting service, regardless of level and mask
uint32_t vp50_state::irc_pending() const
{
	uint32_t pending = 0;
	for (int source = 1; source < 31; source++)
	{
		const uint32_t bit = 1U << source;
		const uint32_t mode = irc_mode(source);
		if (source >= 2 && source <= 9)
		{
			// external pins: low level, high level, falling edge, rising edge
			switch (mode)
			{
				case 0: pending |= ~m_irc_lines & bit; break;
				case 1: pending |= m_irc_lines & bit; break;
				default: pending |= m_irc_edges & bit; break;
			}
		}
		else if (mode == 0)
		{
			// internal sources are enabled by mode 0
			pending |= m_irc_lines & bit;
		}
	}
	return pending;
}

void vp50_state::irc_set_input(int source, int state)
{
	const uint32_t bit = 1U << source;
	const uint32_t previous = m_irc_lines;
	if (state)
	{
		m_irc_lines |= bit;
	}
	else
	{
		m_irc_lines &= ~bit;
	}

	// edge modes latch the transition
	const uint32_t mode = irc_mode(source);
	if (source >= 2 && source <= 9)
	{
		if (mode == 2 && (previous & bit) && !state)
		{
			m_irc_edges |= bit;
		}
		if (mode == 3 && !(previous & bit) && state)
		{
			m_irc_edges |= bit;
		}
	}
	irc_update();
}

void vp50_state::irc_update()
{
	const uint32_t pending = irc_pending();

	int cause = -1;
	int best = -1;
	if (m_irc_den & 1)
	{
		for (int source = 1; source < 31; source++)
		{
			if (!(pending & (1U << source)))
			{
				continue;
			}
			const int level = (m_irc_lvl[(source & 15) >> 1] >> (((source & 1) * 8) + ((source >= 16) ? 16 : 0))) & 7;
			if (level > int(m_irc_msk & 7) && level > best)
			{
				best = level;
				cause = source;
			}
		}
	}

	if (cause < 0)
	{
		m_irc_cs = 0x1001f;
		for (int line = MIPS3_IRQ0; line <= MIPS3_IRQ4; line++)
		{
			m_maincpu->set_input_line(line, CLEAR_LINE);
		}
	}
	else
	{
		m_irc_cs = (best << 8) | cause;
		m_maincpu->set_input_line(MIPS3_IRQ0, ASSERT_LINE);
		for (int bit = 0; bit < 4; bit++)
		{
			m_maincpu->set_input_line(MIPS3_IRQ1 + bit, BIT(cause, bit) ? ASSERT_LINE : CLEAR_LINE);
		}
	}
}

uint32_t vp50_state::irc_r(offs_t offset)
{
	switch (offset)
	{
		case 0x00 / 4: return m_irc_den;
		case 0x04 / 4: return m_irc_dm[0];
		case 0x08 / 4: return m_irc_dm[1];
		case 0x10 / 4: case 0x14 / 4: case 0x18 / 4: case 0x1c / 4:
		case 0x20 / 4: case 0x24 / 4: case 0x28 / 4: case 0x2c / 4:
			return m_irc_lvl[offset - 0x10 / 4];
		case 0x40 / 4: return m_irc_msk;
		case 0x80 / 4: return irc_pending();
		case 0xa0 / 4: return m_irc_cs;
	}
	return 0;
}

void vp50_state::irc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: irc_w: %02x = %08x\n", machine().describe_context(), offset * 4, data);
	switch (offset)
	{
		case 0x00 / 4: COMBINE_DATA(&m_irc_den); break;
		case 0x04 / 4: COMBINE_DATA(&m_irc_dm[0]); break;
		case 0x08 / 4: COMBINE_DATA(&m_irc_dm[1]); break;
		case 0x10 / 4: case 0x14 / 4: case 0x18 / 4: case 0x1c / 4:
		case 0x20 / 4: case 0x24 / 4: case 0x28 / 4: case 0x2c / 4:
			COMBINE_DATA(&m_irc_lvl[offset - 0x10 / 4]);
			break;
		case 0x40 / 4: COMBINE_DATA(&m_irc_msk); break;
		case 0x60 / 4:
			// edge detection clear: the source in bits 3-0 when bit 8 is set
			if (BIT(data, 8))
			{
				m_irc_edges &= ~(1U << (data & 0xf));
			}
			break;
	}
	irc_update();
}

uint32_t vp50_state::irc_request_r(offs_t offset)
{
	return m_irc_request[offset];
}

void vp50_state::irc_request_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_irc_request[offset]);
}

// FIXME: a delayed pulse instead of the drive's level, which storms on the loader's overrun read
void vp50_state::ata_irq_w(int state)
{
	if (state)
	{
		m_ata_irq_timer->adjust(attotime::from_usec(50), 1);
	}
	else
	{
		m_ata_irq_timer->adjust(attotime::never);
		irc_set_input(5, 0);
	}
}

TIMER_CALLBACK_MEMBER(vp50_state::ata_irq_ready)
{
	irc_set_input(5, param);
	if (param)
	{
		m_ata_irq_timer->adjust(attotime::from_usec(20), 0);
	}
}


/***************************************************************************
    The PIC security chip
***************************************************************************/

// a command byte, then each zero byte clocks out the next reply byte; 0x22 has the product code
uint8_t vp50_state::pic_transfer(uint8_t data)
{
	static constexpr uint8_t VERSION[] = { 0x01, 0x02, 0x00 };
	static constexpr uint8_t MAGIC[] = { 0xaa, 0x55, 0x18, 0x18, 0xc0, 0x03, 0xf0, 0x0f, 0x09, 0x0a };
	static constexpr uint16_t SERIAL = 6502;

	// the byte after command 0x41 is the output driver state, whatever its value
	if (m_pic_cmd == 0x41 && m_pic_index == 0)
	{
		m_pic_index = 1;
		for (int i = 0; i < 8; i++)
		{
			m_outputs[i] = BIT(data, i);
		}
		return 0;
	}

	if (data != 0)
	{
		m_pic_cmd = data;
		m_pic_index = 0;
		return 0;
	}

	const uint8_t index = m_pic_index++;
	switch (m_pic_cmd)
	{
		case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06:
			return m_latches[m_pic_cmd - 1]->read();

		case 0x20:
			return (index < std::size(VERSION)) ? VERSION[index] : 0;

		case 0x21:
		case 0x22:
			switch (index)
			{
				case 2: return 0x01;
				case 3: return 0x02;
				case 4: return 0x00;
				case 5: return m_pic_product;
				case 6: return SERIAL >> 8;
				case 7: return SERIAL & 0xff;
				default: return 0;
			}

		case 0x23:
			return (index < std::size(MAGIC)) ? MAGIC[index] : 0;

		case 0x24:      // ADC, ten bytes; nothing is connected on these games
			return 0;
	}

	// 0x0c/0x0f and 0x40 are sent by Rhythm Nation but never read back
	return 0xff;
}

uint8_t vp50_state::eeprom_transfer(uint8_t data)
{
	uint8_t result = 0;
	for (int bit = 7; bit >= 0; bit--)
	{
		m_eeprom->di_write(BIT(data, bit));
		result |= m_eeprom->do_read() << bit;
		m_eeprom->clk_write(1);
		m_eeprom->clk_write(0);
	}
	return result;
}

uint32_t vp50_state::spi_r(offs_t offset)
{
	switch (offset)
	{
		case 0x00 / 4: case 0x04 / 4: case 0x08 / 4: case 0x0c / 4:
			return m_spi_control[offset];

		case 0x14 / 4:
			// status: no transmit interrupt, idle, transmit ready, receive ready
			return 0x8006 | (m_spi_ready ? 1 : 0);

		case 0x18 / 4:
			if (!machine().side_effects_disabled())
			{
				m_spi_ready = false;
			}
			return m_spi_data;
	}
	return 0;
}

void vp50_state::spi_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x00 / 4: case 0x04 / 4: case 0x08 / 4: case 0x0c / 4:
			COMBINE_DATA(&m_spi_control[offset]);
			break;

		case 0x18 / 4:
			// a word goes out and one comes back, from whichever slave the PIO selects
			if (BIT(m_pio[0], 20))
			{
				m_spi_data = eeprom_transfer(data & 0xff);
			}
			else if (data & 0xff00)
			{
				LOG("%s: volume word %04x\n", machine().describe_context(), data & 0xffff);
				m_dmadac[BIT(data, 8)]->set_volume(data & 0xff);
				m_spi_data = 0;
			}
			else
			{
				m_spi_data = pic_transfer(data & 0xff);
			}
			m_spi_ready = true;
			break;
	}
}


/***************************************************************************
    Video
***************************************************************************/

uint32_t vp50_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const uint32_t source = m_dmac[1] & 0x03ffffff;
	const int lines = std::min<int>(m_dmac[3] / 400, 256);
	if (m_dmac[1] == 0 || source + (lines * 400) > 0x04000000)
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}

	const uint8_t *const pixels = reinterpret_cast<const uint8_t *>(&m_mainram[0]) + source;
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint32_t *line = &bitmap.pix(y, cliprect.left());
		if (y >= lines)
		{
			std::fill_n(line, cliprect.width(), rgb_t::black());
			continue;
		}
		const uint8_t *src = pixels + (y * 400) + cliprect.left();
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			const uint32_t entry = m_clut[*src++];
			*line++ = rgb_t(entry & 0xff, (entry >> 8) & 0xff, (entry >> 16) & 0xff);
		}
	}
	return 0;
}

void vp50_state::main_map(address_map &map)
{
	map(0x00000000, 0x03ffffff).ram().share(m_mainram);

	map(0x1f000010, 0x1f00001f).rw(m_ata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w));
	map(0x1f000020, 0x1f00002f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w));

	// FPGA
	map(0x1f400000, 0x1f400003).rw(FUNC(vp50_state::fpga_reg0_r), FUNC(vp50_state::fpga_reg0_w));
	map(0x1f400004, 0x1f400007).w(FUNC(vp50_state::audio_data_w));
	map(0x1f400008, 0x1f40000b).rw(FUNC(vp50_state::fpga_ctrl_r), FUNC(vp50_state::fpga_ctrl_w));
	map(0x1f40000c, 0x1f40000f).nopw();     // pixel port, fed by DMA
	map(0x1f400800, 0x1f400bff).rw(FUNC(vp50_state::clut_r), FUNC(vp50_state::clut_w));
	map(0x1f401000, 0x1f4010ff).rw(FUNC(vp50_state::video_r), FUNC(vp50_state::video_w));

	map(0x1fc00000, 0x1fffffff).rom().region("maincpu", 0);

	// TX4925 on-chip peripherals
	map(0xff1f8000, 0xff1f803f).noprw();    // SDRAM controller
	map(0xff1f9000, 0xff1f903f).noprw();    // external bus controller
	map(0xff1fb000, 0xff1fb01f).rw(FUNC(vp50_state::dmac_r), FUNC(vp50_state::dmac_w));
	map(0xff1fe000, 0xff1fe003).noprw();    // chip configuration
	map(0xff1ff400, 0xff1ff43f).rw(FUNC(vp50_state::sio_r), FUNC(vp50_state::sio_w));
	map(0xff1ff500, 0xff1ff50f).rw(FUNC(vp50_state::pio_r), FUNC(vp50_state::pio_w));
	map(0xff1ff510, 0xff1ff527).rw(FUNC(vp50_state::irc_request_r), FUNC(vp50_state::irc_request_w));
	map(0xff1ff600, 0xff1ff6af).rw(FUNC(vp50_state::irc_r), FUNC(vp50_state::irc_w));
	map(0xff1ff800, 0xff1ff81f).rw(FUNC(vp50_state::spi_r), FUNC(vp50_state::spi_w));
}

// the PIC's six input latches, named according to the boot ROM's switch test
static INPUT_PORTS_START( vp50 )
	PORT_START("LATCH1")    // P1
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Select")
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Pad 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Pad 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Pad 3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Pad 4")

	PORT_START("LATCH2")    // P2, nothing connected
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LATCH3")    // C1
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LATCH4")    // GUN, nothing connected
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LATCH5")    // C2
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Ticket Sensor")   // FIXME: ticket dispenser bit not found yet
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BILL1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LATCH6")    // DIP: SW51; Rhythm Nation does not appear to use it
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW51:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW51:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW51:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW51:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW51:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW51:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW51:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW51:8" )
INPUT_PORTS_END

void vp50_state::vp50(machine_config &config)
{
	TX4925LE(config, m_maincpu, 200'000'000);
	m_maincpu->set_dcache_size(32768);
	m_maincpu->set_system_clock(100'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &vp50_state::main_map);

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(vp50_state::screen_update));
	screen.set_size(400, 256);
	screen.set_visarea(0, 399, 0, 255);
	screen.screen_vblank().set(FUNC(vp50_state::vblank_w));

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
	m_ata->irq_handler().set(FUNC(vp50_state::ata_irq_w));

	// the game never polls for ready, so writes complete at once (FIXME: identify the specific part to validate the behavior)
	EEPROM_93C46_16BIT(config, m_eeprom).write_time(attotime::zero);

	SPEAKER(config, "speaker", 2).front();

	DMADAC(config, m_dmadac[0]).add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	DMADAC(config, m_dmadac[1]).add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}


ROM_START(rhnation)
	ROM_REGION(0x400000, "maincpu", 0)  /* Boot ROM */
	ROM_LOAD( "rhythm_nation_rev_3.1.5_m27v322.u13", 0x000000, 0x400000, CRC(456f043d) SHA1(cc166897fdbdaa3583e44816da9dfbbf303f5c61) )

	ROM_REGION(0x4000, "pic", 0)        /* PIC18C242 program - read-protected, need dumped */
	ROM_LOAD( "pic18c242-i-sp.u22", 0x000000, 0x4000, NO_DUMP )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE("rhn010104", 0, SHA1(5bc2e5817b29bf42ec483414242795fd76d749d9) )  // writable: settings and audits live on the drive
ROM_END

ROM_START(zoofari)
	ROM_REGION(0x400000, "maincpu", 0)  /* Boot ROM */
	ROM_LOAD( "zf_boot_rel.u13", 0x000000, 0x400000, CRC(e629689a) SHA1(7352d033c638040c3e51a453e2440a7f38a1b406) )

	ROM_REGION(0x4000, "pic", 0)        /* PIC18C442 program */
	ROM_LOAD( "8777z-568.bin", 0x000000, 0x4000, NO_DUMP )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE("zoofari", 0, SHA1(8fb9cfb1ab2660f40b643fcd772243903bd69a6c) )
ROM_END

} // anonymous namespace


GAME( 2003, rhnation, 0, vp50, vp50, rhnation_state, empty_init, ROT0, "ICE / Play Mechanix", "Rhythm Nation (v01.00.04, boot v3.1.5)", MACHINE_NOT_WORKING )
GAME( 2006, zoofari,  0, vp50, vp50, zoofari_state,  empty_init, ROT0, "ICE / Play Mechanix", "Zoofari",                                MACHINE_NOT_WORKING )
