// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont, Juergen Buchmueller
/******************************************************************************
 *
 *  Acorn Archimedes
 *
 *  Skeleton: Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
 *  Enhanced: R. Belmont, June 2007
 *  Angelo Salese, August 2010
 *
 *  AKB10 - Archimedes 305
 *  AKB15 - Archimedes 310
 *  AKB20 - Archimedes 440
 *  AKB26 - Archimedes 410 (advertised but not known to be produced)
 *  AKB40 - Archimedes 410/1
 *  AKB42 - Archimedes 420/1
 *  AKB50 - Archimedes 440/1
 *  AKB01 - BBC A3000
 *  AKB50 - Archimedes 540
 *  ALB22 - Acorn A5000 2MB HD 80
 *  ALB24 - Acorn A5000 4MB HD 120
 *  AKB62 - Acorn A4 2MB FD
 *  AKB64 - Acorn A4 4MB HD 60
 *  AGB11 - Acorn A3010
 *  AGB22 - Acorn A3020 FD
 *  AGB23 - Acorn A3020 HD 60
 *  AGB33 - Acorn A020 HD 80
 *  AGC10 - Acorn A4000
 *  AGC20 - Acorn A4000 2MB HD 80
 *
 * Notes:
 * - default NVRAM is plainly wrong. Use the status/configure commands to set up properly
 *   (Scroll Lock is currently mapped with Right SHIFT, use this to move to next page of status).
 *   In order to load a floppy, you need at very least:
 *   configure floppies 2
 *   configure filesystem adfs
 *   configure monitortype 12
 *   Then reboot / reset the machine, and use cat to (attempt) to load a floppy contents.
 *
 *  TODO:
 *  - RISC OS Alarm app crash the whole OS
 *  - RISC OS Draw app uses unimplemented copro instructions
 *
 *
 *
=======================================================================================
 *
 *      Memory map (from http://b-em.bbcmicro.com/arculator/archdocs.txt)
 *
 *  0000000 - 1FFFFFF - logical RAM (32 meg)
 *  2000000 - 2FFFFFF - physical RAM (supervisor only - max 16MB - requires quad MEMCs)
 *  3000000 - 33FFFFF - IOC (IO controllers - supervisor only)
 *  3310000 - FDC - WD1772
 *  33A0000 - Econet - 6854
 *  33B0000 - Serial - 6551
 *  3240000 - 33FFFFF - internal expansion cards
 *  32D0000 - hard disc controller (not IDE) - HD63463
 *  3350010 - printer
 *  3350018 - latch A
 *  3350040 - latch B
 *  3270000 - external expansion cards
 *
 *  3400000 - 3FFFFFF - ROM (read - 12 meg - Arthur and RiscOS 2 512k, RiscOS 3 2MB)
 *  3400000 - 37FFFFF - Low ROM  (4 meg, I think this is expansion ROMs)
 *  3800000 - 3FFFFFF - High ROM (main OS ROM)
 *
 *  3400000 - 35FFFFF - VICD10 (write - supervisor only)
 *  3600000 - 3FFFFFF - MEMC (write - supervisor only)
 *
 *****************************************************************************/
/*
    DASM of code (bios 2 / RISC OS 2)
    0x380d4e0: MEMC: control to 0x10c (page size 32 kbytes, DRAM ram refresh only during flyback)
    0x380d4f0: VIDC: params (screen + sound frequency)
    0x380d51c: IOC: sets control to 0xff, clear IRQA and FIQ masks, sets IRQB mask to 0x80 (keyboard receive full irq)
    0x380d530: IOC: sets timer 0 to 0x4e20, go command
        0x380e0a8: work RAM physical check, max size etc.
    0x380e1f8: IOC: Disables DRAM ram refresh, sets timer 1 to 0x7ffe, go command, then it tests the latch of this timer, enables DRAM refresh
        0x380d00c: Set up default logical space
        0x380d16c: Set up case by case logical space


*/


#include "emu.h"
#include "includes/archimds.h"

#include "cpu/arm/arm.h"
#include "formats/acorn_dsk.h"
#include "formats/apd_dsk.h"
#include "formats/jfd_dsk.h"
#include "formats/pc_dsk.h"
#include "machine/i2cmem.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "sound/volt_reg.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

class aa310_state : public archimedes_state
{
public:
	aa310_state(const machine_config &mconfig, device_type type, const char *tag)
		: archimedes_state(mconfig, type, tag)
		, m_physram(*this, "physicalram")
		, m_ram(*this, RAM_TAG)
	{ }

	void aa5000a(machine_config &config);
	void aa305(machine_config &config);
	void aa310(machine_config &config);
	void aa3000(machine_config &config);
	void aa5000(machine_config &config);
	void aa4101(machine_config &config);
	void aa3020(machine_config &config);
	void aa4401(machine_config &config);
	void aa3010(machine_config &config);
	void aa4(machine_config &config);
	void aa4000(machine_config &config);
	void aa540(machine_config &config);
	void aa440(machine_config &config);
	void aa4201(machine_config &config);

	void init_aa310();

	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);
	DECLARE_INPUT_CHANGED_MEMBER(send_mouse_input);

