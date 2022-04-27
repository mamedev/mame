// license:BSD-3-Clause
// copyright-holders: Gregory Lewandowski
/***************************************************************************

Driver by Grego Lewandowski, based on schematics, PAL dumps/equations, and technical assistance from Darksoft. 

This driver emulates the CPS2 network adapter at a very high level over UDP, ignoring many intricacies of the chips involved, but effectively reproducing the overall behaviors.

From cps2.cpp you can see that the network adapter primarily involves three components, the 71051 parallel to serial chip, the MAX232, and a custom PAL chip.

The devices are connected in a chain with the server being the first link.  Each device has an "IN" and an "OUT" port, the servers IN port is supplied with a loopback or dummy cable.
Whatever the server writes to the IN port will be read back during the "Terminal Check" phase of initializing the network devices, this is how the server is determined by the software.
From the servers OUT port clients are daisy chained, each successive units OUT going to the next one's IN. The final client (Terminal 4) has no connection on his OUT port.

Through analysis of the PAL equations Capcom's networking strategy becomes clear.  The pal allows the 71051 to address either the IN or OUT ports on the network adapter through the
two channels of the MAX232, or it can completely disable communication between the two, but it's fourth function is the most interesting.  Via the PAL chip the two channels of the MAX232
can be tied together, ones RX going to the other TX and vice-versa.  This allows the client to act as a passthrough, but in this state the client can still listen to the servers commands
and respond! While in passthrough mode the IN ports signals are being passed to the receive pin of the 71051, and its transmit pin is mixed with the "passthrough" data via the following 
boolean expression:

B5 =  ( I0 ||  I2 ||  I4 || !I5)
  &&  (!I0 ||  I2 ||  I4 || !I5)
  &&  ( I0 || !I2 ||  I4 || !I5)

Where:
  B5 - TIN1 to 232
  I0 - TX Data from 71051 
  I2 - ROUT2 from 232
  I4 - PAL control bit 1
  I5 - PAL control bit 0

The passthrough mode is PAL mode $1 or %01 in binary, applying this to the equation simplifies it to:

B5 = ( I0 ||  I2) && (!I0 ||  I2) && ( I0 || !I2) 

Because the expression (I4 || !I5) evaluates to (0 || 0), this new equation simplifies to (I0 || I2) since if either are 1, it evaluates to 1. So if two clients were to attempt 
communication simultaneously their messages would be OR'd together and cause a framing or overrun error at the server. 

Because of this passthrough mode messages sent between the server and clients have nearly no delay.  The signal is passed from one max232 chip to another thru the pal and so only very minor analog and gate delays exist.

This "instantaneous" messaging was modeled in the driver by having the checkDataAvailable method being polled several times per frame.  This should ensure that passing a message across multiple clients still results in it
being received within a single frame. A more accurate system for modeling the passthrough would likely involve a separate network thread and modeling the delays of sending messages through a UART. However,
this is likely unnecessary and would provide no additional benefit to the end user.

The clients/server use separate ports as during debugging it is useful to be able to launch all the clients on the same dev system. The server/client configuration can be passed to the driver via debug commands OR from the
commandline when launching MAME as so:

	start mame.exe -debug -cps2server ssf2xjtb
	start mame.exe -debug -cps2client 1,127.0.0.1 ssf2xjtb
	start mame.exe -debug -cps2client 2,127.0.0.1 ssf2xjtb
	start mame.exe -debug -cps2client 3,127.0.0.1 ssf2xjtb

When connecting multiple systems together simply replace the localhost address with the address of the next system in the chain.

Currently debugging always assumes the localhost address and so the cps2client debug command only requires one parameter which is the client id.  However both the server and client commands can enable debug logging as so:

	cps2server 1
	cps2client 1,1

Remove one parameter from each command to disable debug logging.

TODO:
  1.) Determine why overrun cannot be reported by net state, I believe the interaction between clients during pass thru isn't modeled entirely correctly.
  2.) Support *nix UDP sockets, should be fairly simple.
  3.) Commonalize send/read methods.

*/

#include "emu.h"
#include "emuopts.h"
#include "includes/cps1.h"

#include "debug/debugcon.h"
#include "debug/debugcmd.h"
#include "debugger.h"

#include <windows.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>

// Port numbers assigned to each of the instances that require a server connection
#define  SERVER_PORT_NUM           1050
#define  CLIENT_PORT_NUM_1         1060
#define  CLIENT_PORT_NUM_2         1070

