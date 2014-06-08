/*
    HDC9234 Hard and Floppy Disk Controller

    This controller handles MFM and FM encoded floppy disks and hard disks.
    The SMC9224 is used in some DEC systems.  The HDC9234 is used in the
    Myarc HFDC card for the TI99/4A.

    References:
    * SMC HDC9234 preliminary data book (1988)

    Michael Zapf, June 2014
*/

#include "emu.h"
#include "hdc9234.h"

hdc9234_device::hdc9234_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, HDC9234, "SMC HDC9234 Universal Disk Controller", tag, owner, clock, "hdc9234", __FILE__),
	m_out_intrq(*this),
	m_out_dip(*this),
	m_out_auxbus(*this),
	m_in_auxbus(*this),
	m_in_dma(*this),
	m_out_dma(*this)
{
}

/*
    Read a byte of data from the controller
    The address (offset) encodes the C/D* line (command and /data)
*/
READ8_MEMBER( hdc9234_device::read )
{
	logerror("%s: Read access to %04x\n", tag(), offset & 0xffff);
	return 0;
}

/*
    Write a byte to the controller
    The address (offset) encodes the C/D* line (command and /data)
*/
WRITE8_MEMBER( hdc9234_device::write )
{
	logerror("%s: Write access to %04x: %d\n", tag(), offset & 0xffff, data);
}

void hdc9234_device::device_start()
{
	logerror("%s: start\n", tag());
	m_out_intrq.resolve_safe();
	m_out_dip.resolve_safe();
	m_out_auxbus.resolve_safe();
	m_in_auxbus.resolve_safe(0);
	m_out_dma.resolve_safe();
	m_in_dma.resolve_safe(0);

	// allocate timers
}

void hdc9234_device::device_reset()
{
	logerror("%s: reset\n", tag());
}

const device_type HDC9234 = &device_creator<hdc9234_device>;