private:
	required_shared_ptr<uint32_t> m_physram;

	DECLARE_READ32_MEMBER(aa310_psy_wram_r);
	DECLARE_WRITE32_MEMBER(aa310_psy_wram_w);
	DECLARE_WRITE_LINE_MEMBER(aa310_wd177x_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(aa310_wd177x_drq_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	void aa310_mem(address_map &map);

	required_device<ram_device> m_ram;
};


WRITE_LINE_MEMBER(aa310_state::aa310_wd177x_intrq_w)
{
	if (state)
		archimedes_request_fiq(ARCHIMEDES_FIQ_FLOPPY);
	else
		archimedes_clear_fiq(ARCHIMEDES_FIQ_FLOPPY);
}

WRITE_LINE_MEMBER(aa310_state::aa310_wd177x_drq_w)
{
	if (state)
		archimedes_request_fiq(ARCHIMEDES_FIQ_FLOPPY_DRQ);
	else
		archimedes_clear_fiq(ARCHIMEDES_FIQ_FLOPPY_DRQ);
}

READ32_MEMBER(aa310_state::aa310_psy_wram_r)
{
	return m_physram[offset];
}

WRITE32_MEMBER(aa310_state::aa310_psy_wram_w)
{
	COMBINE_DATA(&m_physram[offset]);
}


void aa310_state::init_aa310()
{
	uint32_t ram_size = m_ram->size();

	m_maincpu->space(AS_PROGRAM).unmap_readwrite(0x02000000, 0x02ffffff);
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler( 0x02000000, 0x02000000+(ram_size-1), read32_delegate(FUNC(aa310_state::aa310_psy_wram_r), this), write32_delegate(FUNC(aa310_state::aa310_psy_wram_w), this));

	archimedes_driver_init();
}

void aa310_state::machine_start()
{
	archimedes_init();
}

void aa310_state::machine_reset()
{
	archimedes_reset();
}

void aa310_state::aa310_mem(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(FUNC(aa310_state::archimedes_memc_logical_r), FUNC(aa310_state::archimedes_memc_logical_w));
	map(0x02000000, 0x02ffffff).ram().share("physicalram"); /* physical RAM - 16 MB for now, should be 512k for the A310 */
	map(0x03000000, 0x033fffff).rw(FUNC(aa310_state::archimedes_ioc_r), FUNC(aa310_state::archimedes_ioc_w));
	map(0x03400000, 0x035fffff).rom().region("extension", 0x000000).w(m_vidc, FUNC(acorn_vidc10_device::write));
	map(0x03600000, 0x037fffff).rom().region("extension", 0x200000).w(FUNC(aa310_state::archimedes_memc_w));
	map(0x03800000, 0x03ffffff).rom().region("maincpu", 0).w(FUNC(aa310_state::archimedes_memc_page_w));
}


INPUT_CHANGED_MEMBER(aa310_state::key_stroke)
{
	uint8_t row_val = uint8_t(param) >> 4;
	uint8_t col_val = uint8_t(param) & 0xf;

	if(newval && !oldval)
		m_kart->send_keycode_down(row_val,col_val);

	if(oldval && !newval)
		m_kart->send_keycode_up(row_val,col_val);
}

INPUT_CHANGED_MEMBER(aa310_state::send_mouse_input)
{
	int x = ioport("MOUSEX")->read();
	int y = ioport("MOUSEY")->read();

	if (x > 0x7fff) x = x - 0xffff;
	if (y > 0x7fff) y = y - 0xffff;
	if (x > 63)    x = 63;
	if (y > 63)    y = 63;
	if (x < -63)   x = -63;
	if (y < -63)   y = -63;

	m_kart->send_mouse(x & 0x7f, y & 0x7f);
}


static INPUT_PORTS_START( aa310 )
	PORT_START("dip") /* DIP switches */
	PORT_BIT(0xfd, 0xfd, IPT_UNUSED)

	PORT_START("key0") /* KEY ROW 0 */
	PORT_BIT(0x0001, 0x00, IPT_KEYBOARD) PORT_NAME("Esc")         PORT_CODE(KEYCODE_ESC)         PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x00)
	PORT_BIT(0x0002, 0x00, IPT_KEYBOARD) PORT_NAME("F1")          PORT_CODE(KEYCODE_F1)          PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x01)
	PORT_BIT(0x0004, 0x00, IPT_KEYBOARD) PORT_NAME("F2")          PORT_CODE(KEYCODE_F2)          PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x02)
	PORT_BIT(0x0008, 0x00, IPT_KEYBOARD) PORT_NAME("F3")          PORT_CODE(KEYCODE_F3)          PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x03)
	PORT_BIT(0x0010, 0x00, IPT_KEYBOARD) PORT_NAME("F4")          PORT_CODE(KEYCODE_F4)          PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x04)
	PORT_BIT(0x0020, 0x00, IPT_KEYBOARD) PORT_NAME("F5")          PORT_CODE(KEYCODE_F5)          PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x05)
	PORT_BIT(0x0040, 0x00, IPT_KEYBOARD) PORT_NAME("F6")          PORT_CODE(KEYCODE_F6)          PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x06)
	PORT_BIT(0x0080, 0x00, IPT_KEYBOARD) PORT_NAME("F7")          PORT_CODE(KEYCODE_F7)          PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x07)
	PORT_BIT(0x0100, 0x00, IPT_KEYBOARD) PORT_NAME("F8")          PORT_CODE(KEYCODE_F8)          PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x08)
	PORT_BIT(0x0200, 0x00, IPT_KEYBOARD) PORT_NAME("F9")          PORT_CODE(KEYCODE_F9)          PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x09)
	PORT_BIT(0x0400, 0x00, IPT_KEYBOARD) PORT_NAME("F10")         PORT_CODE(KEYCODE_F10)         PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x0a)
	PORT_BIT(0x0800, 0x00, IPT_KEYBOARD) PORT_NAME("F11")         PORT_CODE(KEYCODE_F11)         PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x0b)
	PORT_BIT(0x1000, 0x00, IPT_KEYBOARD) PORT_NAME("F12")         PORT_CODE(KEYCODE_F12)         PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x0c)
	PORT_BIT(0x2000, 0x00, IPT_KEYBOARD) PORT_NAME("Print")       PORT_CODE(KEYCODE_PRTSCR)      PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x0d)
	PORT_BIT(0x4000, 0x00, IPT_KEYBOARD) PORT_NAME("Scroll")    /*PORT_CODE(KEYCODE_SCRLOCK)*/   PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x0e)
	PORT_BIT(0x8000, 0x00, IPT_KEYBOARD) PORT_NAME("Break")       PORT_CODE(KEYCODE_PAUSE)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x0f)

	PORT_START("key1") /* KEY ROW 1 */
	PORT_BIT(0x0001, 0x00, IPT_KEYBOARD) PORT_NAME("`  ~")        PORT_CODE(KEYCODE_TILDE)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x10)
	PORT_BIT(0x0002, 0x00, IPT_KEYBOARD) PORT_NAME("1  !")        PORT_CODE(KEYCODE_1)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x11)
	PORT_BIT(0x0004, 0x00, IPT_KEYBOARD) PORT_NAME("2  \"")       PORT_CODE(KEYCODE_2)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x12)
	PORT_BIT(0x0008, 0x00, IPT_KEYBOARD) PORT_NAME("3  #")        PORT_CODE(KEYCODE_3)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x13)
	PORT_BIT(0x0010, 0x00, IPT_KEYBOARD) PORT_NAME("4  $")        PORT_CODE(KEYCODE_4)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x14)
	PORT_BIT(0x0020, 0x00, IPT_KEYBOARD) PORT_NAME("5  %")        PORT_CODE(KEYCODE_5)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x15)
	PORT_BIT(0x0040, 0x00, IPT_KEYBOARD) PORT_NAME("6  &")        PORT_CODE(KEYCODE_6)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x16)
	PORT_BIT(0x0080, 0x00, IPT_KEYBOARD) PORT_NAME("7  '")        PORT_CODE(KEYCODE_7)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x17)
	PORT_BIT(0x0100, 0x00, IPT_KEYBOARD) PORT_NAME("8  *")        PORT_CODE(KEYCODE_8)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x18)
	PORT_BIT(0x0200, 0x00, IPT_KEYBOARD) PORT_NAME("9  (")        PORT_CODE(KEYCODE_9)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x19)
	PORT_BIT(0x0400, 0x00, IPT_KEYBOARD) PORT_NAME("0  )")        PORT_CODE(KEYCODE_0)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x1a)
	PORT_BIT(0x0800, 0x00, IPT_KEYBOARD) PORT_NAME("-  _")        PORT_CODE(KEYCODE_MINUS)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x1b)
	PORT_BIT(0x1000, 0x00, IPT_KEYBOARD) PORT_NAME("=  +")        PORT_CODE(KEYCODE_EQUALS)      PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x1c)
	PORT_BIT(0x2000, 0x00, IPT_KEYBOARD) PORT_NAME("\xc2\xa3")    PORT_CODE(KEYCODE_BACKSLASH2)  PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x1d)
	PORT_BIT(0x4000, 0x00, IPT_KEYBOARD) PORT_NAME("Back Space")  PORT_CODE(KEYCODE_BACKSPACE)   PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x1e)
	PORT_BIT(0x8000, 0x00, IPT_KEYBOARD) PORT_NAME("Insert")      PORT_CODE(KEYCODE_INSERT)      PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x1f)

	PORT_START("key2") /* KEY ROW 2 */
	PORT_BIT(0x0001, 0x00, IPT_KEYBOARD) PORT_NAME("Home")        PORT_CODE(KEYCODE_HOME)        PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x20)
	PORT_BIT(0x0002, 0x00, IPT_KEYBOARD) PORT_NAME("PG UP")       PORT_CODE(KEYCODE_PGUP)        PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x21)
	PORT_BIT(0x0004, 0x00, IPT_KEYBOARD) PORT_NAME("Numlock")     PORT_CODE(KEYCODE_NUMLOCK)     PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x22)
	PORT_BIT(0x0008, 0x00, IPT_KEYBOARD) PORT_NAME("/")           PORT_CODE(KEYCODE_SLASH_PAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x23)
	PORT_BIT(0x0010, 0x00, IPT_KEYBOARD) PORT_NAME("*")           PORT_CODE(KEYCODE_ASTERISK)    PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x24)
	PORT_BIT(0x0020, 0x00, IPT_KEYBOARD) PORT_NAME("#")                                          PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x25)
	PORT_BIT(0x0040, 0x00, IPT_KEYBOARD) PORT_NAME("TAB")         PORT_CODE(KEYCODE_TAB)         PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x26)
	PORT_BIT(0x0080, 0x00, IPT_KEYBOARD) PORT_NAME("q  Q")        PORT_CODE(KEYCODE_Q)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x27)
	PORT_BIT(0x0100, 0x00, IPT_KEYBOARD) PORT_NAME("w  W")        PORT_CODE(KEYCODE_W)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x28)
	PORT_BIT(0x0200, 0x00, IPT_KEYBOARD) PORT_NAME("e  E")        PORT_CODE(KEYCODE_E)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x29)
	PORT_BIT(0x0400, 0x00, IPT_KEYBOARD) PORT_NAME("r  R")        PORT_CODE(KEYCODE_R)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x2a)
	PORT_BIT(0x0800, 0x00, IPT_KEYBOARD) PORT_NAME("t  T")        PORT_CODE(KEYCODE_T)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x2b)
	PORT_BIT(0x1000, 0x00, IPT_KEYBOARD) PORT_NAME("y  Y")        PORT_CODE(KEYCODE_Y)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x2c)
	PORT_BIT(0x2000, 0x00, IPT_KEYBOARD) PORT_NAME("u  U")        PORT_CODE(KEYCODE_U)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x2d)
	PORT_BIT(0x4000, 0x00, IPT_KEYBOARD) PORT_NAME("i  I")        PORT_CODE(KEYCODE_I)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x2e)
	PORT_BIT(0x8000, 0x00, IPT_KEYBOARD) PORT_NAME("o  O")        PORT_CODE(KEYCODE_O)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x2f)

	PORT_START("key3") /* KEY ROW 3 */
	PORT_BIT(0x0001, 0x00, IPT_KEYBOARD) PORT_NAME("p  P")        PORT_CODE(KEYCODE_P)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x30)
	PORT_BIT(0x0002, 0x00, IPT_KEYBOARD) PORT_NAME("[  {")        PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x31)
	PORT_BIT(0x0004, 0x00, IPT_KEYBOARD) PORT_NAME("]  }")        PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x32)
	PORT_BIT(0x0008, 0x00, IPT_KEYBOARD) PORT_NAME("\\  |")       PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x33)
	PORT_BIT(0x0010, 0x00, IPT_KEYBOARD) PORT_NAME("DELETE")      PORT_CODE(KEYCODE_DEL)         PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x34)
	PORT_BIT(0x0020, 0x00, IPT_KEYBOARD) PORT_NAME("COPY")        PORT_CODE(KEYCODE_END)         PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x35)
	PORT_BIT(0x0040, 0x00, IPT_KEYBOARD) PORT_NAME("PG DN")       PORT_CODE(KEYCODE_PGDN)        PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x36)
	PORT_BIT(0x0080, 0x00, IPT_KEYBOARD) PORT_NAME("KP 7")        PORT_CODE(KEYCODE_7_PAD)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x37)
	PORT_BIT(0x0100, 0x00, IPT_KEYBOARD) PORT_NAME("KP 8")        PORT_CODE(KEYCODE_8_PAD)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x38)
	PORT_BIT(0x0200, 0x00, IPT_KEYBOARD) PORT_NAME("KP 9")        PORT_CODE(KEYCODE_9_PAD)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x39)
	PORT_BIT(0x0400, 0x00, IPT_KEYBOARD) PORT_NAME("KP -")        PORT_CODE(KEYCODE_MINUS_PAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x3a)
	PORT_BIT(0x0800, 0x00, IPT_KEYBOARD) PORT_NAME("CTRL")        PORT_CODE(KEYCODE_LCONTROL)    PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x3b)
	PORT_BIT(0x1000, 0x00, IPT_KEYBOARD) PORT_NAME("a  A")        PORT_CODE(KEYCODE_A)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x3c)
	PORT_BIT(0x2000, 0x00, IPT_KEYBOARD) PORT_NAME("s  S")        PORT_CODE(KEYCODE_S)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x3d)
	PORT_BIT(0x4000, 0x00, IPT_KEYBOARD) PORT_NAME("d  D")        PORT_CODE(KEYCODE_D)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x3e)
	PORT_BIT(0x8000, 0x00, IPT_KEYBOARD) PORT_NAME("f  F")        PORT_CODE(KEYCODE_F)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x3f)

	PORT_START("key4") /* KEY ROW 4 */
	PORT_BIT(0x0001, 0x00, IPT_KEYBOARD) PORT_NAME("g  G")        PORT_CODE(KEYCODE_G)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x40)
	PORT_BIT(0x0002, 0x00, IPT_KEYBOARD) PORT_NAME("h  H")        PORT_CODE(KEYCODE_H)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x41)
	PORT_BIT(0x0004, 0x00, IPT_KEYBOARD) PORT_NAME("j  J")        PORT_CODE(KEYCODE_J)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x42)
	PORT_BIT(0x0008, 0x00, IPT_KEYBOARD) PORT_NAME("k  K")        PORT_CODE(KEYCODE_K)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x43)
	PORT_BIT(0x0010, 0x00, IPT_KEYBOARD) PORT_NAME("l  L")        PORT_CODE(KEYCODE_L)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x44)
	PORT_BIT(0x0020, 0x00, IPT_KEYBOARD) PORT_NAME(";  :")        PORT_CODE(KEYCODE_COLON)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x45)
	PORT_BIT(0x0040, 0x00, IPT_KEYBOARD) PORT_NAME("'  \"")       PORT_CODE(KEYCODE_QUOTE)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x46)
	PORT_BIT(0x0080, 0x00, IPT_KEYBOARD) PORT_NAME("RETURN")      PORT_CODE(KEYCODE_ENTER)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x47)
	PORT_BIT(0x0100, 0x00, IPT_KEYBOARD) PORT_NAME("KP 4")        PORT_CODE(KEYCODE_4_PAD)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x48)
	PORT_BIT(0x0200, 0x00, IPT_KEYBOARD) PORT_NAME("KP 5")        PORT_CODE(KEYCODE_5_PAD)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x49)
	PORT_BIT(0x0400, 0x00, IPT_KEYBOARD) PORT_NAME("KP 6")        PORT_CODE(KEYCODE_6_PAD)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x4a)
	PORT_BIT(0x0800, 0x00, IPT_KEYBOARD) PORT_NAME("KP +")        PORT_CODE(KEYCODE_PLUS_PAD)    PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x4b)
	PORT_BIT(0x1000, 0x00, IPT_KEYBOARD) PORT_NAME("SHIFT (L)")   PORT_CODE(KEYCODE_LSHIFT)      PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x4c)
	PORT_BIT(0x4000, 0x00, IPT_KEYBOARD) PORT_NAME("z  Z")        PORT_CODE(KEYCODE_Z)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x4e)
	PORT_BIT(0x8000, 0x00, IPT_KEYBOARD) PORT_NAME("x  X")        PORT_CODE(KEYCODE_X)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x4f)

	PORT_START("key5") /* KEY ROW 5 */
	PORT_BIT(0x0001, 0x00, IPT_KEYBOARD) PORT_NAME("c  C")        PORT_CODE(KEYCODE_C)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x50)
	PORT_BIT(0x0002, 0x00, IPT_KEYBOARD) PORT_NAME("v  V")        PORT_CODE(KEYCODE_V)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x51)
	PORT_BIT(0x0004, 0x00, IPT_KEYBOARD) PORT_NAME("b  B")        PORT_CODE(KEYCODE_B)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x52)
	PORT_BIT(0x0008, 0x00, IPT_KEYBOARD) PORT_NAME("n  N")        PORT_CODE(KEYCODE_N)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x53)
	PORT_BIT(0x0010, 0x00, IPT_KEYBOARD) PORT_NAME("m  M")        PORT_CODE(KEYCODE_M)           PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x54)
	PORT_BIT(0x0020, 0x00, IPT_KEYBOARD) PORT_NAME(",  <")        PORT_CODE(KEYCODE_COMMA)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x55)
	PORT_BIT(0x0040, 0x00, IPT_KEYBOARD) PORT_NAME(".  >")        PORT_CODE(KEYCODE_STOP)        PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x56)
	PORT_BIT(0x0080, 0x00, IPT_KEYBOARD) PORT_NAME("/  ?")        PORT_CODE(KEYCODE_SLASH)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x57)
	PORT_BIT(0x0100, 0x00, IPT_KEYBOARD) PORT_NAME("SHIFT (R)")   PORT_CODE(KEYCODE_RSHIFT)      PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x58)
	PORT_BIT(0x0200, 0x00, IPT_KEYBOARD) PORT_NAME("Up")          PORT_CODE(KEYCODE_UP)          PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x59)
	PORT_BIT(0x0400, 0x00, IPT_KEYBOARD) PORT_NAME("KP 1")        PORT_CODE(KEYCODE_1_PAD)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x5a)
	PORT_BIT(0x0800, 0x00, IPT_KEYBOARD) PORT_NAME("KP 2")        PORT_CODE(KEYCODE_2_PAD)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x5b)
	PORT_BIT(0x1000, 0x00, IPT_KEYBOARD) PORT_NAME("KP 3")        PORT_CODE(KEYCODE_3_PAD)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x5c)
	PORT_BIT(0x2000, 0x00, IPT_KEYBOARD) PORT_NAME("CAPS")        PORT_CODE(KEYCODE_CAPSLOCK)    PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x5d)
	PORT_BIT(0x4000, 0x00, IPT_KEYBOARD) PORT_NAME("ALT (L)")     PORT_CODE(KEYCODE_LALT)        PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x5e)
	PORT_BIT(0x8000, 0x00, IPT_KEYBOARD) PORT_NAME("SPACE")       PORT_CODE(KEYCODE_SPACE)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x5f)

	PORT_START("key6") /* KEY ROW 6 */
	PORT_BIT(0x0001, 0x00, IPT_KEYBOARD) PORT_NAME("ALT (R)")     PORT_CODE(KEYCODE_RALT)        PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x60)
	PORT_BIT(0x0002, 0x00, IPT_KEYBOARD) PORT_NAME("CTRL")        PORT_CODE(KEYCODE_RCONTROL)    PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x61)
	PORT_BIT(0x0004, 0x00, IPT_KEYBOARD) PORT_NAME("Left")        PORT_CODE(KEYCODE_LEFT)        PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x62)
	PORT_BIT(0x0008, 0x00, IPT_KEYBOARD) PORT_NAME("Down")        PORT_CODE(KEYCODE_DOWN)        PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x63)
	PORT_BIT(0x0010, 0x00, IPT_KEYBOARD) PORT_NAME("Right")       PORT_CODE(KEYCODE_RIGHT)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x64)
	PORT_BIT(0x0020, 0x00, IPT_KEYBOARD) PORT_NAME("KP 0")        PORT_CODE(KEYCODE_0_PAD)       PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x65)
	PORT_BIT(0x0040, 0x00, IPT_KEYBOARD) PORT_NAME("KP .")        PORT_CODE(KEYCODE_DEL_PAD)     PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x66)
	PORT_BIT(0x0080, 0x00, IPT_KEYBOARD) PORT_NAME("KP ENTER")    PORT_CODE(KEYCODE_ENTER_PAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x67)

	PORT_START("MOUSE")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Mouse Left")    PORT_CODE(MOUSECODE_BUTTON1)   PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x70)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Mouse Center")  PORT_CODE(MOUSECODE_BUTTON2)   PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x71)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Mouse Right")   PORT_CODE(MOUSECODE_BUTTON3)   PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, key_stroke, 0x72)

	PORT_START("MOUSEX")
	PORT_BIT( 0xffff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)                 PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, send_mouse_input, 0) PORT_RESET

	PORT_START("MOUSEY")
	PORT_BIT( 0xffff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)                 PORT_CHANGED_MEMBER(DEVICE_SELF, aa310_state, send_mouse_input, 0) PORT_RESET PORT_REVERSE

	// standard Atari/Commodore DB9
	// TODO: 10 different joystick configurations (!), some of them supports multiple buttons as well
	PORT_START("joy_p1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("joy_p2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

FLOPPY_FORMATS_MEMBER( aa310_state::floppy_formats )
	FLOPPY_ACORN_ADFS_NEW_FORMAT,
	FLOPPY_ACORN_ADFS_OLD_FORMAT,
	FLOPPY_APD_FORMAT,
	FLOPPY_JFD_FORMAT,
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static void aa310_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35hd", FLOPPY_35_HD);
}

