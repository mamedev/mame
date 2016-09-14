// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef __FGA002_H__
#define __FGA002_H__

#include "emu.h"

#define MCFG_FGA002_ADD(_tag, _clock)   MCFG_DEVICE_ADD(_tag, FGA002, _clock)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> fga002_device

class fga002_device :  public device_t
{
	public:
	// construction/destruction
	fga002_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source);
	fga002_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER (write);
	DECLARE_READ8_MEMBER (read);
	protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer (emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	private:
	UINT8 m_tim0count;
	UINT8   m_fga002[0x500];

	/* Timer functions */
	UINT8 do_fga002reg_tim0preload_r();
	void  do_fga002reg_tim0preload_w(UINT8 data);
	UINT8 do_fga002reg_tim0ctl_r();
	void  do_fga002reg_tim0ctl_w(UINT8 data);
	UINT8 do_fga002reg_tim0count_r();
	void  do_fga002reg_tim0count_w(UINT8 data);
	UINT8 do_fga002reg_icrtim0_r();
	void  do_fga002reg_icrtim0_w(UINT8 data);
	UINT8 do_fga002reg_istim0_r();
	void  do_fga002reg_istim0_w(UINT8 data);
	emu_timer *fga_timer;
	enum
	{
		TIMER_ID_FGA
	};
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
extern const device_type FGA002;
#endif // __FGA002_H__
