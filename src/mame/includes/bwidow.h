
#ifndef BWIDOW_H_
#define BWIDOW_H_

#define MASTER_CLOCK (12096000)
#define CLOCK_3KHZ  (MASTER_CLOCK / 4096)


class bwidow_state : public driver_device
{
public:
	bwidow_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_lastdata;
	DECLARE_READ8_MEMBER(spacduel_IN3_r);
	DECLARE_WRITE8_MEMBER(bwidow_misc_w);
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_CUSTOM_INPUT_MEMBER(clock_r);
};


/*----------- defined in audio/bwidow.c -----------*/

MACHINE_CONFIG_EXTERN( bwidow_audio );
MACHINE_CONFIG_EXTERN( gravitar_audio );

#endif /* BWIDOW_H_ */
