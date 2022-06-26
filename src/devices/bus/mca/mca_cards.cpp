// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/**********************************************************************

    MCA cards

**********************************************************************/

#include "emu.h"
#include "mca_cards.h"

// Communications
#include "3c523.h"
#include "ibm_dual_async.h"
#include "planar_lpt.h"
#include "planar_uart.h"

// Memory
#include "curam.h"
#include "ibm_memory_exp_16.h"
#include "ibm_memory_exp_32.h"

// Multimedia
#include "adlib.h"
#include "macpa.h"
#include "snark_barker.h"

// Storage
#include "c1000.h"
#include "planar_fdc.h"

// Video
#include "ibm_svga.h"
#include "planar_vga.h"

// Template, not a real card - remove before submitting
#include "template.h"

// 16-bit MCA cards go here.
void pc_mca16_cards(device_slot_interface &device)
{
    // Communications
    device.option_add("3c523", MCA16_3C523);                        //  @6042
    device.option_add("ibm_dual_async", MCA16_IBM_DUAL_ASYNC);      //  @EEFF
    device.option_add("planar_lpt", MCA16_PLANAR_LPT);              //  (planar device)
    device.option_add("planar_uart", MCA16_PLANAR_UART);            //  (planar device)

    // Memory
    device.option_add("curam", MCA16_CURAM16_PLUS);                 //  @6025
    device.option_add("ibm_memory_exp_16", MCA16_IBM_MEMORY_EXP);   //  @F7F7

    // Multimedia
    device.option_add("adlib", MCA16_ADLIB);                        //  @70D7
    device.option_add("macpa", MCA16_MACPA);                        //  @6E6C
    device.option_add("snark_barker", MCA16_SNARK_BARKER);          //  @5085

    // Storage
    device.option_add("c1000", MCA16_C1000_IDE);                    //  @6213
    device.option_add("planar_fdc", MCA16_PLANAR_FDC);              //  (planar device)

    // Video
    device.option_add("ibm_svga", MCA16_IBM_SVGA);                  //  @917B
    device.option_add("planar_vga", MCA16_PLANAR_VGA);              //  (planar device)
}

// 32-bit MCA cards go here.
void pc_mca32_cards(device_slot_interface &device)
{
    pc_mca16_cards(device); // They're compatible.

    device.option_add("ibm_memory_exp_32", MCA32_IBM_MEMORY_EXP);   //  @FCFF
}