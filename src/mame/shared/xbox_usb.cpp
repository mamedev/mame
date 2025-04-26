// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli

#include "emu.h"
#include "xbox_pci.h"
#include "xbox_usb.h"
#include "xbox.h"

#include "machine/pci.h"
#include "machine/idectrl.h"

#include "multibyte.h"

#define LOG_OHCI (0)


/*
 * OHCI usb controller
 */

static const char *const usbregnames[] = {
	"HcRevision",
	"HcControl",
	"HcCommandStatus",
	"HcInterruptStatus",
	"HcInterruptEnable",
	"HcInterruptDisable",
	"HcHCCA",
	"HcPeriodCurrentED",
	"HcControlHeadED",
	"HcControlCurrentED",
	"HcBulkHeadED",
	"HcBulkCurrentED",
	"HcDoneHead",
	"HcFmInterval",
	"HcFmRemaining",
	"HcFmNumber",
	"HcPeriodicStart",
	"HcLSThreshold",
	"HcRhDescriptorA",
	"HcRhDescriptorB",
	"HcRhStatus",
	"HcRhPortStatus[1]"
};

ohci_usb_controller::ohci_usb_controller()
{
	m_maincpu = nullptr;
}

void ohci_usb_controller::start()
{
	ohcist.hc_regs[HcRevision] = 0x10;
	ohcist.hc_regs[HcFmInterval] = 0x2edf;
	ohcist.hc_regs[HcLSThreshold] = 0x628;
	ohcist.hc_regs[HcRhDescriptorA] = 4;
	ohcist.hc_regs[HcControl] = UsbReset << 6;
	ohcist.state = UsbReset;
	ohcist.interruptbulkratio = 1;
	ohcist.writebackdonehadcounter = 7;
	for (int n = 0; n <= 4; n++)
		ohcist.ports[n].address = -1;
	for (int n = 0; n < 256; n++)
		ohcist.address[n].port = -1;
	ohcist.space = &(m_maincpu->space());
	ohcist.timer->enable(false);
}

void ohci_usb_controller::reset()
{
}

uint32_t ohci_usb_controller::read(offs_t offset)
{
	uint32_t ret;

	if (LOG_OHCI)
	{
		if (offset >= 0x54 / 4)
			m_maincpu->machine().logerror("usb controller 0 register HcRhPortStatus[%d] read\n", (offset - 0x54 / 4) + 1);
		else
			m_maincpu->machine().logerror("usb controller 0 register %s read\n", usbregnames[offset]);
	}
	ret = ohcist.hc_regs[offset];
	return ret;
}

void ohci_usb_controller::write(offs_t offset, uint32_t data)
{
	uint32_t old = ohcist.hc_regs[offset];

	if (LOG_OHCI)
	{
		if (offset >= 0x54 / 4)
			m_maincpu->machine().logerror("usb controller 0 register HcRhPortStatus[%d] write %08X\n", (offset - 0x54 / 4) + 1, data);
		else
			m_maincpu->machine().logerror("usb controller 0 register %s write %08X\n", usbregnames[offset], data);
	}
	if (offset == HcRhStatus) {
		if (data & CRWE)
			ohcist.hc_regs[HcRhStatus] &= ~DRWE;
		if (data & OCIC)
			ohcist.hc_regs[HcRhStatus] &= ~OCI;
		if (data & LPSC)
			ohcist.hc_regs[HcRhStatus] &= ~LPS;
		return;
	}
	if (offset == HcControl) {
		int hcfs;

		hcfs = (data >> 6) & 3; // HostControllerFunctionalState
		if (hcfs == UsbOperational) {
			ohcist.timer->enable();
			ohcist.timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
			ohcist.writebackdonehadcounter = 7;
			// need to load the FrameRemaining field of HcFmRemaining with the value of the FrameInterval field in HcFmInterval
		}
		else
			ohcist.timer->enable(false);
		ohcist.interruptbulkratio = (data & 3) + 1;
		if ((hcfs != UsbReset) && (ohcist.state == UsbReset))
		{
			ohcist.hc_regs[HcInterruptStatus] |= RootHubStatusChange;
			usb_ohci_interrupts();
		}
		ohcist.state = hcfs;
	}
	if (offset == HcCommandStatus) {
		ohcist.hc_regs[HcCommandStatus] |= data;
		if (data & 1) // HostControllerReset
		{
			ohcist.hc_regs[HcControl] |= 3 << 6;
			ohcist.hc_regs[HcCommandStatus] &= ~1;
		}
		return;
	}
	if (offset == HcInterruptStatus) {
		ohcist.hc_regs[HcInterruptStatus] &= ~data;
		usb_ohci_interrupts();
		return;
	}
	if (offset == HcInterruptEnable) {
		ohcist.hc_regs[HcInterruptEnable] |= data;
		usb_ohci_interrupts();
		return;
	}
	if (offset == HcInterruptDisable) {
		ohcist.hc_regs[HcInterruptEnable] &= ~data;
		usb_ohci_interrupts();
		return;
	}
	if (offset >= HcRhPortStatus1) {
		int port = offset - HcRhPortStatus1 + 1; // port 0 not used
													// bit 0  R:CurrentConnectStatus           W:ClearPortEnable: 1 clears PortEnableStatus
		if (data & CCS) {
			ohcist.hc_regs[offset] &= ~PES;
			ohcist.address[ohcist.ports[port].address].port = -1;
		}
		// bit 1  R:PortEnableStatus               W:SetPortEnable: 1 sets PortEnableStatus
		if (data & PES) {
			ohcist.hc_regs[offset] |= PES;
			// the port is enabled, so the device connected to it can communicate on the bus
			ohcist.address[ohcist.ports[port].address].function = ohcist.ports[port].function;
			ohcist.address[ohcist.ports[port].address].port = port;
		}
		// bit 2  R:PortSuspendStatus              W:SetPortSuspend: 1 sets PortSuspendStatus
		if (data & PSS) {
			ohcist.hc_regs[offset] |= PSS;
		}
		// bit 3  R:PortOverCurrentIndicator       W:ClearSuspendStatus: 1 clears PortSuspendStatus
		if (data & POCI) {
			ohcist.hc_regs[offset] &= ~PSS;
		}
		// bit 4  R: PortResetStatus               W:SetPortReset: 1 sets PortResetStatus
		if (data & PRS) {
			ohcist.hc_regs[offset] |= PRS;
			if (ohcist.ports[port].address >= 0)
				ohcist.address[ohcist.ports[port].address].port = -1;
			ohcist.ports[port].address = 0;
			if (ohcist.hc_regs[offset] & PES)
			{
				ohcist.address[0].function = ohcist.ports[port].function;
				ohcist.address[0].port = port;
			}
			ohcist.ports[port].function->execute_reset();
			// after 10ms set PortResetStatusChange and clear PortResetStatus and set PortEnableStatus
			ohcist.ports[port].delay = 10;
		}
		// bit 8  R:PortPowerStatus                W:SetPortPower: 1 sets PortPowerStatus
		if (data & PPS) {
			ohcist.hc_regs[offset] |= PPS;
		}
		// bit 9  R:LowSpeedDeviceAttached         W:ClearPortPower: 1 clears PortPowerStatus
		if (data & LSDA) {
			ohcist.hc_regs[offset] &= ~PPS;
		}
		// bit 16 R:ConnectStatusChange            W: 1 clears ConnectStatusChange
		if (data & CSC) {
			ohcist.hc_regs[offset] &= ~CSC;
		}
		// bit 17 R:PortEnableStatusChange         W: 1 clears PortEnableStatusChange
		if (data & PESC) {
			ohcist.hc_regs[offset] &= ~PESC;
		}
		// bit 18 R:PortSuspendStatusChange        W: 1 clears PortSuspendStatusChange
		if (data & PSSC) {
			ohcist.hc_regs[offset] &= ~PSSC;
		}
		// bit 19 R:PortOverCurrentIndicatorChange W: 1 clears PortOverCurrentIndicatorChange
		if (data & POCIC) {
			ohcist.hc_regs[offset] &= ~POCIC;
		}
		// bit 20 R:PortResetStatusChange          W: 1 clears PortResetStatusChange
		if (data & PRSC) {
			ohcist.hc_regs[offset] &= ~PRSC;
		}
		if (ohcist.hc_regs[offset] != old)
			ohcist.hc_regs[HcInterruptStatus] |= RootHubStatusChange;
		usb_ohci_interrupts();
		return;
	}
	ohcist.hc_regs[offset] = data;
}

