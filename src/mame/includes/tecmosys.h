/***************************************************************************

    tecmosys protection simulation

***************************************************************************/

enum DEV_STATUS
{
	DS_IDLE,
	DS_LOGIN,
	DS_SEND_CODE,
	DS_SEND_ADRS,
	DS_SEND_CHKSUMS,
	DS_DONE
};

struct prot_data
{
	UINT8 passwd_len;
	UINT8* passwd;
	UINT8* code;
	UINT8 checksum_ranges[17];
	UINT8 checksums[4];
};

/*----------- defined in machine/tecmosys.c -----------*/

READ16_HANDLER(prot_status_r);
WRITE16_HANDLER(prot_status_w);
READ16_HANDLER(prot_data_r);
WRITE16_HANDLER(prot_data_w);

extern UINT8 device_read_ptr;
extern UINT8 device_status;
extern struct prot_data* device_data;
extern struct prot_data deroon_data;
extern struct prot_data tkdensho_data;



