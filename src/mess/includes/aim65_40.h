#pragma once

#ifndef __AIM65_40__
#define __AIM65_40__

#define M6502_TAG       "m6502"
#define M6522_0_TAG     "m6522_0"
#define M6522_1_TAG     "m6522_1"
#define M6522_2_TAG     "m6522_2"
#define M6551_TAG       "m6551"

class aim65_40_state : public driver_device
{
public:
	aim65_40_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* devices */
	device_t *m_via0;
	device_t *m_via1;
	device_t *m_via2;
	device_t *m_speaker;
};

#endif