void ohci_usb_controller::timer(s32 param)
{
	uint32_t plh;
	int changed = 0;
	int list = 1;
	bool cont = false;
	bool retire = false;
	int pid, remain, mps, done;

	uint32_t hcca = ohcist.hc_regs[HcHCCA];
	if (ohcist.state == UsbOperational) {
		// increment frame number
		ohcist.framenumber = (ohcist.framenumber + 1) & 0xffff;
		if (hcca)
			ohcist.space->write_dword(hcca + 0x80, ohcist.framenumber);
		ohcist.hc_regs[HcFmNumber] = ohcist.framenumber;
	}
	// port reset delay
	for (int p = 1; p <= 4; p++) {
		if (ohcist.ports[p].delay > 0) {
			ohcist.ports[p].delay--;
			if (ohcist.ports[p].delay == 0) {
				ohcist.hc_regs[HcRhPortStatus1 + p - 1] = (ohcist.hc_regs[HcRhPortStatus1 + p - 1] & ~PRS) | PRSC | PES;
				ohcist.address[ohcist.ports[p].address].function = ohcist.ports[p].function;
				ohcist.address[ohcist.ports[p].address].port = p;
				changed = 1;
			}
		}
	}
	if (ohcist.state == UsbOperational) {
		while (list >= 0)
		{
			// select list, do transfer
			if (list == 0) {
				if (ohcist.hc_regs[HcControl] & PLE) {
					// periodic list
					plh = ohcist.space->read_dword(hcca + (ohcist.framenumber & 0x1f) * 4);
					cont = true;
					while (cont == true) {
						if (plh != 0) {
							usb_ohci_read_endpoint_descriptor(plh);
							// if this an isochronous endpoint and isochronous list not enabled, stop list processing
							if (((ohcist.hc_regs[HcControl] & IE) == 0) && (ohcist.endpoint_descriptor.f == 1))
								cont = false;
						}
						else
							cont = false;
						if (cont == false)
							break;
						// service endpoint descriptor
						// only if it is not halted and not to be skipped
						if (!(ohcist.endpoint_descriptor.h | ohcist.endpoint_descriptor.k)) {
							// compare the Endpoint Descriptor TailPointer and NextTransferDescriptor fields.
							if (ohcist.endpoint_descriptor.headp != ohcist.endpoint_descriptor.tailp) {
								uint32_t a, b;
								int R = 0;

								// service transfer descriptor
								if (ohcist.endpoint_descriptor.f != 1) {
									usb_ohci_read_transfer_descriptor(ohcist.endpoint_descriptor.headp);
									// get pid
									if (ohcist.endpoint_descriptor.d == 1)
										pid = OutPid; // out
									else if (ohcist.endpoint_descriptor.d == 2)
										pid = InPid; // in
									else {
										pid = ohcist.transfer_descriptor.dp; // 0 setup 1 out 2 in
									}
									a = ohcist.transfer_descriptor.be;
									b = ohcist.transfer_descriptor.cbp;
								}
								else {
									usb_ohci_read_isochronous_transfer_descriptor(ohcist.endpoint_descriptor.headp);
									// get pid
									if (ohcist.endpoint_descriptor.d == 1)
										pid = OutPid; // out
									else if (ohcist.endpoint_descriptor.d == 2)
										pid = InPid; // in
									else
										pid = InPid; // in
									R = (int)ohcist.framenumber - (int)ohcist.isochronous_transfer_descriptor.sf;
									//if ((R < 0) || (R > (int)ohcist.isochronous_transfer_descriptor.fc))
									//  ; // greater than fc should be an error
									if (R == (int)ohcist.isochronous_transfer_descriptor.fc)
										a = ohcist.isochronous_transfer_descriptor.be;
									else {
										a = ohcist.isochronous_transfer_descriptor.offset[R + 1] - 1;
										if (a & (1 << 12))
											a = (ohcist.isochronous_transfer_descriptor.be & 0xfffff000) | (a & 0xfff);
										else
											a = ohcist.isochronous_transfer_descriptor.bp0 | (a & 0xfff);
									}
									b = ohcist.isochronous_transfer_descriptor.offset[R];
									if (b & (1 << 12))
										b = (ohcist.isochronous_transfer_descriptor.be & 0xfffff000) | (b & 0xfff);
									else
										b = ohcist.isochronous_transfer_descriptor.bp0 | (b & 0xfff);
								}
								if ((a ^ b) & 0xfffff000)
									remain = ((a | 0x1000) & 0x1fff) - (b & 0xfff) + 1;
								else
									remain = a - b + 1;
								mps = ohcist.endpoint_descriptor.mps;
								if (remain < mps)
									mps = remain;
								// if sending ...
								if (pid != InPid) {
									// ... get mps bytes
									for (int c = 0; c < remain; c++) {
										ohcist.buffer[c] = ohcist.space->read_byte(b);
										b++;
										if ((b & 0xfff) == 0)
											b = ohcist.transfer_descriptor.be & 0xfffff000;
									}
								}
								// should check for time available
								// execute transaction
								done = ohcist.address[ohcist.endpoint_descriptor.fa].function->execute_transfer(ohcist.endpoint_descriptor.en, pid, ohcist.buffer, mps);
								// if receiving ...
								if (pid == InPid) {
									// ... store done bytes
									for (int c = 0; c < done; c++) {
										ohcist.space->write_byte(b, ohcist.buffer[c]);
										b++;
										if ((b & 0xfff) == 0)
											b = a & 0xfffff000;
									}
								}
								if (ohcist.endpoint_descriptor.f != 1) {
									// status writeback (CompletionCode field, DataToggleControl field, CurrentBufferPointer field, ErrorCount field)
									ohcist.transfer_descriptor.cc = NoError;
									ohcist.transfer_descriptor.t = (ohcist.transfer_descriptor.t ^ 1) | 2;
									// if all data is transferred (or there was no data to transfer) cbp must be 0, otherwise it must be updated
									if (done == remain)
										b = 0;
									ohcist.transfer_descriptor.cbp = b;
									ohcist.transfer_descriptor.ec = 0;
									retire = false;
									if ((done == mps) && (done == remain)) {
										retire = true;
									}
									if ((done != mps) && (done <= remain))
										retire = true;
									if (done == 0)
										retire = true;
									if (retire == true) {
										// retire transfer descriptor
										a = ohcist.endpoint_descriptor.headp;
										ohcist.endpoint_descriptor.headp = ohcist.transfer_descriptor.nexttd;
										ohcist.transfer_descriptor.nexttd = ohcist.hc_regs[HcDoneHead];
										ohcist.hc_regs[HcDoneHead] = a;
										ohcist.endpoint_descriptor.c = ohcist.transfer_descriptor.t & 1;
										if (ohcist.transfer_descriptor.di != 7) {
											if (ohcist.transfer_descriptor.di < ohcist.writebackdonehadcounter)
												ohcist.writebackdonehadcounter = ohcist.transfer_descriptor.di;
										}
										usb_ohci_writeback_transfer_descriptor(a);
										usb_ohci_writeback_endpoint_descriptor(plh);
									}
									else {
										usb_ohci_writeback_transfer_descriptor(ohcist.endpoint_descriptor.headp);
									}
								}
								else
								{
									// status writeback
									ohcist.isochronous_transfer_descriptor.cc = NoError;
									if (done == remain)
										b = 0;
									ohcist.isochronous_transfer_descriptor.offset[R] = b;
									retire = false;
									if ((done == mps) && (done == remain)) {
										retire = true;
									}
									if ((done != mps) && (done <= remain))
										retire = true;
									if (done == 0)
										retire = true;
									if (retire == true) {
										// retire transfer descriptor
									}
									else {
										usb_ohci_writeback_isochronous_transfer_descriptor(ohcist.endpoint_descriptor.headp);
									}
								}
							}
						}
						// go to next endpoint
						if (ohcist.endpoint_descriptor.nexted != 0)
						{
							plh = ohcist.endpoint_descriptor.nexted;
						}
						else
							cont = false;
					}
				}
				list = -1;
			}
			if (list == 1) {
				// control list
				// check if control list active
				if (ohcist.hc_regs[HcControl] & CLE) {
					cont = true;
					while (cont == true) {
						// if current endpoint descriptor is not 0 use it, otherwise ...
						if (ohcist.hc_regs[HcControlCurrentED] == 0) {
							// ... check the filled bit ...
							if (ohcist.hc_regs[HcCommandStatus] & CLF) {
								// ... if 1 start processing from the head of the list
								ohcist.hc_regs[HcControlCurrentED] = ohcist.hc_regs[HcControlHeadED];
								// clear CLF (ControlListFilled)
								ohcist.hc_regs[HcCommandStatus] &= ~CLF;
								// but if the list is empty, go to the next list
								if (ohcist.hc_regs[HcControlCurrentED] == 0)
									cont = false;
								else
									cont = true;
							}
							else
								cont = false;
						}
						else
							cont = true;
						if (cont == false)
							break;
						// service endpoint descriptor
						usb_ohci_read_endpoint_descriptor(ohcist.hc_regs[HcControlCurrentED]);
						// only if it is not halted and not to be skipped
						if (!(ohcist.endpoint_descriptor.h | ohcist.endpoint_descriptor.k)) {
							// compare the Endpoint Descriptor TailPointer and NextTransferDescriptor fields.
							if (ohcist.endpoint_descriptor.headp != ohcist.endpoint_descriptor.tailp) {
								uint32_t a, b;
								// set CLF (ControlListFilled)
								ohcist.hc_regs[HcCommandStatus] |= CLF;
								// service transfer descriptor
								usb_ohci_read_transfer_descriptor(ohcist.endpoint_descriptor.headp);
								// get pid
								if (ohcist.endpoint_descriptor.d == 1)
									pid = OutPid; // out
								else if (ohcist.endpoint_descriptor.d == 2)
									pid = InPid; // in
								else {
									pid = ohcist.transfer_descriptor.dp; // 0 setup 1 out 2 in
								}
								// determine how much data to transfer
								// setup pid must be 8 bytes
								a = ohcist.transfer_descriptor.be & 0xfff;
								b = ohcist.transfer_descriptor.cbp & 0xfff;
								if ((ohcist.transfer_descriptor.be ^ ohcist.transfer_descriptor.cbp) & 0xfffff000)
									a |= 0x1000;
								remain = a - b + 1;
								mps = ohcist.endpoint_descriptor.mps;
								if ((pid == InPid) || (pid == OutPid)) {
									if (remain < mps)
										mps = remain;
								}
								if (ohcist.transfer_descriptor.cbp == 0) {
									remain = 0;
									mps = 0;
								}
								b = ohcist.transfer_descriptor.cbp;
								// if sending ...
								if (pid != InPid) {
									// ... get mps bytes
									for (int c = 0; c < remain; c++) {
										ohcist.buffer[c] = ohcist.space->read_byte(b);
										b++;
										if ((b & 0xfff) == 0)
											b = ohcist.transfer_descriptor.be & 0xfffff000;
									}
								}
								// should check for time available
								// execute transaction
								done = ohcist.address[ohcist.endpoint_descriptor.fa].function->execute_transfer(ohcist.endpoint_descriptor.en, pid, ohcist.buffer, mps);
								// if receiving ...
								if (pid == InPid) {
									// ... store done bytes
									for (int c = 0; c < done; c++) {
										ohcist.space->write_byte(b, ohcist.buffer[c]);
										b++;
										if ((b & 0xfff) == 0)
											b = ohcist.transfer_descriptor.be & 0xfffff000;
									}
								}
								// status writeback (CompletionCode field, DataToggleControl field, CurrentBufferPointer field, ErrorCount field)
								ohcist.transfer_descriptor.cc = NoError;
								ohcist.transfer_descriptor.t = (ohcist.transfer_descriptor.t ^ 1) | 2;
								// if all data is transferred (or there was no data to transfer) cbp must be 0, otherwise it must be updated
								if ((done == remain) || (pid == SetupPid))
									b = 0;
								ohcist.transfer_descriptor.cbp = b;
								ohcist.transfer_descriptor.ec = 0;
								retire = false;
								if ((done == mps) && (done == remain)) {
									retire = true;
								}
								if ((done != mps) && (done <= remain))
									retire = true;
								if (done == 0)
									retire = true;
								if (retire == true) {
									// retire transfer descriptor
									a = ohcist.endpoint_descriptor.headp;
									ohcist.endpoint_descriptor.headp = ohcist.transfer_descriptor.nexttd;
									ohcist.transfer_descriptor.nexttd = ohcist.hc_regs[HcDoneHead];
									ohcist.hc_regs[HcDoneHead] = a;
									ohcist.endpoint_descriptor.c = ohcist.transfer_descriptor.t & 1;
									if (ohcist.transfer_descriptor.di != 7) {
										if (ohcist.transfer_descriptor.di < ohcist.writebackdonehadcounter)
											ohcist.writebackdonehadcounter = ohcist.transfer_descriptor.di;
									}
									usb_ohci_writeback_transfer_descriptor(a);
									usb_ohci_writeback_endpoint_descriptor(ohcist.hc_regs[HcControlCurrentED]);
								}
								else {
									usb_ohci_writeback_transfer_descriptor(ohcist.endpoint_descriptor.headp);
								}
							}
							else {
								// no transfer descriptors for this endpoint, so go to next endpoint
								ohcist.hc_regs[HcControlCurrentED] = ohcist.endpoint_descriptor.nexted;
							}
						}
						else {
							// not enabled, so go to next endpoint
							ohcist.hc_regs[HcControlCurrentED] = ohcist.endpoint_descriptor.nexted;
						}
						// one bulk every n control transfers
						ohcist.interruptbulkratio--;
						if (ohcist.interruptbulkratio <= 0) {
							ohcist.interruptbulkratio = (ohcist.hc_regs[HcControl] & 3) + 1; // ControlBulkServiceRatio
							cont = false;
						}
					}
				}
				list = 2;
			}
			if (list == 2) {
				// bulk list
				// check if bulk list active
				if (ohcist.hc_regs[HcControl] & BLE) {
					// if current endpoint descriptor is not 0 use it, otherwise ...
					if (ohcist.hc_regs[HcBulkCurrentED] == 0) {
						// ... check the filled bit ...
						if (ohcist.hc_regs[HcCommandStatus] & BLF) {
							// ... if 1 start processing from the head of the list
							ohcist.hc_regs[HcBulkCurrentED] = ohcist.hc_regs[HcBulkHeadED];
							// clear BLF (BulkListFilled)
							ohcist.hc_regs[HcCommandStatus] &= ~BLF;
							// but if the list is empty, go to the next list
							if (ohcist.hc_regs[HcBulkCurrentED] == 0)
								cont = false;
							else
								cont = true;
						}
						else
							cont = false;
					}
					else
						cont = true;
					if (cont == true) {
						// service endpoint descriptor
						usb_ohci_read_endpoint_descriptor(ohcist.hc_regs[HcBulkCurrentED]);
						// only if it is not halted and not to be skipped
						if (!(ohcist.endpoint_descriptor.h | ohcist.endpoint_descriptor.k)) {
							// compare the Endpoint Descriptor TailPointer and NextTransferDescriptor fields.
							if (ohcist.endpoint_descriptor.headp != ohcist.endpoint_descriptor.tailp) {
								uint32_t a, b;
								// set BLF (BulkListFilled)
								ohcist.hc_regs[HcCommandStatus] |= BLF;
								// service transfer descriptor
								usb_ohci_read_transfer_descriptor(ohcist.endpoint_descriptor.headp);
								// get pid
								if (ohcist.endpoint_descriptor.d == 1)
									pid = OutPid; // out
								else if (ohcist.endpoint_descriptor.d == 2)
									pid = InPid; // in
								else {
									pid = ohcist.transfer_descriptor.dp; // 0 setup 1 out 2 in
								}
								// determine how much data to transfer
								a = ohcist.transfer_descriptor.be & 0xfff;
								b = ohcist.transfer_descriptor.cbp & 0xfff;
								if ((ohcist.transfer_descriptor.be ^ ohcist.transfer_descriptor.cbp) & 0xfffff000)
									a |= 0x1000;
								remain = a - b + 1;
								mps = ohcist.endpoint_descriptor.mps;
								if (remain < mps)
									mps = remain;
								b = ohcist.transfer_descriptor.cbp;
								// if sending ...
								if (pid != InPid) {
									// ... get mps bytes
									for (int c = 0; c < remain; c++) {
										ohcist.buffer[c] = ohcist.space->read_byte(b);
										b++;
										if ((b & 0xfff) == 0)
											b = ohcist.transfer_descriptor.be & 0xfffff000;
									}
								}
								// should check for time available
								// execute transaction
								done = ohcist.address[ohcist.endpoint_descriptor.fa].function->execute_transfer(ohcist.endpoint_descriptor.en, pid, ohcist.buffer, mps);
								// if receiving ...
								if (pid == InPid) {
									// ... store done bytes
									for (int c = 0; c < done; c++) {
										ohcist.space->write_byte(b, ohcist.buffer[c]);
										b++;
										if ((b & 0xfff) == 0)
											b = ohcist.transfer_descriptor.be & 0xfffff000;
									}
								}
								// status writeback (CompletionCode field, DataToggleControl field, CurrentBufferPointer field, ErrorCount field)
								ohcist.transfer_descriptor.cc = NoError;
								ohcist.transfer_descriptor.t = (ohcist.transfer_descriptor.t ^ 1) | 2;
								// if all data is transferred (or there was no data to transfer) cbp must be 0, otherwise it must be updated
								if (done == remain)
									b = 0;
								ohcist.transfer_descriptor.cbp = b;
								ohcist.transfer_descriptor.ec = 0;
								retire = false;
								if ((done == mps) && (done == remain)) {
									retire = true;
								}
								if ((done != mps) && (done <= remain))
									retire = true;
								if (done == 0)
									retire = true;
								if (retire == true) {
									// retire transfer descriptor
									a = ohcist.endpoint_descriptor.headp;
									ohcist.endpoint_descriptor.headp = ohcist.transfer_descriptor.nexttd;
									ohcist.transfer_descriptor.nexttd = ohcist.hc_regs[HcDoneHead];
									ohcist.hc_regs[HcDoneHead] = a;
									ohcist.endpoint_descriptor.c = ohcist.transfer_descriptor.t & 1;
									if (ohcist.transfer_descriptor.di != 7) {
										if (ohcist.transfer_descriptor.di < ohcist.writebackdonehadcounter)
											ohcist.writebackdonehadcounter = ohcist.transfer_descriptor.di;
									}
									usb_ohci_writeback_transfer_descriptor(a);
									usb_ohci_writeback_endpoint_descriptor(ohcist.hc_regs[HcBulkCurrentED]);
								}
								else {
									usb_ohci_writeback_transfer_descriptor(ohcist.endpoint_descriptor.headp);
								}
							}
							else {
								// no transfer descriptors for this endpoint, so go to next endpoint
								ohcist.hc_regs[HcBulkCurrentED] = ohcist.endpoint_descriptor.nexted;
							}
						}
						else {
							// not enabled, so go to next endpoint
							ohcist.hc_regs[HcBulkCurrentED] = ohcist.endpoint_descriptor.nexted;
						}
					}
					// go to the next list
					if ((ohcist.hc_regs[HcCommandStatus] & CLF) && (ohcist.hc_regs[HcControl] & CLE))
						list = 1; // go to control list if enabled and filled
					else if ((ohcist.hc_regs[HcCommandStatus] & BLF) && (ohcist.hc_regs[HcControl] & BLE))
						list = 2; // otherwise stay in bulk list if enabled and filled
					else
						list = 0; // if no control or bulk lists, go to periodic list
				}
				else
					list = 0;
			}
		}
		if (ohcist.framenumber == 0)
			ohcist.hc_regs[HcInterruptStatus] |= FrameNumberOverflow;
		ohcist.hc_regs[HcInterruptStatus] |= StartofFrame;
		if ((ohcist.writebackdonehadcounter != 0) && (ohcist.writebackdonehadcounter != 7))
			ohcist.writebackdonehadcounter--;
		if ((ohcist.writebackdonehadcounter == 0) && ((ohcist.hc_regs[HcInterruptStatus] & WritebackDoneHead) == 0)) {
			uint32_t b = 0;

			if ((ohcist.hc_regs[HcInterruptStatus] & ohcist.hc_regs[HcInterruptEnable]) != WritebackDoneHead)
				b = 1;
			ohcist.hc_regs[HcInterruptStatus] |= WritebackDoneHead;
			if (hcca)
				ohcist.space->write_dword(hcca + 0x84, ohcist.hc_regs[HcDoneHead] | b);
			ohcist.hc_regs[HcDoneHead] = 0;
			ohcist.writebackdonehadcounter = 7;
		}
	}
	if (changed != 0) {
		ohcist.hc_regs[HcInterruptStatus] |= RootHubStatusChange;
	}
	usb_ohci_interrupts();
}