#define  LOCALHOST_ADDR      "127.0.0.1" // Local host

// Offsets from the base net adapter address to access each component
static const int NET_DATA_OFFSET = 0;
static const int NET_STAT_OFFSET = 16;
static const int NET_PAL_OFFSET = 4;

// Array used to determine port number based on client/server id
static const u_short PORT_NUMBERS[] = { SERVER_PORT_NUM, CLIENT_PORT_NUM_1 , CLIENT_PORT_NUM_2 };

// Pal modes
static const int PAL_MODE_NO_CON = 0; // Reading/writing do nothing
static const int PAL_MODE_PASS_THRU = 1; // Pass data across connections, listen to whats coming from server (left/in)
static const int PAL_MODE_SEND_TO_CLIENT = 2; // Send to right/out
static const int PAL_MODE_SEND_TO_SERVER = 3; // Send to left/in

WORD wVersionRequested = MAKEWORD(1, 1);       // Stuff for WSA functions
WSADATA wsaData;

u_long iMode = 1; // Non blocking io

char out_buf[100];
char in_buf[100];

// Struct which contains the data necessary for a udp connection
struct udp_connection {
	int	socket;
	struct sockaddr_in address;
	int addr_len;
};

// The primary data needed to simulate the net adapter
struct cps2_network {
	int palState = 0;
	int dataIn = 0;
	int dataRdy = 0;
	int overrun = 0;
	int isServer = 0;
	int clientNumber = 0;
	int debugLogging = 0;
};

struct cps2_network networkVars;
struct udp_connection toServer;
struct udp_connection toClient;

/**
 * Sends data toward the server, or to the IN port as it is labeled.
 */
void sendDataToServer(u16 data, int debug) {
	out_buf[0] = data & 0xFF;
	sendto(toServer.socket, out_buf, 1, 0, (struct sockaddr *)&toServer.address, sizeof(toServer.address));
	if (debug) {
		if (networkVars.debugLogging) std::cout << networkVars.clientNumber << " - Network write to server " << std::hex << data << "\r\n";
	}
}

/**
 * Sends data to the next client, or to the OUT port as it is labeled.
 */
void sendDataToClient(u16 data, int debug) {
	out_buf[0] = data & 0xFF;
	sendto(toClient.socket, out_buf, 1, 0, (struct sockaddr *)&toClient.address, sizeof(toClient.address));
	if (debug) {
		if (networkVars.debugLogging) std::cout << networkVars.clientNumber << " - Network write to client " << std::hex << data << "\r\n";
	}
}

/**
 * Attempts to read data sent from the server, returns 0x7FFF if no data was available.
 */
int readFromServer() {
	toServer.addr_len = sizeof(toServer.address);
	int retcode = recvfrom(toServer.socket, in_buf, 1, 0, (struct sockaddr *)&toServer.address, &toServer.addr_len);
	if (retcode > 0) {
		if (networkVars.debugLogging) std::cout << networkVars.clientNumber << " - Network read from server " << std::hex << (((int)in_buf[0]) & 0xFF) << "\r\n";
		return in_buf[0];
	} else {
		return 0x7FFF;
	}
}

/**
 * Attempts to read data sent from the client, returns 0x7FFF if no data was available.
 */
int readFromClient() {
	toClient.addr_len = sizeof(toClient.address);
	int retcode = recvfrom(toClient.socket, in_buf, 1, 0, (struct sockaddr *)&toClient.address, &toClient.addr_len);
	if (retcode > 0) {
		if (networkVars.debugLogging) std::cout << networkVars.clientNumber << " - Network read from client " << std::hex << (((int)in_buf[0]) & 0xFF) << "\r\n";
		return in_buf[0];
	} else {
		return 0x7FFF;
	}
}

/**
 * Checks whether any data was available from either the client or server connections and applys the correct action.
 * This method is called many times per frame to approximate the real behavior of the net adapter when in pass thru mode.
 */
