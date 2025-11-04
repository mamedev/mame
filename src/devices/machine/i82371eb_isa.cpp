// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

    PIIX4E ISA interface

    TODO:
    - i82371ab PIIX4 / i82371mb PIIX4M dispatches
    - pinpoint actual differences wrt i82371sb (definitely EISA, then ...?)

**************************************************************************************************/

#include "emu.h"
#include "i82371eb_isa.h"

#define LOG_IO     (1U << 1) // log PCI register accesses
#define LOG_TODO   (1U << 2) // log unimplemented registers
#define LOG_MAP    (1U << 3) // log full remaps

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_TODO | LOG_MAP)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)
#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)

DEFINE_DEVICE_TYPE(I82371EB_ISA, i82371eb_isa_device, "i82371eb_isa", "Intel 82371EB PIIX4E PCI to ISA/EIO southbridge")

i82371eb_isa_device::i82371eb_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82371sb_isa_device(mconfig, I82371EB_ISA, tag, owner, clock)

{
	// 0x060100 - Bridge device, PCI-to-ISA bridge
	// TODO: above can change to 0x068000 if positive decode is used.
	// rev 0x00 PIIX4 A-0 / A-1
	// rev 0x01 PIIX4 B-0
	// rev 0x02 for PIIX4E A-0 / PIIX4M A-0
	set_ids(0x80867110, 0x02, 0x060100, 0x00);
}

void i82371eb_isa_device::config_map(address_map &map)
{
	i82371sb_isa_device::config_map(map);
//  map(0x90, 0x91) PDMACFG
//  map(0xb0, 0xb0) GENCFG

}

void i82371eb_isa_device::internal_io_map(address_map &map)
{
	i82371sb_isa_device::internal_io_map(map);
	map(0x00eb, 0x00eb).lw8(NAME([] (offs_t offset, u8 data) { }));
}