void ohci_usb_controller::usb_ohci_plug(int port, device_usb_ohci_function_interface *function)
{
	if ((port > 0) && (port <= 4)) {
		ohcist.ports[port].function = function;
		ohcist.ports[port].address = -1;
		ohcist.hc_regs[HcRhPortStatus1 + port - 1] = CCS | CSC;
		if (ohcist.state != UsbReset)
		{
			ohcist.hc_regs[HcInterruptStatus] |= RootHubStatusChange;
			usb_ohci_interrupts();
		}
	}
}

void ohci_usb_controller::usb_ohci_interrupts()
{
	if (((ohcist.hc_regs[HcInterruptStatus] & ohcist.hc_regs[HcInterruptEnable]) != 0) && ((ohcist.hc_regs[HcInterruptEnable] & MasterInterruptEnable) != 0))
	{
		irq_callback(1);
	} else
	{
		irq_callback(0);
	}
}

void ohci_usb_controller::usb_ohci_read_endpoint_descriptor(uint32_t address)
{
	uint32_t w;

	w = ohcist.space->read_dword(address);
	ohcist.endpoint_descriptor.word0 = w;
	ohcist.endpoint_descriptor.fa = w & 0x7f;
	ohcist.endpoint_descriptor.en = (w >> 7) & 15;
	ohcist.endpoint_descriptor.d = (w >> 11) & 3;
	ohcist.endpoint_descriptor.s = (w >> 13) & 1;
	ohcist.endpoint_descriptor.k = (w >> 14) & 1;
	ohcist.endpoint_descriptor.f = (w >> 15) & 1;
	ohcist.endpoint_descriptor.mps = (w >> 16) & 0x7ff;
	ohcist.endpoint_descriptor.tailp = ohcist.space->read_dword(address + 4);
	w = ohcist.space->read_dword(address + 8);
	ohcist.endpoint_descriptor.headp = w & 0xfffffffc;
	ohcist.endpoint_descriptor.h = w & 1;
	ohcist.endpoint_descriptor.c = (w >> 1) & 1;
	ohcist.endpoint_descriptor.nexted = ohcist.space->read_dword(address + 12);
}