void cps2_state::checkDataAvailable() {
	int canReceiveFromServer = !networkVars.isServer;
	int canReceiveFromClient = networkVars.clientNumber < 3;

	int serverData = 0x7FFF;
	if (canReceiveFromServer) {
		serverData = readFromServer();
	}

	int clientData = 0x7FFF;
	if (canReceiveFromClient) {
		clientData = readFromClient();
	}

	switch (networkVars.palState) {
		case PAL_MODE_SEND_TO_CLIENT:
			if (clientData != 0x7FFF) {
				networkVars.dataIn = clientData & 0xFF;
				networkVars.overrun = networkVars.dataRdy;
				networkVars.dataRdy = 1;
			}
			break;
		case PAL_MODE_SEND_TO_SERVER:
			if (serverData != 0x7FFF) {
				networkVars.dataIn = serverData & 0xFF;
				networkVars.overrun = networkVars.dataRdy;
				networkVars.dataRdy = 1;
			}
			break;
		case PAL_MODE_PASS_THRU:
			if (clientData != 0x7FFF) {
				sendDataToServer(clientData & 0xFF, 0);
				if (networkVars.debugLogging) std::cout << networkVars.clientNumber << " - Passthru to server " << std::hex << (clientData & 0xFF) << "\r\n";
			}
			if (serverData != 0x7FFF) {
				sendDataToClient(serverData & 0xFF, 0);
				networkVars.dataIn = serverData & 0xFF;
				networkVars.overrun = networkVars.dataRdy;
				networkVars.dataRdy = 1;
				if (networkVars.debugLogging) std::cout << networkVars.clientNumber << " - Passthru to client " << std::hex << (serverData & 0xFF) << "\r\n";
			}
			break;
		case PAL_MODE_NO_CON:
		default:
			return;
	}
}

/**
 * Calculates and returns the current net status.
 */
int calculateNetStat() {
	int result = 1; // Transmit ready... never checked?

	if (networkVars.dataRdy) {
		result += 2;
	}

	if (networkVars.overrun) {
		// TODO: Dunno why overrun can't be reported, must not be a real overrun happening, just an issue with passthru not being entirely correct
		//	result += 16;
		if (networkVars.debugLogging) std::cout << networkVars.clientNumber << " - OVERRUN \r\n";
	}

	return result;
}

/**
 * Read handler
 */
READ16_MEMBER(cps2_state::cps2_network_r) {
	checkDataAvailable();

	if (offset == NET_DATA_OFFSET) {
		if (networkVars.debugLogging) std::cout << networkVars.clientNumber << " - Network data read " << std::hex << (networkVars.dataIn & 0xFF) << "\r\n";
		int result = networkVars.dataIn & 0xFF;
		networkVars.dataRdy = 0;
		return result;
	} else if (offset == NET_STAT_OFFSET) {
		return calculateNetStat();
	}
	return 0;
}

/**
 * Write handler sub method which handles the server loop back functionality. Required for the software to determine the server.
 */
void handleWriteNetDataToServer(u16 data) {
	if (networkVars.isServer) {
		networkVars.dataIn = data & 0xFF; // Server writes back to itself due to loopback cable
		networkVars.overrun = networkVars.dataRdy;
		networkVars.dataRdy = 1;
		if (networkVars.debugLogging) std::cout << networkVars.clientNumber << " - Server loopback " << std::hex << (data & 0xFF) << "\r\n";
	}
	else {
		sendDataToServer(data, 1);
	}
}

/**
 * Write handler sub
 */
void handleWriteNetDataToClient(u16 data) {
	if (networkVars.clientNumber < 3) {
		sendDataToClient(data, 1);
	}
}

/**
 * Write handler
 */
WRITE16_MEMBER(cps2_state::cps2_network_w) {
	checkDataAvailable();

	if (offset == NET_PAL_OFFSET) {
		networkVars.palState = data & 0x03;
		if (networkVars.debugLogging) std::cout << networkVars.clientNumber << " - Pal state change: " << networkVars.palState << "\r\n";
	} else if (offset == NET_DATA_OFFSET) {
		switch (networkVars.palState) {
			case PAL_MODE_SEND_TO_CLIENT:
				handleWriteNetDataToClient(data);
				break;
			case PAL_MODE_PASS_THRU:
			case PAL_MODE_SEND_TO_SERVER:
				handleWriteNetDataToServer(data);
				break;
			case PAL_MODE_NO_CON:
			default:
				return;
		}
	} else if (offset == NET_STAT_OFFSET) {
		if (data & 0x10) {
			networkVars.overrun = 0;
			if (networkVars.debugLogging) std::cout << networkVars.clientNumber << " - OVERRUN CLEARED \r\n";
		}
		if (networkVars.debugLogging) std::cout << networkVars.clientNumber << " - Net stat write " << std::hex << data << "\r\n";
	}
}

/**
 * Initializes a udp server connection.
 */
