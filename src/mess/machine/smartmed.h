/*
    smartmed.h: header file for smartmed.c
*/

#ifndef __SMARTMEDIA_H__
#define __SMARTMEDIA_H__

//#define SMARTMEDIA_IMAGE_SAVE

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct smartmedia_cartslot_config
{
	const char *                    interface;
};


enum sm_mode_t
{
	SM_M_INIT,      // initial state
	SM_M_READ,      // read page data
	SM_M_PROGRAM,   // program page data
	SM_M_ERASE,     // erase block data
	SM_M_READSTATUS,// read status
	SM_M_READID,        // read ID
	SM_M_30,
	SM_M_RANDOM_DATA_INPUT,
	SM_M_RANDOM_DATA_OUTPUT
};

enum pointer_sm_mode_t
{
	SM_PM_A,        // accessing first 256-byte half of 512-byte data field
	SM_PM_B,        // accessing second 256-byte half of 512-byte data field
	SM_PM_C         // accessing spare field
};


// "Sequential Row Read is available only on K9F5608U0D_Y,P,V,F or K9F5608D0D_Y,P"

#define NAND_CHIP_K9F5608U0D   { 2, { 0xEC, 0x75                   },  512, 16,  32, 2048, 1, 2, 1 } /* K9F5608U0D */
#define NAND_CHIP_K9F5608U0D_J { 2, { 0xEC, 0x75                   },  512, 16,  32, 2048, 1, 2, 0 } /* K9F5608U0D-Jxxx */
#define NAND_CHIP_K9F5608U0B   { 2, { 0xEC, 0x75                   },  512, 16,  32, 2048, 1, 2, 0 } /* K9F5608U0B */
#define NAND_CHIP_K9F1G08U0B   { 5, { 0xEC, 0xF1, 0x00, 0x95, 0x40 }, 2048, 64,  64, 1024, 2, 2, 0 } /* K9F1G08U0B */
#define NAND_CHIP_K9LAG08U0M   { 5, { 0xEC, 0xD5, 0x55, 0x25, 0x68 }, 2048, 64, 128, 8192, 2, 3, 0 } /* K9LAG08U0M */

struct nand_chip
{
	int id_len;
	UINT8 id[5];
	int page_size;
	int oob_size;
	int pages_per_block;
	int blocks_per_device;
	int col_address_cycles;
	int row_address_cycles;
	int sequential_row_read;
};

// ======================> nand_interface

struct nand_interface
{
	nand_chip        m_chip;
	devcb_write_line m_devcb_write_line_cb;
};

// ======================> nand_device
class nand_device : public device_t,
					public nand_interface
{
public:
	// construction/destruction
	nand_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	nand_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	int is_present();
	int is_protected();
	int is_busy();

	UINT8 data_r();
	void command_w(UINT8 data);
	void address_w(UINT8 data);
	void data_w(UINT8 data);

	void read(int offset, void *data, int size);

	void set_data_ptr(void *ptr);
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

	int m_page_data_size;   // 256 for a 2MB card, 512 otherwise
	int m_page_total_size;// 264 for a 2MB card, 528 otherwise
	int m_num_pages;        // 8192 for a 4MB card, 16184 for 8MB, 32768 for 16MB,
						// 65536 for 32MB, 131072 for 64MB, 262144 for 128MB...
						// 0 means no card loaded
	int m_log2_pages_per_block; // log2 of number of pages per erase block (usually 4 or 5)

	UINT8 *m_data_ptr;  // FEEPROM data area
	UINT8 *m_data_uid_ptr;

	sm_mode_t m_mode;               // current operation mode
	pointer_sm_mode_t m_pointer_mode;       // pointer mode

	unsigned int m_page_addr;       // page address pointer
	int m_byte_addr;        // byte address pointer
	int m_addr_load_ptr;    // address load pointer

	int m_status;           // current status
	int m_accumulated_status;   // accumulated status

	UINT8 *m_pagereg;   // page register used by program command
	UINT8 m_id[5];      // chip ID
	UINT8 m_mp_opcode;  // multi-plane operation code

	int m_mode_3065;

	// Palm Z22 NAND has 512 + 16 byte pages but, for some reason, Palm OS writes 512 + 64 bytes when
	// programming a page, so we need to keep track of the number of bytes written so we can ignore the
	// last 48 (64 - 16) bytes or else the first 48 bytes get overwritten
	int m_program_byte_count;

	int m_id_len;
	int m_col_address_cycles;
	int m_row_address_cycles;
	int m_sequential_row_read;

	devcb_resolved_write_line m_devcb_write_line_rnb;

	#ifdef SMARTMEDIA_IMAGE_SAVE
	int m_image_format;
	#endif
};

// device type definition
extern const device_type NAND;

class smartmedia_image_device : public nand_device,
								public device_image_interface
{
public:
	// construction/destruction
	smartmedia_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// image-level overrides
	virtual iodevice_t image_type() const { return IO_MEMCARD; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return "sm_memc"; }
	virtual const char *file_extensions() const { return "smc"; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	virtual bool call_load();
	virtual void call_unload();
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry) { return load_software(swlist, swname, start_entry); }

	void set_image_interface(const char *image_interface) { m_image_interface = image_interface; }
protected:
	// device-level overrides
	virtual void device_config_complete();

	bool smartmedia_format_1();
	bool smartmedia_format_2();
	int detect_geometry( UINT8 id1, UINT8 id2);

	const char *m_image_interface;
};

// device type definition
extern const device_type SMARTMEDIA;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_NAND_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, NAND, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define NAND_INTERFACE(name) \
	const nand_interface(name) =

#define MCFG_SMARTMEDIA_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SMARTMEDIA, 0)

#define MCFG_SMARTMEDIA_INTERFACE(_interface)                           \
	downcast<smartmedia_image_device *>(device)->set_image_interface(_interface);

#endif /* __SMARTMEDIA_H__ */