WRITE_LINE_MEMBER( archimedes_state::a310_kart_tx_w )
{
	if(state)
		archimedes_request_irq_b(ARCHIMEDES_IRQB_KBD_RECV_FULL);
	else
		archimedes_clear_irq_b(ARCHIMEDES_IRQB_KBD_RECV_FULL);
}

WRITE_LINE_MEMBER( archimedes_state::a310_kart_rx_w )
{
	if(state)
		archimedes_request_irq_b(ARCHIMEDES_IRQB_KBD_XMIT_EMPTY);
	else
		archimedes_clear_irq_b(ARCHIMEDES_IRQB_KBD_XMIT_EMPTY);
}

void aa310_state::aa310(machine_config &config)
{
	/* basic machine hardware */
	ARM(config, m_maincpu, 24_MHz_XTAL / 3); /* ARM2 8 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &aa310_state::aa310_mem);
	m_maincpu->set_copro_type(arm_cpu_device::copro_type::VL86C020);

	AAKART(config, m_kart, 8000000/256);
	m_kart->out_tx_callback().set(FUNC(archimedes_state::a310_kart_tx_w));
	m_kart->out_rx_callback().set(FUNC(archimedes_state::a310_kart_rx_w));

	I2CMEM(config, "i2cmem", 0).set_data_size(0x100);

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	ACORN_VIDC10(config, m_vidc, 24_MHz_XTAL);
	m_vidc->set_screen("screen");
	m_vidc->vblank().set(FUNC(aa310_state::vblank_irq));
	m_vidc->sound_drq().set(FUNC(aa310_state::sound_drq));

	RAM(config, m_ram).set_default_size("1M");

	wd1772_device& fdc(WD1772(config, "fdc", 8000000 / 1)); // TODO: frequency
	fdc.set_disable_motor_control(true);
	fdc.intrq_wr_callback().set(FUNC(aa310_state::aa310_wd177x_intrq_w));
	fdc.drq_wr_callback().set(FUNC(aa310_state::aa310_wd177x_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", aa310_floppies, "35dd", aa310_state::floppy_formats).enable_sound(true);

	// rarely had 2nd FDD installed, space was used for HDD
	FLOPPY_CONNECTOR(config, "fdc:1", aa310_floppies, nullptr, aa310_state::floppy_formats).enable_sound(true);

	SOFTWARE_LIST(config, "flop_list").set_original("archimedes");

	/* Expansion slots - 2-card backplane */
}

