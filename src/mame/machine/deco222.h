
#include "emu.h"
#include "cpu/hd6309/hd6309.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6502/m6502.h"

class deco_222_device : public m6502_device {
public:
	deco_222_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	class mi_decrypt : public mi_default_normal {
	public:
		bool had_written;

		virtual ~mi_decrypt() {}
		virtual UINT8 read_decrypted(UINT16 adr);
	};

	virtual void device_start();
	virtual void device_reset();

};

static const device_type DECO_222 = &device_creator<deco_222_device>;