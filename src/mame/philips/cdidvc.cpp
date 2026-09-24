// license:BSD-3-Clause
// copyright-holders:Alexandre Derumier
/******************************************************************************

    Philips CD-i Digital Video Cartridge slot and VMPEG cartridge.  See
    cdidvc.h.

    The board level behaviour was worked out using the reverse engineering of
    the CDi_MiSTer FPGA core by Andre Zeps as the hardware reference, in
    particular its notes in doc/dvc.md:
    https://github.com/MiSTer-devel/CDi_MiSTer

    Emulation notes:

    - Both decoders see the whole MPEG-1 system stream and select their own
      elementary stream from it, so a transfer is handed to whichever of them
      asked for it.
    - MAME's SCC68070 has no peripheral DMA handshake, so a transfer request
      from a decoder drives channel 1 directly, as cdicdic.cpp does.
    - The 512KB window at 0xe80000 is the video decoder's own picture DRAM,
      which the chip lets the host reach through its second chip select.
    - The decoders share the interrupt line to the player.  On acknowledge
      the audio decoder's vector wins, which is the priority the driver's
      handler expects.

*******************************************************************************/

#include "emu.h"
#include "cdidvc.h"

#define LOG_REGS_R    (1U << 1)
#define LOG_REGS_W    (1U << 2)
#define LOG_DMA       (1U << 3)
#define LOG_IRQ       (1U << 4)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(CDI_DVC_SLOT, cdi_dvc_slot_device, "cdi_dvc_slot", "CD-i Digital Video Cartridge slot")
DEFINE_DEVICE_TYPE(CDI_DVC_VMPEG, cdi_dvc_vmpeg_device, "cdi_dvc_vmpeg", "CD-i Digital Video Cartridge (VMPEG)")

//**************************************************************************
//  ROM
//**************************************************************************

// The cartridge ROM holds the OS-9 FMV/FMA driver the base machine loads.
ROM_START( cdi_dvc )
	// Philips CD-i DVC card 22ER9141
	ROM_REGION16_BE(0x20000, "vmpeg", ROMREGION_ERASEFF)
	ROMX_LOAD( "fmv ffd9 p7308 r4.1 vmpeg.bin", 0x00000, 0x10000, CRC(30ba9273) SHA1(d8adca0627b356ced6131b9458ac1175e43e6548), ROM_SKIP(1) )
	ROMX_LOAD( "fmv 4ba9 p7307 r4.1 vmpeg.bin", 0x00001, 0x10000, CRC(623edb1f) SHA1(4c6b11e28ad4c2f5c2e439f7910a783e0a79d1a9), ROM_SKIP(1) )
ROM_END

const tiny_rom_entry *cdi_dvc_vmpeg_device::device_rom_region() const
{
	return ROM_NAME( cdi_dvc );
}

//**************************************************************************
//  CONSTRUCTION
//**************************************************************************

cdi_dvc_vmpeg_device::cdi_dvc_vmpeg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CDI_DVC_VMPEG, tag, owner, clock)
	, device_cdi_dvc_interface(mconfig, *this)
	, m_fmv(*this, "mcd251")
	, m_fma(*this, "fma")
	, m_rom(*this, "vmpeg")
{
}

cdi_dvc_vmpeg_device::~cdi_dvc_vmpeg_device()
{
}

void cdi_dvc_vmpeg_device::device_add_mconfig(machine_config &config)
{
	MCD251(config, m_fmv, 0);
	m_fmv->irq_callback().set(FUNC(cdi_dvc_vmpeg_device::chip_irq_w));
	m_fmv->drq_callback().set(FUNC(cdi_dvc_vmpeg_device::fmv_drq_w));
	// the video decoder's programmable timer paces the audio decoder's poll
	// flag as well as its own
	m_fmv->timer_callback().set(m_fma, FUNC(gsc38gg307_device::poll_w));

	GSC38GG307(config, m_fma, 0);
	m_fma->irq_callback().set(FUNC(cdi_dvc_vmpeg_device::chip_irq_w));
	m_fma->drq_callback().set(FUNC(cdi_dvc_vmpeg_device::fma_drq_w));
	// the cartridge's audio is a pair of channels the slot carries
	m_fma->add_route(0, DEVICE_SELF_OWNER, 1.0, 0);
	m_fma->add_route(1, DEVICE_SELF_OWNER, 1.0, 1);
}

