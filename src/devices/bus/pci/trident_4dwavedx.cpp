// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Trident 4DWAVE-DX / 4DWAVE-NX

AC'97 v1.x (fixed 48 kHz)

TODO:
- dxdiag: sound test don't playback on lower modes;
- diablo: ambient BGM doesn't work, SFXs don't playback from time to time;
- Missing features in Bank A (testable in dxdiag -> Music -> Trident PCI WaveTable MIDI);
- Move DMA reading out of sound_stream_update;
- Mix-in wave engine output to AC'97 input;
- wavetsr.com can't find a free IRQ for SB emulation under DOS;
- Soundblaster, FM and MPU-401 are emulated by the 4dwave sound engine;
- Support for 4DWAVE-NX (minor upgrade with extra TLB, scatter-gather and SPDIF interface)

Notes:
- 66 MHz CPU is too slow for winamp, will hiccup a ton on "serious" playback;

**************************************************************************************************/

#include "emu.h"
#include "trident_4dwavedx.h"

#include "speaker.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

#define LIVE_AUDIO_VIEW 0


DEFINE_DEVICE_TYPE(TRIDENT_4DWAVEDX, trident_4dwavedx_device,   "trident_4dwavedx",   "Trident 4D Wave-DX sound card")
DEFINE_DEVICE_TYPE(T4DWAVE_PCM, t4dwave_pcm_device,   "t4dwave_pcm",   "Trident 4D Wave-DX PCM sound engine")


trident_4dwavedx_device::trident_4dwavedx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_pcm(*this, "pcm")
	, m_ac97(*this, "ac97")
	, m_joy(*this, "pc_joy")
{
}

trident_4dwavedx_device::trident_4dwavedx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: trident_4dwavedx_device(mconfig, TRIDENT_4DWAVEDX, tag, owner, clock)
{
	set_ids(0x10232000, 0x00, 0x040100, 0x10232000);
}

void trident_4dwavedx_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();

	T4DWAVE_PCM(config, m_pcm, 0);
	m_pcm->datain_cb().set([this] (offs_t offset) {
		address_space &dma_space = *get_pci_busmaster_space();
		return dma_space.read_dword(offset);
	});
	m_pcm->irq_cb().set([this] (int state) {
		irq_pin_w(0, state);
		//irq_pin_w(0, 0);
		//if (state)
		//	irq_pin_w(0, 1);
	});
	m_pcm->add_route(0, m_ac97, 1.00, 0);
	m_pcm->add_route(1, m_ac97, 1.00, 1);

	// Trident suggests to use an AD1819A
	// known actual configs:
	// '9700 for Hoontech ST-DIGITAL 4D_NX/SoundTrack 4D Wave (with the -NX variant)
	// '9704 for Addonics SV750/SIIG IC1607/GoodWell EPC-C4DWV840
	AC97_STAC9704(config, m_ac97, 12'288'000);
	m_ac97->set_pcm_tag(m_pcm);
	m_ac97->add_route(0, "speaker", 1.0, 0);
	m_ac97->add_route(1, "speaker", 1.0, 1);

	PC_JOY(config, m_joy);
}

void trident_4dwavedx_device::device_start()
{
	pci_card_device::device_start();

	add_map( 256,    M_IO,  FUNC(trident_4dwavedx_device::io_map));
	add_map( 4*1024, M_MEM, FUNC(trident_4dwavedx_device::mmio_map));

	// INTA#
	intr_pin = 1;
	intr_line = 0x05;

	// min_gnt = 0.5 usec, max_lat = 1.25 usec
	minimum_grant = 0x02;
	maximum_latency = 0x05;

	save_item(NAME(m_ddma_config));
	save_item(NAME(m_legacy_control));
	save_item(NAME(m_power_state));
	save_item(STRUCT_MEMBER(m_interrupt_snoop, enable));
	save_item(STRUCT_MEMBER(m_interrupt_snoop, vector));

	save_item(NAME(m_asr3));
	save_item(NAME(m_asr4));
	save_item(NAME(m_asr5));
	save_item(NAME(m_asr6));
}