void aa310_state::aa305(machine_config &config)
{
	aa310(config);
	m_ram->set_default_size("512K").set_extra_options("1M");
}

void aa310_state::aa440(machine_config &config)
{
	aa310(config);
	m_ram->set_default_size("4M");

	/* 20MB HDD */

	/* Expansion slots - 4-card backplane */
}

void aa310_state::aa3000(machine_config &config)
{
	aa310(config);
	m_ram->set_default_size("1M").set_extra_options("2M");
}

void aa310_state::aa4101(machine_config &config)
{
	aa310(config);
	m_ram->set_default_size("1M").set_extra_options("2M,4M");

	/* Expansion slots - 4-card backplane */
}

void aa310_state::aa4201(machine_config &config)
{
	aa310(config);
	m_ram->set_default_size("2M").set_extra_options("4M");

	/* 20MB HDD */

	/* Expansion slots - 4-card backplane */
}

void aa310_state::aa4401(machine_config &config)
{
	aa310(config);
	m_ram->set_default_size("4M").set_extra_options("8M");

	/* 50MB HDD */

	/* Expansion slots - 4-card backplane */
}

void aa310_state::aa540(machine_config &config)
{
	aa310(config);
	m_maincpu->set_clock(52_MHz_XTAL / 2); // ARM3

	m_ram->set_default_size("4M").set_extra_options("8M,12M,16M");

	/* 100MB HDD */

	/* Expansion slots - 4-card backplane */
}

