
#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "imagedev/flopdrv.h"
#include "machine/ctronics.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/ieee488.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/ram.h"
#include "machine/terminal.h"
#include "machine/upd765.h"

#define M68000_TAG		"u68"
#define I8255A_0_TAG	"u22"
#define I8255A_1_TAG	"u39"
#define I8253_0_TAG		"u74"
#define I8253_1_TAG		"u75"
#define I8259_TAG		"u73"
#define I8251_0_TAG		"u58"
#define I8251_1_TAG		"u67"
#define UPD765_TAG		"u21"
#define TMS9914_TAG		"u6"
#define CENTRONICS_TAG	"centronics"

class sage2_state : public driver_device
{
public:
	sage2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, M68000_TAG),
		  m_pic(*this, I8259_TAG),
		  m_usart0(*this, I8251_0_TAG),
		  m_usart1(*this, I8251_1_TAG),
		  m_fdc(*this, UPD765_TAG),
		  m_ram(*this, RAM_TAG),
		  m_floppy0(*this, UPD765_TAG ":0:525dd"),
		  m_floppy1(*this, UPD765_TAG ":1:525dd"),
		  m_centronics(*this, CENTRONICS_TAG),
		  m_ieee488(*this, IEEE488_TAG),
		  m_terminal(*this, TERMINAL_TAG),
		  m_reset(1),
		  m_fdc_int(0),
		  m_fdie(0),
		  m_sl0(1),
		  m_sl1(1)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device<i8251_device> m_usart0;
	required_device<i8251_device> m_usart1;
	required_device<upd765a_device> m_fdc;
	required_device<ram_device> m_ram;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;
	required_device<centronics_device> m_centronics;
	required_device<ieee488_device> m_ieee488;
	required_device<generic_terminal_device> m_terminal;

	virtual void machine_start();
	virtual void machine_reset();

	void update_fdc_int();

	DECLARE_READ8_MEMBER( mmu_r );
	DECLARE_WRITE8_MEMBER( mmu_w );
	DECLARE_WRITE_LINE_MEMBER( br1_w );
	DECLARE_WRITE_LINE_MEMBER( br2_w );
	DECLARE_WRITE8_MEMBER( ppi0_pc_w );
	DECLARE_READ8_MEMBER( ppi1_pb_r );
	DECLARE_WRITE8_MEMBER( ppi1_pc_w );
	DECLARE_WRITE_LINE_MEMBER( ack_w );

	DECLARE_WRITE8_MEMBER(kbd_put);

	DECLARE_DIRECT_UPDATE_MEMBER(sage2_direct_update_handler);

	void fdc_irq(bool state);

	int m_reset;

	// floppy state
	int m_fdc_int;
	int m_fdie;
	int m_sl0;
	int m_sl1;
	DECLARE_DRIVER_INIT(sage2);
};
