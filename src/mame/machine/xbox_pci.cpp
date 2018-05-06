// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli

#include "emu.h"
#include "includes/xbox.h"
#include "includes/xbox_pci.h"

#include <functional>

//#define LOG_AUDIO

/*
 * Host
 */

DEFINE_DEVICE_TYPE(NV2A_HOST, nv2a_host_device, "nv2a_host", "NV2A PCI Bridge Device - Host Bridge")

nv2a_host_device::nv2a_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, NV2A_HOST, tag, owner, clock),
	  cpu_tag(nullptr),
	  cpu(nullptr)
{
}

void nv2a_host_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	io_space->install_device(0, 0xffff, *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);
}

void nv2a_host_device::set_cpu_tag(const char *_cpu_tag)
{
	cpu_tag = _cpu_tag;
}

void nv2a_host_device::device_start()
{
	pci_host_device::device_start();
	cpu = machine().device<cpu_device>(cpu_tag);
	memory_space = &cpu->space(AS_PROGRAM);
	io_space = &cpu->space(AS_IO);

	// do not change the next two
	memory_window_start = 0x10000000;
	memory_window_end = 0xfeefffff;
	memory_offset = 0;
	// do not change the next two
	io_window_start = 0x5000;
	io_window_end = 0xefff;
	io_offset = 0;
}

void nv2a_host_device::device_reset()
{
	pci_host_device::device_reset();
}

/*
 * Ram
 */

DEFINE_DEVICE_TYPE(NV2A_RAM, nv2a_ram_device, "nv2a_ram", "NV2A Memory Controller - SDRAM")

void nv2a_ram_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x6c, 0x6f).rw(this, FUNC(nv2a_ram_device::config_register_r), FUNC(nv2a_ram_device::config_register_w));
}

nv2a_ram_device::nv2a_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, NV2A_RAM, tag, owner, clock)
{
}

READ32_MEMBER(nv2a_ram_device::config_register_r)
{
	return 0x08800044;
}

WRITE32_MEMBER(nv2a_ram_device::config_register_w)
{
}

/*
 * LPC Bus
 */

DEFINE_DEVICE_TYPE(MCPX_LPC, mcpx_lpc_device, "mcpx_lpc", "MCPX HUB Interface - ISA Bridge")

void mcpx_lpc_device::lpc_io(address_map &map)
{
	map(0x00000000, 0x000000ff).rw(this, FUNC(mcpx_lpc_device::lpc_r), FUNC(mcpx_lpc_device::lpc_w));
}

mcpx_lpc_device::mcpx_lpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MCPX_LPC, tag, owner, clock)
{
}

void mcpx_lpc_device::device_start()
{
	pci_device::device_start();
	add_map(0x00000100, M_IO, FUNC(mcpx_lpc_device::lpc_io));
	bank_infos[0].adr = 0x8000;
}

void mcpx_lpc_device::device_reset()
{
	pci_device::device_reset();
}

READ32_MEMBER(mcpx_lpc_device::lpc_r)
{
	return 0;
}

WRITE32_MEMBER(mcpx_lpc_device::lpc_w)
{
}

/*
 * SMBus
 */

DEFINE_DEVICE_TYPE(MCPX_SMBUS, mcpx_smbus_device, "mcpx_smbus", "MCPX SMBus Controller")

void mcpx_smbus_device::smbus_io0(address_map &map)
{
	map(0x00000000, 0x0000000f).noprw();
}

void mcpx_smbus_device::smbus_io1(address_map &map)
{
	map(0x00000000, 0x0000000f).rw(this, FUNC(mcpx_smbus_device::smbus_r), FUNC(mcpx_smbus_device::smbus_w));
}

void mcpx_smbus_device::smbus_io2(address_map &map)
{
	map(0x00000000, 0x0000001f).noprw();
}

mcpx_smbus_device::mcpx_smbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MCPX_SMBUS, tag, owner, clock),
	m_interrupt_handler(*this)
{
}

