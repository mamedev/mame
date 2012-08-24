/***************************************************************************

        Hawthorne Technologies TinyGiant HT68k

        29/11/2009 Skeleton driver.

Monitor commands (most do unknown things) (must be in Uppercase)
B boot
C compare memory blocks
D dump memory to screen
E Edit memory (. to escape)
F
G
K
L
M
P
R
S
T
W
X (. to escape)

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/68681.h"
#include "formats/hxcmfm_dsk.h"
#include "formats/mfi_dsk.h"
#include "imagedev/flopdrv.h"
#include "machine/wd1772.h"
#include "machine/terminal.h"


class ht68k_state : public driver_device
{
public:
	ht68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_terminal(*this, TERMINAL_TAG),
	m_duart(*this, "duart68681"),
	m_fdc(*this, "wd1770")
	,
		m_p_ram(*this, "p_ram"){ }

		
	static const floppy_format_type floppy_formats[];
		
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device<device_t> m_duart;
	required_device<wd1770_t> m_fdc;
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_WRITE_LINE_MEMBER(ht68k_fdc_intrq_w);
	required_shared_ptr<UINT16> m_p_ram;
};


const floppy_format_type ht68k_state::floppy_formats[] = {
	FLOPPY_MFM_FORMAT, FLOPPY_MFI_FORMAT,
	NULL
};


static ADDRESS_MAP_START(ht68k_mem, AS_PROGRAM, 16, ht68k_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM AM_SHARE("p_ram") // 512 KB RAM / ROM at boot
	//AM_RANGE(0x00080000, 0x000fffff) // Expansion
	//AM_RANGE(0x00d80000, 0x00d8ffff) // Printer
	AM_RANGE(0x00e00000, 0x00e00007) AM_MIRROR(0xfff8) AM_DEVREADWRITE8("wd1770", wd177x_t, read, write, 0x00ff) // FDC WD1770
	AM_RANGE(0x00e80000, 0x00e800ff) AM_MIRROR(0xff00) AM_DEVREADWRITE8_LEGACY("duart68681", duart68681_r, duart68681_w, 0xff )
	AM_RANGE(0x00f00000, 0x00f07fff) AM_ROM AM_MIRROR(0xf8000) AM_REGION("user1",0)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ht68k )
INPUT_PORTS_END


static MACHINE_RESET(ht68k)
{
	ht68k_state *state = machine.driver_data<ht68k_state>();
	UINT8* user1 = state->memregion("user1")->base();

	memcpy((UINT8*)state->m_p_ram.target(),user1,0x8000);

	machine.device("maincpu")->reset();
}

static void duart_irq_handler(device_t *device, int state, UINT8 vector)
{
	cputag_set_input_line_and_vector(device->machine(), "maincpu", M68K_IRQ_3, state, M68K_INT_ACK_AUTOVECTOR);
}

static void duart_tx(device_t *device, int channel, UINT8 data)
{
	ht68k_state *state = device->machine().driver_data<ht68k_state>();
	state->m_terminal->write(*device->machine().memory().first_space(), 0, data);
}

static UINT8 duart_input(device_t *device)
{
	return 0;
}

static void duart_output(device_t *device, UINT8 data)
{
	ht68k_state *state = device->machine().driver_data<ht68k_state>();

	static const char *names[] = { "fd0", "fd1", "fd2", "fd3" };
	floppy_image_device *floppy = 0;
	for(int i=0; i<4; i++) {
		if(BIT(data, 7-i)==0) {
			floppy_connector *con = device->machine().device<floppy_connector>(names[i]);
			if(con)
				floppy = con->get_device();
			break;
		}		
	}
	if (floppy) floppy->ss_w(BIT(data,3) ? 0 : 1);
	state->m_fdc->set_floppy(floppy);		
}

WRITE8_MEMBER( ht68k_state::kbd_put )
{
	duart68681_rx_data(m_duart, 0, data);
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(ht68k_state, kbd_put)
};

static const duart68681_config ht68k_duart68681_config =
{
	duart_irq_handler,
	duart_tx,
	duart_input,
	duart_output
};

static SLOT_INTERFACE_START( ht68k_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END


static MACHINE_CONFIG_START( ht68k, ht68k_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(ht68k_mem)

	MCFG_MACHINE_RESET(ht68k)

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)

	MCFG_DUART68681_ADD( "duart68681", XTAL_8MHz / 2, ht68k_duart68681_config )

	MCFG_WD1770x_ADD("wd1770", XTAL_8MHz )

	MCFG_FLOPPY_DRIVE_ADD("fd0", ht68k_floppies, "525dd", 0, ht68k_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd1", ht68k_floppies, "525dd", 0, ht68k_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd2", ht68k_floppies, "525dd", 0, ht68k_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd3", ht68k_floppies, "525dd", 0, ht68k_state::floppy_formats)	
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ht68k )
	ROM_REGION( 0x8000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ht68k-u4.bin", 0x0000, 0x4000, CRC(3fbcdd0a) SHA1(45fcbbf920dc1e9eada3b7b0a55f5720d08ffdd5))
	ROM_LOAD16_BYTE( "ht68k-u3.bin", 0x0001, 0x4000, CRC(1d85d101) SHA1(8ba01e1595b0b3c4fb128a4a50242f3588b89c43))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                     FULLNAME                    FLAGS */
COMP( 1987, ht68k,  0,       0,      ht68k,     ht68k, driver_device,   0,   "Hawthorne Technologies", "TinyGiant HT68k", GAME_NO_SOUND)
