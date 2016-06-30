// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz,Fabio Priuli
/***********************************************************************************************************


 Game Boy Advance cart emulation


 We support carts with several kind of Save RAM (actual SRAM, Flash RAM or EEPROM)



 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  gba_rom_device - constructor
//-------------------------------------------------

const device_type GBA_ROM_STD = &device_creator<gba_rom_device>;
const device_type GBA_ROM_SRAM = &device_creator<gba_rom_sram_device>;
const device_type GBA_ROM_DRILLDOZ = &device_creator<gba_rom_drilldoz_device>;
const device_type GBA_ROM_WARIOTWS = &device_creator<gba_rom_wariotws_device>;
const device_type GBA_ROM_EEPROM = &device_creator<gba_rom_eeprom_device>;
const device_type GBA_ROM_YOSHIUG = &device_creator<gba_rom_yoshiug_device>;
const device_type GBA_ROM_EEPROM64 = &device_creator<gba_rom_eeprom64_device>;
const device_type GBA_ROM_BOKTAI = &device_creator<gba_rom_boktai_device>;
const device_type GBA_ROM_FLASH = &device_creator<gba_rom_flash_device>;
const device_type GBA_ROM_FLASH_RTC = &device_creator<gba_rom_flash_rtc_device>;
const device_type GBA_ROM_FLASH1M = &device_creator<gba_rom_flash1m_device>;
const device_type GBA_ROM_FLASH1M_RTC = &device_creator<gba_rom_flash1m_rtc_device>;
const device_type GBA_ROM_3DMATRIX = &device_creator<gba_rom_3dmatrix_device>;


gba_rom_device::gba_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_gba_cart_interface( mconfig, *this )
{
}

gba_rom_device::gba_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, GBA_ROM_STD, "GBA Carts", tag, owner, clock, "gba_rom", __FILE__),
						device_gba_cart_interface( mconfig, *this )
{
}

gba_rom_sram_device::gba_rom_sram_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: gba_rom_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

gba_rom_sram_device::gba_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_device(mconfig, GBA_ROM_SRAM, "GBA Carts + SRAM", tag, owner, clock, "gba_sram", __FILE__)
{
}

gba_rom_drilldoz_device::gba_rom_drilldoz_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_sram_device(mconfig, GBA_ROM_DRILLDOZ, "GBA Carts + SRAM + Rumble", tag, owner, clock, "gba_drilldoz", __FILE__)
{
}

gba_rom_wariotws_device::gba_rom_wariotws_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_sram_device(mconfig, GBA_ROM_WARIOTWS, "GBA Carts + SRAM + Rumble + Gyroscope", tag, owner, clock, "gba_wariotws", __FILE__),
						m_gyro_z(*this, "GYROZ")
{
}

gba_rom_eeprom_device::gba_rom_eeprom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: gba_rom_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

gba_rom_eeprom_device::gba_rom_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_device(mconfig, GBA_ROM_EEPROM, "GBA Carts + EEPROM", tag, owner, clock, "gba_eeprom", __FILE__)
{
}

gba_rom_yoshiug_device::gba_rom_yoshiug_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_eeprom_device(mconfig, GBA_ROM_YOSHIUG, "GBA Carts + EEPROM + Tilt Sensor", tag, owner, clock, "gba_yoshiug", __FILE__),
						m_tilt_x(*this, "TILTX"),
						m_tilt_y(*this, "TILTY")
{
}

gba_rom_eeprom64_device::gba_rom_eeprom64_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: gba_rom_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

gba_rom_eeprom64_device::gba_rom_eeprom64_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_device(mconfig, GBA_ROM_EEPROM64, "GBA Carts + EEPROM 64K", tag, owner, clock, "gba_eeprom64", __FILE__)
{
}

gba_rom_boktai_device::gba_rom_boktai_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_eeprom64_device(mconfig, GBA_ROM_BOKTAI, "GBA Carts + EEPROM 64K + RTC", tag, owner, clock, "gba_boktai", __FILE__),
						m_sensor(*this, "LIGHTSENSE")
{
}

gba_rom_flash_device::gba_rom_flash_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: gba_rom_device(mconfig, type, name, tag, owner, clock, shortname, source),
						m_flash_mask(0),
						m_flash(*this, "flash")
{
}

gba_rom_flash_device::gba_rom_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_device(mconfig, GBA_ROM_FLASH, "GBA Carts + Panasonic Flash", tag, owner, clock, "gba_flash", __FILE__),
						m_flash_mask(0),
						m_flash(*this, "flash")
{
}

gba_rom_flash_rtc_device::gba_rom_flash_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_flash_device(mconfig, GBA_ROM_FLASH_RTC, "GBA Carts + Panasonic Flash + RTC", tag, owner, clock, "gba_flash_rtc", __FILE__)
{
}

gba_rom_flash1m_device::gba_rom_flash1m_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: gba_rom_device(mconfig, type, name, tag, owner, clock, shortname, source),
						m_flash_mask(0),
						m_flash(*this, "flash")
{
}

gba_rom_flash1m_device::gba_rom_flash1m_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_device(mconfig, GBA_ROM_FLASH1M, "GBA Carts + Sanyo Flash", tag, owner, clock, "gba_flash1m", __FILE__),
						m_flash_mask(0),
						m_flash(*this, "flash")
{
}

gba_rom_flash1m_rtc_device::gba_rom_flash1m_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_flash1m_device(mconfig, GBA_ROM_FLASH1M_RTC, "GBA Carts + Sanyo Flash + RTC", tag, owner, clock, "gba_flash1m_rtc", __FILE__)
{
}

gba_rom_3dmatrix_device::gba_rom_3dmatrix_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_device(mconfig, GBA_ROM_3DMATRIX, "GBA Carts + 3D Matrix Memory Mapper", tag, owner, clock, "gba_3dmatrix", __FILE__)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void gba_rom_device::device_start()
{
	save_item(NAME(m_gpio_regs));
	save_item(NAME(m_gpio_write_only));
	save_item(NAME(m_gpio_dirs));
}

void gba_rom_device::device_reset()
{
	m_gpio_regs[0] = 0;
	m_gpio_regs[1] = 0;
	m_gpio_regs[2] = 0;
	m_gpio_regs[3] = 0;
	m_gpio_write_only = 0;
	m_gpio_dirs = 0;
}

void gba_rom_wariotws_device::device_start()
{
	save_item(NAME(m_last_val));
	save_item(NAME(m_counter));
}

void gba_rom_wariotws_device::device_reset()
{
	m_last_val = 0;
	m_counter = 0;
}

void gba_rom_flash_device::device_reset()
{
	m_flash_mask = 0xffff/4;
}

void gba_rom_flash1m_device::device_reset()
{
	m_flash_mask = 0x1ffff/4;
}


void gba_rom_eeprom_device::device_start()
{
	// for the moment we use a custom eeprom implementation, so we alloc/save it as nvram
	nvram_alloc(0x200);
	m_eeprom = std::make_unique<gba_eeprom_device>(machine(), (UINT8*)get_nvram_base(), get_nvram_size(), 6);
}

void gba_rom_yoshiug_device::device_start()
{
	gba_rom_eeprom_device::device_start();
	save_item(NAME(m_tilt_ready));
	save_item(NAME(m_xpos));
	save_item(NAME(m_ypos));
}

void gba_rom_yoshiug_device::device_reset()
{
	m_tilt_ready = 0;
	m_xpos = 0;
	m_ypos = 0;
}


void gba_rom_eeprom64_device::device_start()
{
	// for the moment we use a custom eeprom implementation, so we alloc/save it as nvram
	nvram_alloc(0x2000);
	m_eeprom = std::make_unique<gba_eeprom_device>(machine(), (UINT8*)get_nvram_base(), get_nvram_size(), 14);
}

void gba_rom_boktai_device::device_start()
{
	gba_rom_eeprom64_device::device_start();
	m_rtc = std::make_unique<gba_s3511_device>(machine());

	save_item(NAME(m_last_val));
	save_item(NAME(m_counter));
}

void gba_rom_boktai_device::device_reset()
{
	m_last_val = 0;
	m_counter = 0;
}

void gba_rom_flash_rtc_device::device_start()
{
	m_rtc = std::make_unique<gba_s3511_device>(machine());
}

void gba_rom_flash1m_rtc_device::device_start()
{
	m_rtc = std::make_unique<gba_s3511_device>(machine());
}

void gba_rom_3dmatrix_device::device_start()
{
	save_item(NAME(m_src));
	save_item(NAME(m_dst));
	save_item(NAME(m_nblock));
}

void gba_rom_3dmatrix_device::device_reset()
{
	m_src = 0;
	m_dst = 0;
	m_nblock = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------
 This is a preliminary implementation of the
 General Purpose I/O Port embedded in the GBA PCBs
 as described at : http://problemkaputt.de/gbatek.htm#gbacartioportgpio

 Functions read_gpio/write_gpio only give the
 I/O interface while the actual on-cart devices
 are read and written through gpio_dev_read/gpio_dev_write
 which are virtual methods defined in the specific
 cart types.
 -------------------------------------------------*/