void mcpx_smbus_device::device_start()
{
	pci_device::device_start();
	m_interrupt_handler.resolve_safe();
	add_map(0x00000010, M_IO, FUNC(mcpx_smbus_device::smbus_io0));
	bank_infos[0].adr = 0x1000;
	add_map(0x00000010, M_IO, FUNC(mcpx_smbus_device::smbus_io1));
	bank_infos[1].adr = 0xc000;
	add_map(0x00000020, M_IO, FUNC(mcpx_smbus_device::smbus_io2));
	bank_infos[2].adr = 0xc200;
	memset(&smbusst, 0, sizeof(smbusst));
}

void mcpx_smbus_device::device_reset()
{
	pci_device::device_reset();
}

READ32_MEMBER(mcpx_smbus_device::smbus_r)
{
	if ((offset == 0) && (mem_mask == 0xff)) // 0 smbus status
		smbusst.words[offset] = (smbusst.words[offset] & ~mem_mask) | ((smbusst.status << 0) & mem_mask);
	if ((offset == 1) && ((mem_mask == 0x00ff0000) || (mem_mask == 0xffff0000))) // 6 smbus data
		smbusst.words[offset] = (smbusst.words[offset] & ~mem_mask) | ((smbusst.data << 16) & mem_mask);
	return smbusst.words[offset];
}

WRITE32_MEMBER(mcpx_smbus_device::smbus_w)
{
	COMBINE_DATA(smbusst.words);
	if ((offset == 0) && (mem_mask == 0xff)) // 0 smbus status
	{
		if (!((smbusst.status ^ data) & 0x10)) // clearing interrupt
		{
			m_interrupt_handler(0);
		}
		smbusst.status &= ~data;
	}
	if ((offset == 0) && (mem_mask == 0xff0000)) // 2 smbus control
	{
		data = data >> 16;
		smbusst.control = data;
		int cycletype = smbusst.control & 7;
		if (smbusst.control & 8) { // start
			if ((cycletype & 6) == 2)
			{
				if (smbusst.devices[smbusst.address])
					if (smbusst.rw == 0)
						smbusst.devices[smbusst.address](smbusst.command, smbusst.rw, smbusst.data);
					else
						smbusst.data = smbusst.devices[smbusst.address](smbusst.command, smbusst.rw, smbusst.data);
				else
					logerror("SMBUS: access to missing device at address %d\n", smbusst.address);
				smbusst.status |= 0x10;
				if (smbusst.control & 0x10)
				{
					m_interrupt_handler(1);
				}
			}
		}
	}
	if ((offset == 1) && (mem_mask == 0xff)) // 4 smbus address
	{
		smbusst.address = data >> 1;
		smbusst.rw = data & 1;
	}
	if ((offset == 1) && ((mem_mask == 0x00ff0000) || (mem_mask == 0xffff0000))) // 6 smbus data
	{
		data = data >> 16;
		smbusst.data = data;
	}
	if ((offset == 2) && (mem_mask == 0xff)) // 8 smbus command
		smbusst.command = data;
}

/*
 * OHCI USB Controller
 */

DEFINE_DEVICE_TYPE(MCPX_OHCI, mcpx_ohci_device, "mcpx_ohci", "MCPX OHCI USB Controller")

void mcpx_ohci_device::ohci_mmio(address_map &map)
{
	map(0x00000000, 0x00000fff).rw(this, FUNC(mcpx_ohci_device::ohci_r), FUNC(mcpx_ohci_device::ohci_w));
}

mcpx_ohci_device::mcpx_ohci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MCPX_OHCI, tag, owner, clock),
	ohci_usb(nullptr),
	m_interrupt_handler(*this),
	timer(nullptr),
	connecteds_count(0)
{
}

void mcpx_ohci_device::plug_usb_device(int port, ohci_function *function)
{
	function->set_bus_manager(ohci_usb);
	ohci_usb->usb_ohci_plug(port, function);
}

