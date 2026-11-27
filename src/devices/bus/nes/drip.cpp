// license:BSD-3-Clause
// copyright-holders:Brian H. Gilbert
/***********************************************************************************************************

 NES/Famicom cartridge emulation for the Drip PCB

 Quietust's FPGA mapper for his port of Drip (UNIF UNL-DripGame, NES 2.0 mapper 284, once iNES mapper 102).

 Registers (NESdev Wiki, UNIF/UNL-DripGame):
 $4800-$4FFF  status: bit 7 = DIP switch, low bits read $64 when the FPGA is ready
 $5000-$57FF  sound channel 0 status (bit 7 = FIFO full, bit 6 = FIFO empty)
 $5800-$5FFF  sound channel 1 status
 $8000-$BFFF  writes, decoded on the low four bits:
   0-3 and 4-7: sound channel 0 and 1: silence/clear FIFO, FIFO data, period low, period high and volume
   8, 9:  IRQ low and high (a write to the high register loads the counter, bit 7 enables and it also
          acknowledges the IRQ)
   A:     bits 0-1 mirroring (V, H, one screen A, one screen B), bit 2 extended attributes, bit 3 SRAM write
   B:     16K PRG bank at $8000-$BFFF (the last bank is fixed at $C000-$FFFF)
   C-F:   2K CHR banks at $0000, $0800, $1000 and $1800
 $C000-$FFFF  write-only extended attribute RAM, $C000-$C3FF and $C400-$C7FF, mirrored in the whole range;
              the two low bits of each byte are the palette of the same tile of a nametable

 The IRQ counter counts down on every CPU cycle and stops with the IRQ when it reaches zero.

 The playback timing is in Quietust's mapper document (qmtpro.com/~nes/drip/dripmap.txt, 2006/08/22), which the
 NESdev page links: the period counts CPU cycles per sample, a write to an empty channel starts it at once, a
 channel is silenced when its FIFO runs dry, a period change waits for the sample being played and a volume
 change applies at once. A sample is biased by -$80 and pulse width modulated by the volume: volume v outputs it
 for v + 1 cycles of every 16 and zero for the rest. Only the average of that is produced here, (data - $80) *
 (v + 1) / 16, as the modulation runs far above the audible range.

 Not specified by the document, and not verified against hardware: what a write to a full FIFO does (it is
 ignored here), a period of 0, whether the FIFO flags count the sample being played (they do here, as in
 Mesen2) and the output level (the route gain is an estimate).

 ***********************************************************************************************************/

#include "emu.h"
#include "drip.h"


//-------------------------------------------------
//  constructors
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_DRIP_SOUND, nes_drip_sound_device, "nes_drip_snd", "NES Cart Drip Sound")
DEFINE_DEVICE_TYPE(NES_DRIPGAME,   nes_dripgame_device,   "nes_dripgame", "NES Cart Drip (Quietust) PCB")

nes_drip_sound_device::nes_drip_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_DRIP_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
{
}

nes_dripgame_device::nes_dripgame_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_DRIPGAME, tag, owner, clock)
	, m_snd(*this, "drip_snd")
	, m_dipsw(*this, "DIP")
{
}


//-------------------------------------------------
//  sound
//-------------------------------------------------

void nes_drip_sound_device::device_start()
{
	m_stream = stream_alloc(0, 1, clock());

	save_item(NAME(m_fifo));
	save_item(NAME(m_head));
	save_item(NAME(m_count));
	save_item(NAME(m_period));
	save_item(NAME(m_timer));
	save_item(NAME(m_volume));
}

void nes_drip_sound_device::device_reset()
{
	std::fill(std::begin(m_fifo), std::end(m_fifo), 0);
	for (int i = 0; i < 2; i++)
	{
		m_head[i] = 0;
		m_count[i] = 0;
		m_period[i] = 0;
		m_timer[i] = 0;
		m_volume[i] = 0;
	}
}

u8 nes_drip_sound_device::status_r(unsigned channel)
{
	m_stream->update();     // the flags have to be current, games poll them to keep the FIFO filled
	return (m_count[channel] == 256 ? 0x80 : 0x00) | (m_count[channel] == 0 ? 0x40 : 0x00);
}

void nes_drip_sound_device::write(offs_t offset, u8 data)
{
	m_stream->update();

	const unsigned ch = BIT(offset, 2);

	switch (offset & 3)
	{
	case 0: // silence the channel and clear its FIFO; the timer goes back to the last written period
		m_count[ch] = 0;
		m_head[ch] = 0;
		m_timer[ch] = m_period[ch];
		break;

	case 1: // FIFO data
		if (m_count[ch] < 256)
		{
			if (m_count[ch] == 0)
				m_timer[ch] = m_period[ch];     // playback starts at once on an empty channel
			m_fifo[(ch << 8) | u8(m_head[ch] + m_count[ch])] = data;
			m_count[ch]++;
		}
		break;

	case 2: // period, low 8 bits (it applies when the sample being played has finished)
		m_period[ch] = (m_period[ch] & 0x0f00) | data;
		break;

	case 3: // period, high 4 bits, and volume (the volume applies at once)
		m_period[ch] = (m_period[ch] & 0x00ff) | ((data & 0x0f) << 8);
		m_volume[ch] = data >> 4;
		break;
	}
}

