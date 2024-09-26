// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI IOC2 I/O Controller emulation

**********************************************************************/

#ifndef MAME_SGI_IOC2_H
#define MAME_SGI_IOC2_H

#pragma once

#include "machine/at_keybc.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"
#include "machine/pc_lpt.h"
#include "machine/pckeybrd.h"
#include "machine/pit8253.h"
#include "machine/z80scc.h"

class ioc2_device : public device_t
{
public:
	template <typename T> void set_cpu_tag(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }

	DECLARE_INPUT_CHANGED_MEMBER(power_button);
	DECLARE_INPUT_CHANGED_MEMBER(volume_down);
	DECLARE_INPUT_CHANGED_MEMBER(volume_up);

	void gio_int0_w(int state);
	void gio_int1_w(int state);
	void gio_int2_w(int state);
	void hpc_dma_done_w(int state);
	void mc_dma_done_w(int state);
	void scsi0_int_w(int state);
	void scsi1_int_w(int state);
	void enet_int_w(int state);
	void video_int_w(int state);

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
		INT3_LOCAL1_VIDEO     = 0x40,
		INT3_LOCAL1_RETRACE   = 0x80,
	};

	void set_local_int_mask(int channel, const uint8_t mask);
	void set_map_int_mask(int channel, const uint8_t mask);
	void set_timer_int_clear(const uint8_t data);
	void set_mappable_int(uint8_t mask, bool state);

protected:
	ioc2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void timer0_int(int state);
	void timer1_int(int state);
	void pit_clock2_out(int state);
	void kbdc_int_w(int state);
	void duart_int_w(int state);

	void check_mappable_interrupt(int channel);

	u8 pi1_dma_ctrl_r();
	void pi1_dma_ctrl_w(u8 data);
	u8 pi1_int_status_r();
	void pi1_int_status_w(u8 data);
	u8 pi1_int_mask_r();
	void pi1_int_mask_w(u8 data);
	u8 pi1_timer1_r();
	u8 pi1_timer2_r();
	u8 pi1_timer3_r();
	u8 pi1_timer4_r();
	void pi1_timer1_w(u8 data);
	void pi1_timer2_w(u8 data);
	void pi1_timer3_w(u8 data);
	void pi1_timer4_w(u8 data);
	u8 gc_select_r();
	void gc_select_w(u8 data);
	u8 gen_cntl_r();
	void gen_cntl_w(u8 data);
	u8 front_panel_r();
	void front_panel_w(u8 data);
	u8 system_id_r();
	u8 read_r();
	u8 dma_sel_r();
	void dma_sel_w(u8 data);
	u8 reset_r();
	void reset_w(u8 data);
	u8 write_r();
	void write_w(u8 data);
	template <int N> u8 local_status_r();
	template <int N> u8 local_mask_r();
	template <int N> void local_mask_w(u8 data);
	u8 map_status_r();
	template <int N> u8 map_mask_r();
	template <int N> void map_mask_w(u8 data);
	u8 map_pol_r();
	void map_pol_w(u8 data);
	void timer_int_clear_w(u8 data);
	u8 error_status_r();

	void base_map(address_map &map) ATTR_COLD;

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
											// TODO: SGI parallel port (SGIPP), HP BOISE high speed parallel port (HPBPP), and Ricoh scanner mode
	required_device<ps2_keyboard_controller_device> m_kbdc;
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
	uint8_t m_system_id = 0;

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

	void map(address_map &map) ATTR_COLD;
	ioc2_guinness_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	uint8_t get_system_id() override { return 0x26; }
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

	void map(address_map &map) ATTR_COLD;
	void int2_map(address_map &map) ATTR_COLD;

protected:
	uint8_t get_system_id() override { return 0x11; }
};

DECLARE_DEVICE_TYPE(SGI_IOC2_GUINNESS,   ioc2_guinness_device)
DECLARE_DEVICE_TYPE(SGI_IOC2_FULL_HOUSE, ioc2_full_house_device)

#endif // MAME_SGI_IOC2_H
