/*************************************************************************

    Exidy 440 hardware

*************************************************************************/

#define EXIDY440_MASTER_CLOCK		(XTAL_12_9792MHz)


class exidy440_state : public driver_device
{
public:
	exidy440_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_bank;
	const UINT8 *m_showdown_bank_data[2];
	INT8 m_showdown_bank_select;
	UINT8 m_showdown_bank_offset;
	UINT8 *m_imageram;
	UINT8 *m_scanline;
	UINT8 m_firq_vblank;
	UINT8 m_firq_beam;
	UINT8 *m_topsecex_yscroll;
	UINT8 m_latched_x;
	UINT8 *m_local_videoram;
	UINT8 *m_local_paletteram;
	UINT8 m_firq_enable;
	UINT8 m_firq_select;
	UINT8 m_palettebank_io;
	UINT8 m_palettebank_vis;
	UINT8 *m_spriteram;
	device_t *m_custom;
	DECLARE_WRITE8_MEMBER(bankram_w);
	DECLARE_READ8_MEMBER(exidy440_input_port_3_r);
	DECLARE_READ8_MEMBER(sound_command_ack_r);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(exidy440_input_port_3_w);
	DECLARE_WRITE8_MEMBER(exidy440_coin_counter_w);
	DECLARE_READ8_MEMBER(showdown_bank0_r);
	DECLARE_READ8_MEMBER(claypign_protection_r);
	DECLARE_READ8_MEMBER(topsecex_input_port_5_r);
	DECLARE_WRITE8_MEMBER(topsecex_yscroll_w);
	DECLARE_READ8_MEMBER(exidy440_videoram_r);
	DECLARE_WRITE8_MEMBER(exidy440_videoram_w);
	DECLARE_READ8_MEMBER(exidy440_paletteram_r);
	DECLARE_WRITE8_MEMBER(exidy440_paletteram_w);
	DECLARE_READ8_MEMBER(exidy440_horizontal_pos_r);
	DECLARE_READ8_MEMBER(exidy440_vertical_pos_r);
	DECLARE_WRITE8_MEMBER(exidy440_spriteram_w);
	DECLARE_WRITE8_MEMBER(exidy440_control_w);
	DECLARE_WRITE8_MEMBER(exidy440_interrupt_clear_w);
};


/*----------- defined in drivers/exidy440.c -----------*/

void exidy440_bank_select(running_machine &machine, UINT8 bank);


/*----------- defined in video/exidy440.c -----------*/

INTERRUPT_GEN( exidy440_vblank_interrupt );


MACHINE_CONFIG_EXTERN( exidy440_video );
MACHINE_CONFIG_EXTERN( topsecex_video );
