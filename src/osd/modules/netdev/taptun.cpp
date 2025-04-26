// license:BSD-3-Clause
// copyright-holders:Carl
#include "netdev_module.h"

#include "modules/osdmodule.h"

#if defined(OSD_NET_USE_TAPTUN)

#include "netdev_common.h"

#include "osdcore.h" // osd_printf_verbose
#include "osdfile.h" // PATH_SEPARATOR

#include "util/hashing.h" // crc32_creator
#include "util/unicode.h"

#include <memory>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#include <winioctl.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <cerrno>
#endif

#ifdef __linux__
#define IFF_TAP     0x0002
#define IFF_NO_PI   0x1000
#define TUNSETIFF     _IOW('T', 202, int)
#elif defined(_WIN32)
#include "tap-windows6/tap-windows.h"

// for some reason this isn't defined in the header, and presumably it changes
// with major? versions of the driver - perhaps it should be configurable?
#define PRODUCT_TAP_WIN_COMPONENT_ID "tap0901"
#endif

namespace osd {

namespace {

// Ethernet minimum frame length
static constexpr int ETHERNET_MIN_FRAME = 64;

class taptun_module : public osd_module, public netdev_module
{
public:

	taptun_module() : osd_module(OSD_NETDEV_PROVIDER, "taptun"), netdev_module()
	{
	}
	virtual ~taptun_module() { }

	virtual int init(osd_interface &osd, const osd_options &options) override;
	virtual void exit() override;

	virtual bool probe() override { return true; }

	virtual std::unique_ptr<network_device> open_device(int id, network_handler &handler) override;
	virtual std::vector<network_device_info> list_devices() override;

private:
	struct device_info
	{
		std::string name;
		std::string description;
	};

	std::vector<device_info> m_devices;
};



class netdev_tap : public network_device_base
{
public:
	netdev_tap(const char *name, network_handler &handler);
	~netdev_tap();