void initServerUDPConnection(int clientNumber) {
	toClient.socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (toClient.socket < 0) {
		std::cout << "*** ERROR - socket() failed \r\n";
		exit(-1);
	}

	ioctlsocket(toClient.socket, FIONBIO, &iMode); // Set non-blocking mode on socket.

	toClient.address.sin_family = AF_INET;                 // Address family to use
	toClient.address.sin_port = htons(PORT_NUMBERS[clientNumber]);           // Port number to use
	toClient.address.sin_addr.s_addr = htonl(INADDR_ANY);  // Listen on any IP address

	int retcode = bind(toClient.socket, (struct sockaddr *)&toClient.address, sizeof(toClient.address));
	if (retcode < 0) {
		std::cout << "*** ERROR - bind() failed \r\n";
		exit(-1);
	}

	std::cout << "Assigned server port " << PORT_NUMBERS[clientNumber] << "\r\n";
}

/**
 * Initializes a udp client connection.
 */
void initClientUDPConnection(char * ipAddress, int clientNumber) {
	toServer.socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (toServer.socket < 0) {
		std::cout << "*** ERROR - socket() failed \r\n";
		exit(-1);
	}

	ioctlsocket(toServer.socket, FIONBIO, &iMode); // Set non-blocking mode on socket.

	toServer.address.sin_family = AF_INET;                 // Address family to use
	toServer.address.sin_port = htons(PORT_NUMBERS[clientNumber - 1]);           // Port num to use
	toServer.address.sin_addr.s_addr = inet_addr(ipAddress); // IP address to use

	std::cout << "Assigned client port " << PORT_NUMBERS[clientNumber - 1] << "\r\n";
}

/**
 * Initializes the cps2 net server.
 */
void initcps2serverimpl(int debugLogging) {
	networkVars.isServer = 1;

	networkVars.debugLogging = debugLogging;

	std::cout << "Debug Logging " << networkVars.debugLogging << "\r\n";

	WSAStartup(wVersionRequested, &wsaData);

	initServerUDPConnection(0);
}

/**
 * Debugger command handler for initializing cps2 net server.
 */
void cps2_state::initcps2server(int ref, const std::vector<std::string> &params) {
	int paramCount = params.size();
	initcps2serverimpl(paramCount == 1 ? 1 : 0);
}

/**
 * Initializes a cps2 net client.
 */
void initcps2clientimpl(char * ipAddress, int clientNumber, int debugLogging) {
	networkVars.clientNumber = clientNumber;
	networkVars.debugLogging = debugLogging;

	std::cout << "Client Number " << networkVars.clientNumber << "\r\n";
	std::cout << "Debug Logging " << networkVars.debugLogging << "\r\n";
	std::cout << "Ip Address " << ipAddress << "\r\n";

	WSAStartup(wVersionRequested, &wsaData);

	initClientUDPConnection(ipAddress, networkVars.clientNumber);

	if (networkVars.clientNumber < 3) {
		initServerUDPConnection(networkVars.clientNumber);
	}
}

/**
 * Debugger command handler for initializing a cps2 net client.
 */
void cps2_state::initcps2client(int ref, const std::vector<std::string> &params) {
	int paramCount = params.size();
	int clientNumber = std::stoi(params[0], nullptr, 16);
	initcps2clientimpl(LOCALHOST_ADDR, clientNumber, paramCount == 2 ? 1 : 0);
}

/**
 * Main init method for cps2 network, registers debugger commands, apply's command line options.
 */
void cps2_state::init_cps2_network() {
	std::cout << "Init network\r\n";

	emu_options &options = machine().options();

	using namespace std::placeholders;
	if (options.debug()) {
		machine().debugger().console().register_command("cps2server", CMDFLAG_NONE, 0, 0, 1, std::bind(&cps2_state::initcps2server, this, _1, _2));
		machine().debugger().console().register_command("cps2client", CMDFLAG_NONE, 0, 1, 2, std::bind(&cps2_state::initcps2client, this, _1, _2));
	}

	bool cps2serveroption = options.cps2server();
	const char * cps2clientoption = options.cps2client();

	std::cout << "SERVER OPTION " << cps2serveroption << "\r\n";
	std::cout << "CLIENT OPTION " << cps2clientoption << "\r\n";

	if (cps2serveroption) {
		initcps2serverimpl(0);
	}

	if (strlen(cps2clientoption) != 0) {
		std::string clientIdString(1, cps2clientoption[0]);
		int clientId = std::stoi(clientIdString, nullptr, 16);
		initcps2clientimpl((char *)(&cps2clientoption[2]), clientId, 0);
	}
}