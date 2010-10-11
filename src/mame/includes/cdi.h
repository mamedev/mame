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
    cdi_state(running_machine &machine, const driver_device_config_base &config)
        : driver_device(machine, config) { }

    UINT16 *planea;
    UINT16 *planeb;

    dmadac_sound_device *dmadac[2];

    UINT8 timer_set;
    emu_timer *test_timer;
    bitmap_t* lcdbitmap;
    scc68070_regs_t scc68070_regs;
    mcd212_regs_t mcd212_regs;
    mcd212_ab_t mcd212_ab;
};

/*----------- debug defines -----------*/

#define VERBOSE_LEVEL   (5)

#define ENABLE_VERBOSE_LOG (0)

#define ENABLE_UART_PRINTING (0)

#endif // _INCLUDES_CDI_H_