void ohci_usb_controller::usb_ohci_writeback_endpoint_descriptor(uint32_t address)
{
	uint32_t w;

	w = ohcist.endpoint_descriptor.word0 & 0xf8000000;
	w = w | (ohcist.endpoint_descriptor.mps << 16) | (ohcist.endpoint_descriptor.f << 15) | (ohcist.endpoint_descriptor.k << 14) | (ohcist.endpoint_descriptor.s << 13) | (ohcist.endpoint_descriptor.d << 11) | (ohcist.endpoint_descriptor.en << 7) | ohcist.endpoint_descriptor.fa;
	ohcist.space->write_dword(address, w);
	w = ohcist.endpoint_descriptor.headp | (ohcist.endpoint_descriptor.c << 1) | ohcist.endpoint_descriptor.h;
	ohcist.space->write_dword(address + 8, w);
}

void ohci_usb_controller::usb_ohci_read_transfer_descriptor(uint32_t address)
{
	uint32_t w;

	w = ohcist.space->read_dword(address);
	ohcist.transfer_descriptor.word0 = w;
	ohcist.transfer_descriptor.cc = (w >> 28) & 15;
	ohcist.transfer_descriptor.ec = (w >> 26) & 3;
	ohcist.transfer_descriptor.t = (w >> 24) & 3;
	ohcist.transfer_descriptor.di = (w >> 21) & 7;
	ohcist.transfer_descriptor.dp = (w >> 19) & 3;
	ohcist.transfer_descriptor.r = (w >> 18) & 1;
	ohcist.transfer_descriptor.cbp = ohcist.space->read_dword(address + 4);
	ohcist.transfer_descriptor.nexttd = ohcist.space->read_dword(address + 8);
	ohcist.transfer_descriptor.be = ohcist.space->read_dword(address + 12);
}

