// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli

struct OHCIEndpointDescriptor {
	int mps; // MaximumPacketSize
	int f; // Format
	int k; // sKip
	int s; // Speed
	int d; // Direction
	int en; // EndpointNumber
	int fa; // FunctionAddress
	UINT32 tailp; // TDQueueTailPointer
	UINT32 headp; // TDQueueHeadPointer
	UINT32 nexted; // NextED
	int c; // toggleCarry
	int h; // Halted
	UINT32 word0;
};

struct OHCITransferDescriptor {
	int cc; // ConditionCode
	int ec; // ErrorCount
	int t; // DataToggle
	int di; // DelayInterrupt
	int dp; // Direction/PID
	int r; // bufferRounding
	UINT32 cbp; // CurrentBufferPointer
	UINT32 nexttd; // NextTD
	UINT32 be; // BufferEnd
	UINT32 word0;
};

struct OHCIIsochronousTransferDescriptor {
	int cc; // ConditionCode
	int fc; // FrameCount
	int di; // DelayInterrupt
	int sf; // StartingFrame
	UINT32 bp0; // BufferPage0
	UINT32 nexttd; // NextTD
	UINT32 be; // BufferEnd
	UINT32 offset[8]; // Offset/PacketStatusWord
};

enum OHCIRegisters {
	HcRevision=0,
	HcControl,
	HcCommandStatus,
	HcInterruptStatus,
	HcInterruptEnable,
	HcInterruptDisable,
	HcHCCA,
	HcPeriodCurrentED,
	HcControlHeadED,
	HcControlCurrentED,
	HcBulkHeadED,
	HcBulkCurrentED,
	HcDoneHead,
	HcFmInterval,
	HcFmRemaining,
	HcFmNumber,
	HcPeriodicStart,
	HcLSThreshold,
	HcRhDescriptorA,
	HcRhDescriptorB,
	HcRhStatus,
	HcRhPortStatus1
};

enum OHCIHostControllerFunctionalState {
	UsbReset=0,
	UsbResume,
	UsbOperational,
	UsbSuspend
};

enum OHCIInterrupt {
	SchedulingOverrun=1,
	WritebackDoneHead=2,
	StartofFrame=4,
	ResumeDetected=8,
	UnrecoverableError=16,
	FrameNumberOverflow=32,
	RootHubStatusChange=64,
	OwnershipChange=0x40000000,
	MasterInterruptEnable=0x80000000
};

enum OHCICompletionCode {
	NoError=0,
	CRC,
	BitStuffing,
	DataToggleMismatch,
	Stall,
	DeviceNotResponding,
	PIDCheckFailure,
	UnexpectedPID,
	DataOverrun,
	DataUnderrun,
	BufferOverrun=12,
	BufferUnderrun,
	NotAccessed=14
};

struct USBSetupPacket {
	UINT8 bmRequestType;
	UINT8 bRequest;
	UINT16 wValue;
	UINT16 wIndex;
	UINT16 wLength;
};

struct USBStandardDeviceDscriptor {
	UINT8 bLength;
	UINT8 bDescriptorType;
	UINT16 bcdUSB;
	UINT8 bDeviceClass;
	UINT8 bDeviceSubClass;
	UINT8 bDeviceProtocol;
	UINT8 bMaxPacketSize0;
	UINT16 idVendor;
	UINT16 idProduct;
	UINT16 bcdDevice;
	UINT8 iManufacturer;
	UINT8 iProduct;
	UINT8 iSerialNumber;
	UINT8 bNumConfigurations;
};

struct USBStandardConfigurationDescriptor {
	UINT8 bLength;
	UINT8 bDescriptorType;
	UINT16 wTotalLength;
	UINT8 bNumInterfaces;
	UINT8 bConfigurationValue;
	UINT8 iConfiguration;
	UINT8 bmAttributes;
	UINT8 MaxPower;
};

struct USBStandardInterfaceDescriptor {
	UINT8 bLength;
	UINT8 bDescriptorType;
	UINT8 bInterfaceNumber;
	UINT8 bAlternateSetting;
	UINT8 bNumEndpoints;
	UINT8 bInterfaceClass;
	UINT8 bInterfaceSubClass;
	UINT8 bInterfaceProtocol;
	UINT8 iInterface;
};

struct USBStandardEndpointDescriptor {
	UINT8 bLength;
	UINT8 bDescriptorType;
	UINT8 bEndpointAddress;
	UINT8 bmAttributes;
	UINT16 wMaxPacketSize;
	UINT8 bInterval;
};

enum USBPid {
	SetupPid=0,
	OutPid,
	InPid
};

enum USBRequestCode {
	GET_STATUS=0,
	CLEAR_FEATURE=1,
	SET_FEATURE=3,
	SET_ADDRESS=5,
	GET_DESCRIPTOR=6,
	SET_DESCRIPTOR=7,
	GET_CONFIGURATION=8,
	SET_CONFIGURATION=9,
	GET_INTERFACE=10,
	SET_INTERFACE=11,
	SYNCH_FRAME=12
};

enum USBDescriptorType {
	DEVICE=1,
	CONFIGURATION=2,
	STRING=3,
	INTERFACE=4,
	ENDPOINT=5
};