void trident_4dwavedx_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0000;
	command_mask = 7;
	// Medium DEVSEL#, support cap list
	status = 0x0210;

	m_ddma_config = m_legacy_control = 0;
	m_power_state = 0;
	m_interrupt_snoop.enable = false;
	m_interrupt_snoop.vector = 0;

	m_asr3 = 0;
	m_asr4 = 0;
	m_asr5 = 0x04;
	m_asr6 = 0x02;

	remap_cb();
}

u8 trident_4dwavedx_device::capptr_r()
{
	return 0x48;
}

void trident_4dwavedx_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);

	// DDMA config
	map(0x40, 0x43).lrw32(
		NAME([this] (offs_t offset, u32 mem_mask) {
			LOG("PCI 40h: DDMA config read (mask %08x)\n", mem_mask);
			return m_ddma_config;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_ddma_config);
			// knock off Legacy DMA Transfer Size Control
			m_ddma_config &= ~(3 << 1);
			LOG("PCI 40h: %08x & %08x\n", data, mem_mask);
			LOG("\tDDMA config write: address %08x Extended Address %d Slave Channel Access %d\n"
				, m_ddma_config & ~0xf, BIT(m_ddma_config, 3), BIT(m_ddma_config, 0));
		})
	);
//	map(0x44, 0x44) Legacy I/O Base
//	map(0x45, 0x45) Legacy DMA
//	map(0x46, 0x46) Legacy Control
	map(0x44, 0x47).lrw32(
		NAME([this] (offs_t offset, u32 mem_mask) {
			LOG("PCI 44h: Legacy control read (mask %08x)\n", mem_mask);
			return m_legacy_control;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_legacy_control);
			m_legacy_control &= 0x0006'07ff;
			LOG("PCI 44h: %08x & %08x\n", data, mem_mask);
			LOG("\tLegacy I/O base: %02x Legacy DMA: %02x Audio Engine Reset: %d Writable subsystem %d\n"
				, m_legacy_control & 0xff
				, (m_legacy_control >> 8) & 7
				, BIT(m_legacy_control, 18)
				, BIT(m_legacy_control, 17)
			);
			if (ACCESSING_BITS_0_7)
				remap_cb();
		})
	);

	// Power Management v1.0, D2 and D1 support, no PME#
	map(0x48, 0x4b).lr32(NAME([] () { return 0x0601'0001; }));
	map(0x4c, 0x4f).lrw32(
		NAME([this] (offs_t offset) { return m_power_state; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_7)
			{
				LOG("PM Power State D%d\n", data & 3);
				m_power_state = data & 3;
			}
		})
	);

	map(0x50, 0x53).lrw32(
		NAME([this] (offs_t offset, u32 mem_mask) { return m_interrupt_snoop.enable | (m_interrupt_snoop.vector << 8); }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("PCI 50h: %08x & %08x\n", data, mem_mask);
			if (ACCESSING_BITS_0_7)
			{
				m_interrupt_snoop.enable = !!BIT(data, 0);
				LOG("\tInterrupt snoop enable %d\n", m_interrupt_snoop.enable);
			}
			if (ACCESSING_BITS_8_15)
			{
				m_interrupt_snoop.vector = (data >> 8) & 0xff;
				LOG("\tInterrupt snoop vector %02x\n", m_interrupt_snoop.vector);
			}
		})
	);
}

