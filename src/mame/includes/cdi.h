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
        : driver_device(mconfig, type, tag) { }

    UINT16 *m_planea;
    UINT16 *m_planeb;

    dmadac_sound_device *m_dmadac[2];

    UINT8 m_timer_set;
    emu_timer *m_test_timer;
    bitmap_rgb32 m_lcdbitmap;
    scc68070_regs_t m_scc68070_regs;
    mcd212_regs_t m_mcd212_regs;
    mcd212_ab_t m_mcd212_ab;
	DECLARE_INPUT_CHANGED_MEMBER(mcu_input);
};

/*----------- debug defines -----------*/

#define VERBOSE_LEVEL   (0)

#define ENABLE_VERBOSE_LOG (0)

#define ENABLE_UART_PRINTING (0)

#endif // _INCLUDES_CDI_H_