READ32_MEMBER(gba_rom_device::read_gpio)
{
	logerror("read GPIO offs %X\n", offset);
	if (!m_gpio_write_only)
	{
		switch (offset)
		{
			case 0:
			default:
				if (ACCESSING_BITS_0_15)
				{
					UINT16 ret = gpio_dev_read(m_gpio_dirs);
					return ret;
				}
				if (ACCESSING_BITS_16_31)
					return m_gpio_regs[1] << 16;
			case 1:
				if (ACCESSING_BITS_0_15)
					return m_gpio_regs[2];
				if (ACCESSING_BITS_16_31)
					return m_gpio_regs[3] << 16;
		}
		return 0;
	}
	else
		return m_rom[offset + 0xc4/4];
}

WRITE32_MEMBER(gba_rom_device::write_gpio)
{
	logerror("write GPIO offs %X data %X\n", offset, data);
	switch (offset)
	{
		case 0:
		default:
			if (ACCESSING_BITS_0_15)
			{
				gpio_dev_write(data & 0xffff, m_gpio_dirs);
			}
			if (ACCESSING_BITS_16_31)
			{
				m_gpio_dirs = (data >> 16) & 0x0f;
				m_gpio_regs[1] = (data >> 16) & 0xffff;
			}
			break;
		case 1:
			if (ACCESSING_BITS_0_15)
			{
				m_gpio_write_only = BIT(data, 0) ? 0 : 1;
				m_gpio_regs[2] = data & 0xffff;
			}
			if (ACCESSING_BITS_16_31)
				m_gpio_regs[3] = (data >> 16) & 0xffff;
			break;
	}
}