void aa310_state::aa5000(machine_config &config)
{
	aa310(config);
	m_maincpu->set_clock(50_MHz_XTAL / 2); // ARM3

	m_ram->set_default_size("2M").set_extra_options("4M");

	/* 80MB HDD */

	/* Expansion slots - 4-card backplane */
}

void aa310_state::aa4(machine_config &config)
{
	aa5000(config);
	m_maincpu->set_clock(24_MHz_XTAL); // ARM3

	/* video hardware */
	SCREEN(config.replace(), "screen", SCREEN_TYPE_LCD);

	ACORN_VIDC10_LCD(config.replace(), m_vidc, 24_MHz_XTAL);
	m_vidc->set_screen("screen");
	m_vidc->vblank().set(FUNC(aa310_state::vblank_irq));
	m_vidc->sound_drq().set(FUNC(aa310_state::sound_drq));

	/* 765 FDC */

	/* 60MB HDD */
}

void aa310_state::aa5000a(machine_config &config)
{
	aa5000(config);
	m_maincpu->set_clock(33000000); // ARM3
}

void aa310_state::aa3010(machine_config &config)
{
	aa310(config);
	m_maincpu->set_clock(72_MHz_XTAL / 6); // ARM250

	m_ram->set_default_size("1M").set_extra_options("2M");
}

