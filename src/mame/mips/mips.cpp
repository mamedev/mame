// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An emulation of systems designed and manufactured by MIPS Computer Systems,
 * all of which use MIPS R2000, R3000 or R6000 CPUs, and run the RISC/os
 * operating system.
 *
 * The drivers are intended to eventually cover the following models:
 *
 *   Model       Board    CPU      Clock  Slots    Disk  Package       Other
 *   M/500       R2300    R2000     5MHz  VME      ESDI
 *   M/800       R2600    R2000     8MHz  VME      ESDI
 *   M/1000      R2800    R2000    10MHz  VME      ESDI
 *   M/120-3     R2400    R2000  12.5MHz  PC-AT    SCSI  Deskside      aka Intrepid
 *   M/120-5     R2400    R2000    16MHz  PC-AT    SCSI  Deskside
 *   M/180       R2400
 *   M/2000-6    R3200    R3000    20MHz  VMEx13   SMD   Rack Cabinet
 *   M/2000-8    R3200    R3000    25MHz  VMEx13   SMD   Rack Cabinet
 *   M/2000-?    RB3125   R3000    33MHz
 *   RC2030      I2000    R2000    16MHz           SCSI  Desktop       aka M/12, Jupiter
 *   RS2030      I2000    R2000    16MHz           SCSI  Desktop       aka M/12, Jupiter
 *   RC3230      R3030    R3000    25MHz  PC-ATx1  SCSI  Desktop       aka M/20, Pizazz
 *   RS3230      R3030    R3000    25MHz  PC-ATx1  SCSI  Desktop       aka M/20, Pizazz, Magnum 3000
 *   RC3240      R2400    R3000    25MHz  PC-ATx4  SCSI  Deskside      M/120 with CPU-board upgrade
 *   RC3330               R3000    33MHz  PC-AT    SCSI  Desktop
 *   RS3330               R3000    33MHz  PC-AT    SCSI  Desktop
 *   RC3260               R3000    25MHz  VMEx7    SCSI  Pedestal
 *   RC3360      RB3133   R3000    33MHz  VME      SCSI  Pedestal
 *   RC3370      RB3133
 *   RC6260      R6300    R6000    66MHz  VME      SCSI  Pedestal
 *   RC6280      R6300    R6000    66MHz  VMEx6    SMD   Data Center
 *   RC6380-100           R6000x1  66MHz  VME      SMD   Data Center
 *   RC6380-200           R6000x2  66MHz  VME      SMD   Data Center
 *   RC6380-400           R6000x4  66MHz  VME      SMD   Data Center
 *
 * Sources:
 *
 *   http://www.umips.net/
 *   http://www.geekdot.com/the-mips-rs2030/
 *   http://www.jp.netbsd.org/ports/mipsco/models.html
 *   http://www.prumpleffer.de/~miod/machineroom/machines/mips/magnum/index.html
 *   https://web.archive.org/web/20140518203135/http://no-l.org/pages/riscos.html
 *
 * See individual board driver sources for details.
 */

#include "emu.h"
#include "mips.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/tape.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/rs232.h"
#include "machine/nscsi_bus.h"


void mips_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("tape", NSCSI_TAPE);
}

void mips_rs232_devices(device_slot_interface &device)
{
	default_rs232_devices(device);

	device.option_add("mouse", MSYSTEMS_HLE_SERIAL_MOUSE);
}