/*-------------------------------------------------
 Carts with SRAM
 -------------------------------------------------*/

READ32_MEMBER(gba_rom_sram_device::read_ram)
{
	if (!m_nvram.empty() && offset < m_nvram.size())
		return m_nvram[offset];
	else    // this cannot actually happen...
		return 0xffffffff;
}

WRITE32_MEMBER(gba_rom_sram_device::write_ram)
{
	if (!m_nvram.empty() && offset < m_nvram.size())
		COMBINE_DATA(&m_nvram[offset]);
}


// SRAM cart variant with additional Rumble motor (used by Drill Dozer)

void gba_rom_drilldoz_device::gpio_dev_write(UINT16 data, int gpio_dirs)
{
	if ((gpio_dirs & 0x08))
	{
		// send impulse to Rumble sensor
		machine().output().set_value("Rumble", BIT(data, 3));
	}
}


// SRAM cart variant with additional Rumble motor + Gyroscope (used by Warioware Twist)

static INPUT_PORTS_START( wariotws_gyroscope )
	PORT_START("GYROZ")
	PORT_BIT( 0xfff, 0x6c0, IPT_AD_STICK_Z ) PORT_MINMAX(0x354,0x9e3) PORT_SENSITIVITY(0x10) PORT_KEYDELTA(0x50)
INPUT_PORTS_END

ioport_constructor gba_rom_wariotws_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( wariotws_gyroscope );
}

UINT16 gba_rom_wariotws_device::gpio_dev_read(int gpio_dirs)
{
	int gyro = 0;
	if (gpio_dirs == 0x0b)
		gyro = BIT(m_gyro_z->read(), m_counter);
	return (gyro << 2);
}

void gba_rom_wariotws_device::gpio_dev_write(UINT16 data, int gpio_dirs)
{
	if ((gpio_dirs & 0x08))
	{
		// send impulse to Rumble sensor
		machine().output().set_value("Rumble", BIT(data, 3));
	}

	if (gpio_dirs == 0x0b)
	{
		if ((data & 2) && (m_counter > 0))
			m_counter--;

		if (data & 1)
			m_counter = 15;

		m_last_val = data & 0x0b;
	}
}



