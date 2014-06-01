#include "emu.h"
#include "yamaha.h"


const device_type MSX_CART_SFG01 = &device_creator<msx_cart_sfg01>;


msx_cart_sfg01::msx_cart_sfg01(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_SFG01, "MSX Cartridge - SFG01", tag, owner, clock, "msx_cart_sfg01", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_region_sfg01(*this, "sfg01")
	, m_ym2151(*this, "ym2151")
	, m_kbdc(*this, "kbdc")
	, m_ym2151_irq_state(CLEAR_LINE)
	, m_ym2148_irq_state(CLEAR_LINE)
{
}


static MACHINE_CONFIG_FRAGMENT( msx_sfg01 )
	// YM2151 (OPM)
	// YM3012 (DAC)
	// YM2148 (MKS)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_YM2151_ADD("ym2151", XTAL_4MHz)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(msx_cart_sfg01, ym2151_irq_w))
//	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(msx_cart_sfg01, port_w))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	MCFG_MSX_AUDIO_KBDC_PORT_ADD("kbdc", msx_audio_keyboards, NULL)
MACHINE_CONFIG_END


machine_config_constructor msx_cart_sfg01::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( msx_sfg01 );
}


ROM_START( msx_sfg01 )
	ROM_REGION(0x4000, "sfg01", 0)
	ROM_LOAD("sfg01.rom", 0x0, 0x4000, CRC(0995fb36) SHA1(434651305f92aa770a89e40b81125fb22d91603d))
ROM_END


const rom_entry *msx_cart_sfg01::device_rom_region() const
{
	return ROM_NAME( msx_sfg01 );
}


void msx_cart_sfg01::device_start()
{
}


void msx_cart_sfg01::initialize_cartridge()
{
}


WRITE_LINE_MEMBER(msx_cart_sfg01::ym2151_irq_w)
{
	m_ym2151_irq_state = state ? ASSERT_LINE : CLEAR_LINE;
	check_irq();
}


WRITE_LINE_MEMBER(msx_cart_sfg01::ym2148_irq_w)
{
	m_ym2148_irq_state = state ? ASSERT_LINE : CLEAR_LINE;
	check_irq();
}


void msx_cart_sfg01::check_irq()
{
	if (m_ym2151_irq_state != CLEAR_LINE || m_ym2148_irq_state != CLEAR_LINE)
	{
		m_out_irq_cb(ASSERT_LINE);
	}
	else
	{
		m_out_irq_cb(CLEAR_LINE);
	}
}


READ8_MEMBER(msx_cart_sfg01::read_cart)
{
	switch (offset)
	{
		case 0x3ff0:     // YM-2151 status read
		case 0x3ff1:	 // YM-2151 status read mirror?
			return m_ym2151->status_r(space, 0);

		case 0x3ff2:     // YM-2148 keyboard column read
			return m_kbdc->read(space, 0);

		case 0x3ff3:     // YM-2148 --
		case 0x3ff4:     // YM-2148 --
		case 0x3ff5:     // YM-2148 MIDI UART data read register
		case 0x3ff6:     // YM-2148 MIDI UART status register
			break;
	}

	if (offset < 0x8000)
	{
		return m_region_sfg01->u8(offset & 0x3fff);
	}

	return 0xff;
}


WRITE8_MEMBER(msx_cart_sfg01::write_cart)
{
	switch (offset)
	{
		case 0x3ff0:     // YM-2151 register
			m_ym2151->register_w(space, 0, data);
			break;

		case 0x3ff1:    // YM-2151 data
			m_ym2151->data_w(space, 0, data);
			break;

		case 0x3ff2:   // YM-2148 write keyboard row
			m_kbdc->write(space, 0, data);
			break;

		case 0x3ff3:   // YM-2148 MIDI IRQ vector
		case 0x3ff4:   // YM-2148 External IRQ vector
		case 0x3ff5:   // YM-2148 MIDI UART data write register
		case 0x3ff6:   // YM-2148 MIDI UART command register
		default:
			logerror("msx_cart_sfg01::write_cart: write %02x to %04x\n", data, offset);
			break;
	}
}