void trident_4dwavedx_device::io_map(address_map &map)
{
//	map(0x00, 0x0f) Legacy DMA
//	map(0x10, 0x1f) Legacy SB mapping

//	map(0x20, 0x23) Legacy MPU-401

//	map(0x30, 0x31) Legacy Game Port
//	map(0x34, 0x37) Enhanced Game Port 1
//	map(0x38, 0x3b) Enhanced Game Port 2

	map(0x40, 0x43).rw(m_ac97, FUNC(ac97_stac9704_device::codec_write_r), FUNC(ac97_stac9704_device::codec_write_w));
	map(0x44, 0x47).rw(m_ac97, FUNC(ac97_stac9704_device::codec_read_r), FUNC(ac97_stac9704_device::codec_read_w));
//	map(0x48, 0x4b) AC'97 Command/Status

//	map(0x50, 0x50) 4DWAVE-DX Status (r/o)
//	map(0x54, 0x55) Legacy SB Frequency (r/o)
//	map(0x57, 0x57) Legacy SB Time Constant (r/o)
	map(0x58, 0x5b).lrw32(
		NAME([this] (offs_t offset) {
			return m_asr3;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("ASR3: Scratch %04x\n", data);
			COMBINE_DATA(&m_asr3);
		})
	);
	map(0x5c, 0x5f).lrw32(
		NAME([this] (offs_t offset) {
			return m_asr4 | (m_asr5 << 16) | (m_asr6 << 24);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_7)
				m_asr4 = data & 0xff;
			if (ACCESSING_BITS_16_23)
			{
				m_asr5 = (data >> 16) & 0xff;
				LOG("ASR5: SB ESP Version High %02x\n", m_asr5);
			}
			if (ACCESSING_BITS_24_31)
			{
				m_asr6 = (data >> 24) & 0xff;
				LOG("ASR6: SB ESP Version High %02x\n", m_asr6);
			}
		})
	);
//	map(0x60, 0x63) OPL3 Emulation Channel Key on/off Trace

//	map(0x70, 0x73) Record Channel index 2 (chorus) / 1 (reverb) / 0 (mixer)
//	map(0x78, 0x7b) Bank A PCI Stream Buffer Valid flags (testing only)
//	map(0x7c, 0x7f) Bank B PCI Stream Buffer Valid flags (testing only)

//	map(0x80, 0xff) Wave Engine
	map(0x80, 0x83).rw(m_pcm, FUNC(t4dwave_pcm_device::banka_status_r), FUNC(t4dwave_pcm_device::starta_w));
	map(0x84, 0x87).rw(m_pcm, FUNC(t4dwave_pcm_device::banka_status_r), FUNC(t4dwave_pcm_device::stopa_w));
//	map(0x88, 0x8b) DLYA
//	map(0x8c, 0x8f) SIGNCSOA
//	map(0x90, 0x93) CSPFA
//	map(0x94, 0x97) CEBCA
	map(0x98, 0x9b).rw(m_pcm, FUNC(t4dwave_pcm_device::aina_r), FUNC(t4dwave_pcm_device::aina_w));
//	map(0x9c, 0x9f) EINTA
	map(0xa0, 0xa3).rw(m_pcm, FUNC(t4dwave_pcm_device::global_control_r), FUNC(t4dwave_pcm_device::global_control_w));
	map(0xa4, 0xa7).rw(m_pcm, FUNC(t4dwave_pcm_device::aintena_r), FUNC(t4dwave_pcm_device::aintena_w));
	map(0xa8, 0xab).rw(m_pcm, FUNC(t4dwave_pcm_device::wavevol_r), FUNC(t4dwave_pcm_device::wavevol_w));
//	map(0xac, 0xaf) DELTAR
	map(0xb0, 0xb3).rw(m_pcm, FUNC(t4dwave_pcm_device::miscint_r), FUNC(t4dwave_pcm_device::miscint_w));
	map(0xb4, 0xb7).rw(m_pcm, FUNC(t4dwave_pcm_device::bankb_status_r), FUNC(t4dwave_pcm_device::startb_w));
	map(0xb8, 0xbb).rw(m_pcm, FUNC(t4dwave_pcm_device::bankb_status_r), FUNC(t4dwave_pcm_device::stopb_w));