/*-------------------------------------------------
 Carts with Flash RAM
 -------------------------------------------------*/

static MACHINE_CONFIG_FRAGMENT( panasonic_flash )
	MCFG_PANASONIC_MN63F805MNP_ADD("flash")
MACHINE_CONFIG_END

machine_config_constructor gba_rom_flash_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( panasonic_flash );
}


READ32_MEMBER(gba_rom_flash_device::read_ram)
{
	UINT32 rv = 0;

	offset &= m_flash_mask;

	if (mem_mask & 0xff)
		rv |= m_flash->read(offset * 4);
	if (mem_mask & 0xff00)
		rv |= m_flash->read((offset * 4) + 1) << 8;
	if (mem_mask & 0xff0000)
		rv |= m_flash->read((offset * 4) + 2) << 16;
	if (mem_mask & 0xff000000)
		rv |= m_flash->read((offset * 4) + 3) << 24;

	return rv;
}

WRITE32_MEMBER(gba_rom_flash_device::write_ram)
{
	offset &= m_flash_mask;

	switch (mem_mask)
	{
		case 0xff:
			m_flash->write(offset * 4, data & 0xff);
			break;
		case 0xff00:
			m_flash->write((offset * 4) + 1, (data >> 8) & 0xff);
			break;
		case 0xff0000:
			m_flash->write((offset * 4) + 2, (data >> 16) & 0xff);
			break;
		case 0xff000000:
			m_flash->write((offset * 4) + 3, (data >> 24) & 0xff);
			break;
		default:
			fatalerror("Unknown mem_mask for GBA flash write %x\n", mem_mask);
	}
}

static MACHINE_CONFIG_FRAGMENT( sanyo_flash )
	MCFG_SANYO_LE26FV10N1TS_ADD("flash")
MACHINE_CONFIG_END

machine_config_constructor gba_rom_flash1m_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sanyo_flash );
}


READ32_MEMBER(gba_rom_flash1m_device::read_ram)
{
	UINT32 rv = 0;

	offset &= m_flash_mask;

	if (mem_mask & 0xff)
		rv |= m_flash->read(offset * 4);
	if (mem_mask & 0xff00)
		rv |= m_flash->read((offset * 4) + 1) << 8;
	if (mem_mask & 0xff0000)
		rv |= m_flash->read((offset * 4) + 2) << 16;
	if (mem_mask & 0xff000000)
		rv |= m_flash->read((offset * 4) + 3) << 24;

	return rv;
}

WRITE32_MEMBER(gba_rom_flash1m_device::write_ram)
{
	offset &= m_flash_mask;

	switch (mem_mask)
	{
		case 0xff:
			m_flash->write(offset * 4, data & 0xff);
			break;
		case 0xff00:
			m_flash->write((offset * 4) + 1, (data >> 8) & 0xff);
			break;
		case 0xff0000:
			m_flash->write((offset * 4) + 2, (data >> 16) & 0xff);
			break;
		case 0xff000000:
			m_flash->write((offset * 4) + 3, (data >> 24) & 0xff);
			break;
		default:
			fatalerror("Unknown mem_mask for GBA flash write %x\n", mem_mask);
	}
}

// cart variants with additional S3511 RTC

UINT16 gba_rom_flash_rtc_device::gpio_dev_read(int gpio_dirs)
{
	return 5 | (m_rtc->read_line() << 1);
}

void gba_rom_flash_rtc_device::gpio_dev_write(UINT16 data, int gpio_dirs)
{
	m_rtc->write(data, gpio_dirs);
}


UINT16 gba_rom_flash1m_rtc_device::gpio_dev_read(int gpio_dirs)
{
	return 5 | (m_rtc->read_line() << 1);
}

void gba_rom_flash1m_rtc_device::gpio_dev_write(UINT16 data, int gpio_dirs)
{
	m_rtc->write(data, gpio_dirs);
}


/*-------------------------------------------------
 Carts with EEPROM
 -------------------------------------------------*/