	int send(void const *buf, int len) override;

protected:
	int recv_dev(uint8_t **buf) override;

private:
#if defined(_WIN32)
	HANDLE m_handle = INVALID_HANDLE_VALUE;
	OVERLAPPED m_overlapped;
	bool m_receive_pending;
#else
	int m_fd = -1;
	char m_ifname[10];
#endif
	uint8_t m_buf[2048];
};

netdev_tap::netdev_tap(const char *name, network_handler &handler)
	: network_device_base(handler)
{
#if defined(__linux__)
	m_fd = -1;
	if((m_fd = open("/dev/net/tun", O_RDWR)) == -1) {
		osd_printf_verbose("tap: open failed %d\n", errno);
		return;
	}

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	sprintf(ifr.ifr_name, "tap-mess-%d-0", getuid());
	if(ioctl(m_fd, TUNSETIFF, (void *)&ifr) == -1) {
		osd_printf_verbose("tap: ioctl failed %d\n", errno);
		close(m_fd);
		m_fd = -1;
		return;
	}
	osd_printf_verbose("netdev_tap: network up!\n");
	strncpy(m_ifname, ifr.ifr_name, 10);
	fcntl(m_fd, F_SETFL, O_NONBLOCK);
#elif defined(_WIN32)
	std::wstring device_path(L"" USERMODEDEVICEDIR);
	device_path.append(wstring_from_utf8(name));
	device_path.append(L"" TAP_WIN_SUFFIX);

	m_handle = CreateFileW(device_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED, 0);
	if (m_handle != INVALID_HANDLE_VALUE)
	{
		ULONG status = TRUE;
		DWORD len;

		// set media status to connected
		DeviceIoControl(m_handle, TAP_WIN_IOCTL_SET_MEDIA_STATUS, &status, sizeof(status), &status, sizeof(status), &len, NULL);
	}

	m_receive_pending = false;
#else
	m_fd = -1;
#endif
}

netdev_tap::~netdev_tap()
{
#if defined(_WIN32)
	if (m_handle != INVALID_HANDLE_VALUE)
	{
		if (m_receive_pending)
			CancelIo(m_handle);

		CloseHandle(m_handle);
	}
#else
	close(m_fd);
#endif
}

static u32 finalise_frame(u8 buf[], u32 length)
{
	/*
	 * The taptun driver receives frames which are shorter than the Ethernet
	 * minimum. Partly this is because it can't see the frame check sequence
	 * bytes, but mainly it's because the OS expects the lower level device
	 * to add the required padding.
	 *
	 * We do the equivalent padding here (i.e. pad with zeroes to the
	 * minimum Ethernet length minus FCS), so that devices which check
	 * for this will not reject these packets.
	 */
	if (length < ETHERNET_MIN_FRAME - 4)
	{
		std::fill_n(&buf[length], ETHERNET_MIN_FRAME - length - 4, 0);

		length = ETHERNET_MIN_FRAME - 4;
	}

	// compute and append the frame check sequence
	const u32 fcs = util::crc32_creator::simple(buf, length);

	buf[length++] = (fcs >> 0) & 0xff;
	buf[length++] = (fcs >> 8) & 0xff;
	buf[length++] = (fcs >> 16) & 0xff;
	buf[length++] = (fcs >> 24) & 0xff;

	return length;
}

#if defined(_WIN32)
int netdev_tap::send(void const *buf, int len)
{
	OVERLAPPED overlapped = {};

	if (m_handle == INVALID_HANDLE_VALUE)
		return 0;

	if (WriteFile(m_handle, buf, len, NULL, &overlapped) || GetLastError() == ERROR_IO_PENDING)
	{
		DWORD bytes_transferred;

		// block until transfer complete
		if (GetOverlappedResult(m_handle, &overlapped, &bytes_transferred, TRUE))
			return bytes_transferred;
	}

	return 0;
}

int netdev_tap::recv_dev(uint8_t **buf)
{
	DWORD bytes_transferred;

	if (m_handle == INVALID_HANDLE_VALUE)
		return 0;

	if (!m_receive_pending)
	{
		// start a new asynchronous read
		m_overlapped = {};
		if (ReadFile(m_handle, m_buf, sizeof(m_buf), &bytes_transferred, &m_overlapped))
		{
			// handle unexpected synchronous completion
			*buf = m_buf;

			return finalise_frame(m_buf, bytes_transferred);
		}
		else if (GetLastError() == ERROR_IO_PENDING)
			m_receive_pending = true;
	}
	else
	{
		if (GetOverlappedResult(m_handle, &m_overlapped, &bytes_transferred, FALSE))
		{
			// handle asynchronous completion
			m_receive_pending = false;
			*buf = m_buf;

			return finalise_frame(m_buf, bytes_transferred);
		}
	}

	return 0;
}

static std::wstring safe_string(WCHAR value[], int length)
{
	if (value[length] != L'\0')
		value[length + 1] = L'\0';

	return std::wstring(value);
}

// find the friendly name for an adapter in the registry
static std::wstring get_connection_name(std::wstring const &id)
{
	std::wstring result;

	std::wstring connection(L"" NETWORK_CONNECTIONS_KEY PATH_SEPARATOR);
	connection.append(id);
	connection.append(PATH_SEPARATOR L"Connection");

	HKEY connection_key;

	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, connection.c_str(), 0, KEY_READ, &connection_key) == ERROR_SUCCESS)
	{
		WCHAR connection_name[MAX_PATH];
		DWORD connection_name_len = sizeof(connection_name);
		DWORD data_type;

		if (RegQueryValueExW(connection_key, L"Name", NULL, &data_type, LPBYTE(connection_name), &connection_name_len) == ERROR_SUCCESS && data_type == REG_SZ)
		{
			result.assign(safe_string(connection_name, connection_name_len));
		}
	}

	return result;
}

