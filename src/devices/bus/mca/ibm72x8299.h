// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
#ifndef MAME_BUS_MCA_IBM72X8299_H
#define MAME_BUS_MCA_IBM72X8299_H

#pragma once

#include "bus/mca/mca.h"
#include "machine/pit8253.h"

class ps2_mb_device;

class ibm72x8299_device : public device_t, public device_mca16_card_interface
{
public:
	ibm72x8299_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, ps2_mb_device *planar);
    ibm72x8299_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// outputs to host
	template<std::size_t Line> auto pit_ch_callback() { return m_pit_ch_cb[Line].bind(); }

    uint8_t pit_r(offs_t offset) { return m_pit->read(offset); };
    void pit_w(offs_t offset, uint8_t data) { m_pit->write(offset, data); }

    void    system_board_io_w(uint8_t data);
    uint8_t system_board_io_r() { return m_system_board_io_enable; }

    uint8_t system_board_pos_r();
    void    system_board_pos_w(uint8_t data);

    uint8_t pos_registers_r(offs_t offset);
    void    pos_registers_w(offs_t offset, uint8_t data);

    uint8_t adapter_pos_r();
    void    adapter_pos_w(uint8_t data);

    void    set_planar(ps2_mb_device *planar) { m_planar = planar; }

    uint8_t card_select_feedback_r(offs_t offset);

    DECLARE_WRITE_LINE_MEMBER(pit_ch2_gate_w);
    DECLARE_WRITE_LINE_MEMBER(pit_ch3_gate_w);
    DECLARE_WRITE_LINE_MEMBER(pit_ch3_clk_w);
    
    DECLARE_WRITE_LINE_MEMBER(cd_sfdbk_w);

protected:
	void device_start() override;
	void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
    virtual void device_resolve_objects() override;
    virtual void device_config_complete() override;

	required_device<mca16_slot_device> m_planar_vga;
    required_device<mca16_slot_device> m_planar_fdc;
	required_device<mca16_slot_device> m_planar_uart;
	required_device<mca16_slot_device> m_planar_lpt;

private:
    required_device<ps2_pit_device> m_pit;

    DECLARE_WRITE_LINE_MEMBER(pit_ch0_w);
    DECLARE_WRITE_LINE_MEMBER(pit_ch2_w);
    DECLARE_WRITE_LINE_MEMBER(pit_ch3_w);
    
	devcb_write_line::array<4> m_pit_ch_cb;

    uint8_t m_system_board_io_enable;
    uint8_t m_system_board_setup;
    uint8_t m_adapter_setup;
   	bool m_cd_sfdbk;

    ps2_mb_device *m_planar;
    uint16_t m_planar_id;
};

DECLARE_DEVICE_TYPE(IBM72X8299, ibm72x8299_device)

#endif