READ32_MEMBER(gba_rom_eeprom_device::read_ram)
{
	// Larger games have smaller access to EERPOM content
	if (m_rom_size > (16 * 1024 * 1024) && offset < 0xffff00/4)
		return 0xffffffff;

	return m_eeprom->read();
}

WRITE32_MEMBER(gba_rom_eeprom_device::write_ram)
{
	// Larger games have smaller access to EEPROM content
	if (m_rom_size > (16 * 1024 * 1024) && offset < 0xffff00/4)
		return;

	if (~mem_mask == 0x0000ffff)
		data >>= 16;

	m_eeprom->write(data);
}

READ32_MEMBER(gba_rom_eeprom64_device::read_ram)
{
	// Larger games have smaller access to EERPOM content
	if (m_rom_size > (16 * 1024 * 1024) && offset < 0xffff00/4)
		return 0xffffffff;

	return m_eeprom->read();
}

WRITE32_MEMBER(gba_rom_eeprom64_device::write_ram)
{
	// Larger games have smaller access to EEPROM content
	if (m_rom_size > (16 * 1024 * 1024) && offset < 0xffff00/4)
		return;

	if (~mem_mask == 0x0000ffff)
		data >>= 16;

	m_eeprom->write(data);
}


/*-------------------------------------------------
 Carts with EEPROM + Tilt Sensor

 Note about the calibration: this can seem a bit
 tricky at first, because the emulated screen
 does not turn as the GBA would...
 In order to properly calibrate the sensor, just
 keep pressed right for a few seconds when requested
 to calibrate right inclination (first calibration
 screen in Yoshi Universal Gravitation) so to get the
 full right range; then keep pressed for left for a
 few seconds when requested to calibrate left
 inclination (second calibration screen in Yoshi
 Universal Gravitation) so to get the full left range

 -------------------------------------------------*/

static INPUT_PORTS_START( yoshiug_tilt )
	PORT_START("TILTX")
	PORT_BIT( 0xfff, 0x3a0, IPT_AD_STICK_X ) PORT_MINMAX(0x2af,0x477) PORT_SENSITIVITY(0x30) PORT_KEYDELTA(0x50)
	PORT_START("TILTY")
	PORT_BIT( 0xfff, 0x3a0, IPT_AD_STICK_Y ) PORT_MINMAX(0x2c3,0x480) PORT_SENSITIVITY(0x30) PORT_KEYDELTA(0x50)
INPUT_PORTS_END

ioport_constructor gba_rom_yoshiug_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( yoshiug_tilt );
}


READ32_MEMBER(gba_rom_yoshiug_device::read_tilt)
{
	switch (offset)
	{
		case 0x200/4:
			if (ACCESSING_BITS_0_15)
				return (m_xpos & 0xff);
			break;
		case 0x300/4:
			if (ACCESSING_BITS_0_15)
				return ((m_xpos >> 8) & 0x0f) | 0x80;
			break;
		case 0x400/4:
			if (ACCESSING_BITS_0_15)
				return (m_ypos & 0xff);
			break;
		case 0x500/4:
			if (ACCESSING_BITS_0_15)
				return ((m_ypos >> 8) & 0x0f);
			break;
		default:
			break;
	}
	return 0xffffffff;
}

WRITE32_MEMBER(gba_rom_yoshiug_device::write_tilt)
{
	switch (offset)
	{
		case 0x000/4:
			if (data == 0x55) m_tilt_ready = 1;
			break;
		case 0x100/4:
			if (data == 0xaa)
			{
				m_xpos = m_tilt_x->read();
				m_ypos = m_tilt_y->read();
				m_tilt_ready = 0;
			}
			break;
		default:
			break;
	}
}


/*-------------------------------------------------
 Carts with EEPROM + S3511 RTC + Light Sensor
 -------------------------------------------------*/

static INPUT_PORTS_START( boktai_sensor )
	PORT_START("LIGHTSENSE")
	PORT_CONFNAME( 0xff, 0xe8, "Light Sensor" )
	PORT_CONFSETTING( 0xe8, "Complete Darkness" )
	PORT_CONFSETTING( 0xe4, "10%" )
	PORT_CONFSETTING( 0xdc, "20%" )
	PORT_CONFSETTING( 0xd4, "30%" )
	PORT_CONFSETTING( 0xc8, "40%" )
	PORT_CONFSETTING( 0xb8, "50%" )
	PORT_CONFSETTING( 0xa8, "60%" )
	PORT_CONFSETTING( 0x98, "70%" )
	PORT_CONFSETTING( 0x88, "80%" )
	PORT_CONFSETTING( 0x68, "90%" )
	PORT_CONFSETTING( 0x48, "Very Bright" )