//	map(0xbc, 0xbf) CSPFB
//	map(0xc0, 0xc3) SBBL/SBCL
//	map(0xc4, 0xc7) SBE2R/SBDD/SBCTRL
//	map(0xc8, 0xcb) STIMER (r/o)
//	map(0xcc, 0xcd) ROM Test
//	map(0xce, 0xcf) LFOB
//	map(0xd0, 0xd3) Test Mixer FIFO
//	map(0xd4, 0xd7) Test Mixer Accumulator
	map(0xd8, 0xdb).rw(m_pcm, FUNC(t4dwave_pcm_device::ainb_r), FUNC(t4dwave_pcm_device::ainb_w));
	map(0xdc, 0xdf).rw(m_pcm, FUNC(t4dwave_pcm_device::aintenb_r), FUNC(t4dwave_pcm_device::aintenb_w));

	map(0xe0, 0xe3).rw(m_pcm, FUNC(t4dwave_pcm_device::cso_r), FUNC(t4dwave_pcm_device::cso_w));
	map(0xe4, 0xe7).w(m_pcm, FUNC(t4dwave_pcm_device::lba_w));
	map(0xe8, 0xeb).rw(m_pcm, FUNC(t4dwave_pcm_device::eso_r), FUNC(t4dwave_pcm_device::eso_w));
//	map(0xec, 0xef) FMC/RVOL/CVOL
	map(0xf0, 0xf3).rw(m_pcm, FUNC(t4dwave_pcm_device::gvsel_r), FUNC(t4dwave_pcm_device::gvsel_w));
//	map(0xf4, 0xf7) EBUF1 (bank A only)
//	map(0xf8, 0xfb) EBUF2 (bank A only)
}

void trident_4dwavedx_device::mmio_map(address_map &map)
{
	map(0x00, 0xff).m(*this, FUNC(trident_4dwavedx_device::io_map));
}

void trident_4dwavedx_device::gameport_map(address_map &map)
{
	map(0x00, 0x07).rw(m_joy, FUNC(pc_joy_device::joy_port_r), FUNC(pc_joy_device::joy_port_w));
}

void trident_4dwavedx_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	// TODO: legacy mapping
#if 0
	if (BIT(m_legacy_control, 7))
	{
		const u16 mpu401_port = BIT(m_legacy_control, 6) ? 0x0300 : 0x0330;
		io_space->install_device(mpu401_port, mpu401_port + 3, *this, &trident_4dwavedx_device::midi_map);
	}
#endif

	if (BIT(m_legacy_control, 5))
	{
		const u16 game_port = BIT(m_legacy_control, 4) ? 0x0208 : 0x0200;
		io_space->install_device(game_port, game_port + 7, *this, &trident_4dwavedx_device::gameport_map);
	}

#if 0
	if (BIT(m_legacy_control, 3))
	{
		const u16 fm_port = BIT(m_legacy_control, 2) ? 0x038c : 0x0388;
		io_space->install_device(fm_port, fm_port + 3, *this, &trident_4dwavedx_device::fm_map);
	}

	if (BIT(m_legacy_control, 1))
	{
		const u16 sb_port = BIT(m_legacy_control, 0) ? 0x0240 : 0x0220;
		io_space->install_device(sb_port, sb_port + 0xf, *this, &trident_4dwavedx_device::sb_map);
	}

#endif
}

/*
 *
 * 4DWAVE-DX wave engine
 *
 */

t4dwave_pcm_device::t4dwave_pcm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_datain_cb(*this, 0)
	, m_irq_cb(*this)
{
}

t4dwave_pcm_device::t4dwave_pcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: t4dwave_pcm_device(mconfig, T4DWAVE_PCM, tag, owner, clock)
{
}

