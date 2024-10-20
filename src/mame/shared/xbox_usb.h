// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
#ifndef MAME_SHARED_XBOX_USB_H
#define MAME_SHARED_XBOX_USB_H

#pragma once

#include <forward_list>
#include <functional>
#include <memory>


struct OHCIEndpointDescriptor {
	int mps = 0; // MaximumPacketSize
	int f = 0; // Format
	int k = 0; // sKip
	int s = 0; // Speed
	int d = 0; // Direction
	int en = 0; // EndpointNumber
	int fa = 0; // FunctionAddress
	uint32_t tailp = 0; // TDQueueTailPointer
	uint32_t headp = 0; // TDQueueHeadPointer
	uint32_t nexted = 0; // NextED
	int c = 0; // toggleCarry
	int h = 0; // Halted
	uint32_t word0 = 0;
};

struct OHCITransferDescriptor {
	int cc = 0; // ConditionCode
	int ec = 0; // ErrorCount
	int t = 0; // DataToggle
	int di = 0; // DelayInterrupt
	int dp = 0; // Direction/PID
	int r = 0; // bufferRounding
	uint32_t cbp = 0; // CurrentBufferPointer
	uint32_t nexttd = 0; // NextTD
	uint32_t be = 0; // BufferEnd
	uint32_t word0 = 0;
};

struct OHCIIsochronousTransferDescriptor {
	int cc = 0; // ConditionCode
	int fc = 0; // FrameCount
	int di = 0; // DelayInterrupt
	int sf = 0; // StartingFrame
	uint32_t bp0 = 0; // BufferPage0
	uint32_t nexttd = 0; // NextTD
	uint32_t be = 0; // BufferEnd
	uint32_t offset[8]{}; // Offset/PacketStatusWord
	uint32_t word0 = 0;
	uint32_t word1 = 0;
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
	uint8_t bmRequestType = 0;
	uint8_t bRequest = 0;
	uint16_t wValue = 0;
	uint16_t wIndex = 0;
	uint16_t wLength = 0;
};

struct USBStandardDeviceDescriptor {
	uint8_t bLength = 0;
	uint8_t bDescriptorType = 0;
	uint16_t bcdUSB = 0;
	uint8_t bDeviceClass = 0;
	uint8_t bDeviceSubClass = 0;
	uint8_t bDeviceProtocol = 0;
	uint8_t bMaxPacketSize0 = 0;
	uint16_t idVendor = 0;
	uint16_t idProduct = 0;
	uint16_t bcdDevice = 0;
	uint8_t iManufacturer = 0;
	uint8_t iProduct = 0;
	uint8_t iSerialNumber = 0;
	uint8_t bNumConfigurations = 0;
};

struct USBStandardConfigurationDescriptor {
	uint8_t bLength = 0;
	uint8_t bDescriptorType = 0;
	uint16_t wTotalLength = 0;
	uint8_t bNumInterfaces = 0;
	uint8_t bConfigurationValue = 0;
	uint8_t iConfiguration = 0;
	uint8_t bmAttributes = 0;
	uint8_t MaxPower = 0;
};

struct USBStandardInterfaceDescriptor {
	uint8_t bLength = 0;
	uint8_t bDescriptorType = 0;
	uint8_t bInterfaceNumber = 0;
	uint8_t bAlternateSetting = 0;
	uint8_t bNumEndpoints = 0;
	uint8_t bInterfaceClass = 0;
	uint8_t bInterfaceSubClass = 0;
	uint8_t bInterfaceProtocol = 0;
	uint8_t iInterface = 0;
};

struct USBStandardEndpointDescriptor {
	uint8_t bLength = 0;
	uint8_t bDescriptorType = 0;
	uint8_t bEndpointAddress = 0;
	uint8_t bmAttributes = 0;
	uint16_t wMaxPacketSize = 0;
	uint8_t bInterval = 0;
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
	uint8_t *position = nullptr;
	int size = 0;
};

struct usb_device_interfac_alternate
{
	uint8_t *position = nullptr;
	int size = 0;
	USBStandardInterfaceDescriptor interface_descriptor;
	std::forward_list<USBStandardEndpointDescriptor> endpoint_descriptors;
};

struct usb_device_interfac
{
	uint8_t *position = nullptr;
	int size = 0;
	std::forward_list<usb_device_interfac_alternate> alternate_settings;
	int selected_alternate;
};

struct usb_device_configuration
{
	USBStandardConfigurationDescriptor configuration_descriptor;
	uint8_t *position = nullptr;
	int size = 0;
	std::forward_list<usb_device_interfac> interfaces;
};

/*
 * OHCI Usb Controller
 */

class device_usb_ohci_function_interface; // forward declaration

class ohci_usb_controller
{
public:
	ohci_usb_controller();
	~ohci_usb_controller() {}
	void usb_ohci_plug(int port, device_usb_ohci_function_interface *function);
	void usb_ohci_device_address_changed(int old_address, int new_address);
	void set_cpu(cpu_device *cpu) { m_maincpu = cpu; }
	void set_timer(emu_timer *timer) { ohcist.timer = timer; }
	void set_irq_callback(std::function<void(int state)> callback) { irq_callback = callback; }

