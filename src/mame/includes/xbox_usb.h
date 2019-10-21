// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
#ifndef MAME_INCLUDES_XBOX_USB_H
#define MAME_INCLUDES_XBOX_USB_H

#pragma once

#include <forward_list>

struct OHCIEndpointDescriptor {
	int mps; // MaximumPacketSize
	int f; // Format
	int k; // sKip
	int s; // Speed
	int d; // Direction
	int en; // EndpointNumber
	int fa; // FunctionAddress
	uint32_t tailp; // TDQueueTailPointer
	uint32_t headp; // TDQueueHeadPointer
	uint32_t nexted; // NextED
	int c; // toggleCarry
	int h; // Halted
	uint32_t word0;
};

struct OHCITransferDescriptor {
	int cc; // ConditionCode
	int ec; // ErrorCount
	int t; // DataToggle
	int di; // DelayInterrupt
	int dp; // Direction/PID
	int r; // bufferRounding
	uint32_t cbp; // CurrentBufferPointer
	uint32_t nexttd; // NextTD
	uint32_t be; // BufferEnd
	uint32_t word0;
};

struct OHCIIsochronousTransferDescriptor {
	int cc; // ConditionCode
	int fc; // FrameCount
	int di; // DelayInterrupt
	int sf; // StartingFrame
	uint32_t bp0; // BufferPage0
	uint32_t nexttd; // NextTD
	uint32_t be; // BufferEnd
	uint32_t offset[8]; // Offset/PacketStatusWord
	uint32_t word0;
	uint32_t word1;
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

enum HcControlBits
{
	CBSR = 3 << 0, // ControlBulkServiceRatio
	PLE = 1 << 2, // PeriodicListEnable
	IE = 1 << 3, // IsochronousEnable
	CLE = 1 << 4, // ControlListEnable
	BLE = 1 << 5, // BulkListEnable
	HCFS = 3 << 6, // HostControllerFunctionalState
	IR = 1 << 8, // InterruptRouting
	RWC = 1 << 9, // RemoteWakeupConnected
	RWE = 1 << 10 // RemoteWakeupEnable
};

enum HcCommandStatusBits
{
	HCR = 1 << 0, // HostControllerReset
	CLF = 1 << 1, // ControlListFilled
	BLF = 1 << 2, // BulkListFilled
	OCR = 1 << 3, // OwnershipChangeRequest
	SOC = 3 << 16 // SchedulingOverrunCount
};

enum HcInterruptEnableBits
{
	SO = 1 << 0, // SchedulingOverrun
	WDH = 1 << 1, // WritebackDoneHead
	SF = 1 << 2, // StartofFrame
	RD = 1 << 3, // ResumeDetected
	UE = 1 << 4, // UnrecoverableError
	FNO = 1 << 5, // FrameNumberOverflow
	RHSC = 1 << 6, // RootHubStatusChange
	OC = 1 << 30, // OwnershipChange
	MIE = 1 << 31, // MasterInterruptEnable
};


enum HcRhDescriptorABits
{
	NDP = 0xff << 0, // NumberDownstreamPorts
	PSM = 1 << 8, // PowerSwitchingMode
	NPS = 1 << 9, // NoPowerSwitching
	DT = 1 << 10, // DeviceType
	OCPM = 1 << 11, // OverCurrentProtectionMode
	NOCPM = 1 << 12, // NoOverCurrentProtection
	POTPGT = 0xff << 24 // PowerOnToPowerGoodTime
};

enum HcRhDescriptorBBits
{
	DR = 0xffff << 0, // DeviceRemovable
	PPCM = 0xffff << 16 // PortPowerControlMask
};

enum HcRhStatusBits
{
	LPS = 1 << 0, // LocalPowerStatus
	OCI = 1 << 1, // OverCurrentIndicator
	DRWE = 1 << 15, // DeviceRemoteWakeupEnable
	LPSC = 1 << 16, // LocalPowerStatusChange
	OCIC = 1 << 17, // OverCurrentIndicatorChange
	CRWE = 1 << 31, // ClearRemoteWakeupEnable
};

enum HcRhPortStatusBits
{
	CCS = 1 << 0, // CurrentConnectStatus
	PES = 1 << 1, // PortEnableStatus
	PSS = 1 << 2, // PortSuspendStatus
	POCI = 1 << 3, // PortOverCurrentIndicator
	PRS = 1 << 4, // PortResetStatus
	PPS = 1 << 8, // PortPowerStatus
	LSDA = 1 << 9, // LowSpeedDeviceAttached
	CSC = 1 << 16, // ConnectStatusChange
	PESC = 1 << 17, // PortEnableStatusChange
	PSSC = 1 << 18, // PortSuspendStatusChange
	POCIC = 1 << 19, // PortOverCurrentIndicatorChange
	PRSC = 1 << 20 // PortResetStatusChange
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
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
};

struct USBStandardDeviceDescriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
};

struct USBStandardConfigurationDescriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t MaxPower;
};

struct USBStandardInterfaceDescriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
};

struct USBStandardEndpointDescriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
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

enum USBRequestType
{
	StandardType=0,
	ClassType,
	VendorType,
	ReservedType
};

enum USBRequestRecipient
{
	DeviceRecipient=0,
	InterfaceRecipient,
	EndpointRecipient,
	OtherRecipient
};

enum USBDeviceState
{
	DefaultState,
	AddressState,
	ConfiguredState
};

enum USBControlDirection
{
	HostToDevice=0,
	DeviceToHost=1
};

enum USBEndpointType
{
	ControlEndpoint=0,
	IsochronousEndpoint,
	BulkEndpoint,
	InterruptEndpoint
};

struct usb_device_string
{
	uint8_t *position;
	int size;
};

struct usb_device_interfac_alternate
{
	uint8_t *position;
	int size;
	USBStandardInterfaceDescriptor interface_descriptor;
	std::forward_list<USBStandardEndpointDescriptor> endpoint_descriptors;
};

struct usb_device_interfac
{
	uint8_t *position;
	int size;
	std::forward_list<usb_device_interfac_alternate *> alternate_settings;
	int selected_alternate;
};

struct usb_device_configuration
{
	USBStandardConfigurationDescriptor configuration_descriptor;
	uint8_t *position;
	int size;
	std::forward_list<usb_device_interfac *> interfaces;
};

/*
 * OHCI Usb Controller
 */

class ohci_function; // forward declaration

class ohci_usb_controller
{
public:
	ohci_usb_controller();
	~ohci_usb_controller() {}
	void usb_ohci_plug(int port, ohci_function *function);
	void usb_ohci_device_address_changed(int old_address, int new_address);
	void set_cpu(cpu_device *cpu) { m_maincpu = cpu; }
	void set_timer(emu_timer *timer) { ohcist.timer = timer; }
	void set_irq_callbaclk(std::function<void(int state)> callback) { irq_callback = callback; }

	void start();
	void reset();
	void timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);

private:
	void usb_ohci_interrupts();
	void usb_ohci_read_endpoint_descriptor(uint32_t address);
	void usb_ohci_writeback_endpoint_descriptor(uint32_t address);
	void usb_ohci_read_transfer_descriptor(uint32_t address);
	void usb_ohci_writeback_transfer_descriptor(uint32_t address);
	void usb_ohci_read_isochronous_transfer_descriptor(uint32_t address);
	void usb_ohci_writeback_isochronous_transfer_descriptor(uint32_t address);
	std::function<void (int state)> irq_callback;
	cpu_device *m_maincpu;
	struct {
		uint32_t hc_regs[256];
		struct {
			ohci_function *function;
			int address;
			int delay;
		} ports[4 + 1];
		struct
		{
			ohci_function *function;
			int port;
		} address[256];
		emu_timer *timer;
		int state;
		uint32_t framenumber;
		uint32_t nextinterupted;
		uint32_t nextbulked;
		int interruptbulkratio;
		int writebackdonehadcounter;
		address_space *space;
		uint8_t buffer[1024];
		OHCIEndpointDescriptor endpoint_descriptor;
		OHCITransferDescriptor transfer_descriptor;
		OHCIIsochronousTransferDescriptor isochronous_transfer_descriptor;
	} ohcist;
};

/*
 * Base class for usb devices
 */

