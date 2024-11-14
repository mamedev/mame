// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef MAME_MACHINE_FGA002_H
#define MAME_MACHINE_FGA002_H

#include "cpu/m68000/m68000.h" // The FGA002 is designed for the 68K interrupt PL0-PL2 signalling, however used on SPARC and x86 boards too

class fga002_device :  public device_t
//      ,public device_z80daisy_interface
{
	public:
	// construction/destruction
	fga002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);

	void lirq_w(int status, int vector, int control, int state);
	void lirq0_w(int state);
	void lirq1_w(int state);
	void lirq2_w(int state);
	void lirq3_w(int state);
	void lirq4_w(int state);
	void lirq5_w(int state);
	void lirq6_w(int state);
	void lirq7_w(int state);

	u16 iack();
	int acknowledge();
	int get_irq_level();

	auto out_int() { return m_out_int_cb.bind(); }
	auto liack4() { return m_liack4_cb.bind(); }
	auto liack5() { return m_liack5_cb.bind(); }
	auto liack6() { return m_liack6_cb.bind(); }
	auto liack7() { return m_liack7_cb.bind(); }

 protected:
	// type for array of mapping of FGA registers that assembles an IRQ source
	typedef struct {
		int vector;
		int status;
		int control;
	} fga_irq_t;

	// interrupt sources in prio order if on same interrupt level. TODO: Add all sources
	const static fga_irq_t s_irq_sources[];

	fga002_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
#if 0
	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;