class ohci_function_device {
public:
	ohci_function_device();
	void execute_reset();
	int execute_transfer(int address, int endpoint, int pid, UINT8 *buffer, int size);
private:
	int address;
	int controldir;
	int remain;
	UINT8 *position;
};

class xbox_base_state : public driver_device
{
public:
	xbox_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		nvidia_nv2a(nullptr),
		debug_irq_active(false),
		debug_irq_number(0),
		m_maincpu(*this, "maincpu") { }

	DECLARE_READ32_MEMBER(geforce_r);
	DECLARE_WRITE32_MEMBER(geforce_w);
	DECLARE_READ32_MEMBER(usbctrl_r);
	DECLARE_WRITE32_MEMBER(usbctrl_w);
	DECLARE_READ32_MEMBER(smbus_r);
	DECLARE_WRITE32_MEMBER(smbus_w);
	DECLARE_READ32_MEMBER(audio_apu_r);
	DECLARE_WRITE32_MEMBER(audio_apu_w);
	DECLARE_READ32_MEMBER(audio_ac93_r);
	DECLARE_WRITE32_MEMBER(audio_ac93_w);
	DECLARE_READ32_MEMBER(dummy_r);
	DECLARE_WRITE32_MEMBER(dummy_w);

	void smbus_register_device(int address, int(*handler)(xbox_base_state &chs, int command, int rw, int data));
	int smbus_pic16lc(int command, int rw, int data);
	int smbus_cx25871(int command, int rw, int data);
	int smbus_eeprom(int command, int rw, int data);
	void usb_ohci_plug(int port, ohci_function_device *function);
	void usb_ohci_interrupts();
	void usb_ohci_read_endpoint_descriptor(UINT32 address);
	void usb_ohci_writeback_endpoint_descriptor(UINT32 address);
	void usb_ohci_read_transfer_descriptor(UINT32 address);
	void usb_ohci_writeback_transfer_descriptor(UINT32 address);
	void usb_ohci_read_isochronous_transfer_descriptor(UINT32 address);
	void dword_write_le(UINT8 *addr, UINT32 d);
	void word_write_le(UINT8 *addr, UINT16 d);
	void debug_generate_irq(int irq, bool active);
	virtual void hack_eeprom() {};
	virtual void hack_usb() {};

	void vblank_callback(screen_device &screen, bool state);
	UINT32 screen_update_callback(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start();
	DECLARE_WRITE_LINE_MEMBER(xbox_pic8259_1_set_int_line);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE_LINE_MEMBER(xbox_pit8254_out0_changed);
	DECLARE_WRITE_LINE_MEMBER(xbox_pit8254_out2_changed);
	IRQ_CALLBACK_MEMBER(irq_callback);
	TIMER_CALLBACK_MEMBER(audio_apu_timer);
	TIMER_CALLBACK_MEMBER(usb_ohci_timer);

	struct xbox_devices {
		pic8259_device    *pic8259_1;
		pic8259_device    *pic8259_2;
		bus_master_ide_controller_device    *ide;
	} xbox_base_devs;
	struct smbus_state {
		int status;
		int control;
		int address;
		int data;
		int command;
		int rw;
		int(*devices[128])(xbox_base_state &chs, int command, int rw, int data);
		UINT32 words[256 / 4];
	} smbusst;
	struct apu_state {
		UINT32 memory[0x60000 / 4];
		UINT32 gpdsp_sgaddress; // global processor scatter-gather
		UINT32 gpdsp_sgblocks;
		UINT32 gpdsp_address;
		UINT32 epdsp_sgaddress; // encoder processor scatter-gather
		UINT32 epdsp_sgblocks;
		UINT32 unknown_sgaddress;
		UINT32 unknown_sgblocks;
		int voice_number;
		UINT32 voices_heap_blockaddr[1024];
		UINT64 voices_active[4]; //one bit for each voice: 1 playing 0 not
		UINT32 voicedata_address;
		int voices_frequency[256]; // sample rate
		int voices_position[256]; // position in samples * 1000
		int voices_position_start[256]; // position in samples * 1000
		int voices_position_end[256]; // position in samples * 1000
		int voices_position_increment[256]; // position increment every 1ms * 1000
		emu_timer *timer;
		address_space *space;
	} apust;
	struct ac97_state {
		UINT32 mixer_regs[0x80 / 4];
		UINT32 controller_regs[0x38 / 4];
	} ac97st;
	struct ohci_state {
		UINT32 hc_regs[255];
		struct {
			ohci_function_device *function;
			int delay;
		} ports[4 + 1];
		emu_timer *timer;
		int state;
		UINT32 framenumber;
		UINT32 nextinterupted;
		UINT32 nextbulked;
		int interruptbulkratio;
		int writebackdonehadcounter;
		address_space *space;
		UINT8 buffer[1024];
		OHCIEndpointDescriptor endpoint_descriptor;
		OHCITransferDescriptor transfer_descriptor;
		OHCIIsochronousTransferDescriptor isochronous_transfer_descriptor;
	} ohcist;
	UINT8 pic16lc_buffer[0xff];
	nv2a_renderer *nvidia_nv2a;
	bool debug_irq_active;
	int debug_irq_number;
	required_device<cpu_device> m_maincpu;
};

ADDRESS_MAP_EXTERN(xbox_base_map, 32);
ADDRESS_MAP_EXTERN(xbox_base_map_io, 32);
MACHINE_CONFIG_EXTERN(xbox_base);
