// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*******************************************************************************************************

Photo Play (c) 199? Funworld

Preliminary driver by Angelo Salese

TODO:
- DISK BOOT FAILURE after eeprom checking (many unknown IDE cs1 reads/writes);
- partition boot sector is missing from the CHD dump, protection?
- Detects CPU type as "-S 16 MHz"? Sometimes it detects it as 486SX, unknown repro (after fiddling with CMOS settings anyway)
- VGA BIOS reports being a Cirrus Logic GD5436 / 5446, it is unknown what exactly this game uses.
- PCI hookups (no idea about what this uses), and improve/device-ify SiS85C49x;
- ISA bus cards are completely guessworked;
- EEPROM timings are hacked (writes mostly fail otherwise);
- Eventually needs AudioDrive ES688 / ES1688 / ES1788 & ES1868 devices and serial ports "for linking" before actually booting;


*******************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/eepromser.h"
#include "machine/lpci.h"
#include "machine/pcshare.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "video/clgd542x.h"

class photoply_state : public pcat_base_state
{
public:
	photoply_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag),
		m_eeprom(*this, "eeprom"),
		m_main_bios(*this, "bios"),
		m_video_bios(*this, "video_bios"),
		m_ex_bios(*this, "ex_bios")
	{
	}

	required_device<eeprom_serial_93cxx_device> m_eeprom;

	required_region_ptr<uint8_t> m_main_bios;
	required_region_ptr<uint8_t> m_video_bios;
	required_region_ptr<uint8_t> m_ex_bios;
	
	uint8_t *m_shadow_ram;

	DECLARE_READ8_MEMBER(bios_r);
	DECLARE_WRITE8_MEMBER(bios_w);
	DECLARE_WRITE8_MEMBER(eeprom_w);

	uint16_t m_pci_shadow_reg;
	DECLARE_DRIVER_INIT(photoply);
	void photoply(machine_config &config);

	void photoply_io(address_map &map);
	void photoply_map(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};

// regs 0x00-0x3f both devices below
// regs 0x40-0x7f SiS85C496 PCI & CPU Memory Controller (PCM)
// regs 0x80-0xff SiS85C497 AT Bus Controller & Megacell (ATM)
static uint32_t sis_pcm_r(device_t *busdevice, device_t *device, int function, int reg, uint32_t mem_mask)
{
	uint32_t r = 0;
	photoply_state *state = busdevice->machine().driver_data<photoply_state>();

	//printf("PCM %02x %08x\n",reg,mem_mask);
	if(reg == 0)
	{
		// Device ID / Vendor ID
		return 0x04961039;
	}
	
	if(reg == 8)
	{
		// Device Class Code / Device Revision Identification
		return 0x06000002;
	}
	
	// Device Header Type
	if(reg == 0xc)
		return 0;

	if(reg == 0x44)
	{

		if(ACCESSING_BITS_8_15) // reg 0x45
			r |= (state->m_pci_shadow_reg & 0xff00);
		if(ACCESSING_BITS_0_7) // reg 0x44
			r |= (state->m_pci_shadow_reg & 0x00ff);
	}
	return r;
}

static void sis_pcm_w(device_t *busdevice, device_t *device, int function, int reg, uint32_t data, uint32_t mem_mask)
{
	photoply_state *state = busdevice->machine().driver_data<photoply_state>();

	if(reg == 0x44)
	{

		/* 
		 * reg 0x45
		 * shadow RAM control
		 * xxxx ---- <reserved>
		 * ---- x--- Internal Cache Cacheable Area Control
		 * ---- -x-- PCI,ISA Master Access Shadow RAM Area Enable
		 * ---- --x- Shadow RAM Read Control (1=Enable)
		 * ---- ---x Shadow RAM Write Control (0=Enable)
		 */
		if(ACCESSING_BITS_8_15)
			state->m_pci_shadow_reg = (data & 0xff00) | (state->m_pci_shadow_reg & 0x00ff);
		
		/*
		 * shadow RAM enable:
		 * bit 7: 0xf8000-0xfffff shadow RAM enable
		 * ...
		 * bit 0: 0xc0000-0xc7fff shadow RAM enable
		 */
		if(ACCESSING_BITS_0_7) // reg 0x44
			state->m_pci_shadow_reg = (data & 0x00ff) | (state->m_pci_shadow_reg & 0xff00);
			
		//printf("%04x\n",state->m_pci_shadow_reg);
	}
}