INPUT_PORTS_END


ioport_constructor gba_rom_boktai_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( boktai_sensor );
}

UINT16 gba_rom_boktai_device::gpio_dev_read(int gpio_dirs)
{
	int light = (gpio_dirs == 7 && m_counter >= m_sensor->read()) ? 1 : 0;
	return 5 | (m_rtc->read_line() << 1) | (light << 3);
}

void gba_rom_boktai_device::gpio_dev_write(UINT16 data, int gpio_dirs)
{
	m_rtc->write(data, gpio_dirs);
	if (gpio_dirs == 7)
	{
		if (data & 2)
			m_counter = 0;

		if ((data & 1) && !(m_last_val & 1))
		{
			m_counter++;
			if (m_counter == 0x100)
				m_counter = 0;
		}

		m_last_val = data & 7;
	}
}




/*-------------------------------------------------
 Carts with 3D Matrix Memory controller

 Used by Video carts with 64MB ROM chips
 Emulation based on the reverse engineering efforts
 by endrift

 The Memory controller basically behaves like a DMA
 chip by writing first source and destination address,
 then the number of 512K blocks to copy and finally
 by issuing the transfer command.
 Disney Collection 2 carts uses command 0x01 to start
 the transfer, other carts might use 0x11 but currently
 they die before getting to the mapper communication
 (CPU emulation issue? cart mapping issue? still unknown)

 To investigate:
 - why the other carts fail
 - which addresses might be used by the mapper
   (Disney Collection 2 uses 0x08800180-0x0880018f
   but it might well be possible to issue commands
   in an extended range...)
 - which bus addresses can be used by the mapper
   (currently we restrict the mapping in the range
   0x08000000-0x09ffffff but maybe also the rest of
   the cart "range" is accessible...)
 -------------------------------------------------*/

WRITE32_MEMBER(gba_rom_3dmatrix_device::write_mapper)
{
	//printf("mapper write 0x%.8X - 0x%X\n", offset, data);
	switch (offset & 3)
	{
		case 0:
			if (data == 0x1)    // transfer data
				memcpy((UINT8 *)m_romhlp + m_dst, (UINT8 *)m_rom + m_src, m_nblock * 0x200);
			else
				printf("Unknown mapper command 0x%X\n", data);
			break;
		case 1:
			m_src = data;
			break;
		case 2:
			if (data >= 0xa000000)
				printf("Unknown transfer destination 0x%X\n", data);
			m_dst = (data & 0x1ffffff);
			break;
		case 3:
		default:
			m_nblock = data;
			break;
	}
}



// Additional devices, to be moved to separate source files at a later stage

/*-------------------------------------------------
 Seiko S-3511 RTC implementation

 TODO: transform this into a separate device, using
 also dirtc.cpp!
 -------------------------------------------------*/

gba_s3511_device::gba_s3511_device(running_machine &machine) :
			m_phase(S3511_RTC_IDLE),
			m_machine(machine)
{
	m_last_val = 0;
	m_bits = 0;
	m_command = 0;
	m_data_len = 1;
	m_data[0] = 0;

	m_machine.save().save_item(m_phase, "GBA_RTC/m_phase");
	m_machine.save().save_item(m_data, "GBA_RTC/m_data");
	m_machine.save().save_item(m_last_val, "GBA_RTC/m_last_val");
	m_machine.save().save_item(m_bits, "GBA_RTC/m_bits");
	m_machine.save().save_item(m_command, "GBA_RTC/m_command");
	m_machine.save().save_item(m_data_len, "GBA_RTC/m_data_len");
}


UINT8 gba_s3511_device::convert_to_bcd(int val)
{
	return (((val % 100) / 10) << 4) | (val % 10);
}