	void start();
	void reset();
	void timer(s32 param);

	uint32_t read(offs_t offset);
	void write(offs_t offset, uint32_t data);

private:
	void usb_ohci_interrupts();
	void usb_ohci_read_endpoint_descriptor(uint32_t address);
	void usb_ohci_writeback_endpoint_descriptor(uint32_t address);
	void usb_ohci_read_transfer_descriptor(uint32_t address);
	void usb_ohci_writeback_transfer_descriptor(uint32_t address);
	void usb_ohci_read_isochronous_transfer_descriptor(uint32_t address);
	void usb_ohci_writeback_isochronous_transfer_descriptor(uint32_t address);
	std::function<void (int state)> irq_callback;
	cpu_device *m_maincpu = nullptr;
	struct {
		uint32_t hc_regs[256]{};
		struct {
			device_usb_ohci_function_interface *function = nullptr;
			int address = 0;
			int delay = 0;
		} ports[4 + 1];
		struct
		{
			device_usb_ohci_function_interface *function = nullptr;
			int port = 0;
		} address[256];
		emu_timer *timer = nullptr;
		int state = 0;
		uint32_t framenumber = 0;
		int interruptbulkratio = 0;
		int writebackdonehadcounter = 0;
		address_space *space = nullptr;
		uint8_t buffer[1024]{};
		OHCIEndpointDescriptor endpoint_descriptor;
		OHCITransferDescriptor transfer_descriptor;
		OHCIIsochronousTransferDescriptor isochronous_transfer_descriptor;
	} ohcist;
};

/*
 * Base class for usb devices
 */

class device_usb_ohci_function_interface : public device_interface {
public:
	virtual void initialize();
	virtual void execute_reset();
	virtual void execute_connect() {}
	virtual void execute_disconnect() {}
	void set_bus_manager(ohci_usb_controller *usb_bus_manager);
	int execute_transfer(int endpoint, int pid, uint8_t *buffer, int size);
protected:
	device_usb_ohci_function_interface(const machine_config &config, device_t &device);
	virtual int handle_nonstandard_request(int endpoint, USBSetupPacket *setup) { return -1; }
	virtual int handle_get_status_request(int endpoint, USBSetupPacket *setup) { return 0; }
	virtual int handle_clear_feature_request(int endpoint, USBSetupPacket *setup) { return 0; }
	virtual int handle_set_feature_request(int endpoint, USBSetupPacket *setup) { return 0; }
	virtual int handle_set_descriptor_request(int endpoint, USBSetupPacket *setup) { return 0; }
	virtual int handle_synch_frame_request(int endpoint, USBSetupPacket *setup) { return 0; }
	virtual void handle_status_stage(int endpoint) { return; }
	virtual int handle_bulk_pid(int endpoint, int pid, uint8_t *buffer, int size) { return 0; }
	virtual int handle_interrupt_pid(int endpoint, int pid, uint8_t *buffer, int size) { return 0; }
	virtual int handle_isochronous_pid(int endpoint, int pid, uint8_t *buffer, int size) { return 0; }

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
	ohci_usb_controller *busmanager = nullptr;
	struct {
		int type = 0;
		int controldirection = 0;
		int controltype = 0;
		int controlrecipient = 0;
		int remain = 0;
		uint8_t *position = nullptr;
		uint8_t buffer[128]{};
	} endpoints[256];
	int state = 0;
	bool settingaddress = false;
	int newaddress = 0;
	int address = 0;
	int configurationvalue = 0;
	std::unique_ptr<uint8_t []> descriptors;
	int descriptors_pos = 0;
	bool wantstatuscallback = false;
	USBStandardDeviceDescriptor device_descriptor;
	std::forward_list<usb_device_configuration> configurations;
	std::forward_list<usb_device_string> device_strings;
	usb_device_configuration *latest_configuration = nullptr;
	usb_device_interfac_alternate *latest_alternate = nullptr;
	usb_device_configuration *selected_configuration = nullptr;
};

/*
 * Usb port connector
 */

class ohci_usb_connector : public device_t, public device_single_card_slot_interface<device_usb_ohci_function_interface>
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

protected:
	virtual void device_start() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(OHCI_USB_CONNECTOR, ohci_usb_connector)

/*
 * Game controller usb device
 */

DECLARE_DEVICE_TYPE(OHCI_GAME_CONTROLLER, ohci_game_controller_device)

class ohci_game_controller_device : public device_t, public device_usb_ohci_function_interface
{
public:
	ohci_game_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void initialize() override;
	int handle_nonstandard_request(int endpoint, USBSetupPacket *setup) override;
	int handle_interrupt_pid(int endpoint, int pid, uint8_t *buffer, int size) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
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

#endif // MAME_SHARED_XBOX_USB_H
