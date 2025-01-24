// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    NuBus, SE/30 PDS, and LC slot cards

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "8lc.h"
#include "bootbug.h"
#include "laserview.h"
#include "nubus_48gc.h"
#include "nubus_asntmc3b.h"
#include "nubus_cb264.h"
#include "nubus_image.h"
#include "nubus_m2hires.h"
#include "nubus_m2video.h"
#include "nubus_radiustpd.h"
#include "nubus_spec8.h"
#include "nubus_specpdq.h"
#include "nubus_vikbw.h"
#include "nubus_wsportrait.h"
#include "pds30_30hr.h"
#include "pds30_cb264.h"
#include "pds30_mc30.h"
#include "pds30_procolor816.h"
#include "pds30_sigmalview.h"
#include "quadralink.h"
#include "thunder4gx.h"

void mac_nubus_cards(device_slot_interface &device)
{
	device.option_add("m2video",    NUBUS_M2VIDEO);     // Apple Macintosh II Video Card
	device.option_add("mdc48",      NUBUS_MDC48);       // Apple Macintosh Display Card 4•8
	device.option_add("mdc824",     NUBUS_MDC824);      // Apple Macintosh Display Card 8•24
	device.option_add("cb264",      NUBUS_CB264);       // RasterOps ColorBoard 264
	device.option_add("vikbw",      NUBUS_VIKBW);       // Moniterm Viking board
	device.option_add("image",      NUBUS_IMAGE);       // Disk Image Pseudo-Card
	device.option_add("specpdq",    NUBUS_SPECPDQ);     // SuperMac Spectrum PDQ
	device.option_add("m2hires",    NUBUS_M2HIRES);     // Apple Macintosh II Hi-Resolution Card
	device.option_add("spec8s3",    NUBUS_SPEC8S3);     // SuperMac Spectrum/8 Series III
	device.option_add("radiustpd",  NUBUS_RADIUSTPD);   // Radius Two Page Display
	device.option_add("asmc3nb",    NUBUS_ASNTMC3NB);   // Asante MC3NB Ethernet card
	device.option_add("portrait",   NUBUS_WSPORTRAIT);  // Apple Macintosh II Portrait video card
	device.option_add("enetnb",     NUBUS_APPLEENET);   // Apple NuBus Ethernet
	device.option_add("bootbug",    NUBUS_BOOTBUG);     // Brigent BootBug debugger card
	device.option_add("quadralink", NUBUS_QUADRALINK);  // AE Quadralink serial card
	device.option_add("laserview",  NUBUS_LASERVIEW);   // Sigma Designs LaserView monochrome video card
	device.option_add("thunder4gx", NUBUS_THUNDERIVGX); // Radius Thunder IV GX video card
}

void powermac_nubus_cards(device_slot_interface &device)
{
	device.option_add("mdc48", NUBUS_MDC48);            // Apple Macintosh Display Card 4•8
	device.option_add("mdc824", NUBUS_MDC824);          // Apple Macintosh Display Card 8•24
	device.option_add("cb264", NUBUS_CB264);            // RasterOps ColorBoard 264
	device.option_add("asmc3nb", NUBUS_ASNTMC3NB);      // Asante MC3NB Ethernet card
	device.option_add("enetnb", NUBUS_APPLEENET);       // Apple NuBus Ethernet
	device.option_add("quadralink", NUBUS_QUADRALINK);  // AE Quadralink serial card
	device.option_add("thunder4gx", NUBUS_THUNDERIVGX); // Radius Thunder IV GX video card
}

void mac_pds030_cards(device_slot_interface &device)
{
	device.option_add("cb264se30", PDS030_CB264SE30);   // RasterOps Colorboard 264/SE30
	device.option_add("pc816", PDS030_PROCOLOR816); // Lapis ProColor Server 8*16 PDS
	device.option_add("lview", PDS030_LVIEW);       // Sigma Designs L-View
	device.option_add("30hr",  PDS030_XCEED30HR);   // Micron/XCEED Technology Color 30HR
	device.option_add("mc30",  PDS030_XCEEDMC30);   // Micron/XCEED Technology MacroColor 30
	device.option_add("ethermac30", PDS030_ETHERMAC30I);    // Farallon EtherMac 30i Ethernet card
}

void mac_pdslc_cards(device_slot_interface &device)
{
	device.option_add("macconilc", PDSLC_MACCONILC);    // Asante MacCON i LC Ethernet card
}

// cards for the LC PDS slot that only work in the original LC
void mac_pdslc_orig_cards(device_slot_interface &device)
{
	device.option_add("macconilc", PDSLC_MACCONILC);    // Asante MacCON i LC Ethernet card
	device.option_add("ro8lc", PDSLC_COLORVUE8LC);      // RasterOps ColorVue 8LC
}

// IIsi can take either one SE/30 PDS or one NuBus card
void mac_iisi_cards(device_slot_interface &device)
{
	device.option_add("m2video", NUBUS_M2VIDEO);       // Apple Macintosh II Video Card
	device.option_add("mdc48", NUBUS_MDC48);           // Apple Macintosh Display Card 4•8
	device.option_add("mdc824", NUBUS_MDC824);         // Apple Macintosh Display Card 8•24
	device.option_add("cb264", NUBUS_CB264);           // RasterOps ColorBoard 264
	device.option_add("vikbw", NUBUS_VIKBW);           // Moniterm Viking board
	device.option_add("image", NUBUS_IMAGE);           // Disk Image Pseudo-Card
	device.option_add("specpdq", NUBUS_SPECPDQ);       // SuperMac Spectrum PDQ
	device.option_add("m2hires", NUBUS_M2HIRES);       // Apple Macintosh II Hi-Resolution Card
	device.option_add("spec8s3", NUBUS_SPEC8S3);       // SuperMac Spectrum/8 Series III
	device.option_add("radiustpd", NUBUS_RADIUSTPD);   // Radius Two Page Display
	device.option_add("asmc3nb", NUBUS_ASNTMC3NB);     // Asante MC3NB Ethernet card
	device.option_add("portrait", NUBUS_WSPORTRAIT);   // Apple Macintosh II Portrait video card
	device.option_add("enetnb", NUBUS_APPLEENET);      // Apple NuBus Ethernet
	device.option_add("bootbug", NUBUS_BOOTBUG);       // Brigent BootBug debugger card
	device.option_add("quadralink", NUBUS_QUADRALINK); // AE Quadralink serial card
	device.option_add("laserview", NUBUS_LASERVIEW);   // Sigma Designs LaserView monochrome video card
	device.option_add("thunder4gx", NUBUS_THUNDERIVGX); // Radius Thunder IV GX video card

	device.option_add("cb264se30", PDS030_CB264SE30);     // RasterOps Colorboard 264/SE30
	device.option_add("pc816", PDS030_PROCOLOR816);   // Lapis ProColor Server 8*16 PDS
	device.option_add("lview", PDS030_LVIEW);         // Sigma Designs L-View
	device.option_add("30hr", PDS030_XCEED30HR);      // Micron/XCEED Technology Color 30HR
	device.option_add("mc30", PDS030_XCEEDMC30);      // Micron/XCEED Technology MacroColor 30
	device.option_add("ethermac30", PDS030_ETHERMAC30I); // Farallon EtherMac 30i Ethernet card
}