void cdi_dvc_vmpeg_device::device_start()
{
	m_memory_space = &m_slot->scc()->space(AS_PROGRAM);

	// The cartridge answers for its own part of the bus.  What it does not
	// take stays with the machine's catch-all, which bus errors, so a player
	// with no cartridge fitted behaves as one.
	m_memory_space->install_readwrite_handler(0xe00000, 0xe3ffff,
			read16s_delegate(*this, FUNC(cdi_dvc_vmpeg_device::regs_r)),
			write16s_delegate(*this, FUNC(cdi_dvc_vmpeg_device::regs_w)));
	m_memory_space->install_read_handler(0xe40000, 0xe7ffff,
			read16sm_delegate(*this, FUNC(cdi_dvc_vmpeg_device::rom_r)));

	m_fmv->set_pal(m_slot->pal());

	save_item(NAME(m_mpeg_ram_enabled));
	save_item(NAME(m_mpeg_ram_enable_cnt));
	save_item(NAME(m_transfer_for_fma));
	save_item(NAME(m_intreq_state));
}

void cdi_dvc_vmpeg_device::device_reset()
{
	m_mpeg_ram_enabled = false;
	m_mpeg_ram_enable_cnt = 0;
	m_transfer_for_fma = false;

	m_intreq_state = false;
	m_slot->intreq_w(0);
}

//**************************************************************************
//  TRANSFERS
//**************************************************************************

void cdi_dvc_vmpeg_device::fma_drq_w(int state)
{
	if (state)
		run_dma(true);
}

void cdi_dvc_vmpeg_device::fmv_drq_w(int state)
{
	if (state)
		run_dma(false);
}

// A decoder has asked for the next part of the stream.  It arrives over the
// 68070's DMA channel 1, which we drive directly for want of a handshake.
void cdi_dvc_vmpeg_device::run_dma(bool for_fma)
{
	auto &ch = m_slot->scc()->dma().channel[1];

	const uint32_t start = ch.memory_address_counter;
	const uint32_t count = ch.transfer_counter;

	LOGMASKED(LOG_DMA, "%s: DVC DMA %s: %08x, %04x words\n", machine().describe_context(),
			for_fma ? "FMA" : "FMV", start, count);

	m_transfer_for_fma = for_fma;

	for (uint32_t i = 0; i < count; i++)
	{
		const uint16_t word = m_memory_space->read_word(start + i * 2);
		// each bus word carries the high byte first
		if (for_fma)
		{
			m_fma->write_data(uint8_t(word >> 8));
			m_fma->write_data(uint8_t(word));
		}
		else
		{
			m_fmv->write_data(uint8_t(word >> 8));
			m_fmv->write_data(uint8_t(word));
		}
	}

	ch.memory_address_counter += count * 2;

	if (for_fma)
		m_fma->end_of_transfer(true);
	else
		m_fmv->end_of_transfer(true);
}

//**************************************************************************
//  INTERRUPTS
//**************************************************************************

// Either decoder can drive the line, so the state is the two of them ored.
void cdi_dvc_vmpeg_device::chip_irq_w(int state)
{
	const bool req = m_fma->irq_pending() || m_fmv->irq_pending();

	if (req != m_intreq_state)
	{
		m_intreq_state = req;
		LOGMASKED(LOG_IRQ, "DVC intreq %d\n", req);
		m_slot->intreq_w(req ? 1 : 0);
	}
}

uint8_t cdi_dvc_vmpeg_device::intack_r()
{
	if (m_fma->irq_pending())
		return m_fma->vector();

	return m_fmv->vector();
}

//**************************************************************************
//  VIDEO OUTPUT
//**************************************************************************

// x is a column of the 768 pixel MCD212 line buffer, y is a display line
// counted from the start of active video.  The decoder works in CD-i pixels,
// of which there are 384 across the active line.
bool cdi_dvc_vmpeg_device::ext_video_pixel(int x, int y, uint32_t &argb) const
{
	return m_fmv->video_pixel(x / 2, y, argb);
}

void cdi_dvc_vmpeg_device::screen_vblank(int state)
{
	m_fmv->vblank_w(state);
}

//**************************************************************************
//  ROM AND RAM
//**************************************************************************

uint16_t cdi_dvc_vmpeg_device::rom_r(offs_t offset)
{
	if (!m_rom)
		return 0xffff;

	// 128KB of ROM mirrored across the 256KB window
	return m_rom[offset & 0xffff];
}

// The 512KB window is the video decoder's own picture DRAM, which it lets
// the host reach.
uint16_t cdi_dvc_vmpeg_device::ram_r(offs_t offset, uint16_t mem_mask)
{
	return m_fmv->dram_r(offset, mem_mask);
}

void cdi_dvc_vmpeg_device::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_fmv->dram_w(offset, data, mem_mask);
}

