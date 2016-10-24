// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Bernd Wiebelt, Allard van der Bas

#ifndef BWIDOW_H_
#define BWIDOW_H_

#define MASTER_CLOCK (XTAL_12_096MHz)
#define CLOCK_3KHZ   ((double)MASTER_CLOCK / 4096)


class bwidow_state : public driver_device
{
public:
	bwidow_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_in3(*this, "IN3"),
		m_in4(*this, "IN4"),
		m_dsw2(*this, "DSW2") { }

	int m_lastdata;
	uint8_t spacduel_IN3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t bwidowp_in_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bwidow_misc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spacduel_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value clock_r(ioport_field &field, void *param);
	required_device<cpu_device> m_maincpu;
	optional_ioport m_in3;
	optional_ioport m_in4;
	optional_ioport m_dsw2;
};


/*----------- defined in audio/bwidow.c -----------*/

MACHINE_CONFIG_EXTERN( bwidow_audio );
MACHINE_CONFIG_EXTERN( gravitar_audio );

#endif /* BWIDOW_H_ */
