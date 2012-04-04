/*************************************************************************

    Atari Major Havoc hardware

*************************************************************************/

#define MHAVOC_CLOCK		10000000
#define MHAVOC_CLOCK_5M		(MHAVOC_CLOCK/2)
#define MHAVOC_CLOCK_2_5M	(MHAVOC_CLOCK/4)
#define MHAVOC_CLOCK_1_25M	(MHAVOC_CLOCK/8)
#define MHAVOC_CLOCK_625K	(MHAVOC_CLOCK/16)

#define MHAVOC_CLOCK_156K	(MHAVOC_CLOCK_625K/4)
#define MHAVOC_CLOCK_5K		(MHAVOC_CLOCK_625K/16/8)
#define MHAVOC_CLOCK_2_4K	(MHAVOC_CLOCK_625K/16/16)


class mhavoc_state : public driver_device
{
public:
	mhavoc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_zram0;
	UINT8 *m_zram1;
	UINT8 m_alpha_data;
	UINT8 m_alpha_rcvd;
	UINT8 m_alpha_xmtd;
	UINT8 m_gamma_data;
	UINT8 m_gamma_rcvd;
	UINT8 m_gamma_xmtd;
	UINT8 m_player_1;
	UINT8 m_alpha_irq_clock;
	UINT8 m_alpha_irq_clock_enable;
	UINT8 m_gamma_irq_clock;
	UINT8 m_has_gamma_cpu;
	UINT8 m_speech_write_buffer;
	DECLARE_READ8_MEMBER(dual_pokey_r);
	DECLARE_WRITE8_MEMBER(dual_pokey_w);
};


/*----------- defined in machine/mhavoc.c -----------*/

TIMER_DEVICE_CALLBACK( mhavoc_cpu_irq_clock );

WRITE8_HANDLER( mhavoc_alpha_irq_ack_w );
WRITE8_HANDLER( mhavoc_gamma_irq_ack_w );

MACHINE_START( mhavoc );
MACHINE_RESET( mhavoc );
DRIVER_INIT( mhavocrv );

WRITE8_HANDLER( mhavoc_gamma_w );
READ8_HANDLER( mhavoc_alpha_r );

WRITE8_HANDLER( mhavoc_alpha_w );
READ8_HANDLER( mhavoc_gamma_r );

WRITE8_HANDLER( mhavoc_ram_banksel_w );
WRITE8_HANDLER( mhavoc_rom_banksel_w );

CUSTOM_INPUT( tms5220_r );
CUSTOM_INPUT( gamma_rcvd_r );
CUSTOM_INPUT( gamma_xmtd_r );
CUSTOM_INPUT( alpha_rcvd_r );
CUSTOM_INPUT( alpha_xmtd_r );
CUSTOM_INPUT( mhavoc_bit67_r );

WRITE8_HANDLER( mhavoc_out_0_w );
WRITE8_HANDLER( alphaone_out_0_w );
WRITE8_HANDLER( mhavoc_out_1_w );