//**************************************************************************
//  REGISTER WINDOW
//**************************************************************************

uint16_t cdi_dvc_vmpeg_device::regs_r(offs_t offset, uint16_t mem_mask)
{
	// only A[15:1] is decoded, so the register file mirrors every 64KB
	const uint16_t reg = offset & 0x7fff;
	uint16_t data = 0;

	if ((reg & 0xff00) == 0x1800)
		data = m_fma->regs_r(reg & 0xff, mem_mask);
	else if ((reg & 0xff00) == 0x2000)
		data = m_fmv->regs_r(reg & 0xff, mem_mask);

	LOGMASKED(LOG_REGS_R, "%s: dvc_r: %08x = %04x & %04x\n", machine().describe_context(),
			0xe00000 + (offset << 1), data, mem_mask);

	return data;
}

void cdi_dvc_vmpeg_device::regs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	const uint16_t reg = offset & 0x7fff;

	LOGMASKED(LOG_REGS_W, "%s: dvc_w: %08x = %04x & %04x\n", machine().describe_context(),
			0xe00000 + (offset << 1), data, mem_mask);

	// The video decoder's DRAM only answers the bus once the register file
	// has been written 64 times; until then the machine's catch-all bus
	// errors, so the OS RAM crawler never finds it and mis-sizes system
	// memory.  How the real cartridge keeps the crawler out is not known.
	if (!m_mpeg_ram_enabled)
	{
		if (++m_mpeg_ram_enable_cnt >= 64)
		{
			m_mpeg_ram_enabled = true;
			m_memory_space->install_readwrite_handler(0xe80000, 0xefffff,
					read16s_delegate(*this, FUNC(cdi_dvc_vmpeg_device::ram_r)),
					write16s_delegate(*this, FUNC(cdi_dvc_vmpeg_device::ram_w)));
			LOGMASKED(LOG_REGS_W, "DVC: decoder DRAM mapped\n");
		}
	}

	// VMPEG pixel clock select @ 0xe01xxx, lower bits are don't care
	if ((reg >> 8) == 0x08)
		return;

	// The transfer port the driver pushes stream data through by hand (host
	// play).  It answers in the video decoder's window but feeds whichever
	// decoder the transfer in progress belongs to, so the board owns it.
	if (reg == 0x206f)
	{
		if (m_transfer_for_fma)
		{
			m_fma->write_data(uint8_t(data >> 8));
			m_fma->write_data(uint8_t(data));
			m_fma->end_of_transfer(false);
		}
		else
		{
			m_fmv->write_data(uint8_t(data >> 8));
			m_fmv->write_data(uint8_t(data));
			m_fmv->end_of_transfer(false);
		}
		return;
	}

	if ((reg & 0xff00) == 0x1800)
		m_fma->regs_w(reg & 0xff, data, mem_mask);
	else if ((reg & 0xff00) == 0x2000)
		m_fmv->regs_w(reg & 0xff, data, mem_mask);
}

//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

device_cdi_dvc_interface::device_cdi_dvc_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "cdidvc")
	, m_slot(nullptr)
{
}

device_cdi_dvc_interface::~device_cdi_dvc_interface()
{
}

void device_cdi_dvc_interface::interface_pre_start()
{
	m_slot = downcast<cdi_dvc_slot_device *>(device().owner());
}

//**************************************************************************
//  CARTRIDGE SLOT
//**************************************************************************

cdi_dvc_slot_device::cdi_dvc_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CDI_DVC_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_cdi_dvc_interface>(mconfig, *this)
	, device_mixer_interface(mconfig, *this)
	, m_scc(*this, finder_base::DUMMY_TAG)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_intreq_cb(*this)
{
}

void cdi_dvc_slot_device::device_start()
{
	if (m_screen)
		m_screen->register_vblank_callback(vblank_state_delegate(&cdi_dvc_slot_device::vblank_callback, this));
}

uint8_t cdi_dvc_slot_device::intack_r()
{
	device_cdi_dvc_interface *const card = get_card_device();
	return card ? card->intack_r() : 0;
}

void cdi_dvc_slot_device::screen_vblank(int state)
{
	if (device_cdi_dvc_interface *const card = get_card_device())
		card->screen_vblank(state);
}

bool cdi_dvc_slot_device::ext_video_pixel(int x, int y, uint32_t &argb) const
{
	device_cdi_dvc_interface *const card = const_cast<cdi_dvc_slot_device *>(this)->get_card_device();
	return card && card->ext_video_pixel(x, y, argb);
}

void cdi_dvc_cards(device_slot_interface &device)
{
	device.option_add("vmpeg", CDI_DVC_VMPEG);
}