void aa310_state::aa3020(machine_config &config)
{
	aa3010(config);
	m_ram->set_default_size("2M").set_extra_options("4M");
}

void aa310_state::aa4000(machine_config &config)
{
	aa3010(config);
	m_ram->set_default_size("2M").set_extra_options("4M");

	/* 80MB HDD */

	/* Expansion slots - 4-card backplane */
}

ROM_START( aa305 )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "120", "Arthur 1.20 (25 Sep 1987)" ) // Parts 0277,022-02, 0277,023-03, 0277,024-02, 0277,025-02,
	ROMX_LOAD( "arthur120.bin", 0x000000, 0x80000, CRC(eb3fda57) SHA1(1181ff9c2c2f3d6d414054ec33b2260404bafc81), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "030", "Arthur 0.30 (17 Jun 1987)" ) // Parts 0276,322-01, 0276,323-01, 0276,324-01, 0276,325-01,
	ROMX_LOAD( "arthur030.bin", 0x000000, 0x80000, CRC(5df8ed42) SHA1(6aebd686d97dfdf6726fa5f3246ef35b840b286d), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "200", "RISC OS 2.00 (05 Oct 1988)" )
	ROMX_LOAD( "0283,022-01.ic24", 0x000000, 0x20000, CRC(52186922) SHA1(06a7ca871407fec7b55b56ed1b419d10ced03e72), ROM_BIOS(2) )
	ROMX_LOAD( "0283,023-01.ic25", 0x020000, 0x20000, CRC(b8284110) SHA1(021f29e09a5ad994f71992fb6118d78069c545bf), ROM_BIOS(2) )
	ROMX_LOAD( "0283,024-01.ic26", 0x040000, 0x20000, CRC(b80d1a22) SHA1(6b8cb24f7f7095c0c64e62697553fb5294146135), ROM_BIOS(2) )
	ROMX_LOAD( "0283,025-01.ic27", 0x060000, 0x20000, CRC(ad1d4715) SHA1(f9c5a771ae5dfbc4b19c95e2e7423166735e8697), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "201", "RISC OS 2.01 (05 Jul 1990)" ) // Parts 0270,601-01, 0270,602-01, 0270,603-01, 0270,604-01,
	ROMX_LOAD( "riscos201.bin", 0x000000, 0x080000, CRC(7cb5ea3f) SHA1(9ed7dee8553d96a6d58dd23d31682234f57beb62), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "300", "RISC OS 3.00 (25 Sep 1991)" )
	ROMX_LOAD( "0270,251-01.ic24", 0x000000, 0x80000, CRC(44af65d3) SHA1(7097f39722ed10f3b2c27805cb8866c14f878a7b), ROM_BIOS(4) )
	ROMX_LOAD( "0270,252-01.ic25", 0x080000, 0x80000, CRC(b4a28cba) SHA1(3b8c8fa5068ebab74d2c229c1582de67eef81ac9), ROM_BIOS(4) )
	ROMX_LOAD( "0270,253-01.ic26", 0x100000, 0x80000, CRC(a3661f66) SHA1(5d4ddd776945321a07c9e59b6b24b90815a3e861), ROM_BIOS(4) )
	ROMX_LOAD( "0270,254-01.ic27", 0x180000, 0x80000, CRC(3aae93ad) SHA1(13efdd4a09aad1a3a4cd39f81994d68be511232e), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "310", "RISC OS 3.10 (30 Apr 1992)" )
	ROMX_LOAD( "0296,041-01.ic24", 0x000000, 0x80000, CRC(3aab9849) SHA1(ce910fd0c53fef609a5ab70f251da798c10235c0), ROM_BIOS(5) )
	ROMX_LOAD( "0296,042-01.ic25", 0x080000, 0x80000, CRC(15d89664) SHA1(78f5d0e6f1e8ee603317807f53ff8fe65a3b3518), ROM_BIOS(5) )
	ROMX_LOAD( "0296,043-01.ic26", 0x100000, 0x80000, CRC(a81ceb7c) SHA1(46b870876bc1f68f242726415f0c49fef7be0c72), ROM_BIOS(5) )
	ROMX_LOAD( "0296,044-01.ic27", 0x180000, 0x80000, CRC(fc1badad) SHA1(fc1326cf949ec54ef6ef32cf928b996f35b69b50), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "311", "RISC OS 3.11 (29 Sep 1992)" )
	ROMX_LOAD( "0296,041-02.ic24", 0x000000, 0x80000, CRC(c1adde84) SHA1(12d060e0401dd0523d44453f947bdc55dd2c3240), ROM_BIOS(6) )
	ROMX_LOAD( "0296,042-02.ic25", 0x080000, 0x80000, CRC(15d89664) SHA1(78f5d0e6f1e8ee603317807f53ff8fe65a3b3518), ROM_BIOS(6) )
	ROMX_LOAD( "0296,043-02.ic26", 0x100000, 0x80000, CRC(a81ceb7c) SHA1(46b870876bc1f68f242726415f0c49fef7be0c72), ROM_BIOS(6) )
	ROMX_LOAD( "0296,044-02.ic27", 0x180000, 0x80000, CRC(707b0c6c) SHA1(345199a33fed23996374b9db8170a52ab63f0380), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "319", "RISC OS 3.19 (09 Jun 1993)" ) // Parts 0296,241-01, 0296,242-01, 0296,243-01, 0296,244-01,
	ROMX_LOAD( "riscos319.bin", 0x000000, 0x200000, CRC(00c7a3d3) SHA1(be7a8cba5d6c6c0e1c4838712524056cf4b8c8cb), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 8, "test", "Diagnostic Test ROMs" ) // Usage described in Archimedes 300 Series Service Manual
	ROMX_LOAD( "0276,146-01.ic24", 0x000000, 0x10000, CRC(9c45283c) SHA1(9eb5bd7ad0958f194a3416d79d7e01e4c45741e1), ROM_BIOS(8) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,147-01.ic25", 0x000001, 0x10000, CRC(ad94e17f) SHA1(1c8e39c69d4ae1b674e0f732aaa62a4403998f41), ROM_BIOS(8) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,148-01.ic26", 0x000002, 0x10000, CRC(1ab02f2d) SHA1(dd7d216967524e64d1a03076a6081461ec8528c3), ROM_BIOS(8) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,149-01.ic27", 0x000003, 0x10000, CRC(5fd6a406) SHA1(790af8a4c74d0f6714d528f7502443ce5898a618), ROM_BIOS(8) | ROM_SKIP(3) )


	ROM_REGION( 0x400000, "extension", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROMX_LOAD( "cmos_arthur.bin",  0x0000, 0x0100, CRC(017cdf3b) SHA1(03aa58fc8578de2019a34c6eeb4072e953f3444f), ROM_BIOS(0) )
	ROMX_LOAD( "cmos_arthur.bin",  0x0000, 0x0100, CRC(017cdf3b) SHA1(03aa58fc8578de2019a34c6eeb4072e953f3444f), ROM_BIOS(1) )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(763b14e3) SHA1(0ccd41648a798ba4a5d92c19c24b605a8927fb76), ROM_BIOS(2) )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(763b14e3) SHA1(0ccd41648a798ba4a5d92c19c24b605a8927fb76), ROM_BIOS(3) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(4) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(5) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(6) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(7) )