void t4dwave_pcm_device::device_start()
{
	m_stream = stream_alloc(0, 2, 48000);

	save_item(NAME(m_global_control));
	save_item(NAME(m_cir));

	save_item(NAME(m_aina));
	save_item(NAME(m_aintena));
	save_item(NAME(m_bankA_keyon));

	save_item(NAME(m_ainb));
	save_item(NAME(m_aintenb));
	save_item(NAME(m_bankB_keyon));

	save_item(NAME(m_miscint));

	save_item(NAME(m_vol_cache));
	save_item(NAME(m_volL));
	save_item(NAME(m_volR));

	save_item(STRUCT_MEMBER(m_channel, lba));
	save_item(STRUCT_MEMBER(m_channel, cso));
	save_item(STRUCT_MEMBER(m_channel, hso));
	save_item(STRUCT_MEMBER(m_channel, eso));
	save_item(STRUCT_MEMBER(m_channel, eso_cache));
	save_item(STRUCT_MEMBER(m_channel, delta));
	save_item(STRUCT_MEMBER(m_channel, gvsel_cache));
	save_item(STRUCT_MEMBER(m_channel, gvsel));
	save_item(STRUCT_MEMBER(m_channel, pan_control));
	save_item(STRUCT_MEMBER(m_channel, pan_vol));
	save_item(STRUCT_MEMBER(m_channel, vol));
	save_item(STRUCT_MEMBER(m_channel, play_mode));
//	save_item(STRUCT_MEMBER(m_channel, is_16bit));
//	save_item(STRUCT_MEMBER(m_channel, is_stereo));
//	save_item(STRUCT_MEMBER(m_channel, is_signed));
	save_item(STRUCT_MEMBER(m_channel, loop_enable));
	save_item(STRUCT_MEMBER(m_channel, ec_envelope));
	save_item(STRUCT_MEMBER(m_channel, pci_buf));

	save_item(STRUCT_MEMBER(m_channel, ticks));
	save_item(STRUCT_MEMBER(m_channel, sample_data));
	save_item(STRUCT_MEMBER(m_channel, dma_fetch));
}

void t4dwave_pcm_device::device_reset()
{
	m_global_control = 0;
	m_cir = 0;

	m_miscint = 0;
	m_aina = m_aintena = m_ainb = m_aintenb = 0;
	m_bankA_keyon = m_bankB_keyon = 0;
}

// START_A & STOP_A
u32 t4dwave_pcm_device::banka_status_r(offs_t offset)
{
	return m_bankA_keyon;
}

u32 t4dwave_pcm_device::aina_r(offs_t offset)
{
	return m_aina;
}

void t4dwave_pcm_device::aina_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_aina &= ~data;
	update_irq_state();
}

void t4dwave_pcm_device::starta_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_bankA_keyon |= data;
	m_stream->update();
}

void t4dwave_pcm_device::stopa_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_bankA_keyon &= ~data;
	m_stream->update();
}

u32 t4dwave_pcm_device::aintena_r(offs_t offset)
{
	return m_aintena;
}

void t4dwave_pcm_device::aintena_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_aintena);
	update_irq_state();
}

u32 t4dwave_pcm_device::wavevol_r(offs_t offset)
{
	return m_vol_cache;
}

// 6.2 format
void t4dwave_pcm_device::wavevol_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vol_cache);
	m_volR[MUSICVOL] = 0x3f - ((m_vol_cache >> 26) & 0x3f);
	m_volL[MUSICVOL] = 0x3f - ((m_vol_cache >> 18) & 0x3f);
	m_volR[WAVEVOL]  = 0x3f - ((m_vol_cache >> 10) & 0x3f);
	m_volL[WAVEVOL]  = 0x3f - ((m_vol_cache >> 2)  & 0x3f);
}

// LFO_A & GC & CIR
u32 t4dwave_pcm_device::global_control_r(offs_t offset)
{
	return m_global_control;
}

void t4dwave_pcm_device::global_control_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_global_control);

	if (mem_mask & 0xffff'ff00)
		LOG("WAVE A0: Global Control %08x & %08x\n", data, mem_mask);

	if (ACCESSING_BITS_0_7)
		m_cir = data & 0x3f;
}

// ---- ---- ---- --x- ---- ---- ---- ---- opltimer_ie
// ---- ---- ---- ---x ---- ---- ---- ---- pb_24k_mode
// ---- ---- ---- ---- ---x ---- ---- ---- SB Record Interrupt Control
// ---- ---- ---- ---- ---- x--- ---- ---- mixer overflow flag (wc)
// ---- ---- ---- ---- ---- -x-- ---- ---- mixer underflow flag (wc)
// ---- ---- ---- ---- ---- --x- ---- ---- rec overrun status (wc)
// ---- ---- ---- ---- ---- ---x ---- ---- playback underrun (wc)
// ---- ---- ---- ---- ---- ---- -x-- ---- envelope irq (ro)
// ---- ---- ---- ---- ---- ---- --x- ---- address irq (ro)
// ---- ---- ---- ---- ---- ---- ---x ---- opl3 irq (ro)
// ---- ---- ---- ---- ---- ---- ---- x--- mpu401 irq (ro)
// ---- ---- ---- ---- ---- ---- ---- -x-- sb irq (ro)
// ---- ---- ---- ---- ---- ---- ---- --x- rec overrun irq (ro)
// ---- ---- ---- ---- ---- ---- ---- ---x playback overrun irq (ro)
u32 t4dwave_pcm_device::miscint_r(offs_t offset)
{
	u32 res = m_miscint & 0x0003'1f7f;
	return res;
}