void ohci_usb_controller::usb_ohci_writeback_transfer_descriptor(uint32_t address)
{
	uint32_t w;

	w = ohcist.transfer_descriptor.word0 & 0x0003ffff;
	w = w | (ohcist.transfer_descriptor.cc << 28) | (ohcist.transfer_descriptor.ec << 26) | (ohcist.transfer_descriptor.t << 24) | (ohcist.transfer_descriptor.di << 21) | (ohcist.transfer_descriptor.dp << 19) | (ohcist.transfer_descriptor.r << 18);
	ohcist.space->write_dword(address, w);
	ohcist.space->write_dword(address + 4, ohcist.transfer_descriptor.cbp);
	ohcist.space->write_dword(address + 8, ohcist.transfer_descriptor.nexttd);
}

void ohci_usb_controller::usb_ohci_read_isochronous_transfer_descriptor(uint32_t address)
{
	uint32_t w;

	w = ohcist.space->read_dword(address);
	ohcist.isochronous_transfer_descriptor.word0 = w;
	ohcist.isochronous_transfer_descriptor.cc = (w >> 28) & 15;
	ohcist.isochronous_transfer_descriptor.fc = (w >> 24) & 7;
	ohcist.isochronous_transfer_descriptor.di = (w >> 21) & 7;
	ohcist.isochronous_transfer_descriptor.sf = w & 0xffff;
	w = ohcist.space->read_dword(address + 4);
	ohcist.isochronous_transfer_descriptor.word1 = w;
	ohcist.isochronous_transfer_descriptor.bp0 = w & 0xfffff000;
	ohcist.isochronous_transfer_descriptor.nexttd = ohcist.space->read_dword(address + 8);
	ohcist.isochronous_transfer_descriptor.be = ohcist.space->read_dword(address + 12);
	w = ohcist.space->read_dword(address + 16);
	ohcist.isochronous_transfer_descriptor.offset[0] = w & 0xffff;
	ohcist.isochronous_transfer_descriptor.offset[1] = (w >> 16) & 0xffff;
	w = ohcist.space->read_dword(address + 20);
	ohcist.isochronous_transfer_descriptor.offset[2] = w & 0xffff;
	ohcist.isochronous_transfer_descriptor.offset[3] = (w >> 16) & 0xffff;
	w = ohcist.space->read_dword(address + 24);
	ohcist.isochronous_transfer_descriptor.offset[4] = w & 0xffff;
	ohcist.isochronous_transfer_descriptor.offset[5] = (w >> 16) & 0xffff;
	w = ohcist.space->read_dword(address + 28);
	ohcist.isochronous_transfer_descriptor.offset[6] = w & 0xffff;
	ohcist.isochronous_transfer_descriptor.offset[7] = (w >> 16) & 0xffff;
}

void ohci_usb_controller::usb_ohci_writeback_isochronous_transfer_descriptor(uint32_t address)
{
	uint32_t w;

	w = ohcist.isochronous_transfer_descriptor.word0 & 0x1f0000;
	w = w | (ohcist.isochronous_transfer_descriptor.cc << 28) | (ohcist.isochronous_transfer_descriptor.fc << 24) | (ohcist.isochronous_transfer_descriptor.di << 21) | ohcist.isochronous_transfer_descriptor.sf;
	ohcist.space->write_dword(address, w);
	w = ohcist.isochronous_transfer_descriptor.word1 & 0xfff;
	w = w | ohcist.isochronous_transfer_descriptor.bp0;
	ohcist.space->write_dword(address + 4, w);
	ohcist.space->write_dword(address + 8, ohcist.isochronous_transfer_descriptor.nexttd);
	ohcist.space->write_dword(address + 12, ohcist.isochronous_transfer_descriptor.be);
	w = (ohcist.isochronous_transfer_descriptor.offset[1] << 16) | ohcist.isochronous_transfer_descriptor.offset[0];
	ohcist.space->write_dword(address + 16, w);
	w = (ohcist.isochronous_transfer_descriptor.offset[3] << 16) | ohcist.isochronous_transfer_descriptor.offset[2];
	ohcist.space->write_dword(address + 20, w);
	w = (ohcist.isochronous_transfer_descriptor.offset[5] << 16) | ohcist.isochronous_transfer_descriptor.offset[4];
	ohcist.space->write_dword(address + 24, w);
	w = (ohcist.isochronous_transfer_descriptor.offset[7] << 16) | ohcist.isochronous_transfer_descriptor.offset[6];
	ohcist.space->write_dword(address + 28, w);
}

void ohci_usb_controller::usb_ohci_device_address_changed(int old_address, int new_address)
{
	ohcist.address[new_address].function = ohcist.address[old_address].function;
	ohcist.address[new_address].port = ohcist.address[old_address].port;
	ohcist.address[old_address].port = -1;
}

/*
 * Base class for usb devices
 */

device_usb_ohci_function_interface::device_usb_ohci_function_interface(machine_config const &mconfig, device_t &device)
	: device_interface(device, "usbohci")
{
}

void device_usb_ohci_function_interface::initialize()
{
	state = DefaultState;
	descriptors = std::make_unique<uint8_t []>(1024);
	descriptors_pos = 0;
	address = 0;
	newaddress = 0;
	for (int e = 0; e < 256;e++) {
		endpoints[e].type = -1;
		endpoints[e].controldirection = 0;
		endpoints[e].controltype = 0;
		endpoints[e].controlrecipient = 0;
		endpoints[e].remain = 0;
		endpoints[e].position = nullptr;
	}
	endpoints[0].type = ControlEndpoint;
	wantstatuscallback = false;
	settingaddress = false;
	configurationvalue = 0;
	selected_configuration = nullptr;
	latest_configuration = nullptr;
	latest_alternate = nullptr;
}

void device_usb_ohci_function_interface::set_bus_manager(ohci_usb_controller *usb_bus_manager)
{
	busmanager = usb_bus_manager;
}


void device_usb_ohci_function_interface::add_device_descriptor(const USBStandardDeviceDescriptor &descriptor)
{
	uint8_t *const p = &descriptors[descriptors_pos];
	p[0] = descriptor.bLength;
	p[1] = descriptor.bDescriptorType;
	put_u16le(&p[2], descriptor.bcdUSB);
	p[4] = descriptor.bDeviceClass;
	p[5] = descriptor.bDeviceSubClass;
	p[6] = descriptor.bDeviceProtocol;
	p[7] = descriptor.bMaxPacketSize0;
	put_u16le(&p[8], descriptor.idVendor);
	put_u16le(&p[10], descriptor.idProduct);
	put_u16le(&p[12], descriptor.bcdDevice);
	p[14] = descriptor.iManufacturer;
	p[15] = descriptor.iProduct;
	p[16] = descriptor.iSerialNumber;
	p[17] = descriptor.bNumConfigurations;
	descriptors_pos += descriptor.bLength;
	memcpy(&device_descriptor, &descriptor, sizeof(USBStandardDeviceDescriptor));
}

