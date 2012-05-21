#include "machine/68681.h"

#define m68307SER_UMR1_UMR2  (0x01)
#define m68307SER_USR_UCSR   (0x03)
#define m68307SER_UCR        (0x05)
#define m68307SER_URB_UTB    (0x07)
#define m68307SER_UIPCR_UACR (0x09)
#define m68307SER_UISR_UIMR  (0x0b)
#define m68307SER_UBG1       (0x0d)
#define m68307SER_UBG2       (0x0f)
//                           (0x11)
//                           (0x13)
//                           (0x15)
//                           (0x17)
#define m68307SER_UIVR       (0x19)
#define m68307SER_UIP        (0x1b)
#define m68307SER_UOP1       (0x1d)
#define m68307SER_UOP0       (0x1f)


READ8_HANDLER( m68307_internal_serial_r );
WRITE8_HANDLER( m68307_internal_serial_w );

class m68307_serial
{
	public:
	void reset(void);
	UINT8 m_uivr;

	void m68307ser_set_duart68681(device_t* duart68681)
	{
		m_duart68681 = duart68681;
	}

	device_t* m_duart68681;
};