class ohci_function {
public:
	ohci_function();
	virtual void initialize(running_machine &machine);
	virtual void execute_reset();
	virtual void execute_connect() {};
	virtual void execute_disconnect() {};
	void set_bus_manager(ohci_usb_controller *usb_bus_manager);
	int execute_transfer(int endpoint, int pid, uint8_t *buffer, int size);
protected:
	virtual int handle_nonstandard_request(int endpoint, USBSetupPacket *setup) { return -1; };
	virtual int handle_get_status_request(int endpoint, USBSetupPacket *setup) { return 0; };
	virtual int handle_clear_feature_request(int endpoint, USBSetupPacket *setup) { return 0; };
	virtual int handle_set_feature_request(int endpoint, USBSetupPacket *setup) { return 0; };
	virtual int handle_set_descriptor_request(int endpoint, USBSetupPacket *setup) { return 0; };
	virtual int handle_synch_frame_request(int endpoint, USBSetupPacket *setup) { return 0; };
	virtual void handle_status_stage(int endpoint) { return; };
	virtual int handle_bulk_pid(int endpoint, int pid, uint8_t *buffer, int size) { return 0; };
	virtual int handle_interrupt_pid(int endpoint, int pid, uint8_t *buffer, int size) { return 0; };
	virtual int handle_isochronous_pid(int endpoint, int pid, uint8_t *buffer, int size) { return 0; };

	void add_device_descriptor(const USBStandardDeviceDescriptor &descriptor);
	void add_configuration_descriptor(const USBStandardConfigurationDescriptor &descriptor);
	void add_interface_descriptor(const USBStandardInterfaceDescriptor &descriptor);
	void add_endpoint_descriptor(const USBStandardEndpointDescriptor &descriptor);
	void add_string_descriptor(const uint8_t *descriptor);
	void select_configuration(int index);
	void select_alternate(int interfacei, int index);
	int find_alternate(int interfacei);
	uint8_t *position_device_descriptor(int &size);
	uint8_t *position_configuration_descriptor(int index, int &size);
	uint8_t *position_string_descriptor(int index, int &size);
	ohci_usb_controller *busmanager;
	struct {
		int type;
		int controldirection;
		int controltype;
		int controlrecipient;
		int remain;
		uint8_t *position;
		uint8_t buffer[128];
	} endpoints[256];
	int state;
	bool settingaddress;
	int newaddress;
	int address;
	int configurationvalue;
	uint8_t *descriptors;
	int descriptors_pos;
	bool wantstatuscallback;
	USBStandardDeviceDescriptor device_descriptor;
	std::forward_list<usb_device_configuration *> configurations;
	std::forward_list<usb_device_string *> device_strings;
	usb_device_configuration *latest_configuration;
	usb_device_interfac_alternate *latest_alternate;
	usb_device_configuration *selected_configuration;
};

/*
 * Usb port connector
 */

class ohci_usb_connector : public device_t, public device_slot_interface
{
public:
	template <typename T>
	ohci_usb_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt, bool fixed)
		: ohci_usb_connector(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
	}

	ohci_usb_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~ohci_usb_connector();

	ohci_function *get_device();

protected:
	virtual void device_start() override;
};

DECLARE_DEVICE_TYPE(OHCI_USB_CONNECTOR, ohci_usb_connector)

/*
 * Game controller usb device
 */

DECLARE_DEVICE_TYPE(OHCI_GAME_CONTROLLER, ohci_game_controller_device)

class ohci_game_controller_device : public device_t, public ohci_function, public device_slot_card_interface
{
public:
	ohci_game_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void initialize(running_machine &machine) override;
	int handle_nonstandard_request(int endpoint, USBSetupPacket *setup) override;
	int handle_interrupt_pid(int endpoint, int pid, uint8_t *buffer, int size) override;

protected:
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;
private:
	static const USBStandardDeviceDescriptor devdesc;
	static const USBStandardConfigurationDescriptor condesc;
	static const USBStandardInterfaceDescriptor intdesc;
	static const USBStandardEndpointDescriptor enddesc82;
	static const USBStandardEndpointDescriptor enddesc02;
	required_ioport m_ThumbstickLh; // left analog thumbstick horizontal movement
	required_ioport m_ThumbstickLv; // left analog thumbstick vertical movement
	required_ioport m_ThumbstickRh; // right analog thumbstick horizontal movement
	required_ioport m_ThumbstickRv; // right analog thumbstick vertical movement
	required_ioport m_DPad; // pressure sensitive directional pad
	required_ioport m_TriggerL; // analog trigger
	required_ioport m_TriggerR; // analog trigger
	required_ioport m_Buttons; // digital buttons
	required_ioport m_AGreen; // analog button
	required_ioport m_BRed; // analog button
	required_ioport m_XBlue; // analog button
	required_ioport m_YYellow; // analog button
	required_ioport m_Black; // analog button
	required_ioport m_White; // analog button
};

#endif // MAME_INCLUDES_XBOX_USB_H
