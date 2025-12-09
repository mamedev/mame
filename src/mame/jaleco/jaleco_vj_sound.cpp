// license:BSD-3-Clause
// copyright-holders:windyfairy
/*
ISA card used by VJ
Handles sound playback and communication between PC and arcade PCB

------------------------------------------------------------------
|                    IC26?                             CN6       |
|     IC23 IC24 IC25                                             |
|                           IC27      IC28            IC2x1      |
|     IC15 IC16 IC17                        IC29     IC2x2       |
| CN5                IC18                                    CN4 |
|     IC9  IC10 IC11                  IC19  IC20   IC14          |
|                                        X1 IC13   IC6           |
|     IC1  IC2  IC3  IC4    IC12         IC5       IC7    IC8    |
---------------------|           |-|                |-------------
                        |-----------| |----------------|


CN4 - 34-pin connector for IDC cable (chains to both CN6 on main board and CN1 on subboard)
CN5 - 80-pin connector
CN6 - 8-pin header, carries 4 channel stereo audio

IC1, IC2, IC3, IC9, IC10, IC11, IC15, IC16, IC17, IC23, IC24, IC25 - Toshiba TC554001 FL-70 4MBit 512Kx8 SRAM
IC4 - ?
IC5 - XILINX XC9572
IC7 - Texas Instruments HC24(5?)
IC6 - Texas Instruments HC244
IC12 - XILINX XCS30 PQ240CKN9817 A2014617A 3C
IC13 - Motorola 74HCU04A 845UE
IC14 - Texas Instruments HC245
IC18 - XILINX 17S30PC One-Time Programmable Configuration PROM
IC19 - Yamaha YMZ280B-F sound chip
IC20 - Yamaha YAC516-M DAC
IC2x1 - Unidentified 8-pin chip
IC2x2 - Unidentified TO-92 package 3-pin IC
IC26 - XILINX 17S30PC One-Time Programmable Configuration PROM
IC27 - XILINX XCS30 PQ240CKN9817 A2014617A 3C
IC28 - Yamaha YMZ280B-F sound chip
IC29 - Yamaha YAC516-M DAC
X1 - 16.9344MHz clock

Audio gets sent to the VJ-98346 4ch amp via 8 pin header (CN6) on ISA card


TODO: Check Stepping Stage
VJ and Stepping Stage most likely have different FPGA programs in IC18 and IC26 based on the differences in where
each game expects to be able to play certain audio files when loaded into the assigned channels.

TODO: Main PCB's YMZ should be used for the sound effects
Main board PCB's YMZ is used for sound effect channels (0x01, 0x02) but there's no way to differentiate
what YMZ chip a write to the YMZ register should go to so just using the two YMZ chips in this driver simplifies
the code a lot and also fixes some popping issues that would result from trying to play empty buffers.
*/

#include "emu.h"
#include "jaleco_vj_sound.h"

#include "speaker.h"


#define LOG_COMM      (1U << 1)
#define LOG_SOUND     (1U << 2)
// #define VERBOSE        (LOG_SOUND | LOG_COMM)
// #define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"


uint8_t jaleco_vj_isa16_sound_device::comm_r(offs_t offset)
{
	uint8_t r = 0;

	if (offset == 0) {
		r = m_comm & 0xff;
	} else if (offset == 1) {
		r = m_comm >> 8;
	}

	LOGMASKED(LOG_COMM, "comm_r: %04x\n", r);

	return r;
}

uint8_t jaleco_vj_isa16_sound_device::unk2_r(offs_t offset)
{
	// Usage unknown but it's known that it's a single byte sized register thanks to the
	// debug GUI output in the vj.exe program used by Stepping 3 Superior.
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_COMM, "unk2_r called!\n");
	return 0;
}

uint16_t jaleco_vj_isa16_sound_device::buffer_status_r(offs_t offset, uint16_t mem_mask)
{
	// This will only be checked if there is additional BGM data that can be transferred
	uint16_t r = m_request_extra_data && !m_receiving_extra_data ? 3 : 0;
	if (!machine().side_effects_disabled()) {
		m_is_checking_additional_bgm = true;
		LOGMASKED(LOG_COMM, "buffer_status_r: %04x\n", r);
	}
	return r;
}

uint16_t jaleco_vj_isa16_sound_device::response_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t r = m_response;
	if (!machine().side_effects_disabled()) {
		m_response = 0;
		LOGMASKED(LOG_COMM, "response_r: %04x\n", r);
	}
	return r;
}