ROM_END

#define rom_aa310 rom_aa305
#define rom_aa440 rom_aa305

ROM_START( aa3000 )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "200", "RISC OS 2.00 (05 Oct 1988)" )
	ROMX_LOAD( "0283,022-01.ic14", 0x000000, 0x20000, CRC(52186922) SHA1(06a7ca871407fec7b55b56ed1b419d10ced03e72), ROM_BIOS(0) )
	ROMX_LOAD( "0283,023-01.ic15", 0x020000, 0x20000, CRC(b8284110) SHA1(021f29e09a5ad994f71992fb6118d78069c545bf), ROM_BIOS(0) )
	ROMX_LOAD( "0283,024-01.ic16", 0x040000, 0x20000, CRC(b80d1a22) SHA1(6b8cb24f7f7095c0c64e62697553fb5294146135), ROM_BIOS(0) )
	ROMX_LOAD( "0283,025-01.ic17", 0x060000, 0x20000, CRC(ad1d4715) SHA1(f9c5a771ae5dfbc4b19c95e2e7423166735e8697), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "201", "RISC OS 2.01 (05 Jul 1990)" ) // Parts 0270,601-01, 0270,602-01, 0270,603-01, 0270,604-01,
	ROMX_LOAD( "riscos201.bin", 0x000000, 0x080000, CRC(7cb5ea3f) SHA1(9ed7dee8553d96a6d58dd23d31682234f57beb62), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "300", "RISC OS 3.00 (25 Sep 1991)" )
	ROMX_LOAD( "0270,251-01.ic14", 0x000000, 0x80000, CRC(44af65d3) SHA1(7097f39722ed10f3b2c27805cb8866c14f878a7b), ROM_BIOS(2) )
	ROMX_LOAD( "0270,252-01.ic15", 0x080000, 0x80000, CRC(b4a28cba) SHA1(3b8c8fa5068ebab74d2c229c1582de67eef81ac9), ROM_BIOS(2) )
	ROMX_LOAD( "0270,253-01.ic16", 0x100000, 0x80000, CRC(a3661f66) SHA1(5d4ddd776945321a07c9e59b6b24b90815a3e861), ROM_BIOS(2) )
	ROMX_LOAD( "0270,254-01.ic17", 0x180000, 0x80000, CRC(3aae93ad) SHA1(13efdd4a09aad1a3a4cd39f81994d68be511232e), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "310", "RISC OS 3.10 (30 Apr 1992)" )
	ROMX_LOAD( "0296,041-01.ic14", 0x000000, 0x80000, CRC(3aab9849) SHA1(ce910fd0c53fef609a5ab70f251da798c10235c0), ROM_BIOS(3) )
	ROMX_LOAD( "0296,042-01.ic15", 0x080000, 0x80000, CRC(15d89664) SHA1(78f5d0e6f1e8ee603317807f53ff8fe65a3b3518), ROM_BIOS(3) )
	ROMX_LOAD( "0296,043-01.ic16", 0x100000, 0x80000, CRC(a81ceb7c) SHA1(46b870876bc1f68f242726415f0c49fef7be0c72), ROM_BIOS(3) )
	ROMX_LOAD( "0296,044-01.ic17", 0x180000, 0x80000, CRC(fc1badad) SHA1(fc1326cf949ec54ef6ef32cf928b996f35b69b50), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "311", "RISC OS 3.11 (29 Sep 1992)" )
	ROMX_LOAD( "0296,041-02.ic14", 0x000000, 0x80000, CRC(c1adde84) SHA1(12d060e0401dd0523d44453f947bdc55dd2c3240), ROM_BIOS(4) )
	ROMX_LOAD( "0296,042-02.ic15", 0x080000, 0x80000, CRC(15d89664) SHA1(78f5d0e6f1e8ee603317807f53ff8fe65a3b3518), ROM_BIOS(4) )
	ROMX_LOAD( "0296,043-02.ic16", 0x100000, 0x80000, CRC(a81ceb7c) SHA1(46b870876bc1f68f242726415f0c49fef7be0c72), ROM_BIOS(4) )
	ROMX_LOAD( "0296,044-02.ic17", 0x180000, 0x80000, CRC(707b0c6c) SHA1(345199a33fed23996374b9db8170a52ab63f0380), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "319", "RISC OS 3.19 (09 Jun 1993)" ) // Parts 0296,241-01, 0296,242-01, 0296,243-01, 0296,244-01,
	ROMX_LOAD( "riscos319.bin", 0x000000, 0x200000, CRC(00c7a3d3) SHA1(be7a8cba5d6c6c0e1c4838712524056cf4b8c8cb), ROM_BIOS(5) )


	ROM_REGION( 0x400000, "extension", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(763b14e3) SHA1(0ccd41648a798ba4a5d92c19c24b605a8927fb76), ROM_BIOS(0) )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(763b14e3) SHA1(0ccd41648a798ba4a5d92c19c24b605a8927fb76), ROM_BIOS(1) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(2) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(3) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(4) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(5) )
ROM_END

#define rom_aa4101 rom_aa3000 // ROMs in IC16, IC17, IC18, IC19
#define rom_aa4201 rom_aa3000 // ROMs in IC16, IC17, IC18, IC19
#define rom_aa4401 rom_aa3000 // ROMs in IC16, IC17, IC18, IC19
#define rom_aa540  rom_aa3000 // ROMs in IC47, IC48, IC49, IC50

