// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI IOC2 I/O Controller emulation

**********************************************************************/

#ifndef MAME_MACHINE_IOC2_H
#define MAME_MACHINE_IOC2_H

#pragma once

#include "machine/8042kbdc.h"
#include "machine/pc_lpt.h"
#include "machine/pckeybrd.h"
#include "machine/pit8253.h"
#include "machine/z80scc.h"

class ioc2_device : public device_t
{
public:
	template <typename T> void set_cpu_tag(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }

	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( read );

	DECLARE_INPUT_CHANGED_MEMBER( power_button );
	DECLARE_INPUT_CHANGED_MEMBER( volume_down );
	DECLARE_INPUT_CHANGED_MEMBER( volume_up );

	void raise_local_irq(int channel, uint8_t mask);
	void lower_local_irq(int channel, uint8_t mask);

	enum : uint8_t
	{
		INT3_LOCAL0_FIFO      = 0x01,
		INT3_LOCAL0_SCSI0     = 0x02,
		INT3_LOCAL0_SCSI1     = 0x04,
		INT3_LOCAL0_ETHERNET  = 0x08,
		INT3_LOCAL0_MC_DMA    = 0x10,
		INT3_LOCAL0_PARALLEL  = 0x20,
		INT3_LOCAL0_GRAPHICS  = 0x40,
		INT3_LOCAL0_MAPPABLE0 = 0x80,
	};

	enum : uint8_t
	{
		INT3_LOCAL1_GP0       = 0x01,
		INT3_LOCAL1_PANEL     = 0x02,
		INT3_LOCAL1_GP2       = 0x04,
		INT3_LOCAL1_MAPPABLE1 = 0x08,
		INT3_LOCAL1_HPC_DMA   = 0x10,
		INT3_LOCAL1_AC_FAIL   = 0x20,
		INT3_LOCAL1_VSYNC     = 0x40,
		INT3_LOCAL1_RETRACE   = 0x80,
	};

	uint32_t get_local_int_status(int channel) const { return m_int3_local_status_reg[channel]; }
	uint32_t get_local_int_mask(int channel) const { return m_int3_local_mask_reg[channel]; }
	uint32_t get_map_int_status() const { return m_int3_map_status_reg; }
	uint32_t get_map_int_mask(int channel) const { return m_int3_map_mask_reg[channel]; }

	void set_local_int_mask(int channel, const uint32_t mask);
	void set_map_int_mask(int channel, const uint32_t mask);
	void set_timer_int_clear(const uint32_t data);

	uint8_t get_pit_reg(uint32_t offset) { return m_pit->read(offset); }
	void set_pit_reg(uint32_t offset, uint8_t data) { return m_pit->write(offset, data); }

protected:
	ioc2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE_LINE_MEMBER(timer0_int);
	DECLARE_WRITE_LINE_MEMBER(timer1_int);
	DECLARE_WRITE_LINE_MEMBER(pit_clock2_out);
	DECLARE_WRITE_LINE_MEMBER(kbdc_int_w);
	DECLARE_WRITE_LINE_MEMBER(duart_int_w);

	void set_mappable_int(uint8_t mask, bool state);
	void check_mappable_interrupt(int channel);

	enum
	{
		PI1_DATA_REG       = 0x00/4,
		PI1_CTRL_REG       = 0x04/4,
		PI1_STATUS_REG     = 0x08/4,
		PI1_DMA_CTRL_REG   = 0x0c/4,
		PI1_INT_STATUS_REG = 0x10/4,
		PI1_INT_MASK_REG   = 0x14/4,
		PI1_TIMER1_REG     = 0x18/4,
		PI1_TIMER2_REG     = 0x1c/4,
		PI1_TIMER3_REG     = 0x20/4,
		PI1_TIMER4_REG     = 0x24/4,

		SERIAL1_CMD_REG    = 0x30/4,
		SERIAL1_DATA_REG   = 0x34/4,
		SERIAL2_CMD_REG    = 0x38/4,
		SERIAL2_DATA_REG   = 0x3c/4,

		KBD_MOUSE_REGS1    = 0x40/4,
		KBD_MOUSE_REGS2    = 0x44/4,

		GENCTRL_SELECT_REG = 0x48/4,
		GENCTRL_REG        = 0x4c/4,

		PANEL_REG          = 0x50/4,
		SYSID_REG          = 0x58/4,
		READ_REG           = 0x60/4,
		DMA_SEL_REG        = 0x68/4,
		RESET_REG          = 0x70/4,
		WRITE_REG          = 0x78/4,

