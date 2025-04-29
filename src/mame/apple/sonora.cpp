// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "Sonora" system ASIC
    Emulation by R. Belmont

    Sonora contains the following:
    - A memory controller for up to 36MB (up to 4MB soldered and 32MB of SIMMs)
    - A VRAM controller and framebuffer controller, supporting monitor ID selection
    - A full VIA (VIA1) and a "pseudo-VIA", which is basically a combination GPIO and
      interrupt controller that looks somewhat like a VIA with no timers and no shift register.
    - A SWIM2 floppy controller
    - An EASC-like 4-channel audio controller
    - A 16/25/33 MHz CPU clock generator
    - Support logic for various external subsystems (ADB, PDS, SCC, SCSI, SONIC)

    The "Ardbeg" ASIC (LC 520) is a modest update of Sonora, adding support for pushbuttons
    controlling display brightness and sound volume, plus monitor power saver mode and PWM
    outputs for software control of the display brightness and contrast.

    Macintosh TV has a Sonora/Ardbeg derivative called "Tinker Bell" which adds video
    overlay support.

    Sonora was succeeded for 68040 machines by IOSB, PrimeTime, and PrimeTime II, with similar
    functionality but became pure I/O hubs with no memory controller or video.

    Sonora's video controller is in some of the PowerMac chipsets as well.
*/

#include "emu.h"
#include "sonora.h"

#include "formats/ap_dsk35.h"


