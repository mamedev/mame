// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef MAME_LUXOR_ABC1600_H
#define MAME_LUXOR_ABC1600_H

#include "bus/abcbus/abcbus.h"
#include "bus/abckb/abckb.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68008.h"
#include "formats/abc1600_dsk.h"
#include "imagedev/floppy.h"
#include "abc1600mac.h"
#include "machine/e0516.h"
#include "machine/nmc9306.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/z80dma.h"
#include "machine/z80scc.h"
#include "machine/z80sio.h"
#include "machine/z8536.h"
#include "abc1600_v.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MC68008P8_TAG       "3f"
#define Z8410AB1_0_TAG      "5g"
#define Z8410AB1_1_TAG      "7g"
#define Z8410AB1_2_TAG      "9g"
#define Z8470AB1_TAG        "17b"
#define Z8530B1_TAG         "2a"
#define Z8536B1_TAG         "15b"
#define SAB1797_02P_TAG     "5a"
#define FDC9229BT_TAG       "7a"
#define E050_C16PC_TAG      "13b"
#define NMC9306_TAG         "14c"
#define SCREEN_TAG          "screen"
#define BUS0I_TAG           "bus0i"
#define BUS0X_TAG           "bus0x"
#define BUS1_TAG            "bus1"
#define BUS2_TAG            "bus2"
#define RS232_A_TAG         "rs232a"
#define RS232_B_TAG         "rs232b"
#define RS232_PR_TAG        "rs232pr"
#define ABC_KEYBOARD_PORT_TAG   "kb"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc1600_state

class abc1600_state : public driver_device
{
public:
	abc1600_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, MC68008P8_TAG),
		m_mac(*this, ABC1600_MAC_TAG),
		m_dma0(*this, Z8410AB1_0_TAG),
		m_dma1(*this, Z8410AB1_1_TAG),
		m_dma2(*this, Z8410AB1_2_TAG),
		m_dart(*this, Z8470AB1_TAG),
		m_scc(*this, Z8530B1_TAG),
		m_cio(*this, Z8536B1_TAG),
		m_fdc(*this, SAB1797_02P_TAG),
		m_rtc(*this, E050_C16PC_TAG),
		m_nvram(*this, NMC9306_TAG),
		m_ram(*this, RAM_TAG),
		m_floppy(*this, SAB1797_02P_TAG":%u", 0U),
		m_bus0i(*this, BUS0I_TAG),
		m_bus0x(*this, BUS0X_TAG),
		m_bus1(*this, BUS1_TAG),
		m_bus2(*this, BUS2_TAG),
		m_kb(*this, ABC_KEYBOARD_PORT_TAG)
	{ }

	void abc1600(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER( reset );

private:
	required_device<m68008_device> m_maincpu;
	required_device<abc1600_mac_device> m_mac;
	required_device<z80dma_device> m_dma0;
	required_device<z80dma_device> m_dma1;
	required_device<z80dma_device> m_dma2;
	required_device<z80dart_device> m_dart;
	required_device<scc8530_device> m_scc;
	required_device<z8536_device> m_cio;
	required_device<fd1797_device> m_fdc;
	required_device<e0516_device> m_rtc;
	required_device<nmc9306_device> m_nvram;
	required_device<ram_device> m_ram;
	required_device_array<floppy_connector, 3> m_floppy;
	required_device<abcbus_slot_device> m_bus0i;
	required_device<abcbus_slot_device> m_bus0x;
	required_device<abcbus_slot_device> m_bus1;
	required_device<abcbus_slot_device> m_bus2;
	required_device<abc_keyboard_port_device> m_kb;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	static void floppy_formats(format_registration &fr);

	uint8_t bus_r(offs_t offset);
	void bus_w(offs_t offset, uint8_t data);
	uint8_t dart_r(offs_t offset);
	void dart_w(offs_t offset, uint8_t data);
	uint8_t scc_r(offs_t offset);
	void scc_w(offs_t offset, uint8_t data);
	uint8_t cio_r(offs_t offset);
	void cio_w(offs_t offset, uint8_t data);
	void fw0_w(uint8_t data);
	void fw1_w(uint8_t data);

	void cs7_w(int state);
	void btce_w(int state);
	void atce_w(int state);
	void dmadis_w(int state);
	void sysscc_w(int state);
	void sysfs_w(int state);
	void dbrq0_w(int state) { m_dbrq0 = state; update_br(); }
	void dbrq1_w(int state) { m_dbrq1 = state; update_br(); }
	void dbrq2_w(int state) { m_dbrq2 = state; update_br(); }

	uint8_t cio_pa_r();
	uint8_t cio_pb_r();
	void cio_pb_w(uint8_t data);
	uint8_t cio_pc_r();
	void cio_pc_w(uint8_t data);

	void nmi_w(int state);

	void cpu_space_map(address_map &map) ATTR_COLD;

	void update_br();
	void update_pren0(int state);
	void update_pren1(int state);
	void update_drdy0(int state);
	void update_drdy1(int state);
	void sccrq_a_w(int state) { m_sccrq_a = state; update_drdy1(0); }
	void sccrq_b_w(int state) { m_sccrq_b = state; update_drdy1(0); }
	void dart_irq_w(int state) { m_dart_irq = state; m_maincpu->set_input_line(M68K_IRQ_5, (m_dart_irq || m_scc_irq) ? ASSERT_LINE : CLEAR_LINE); }
	void scc_irq_w(int state) { m_scc_irq = state; m_maincpu->set_input_line(M68K_IRQ_5, (m_dart_irq || m_scc_irq) ? ASSERT_LINE : CLEAR_LINE); }

	// DMA
	int m_dmadis = 0;
	int m_sysscc = 0;
	int m_sysfs = 0;
	int m_dbrq0 = CLEAR_LINE;
	int m_dbrq1 = CLEAR_LINE;
	int m_dbrq2 = CLEAR_LINE;

	void abc1600_mem(address_map &map) ATTR_COLD;
	void mac_mem(address_map &map) ATTR_COLD;

	// peripherals
	int m_cs7 = 0;                  // card select address bit 7
	int m_bus0 = 0;                 // BUS 0 selected
	uint8_t m_csb = 0U;             // card select
	int m_atce = 0;                 // V.24 channel A external clock enable
	int m_btce = 0;                 // V.24 channel B external clock enable
	bool m_sccrq_a = 0;
	bool m_sccrq_b = 0;
	int m_scc_irq = 0;
	int m_dart_irq = 0;
};



#endif // MAME_LUXOR_ABC1600_H
