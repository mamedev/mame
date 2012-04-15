/***************************************************************************

    Sigma Spiders hardware

***************************************************************************/


#define NUM_PENS	(8)

class spiders_state : public driver_device
{
public:
	spiders_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_ram(*this, "ram"){ }

	required_shared_ptr<UINT8> m_ram;
	UINT8 m_flipscreen;
	UINT16 m_gfx_rom_address;
	UINT8 m_gfx_rom_ctrl_mode;
	UINT8 m_gfx_rom_ctrl_latch;
	UINT8 m_gfx_rom_ctrl_data;
	pen_t m_pens[NUM_PENS];
};


/*----------- defined in audio/spiders.c -----------*/

WRITE8_DEVICE_HANDLER( spiders_audio_command_w );
WRITE8_DEVICE_HANDLER( spiders_audio_a_w );
WRITE8_DEVICE_HANDLER( spiders_audio_b_w );
WRITE8_DEVICE_HANDLER( spiders_audio_ctrl_w );

MACHINE_CONFIG_EXTERN( spiders_audio );