static constexpr u32 C7M  = 7833600;
static constexpr u32 C15M = (C7M * 2);

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SONORA, sonora_device, "sonora", "Apple Sonora system ASIC")

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void sonora_device::map(address_map &map)
{
	map(0x00000000, 0x000fffff).r(FUNC(sonora_device::rom_switch_r)).mirror(0x0ff00000);

	map(0x10000000, 0x10001fff).rw(FUNC(sonora_device::mac_via_r), FUNC(sonora_device::mac_via_w)).mirror(0x00fc0000);
	map(0x10014000, 0x10015fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00f00000);
	map(0x10016000, 0x10017fff).rw(FUNC(sonora_device::swim_r), FUNC(sonora_device::swim_w)).mirror(0x00f00000);
	map(0x10026000, 0x10027fff).rw(m_pseudovia, FUNC(pseudovia_device::read), FUNC(pseudovia_device::write)).mirror(0x00f00000);
	map(0x10f24000, 0x10f24003).rw(m_video, FUNC(mac_video_sonora_device::dac_r), FUNC(mac_video_sonora_device::dac_w));
	map(0x10f28000, 0x10f28007).rw(m_video, FUNC(mac_video_sonora_device::vctrl_r), FUNC(mac_video_sonora_device::vctrl_w));

	map(0x20000000, 0x200fffff).ram().mirror(0x0ff00000).rw(FUNC(sonora_device::vram_r), FUNC(sonora_device::vram_w));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sonora_device::device_add_mconfig(machine_config &config)
{
	MAC_VIDEO_SONORA(config, m_video);
	m_video->screen_vblank().set(m_pseudovia, FUNC(pseudovia_device::slot_irq_w<0x40>));

	R65NC22(config, m_via1, C7M / 10);
	m_via1->readpa_handler().set(FUNC(sonora_device::via_in_a));
	m_via1->readpb_handler().set(FUNC(sonora_device::via_in_b));
	m_via1->writepa_handler().set(FUNC(sonora_device::via_out_a));
	m_via1->writepb_handler().set(FUNC(sonora_device::via_out_b));
	m_via1->cb2_handler().set(FUNC(sonora_device::via_out_cb2));
	m_via1->irq_handler().set(FUNC(sonora_device::via1_irq));

	APPLE_PSEUDOVIA(config, m_pseudovia, C15M);
	m_pseudovia->irq_callback().set(FUNC(sonora_device::via2_irq));

	ASC(config, m_asc, C15M, asc_device::asc_type::SONORA);
	m_asc->add_route(0, tag(), 1.0);
	m_asc->add_route(1, tag(), 1.0);
	m_asc->irqf_callback().set(m_pseudovia, FUNC(pseudovia_device::asc_irq_w));

	SWIM2(config, m_fdc, C15M);
	m_fdc->devsel_cb().set(FUNC(sonora_device::devsel_w));
	m_fdc->phases_cb().set(FUNC(sonora_device::phases_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);
}

//-------------------------------------------------
//  sonora_device - constructor
//-------------------------------------------------

sonora_device::sonora_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SONORA, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	write_pb4(*this),
	write_pb5(*this),
	write_cb2(*this),
	read_pb3(*this, 0),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_video(*this, "sonora_video"),
	m_via1(*this, "via1"),
	m_pseudovia(*this, "pseudovia"),
	m_asc(*this, "asc"),
	m_fdc(*this, "fdc"),
	m_floppy(*this, "fdc:%d", 0U),
	m_rom(*this, finder_base::DUMMY_TAG),
	m_cur_floppy(nullptr),
	m_hdsel(0),
	m_overlay(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sonora_device::device_start()
{
	m_vram = std::make_unique<u32[]>(0x100000 / sizeof(u32));

	m_stream = stream_alloc(8, 2, m_asc->clock(), STREAM_SYNCHRONOUS);

	m_6015_timer = timer_alloc(FUNC(sonora_device::mac_6015_tick), this);
	m_6015_timer->adjust(attotime::never);

	save_pointer(NAME(m_vram), 0x100000/sizeof(u32));
	save_item(NAME(m_via_interrupt));
	save_item(NAME(m_via2_interrupt));
	save_item(NAME(m_scc_interrupt));
	save_item(NAME(m_last_taken_interrupt));
	save_item(NAME(m_hdsel));

	m_rom_ptr = &m_rom[0];
	m_rom_size = m_rom.length() << 2;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sonora_device::device_reset()
{
	m_video->set_vram_base((const u64 *)&m_vram[0]);
	m_video->set_vram_offset(0);
	m_video->set_32bit();

	// start 60.15 Hz timer
	m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));

	m_via_interrupt = m_via2_interrupt = m_scc_interrupt = 0;
	m_last_taken_interrupt = -1;
	m_hdsel = 0;

	// main cpu shouldn't start until Egret wakes it up
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	m_overlay = true;

	// put ROM mirror at 0
	address_space &space = m_maincpu->space(AS_PROGRAM);
	const u32 memory_size = std::min((u32)0x3fffff, m_rom_size);
	const u32 memory_end = memory_size - 1;
	offs_t memory_mirror = memory_end & ~(memory_size - 1);

	space.unmap_write(0x00000000, memory_end);
	space.install_rom(0x00000000, memory_end & ~memory_mirror, memory_mirror, m_rom_ptr);
}

void sonora_device::sound_stream_update(sound_stream &stream)
{
	stream.copy(0, 0);
	stream.copy(1, 1);
}

u32 sonora_device::rom_switch_r(offs_t offset)
{
	// disable the overlay
	if (m_overlay && !machine().side_effects_disabled())
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		const u32 memory_end = m_ram_size - 1;
		void *memory_data = m_ram_ptr;
		offs_t memory_mirror = memory_end & ~memory_end;

		space.install_ram(0x00000000, memory_end & ~memory_mirror, memory_mirror, memory_data);
		m_overlay = false;
	}

	return m_rom_ptr[offset & ((m_rom_size - 1) >> 2)];
}

void sonora_device::set_ram_info(u32 *ram, u32 size)
{
	m_ram_ptr = ram;
	m_ram_size = size;
}

TIMER_CALLBACK_MEMBER(sonora_device::mac_6015_tick)
{
	m_via1->write_ca1(CLEAR_LINE);
	m_via1->write_ca1(ASSERT_LINE);
}

uint8_t sonora_device::via_in_a()
{
	return 0x80;
}

uint8_t sonora_device::via_in_b()
{
	return read_pb3() << 3;
}

void sonora_device::via_out_cb2(int state)
{
	write_cb2(state & 1);
}

void sonora_device::via_out_a(uint8_t data)
{
	int hdsel = BIT(data, 5);
	if (hdsel != m_hdsel)
	{
		if (m_cur_floppy)
		{
			m_cur_floppy->ss_w(hdsel);
		}
	}
	m_hdsel = hdsel;
}

void sonora_device::via_out_b(uint8_t data)
{
	write_pb4(BIT(data, 4));
	write_pb5(BIT(data, 5));
}

void sonora_device::via1_irq(int state)
{
	m_via_interrupt = state;
	field_interrupts();
}

void sonora_device::via2_irq(int state)
{
	m_via2_interrupt = state;
	field_interrupts();
}

void sonora_device::field_interrupts()
{
	int take_interrupt = -1;

	if (m_scc_interrupt)
	{
		take_interrupt = 4;
	}
	else if (m_via2_interrupt)
	{
		take_interrupt = 2;
	}
	else if (m_via_interrupt)
	{
		take_interrupt = 1;
	}

	if (m_last_taken_interrupt > -1)
	{
		m_maincpu->set_input_line(m_last_taken_interrupt, CLEAR_LINE);
		m_last_taken_interrupt = -1;
	}

	if (take_interrupt > -1)
	{
		m_maincpu->set_input_line(take_interrupt, ASSERT_LINE);
		m_last_taken_interrupt = take_interrupt;
	}
}

void sonora_device::scc_irq_w(int state)
{
	m_scc_interrupt = (state == ASSERT_LINE);
	field_interrupts();
}

void sonora_device::slot0_irq_w(int state)
{
	m_pseudovia->slot_irq_w<0x08>(state);
}

void sonora_device::slot1_irq_w(int state)
{
	m_pseudovia->slot_irq_w<0x10>(state);
}

void sonora_device::slot2_irq_w(int state)
{
	m_pseudovia->slot_irq_w<0x20>(state);
}

void sonora_device::cb1_w(int state)
{
	m_via1->write_cb1(state);
}

void sonora_device::cb2_w(int state)
{
	m_via1->write_cb2(state);
}

uint16_t sonora_device::mac_via_r(offs_t offset)
{
	uint16_t data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		via_sync();

	data = m_via1->read(offset);

	return (data & 0xff) | (data << 8);
}

void sonora_device::mac_via_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	via_sync();

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);
}

