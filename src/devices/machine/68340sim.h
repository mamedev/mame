// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_68340SIM_H
#define MAME_MACHINE_68340SIM_H

#pragma once

class m68340_sim
{
public:
	// Chip selects
	uint32_t m_am[4]; // chip select address mask + control, unaffected by reset
	uint32_t m_ba[4]; // chip select base address + control, unaffected by reset

	// Port registers    Reset values
	uint8_t m_porta;  // unaffected by reset
	uint8_t m_ddra;   // 0x00
	uint8_t m_ppara1; // 0xff
	uint8_t m_ppara2; // 0x00
	uint8_t m_portb;  // unaffected by reset
	uint8_t m_ddrb;   // 0x00
	uint8_t m_pparb;  // 0xff

	// SIM
	uint16_t m_mcr;     // 0x608f
	uint16_t m_syncr;   // 0x3f00
	uint16_t m_avr_rsr; // 0x0000
	uint16_t m_swiv_sypcr;//0x0f00
	uint16_t m_picr;    // 0x000f
	uint16_t m_pitr;    // 0x0000 | ~MODCK pin
	uint16_t m_swsr;    // 0x0000

	// Timers and counters
	uint8_t m_pit_counter;
	bool m_pit_irq;

	void reset();
	void module_reset();

	enum {
		REG_MCR          = 0x00,
		REG_SYNCR        = 0x04,
		REG_AVR_RSR      = 0x06,
		REG_PORTA        = 0x11,
		REG_DDRA         = 0x13,
		REG_PPARA1       = 0x15,
		REG_PPARA2       = 0x17,
		REG_PORTB        = 0x19,
		REG_PORTB1       = 0x1b,
		REG_DDRB         = 0x1d,
		REG_PPARB        = 0x1f,
		REG_SWIV_SYPCR   = 0x20,
		REG_PICR         = 0x22,
		REG_PITR         = 0x24,
		REG_SWSR         = 0x26,
		REG_AM_CS0       = 0x40,
		REG_BA_CS0       = 0x44,
		REG_AM_CS1       = 0x48,
		REG_BA_CS1       = 0x4c,
		REG_AM_CS2       = 0x50,
		REG_BA_CS2       = 0x54,
		REG_AM_CS3       = 0x58,
		REG_BA_CS3       = 0x5c,
	};

	enum {
		REG_MCR_FRZ1   = 0x4000,
		REG_MCR_FRZ0   = 0x2000,
		REG_MCR_FIRQ   = 0x1000,
		REG_MCR_SHEN   = 0x0300,
		REG_MCR_SVREG  = 0x0800,
		REG_MCR_ARBLV  = 0x000f,
	};

	enum {
		REG_AVR_VEC     = 0xff00,
		REG_RSR_RESBITS = 0x00ff,
	};

	enum {
		REG_SWIV_VEC    = 0xff00,
		REG_SYPCR_SWE   = 0x0080,
		REG_SYPCR_SWRI  = 0x0040,
		REG_SYPCR_SWT   = 0x0030,
		REG_SYPCR_DBFE  = 0x0008,
		REG_SYPCR_BME   = 0x0004,
		REG_SYPCR_BMT   = 0x0003,
	};

	enum {
		REG_PICR_PIRQL  = 0x0700,
		REG_PICR_PIVEC  = 0x00ff,
	};

	enum {
		REG_PITR_SWP    = 0x0200,
		REG_PITR_PTP    = 0x0100,
		REG_PITR_COUNT  = 0x00ff,
	};

	enum {
		REG_SYNCR_W     = 0x8000,
		REG_SYNCR_X     = 0x4000,
		REG_SYNCR_Y_MSK = 0x3f00,
		REG_SYNCR_SLIMP = 0x0010,
		REG_SYNCR_SLOCK = 0x0008,
		REG_SYNCR_RSTEN = 0x0004,
		REG_SYNCR_STSIM = 0x0002,
		REG_SYNCR_STEXT = 0x0001,
	};

	enum {
		CLOCK_PLL    = 0x01,
		CLOCK_MODCK  = 0x02,
	};

	enum {
		CLOCK_MODE_EXT      = 0x00,
		CLOCK_MODE_EXT_PLL  = 0x01,
		CLOCK_MODE_CRYSTAL  = 0x03,
	};
};

#endif // MAME_MACHINE_68340SIM_H