void nes_drip_sound_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		s32 sum = 0;
		for (int ch = 0; ch < 2; ch++)
		{
			if (m_count[ch])
			{
				if (--m_timer[ch] == 0)         // the sample has been played
				{
					m_timer[ch] = m_period[ch];
					m_head[ch]++;
					m_count[ch]--;
				}
				if (m_count[ch])
					sum += (int(m_fifo[(ch << 8) | m_head[ch]]) - 0x80) * (m_volume[ch] + 1);
			}
		}
		stream.put_int(0, i, sum, 128 * 16 * 2);
	}
}


//-------------------------------------------------
//  Drip PCB
//-------------------------------------------------

static INPUT_PORTS_START( drip )
	PORT_START("DIP")
	PORT_CONFNAME( 0x80, 0x00, "DIP Switch" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

ioport_constructor nes_dripgame_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(drip);
}

void nes_dripgame_device::device_add_mconfig(machine_config &config)
{
	// the level is an estimate, nothing was measured
	NES_DRIP_SOUND(config, m_snd, DERIVED_CLOCK(1, 1)).add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.40);
}

void nes_dripgame_device::device_start()
{
	common_start();
	save_item(NAME(m_ext_attr));
	save_item(NAME(m_irq_low));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_mirror));
	save_item(NAME(m_ext_attr_enable));
	save_item(NAME(m_sram_write));
	save_item(NAME(m_last_nt));
}

void nes_dripgame_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	std::fill(&m_ext_attr[0][0], &m_ext_attr[0][0] + sizeof(m_ext_attr), 0);
	m_irq_low = 0;
	m_irq_count = 0;
	m_irq_enable = false;
	m_mirror = 0;
	m_ext_attr_enable = false;
	m_sram_write = false;
	m_last_nt = 0;
}

u8 nes_dripgame_device::read_l(offs_t offset)
{
	offset += 0x100;    // $4100-$5FFF, relative to $4000
	if (offset < 0x800)
		return get_open_bus();
	if (offset < 0x1000)
		return (m_dipsw->read() & 0x80) | 0x64;
	return m_snd->status_r(offset < 0x1800 ? 0 : 1);
}

void nes_dripgame_device::write_m(offs_t offset, u8 data)
{
	if (m_sram_write)
		nes_nrom_device::write_m(offset, data);
}

void nes_dripgame_device::write_h(offs_t offset, u8 data)
{
	if (offset >= 0x4000)
	{
		// $C000-$FFFF: extended attribute RAM, two nametable pages
		m_ext_attr[BIT(offset, 10)][offset & 0x3ff] = data;
		return;
	}

	switch (offset & 0x0f)
	{
	case 0x00: case 0x01: case 0x02: case 0x03:
	case 0x04: case 0x05: case 0x06: case 0x07:
		m_snd->write(offset & 7, data);
		break;

	case 0x08:
		m_irq_low = data;
		break;

	case 0x09:
		// the low byte is buffered until this write, which loads the counter and acknowledges the IRQ
		m_irq_count = (u16(data & 0x7f) << 8) | m_irq_low;
		m_irq_enable = BIT(data, 7);
		set_irq_line(CLEAR_LINE);
		break;

	case 0x0a:
		m_mirror = data & 3;
		switch (m_mirror)
		{
		case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;
		case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;
		case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
		case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
		}
		m_ext_attr_enable = BIT(data, 2);
		m_sram_write = BIT(data, 3);
		break;

	case 0x0b:
		prg16_89ab(data & 0x0f);
		break;

	case 0x0c:
		chr2_0(data & 0x0f, m_chr_source);
		break;

	case 0x0d:
		chr2_2(data & 0x0f, m_chr_source);
		break;

	case 0x0e:
		chr2_4(data & 0x0f, m_chr_source);
		break;

	case 0x0f:
		chr2_6(data & 0x0f, m_chr_source);
		break;
	}
}

u8 nes_dripgame_device::nt_r(offs_t offset)
{
	if (m_ext_attr_enable && !machine().side_effects_disabled())
	{
		if ((offset & 0x3ff) < 0x3c0)
		{
			// remember the tile, the attribute fetch of the same tile follows
			m_last_nt = offset & 0x3ff;
		}
		else
		{
			// the palette comes from the extended attribute RAM; it is repeated four times so that the PPU
			// finds it whichever part of the attribute byte it picks
			const int page = (offset >> 10) & 3;
			int bank;
			switch (m_mirror)
			{
			default:
			case 0: bank = page & 1; break;     // vertical
			case 1: bank = page >> 1; break;    // horizontal
			case 2: bank = 0; break;
			case 3: bank = 1; break;
			}
			return (m_ext_attr[bank][m_last_nt] & 3) * 0x55;
		}
	}
	return nes_nrom_device::nt_r(offset);
}

void nes_dripgame_device::m2_w(int state, offs_t address, bool write)
{
	if (!state || !m_irq_enable)
		return;

	// counts down on every CPU cycle and stops with the IRQ at zero
	if ((m_irq_count > 0) && (--m_irq_count == 0))
	{
		m_irq_enable = false;
		set_irq_line(ASSERT_LINE);
	}
}