void jaleco_vj_isa16_sound_device::response_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
		m_response = (m_response & ~0xff) | (data & 0xff);
	else if (offset == 1)
		m_response = (m_response & ~0xff00) | ((data & 0xff) << 8);

	LOGMASKED(LOG_COMM, "response_w: %04x\n", m_response);
}

uint16_t jaleco_vj_isa16_sound_device::target_buffer_r(offs_t offset, uint16_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_COMM, "target_buffer_r called!\n");
	return 0;
}

void jaleco_vj_isa16_sound_device::target_buffer_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// 0x01 Game SE
	// 0x02 Song SE
	// 0x09 Song BGM Normal
	// 0x0a Song BGM Woofer
	// 0x0b Song BGM Normal
	// 0x0c Song BGM Woofer
	if (data == 0x0101) {
		m_target_addr = 0x000000;
	} else if (data == 0x0202) {
		m_target_addr = 0x080000;
	} else if (data == 0x0909) {
		if (m_is_steppingstage)
			m_target_addr = 0x900000;
		else
			m_target_addr = m_receiving_extra_data ? 0x500000 : 0x400000;
	} else if (data == 0x0b0b) {
		if (m_is_steppingstage)
			m_target_addr = 0x980000;
		else
			m_target_addr = m_receiving_extra_data ? 0x500000 : 0x480000;
	} else if (data == 0x0a0a) {
		if (m_is_steppingstage)
			m_target_addr = 0xa00000;
		else
			m_target_addr = m_receiving_extra_data ? 0xb00000 : 0xa00000;
	} else if (data == 0x0c0c) {
		if (m_is_steppingstage)
			m_target_addr = 0xa80000;
		else
			m_target_addr = m_receiving_extra_data ? 0xb00000 : 0xa80000;
	}

	if (data == 0 && (m_target_buffer == 0x0101 || m_target_buffer == 0x0202) && m_is_checking_additional_bgm) {
		// The code used to load additional BGM is run in a separate thread, and it does not check if another file
		// is being transferred already, so if the code is called too soon then it'll clash with the code to load
		// the song keysound effects by overwriting the target address.
		m_request_extra_data = true;
	}

	m_target_buffer = data;

	LOGMASKED(LOG_COMM, "target_buffer_w: %04x (%08x)\n", data, m_target_addr);
}

void jaleco_vj_isa16_sound_device::sound_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_SOUND, "sound_data_w: %08x %04x\n", m_target_addr, data);

	m_ymzram[m_target_addr] = BIT(data, 0, 8);
	m_ymzram2[m_target_addr] = BIT(data, 8, 8);
	m_target_addr++;
}

uint16_t jaleco_vj_isa16_sound_device::unkc_r(offs_t offset, uint16_t mem_mask)
{
	// unkc_r and unke_r combined seem to do something together but it's unclear.
	// The values read back are never used, but the number of times unke_r is called before unkc_r is unique for different situations.
	// no calls to unke_r = writing additional BGM data
	// 1 call to unke_r = a BGM/sound request that will overwrite the address using the value written to target_buffer_w
	// 3 calls to unkc_r = preparing the thread that will write additional BGM data
	// It may be a hack but it's the only unique action performed that can differentiate between the different types of requests.
	if (!machine().side_effects_disabled()) {
		LOGMASKED(LOG_COMM, "unkc_r: %d\n", m_unke_read_cnt);

		if (m_unke_read_cnt == 0) {
			m_request_extra_data = false;
			m_receiving_extra_data = true;
		} else {
			m_receiving_extra_data = false;
		}

		m_unke_read_cnt = 0;
	}

	return 0;
}

uint16_t jaleco_vj_isa16_sound_device::unke_r(offs_t offset, uint16_t mem_mask)
{
	if (!machine().side_effects_disabled()) {
		m_unke_read_cnt++;
		LOGMASKED(LOG_COMM, "unke_r: %d\n", m_unke_read_cnt);
	}
	return 0;
}

