/*****************************************************************************
 *
 *   tlcs_z80.c
 *   TOSHIBA TLCS Z80 emulation
 */

#include "emu.h"
#include "z80.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"

//TODO: These interfaces should default to DEVCB_NULL pointers and
// the actual callbacks should be provided by the driver that instantiates the TLCS-Z80 CPU.
// We need methods for the driver to provide these interface configurations to the CPU core.
// something like:
//  m_tlcsz80->set_internal_ctc_interface (ctc_intf);
//  m_tlcsz80->set_internal_pio_interface (pio_intf);
//  m_tlcsz80->set_internal_sio_interface (sio_intf);

static Z80CTC_INTERFACE( ctc_intf )
{
	DEVCB_CPU_INPUT_LINE(DEVICE_SELF_OWNER, INPUT_LINE_IRQ0), /* interrupt handler */
	DEVCB_NULL, /* ZC/TO0 callback */
	DEVCB_NULL, /* ZC/TO1 callback */
	DEVCB_NULL  /* ZC/TO2 callback */
};

static Z80PIO_INTERFACE( pio_intf )
{
	DEVCB_CPU_INPUT_LINE(DEVICE_SELF_OWNER, INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static const z80sio_interface sio_intf =
{
	DEVCB_CPU_INPUT_LINE(DEVICE_SELF_OWNER, INPUT_LINE_IRQ0), /* interrupt handler */
	DEVCB_NULL, /* DTR changed handler */
	DEVCB_NULL, /* RTS changed handler */
	DEVCB_NULL, /* BREAK changed handler */
	DEVCB_NULL, /* transmit handler */
	DEVCB_NULL  /* receive handler */
};

/* Daisy Chaining */

#ifdef UNUSED
static const z80_daisy_config tlcsz80_daisy_chain[] =
{
	{ TLCSZ80_INTERNAL_CTC_TAG },
	{ TLCSZ80_INTERNAL_PIO_TAG },
	{ TLCSZ80_INTERNAL_SIO_TAG },
	{ NULL }
};
#endif

static ADDRESS_MAP_START( tlcs_z80_internal_io_map, AS_IO, 8, tlcs_z80_device )
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE(TLCSZ80_INTERNAL_CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x18, 0x1B) AM_DEVREADWRITE(TLCSZ80_INTERNAL_SIO_TAG, z80sio_device, read, write)
	AM_RANGE(0x1C, 0x1F) AM_DEVREADWRITE(TLCSZ80_INTERNAL_PIO_TAG, z80pio_device, read, write)
//  AM_RANGE(0xF0, 0xF0) TODO: Watchdog Timer: Stand-by mode Register
//  AM_RANGE(0xF1, 0xF1) TODO: Watchdog Timer: command Register
//  AM_RANGE(0xF4, 0xF4) TODO: Daisy chain interrupt precedence Register
ADDRESS_MAP_END

//This is wrong!
//We should use the same clock as declared in the TLCS_Z80 instantiation in the driver that uses it.
#define TLCS_Z80_CLOCK 8000000

static MACHINE_CONFIG_FRAGMENT( tlcs_z80 )
	MCFG_Z80CTC_ADD(TLCSZ80_INTERNAL_CTC_TAG, TLCS_Z80_CLOCK, ctc_intf)
	MCFG_Z80SIO_ADD(TLCSZ80_INTERNAL_SIO_TAG, TLCS_Z80_CLOCK, sio_intf)
	MCFG_Z80PIO_ADD(TLCSZ80_INTERNAL_PIO_TAG, TLCS_Z80_CLOCK, pio_intf)
MACHINE_CONFIG_END

tlcs_z80_device::tlcs_z80_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80_device(mconfig, TLCS_Z80, "TLCS-Z80", tag, owner, clock, "tlcs_z80", __FILE__),
		m_z80ctc(*this, TLCSZ80_INTERNAL_CTC_TAG),
		m_io_space_config( "io", ENDIANNESS_LITTLE, 8, 8, 0, ADDRESS_MAP_NAME( tlcs_z80_internal_io_map ) )
	{ }


WRITE_LINE_MEMBER( tlcs_z80_device::ctc_trg0 ) { m_z80ctc->trg0(state ? 0 : 1); }
WRITE_LINE_MEMBER( tlcs_z80_device::ctc_trg1 ) { m_z80ctc->trg1(state ? 0 : 1); }
WRITE_LINE_MEMBER( tlcs_z80_device::ctc_trg2 ) { m_z80ctc->trg2(state ? 0 : 1); }
WRITE_LINE_MEMBER( tlcs_z80_device::ctc_trg3 ) { m_z80ctc->trg3(state ? 0 : 1); }


machine_config_constructor tlcs_z80_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( tlcs_z80 );
}

const device_type TLCS_Z80 = &device_creator<tlcs_z80_device>;