void device_usb_ohci_function_interface::add_configuration_descriptor(const USBStandardConfigurationDescriptor &descriptor)
{
	uint8_t *const p = &descriptors[descriptors_pos];
	p[0] = descriptor.bLength;
	p[1] = descriptor.bDescriptorType;
	put_u16le(&p[2], descriptor.wTotalLength);
	p[4] = descriptor.bNumInterfaces;
	p[5] = descriptor.bConfigurationValue;
	p[6] = descriptor.iConfiguration;
	p[7] = descriptor.bmAttributes;
	p[8] = descriptor.MaxPower;
	descriptors_pos += descriptor.bLength;

	configurations.emplace_front();
	usb_device_configuration &c(configurations.front());
	c.position = p;
	c.size = descriptor.bLength;
	memcpy(&c.configuration_descriptor, &descriptor, sizeof(USBStandardConfigurationDescriptor));
	latest_configuration = &c;
	latest_alternate = nullptr;
}

void device_usb_ohci_function_interface::add_interface_descriptor(const USBStandardInterfaceDescriptor &descriptor)
{
	if (latest_configuration == nullptr)
		return;

	uint8_t *const p = &descriptors[descriptors_pos];
	p[0] = descriptor.bLength;
	p[1] = descriptor.bDescriptorType;
	p[2] = descriptor.bInterfaceNumber;
	p[3] = descriptor.bAlternateSetting;
	p[4] = descriptor.bNumEndpoints;
	p[5] = descriptor.bInterfaceClass;
	p[6] = descriptor.bInterfaceSubClass;
	p[7] = descriptor.bInterfaceProtocol;
	p[8] = descriptor.iInterface;
	descriptors_pos += descriptor.bLength;
	latest_configuration->size += descriptor.bLength;

	for (auto &i : latest_configuration->interfaces)
	{
		if (i.alternate_settings.front().interface_descriptor.bInterfaceNumber == descriptor.bInterfaceNumber)
		{
			i.size += descriptor.bLength;
			latest_configuration->interfaces.front().size += descriptor.bLength;

			i.alternate_settings.emplace_front();
			usb_device_interfac_alternate &aa(i.alternate_settings.front());
			memcpy(&aa.interface_descriptor, &descriptor, sizeof(USBStandardInterfaceDescriptor));
			aa.position = p;
			aa.size = descriptor.bLength;
			latest_alternate = &aa;
			return;
		}
	}

	latest_configuration->interfaces.emplace_front();
	usb_device_interfac &ii(latest_configuration->interfaces.front());
	ii.position = p;
	ii.size = descriptor.bLength;
	ii.selected_alternate = -1;

	ii.alternate_settings.emplace_front();
	usb_device_interfac_alternate &aa(ii.alternate_settings.front());
	memcpy(&aa.interface_descriptor, &descriptor, sizeof(USBStandardInterfaceDescriptor));
	aa.position = p;
	aa.size = descriptor.bLength;
	latest_alternate = &aa;
}

void device_usb_ohci_function_interface::add_endpoint_descriptor(const USBStandardEndpointDescriptor &descriptor)
{
	if (latest_alternate == nullptr)
		return;

	uint8_t *const p = &descriptors[descriptors_pos];
	p[0] = descriptor.bLength;
	p[1] = descriptor.bDescriptorType;
	p[2] = descriptor.bEndpointAddress;
	p[3] = descriptor.bmAttributes;
	put_u16le(&p[4], descriptor.wMaxPacketSize);
	p[6] = descriptor.bInterval;
	descriptors_pos += descriptor.bLength;

	latest_alternate->endpoint_descriptors.push_front(descriptor);
	latest_alternate->size += descriptor.bLength;
	latest_configuration->interfaces.front().size += descriptor.bLength;
	latest_configuration->size += descriptor.bLength;
}

void device_usb_ohci_function_interface::add_string_descriptor(const uint8_t *descriptor)
{
	int len = descriptor[0];
	uint8_t *const p = &descriptors[descriptors_pos];
	memcpy(p, descriptor, len);
	descriptors_pos += len;

	device_strings.emplace_front();
	usb_device_string &ss(device_strings.front());
	ss.size = len;
	ss.position = p;
	//latest_configuration->size += len;
}

void device_usb_ohci_function_interface::select_configuration(int index)
{
	configurationvalue = index;
	for (auto &c : configurations)
	{
		if (c.configuration_descriptor.bConfigurationValue == index)
		{
			selected_configuration = &c;
			// by default, activate alternate setting 0 in each interface
			for (auto &i : c.interfaces)
			{
				i.selected_alternate = 0;
				for (auto &a : i.alternate_settings)
				{
					if (a.interface_descriptor.bAlternateSetting == 0)
					{
						// activate the endpoints in interface i alternate setting 0
						for (auto &e : a.endpoint_descriptors)
						{
							endpoints[e.bEndpointAddress].type = e.bmAttributes & 3;
							endpoints[e.bEndpointAddress].remain = 0;
						}
						break;
					}
				}
			}
			break;
		}
	}
}

void device_usb_ohci_function_interface::select_alternate(int interfacei, int index)
{
	// among all the interfaces in the currently selected configuration, consider interface interfacei
	for (auto &i : selected_configuration->interfaces)
	{
		// deactivate the endpoints in the currently selected alternate setting for interface interfacei
		for (auto &a : i.alternate_settings)
		{
			if ((a.interface_descriptor.bInterfaceNumber == interfacei) && (a.interface_descriptor.bAlternateSetting == i.selected_alternate))
			{
				for (auto &e : a.endpoint_descriptors)
				{
					endpoints[e.bEndpointAddress].type = -1;
				}
				break;
			}
		}
		// activate the endpoints in the newly selected alternate setting
		for (auto &a : i.alternate_settings)
		{
			if ((a.interface_descriptor.bInterfaceNumber == interfacei) && (a.interface_descriptor.bAlternateSetting == index))
			{
				i.selected_alternate = index;
				for (auto &e : a.endpoint_descriptors)
				{
					endpoints[e.bEndpointAddress].type = e.bmAttributes & 3;
					endpoints[e.bEndpointAddress].remain = 0;
				}
				break;
			}
		}
	}
}

int device_usb_ohci_function_interface::find_alternate(int interfacei)
{
	// find the active alternate setting for interface inteerfacei
	for (const auto &i : selected_configuration->interfaces)
	{
		for (const auto &a : i.alternate_settings)
		{
			if (a.interface_descriptor.bInterfaceNumber == interfacei)
			{
				return i.selected_alternate;
			}
		}
	}
	return 0;
}

uint8_t *device_usb_ohci_function_interface::position_device_descriptor(int &size)
{
	size = descriptors_pos; // descriptors[0];
	return &descriptors[0];
}

uint8_t *device_usb_ohci_function_interface::position_configuration_descriptor(int index, int &size)
{
	for (const auto &c : configurations)
	{
		if (c.configuration_descriptor.bConfigurationValue == (index + 1))
		{
			size = c.size;
			return c.position;
		}
	}
	size = 0;
	return nullptr;
}

uint8_t *device_usb_ohci_function_interface::position_string_descriptor(int index, int &size)
{
	int i = 0;

	for (const auto &s : device_strings)
	{
		if (index == i)
		{
			size = s.size;
			return s.position;
		}
		i++;
	}
	size = 0;
	return nullptr;
}

void device_usb_ohci_function_interface::execute_reset()
{
	address = 0;
	newaddress = 0;
}