void jaleco_vj_isa16_sound_device::comm_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// 0x00XX - "Rewrite" (Skipped)
	// 0x0100 - NOP?
	// 0x0101 - Check video ready
	// 0x0102 - COM_MSTART Start music
	// 0x0103 - COM_STOP Stop video
	// 0x0104 - Check music ready
	// 0x0106 - COM_RESTART (restarts video, Stepping Stage only)
	// 0x0107 - JSP dongle check (Stepping Stage only)
	// 0x0165 - COM_INIT
	// 0x029a - COM_START
	// 0x03XX - Video load command
	// 0x07XX - Music load command
	LOGMASKED(LOG_COMM, "comm_w: %04x\n", data);

	m_comm = data;
}

jaleco_vj_isa16_sound_device::jaleco_vj_isa16_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, JALECO_VJ_ISA16_SOUND, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	device_mixer_interface(mconfig, *this),
	m_ymz(*this, "ymz%u", 1),
	m_ymzram(*this, "ymz_ram"),
	m_ymzram2(*this, "ymz_ram2"),
	m_response(0),
	m_comm(0),
	m_target_buffer(0),
	m_target_addr(0),
	m_unke_read_cnt(0),
	m_request_extra_data(false),
	m_receiving_extra_data(false),
	m_is_checking_additional_bgm(false),
	m_is_steppingstage(false)
{
}

void jaleco_vj_isa16_sound_device::device_add_mconfig(machine_config &config)
{
	// BGM normal
	YMZ280B(config, m_ymz[0], 16.9344_MHz_XTAL);
	m_ymz[0]->set_addrmap(0, &jaleco_vj_isa16_sound_device::ymz280b_map);
	m_ymz[0]->add_route(1, *this, 1.0, 0);

	// BGM subwoofer
	YMZ280B(config, m_ymz[1], 16.9344_MHz_XTAL);
	m_ymz[1]->set_addrmap(0, &jaleco_vj_isa16_sound_device::ymz280b_map2);
	m_ymz[1]->add_route(1, *this, 1.0, 1);
}

void jaleco_vj_isa16_sound_device::device_start()
{
	set_isa_device();

	save_item(NAME(m_response));
	save_item(NAME(m_comm));
	save_item(NAME(m_target_buffer));
	save_item(NAME(m_target_addr));
	save_item(NAME(m_unke_read_cnt));
	save_item(NAME(m_request_extra_data));
	save_item(NAME(m_receiving_extra_data));
	save_item(NAME(m_is_checking_additional_bgm));
	save_item(NAME(m_is_steppingstage));
}

void jaleco_vj_isa16_sound_device::device_reset()
{
	m_response = 0;
	m_comm = 0;
	m_target_buffer = 0;
	m_target_addr = 0;
	m_unke_read_cnt = 0;
	m_request_extra_data = false;
	m_receiving_extra_data = false;
	m_is_checking_additional_bgm = false;
}

void jaleco_vj_isa16_sound_device::ymz280b_map(address_map &map)
{
	map(0x000000, 0xffffff).ram().share(m_ymzram);
}

void jaleco_vj_isa16_sound_device::ymz280b_map2(address_map &map)
{
	map(0x000000, 0xffffff).ram().share(m_ymzram2);
}

void jaleco_vj_isa16_sound_device::io_map(address_map &map)
{
	map(0x00, 0x01).r(FUNC(jaleco_vj_isa16_sound_device::comm_r));
	map(0x02, 0x02).r(FUNC(jaleco_vj_isa16_sound_device::unk2_r));
	map(0x04, 0x05).r(FUNC(jaleco_vj_isa16_sound_device::buffer_status_r));
	map(0x05, 0x06).w(FUNC(jaleco_vj_isa16_sound_device::response_w));
	map(0x08, 0x09).rw(FUNC(jaleco_vj_isa16_sound_device::target_buffer_r), FUNC(jaleco_vj_isa16_sound_device::target_buffer_w));
	map(0x0a, 0x0b).w(FUNC(jaleco_vj_isa16_sound_device::sound_data_w));
	map(0x0c, 0x0d).r(FUNC(jaleco_vj_isa16_sound_device::unkc_r));
	map(0x0e, 0x0f).r(FUNC(jaleco_vj_isa16_sound_device::unke_r));
}

void jaleco_vj_isa16_sound_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO) {
		m_isa->install_device(0x300, 0x30f, *this, &jaleco_vj_isa16_sound_device::io_map);
	}
}


DEFINE_DEVICE_TYPE(JALECO_VJ_ISA16_SOUND, jaleco_vj_isa16_sound_device, "jaleco_vj_sound", "VJSlap ISA Sound Card")
