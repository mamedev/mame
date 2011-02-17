/***************************************************************************

    JPM IMPACT with Video hardware

****************************************************************************/

struct duart_t
{
	UINT8 MR1A, MR2A;
	UINT8 SRA, CSRA;
	UINT8 CRA;
	UINT8 RBA, TBA;

	UINT8 IPCR;
	UINT8 ACR;
	UINT8 ISR, IMR;

	union
	{
		UINT8 CUR, CLR;
		UINT16 CR;
	};
	union
	{
		UINT8 CTUR, CTLR;
		UINT16 CT;
	};

	int tc;

	UINT8 MR1B, MR2B;
	UINT8 SRB, CSRB;
	UINT8 CRB;
	UINT8 RBB, TBB;

	UINT8 IVR;
	UINT8 IP;
	UINT8 OP;
	UINT8 OPR;
	UINT8 OPCR;
};

struct bt477_t
{
	UINT8 address;
	UINT8 addr_cnt;
	UINT8 pixmask;
	UINT8 command;
	rgb_t color;
};

class jpmimpct_state : public driver_device
{
public:
	jpmimpct_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 tms_irq;
	UINT8 duart_1_irq;
	struct duart_t duart_1;
	UINT8 touch_cnt;
	UINT8 touch_data[3];
	int lamp_strobe;
	UINT8 Lamps[256];
	int optic_pattern;
	int payen;
	UINT16 *vram;
	struct bt477_t bt477;
};


/*----------- defined in video/jpmimpct.c -----------*/

READ16_HANDLER( jpmimpct_bt477_r );
WRITE16_HANDLER( jpmimpct_bt477_w );

void jpmimpct_to_shiftreg(address_space *space, UINT32 address, UINT16 *shiftreg);
void jpmimpct_from_shiftreg(address_space *space, UINT32 address, UINT16 *shiftreg);
void jpmimpct_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);

VIDEO_START( jpmimpct );