void gba_s3511_device::update_time(int len)
{
	system_time curtime;
	m_machine.current_datetime(curtime);

	if (len == 7)
	{
		m_data[0] = convert_to_bcd(curtime.local_time.year);
		m_data[1] = convert_to_bcd(curtime.local_time.month + 1);
		m_data[2] = convert_to_bcd(curtime.local_time.mday);
		m_data[3] = convert_to_bcd(curtime.local_time.weekday);
		m_data[4] = convert_to_bcd(curtime.local_time.hour);
		m_data[5] = convert_to_bcd(curtime.local_time.minute);
		m_data[6] = convert_to_bcd(curtime.local_time.second);
	}
	else if (len == 3)
	{
		m_data[0] = convert_to_bcd(curtime.local_time.hour);
		m_data[1] = convert_to_bcd(curtime.local_time.minute);
		m_data[2] = convert_to_bcd(curtime.local_time.second);
	}
}


int gba_s3511_device::read_line()
{
	int pin = 0;
	switch (m_phase)
	{
		case S3511_RTC_DATAOUT:
			//printf("mmm %d - %X - %d - %d\n", m_bits, m_data[m_bits >> 3], m_bits >> 3, BIT(m_data[m_bits >> 3], (m_bits & 7)));
			pin = BIT(m_data[m_bits >> 3], (m_bits & 7));
			m_bits++;
			if (m_bits == 8 * m_data_len)
			{
				//for (int i = 0; i < m_data_len; i++)
				//  printf("RTC DATA OUT COMPLETE %X (reg %d) \n", m_data[i], i);
				m_bits = 0;
				m_phase = S3511_RTC_IDLE;
			}
			break;
	}
	return pin;
}


void gba_s3511_device::write(UINT16 data, int gpio_dirs)
{
//  printf("gpio_dev_write data %X\n", data);
	if (m_phase == S3511_RTC_IDLE && (m_last_val & 5) == 1 && (data & 5) == 5)
	{
		m_phase = S3511_RTC_COMMAND;
		m_bits = 0;
		m_command = 0;
	}
	else
	{
//      if (m_phase == 3)
//          printf("RTC command OK\n");
		if (!(m_last_val & 1) && (data & 1))
		{
			// bit transfer
			m_last_val = data & 0xff;
			switch (m_phase)
			{
				case S3511_RTC_DATAIN:
					if (!BIT(gpio_dirs, 1))
					{
						m_data[m_bits >> 3] = (m_data[m_bits >> 3] >> 1) | ((data << 6) & 0x80);
						m_bits++;
						if (m_bits == 8 * m_data_len)
						{
							//for (int i = 0; i < m_data_len; i++)
							//  printf("RTC DATA IN COMPLETE %X (reg %d) \n", m_data[i], i);
							m_bits = 0;
							m_phase = S3511_RTC_IDLE;
						}
					}
					break;
				case S3511_RTC_DATAOUT:
					break;
				case S3511_RTC_COMMAND:
					m_command |= (BIT(data, 1) << (7 - m_bits));
					m_bits++;
					if (m_bits == 8)
					{
						m_bits = 0;
						//printf("RTC command %X ENTERED!!!\n", m_command);
						switch (m_command)
						{
							case 0x60:
								// reset?
								m_phase = S3511_RTC_IDLE;
								m_bits = 0;
								break;
							case 0x62:
								m_phase = S3511_RTC_DATAIN;
								m_data_len = 1;
								break;
							case 0x63:
								m_data_len = 1;
								m_data[0] = 0x40;
								m_phase = S3511_RTC_DATAOUT;
								break;
							case 0x64:
								break;
							case 0x65:
								m_data_len = 7;
								update_time(m_data_len);
								m_phase = S3511_RTC_DATAOUT;
								break;
							case 0x67:
								m_data_len = 3;
								update_time(m_data_len);
								m_phase = S3511_RTC_DATAOUT;
								break;
							default:
								printf("Unknown RTC command %02X\n", m_command);
								m_phase = S3511_RTC_IDLE;
								break;
						}
					}
					break;
				case S3511_RTC_IDLE:
				default:
					break;
			}
		}
		else
			m_last_val = data & 0xff;
	}
}


/*-------------------------------------------------
 GBA EEPROM Device

 TODO: can this sketchy EEPROM device be merged
       with the core implementation?
 -------------------------------------------------*/

//