int device_usb_ohci_function_interface::execute_transfer(int endpoint, int pid, uint8_t *buffer, int size)
{
	int descriptortype, descriptorindex;

	if (pid == SetupPid) {
		USBSetupPacket *p=(USBSetupPacket *)buffer;
		// control transfers are done in 3 stages: first the setup stage, then an optional data stage, then a status stage
		// so there are 3 cases:
		// 1- control transfer with a data stage where the host sends data to the device
		//    in this case the sequence of pids transferred is control pid, data out pid, data in pid
		// 2- control transfer with a data stage where the host receives data from the device
		//    in this case the sequence of pids transferred is control pid, data in pid, data out pid
		// 3- control transfer without a data stage
		//    in this case the sequence of pids transferred is control pid, data in pid
		// define direction 0:host->device 1:device->host
		// direction == 1 -> IN data stage and OUT status stage
		// direction == 0 -> OUT data stage and IN status stage
		// data stage not present -> IN status stage
		endpoints[endpoint].controldirection = (p->bmRequestType & 128) >> 7;
		endpoints[endpoint].controltype = (p->bmRequestType & 0x60) >> 5;
		endpoints[endpoint].controlrecipient = p->bmRequestType & 0x1f;
		wantstatuscallback = false;
		if (endpoint == 0) {
			endpoints[endpoint].position = nullptr;
			// number of byte to transfer in data stage (0 no data stage)
			endpoints[endpoint].remain = p->wLength;
			// if standard device request
			if ((endpoints[endpoint].controltype == StandardType) && (endpoints[endpoint].controlrecipient == DeviceRecipient)) {
				switch (p->bRequest) {
				case GET_STATUS:
					return handle_get_status_request(endpoint, p);
					break;
				case CLEAR_FEATURE:
					return handle_clear_feature_request(endpoint, p);
					break;
				case SET_FEATURE:
					return handle_set_feature_request(endpoint, p);
					break;
				case SET_ADDRESS:
					newaddress = p->wValue;
					settingaddress = true;
					break;
				case GET_DESCRIPTOR:
					descriptortype = p->wValue >> 8;
					descriptorindex = p->wValue & 255;
					if (descriptortype == DEVICE) { // device descriptor
						endpoints[endpoint].position = position_device_descriptor(endpoints[endpoint].remain);
					}
					else if (descriptortype == CONFIGURATION) { // configuration descriptor
						endpoints[endpoint].position = position_configuration_descriptor(descriptorindex, endpoints[endpoint].remain);
					}
					else if (descriptortype == STRING) { // string descriptor
						//p->wIndex; language id
						endpoints[endpoint].position = position_string_descriptor(descriptorindex, endpoints[endpoint].remain);
					}
					else
						endpoints[endpoint].remain = 0;
					if (endpoints[endpoint].remain > p->wLength)
						endpoints[endpoint].remain = p->wLength;
					break;
				case SET_CONFIGURATION:
					if (p->wValue == 0)
						state = AddressState;
					else {
						select_configuration(p->wValue);
						state = ConfiguredState;
					}
					break;
				case SET_INTERFACE:
					select_alternate(p->wIndex, p->wValue);
					break;
				case SET_DESCRIPTOR:
					return handle_set_descriptor_request(endpoint, p);
					break;
				case GET_CONFIGURATION:
					endpoints[endpoint].buffer[0] = (uint8_t)configurationvalue;
					endpoints[endpoint].position = endpoints[endpoint].buffer;
					endpoints[endpoint].remain = 1;
					if (p->wLength == 0)
						endpoints[endpoint].remain = 0;
					break;
				case GET_INTERFACE:
					endpoints[endpoint].buffer[0] = (uint8_t)find_alternate(p->wIndex);
					endpoints[endpoint].position = endpoints[endpoint].buffer;
					endpoints[endpoint].remain = 1;
					if (p->wLength == 0)
						endpoints[endpoint].remain = 0;
					break;
				case SYNCH_FRAME:
					return handle_synch_frame_request(endpoint, p);
				default:
					return handle_nonstandard_request(endpoint, p);
					break;
				}
			}
			else
				return handle_nonstandard_request(endpoint, p);
			size = 0;
		}
		else
			return handle_nonstandard_request(endpoint, p);
	}
	else if (pid == InPid) {
		if (endpoints[endpoint].type == ControlEndpoint) { //if (endpoint == 0) {
			// if no data has been transferred (except for the setup stage)
			// and the length of this IN transaction is 0
			// assume this is the status stage
			if ((endpoints[endpoint].remain == 0) && (size == 0)) {
				if ((endpoint == 0) && (settingaddress == true))
				{
					// set of address is active at end of status stage
					busmanager->usb_ohci_device_address_changed(address, newaddress);
					address = newaddress;
					settingaddress = false;
					state = AddressState;
				}
				if (wantstatuscallback == true)
					handle_status_stage(endpoint);
				wantstatuscallback = false;
				return 0;
			}
			// case ==1, give data
			// case ==0, nothing
			// if device->host, since InPid then this is data stage
			if (endpoints[endpoint].controldirection == DeviceToHost) {
				// data stage
				if (size > endpoints[endpoint].remain)
					size = endpoints[endpoint].remain;
				if (endpoints[endpoint].position != nullptr)
					memcpy(buffer, endpoints[endpoint].position, size);
				endpoints[endpoint].position = endpoints[endpoint].position + size;
				endpoints[endpoint].remain = endpoints[endpoint].remain - size;
			}
			else {
				if (wantstatuscallback == true)
					handle_status_stage(endpoint);
				wantstatuscallback = false;
			}
		}
		else if (endpoints[endpoint].type == BulkEndpoint)
			return handle_bulk_pid(endpoint, pid, buffer, size);
		else if (endpoints[endpoint].type == InterruptEndpoint)
			return handle_interrupt_pid(endpoint, pid, buffer, size);
		else if (endpoints[endpoint].type == IsochronousEndpoint)
			return handle_isochronous_pid(endpoint, pid, buffer, size);
		else
			return -1;
	}
	else if (pid == OutPid) {
		if (endpoints[endpoint].type == ControlEndpoint) {
			// case ==1, nothing
			// case ==0, give data
			// if host->device, since OutPid then this is data stage
			if (endpoints[endpoint].controldirection == HostToDevice) {
				// data stage
				if (size > endpoints[endpoint].remain)
					size = endpoints[endpoint].remain;
				if (endpoints[endpoint].position != nullptr)
					memcpy(endpoints[endpoint].position, buffer, size);
				endpoints[endpoint].position = endpoints[endpoint].position + size;
				endpoints[endpoint].remain = endpoints[endpoint].remain - size;
			}
			else {
				if (wantstatuscallback == true)
					handle_status_stage(endpoint);
				wantstatuscallback = false;
			}
		}
		else if (endpoints[endpoint].type == BulkEndpoint)
			return handle_bulk_pid(endpoint, pid, buffer, size);
		else if (endpoints[endpoint].type == InterruptEndpoint)
			return handle_interrupt_pid(endpoint, pid, buffer, size);
		else if (endpoints[endpoint].type == IsochronousEndpoint)
			return handle_isochronous_pid(endpoint, pid, buffer, size);
		else
			return -1;
	}
	return size;
}

/*
 * Usb port connector
 */

DEFINE_DEVICE_TYPE(OHCI_USB_CONNECTOR, ohci_usb_connector, "usb_connector", "Usb Connector Abstraction");

ohci_usb_connector::ohci_usb_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, OHCI_USB_CONNECTOR, tag, owner, clock),
	device_single_card_slot_interface<device_usb_ohci_function_interface>(mconfig, *this)
{
}

ohci_usb_connector::~ohci_usb_connector()
{
}

void ohci_usb_connector::device_start()
{
}

/*
 * Game controller usb device
 */

