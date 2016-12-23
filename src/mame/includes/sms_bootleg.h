#include "sms.h"

class smsbootleg_state : public sms_state
{
	public:
		smsbootleg_state(const machine_config &mconfig, device_type type, const char *tag)
			: sms_state(mconfig, type, tag)
			{}

	DECLARE_DRIVER_INIT(sms_supergame);
	DECLARE_WRITE8_MEMBER(port08_w);
	DECLARE_WRITE8_MEMBER(port18_w);

};
