
DECLARE_READ32_HANDLER( m68340_internal_serial_r );
DECLARE_WRITE32_HANDLER( m68340_internal_serial_w );

class m68340_serial
{
	public:
	void reset(void);
};