void mcpx_ohci_device::device_start()
{
	pci_device::device_start();
	m_interrupt_handler.resolve_safe();
	add_map(0x00001000, M_MEM, FUNC(mcpx_ohci_device::ohci_mmio));
	bank_infos[0].adr = 0xfed00000;
	ohci_usb = new ohci_usb_controller();
	ohci_usb->set_cpu(machine().device<cpu_device>("maincpu"));
	ohci_usb->set_irq_callbaclk(
		[&](int state)
		{
			m_interrupt_handler(state);
		}
	);
	timer = timer_alloc(0);
	ohci_usb->set_timer(timer);
	ohci_usb->start();
	for (int i=0;i < connecteds_count;i++)
		plug_usb_device(connecteds[i].port, connecteds[i].dev);
}

void mcpx_ohci_device::device_reset()
{
	pci_device::device_reset();
	if (ohci_usb)
		ohci_usb->reset();
}

void mcpx_ohci_device::device_config_complete()
{
	char id[8];

	for (int i = 1; i<=4; i++)
	{
		sprintf(id, "port%d", i);
		ohci_usb_connector *conn = downcast<ohci_usb_connector *>(subdevice(id));
		if (conn)
		{
			ohci_function *func = conn->get_device();
			if (func)
			{
				connecteds[connecteds_count].dev = func;
				connecteds[connecteds_count].port = i;
				connecteds_count++;
			}
		}
	}
}

void mcpx_ohci_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (ohci_usb)
		ohci_usb->timer(timer, id, param, ptr);
}

READ32_MEMBER(mcpx_ohci_device::ohci_r)
{
	if (!ohci_usb)
		return 0;
	if (offset == 0) // hacks needed until usb (and jvs) is implemented
	{
		hack_callback();
	}
	return ohci_usb->read(space, offset, mem_mask);
}

WRITE32_MEMBER(mcpx_ohci_device::ohci_w)
{
	if (ohci_usb)
		ohci_usb->write(space, offset, data, mem_mask);
}

/*
 * Ethernet
 */

DEFINE_DEVICE_TYPE(MCPX_ETH, mcpx_eth_device, "mcpx_eth", "MCP Networking Adapter")

void mcpx_eth_device::eth_mmio(address_map &map)
{
	map(0x00000000, 0x0000003ff).rw(this, FUNC(mcpx_eth_device::eth_r), FUNC(mcpx_eth_device::eth_w));
}

void mcpx_eth_device::eth_io(address_map &map)
{
	map(0x00000000, 0x000000007).rw(this, FUNC(mcpx_eth_device::eth_io_r), FUNC(mcpx_eth_device::eth_io_w));
}

mcpx_eth_device::mcpx_eth_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MCPX_ETH, tag, owner, clock)
{
}

void mcpx_eth_device::device_start()
{
	pci_device::device_start();
	add_map(0x00001000, M_MEM, FUNC(mcpx_eth_device::eth_mmio));
	bank_infos[0].adr = 0xfef00000;
	add_map(0x00000100, M_IO, FUNC(mcpx_eth_device::eth_io));
	bank_infos[1].adr = 0xe000;
}

void mcpx_eth_device::device_reset()
{
	pci_device::device_reset();
}

READ32_MEMBER(mcpx_eth_device::eth_r)
{
	return 0;
}

WRITE32_MEMBER(mcpx_eth_device::eth_w)
{
}

READ32_MEMBER(mcpx_eth_device::eth_io_r)
{
	return 0;
}

WRITE32_MEMBER(mcpx_eth_device::eth_io_w)
{
}

/*
 * Audio Processing Unit
 */

DEFINE_DEVICE_TYPE(MCPX_APU, mcpx_apu_device, "mcpx_apu", "MCP APU")

void mcpx_apu_device::apu_mmio(address_map &map)
{
	map(0x00000000, 0x00007ffff).rw(this, FUNC(mcpx_apu_device::apu_r), FUNC(mcpx_apu_device::apu_w));
}

mcpx_apu_device::mcpx_apu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MCPX_APU, tag, owner, clock),
	cpu_tag(nullptr),
	cpu(nullptr)
{
}

void mcpx_apu_device::set_cpu_tag(const char *_cpu_tag)
{
	cpu_tag = _cpu_tag;
}

