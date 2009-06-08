#include "driver.h"
#include "sound/vlm5030.h"
#include "sound/msm5205.h"
#include "sound/sn76496.h"
#include "includes/trackfld.h"


#define TIMER_RATE (4096/4)


static int SN76496_latch;

/* The timer port on TnF and HyperSports sound hardware is derived from
   a 14.318 MHz clock crystal which is passed  through a couple of 74ls393
    ripple counters.
    Various outputs of the ripper counters clock the various chips.
    The Z80 uses 14.318 MHz / 4 (3.4MHz)
    The SN chip uses 14.318 MHz / 8 (1.7MHz)
    And the timer is connected to 14.318 MHz / 4096
    As we are using the Z80 clockrate as a base value we need to multiply
    the no of cycles by 4 to undo the 14.318/4 operation
*/

READ8_HANDLER( trackfld_sh_timer_r )
{
    UINT32 clock = cpu_get_total_cycles(space->cpu) / TIMER_RATE;

    return clock & 0xF;
}

READ8_DEVICE_HANDLER( trackfld_speech_r )
{
    return vlm5030_bsy(device) ? 0x10 : 0;
}

static int last_addr = 0;

WRITE8_DEVICE_HANDLER( trackfld_sound_w )
{
   int changes = offset^last_addr;
   /* A7 = data enable for VLM5030 (don't care )          */
   /* A8 = STA pin (1->0 data data  , 0->1 start speech   */
   /* A9 = RST pin 1=reset                                */

   /* A8 VLM5030 ST pin */
   if( changes & 0x100 )
       vlm5030_st( device, offset&0x100 );
   /* A9 VLM5030 RST pin */
   if( changes & 0x200 )
       vlm5030_rst( device, offset&0x200 );
    last_addr = offset;
}

READ8_HANDLER( hyperspt_sh_timer_r )
{
    UINT32 clock = cpu_get_total_cycles(space->cpu) / TIMER_RATE;

    return (clock & 0x3) | (vlm5030_bsy(devtag_get_device(space->machine, "vlm"))? 0x04 : 0);
}

WRITE8_DEVICE_HANDLER( hyperspt_sound_w )
{
    int changes = offset^last_addr;
    /* A3 = data enable for VLM5030 (don't care )          */
    /* A4 = STA pin (1->0 data data  , 0->1 start speech   */
    /* A5 = RST pin 1=reset                                */
    /* A6 = VLM5030    output disable (don't care ) */
    /* A7 = kONAMI DAC output disable (don't care ) */
    /* A8 = SN76489    output disable (don't care ) */

    /* A4 VLM5030 ST pin */
    if( changes & 0x10 )
        vlm5030_st( device, offset&0x10 );
    /* A5 VLM5030 RST pin */
    if( changes & 0x20 )
        vlm5030_rst( device, offset&0x20 );

    last_addr = offset;
}



WRITE8_HANDLER( konami_sh_irqtrigger_w )
{
    static int last;

    if (last == 0 && data)
    {
        /* setting bit 0 low then high triggers IRQ on the sound CPU */
        cputag_set_input_line_and_vector(space->machine, "audiocpu", 0, HOLD_LINE, 0xff);
    }

    last = data;
}


WRITE8_HANDLER( konami_SN76496_latch_w )
{
    SN76496_latch = data;
}


WRITE8_DEVICE_HANDLER( konami_SN76496_w )
{
    sn76496_w(device, offset, SN76496_latch);
}
