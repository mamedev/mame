#include "sms.h"

class smsbootleg_state : public sms_state
{
	public:
		smsbootleg_state(const machine_config &mconfig, device_type type, const char *tag)
			: sms_state(mconfig, type, tag)
			{}

	void init_sms_supergame();
	void port08_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port18_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

};