void mcpx_apu_device::device_start()
{
	pci_device::device_start();
	add_map(0x00080000, M_MEM, FUNC(mcpx_apu_device::apu_mmio));
	bank_infos[0].adr = 0xfe800000;
	memset(apust.memory, 0, sizeof(apust.memory));
	memset(apust.voices_heap_blockaddr, 0, sizeof(apust.voices_heap_blockaddr));
	memset(apust.voices_active, 0, sizeof(apust.voices_active));
	memset(apust.voices_position, 0, sizeof(apust.voices_position));
	memset(apust.voices_position_start, 0, sizeof(apust.voices_position_start));
	memset(apust.voices_position_end, 0, sizeof(apust.voices_position_end));
	memset(apust.voices_position_increment, 0, sizeof(apust.voices_position_increment));
	cpu = machine().device<cpu_device>(cpu_tag);
	apust.space = &cpu->space();
	apust.timer = timer_alloc(0);
	apust.timer->enable(false);
}

void mcpx_apu_device::device_reset()
{
	pci_device::device_reset();
}

void mcpx_apu_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int cmd;
	int bb, b, v;
	uint64_t bv;
	uint32_t phys;

	// this works only for outr2
	// value at 0x810 is modified by the firmware that has been loaded on the gp dsp
	cmd = apust.space->read_dword(apust.gpdsp_address + 0x800 + 0x10);
	if (cmd == 3)
		apust.space->write_dword(apust.gpdsp_address + 0x800 + 0x10, 0);
	/*else
	logerror("Audio_APU: unexpected value at address %d\n",apust.gpdsp_address+0x800+0x10);*/
	// check all the 256 possible voices
	for (b = 0; b < 4; b++) {
		bv = 1;
		for (bb = 0; bb < 64; bb++) {
			if (apust.voices_active[b] & bv) {
				v = bb + (b << 6);
				apust.voices_position[v] += apust.voices_position_increment[v];
				while (apust.voices_position[v] >= apust.voices_position_end[v])
					apust.voices_position[v] = apust.voices_position_start[v] + apust.voices_position[v] - apust.voices_position_end[v] - 1000;
				phys = apust.voicedata_address + 0x80 * v;
				apust.space->write_dword(phys + 0x58, apust.voices_position[v] / 1000);
			}
			bv = bv << 1;
		}
	}
}

READ32_MEMBER(mcpx_apu_device::apu_r)
{
#ifdef LOG_AUDIO
	logerror("Audio_APU: read from %08X mask %08X\n", 0xfe800000 + offset * 4, mem_mask);
#endif
	if (offset == 0x20010 / 4) // some kind of internal counter or state value
		return 0x20 + 4 + 8 + 0x48 + 0x80;
	return apust.memory[offset];
}

