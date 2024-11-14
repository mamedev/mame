// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

    PIIX4E IDE interface

    TODO:
    - i82371ab PIIX4 / i82371mb PIIX4M dispatches

**************************************************************************************************/

#include "emu.h"
#include "i82371eb_ide.h"

#define LOG_IO     (1U << 1) // log PCI register accesses
#define LOG_TODO   (1U << 2) // log unimplemented registers
#define LOG_MAP    (1U << 3) // log full remaps

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_TODO | LOG_MAP)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)
#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)

DEFINE_DEVICE_TYPE(I82371EB_IDE, i82371eb_ide_device, "i82371eb_ide", "Intel 82371EB PIIX4E PCI to ISA southbridge")

i82371eb_ide_device::i82371eb_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82371sb_ide_device(mconfig, I82371EB_IDE, tag, owner, clock)
{
	// 0x010180 - Mass storage device, IDE controller, bus master capable
	// rev 0x00 PIIX4 A-0 / A-1
	// rev 0x01 PIIX4 B-0 / PIIX4E A-0 / PIIX4M A-0
	set_ids(0x80867111, 0x01, 0x010180, 0x00);
}
