#include "sms.h"

class smsbootleg_state : public sms_state
{
public:
	smsbootleg_state(const machine_config &mconfig, device_type type, const char *tag)
		: sms_state(mconfig, type, tag)
	{}

	void sms_supergame(machine_config &config);

	void init_sms_supergame();

private:
	DECLARE_WRITE8_MEMBER(port08_w);
	DECLARE_WRITE8_MEMBER(port18_w);

	void sms_supergame_io(address_map &map);
	void sms_supergame_map(address_map &map);
};