void sonora_device::via_sync()
{
	// The via runs at 783.36KHz while the main cpu runs at 15MHz or
	// more, so we need to sync the access with the via clock.  Plus
	// the whole access takes half a (via) cycle and ends when synced
	// with the main cpu again.

	// Get the main cpu time
	u64 cycle = m_maincpu->total_cycles();

	// Get the number of the cycle the via is in at that time
	u64 via_cycle = cycle * m_via1->clock() / m_maincpu->clock();

	// The access is going to start at via_cycle+1 and end at
	// via_cycle+1.5, compute what that means in maincpu cycles (the
	// +1 rounds up, since the clocks are too different to ever be
	// synced).
	u64 main_cycle = (via_cycle * 2 + 3) * m_maincpu->clock() / (2 * m_via1->clock()) + 1;

	// Finally adjust the main cpu icount as needed.
	m_maincpu->adjust_icount(-int(main_cycle - cycle));
}

uint16_t sonora_device::swim_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled())
	{
		m_maincpu->adjust_icount(-5);
	}

	u16 result = m_fdc->read((offset >> 8) & 0xf);
	return result << 8;
}
void sonora_device::swim_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_fdc->write((offset >> 8) & 0xf, data & 0xff);
	else
		m_fdc->write((offset >> 8) & 0xf, data >> 8);
}

void sonora_device::phases_w(uint8_t phases)
{
	if (m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void sonora_device::devsel_w(uint8_t devsel)
{
	if (devsel == 1)
		m_cur_floppy = m_floppy[0]->get_device();
	else if (devsel == 2)
		m_cur_floppy = m_floppy[1]->get_device();
	else
		m_cur_floppy = nullptr;

	m_fdc->set_floppy(m_cur_floppy);
	if (m_cur_floppy)
		m_cur_floppy->ss_w(m_hdsel);
}

u32 sonora_device::vram_r(offs_t offset)
{
	return m_vram[offset];
}

void sonora_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
}