WRITE32_MEMBER(mcpx_apu_device::apu_w)
{
	uint32_t v;

#ifdef LOG_AUDIO
	logerror("Audio_APU: write at %08X mask %08X value %08X\n", 0xfe800000 + offset * 4, mem_mask, data);
#endif
	apust.memory[offset] = data;
	if (offset == 0x02040 / 4) // address of memory area with scatter-gather info (gpdsp scratch dma)
		apust.gpdsp_sgaddress = data;
	if (offset == 0x020d4 / 4) { // block count (gpdsp)
		apust.gpdsp_sgblocks = data;
		apust.gpdsp_address = apust.space->read_dword(apust.gpdsp_sgaddress); // memory address of first block
		apust.timer->enable();
		apust.timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
	}
	if (offset == 0x02048 / 4) // (epdsp scratch dma)
		apust.epdsp_sgaddress = data;
	if (offset == 0x020dc / 4) // (epdsp)
		apust.epdsp_sgblocks = data;
	if (offset == 0x0204c / 4) // address of memory area with information about blocks
		apust.epdsp_sgaddress2 = data;
	if (offset == 0x020e0 / 4) // block count - 1
		apust.epdsp_sgblocks2 = data;
	if (offset == 0x0202c / 4) { // address of memory area with 0x80 bytes for each voice
		apust.voicedata_address = data;
		return;
	}
	if (offset == 0x04024 / 4) // offset in memory area indicated by 0x204c (analog output ?)
		return;
	if (offset == 0x04034 / 4) // size
		return;
	if (offset == 0x04028 / 4) // offset in memory area indicated by 0x204c (digital output ?)
		return;
	if (offset == 0x04038 / 4) // size
		return;
	if (offset == 0x20804 / 4) { // block number for scatter-gather heap that stores sampled audio to be played
		if (data >= 1024) {
			logerror("Audio_APU: sg block number too high, increase size of voices_heap_blockaddr\n");
			apust.memory[offset] = 1023;
		}
		return;
	}
	if (offset == 0x20808 / 4) { // block address for scatter-gather heap that stores sampled audio to be played
		apust.voices_heap_blockaddr[apust.memory[0x20804 / 4]] = data;
		return;
	}
	if (offset == 0x202f8 / 4) { // voice number for parameters ?
		apust.voice_number = data;
		return;
	}
	if (offset == 0x202fc / 4) // 1 when accessing voice parameters 0 otherwise
		return;
	if (offset == 0x20304 / 4) { // format
		 /*
		 bits 28-31 sample format:
		 0  8-bit pcm
		 5  16-bit pcm
		 10 adpcm ?
		 14 24-bit pcm
		 15 32-bit pcm
		 bits 16-20 number of channels - 1:
		 0  mono
		 1  stereo
		 */
		return;
	}
	if (offset == 0x2037c / 4) { // value related to sample rate
		int16_t v0 = (int16_t)(data >> 16); // upper 16 bits as a signed 16 bit value
		float vv = ((float)v0) / 4096.0f; // divide by 4096
		float vvv = powf(2, vv); // two to the vv
		int f = vvv*48000.0f; // sample rate
		apust.voices_frequency[apust.voice_number] = f;
		return;
	}
	if (offset == 0x203a0 / 4) // start offset of data in scatter-gather heap
		return;
	if (offset == 0x203a4 / 4) { // first sample to play
		apust.voices_position_start[apust.voice_number] = data * 1000;
		return;
	}
	if (offset == 0x203dc / 4) { // last sample to play
		apust.voices_position_end[apust.voice_number] = data * 1000;
		return;
	}
	if (offset == 0x2010c / 4) // voice processor 0 idle 1 not idle ?
		return;
	if (offset == 0x20124 / 4) { // voice number to activate ?
		v = apust.voice_number;
		apust.voices_active[v >> 6] |= ((uint64_t)1 << (v & 63));
		apust.voices_position[v] = apust.voices_position_start[apust.voice_number];
		apust.voices_position_increment[apust.voice_number] = apust.voices_frequency[apust.voice_number];
		return;
	}
	if (offset == 0x20128 / 4) { // voice number to deactivate ?
		v = apust.voice_number;
		apust.voices_active[v >> 6] &= ~(1 << (v & 63));
		return;
	}
	if (offset == 0x20140 / 4) // voice number to ?
		return;
	if ((offset >= 0x20200 / 4) && (offset < 0x20280 / 4)) // headroom for each of the 32 mixbins
		return;
	if (offset == 0x20280 / 4) // hrtf headroom ?
		return;
}

/*
 * AC97 Audio Controller
 */

DEFINE_DEVICE_TYPE(MCPX_AC97_AUDIO, mcpx_ac97_audio_device, "mcpx_ac97_audio", "MCPX AC'97 Audio Codec Interface")

void mcpx_ac97_audio_device::ac97_mmio(address_map &map)
{
	map(0x00000000, 0x000000fff).rw(this, FUNC(mcpx_ac97_audio_device::ac97_audio_r), FUNC(mcpx_ac97_audio_device::ac97_audio_w));
}

void mcpx_ac97_audio_device::ac97_io0(address_map &map)
{
	map(0x00000000, 0x0000000ff).rw(this, FUNC(mcpx_ac97_audio_device::ac97_audio_io0_r), FUNC(mcpx_ac97_audio_device::ac97_audio_io0_w));
}

