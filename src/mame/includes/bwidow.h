// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Bernd Wiebelt, Allard van der Bas

#ifndef BWIDOW_H_
#define BWIDOW_H_

#define MASTER_CLOCK (XTAL_12_096MHz)
#define CLOCK_3KHZ   ((double)MASTER_CLOCK / 4096)


class bwidow_state : public driver_device
{
public:
	bwidow_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	int m_lastdata;
	DECLARE_READ8_MEMBER(spacduel_IN3_r);
	DECLARE_READ8_MEMBER(bwidowp_in_r);
	DECLARE_WRITE8_MEMBER(bwidow_misc_w);
	DECLARE_WRITE8_MEMBER(spacduel_coin_counter_w);
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_CUSTOM_INPUT_MEMBER(clock_r);
	required_device<cpu_device> m_maincpu;
};


/*----------- defined in audio/bwidow.c -----------*/

MACHINE_CONFIG_EXTERN( bwidow_audio );
MACHINE_CONFIG_EXTERN( gravitar_audio );

#endif /* BWIDOW_H_ */