READ8_MEMBER(photoply_state::bios_r)
{
	uint8_t bit_mask = (offset & 0x38000) >> 15;
	
	if((m_pci_shadow_reg & 0x200) == 0x200)
	{	
		if(m_pci_shadow_reg & (1 << bit_mask))
			return m_shadow_ram[offset];
	}

	// TODO: this mapping is a complete guesswork
	// TODO: worth converting this to bankdev when PCI gets de-legacized
	switch(bit_mask & 7)
	{
		// cirrus logic video bios
		case 0: // c0000-c7fff
			return m_video_bios[offset & 0x7fff];
		// multifunction board bios
		// Note: if filled with a reload on 0x4000-0x7fff it will repeat the "Combo I/O" EEPROM check (which looks unlikely)
		case 1: // c8000-cffff
			if ((offset & 0x7fff) == 0x3fff)
			{
				// other bits are unknown
				return m_eeprom->do_read() << 2;
			}
			return m_ex_bios[offset & 0x7fff];
		case 2: // d0000-dffff
		case 3:
			return 0;
	}

	// e0000-fffff
	return m_main_bios[offset & 0x1ffff];
}

WRITE8_MEMBER(photoply_state::bios_w)
{
//	return m_bios[offset];

	if((m_pci_shadow_reg & 0x100) == 0)
	{
		uint8_t bit_mask = (offset & 0x38000) >> 15;

		if(m_pci_shadow_reg & (1 << bit_mask))
			m_shadow_ram[offset] = data;
	}
}

WRITE8_MEMBER(photoply_state::eeprom_w)
{
	//logerror("Writing %X to EEPROM output port\n", data);
	m_eeprom->di_write(BIT(data, 0));
	m_eeprom->clk_write(BIT(data, 1));
	m_eeprom->cs_write(BIT(data, 2));

	// Bits 2-3 also seem to be used to bank something else.
	// Bits 4-7 are set for some writes, but may do nothing?
}

ADDRESS_MAP_START(photoply_state::photoply_map)
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_DEVREADWRITE8("vga", cirrus_gd5446_device, mem_r, mem_w, 0xffffffff)
//	AM_RANGE(0x000c0000, 0x000c7fff) AM_RAM AM_REGION("video_bios", 0)
//	AM_RANGE(0x000c8000, 0x000cffff) AM_RAM AM_REGION("ex_bios", 0)
	AM_RANGE(0x000c0000, 0x000fffff) AM_READWRITE8(bios_r,bios_w,0xffffffff)
	AM_RANGE(0x00100000, 0x07ffffff) AM_RAM // 64MB RAM, guess!
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END



ADDRESS_MAP_START(photoply_state::photoply_io)
	AM_IMPORT_FROM(pcat32_io_common)
	AM_RANGE(0x00e8, 0x00eb) AM_NOP

	AM_RANGE(0x0170, 0x0177) AM_DEVREADWRITE("ide2", ide_controller_32_device, read_cs0, write_cs0)
	AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE("ide", ide_controller_32_device, read_cs0, write_cs0)
	AM_RANGE(0x0200, 0x0203) AM_WRITE8(eeprom_w, 0x00ff0000)
//	AM_RANGE(0x0278, 0x027f) AM_RAM //parallel port 2
	AM_RANGE(0x0370, 0x0377) AM_DEVREADWRITE("ide2", ide_controller_32_device, read_cs1, write_cs1)
//	AM_RANGE(0x0378, 0x037f) AM_RAM //parallel port
	AM_RANGE(0x03b0, 0x03bf) AM_DEVREADWRITE8("vga", cirrus_gd5446_device, port_03b0_r, port_03b0_w, 0xffffffff)
	AM_RANGE(0x03c0, 0x03cf) AM_DEVREADWRITE8("vga", cirrus_gd5446_device, port_03c0_r, port_03c0_w, 0xffffffff)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE8("vga", cirrus_gd5446_device, port_03d0_r, port_03d0_w, 0xffffffff)

	AM_RANGE(0x03f0, 0x03f7) AM_DEVREADWRITE("ide", ide_controller_32_device, read_cs1, write_cs1)
	
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_legacy_device, read, write)

ADDRESS_MAP_END

#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(text) PORT_CODE(key1)