void mcpx_ac97_audio_device::ac97_io1(address_map &map)
{
	map(0x00000000, 0x00000007f).rw(this, FUNC(mcpx_ac97_audio_device::ac97_audio_io1_r), FUNC(mcpx_ac97_audio_device::ac97_audio_io1_w));
}

mcpx_ac97_audio_device::mcpx_ac97_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MCPX_AC97_AUDIO, tag, owner, clock)
{
}

void mcpx_ac97_audio_device::device_start()
{
	pci_device::device_start();
	add_map(0x00000100, M_IO, FUNC(mcpx_ac97_audio_device::ac97_io0));
	bank_infos[0].adr = 0xd000;
	add_map(0x00000080, M_IO, FUNC(mcpx_ac97_audio_device::ac97_io1));
	bank_infos[1].adr = 0xd200;
	add_map(0x00001000, M_MEM, FUNC(mcpx_ac97_audio_device::ac97_mmio));
	bank_infos[2].adr = 0xfec00000;
	memset(&ac97st, 0, sizeof(ac97st));
}

void mcpx_ac97_audio_device::device_reset()
{
	pci_device::device_reset();
}

READ32_MEMBER(mcpx_ac97_audio_device::ac97_audio_r)
{
	uint32_t ret = 0;

#ifdef LOG_AUDIO
	logerror("Audio_AC3: read from %08X mask %08X\n", 0xfec00000 + offset * 4, mem_mask);
#endif
	if (offset < 0x80 / 4)
	{
		ret = ac97st.mixer_regs[offset];
	}
	if ((offset >= 0x100 / 4) && (offset <= 0x138 / 4))
	{
		offset = offset - 0x100 / 4;
		if (offset == 0x18 / 4)
		{
			ac97st.controller_regs[offset] &= ~0x02000000; // REGRST: register reset
		}
		if (offset == 0x30 / 4)
		{
			ac97st.controller_regs[offset] |= 0x100; // PCRDY: primary codec ready
		}
		if (offset == 0x34 / 4)
		{
			ac97st.controller_regs[offset] &= ~1; // CAS: codec access semaphore
		}
		ret = ac97st.controller_regs[offset];
	}
	return ret;
}

WRITE32_MEMBER(mcpx_ac97_audio_device::ac97_audio_w)
{
#ifdef LOG_AUDIO
	logerror("Audio_AC3: write at %08X mask %08X value %08X\n", 0xfec00000 + offset * 4, mem_mask, data);
#endif
	if (offset < 0x80 / 4)
	{
		COMBINE_DATA(ac97st.mixer_regs + offset);
	}
	if ((offset >= 0x100 / 4) && (offset < 0x13c / 4))
	{
		offset = offset - 0x100 / 4;
		COMBINE_DATA(ac97st.controller_regs + offset);
	}
}

READ32_MEMBER(mcpx_ac97_audio_device::ac97_audio_io0_r)
{
	return 0;
}

WRITE32_MEMBER(mcpx_ac97_audio_device::ac97_audio_io0_w)
{
}

READ32_MEMBER(mcpx_ac97_audio_device::ac97_audio_io1_r)
{
	return 0;
}

WRITE32_MEMBER(mcpx_ac97_audio_device::ac97_audio_io1_w)
{
}

/*
 * AC97 Modem Controller
 */

DEFINE_DEVICE_TYPE(MCPX_AC97_MODEM, mcpx_ac97_modem_device, "mcpx_ac97_modem", "MCPX AC'97 Modem Controller")

mcpx_ac97_modem_device::mcpx_ac97_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MCPX_AC97_MODEM, tag, owner, clock)
{
}

/*
 * IDE Controller
 */

DEFINE_DEVICE_TYPE(MCPX_IDE, mcpx_ide_device, "mcpx_ide", "MCPX IDE Controller")

void mcpx_ide_device::mcpx_ide_io(address_map &map)
{
	map(0x0000, 0x000f).rw("ide", FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w));
}

mcpx_ide_device::mcpx_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MCPX_IDE, tag, owner, clock),
	m_interrupt_handler(*this)
{
}

