
class mexico86_state : public driver_device
{
public:
	mexico86_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_objectram(*this, "objectram"),
		m_protection_ram(*this, "protection_ram"),
		m_videoram(*this, "videoram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_objectram;
	required_shared_ptr<UINT8> m_protection_ram;
	required_shared_ptr<UINT8> m_videoram;

	/* video-related */
	int      m_charbank;

	/* mcu */
	/* mexico86 68705 protection */
	UINT8    m_port_a_in;
	UINT8    m_port_a_out;
	UINT8    m_ddr_a;
	UINT8    m_port_b_in;
	UINT8    m_port_b_out;
	UINT8    m_ddr_b;
	int      m_address;
	int      m_latch;
	/* kikikai mcu simulation */
	int      m_mcu_running;
	int      m_mcu_initialised;
	int      m_coin_last;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	cpu_device *m_subcpu;
	device_t *m_mcu;

	/* queue */
	UINT8 m_queue[64];
	int m_qfront;
	int m_qstate;
	DECLARE_WRITE8_MEMBER(mexico86_sub_output_w);
	DECLARE_WRITE8_MEMBER(mexico86_f008_w);
	DECLARE_READ8_MEMBER(mexico86_68705_port_a_r);
	DECLARE_WRITE8_MEMBER(mexico86_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(mexico86_68705_ddr_a_w);
	DECLARE_READ8_MEMBER(mexico86_68705_port_b_r);
	DECLARE_WRITE8_MEMBER(mexico86_68705_port_b_w);
	DECLARE_WRITE8_MEMBER(mexico86_68705_ddr_b_w);
	DECLARE_WRITE8_MEMBER(mexico86_bankswitch_w);
	DECLARE_READ8_MEMBER(kiki_ym2203_r);
	virtual void machine_start();
	virtual void machine_reset();
};


/*----------- defined in machine/mexico86.c -----------*/

INTERRUPT_GEN( kikikai_interrupt );
INTERRUPT_GEN( mexico86_m68705_interrupt );


/*----------- defined in video/mexico86.c -----------*/


SCREEN_UPDATE_IND16( mexico86 );
SCREEN_UPDATE_IND16( kikikai );