// find TAP-Windows adapters by scanning the registry
static std::vector<std::wstring> get_tap_adapters()
{
	std::vector<std::wstring> result;
	HKEY adapter_key;

	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"" ADAPTER_KEY, 0, KEY_READ, &adapter_key) == ERROR_SUCCESS)
	{
		int i = 0;
		WCHAR enum_name[MAX_PATH];
		DWORD enum_name_len = sizeof(enum_name);

		// iterate through all the adapters
		while (RegEnumKeyExW(adapter_key, i, enum_name, &enum_name_len, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
		{
			std::wstring unit_string(L"" ADAPTER_KEY PATH_SEPARATOR);
			unit_string.append(enum_name);
			HKEY unit_key;

			if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, unit_string.c_str(), 0, KEY_READ, &unit_key) == ERROR_SUCCESS)
			{
				WCHAR component_id[MAX_PATH];
				DWORD component_id_len = sizeof(component_id);
				DWORD data_type;

				// check if the ComponentId value indicates a TAP-Windows adapter
				if (RegQueryValueExW(unit_key, L"ComponentId", NULL, &data_type, LPBYTE(component_id), &component_id_len) == ERROR_SUCCESS
					&& data_type == REG_SZ)
				{
					std::wstring const value(safe_string(component_id, component_id_len));

					// some older versions may not set the "root\" prefix
					if (value == L"root\\" PRODUCT_TAP_WIN_COMPONENT_ID || value == L"" PRODUCT_TAP_WIN_COMPONENT_ID)
					{
						WCHAR net_cfg_instance_id[MAX_PATH];
						DWORD net_cfg_instance_id_len = sizeof(net_cfg_instance_id);

						// add the adapter to the result
						if (RegQueryValueExW(unit_key, L"NetCfgInstanceId", NULL, &data_type, LPBYTE(net_cfg_instance_id), &net_cfg_instance_id_len) == ERROR_SUCCESS
							&& data_type == REG_SZ)
							result.push_back(safe_string(net_cfg_instance_id, net_cfg_instance_id_len));
					}
				}

				RegCloseKey(unit_key);
			}

			enum_name_len = sizeof(enum_name);
			i++;
		}

		RegCloseKey(adapter_key);
	}

	return result;
}
#else
int netdev_tap::send(void const *buf, int len)
{
	if(m_fd == -1) return 0;
	len = write(m_fd, buf, len);
	return (len == -1)?0:len;
}

int netdev_tap::recv_dev(uint8_t **buf)
{
	if (0 > m_fd)
		return 0;

	int len = read(m_fd, m_buf, sizeof(m_buf));

	if (len > 0)
		len = finalise_frame(m_buf, len);

	*buf = m_buf;
	return (len == -1) ? 0 : len;
}
#endif

int taptun_module::init(osd_interface &osd, const osd_options &options)
{
#if defined(_WIN32)
	auto const adapters = get_tap_adapters();
	m_devices.reserve(adapters.size());
	for (std::wstring const &id : adapters)
		m_devices.emplace_back(device_info{ utf8_from_wstring(id), utf8_from_wstring(get_connection_name(id)) });
#else
	m_devices.emplace_back(device_info{ "tap", "TUN/TAP Device" });
#endif
	return 0;
}

void taptun_module::exit()
{
	m_devices.clear();
}

std::unique_ptr<network_device> taptun_module::open_device(int id, network_handler &handler)
{
	if ((0 > id) || (m_devices.size() <= id))
		return nullptr;

	return std::make_unique<netdev_tap>(m_devices[id].name.c_str(), handler);
}

std::vector<network_device_info> taptun_module::list_devices()
{
	std::vector<network_device_info> result;
	result.reserve(m_devices.size());
	for (int id = 0; m_devices.size() > id; ++id)
		result.emplace_back(network_device_info{ id, m_devices[id].description });
	return result;
}

} // anonymous namespace

} // namespace osd


#else

namespace osd { namespace { MODULE_NOT_SUPPORTED(taptun_module, OSD_NETDEV_PROVIDER, "taptun") } }

#endif


MODULE_DEFINITION(NETDEV_TAPTUN, osd::taptun_module)