void mcpx_ide_device::device_start()
{
	pci_device::device_start();
	add_map(0x00000010, M_IO, FUNC(mcpx_ide_device::mcpx_ide_io));
	bank_infos[0].adr = 0xff60;
	m_interrupt_handler.resolve_safe();
}

void mcpx_ide_device::device_reset()
{
	pci_device::device_reset();
}

MACHINE_CONFIG_START(mcpx_ide_device::device_add_mconfig)
	MCFG_DEVICE_ADD("ide", BUS_MASTER_IDE_CONTROLLER, 0)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(*this, mcpx_ide_device, ide_interrupt))
	MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE("maincpu", AS_PROGRAM)
MACHINE_CONFIG_END

WRITE_LINE_MEMBER(mcpx_ide_device::ide_interrupt)
{
	m_interrupt_handler(state);
}

/*
 * AGP Bridge
 */

DEFINE_DEVICE_TYPE(NV2A_AGP, nv2a_agp_device, "nv2a_agp", "NV2A AGP Host to PCI Bridge")

nv2a_agp_device::nv2a_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: agp_bridge_device(mconfig, NV2A_AGP, tag, owner, clock)
{
}

void nv2a_agp_device::device_start()
{
	agp_bridge_device::device_start();
}

void nv2a_agp_device::device_reset()
{
	agp_bridge_device::device_reset();
}

/*
 * NV2A 3D Accelerator
 */

DEFINE_DEVICE_TYPE(NV2A_GPU, nv2a_gpu_device, "nv2a_gpu", "NVIDIA NV2A GPU")

void nv2a_gpu_device::nv2a_mmio(address_map &map)
{
	map(0x00000000, 0x00ffffff).ram().rw(this, FUNC(nv2a_gpu_device::geforce_r), FUNC(nv2a_gpu_device::geforce_w));
}

void nv2a_gpu_device::nv2a_mirror(address_map &map)
{
	map(0x00000000, 0x07ffffff).ram().rw(this, FUNC(nv2a_gpu_device::nv2a_mirror_r), FUNC(nv2a_gpu_device::nv2a_mirror_w));
}

nv2a_gpu_device::nv2a_gpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, NV2A_GPU, tag, owner, clock),
	nvidia_nv2a(nullptr),
	cpu_tag(nullptr),
	m_interrupt_handler(*this),
	m_program(nullptr)
{
}

void nv2a_gpu_device::set_cpu_tag(const char *_cpu_tag)
{
	cpu_tag = _cpu_tag;
}

void nv2a_gpu_device::device_start()
{
	pci_device::device_start();
	m_interrupt_handler.resolve_safe();
	add_map(0x01000000, M_MEM, FUNC(nv2a_gpu_device::nv2a_mmio));
	bank_infos[0].adr = 0xfd000000;
	add_map(0x08000000, M_MEM, FUNC(nv2a_gpu_device::nv2a_mirror));
	bank_infos[1].adr = 0xf0000000;
	m_program = &machine().device<cpu_device>(cpu_tag)->space();
	nvidia_nv2a = new nv2a_renderer(machine());
	nvidia_nv2a->set_irq_callbaclk(
		[&](int state)
		{
			m_interrupt_handler(state);
		}
	);
	nvidia_nv2a->start(m_program);
	nvidia_nv2a->savestate_items();
}

void nv2a_gpu_device::device_reset()
{
	pci_device::device_reset();
}

READ32_MEMBER(nv2a_gpu_device::geforce_r)
{
	return nvidia_nv2a->geforce_r(space, offset, mem_mask);
}

WRITE32_MEMBER(nv2a_gpu_device::geforce_w)
{
	nvidia_nv2a->geforce_w(space, offset, data, mem_mask);
}

READ32_MEMBER(nv2a_gpu_device::nv2a_mirror_r)
{
	return m_program->read_dword(offset << 2);
}

WRITE32_MEMBER(nv2a_gpu_device::nv2a_mirror_w)
{
	m_program->write_dword(offset << 2, data, mem_mask);
}