void t4dwave_pcm_device::miscint_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_16_23)
	{
		m_miscint &= 0xff00'ffff;
		m_miscint |= (data & 0x0003'0000);
	}

	if (ACCESSING_BITS_8_15)
	{
		m_miscint &= ~(1 << 12);
		m_miscint |= BIT(data, 12) << 12;
		// apply write clear flags
		m_miscint &= ~(data & 0x0f00);
	}
}

// START_B / STOP_B
u32 t4dwave_pcm_device::bankb_status_r(offs_t offset)
{
	return m_bankB_keyon;
}

void t4dwave_pcm_device::startb_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_bankB_keyon |= data;
	m_stream->update();
}

void t4dwave_pcm_device::stopb_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_bankB_keyon &= ~data;
	m_stream->update();
}

u32 t4dwave_pcm_device::ainb_r(offs_t offset)
{
	return m_ainb;
}

void t4dwave_pcm_device::ainb_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_ainb &= ~data;
	update_irq_state();
}

u32 t4dwave_pcm_device::aintenb_r(offs_t offset)
{
	return m_aintenb;
}

void t4dwave_pcm_device::aintenb_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_aintenb);
	update_irq_state();
}

u32 t4dwave_pcm_device::cso_r(offs_t offset)
{
	channel_t &channel = m_channel[m_cir];

	if (!machine().side_effects_disabled())
		m_stream->update();
	return (channel.cso) << 16;
}

void t4dwave_pcm_device::cso_w(offs_t offset, u32 data, u32 mem_mask)
{
	channel_t &channel = m_channel[m_cir];

	if (ACCESSING_BITS_16_31)
		channel.cso = (data >> 16);
}

void t4dwave_pcm_device::lba_w(offs_t offset, u32 data, u32 mem_mask)
{
	channel_t &channel = m_channel[m_cir];

	COMBINE_DATA(&channel.lba);
	// bit 31-30 are for PCI Stream Buffer address pointer, set by HW
	channel.lba &= 0x3fff'ffff;
	channel.pci_buf = 0;
	m_stream->update();
}

u32 t4dwave_pcm_device::eso_r(offs_t offset)
{
	channel_t &channel = m_channel[m_cir];

	return (channel.eso_cache << 16) | (channel.delta & 0xffff);
}

void t4dwave_pcm_device::eso_w(offs_t offset, u32 data, u32 mem_mask)
{
	channel_t &channel = m_channel[m_cir];

	if (ACCESSING_BITS_16_31)
	{
		channel.eso_cache = (data >> 16);
		const u32 eso_shift_table[4] = { 2, 1, 1, 0 };

		// prepare ESO/HSO table here, actual range depends on mode used
		for (int i = 0; i < 8; i += 2)
		{
			const u32 eso_shift = eso_shift_table[i >> 1];
			// TODO: on 16-bit stereo loops clearly ends at +1, others apparently don't
			channel.eso[i + 0] = (channel.eso_cache + 1) >> eso_shift;
			channel.eso[i + 1] = channel.eso[i + 0];

			channel.hso[i + 0] = channel.eso[i + 0] >> 1;
			channel.hso[i + 1] = channel.hso[i + 0];
		}
	}
	if (ACCESSING_BITS_0_15)
	{
		channel.delta = data & 0xffff;
		// internal variables init-ed here for convenience
		channel.ticks = 0;
		channel.sample_data = 0;
		channel.dma_fetch = true;
	}
	m_stream->update();
}

u32 t4dwave_pcm_device::gvsel_r(offs_t offset)
{
	return m_channel[m_cir].gvsel_cache;
}