static INPUT_PORTS_START( photoply )
	PORT_START("pc_keyboard_0")
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED )     /* unused scancode 0 */
	AT_KEYB_HELPER( 0x0002, "Esc",          KEYCODE_Q           ) /* Esc                         01  81 */
	AT_KEYB_HELPER( 0x0004, "1",            KEYCODE_1           ) 
	AT_KEYB_HELPER( 0x0008, "2",            KEYCODE_2           ) 

	PORT_START("pc_keyboard_1")
	AT_KEYB_HELPER( 0x0020, "Y",            KEYCODE_Y           ) /* Y                           15  95 */
	AT_KEYB_HELPER( 0x1000, "Enter",        KEYCODE_ENTER       ) /* Enter                       1C  9C */

	PORT_START("pc_keyboard_2")

	PORT_START("pc_keyboard_3")
	AT_KEYB_HELPER( 0x0002, "N",            KEYCODE_N           ) /* N                           31  B1 */
	AT_KEYB_HELPER( 0x0800, "F1",           KEYCODE_F1          ) /* F1                          3B  BB */
	AT_KEYB_HELPER( 0x1000, "F2",           KEYCODE_F2          )
	AT_KEYB_HELPER( 0x4000, "F4",           KEYCODE_F4          )

	PORT_START("pc_keyboard_4")

	PORT_START("pc_keyboard_5")

	PORT_START("pc_keyboard_6")
	AT_KEYB_HELPER( 0x0040, "(MF2)Cursor Up",       KEYCODE_UP          ) /* Up                          67  e7 */
	AT_KEYB_HELPER( 0x0080, "(MF2)Page Up",         KEYCODE_PGUP        ) /* Page Up                     68  e8 */
	AT_KEYB_HELPER( 0x0100, "(MF2)Cursor Left",     KEYCODE_LEFT        ) /* Left                        69  e9 */
	AT_KEYB_HELPER( 0x0200, "(MF2)Cursor Right",    KEYCODE_RIGHT       ) /* Right                       6a  ea */
	AT_KEYB_HELPER( 0x0800, "(MF2)Cursor Down",     KEYCODE_DOWN        ) /* Down                        6c  ec */
	AT_KEYB_HELPER( 0x1000, "(MF2)Page Down",       KEYCODE_PGDN        ) /* Page Down                   6d  ed */
	AT_KEYB_HELPER( 0x4000, "Del",                  KEYCODE_A           ) /* Delete                      6f  ef */

	PORT_START("pc_keyboard_7")
INPUT_PORTS_END

void photoply_state::machine_start()
{
	m_shadow_ram = auto_alloc_array(machine(), uint8_t, 0x40000);
	save_pointer(NAME(m_shadow_ram),0x40000);
}

void photoply_state::machine_reset()
{
	m_pci_shadow_reg = 0;
}


static const gfx_layout CGA_charlayout =
{
	8,8,
	256,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static GFXDECODE_START( photoply )
	GFXDECODE_ENTRY( "video_bios", 0x6000+0xa5*8+7, CGA_charlayout,              0, 256 )
	//there's also a 8x16 entry (just after the 8x8)
GFXDECODE_END

MACHINE_CONFIG_START(photoply_state::photoply)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I486DX4, 75000000) /* I486DX4, 75 or 100 Mhz */
	MCFG_CPU_PROGRAM_MAP(photoply_map)
	MCFG_CPU_IO_MAP(photoply_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_1", pic8259_device, inta_cb)

	pcat_common(config);

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", photoply )

	MCFG_IDE_CONTROLLER_32_ADD("ide", ata_devices, "hdd", nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("pic8259_2", pic8259_device, ir6_w))

	MCFG_IDE_CONTROLLER_32_ADD("ide2", ata_devices, nullptr, nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("pic8259_2", pic8259_device, ir7_w))

	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(5, nullptr, sis_pcm_r, sis_pcm_w)
	
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(25'174'800),900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", cirrus_gd5446_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_DEVICE_ADD("vga", CIRRUS_GD5446, 0)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_EEPROM_WRITE_TIME(attotime::from_usec(1))
	MCFG_EEPROM_ERASE_ALL_TIME(attotime::from_usec(10))
MACHINE_CONFIG_END


ROM_START(photoply)
	ROM_REGION(0x20000, "bios", 0)  /* motherboard bios */
	ROM_LOAD("award bootblock bios v1.0.bin", 0x000000, 0x20000, CRC(e96d1bbc) SHA1(64d0726c4e9ecee8fddf4cc39d92aecaa8184d5c) )

	ROM_REGION(0x8000, "ex_bios", ROMREGION_ERASE00 ) /* multifunction board with a ESS AudioDrive chip,  M27128A */
	ROM_LOAD("enhanced bios.bin", 0x000000, 0x4000, CRC(a216404e) SHA1(c9067cf87d5c8106de00866bb211eae3a6c02c65) )
//	ROM_RELOAD(                   0x004000, 0x4000 )
//	ROM_RELOAD(                   0x008000, 0x4000 )
//	ROM_RELOAD(                   0x00c000, 0x4000 )

	ROM_REGION(0x8000, "video_bios", 0 )
	ROM_LOAD("vga.bin", 0x000000, 0x8000, CRC(7a859659) SHA1(ff667218261969c48082ec12aa91088a01b0cb2a) )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "pp201", 0, SHA1(23e1940d485d19401e7d0ad912ddad2cf2ea10b4) )
ROM_END

DRIVER_INIT_MEMBER(photoply_state,photoply)
{
}

GAME( 199?, photoply,  0,   photoply, photoply, photoply_state, photoply, ROT0, "Funworld", "Photo Play 2000 (v2.01)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND|MACHINE_UNEMULATED_PROTECTION )
