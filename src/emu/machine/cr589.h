#include "scsicd.h"

class cr589_device : public scsicd_device
{
public:
	// construction/destruction
	cr589_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void ExecCommand( int *transferLength );
	virtual void WriteData( UINT8 *data, int dataLength );
	virtual void ReadData( UINT8 *data, int dataLength );

protected:
	// device-level overrides
	virtual void device_start();

private:
	int download;
	UINT8 buffer[ 65536 ];
	int bufferOffset;
};

// device type definition
extern const device_type CR589;