void t4dwave_pcm_device::gvsel_w(offs_t offset, u32 data, u32 mem_mask)
{
	channel_t &channel = m_channel[m_cir];
	COMBINE_DATA(&channel.gvsel_cache);

	channel.gvsel =       !!BIT(channel.gvsel_cache, 31);
	channel.pan_control = !!BIT(channel.gvsel_cache, 30);
	channel.pan_vol =     (channel.gvsel_cache >> 24) & 0x3f;
	channel.vol =         (channel.gvsel_cache >> 16) & 0xff;
	channel.play_mode =   (channel.gvsel_cache >> 13) & 7;
//	channel.is_16bit =    !!BIT(channel.gvsel_cache, 15);
//	channel.is_stereo =   !!BIT(channel.gvsel_cache, 14);
//	channel.is_signed =   !!BIT(channel.gvsel_cache, 13);
	channel.loop_enable = !!BIT(channel.gvsel_cache, 12);
	channel.ec_envelope = channel.gvsel_cache & 0xfff;

	m_stream->update();
}

/*
 * IRQ
 */

void t4dwave_pcm_device::update_irq_state()
{
	const u32 irq_state = (m_aina & m_aintena) || (m_ainb & m_aintenb);
	m_miscint &= ~(1 << ADDRESS_IRQ);

	if (irq_state)
	{
		m_miscint |= 1 << ADDRESS_IRQ;
		m_irq_cb(1);
	}
	else
		m_irq_cb(0);
}

/*
 * Sound Stream
 */

std::string t4dwave_pcm_device::print_audio_state(u64 keyon)
{
	std::map<u8, std::string> sample_modes = {
		{ 0, "u8  mono  " },
		{ 1, "s8  mono  " },
		{ 2, "u8  stereo" },
		{ 3, "s8  stereo" },
		{ 4, "u16 mono  " },
		{ 5, "s16 mono  " },
		{ 6, "u16 stereo" },
		{ 7, "s16 stereo" }
	};

	std::ostringstream outbuffer;

	util::stream_format(outbuffer, "LFO_A & GC & CIR %08x | MISCINT %08x\n", m_global_control, m_miscint);

	for (int ch = 0; ch < 64; ch ++)
	{
		channel_t &channel = m_channel[ch];

		if (!BIT(keyon, ch) || channel.cso >= channel.eso[channel.play_mode])
			continue;

		util::stream_format(outbuffer, "%d: LBA %08x -> CSO %05x ESO %05x DELTA %04x |%s loop %d\n",
			ch, channel.lba, channel.cso, channel.eso[channel.play_mode], channel.delta,
			sample_modes.at(channel.play_mode), channel.loop_enable);
	}

	return outbuffer.str();
}

const t4dwave_pcm_device::get_sample_func t4dwave_pcm_device::get_sample_table[8] =
{
	&t4dwave_pcm_device::get_sample_u8_mono,
	&t4dwave_pcm_device::get_sample_s8_mono,
	&t4dwave_pcm_device::get_sample_u8_stereo,
	&t4dwave_pcm_device::get_sample_s8_stereo,
	&t4dwave_pcm_device::get_sample_u16_mono,
	&t4dwave_pcm_device::get_sample_s16_mono,
	&t4dwave_pcm_device::get_sample_u16_stereo,
	&t4dwave_pcm_device::get_sample_s16_stereo
};

std::tuple<s16, s16> t4dwave_pcm_device::get_sample_u8_mono(u32 sample_data)
{
	u16 sample = (sample_data & 0xff) ^ 0x80;
	s16 out = (s16)(sample << 8);
	return std::make_tuple(out, out);
}

std::tuple<s16, s16> t4dwave_pcm_device::get_sample_s8_mono(u32 sample_data)
{
	u16 sample = (sample_data & 0xff);
	s16 out = (s16)(sample << 8);
	return std::make_tuple(out, out);
}

std::tuple<s16, s16> t4dwave_pcm_device::get_sample_u8_stereo(u32 sample_data)
{
	u16 sample = (sample_data & 0xffff) ^ 0x8080;
	return std::make_tuple((s16)(sample & 0xff00), (s16)((sample & 0x00ff) << 8));
}