gba_eeprom_device::gba_eeprom_device(running_machine &machine, UINT8 *eeprom, UINT32 size, int addr_bits) :
					m_state(EEP_IDLE),
					m_machine(machine)
{
	m_data = eeprom;
	m_data_size = size;
	m_addr_bits = addr_bits;

	m_machine.save().save_item(m_state, "GBA_EEPROM/m_state");
	m_machine.save().save_item(m_command, "GBA_EEPROM/m_command");
	m_machine.save().save_item(m_count, "GBA_EEPROM/m_count");
	m_machine.save().save_item(m_addr, "GBA_EEPROM/m_addr");
	m_machine.save().save_item(m_bits, "GBA_EEPROM/m_bits");
	m_machine.save().save_item(m_eep_data, "GBA_EEPROM/m_eep_data");
}

UINT32 gba_eeprom_device::read()
{
	UINT32 out;

	switch (m_state)
	{
		case EEP_IDLE:
//          printf("eeprom_r: @ %x, mask %08x (state %d) (PC=%x) = %d\n", offset, ~mem_mask, m_state, activecpu_get_pc(), 1);
			return 0x00010001;  // "ready"

		case EEP_READFIRST:
			m_count--;

			if (!m_count)
			{
				m_count = 64;
				m_bits = 0;
				m_eep_data = 0;
				m_state = EEP_READ;
			}
			break;
		case EEP_READ:
			if ((m_bits == 0) && (m_count))
			{
				if (m_addr >= m_data_size)
				{
					fatalerror("eeprom: invalid address (%x)\n", m_addr);
				}
				m_eep_data = m_data[m_addr];
				//printf("EEPROM read @ %x = %x (%x)\n", m_addr, m_eep_data, (m_eep_data & 0x80) ? 1 : 0);
				m_addr++;
				m_bits = 8;
			}

			out = (m_eep_data & 0x80) ? 1 : 0;
			out |= (out<<16);
			m_eep_data <<= 1;

			m_bits--;
			m_count--;

			if (!m_count)
			{
				m_state = EEP_IDLE;
			}

//          printf("out = %08x\n", out);
//          printf("eeprom_r: @ %x, mask %08x (state %d) (PC=%x) = %08x\n", offset, ~mem_mask, m_state, activecpu_get_pc(), out);
			return out;
	}
//  printf("eeprom_r: @ %x, mask %08x (state %d) (PC=%x) = %d\n", offset, ~mem_mask, m_state, space.device().safe_pc(), 0);
	return 0;
}

void gba_eeprom_device::write(UINT32 data)
{
//  printf("eeprom_w: %x @ %x (state %d) (PC=%x)\n", data, offset, m_state, space.device().safe_pc());
	switch (m_state)
	{
		case EEP_IDLE:
			if (data == 1)
				m_state++;
			break;

		case EEP_COMMAND:
			if (data == 1)
				m_command = EEP_READFIRST;
			else
				m_command = EEP_WRITE;
			m_state = EEP_ADDR;
			m_count = m_addr_bits;
			m_addr = 0;
			break;

		case EEP_ADDR:
			m_addr <<= 1;
			m_addr |= (data & 1);
			m_count--;
			if (!m_count)
			{
				m_addr *= 8; // each address points to 8 bytes
				if (m_command == EEP_READFIRST)
					m_state = EEP_AFTERADDR;
				else
				{
					m_count = 64;
					m_bits = 8;
					m_state = EEP_WRITE;
					m_eep_data = 0;
				}
			}
			break;

		case EEP_AFTERADDR:
			m_state = m_command;
			m_count = 64;
			m_bits = 0;
			m_eep_data = 0;
			if (m_state == EEP_READFIRST)
				m_count = 4;
			break;

		case EEP_WRITE:
			m_eep_data <<= 1;
			m_eep_data |= (data & 1);
			m_bits--;
			m_count--;

			if (m_bits == 0)
			{
				osd_printf_verbose("%08x: EEPROM: %02x to %x\n", machine().device("maincpu")->safe_pc(), m_eep_data, m_addr);
				if (m_addr >= m_data_size)
					fatalerror("eeprom: invalid address (%x)\n", m_addr);

				m_data[m_addr] = m_eep_data;
				m_addr++;
				m_eep_data = 0;
				m_bits = 8;
			}

			if (!m_count)
				m_state = EEP_AFTERWRITE;
			break;

		case EEP_AFTERWRITE:
			m_state = EEP_IDLE;
			break;
	}
}