INPUT_PORTS_START(xbox_controller)
	PORT_START("ThumbstickLh") // left analog thumbstick horizontal movement
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_NAME("ThumbstickLh") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_J) PORT_CODE_INC(KEYCODE_L)
	PORT_START("ThumbstickLv") // left analog thumbstick vertical movement
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_NAME("ThumbstickLv") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_K) PORT_CODE_INC(KEYCODE_I)

	PORT_START("ThumbstickRh") // right analog thumbstick horizontal movement
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_NAME("ThumbstickRh") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD)
	PORT_START("ThumbstickRv") // right analog thumbstick vertical movement
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_NAME("ThumbstickRv") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_2_PAD) PORT_CODE_INC(KEYCODE_8_PAD)

	PORT_START("DPad") // pressure sensitive directional pad
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_NAME("DPad Up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_NAME("DPad Down")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_NAME("DPad Left")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_NAME("DPad Right")

	PORT_START("TriggerL") // analog trigger
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("TriggerL") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_1_PAD) PORT_CODE_INC(KEYCODE_7_PAD)

	PORT_START("TriggerR") // analog trigger
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("TriggerR") PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_3_PAD) PORT_CODE_INC(KEYCODE_9_PAD)

	PORT_START("Buttons") // digital buttons
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Start") // Start button
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Back") // Back button

	PORT_START("AGreen") // analog button
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("A-Green") PORT_SENSITIVITY(100) PORT_KEYDELTA(32)  PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_A) PORT_CODE_INC(KEYCODE_Q)

	PORT_START("BRed") // analog button
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("B-Red") PORT_SENSITIVITY(100) PORT_KEYDELTA(32) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_S) PORT_CODE_INC(KEYCODE_W)

	PORT_START("XBlue") // analog button
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("X-Blue") PORT_SENSITIVITY(100) PORT_KEYDELTA(32) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_D) PORT_CODE_INC(KEYCODE_E)

	PORT_START("YYellow") // analog button
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("Y-Yellow") PORT_SENSITIVITY(100) PORT_KEYDELTA(32) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_F) PORT_CODE_INC(KEYCODE_R)

	PORT_START("Black") // analog button
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("Black") PORT_SENSITIVITY(100) PORT_KEYDELTA(32) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_G) PORT_CODE_INC(KEYCODE_T)

	PORT_START("White") // analog button
	PORT_BIT(0xff, 0x00, IPT_PEDAL) PORT_NAME("White") PORT_SENSITIVITY(100) PORT_KEYDELTA(32) PORT_MINMAX(0, 0xff)
		PORT_CODE_DEC(KEYCODE_H) PORT_CODE_INC(KEYCODE_Y)
INPUT_PORTS_END

const USBStandardDeviceDescriptor ohci_game_controller_device::devdesc = { 18,1,0x110,0x00,0x00,0x00,64,0x45e,0x202,0x100,0,0,0,1 };
const USBStandardConfigurationDescriptor ohci_game_controller_device::condesc = { 9,2,0x20,1,1,0,0x80,50 };
const USBStandardInterfaceDescriptor ohci_game_controller_device::intdesc = { 9,4,0,0,2,0x58,0x42,0,0 };
const USBStandardEndpointDescriptor ohci_game_controller_device::enddesc82 = { 7,5,0x82,3,0x20,4 };
const USBStandardEndpointDescriptor ohci_game_controller_device::enddesc02 = { 7,5,0x02,3,0x20,4 };

DEFINE_DEVICE_TYPE(OHCI_GAME_CONTROLLER, ohci_game_controller_device, "ohci_gc", "OHCI Game Controller")

ohci_game_controller_device::ohci_game_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, OHCI_GAME_CONTROLLER, tag, owner, clock),
	device_usb_ohci_function_interface(mconfig, *this),
	m_ThumbstickLh(*this, "ThumbstickLh"),
	m_ThumbstickLv(*this, "ThumbstickLv"),
	m_ThumbstickRh(*this, "ThumbstickRh"),
	m_ThumbstickRv(*this, "ThumbstickRv"),
	m_DPad(*this, "DPad"),
	m_TriggerL(*this, "TriggerL"),
	m_TriggerR(*this, "TriggerR"),
	m_Buttons(*this, "Buttons"),
	m_AGreen(*this, "AGreen"),
	m_BRed(*this, "BRed"),
	m_XBlue(*this, "XBlue"),
	m_YYellow(*this, "YYellow"),
	m_Black(*this, "Black"),
	m_White(*this, "White")
{
}

void ohci_game_controller_device::initialize()
{
	device_usb_ohci_function_interface::initialize();
	add_device_descriptor(devdesc);
	add_configuration_descriptor(condesc);
	add_interface_descriptor(intdesc);
	add_endpoint_descriptor(enddesc82);
	add_endpoint_descriptor(enddesc02);
}

int ohci_game_controller_device::handle_nonstandard_request(int endpoint, USBSetupPacket *setup)
{
	//                                    >=8  ==42  !=0  !=0  1,3       2<20 <=20
	static const uint8_t reportinfo[16] = { 0x10,0x42 ,0x32,0x43,1   ,0x65,0x14,0x20,0x98,0xa9,0xba,0xcb,0xdc,0xed,0xfe };

	if (endpoint != 0)
		return -1;
	if ((endpoints[endpoint].controltype == VendorType) && (endpoints[endpoint].controlrecipient == InterfaceRecipient))
	{
		if ((setup->bRequest == GET_DESCRIPTOR) && (setup->wValue == 0x4200))
		{
			endpoints[endpoint].position = (uint8_t *)reportinfo;
			endpoints[endpoint].remain = 16;
			return 0;
		}
	}
	if ((endpoints[endpoint].controltype == ClassType) && (endpoints[endpoint].controlrecipient == InterfaceRecipient))
	{
		if ((setup->bRequest == 1) && (setup->wValue == 0x0100))
		{
			endpoints[endpoint].position = endpoints[endpoint].buffer;
			endpoints[endpoint].remain = setup->wLength;
			for (int n = 0; n < setup->wLength; n++)
				endpoints[endpoint].buffer[n] = 0x10 ^ n;
			endpoints[endpoint].buffer[2] = 0;
			return 0;
		}
	}
	if ((endpoints[endpoint].controltype == VendorType) && (endpoints[endpoint].controlrecipient == InterfaceRecipient))
	{
		if ((setup->bRequest == 1) && (setup->wValue == 0x0200))
		{
			endpoints[endpoint].position = endpoints[endpoint].buffer;
			endpoints[endpoint].remain = setup->wLength;
			for (int n = 0; n < setup->wLength; n++)
				endpoints[endpoint].buffer[n] = 0x20 ^ n;
			return 0;
		}
	}
	if ((endpoints[endpoint].controltype == VendorType) && (endpoints[endpoint].controlrecipient == InterfaceRecipient))
	{
		if ((setup->bRequest == 1) && (setup->wValue == 0x0100))
		{
			endpoints[endpoint].position = endpoints[endpoint].buffer;
			endpoints[endpoint].remain = setup->wLength;
			for (int n = 0; n < setup->wLength; n++)
				endpoints[endpoint].buffer[n] = 0x30 ^ n;
			return 0;
		}
	}
	return -1;
}

int ohci_game_controller_device::handle_interrupt_pid(int endpoint, int pid, uint8_t *buffer, int size)
{
	if ((endpoint == 2) && (pid == InPid)) {
		int v;

		buffer[0] = 0;
		buffer[1] = 20;
		v = m_DPad->read();
		v = v | (m_Buttons->read() << 4);
		buffer[2] = (uint8_t)v;
		buffer[3] = 0;
		buffer[4] = m_AGreen->read();
		buffer[5] = m_BRed->read();
		buffer[6] = m_XBlue->read();
		buffer[7] = m_YYellow->read();
		buffer[8] = m_Black->read();
		buffer[9] = m_White->read();
		buffer[10] = m_TriggerL->read();
		buffer[11] = m_TriggerR->read();
		v = m_ThumbstickLh->read();
		v = (v - 128) * 256;
		put_u16le(&buffer[12], v);
		v = m_ThumbstickLv->read();
		v = (v - 128) * 256;
		put_u16le(&buffer[14], v);
		v = m_ThumbstickRh->read();
		v = (v - 128) * 256;
		put_u16le(&buffer[16], v);
		v = m_ThumbstickRv->read();
		v = (v - 128) * 256;
		put_u16le(&buffer[18], v);
		return size;
	}
	return -1;
}

void ohci_game_controller_device::device_start()
{
	initialize();
}

ioport_constructor ohci_game_controller_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xbox_controller);
}