#endif

	TIMER_CALLBACK_MEMBER(timer_tick);

	devcb_write_line    m_out_int_cb;
	devcb_read8         m_liack4_cb;
	devcb_read8         m_liack5_cb;
	devcb_read8         m_liack6_cb;
	devcb_read8         m_liack7_cb;
	int m_int_state[0x08]; // interrupt state

 private:

	uint8_t m_tim0count;
	uint8_t   m_fga002[0x500];

	uint8_t do_fga002reg_ctl3_r();
	void do_fga002reg_ctl3_w(uint8_t data);

	/* Timer functions */
	uint8_t do_fga002reg_tim0preload_r();
	void  do_fga002reg_tim0preload_w(uint8_t data);
	uint8_t do_fga002reg_tim0ctl_r();
	void  do_fga002reg_tim0ctl_w(uint8_t data);
	uint8_t do_fga002reg_tim0count_r();
	void  do_fga002reg_tim0count_w(uint8_t data);
	uint8_t do_fga002reg_icrtim0_r();
	void  do_fga002reg_icrtim0_w(uint8_t data);
	uint8_t do_fga002reg_istim0_r();
	void  do_fga002reg_istim0_w(uint8_t data);
	emu_timer *fga_timer;

	enum {
		REG_TIM0CTL_ZERO_STOP   = 0x80,
		REG_TIM0CTL_AUTOPRELOAD = 0x40,
		REG_TIM0CTL_SYSFAIL     = 0x20,
		REG_TIM0CTL_START_STOP  = 0x10,
		REG_TIM0CTL_CLK_MSK     = 0x0f,
		REG_TIM0CTL_CLK_1_MIC   = 0x00,
		REG_TIM0CTL_CLK_2_MIC   = 0x01,
		REG_TIM0CTL_CLK_4_MIC   = 0x02,
		REG_TIM0CTL_CLK_8_MIC   = 0x03,
		REG_TIM0CTL_CLK_16_MIC  = 0x04,
		REG_TIM0CTL_CLK_32_MIC  = 0x05,
		REG_TIM0CTL_CLK_64_MIC  = 0x06,
		REG_TIM0CTL_CLK_128_MIC = 0x07,
		REG_TIM0CTL_CLK_256_MIC = 0x08,
		REG_TIM0CTL_CLK_512_MIC = 0x09,
		REG_TIM0CTL_CLK_2_MIL   = 0x0a,
		REG_TIM0CTL_CLK_8_MIL   = 0x0b,
		REG_TIM0CTL_CLK_32_MIL  = 0x0c,
		REG_TIM0CTL_CLK_125_MIL = 0x0d,
		REG_TIM0CTL_CLK_500_MIL = 0x0e,
		REG_TIM0CTL_CLK_2_SEC   = 0x0f,
	};
	enum {
		REG_ISTIM0_TIM_INT  = 0x80,
	};

	/* Interrupt support */
	uint8_t m_irq_level;
	uint8_t do_fga002reg_localiack_r();
	void do_fga002reg_localiack_w(uint8_t data);

	uint8_t do_fga002reg_islocal0_r();
	uint8_t do_fga002reg_islocal1_r();
	uint8_t do_fga002reg_islocal2_r();
	uint8_t do_fga002reg_islocal3_r();
	uint8_t do_fga002reg_islocal4_r();
	uint8_t do_fga002reg_islocal5_r();
	uint8_t do_fga002reg_islocal6_r();
	uint8_t do_fga002reg_islocal7_r();

	void islocal_w(int status, int vector, int control, int data);
	void  do_fga002reg_islocal0_w(uint8_t data);
	void  do_fga002reg_islocal1_w(uint8_t data);
	void  do_fga002reg_islocal2_w(uint8_t data);
	void  do_fga002reg_islocal3_w(uint8_t data);
	void  do_fga002reg_islocal4_w(uint8_t data);
	void  do_fga002reg_islocal5_w(uint8_t data);
	void  do_fga002reg_islocal6_w(uint8_t data);
	void  do_fga002reg_islocal7_w(uint8_t data);

	uint8_t do_fga002reg_icrlocal0_r();
	uint8_t do_fga002reg_icrlocal1_r();
	uint8_t do_fga002reg_icrlocal2_r();
	uint8_t do_fga002reg_icrlocal3_r();
	uint8_t do_fga002reg_icrlocal4_r();
	uint8_t do_fga002reg_icrlocal5_r();
	uint8_t do_fga002reg_icrlocal6_r();
	uint8_t do_fga002reg_icrlocal7_r();
	void  do_fga002reg_icrlocal0_w(uint8_t data);
	void  do_fga002reg_icrlocal1_w(uint8_t data);
	void  do_fga002reg_icrlocal2_w(uint8_t data);
	void  do_fga002reg_icrlocal3_w(uint8_t data);
	void  do_fga002reg_icrlocal4_w(uint8_t data);
	void  do_fga002reg_icrlocal5_w(uint8_t data);
	void  do_fga002reg_icrlocal6_w(uint8_t data);
	void  do_fga002reg_icrlocal7_w(uint8_t data);

	void trigger_interrupt(uint8_t data);
	void check_interrupts();

	enum {
		REG_ISLOCAL_IRQ = 0x80
	};

	enum {
		REG_CTL3_VECTORBITS7_6 = 0x0c
	};

	enum{
		REG_LIACK_LOCAL4_MSK    = 0x03,
		REG_LIACK_LOCAL5_MSK    = 0x0c, // >> 2 to use patterns below
		REG_LIACK_LOCAL6_MSK    = 0x30, // >> 4 to use patterns below
		REG_LIACK_LOCAL7_MSK    = 0xc0, // >> 6 to use patterns below
		REG_LIACK_INT_IACK      = 0x00, // Assumes bits to be right adjusted
		REG_LIACK_EXT_IACK1     = 0x01,
		REG_LIACK_EXT_IACK2     = 0x02,
		REG_LIACK_EXT_IACK3     = 0x03,
	};

	enum{
		INT_LOCAL0 = 0x30,
		INT_LOCAL1 = 0x31,
		INT_LOCAL2 = 0x32,
		INT_LOCAL3 = 0x33,
		INT_LOCAL4 = 0x34,
		INT_LOCAL5 = 0x35,
		INT_LOCAL6 = 0x36,
		INT_LOCAL7 = 0x37,
		INT_EMPTY  = 0x3F,
		INT_ACK_AUTOVECTOR  = -1,
	};

	enum{
		REG_ICR_LVL_MSK     = 0x07,
		REG_ICR_ENABLE      = 0x08,
		REG_ICR_AUTOCLR     = 0x10,
		REG_ICR_ACTIVITY    = 0x20,
		REG_ICR_EDGE        = 0x40,
	};

	/* Register offsets */
	enum {
		FGA_ICRMBOX0    = 0x0000,
		FGA_ICRMBOX1    = 0x0004,
		FGA_ICRMBOX2    = 0x0008,
		FGA_ICRMBOX3    = 0x000c,
		FGA_ICRMBOX4    = 0x0010,
		FGA_ICRMBOX5    = 0x0014,
		FGA_ICRMBOX6    = 0x0018,
		FGA_ICRMBOX7    = 0x001C,
		FGA_VMEPAGE     = 0x0200,
		FGA_ICRVME1     = 0x0204,
		FGA_ICRVME2     = 0x0208,
		FGA_ICRVME3     = 0x020c,
		FGA_ICRVME4     = 0x0210,
		FGA_ICRVME5     = 0x0214,
		FGA_ICRVME6     = 0x0218,
		FGA_ICRVME7     = 0x021c,
		FGA_ICRTIM0     = 0x0220,
		FGA_ICRDMANORM  = 0x0230,
		FGA_ICRDMAERR   = 0x0234,
		FGA_CTL1        = 0x0238,
		FGA_CTL2        = 0x023c,
		FGA_ICRFMB0REF  = 0x0240,
		FGA_ICRFMB1REF  = 0x0244,
		FGA_ICRFMB0MES  = 0x0248,
		FGA_ICRFMB1MES  = 0x024c,
		FGA_CTL3        = 0x0250,
		FGA_CTL4        = 0x0254,
		FGA_ICRPARITY   = 0x0258,
		FGA_AUXPINCTL   = 0x0260,
		FGA_CTL5        = 0x0264,
		FGA_AUXFIFWEX   = 0x0268,
		FGA_AUXFIFREX   = 0x026c,
		FGA_CTL6        = 0x0270,
		FGA_CTL7        = 0x0274,
		FGA_CTL8        = 0x0278,
		FGA_CTL9        = 0x027c,
		FGA_ICRABORT    = 0x0280,
		FGA_ICRACFAIL   = 0x0284,
		FGA_ICRSYSFAIL  = 0x0288,
		FGA_ICRLOCAL0   = 0x028c,
		FGA_ICRLOCAL1   = 0x0290,
		FGA_ICRLOCAL2   = 0x0294,
		FGA_ICRLOCAL3   = 0x0298,
		FGA_ICRLOCAL4   = 0x029c,
		FGA_ICRLOCAL5   = 0x02a0,
		FGA_ICRLOCAL6   = 0x02a4,
		FGA_ICRLOCAL7   = 0x02a8,
		FGA_ENAMCODE    = 0x02b4,
		FGA_CTL10       = 0x02c0,
		FGA_CTL11       = 0x02c4,
		FGA_MAINUM      = 0x02c8,
		FGA_MAINUU      = 0x02cc,
		FGA_BOTTOMPAGEU = 0x02d0,
		FGA_BOTTOMPAGEL = 0x02d4,
		FGA_TOPPAGEU    = 0x02d8,
		FGA_TOPPAGEL    = 0x02dc,
		FGA_MYVMEPAGE   = 0x02fc,
		FGA_TIM0PRELOAD = 0x0300,
		FGA_TIM0CTL     = 0x0310,
		FGA_DMASRCATT   = 0x0320,
		FGA_DMADSTATT   = 0x0324,
		FGA_DMA_GENERAL = 0x0328,
		FGA_CTL12       = 0x032c,
		FGA_LIOTIMING   = 0x0330,
		FGA_LOCALIACK   = 0x0334,
		FGA_FMBCTL      = 0x0338,
		FGA_FMBAREA     = 0x033c,
		FGA_AUXSRCSTART = 0x0340,
		FGA_AUXDSTSTART = 0x0344,
		FGA_AUXSRCTERM  = 0x0348,
		FGA_AUXDSTTERM  = 0x034c,
		FGA_CTL13       = 0x0350,
		FGA_CTL14       = 0x0354,
		FGA_CTL15       = 0x0358,
		FGA_CTL16       = 0x035c,
		FGA_SPECIALENA  = 0x0424,
		FGA_ISLOCAL0    = 0x0480,
		FGA_ISLOCAL1    = 0x0484,
		FGA_ISLOCAL2    = 0x0488,
		FGA_ISLOCAL3    = 0x048C,
		FGA_ISLOCAL4    = 0x0490,
		FGA_ISLOCAL5    = 0x0494,
		FGA_ISLOCAL6    = 0x0498,
		FGA_ISLOCAL7    = 0x049C,
		FGA_ISTIM0      = 0x04a0,
		FGA_ISDMANORM   = 0x04b0,
		FGA_ISDMAERR    = 0x04b4,
		FGA_ISFMB0REF   = 0x04b8,
		FGA_ISFMB1REF   = 0x04bc,
		FGA_ISPARITY    = 0x04c0,
		FGA_DMARUNCTL   = 0x04c4,
		FGA_ISABORT     = 0x04c8,
		FGA_ISACFAIL    = 0x04cc,
		FGA_ISFMB0MES   = 0x04e0,
		FGA_ISFMB1MES   = 0x04e4,
		FGA_ISSYSFAIL   = 0x04d0,
		FGA_ABORTPIN    = 0x04d4,
		FGA_RSVMECALL   = 0x04f0,
		FGA_RSKEYRES    = 0x04f4,
		FGA_RSCPUCALL   = 0x04f8,
		FGA_RSLOCSW     = 0x04fc,
		FGA_TIM0COUNT   = 0x0c00,
	};
};


// device type definition
DECLARE_DEVICE_TYPE(FGA002, fga002_device)

#endif // MAME_MACHINE_FGA002_H
