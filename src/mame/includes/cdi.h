#ifndef _INCLUDES_CDI_H_
#define _INCLUDES_CDI_H_

#include "machine/cdi070.h"
#include "machine/cdislave.h"
#include "machine/cdicdic.h"
#include "sound/dmadac.h"
#include "video/mcd212.h"

/*----------- driver state -----------*/

#define CLOCK_A XTAL_30MHz
#define CLOCK_B XTAL_19_6608MHz

class cdi_state : public driver_device
{
public:
    cdi_state(const machine_config &mconfig, device_type type, const char *tag)
        : driver_device(mconfig, type, tag) ,
		m_planea(*this, "planea"),
		m_planeb(*this, "planeb"){ }

	required_shared_ptr<UINT16> m_planea;
	required_shared_ptr<UINT16> m_planeb;

    dmadac_sound_device *m_dmadac[2];

    UINT8 m_timer_set;
    emu_timer *m_test_timer;
    bitmap_rgb32 m_lcdbitmap;
    scc68070_regs_t m_scc68070_regs;
    mcd212_regs_t m_mcd212_regs;
    mcd212_ab_t m_mcd212_ab;
	DECLARE_INPUT_CHANGED_MEMBER(mcu_input);
	virtual void machine_start();
	virtual void video_start();
	DECLARE_MACHINE_RESET(cdi);
	DECLARE_MACHINE_RESET(quizrd12);
	DECLARE_MACHINE_RESET(quizrd17);
	DECLARE_MACHINE_RESET(quizrd18);
	DECLARE_MACHINE_RESET(quizrd22);
	DECLARE_MACHINE_RESET(quizrd23);
	DECLARE_MACHINE_RESET(quizrd32);
	DECLARE_MACHINE_RESET(quizrd34);
	DECLARE_MACHINE_RESET(quizrr40);
	DECLARE_MACHINE_RESET(quizrr41);
	DECLARE_MACHINE_RESET(quizrr42);
	UINT32 screen_update_cdimono1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_cdimono1_lcd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

/*----------- debug defines -----------*/

#define VERBOSE_LEVEL   (0)

#define ENABLE_VERBOSE_LOG (0)

#define ENABLE_UART_PRINTING (0)

#endif // _INCLUDES_CDI_H_
