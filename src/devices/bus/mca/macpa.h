// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    M-Audio Capture and Playback Adapter/A

***************************************************************************/

#ifndef MAME_BUS_MCA_MACPA_H
#define MAME_BUS_MCA_MACPA_H

#pragma once

#include "mca.h"
#include "cpu/tms32025/tms32025.h"
#include "machine/ram.h"
#include "sound/dac.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mca16_macpa_device

class mca16_macpa_device :
		public device_t,
		public device_mca16_card_interface
{
public:
	// construction/destruction
	mca16_macpa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t io8_r(offs_t offset) override;
	virtual void io8_w(offs_t offset, uint8_t data) override;

    virtual uint8_t pos_r(offs_t offset) override;
	virtual void pos_w(offs_t offset, uint8_t data) override;

protected:
	mca16_macpa_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

    virtual void unmap() override;
    virtual void remap() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

    void DSP_map_data(address_map &map);
    void DSP_map_program(address_map &map);
    void DSP_map_io(address_map &map);

    uint16_t shared_ram_r(offs_t offset);
    void shared_ram_w(offs_t offset, u16 data, u16 mem_mask);

    uint16_t sample_ram_r(offs_t offset);
    void sample_ram_w(offs_t offset, u16 data, u16 mem_mask);

    required_device<tms32025_device> m_dsp;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;
	std::unique_ptr<uint16_t[]> m_sample_ram; // 2Kx16
    std::unique_ptr<uint16_t[]> m_shared_ram; // 8Kx16

private:
    uint16_t program_r(offs_t offset);
    void program_w(offs_t offset, uint16_t data);

    uint16_t data_r(offs_t offset);
    void data_w(offs_t offset, uint16_t data);

    void update_pos(uint8_t data);

    uint8_t shared_ram_read8_hi(offs_t offset);
    uint8_t shared_ram_read8_lo(offs_t offset);

    void shared_ram_write8_hi(offs_t offset, uint8_t data);
    void shared_ram_write8_lo(offs_t offset, uint8_t data);

    uint16_t tms_reset_r();
    void tms_int_w();
    void hreqack_w();

    uint16_t dsp_status_register_r(offs_t offset);
    void dsp_command_register_w(offs_t offset, uint16_t data);

    // Host control registers
    uint16_t m_data_latch;
    uint16_t m_address_latch;

    bool m_host_to_dsp_int_pending;         // Flag for Host->DSP interrupt (DSP INT1).
    bool m_dsp_to_host_int_pending;         // Flag for DSP->Host interrupt (host IRQ).
    bool m_sample_playback_int_pending;     // Flag for sample playback interrupt (DSP INT0).
    bool m_hintena;                         // Gates interrupts to the host.
    bool m_tms_reset;                       // Hold the DSP, reset all flags to power-up condition.

    uint8_t status_register_r();
    void command_register_w(uint8_t data);

    // The sample playback position. The address the DACs are currently reading from?
    typedef struct {
        uint16_t adc;
        uint16_t scr;
        uint16_t dac_r;
        uint16_t dac_l;
    } SAMPLE_MEMORY_POINTERS;

    bool m_dacl_enabled;
    bool m_dacr_enabled;

    bool m_dacl_int_pending;
    bool m_dacr_int_pending;

    void dacl_enable(bool enable);
    void dacr_enable(bool enable);

    SAMPLE_MEMORY_POINTERS m_sample_ptrs;

    // MCA bus setup
    uint16_t m_mapped_io;
    uint8_t m_mapped_irq;
    bool m_board_enable;
    bool m_board_is_mapped;
};

// device type definition
DECLARE_DEVICE_TYPE(MCA16_MACPA, mca16_macpa_device)

#endif