ROM_START( aa5000 )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "300", "RISC OS 3.00 (25 Sep 1991)" )
	ROMX_LOAD( "0270,251-01.ic27", 0x000000, 0x80000, CRC(44af65d3) SHA1(7097f39722ed10f3b2c27805cb8866c14f878a7b), ROM_BIOS(0) )
	ROMX_LOAD( "0270,252-01.ic28", 0x080000, 0x80000, CRC(b4a28cba) SHA1(3b8c8fa5068ebab74d2c229c1582de67eef81ac9), ROM_BIOS(0) )
	ROMX_LOAD( "0270,253-01.ic29", 0x100000, 0x80000, CRC(a3661f66) SHA1(5d4ddd776945321a07c9e59b6b24b90815a3e861), ROM_BIOS(0) )
	ROMX_LOAD( "0270,254-01.ic30", 0x180000, 0x80000, CRC(3aae93ad) SHA1(13efdd4a09aad1a3a4cd39f81994d68be511232e), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "310", "RISC OS 3.10 (30 Apr 1992)" )
	ROMX_LOAD( "0296,041-01.ic27", 0x000000, 0x80000, CRC(3aab9849) SHA1(ce910fd0c53fef609a5ab70f251da798c10235c0), ROM_BIOS(1) )
	ROMX_LOAD( "0296,042-01.ic28", 0x080000, 0x80000, CRC(15d89664) SHA1(78f5d0e6f1e8ee603317807f53ff8fe65a3b3518), ROM_BIOS(1) )
	ROMX_LOAD( "0296,043-01.ic29", 0x100000, 0x80000, CRC(a81ceb7c) SHA1(46b870876bc1f68f242726415f0c49fef7be0c72), ROM_BIOS(1) )
	ROMX_LOAD( "0296,044-01.ic30", 0x180000, 0x80000, CRC(fc1badad) SHA1(fc1326cf949ec54ef6ef32cf928b996f35b69b50), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "311", "RISC OS 3.11 (29 Sep 1992)" )
	ROMX_LOAD( "0296,041-02.ic27", 0x000000, 0x80000, CRC(c1adde84) SHA1(12d060e0401dd0523d44453f947bdc55dd2c3240), ROM_BIOS(2) )
	ROMX_LOAD( "0296,042-02.ic28", 0x080000, 0x80000, CRC(15d89664) SHA1(78f5d0e6f1e8ee603317807f53ff8fe65a3b3518), ROM_BIOS(2) )
	ROMX_LOAD( "0296,043-02.ic29", 0x100000, 0x80000, CRC(a81ceb7c) SHA1(46b870876bc1f68f242726415f0c49fef7be0c72), ROM_BIOS(2) )
	ROMX_LOAD( "0296,044-02.ic30", 0x180000, 0x80000, CRC(707b0c6c) SHA1(345199a33fed23996374b9db8170a52ab63f0380), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "319", "RISC OS 3.19 (09 Jun 1993)" ) // Parts 0296,241-01, 0296,242-01, 0296,243-01, 0296,244-01,
	ROMX_LOAD( "riscos319.bin", 0x000000, 0x200000, CRC(00c7a3d3) SHA1(be7a8cba5d6c6c0e1c4838712524056cf4b8c8cb), ROM_BIOS(3) )


	ROM_REGION( 0x400000, "extension", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROM_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5) )
ROM_END

#define rom_aa5000a rom_aa5000

ROM_START( aa4 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0296,061-01.ic4",  0x000000, 0x100000, CRC(b77fe215) SHA1(57b19ea4b97a9b6a240aa61211c2c134cb295aa0) )
	ROM_LOAD32_WORD( "0296,062-01.ic15", 0x000002, 0x100000, CRC(d42e196e) SHA1(64243d39d1bca38b10761f66a8042c883bde87a4) )


	ROM_REGION( 0x400000, "extension", ROMREGION_ERASE00 )
	/* Power Management */
	ROM_LOAD32_BYTE( "0296,063-01.ic38", 0x000003, 0x010000, CRC(9ca3a6be) SHA1(75905b031f49960605d55c3e7350d309559ed440) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROM_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5) )
ROM_END

ROM_START( aa3010 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0296,061-02.ic17", 0x000000, 0x100000, CRC(552fc3aa) SHA1(b2f1911e53d7377f2e69e1a870139745d3df494b) )
	ROM_LOAD32_WORD( "0296,062-02.ic18", 0x000002, 0x100000, CRC(308d5a4a) SHA1(b309e1dd85670a06d77ec504dbbec6c42336329f) )


	ROM_REGION( 0x400000, "extension", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROM_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5))
ROM_END

#define rom_aa3020 rom_aa3010
#define rom_aa4000 rom_aa3010

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME             FLAGS */
COMP( 1987, aa305,   aa310,  0,      aa305,   aa310, aa310_state, init_aa310, "Acorn", "Archimedes 305",    MACHINE_NOT_WORKING)
COMP( 1987, aa310,   0,      0,      aa310,   aa310, aa310_state, init_aa310, "Acorn", "Archimedes 310",    MACHINE_NOT_WORKING)
COMP( 1987, aa440,   aa310,  0,      aa440,   aa310, aa310_state, init_aa310, "Acorn", "Archimedes 440",    MACHINE_NOT_WORKING)
COMP( 1989, aa3000,  aa310,  0,      aa3000,  aa310, aa310_state, init_aa310, "Acorn", "BBC A3000",         MACHINE_NOT_WORKING)
COMP( 1989, aa4101,  aa310,  0,      aa4101,  aa310, aa310_state, init_aa310, "Acorn", "Archimedes 410/1",  MACHINE_NOT_WORKING)
COMP( 1989, aa4201,  aa310,  0,      aa4201,  aa310, aa310_state, init_aa310, "Acorn", "Archimedes 420/1",  MACHINE_NOT_WORKING)
COMP( 1989, aa4401,  aa310,  0,      aa4401,  aa310, aa310_state, init_aa310, "Acorn", "Archimedes 440/1",  MACHINE_NOT_WORKING)
COMP( 1990, aa540,   aa310,  0,      aa540,   aa310, aa310_state, init_aa310, "Acorn", "Archimedes 540",    MACHINE_NOT_WORKING)
COMP( 1991, aa5000,  0,      0,      aa5000,  aa310, aa310_state, init_aa310, "Acorn", "Acorn A5000",       MACHINE_NOT_WORKING)
COMP( 1992, aa4,     aa5000, 0,      aa4,     aa310, aa310_state, init_aa310, "Acorn", "Acorn A4",          MACHINE_NOT_WORKING)
COMP( 1992, aa3010,  aa4000, 0,      aa3010,  aa310, aa310_state, init_aa310, "Acorn", "Acorn A3010",       MACHINE_NOT_WORKING)
COMP( 1992, aa3020,  aa4000, 0,      aa3020,  aa310, aa310_state, init_aa310, "Acorn", "Acorn A3020",       MACHINE_NOT_WORKING)
COMP( 1992, aa4000,  0,      0,      aa4000,  aa310, aa310_state, init_aa310, "Acorn", "Acorn A4000",       MACHINE_NOT_WORKING)
COMP( 1993, aa5000a, aa5000, 0,      aa5000a, aa310, aa310_state, init_aa310, "Acorn", "Acorn A5000 Alpha", MACHINE_NOT_WORKING)