std::tuple<s16, s16> t4dwave_pcm_device::get_sample_s8_stereo(u32 sample_data)
{
	u16 sample = (sample_data & 0xffff);
	return std::make_tuple((s16)(sample & 0xff00), (s16)((sample & 0x00ff) << 8));
}

std::tuple<s16, s16> t4dwave_pcm_device::get_sample_u16_mono(u32 sample_data)
{
	u16 sample = (sample_data & 0xffff) ^ 0x8000;
	s16 out = (s16)sample;
	return std::make_tuple(out, out);
}

std::tuple<s16, s16> t4dwave_pcm_device::get_sample_s16_mono(u32 sample_data)
{
	u16 sample = (sample_data & 0xffff);
	s16 out = (s16)sample;
	return std::make_tuple(out, out);
}

std::tuple<s16, s16> t4dwave_pcm_device::get_sample_u16_stereo(u32 sample_data)
{
	u16 lsample = (sample_data >> 16) ^ 0x8000;
	u16 rsample = (sample_data & 0xffff) ^ 0x8000;
	return std::make_tuple((s16)lsample, (s16)rsample);
}

std::tuple<s16, s16> t4dwave_pcm_device::get_sample_s16_stereo(u32 sample_data)
{
	return std::make_tuple((s16)(sample_data >> 16), (s16)(sample_data & 0xffff));
}

void t4dwave_pcm_device::sound_stream_update(sound_stream &stream)
{
	const u32 cso_increment_table[8] = { 1, 1, 2, 2, 2, 2, 4, 4 };
	const u32 sample_shift[8] = { 8, 8, 16, 16, 16, 16, 32, 32 };

	const u64 keyon = m_bankA_keyon | ((u64)m_bankB_keyon << 32);

	if (!keyon)
		return;

	if (LIVE_AUDIO_VIEW)
		popmessage(print_audio_state(keyon));

	for (int ch = 0; ch < 64; ch ++)
	{
		channel_t &channel = m_channel[ch];
		const u8 play_mode = channel.play_mode;

		if (!BIT(keyon, ch) || channel.cso >= channel.eso[play_mode])
			continue;

		const bool is_bankB = !!BIT(ch, 5);
		const u8 chB = ch - 32;

		for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
		{
			if (channel.dma_fetch)
			{
				if (channel.pci_buf)
					channel.sample_data >>= sample_shift[channel.play_mode];
				else
					channel.sample_data = m_datain_cb(channel.lba + (channel.cso << 2));
				channel.dma_fetch = false;
			}

			auto [left, right] = (this->*get_sample_table[channel.play_mode])(channel.sample_data);

			stream.add_int(0, sampindex, left  * m_volL[channel.gvsel], 32768 << 6);
			stream.add_int(1, sampindex, right * m_volR[channel.gvsel], 32768 << 6);

			channel.ticks += channel.delta;
			if (channel.ticks & 0x1000)
			{
				channel.dma_fetch = true;
				channel.ticks -= 0x1000;

				channel.pci_buf += cso_increment_table[channel.play_mode];
				channel.pci_buf &= 3;
				if (channel.pci_buf == 0)
				{
					channel.cso ++;
					if (channel.cso >= channel.hso[play_mode])
					{
						// MIDLP_IE
						if (BIT(m_global_control, 13))
						{
							if (is_bankB)
								m_ainb |= 1 << chB;
							else
								m_aina |= 1 << ch;
							update_irq_state();
						}
					}
					if (channel.cso >= channel.eso[play_mode])
					{
						channel.cso = 0;
						// ENDLP_IE
						if (BIT(m_global_control, 12))
						{
							if (is_bankB)
								m_ainb |= 1 << chB;
							else
								m_aina |= 1 << ch;
							update_irq_state();
						}
						if (!channel.loop_enable)
						{
							if (is_bankB)
								m_bankB_keyon &= ~(1 << chB);
							else
								m_bankA_keyon &= ~(1 << ch);
							continue;
						}
					}
				}
			}
		}
	}
}
