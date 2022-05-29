// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

	SiS 7018 Audio device (AC97 complaint)

    TODO:
	- Stub interface, to be improved;
	- Should be easy to at least inherit SB16/MIDI/game port devices;

**************************************************************************************************/

#include "emu.h"
#include "sis7018_audio.h"

#define LOG_IO     (1U << 1) // log PCI register accesses
#define LOG_TODO   (1U << 2) // log unimplemented registers
#define LOG_MAP    (1U << 3) // log full remaps
#define LOG_PMC    (1U << 4) // log PMC access

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_TODO | LOG_MAP | LOG_PMC)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)
#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)
#define LOGPMC(...)    LOGMASKED(LOG_PMC,  __VA_ARGS__)

DEFINE_DEVICE_TYPE(SIS7018_AUDIO, sis7018_audio_device, "sis7018_audio", "SiS 7018 Audio AC97")

sis7018_audio_device::sis7018_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, SIS7018_AUDIO, tag, owner, clock)

{
	// 0x040100 - Multimedia device, audio device (vendor specific i/f)
	// 0x01 - Rev 1
	set_ids(0x10397018, 0x01, 0x040100, 0x00);
}

void sis7018_audio_device::device_add_mconfig(machine_config &config)
{

}

void sis7018_audio_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	// map(0x003e, 0x003f) max latency min=0x02, max=0x18
	
	map(0x40, 0xe3).rw(FUNC(sis7018_audio_device::unmap_log_r), FUNC(sis7018_audio_device::unmap_log_w));

	// PMC capability identifier
	map(0xdc, 0xdf).r(FUNC(sis7018_audio_device::pmc_id_r));
//	map(0xe0, 0xe3).r(FUNC(sis7018_audio_device::pmc_status_r), FUNC(sis7018_audio_device::pmc_control_w));
}

u8 sis7018_audio_device::capptr_r()
{
	LOGIO("AUDIO Read capptr_r [$34]\n");
	return 0xdc;
}

// TODO: move to specific interface
u32 sis7018_audio_device::pmc_id_r()
{
	LOGPMC("Read PMC ID [$dc]\n");
	// bits 31-16 PPMI v1.0, D1 / D2 supported, no PCI clock for PME#
	// bits 15-8 0x00 no NEXT_PTR (NULL terminates here)
	// bits 7-0 PM_CAP_ID (0x01 for PMC)
	return 0xe6110001;
}

void sis7018_audio_device::memory_map(address_map &map)
{

}

void sis7018_audio_device::io_map(address_map &map)
{

}

void sis7018_audio_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
//	io_space->install_device(0, 0x03ff, *this, &sis7018_audio_device::io_map);
	// TODO: legacy handling, including game port
}

void sis7018_audio_device::device_start()
{
	pci_device::device_start();
	add_map(256, M_IO, FUNC(sis7018_audio_device::io_map));
	// not explicitly stated, assume 4096 size from the MEM decoding
	add_map(4096, M_MEM, FUNC(sis7018_audio_device::memory_map));
}


void sis7018_audio_device::device_reset()
{
	pci_device::device_reset();
	
	command = 0x0000;
	status = 0x0000;
	// INTB#
	intr_pin = 2;
	// TODO: can be written to with $46=1
	subsystem_id = 0x10397018;
}

/*
 * Debugging
 */

u8 sis7018_audio_device::unmap_log_r(offs_t offset)
{
	LOGTODO("AUDIO Unemulated [%02x] R\n", offset + 0x40);
	return 0;
}

void sis7018_audio_device::unmap_log_w(offs_t offset, u8 data)
{
	LOGTODO("AUDIO Unemulated [%02x] %02x W\n", offset + 0x40, data);
}