		INT3_LOCAL0_STATUS_REG = 0x80/4,
		INT3_LOCAL0_MASK_REG   = 0x84/4,
		INT3_LOCAL1_STATUS_REG = 0x88/4,
		INT3_LOCAL1_MASK_REG   = 0x8c/4,
		INT3_MAP_STATUS_REG    = 0x90/4,
		INT3_MAP_MASK0_REG     = 0x94/4,
		INT3_MAP_MASK1_REG     = 0x98/4,
		INT3_MAP_POLARITY_REG  = 0x9c/4,
		INT3_TIMER_CLEAR_REG   = 0xa0/4,
		INT3_ERROR_STATUS_REG  = 0xa4/4,

		TIMER_COUNT0_REG       = 0xb0/4,
		TIMER_COUNT1_REG       = 0xb4/4,
		TIMER_COUNT2_REG       = 0xb8/4,
		TIMER_CONTROL_REG      = 0xbc/4,
	};

	enum
	{
		FRONT_PANEL_POWER_STATE      = 0x01,
		FRONT_PANEL_POWER_BUTTON_INT = 0x02,
		FRONT_PANEL_VOL_DOWN_INT     = 0x10,
		FRONT_PANEL_VOL_DOWN_HOLD    = 0x20,
		FRONT_PANEL_VOL_UP_INT       = 0x40,
		FRONT_PANEL_VOL_UP_HOLD      = 0x80,

		FRONT_PANEL_INT_MASK         = FRONT_PANEL_POWER_BUTTON_INT |
									   FRONT_PANEL_VOL_DOWN_INT |
									   FRONT_PANEL_VOL_UP_INT
	};

	enum
	{
		DMA_SEL_CLOCK_SEL_MASK    = 0x30,
		DMA_SEL_CLOCK_SEL_10MHz   = 0x00,
		DMA_SEL_CLOCK_SEL_6_67MHz = 0x10,
		DMA_SEL_CLOCK_SEL_EXT     = 0x20,
	};

	required_device<cpu_device> m_maincpu;
	required_device<scc85230_device> m_scc;
	required_device<pc_lpt_device> m_pi1;   // we assume standard parallel port (SPP) mode
											// TODO: SGI parallel port (SGIPP), HP BOISE high speed parallel port (HPBPP), and Ricoh scanner modes
	required_device<kbdc8042_device> m_kbdc;
	required_device<pit8254_device> m_pit;

	virtual void handle_reset_reg_write(uint8_t data);
	virtual uint8_t get_system_id() = 0;

	uint8_t m_gen_ctrl_select_reg;
	uint8_t m_gen_ctrl_reg;
	uint8_t m_front_panel_reg;

	uint8_t m_read_reg;
	uint8_t m_dma_sel;
	uint8_t m_reset_reg;
	uint8_t m_write_reg;

	uint8_t m_int3_local_status_reg[2];
	uint8_t m_int3_local_mask_reg[2];
	uint8_t m_int3_map_status_reg;
	uint8_t m_int3_map_mask_reg[2];
	uint8_t m_int3_map_pol_reg;
	uint8_t m_int3_err_status_reg;

	uint32_t    m_par_read_cnt;
	uint32_t    m_par_cntl;
	uint8_t m_system_id;

	static char const *const SCC_TAG;
	static char const *const PI1_TAG;
	static char const *const KBDC_TAG;
	static char const *const PIT_TAG;
	static char const *const RS232A_TAG;
	static char const *const RS232B_TAG;

	static const XTAL SCC_PCLK;
	static const XTAL SCC_RXA_CLK;
	static const XTAL SCC_TXA_CLK;
	static const XTAL SCC_RXB_CLK;
	static const XTAL SCC_TXB_CLK;
};

class ioc2_guinness_device : public ioc2_device
{
public:
	template <typename T>
	ioc2_guinness_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag)
		: ioc2_guinness_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}

	ioc2_guinness_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	uint8_t get_system_id() override { return 0x01; }
};

class ioc2_full_house_device : public ioc2_device
{
public:
	template <typename T>
	ioc2_full_house_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag)
		: ioc2_full_house_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}

	ioc2_full_house_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	uint8_t get_system_id() override { return 0x20; }
};

DECLARE_DEVICE_TYPE(SGI_IOC2_GUINNESS,   ioc2_guinness_device)
DECLARE_DEVICE_TYPE(SGI_IOC2_FULL_HOUSE, ioc2_full_house_device)

#endif // MAME_MACHINE_IOC2_H
