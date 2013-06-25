/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "precompiled.h"
#include <Rocket/Core/URL.h>
#include <stdio.h>

namespace Rocket {
namespace Core {

const char* DEFAULT_PROTOCOL = "file";

// Constructs an Empty URL.
URL::URL()
{
	port = 0;
	url_dirty = false;
}

// Constructs a new URL from the given string.
URL::URL(const String& _url)
{
	port = 0;
	ROCKET_VERIFY(SetURL(_url));
}

// Constructs a new URL from the given string.
URL::URL(const char* _url)
{
	port = 0;
	ROCKET_VERIFY(SetURL(_url));
}

// Destroys the URL.
URL::~URL()
{
}

// Assigns a new URL to the object.
bool URL::SetURL(const String& _url)
{
	url_dirty = false;
	url = _url;

	// Make sure an Empty URL is completely Empty.
	if (url.Empty())
	{
		protocol.Clear();
		login.Clear();
		password.Clear();
		host.Clear();
		port = 0;
		path.Clear();
		file_name.Clear();
		extension.Clear();

		return true;
	}

	// Find the protocol. This consists of the string appearing before the
	// '://' token (ie, file://, http://).
	const char* host_begin = strchr(_url.CString(), ':');
	if (NULL != host_begin)
	{
		protocol.Assign(_url.CString(), host_begin);
		if (0 != strncmp(host_begin, "://", 3))
		{
			char malformed_terminator[4] = {0, 0, 0, 0};
			strncpy(malformed_terminator, host_begin, 3);
			Log::Message(Log::LT_ERROR, "Malformed protocol identifier found in URL %s; expected %s://, found %s%s.\n", _url.CString(), protocol.CString(), protocol.CString(), malformed_terminator);

			return false;
		}
		host_begin += 3;
	}
	else
	{
		protocol = DEFAULT_PROTOCOL;
		host_begin = _url.CString();
	}


	// We only want to look for a host if a protocol was specified.
	const char* path_begin;
	if (host_begin != _url.CString())
	{
		// Find the host. This is the string appearing after the protocol or after
		// the username:password combination, and terminated either with a colon, 
		// if a port is specified, or a forward slash if there is no port.

		// Check for a login pair
		const char* at_symbol = strchr( host_begin, '@' );
		if ( at_symbol )
		{
			String login_password;
			login_password.Assign( host_begin, at_symbol );			
			host_begin = at_symbol + 1;

			const char* password_ptr = strchr( login_password.CString(), ':' );
			if ( password_ptr )
			{
				login.Assign( login_password.CString(), password_ptr );
				password.Assign( password_ptr + 1 );
			}
			else
			{
				login = login_password;
			}
		}

		// Get the host portion
		path_begin = strchr(host_begin, '/');
		// Search for the colon in the host name, which will indicate a port.
		const char* port_begin = strchr(host_begin, ':');
		if (NULL != port_begin && (NULL == path_begin || port_begin < path_begin))
		{
			if (1 != sscanf(port_begin, ":%d", &port))
			{
				Log::Message(Log::LT_ERROR, "Malformed port number found in URL %s.\n", _url.CString());
				return false;
			}

			host.Assign(host_begin, port_begin);

			// Don't continue if there is no path.
			if (NULL == path_begin)
			{
				return true;
			}

			// Increment the path string past the trailing slash.
			++path_begin;
		}
		else
		{
			port = -1;

			if (NULL == path_begin)
			{
				host = host_begin;
				return true;
			}
			else
			{
				// Assign the host name, then increment the path string past the
				// trailing slash.
				host.Assign(host_begin, path_begin);
				++path_begin;
			}
		}
	}
	else
	{
		path_begin = _url.CString();
	}
	
	// Check for parameters
	String path_segment;
	const char* parameters = strchr(path_begin, '?');
	if ( parameters )
	{
		// Pull the path segment out, so further processing doesn't read the parameters
		path_segment.Assign(path_begin, parameters);
		path_begin = path_segment.CString();
		
		// Loop through all parameters, loading them
		StringList parameter_list;
		StringUtilities::ExpandString( parameter_list, parameters + 1, '&' );
		for ( size_t i = 0; i < parameter_list.size(); i++ )
		{
			// Split into key and value
			StringList key_value;
			StringUtilities::ExpandString( key_value, parameter_list[i], '=' );
			
			if ( key_value.size() == 2 )
				this->parameters[key_value[0]] = key_value[1];
			else
				this->parameters[key_value[0]] = "";
		}
	}


	// Find the path. This is the string appearing after the host, terminated
	// by the last forward slash.
	const char* file_name_begin = strrchr(path_begin, '/');
	if (NULL == file_name_begin)
	{
		// No path!
		file_name_begin = path_begin;
		path = "";
	}
	else
	{
		// Copy the path including the trailing slash.
		path.Assign(path_begin, ++file_name_begin);

		// Normalise the path, stripping any ../'s from it
		size_t parent_dir_pos = String::npos;
		while ((parent_dir_pos = path.Find("/..")) != String::npos)
		{
			// If we found a /.. we should be able to find the start of the parent
			// directory, if we can't something wierd has happend, bail
			size_t parent_dir_start_pos = path.RFind("/", parent_dir_pos);
			if (parent_dir_start_pos == String::npos)
				break;

			// Strip out the parent dir and the /..
			path.Erase(parent_dir_start_pos, parent_dir_pos - parent_dir_start_pos + 3);

			// We've altered the URL, mark it dirty
			url_dirty = true;
		}
	}


	// Find the file name. This is the string after the trailing slash of the
	// path, and just before the extension.
	const char* extension_begin = strrchr(file_name_begin, '.');
	if (NULL == extension_begin)
	{
		file_name = file_name_begin;
		extension = "";
	}
	else
	{
		file_name.Assign(file_name_begin, extension_begin);
		extension = extension_begin + 1;
	}
	
	return true;
}

// Returns the entire URL.
const String& URL::GetURL() const
{
	if (url_dirty)
		ConstructURL();
	
	return url;
}

// Sets the URL's protocol.
bool URL::SetProtocol(const String& _protocol)
{
	protocol = _protocol;
	url_dirty = true;

	return true;
}

// Returns the protocol this URL is utilising.
const String& URL::GetProtocol() const
{
	return protocol;
}

/// Sets the URL's login
bool URL::SetLogin( const String& _login )
{
	login = _login;
	url_dirty = true;
	return true;
}

/// Returns the URL's login
const String& URL::GetLogin() const
{
	return login;
}

/// Sets the URL's password
bool URL::SetPassword(const String& _password)
{
	password = _password;
	url_dirty = true;
	return true;
}

/// Returns the URL's password
const String& URL::GetPassword() const
{
	return password;
}

// Sets the URL's host.
bool URL::SetHost(const String& _host)
{
	host = _host;
	url_dirty = true;

	return true;
}

// Returns the URL's host.
const String& URL::GetHost() const
{
	return host;
}

// Sets the URL's port number.
bool URL::SetPort(int _port)
{
	port = _port;
	url_dirty = true;

	return true;
}

// Returns the URL's port number.
int URL::GetPort() const
{
	return port;
}

// Sets the URL's path.
bool URL::SetPath(const String& _path)
{
	path = _path;
	url_dirty = true;

	return true;
}

// Prefixes the URL's existing path with the given prefix.
bool URL::PrefixPath(const String& prefix)
{
	// If there's no trailing slash on the end of the prefix, add one.
	if (!prefix.Empty() &&
		prefix[prefix.Length() - 1] != '/')
		path = prefix + "/" + path;
	else
		path = prefix + path;

	url_dirty = true;

	return true;
}

// Returns the URL's path.
const String& URL::GetPath() const
{
	return path;
}

// Sets the URL's file name.
bool URL::SetFileName(const String& _file_name)
{
	file_name = _file_name;
	url_dirty = true;

	return true;
}

// Returns the URL's file name.
const String& URL::GetFileName() const
{
	return file_name;
}

// Sets the URL's file extension.
bool URL::SetExtension(const String& _extension)
{
	extension = _extension;
	url_dirty = true;

	return true;
}

// Returns the URL's file extension.
const String& URL::GetExtension() const
{
	return extension;
}

// Gets the current parameters
const URL::Parameters& URL::GetParameters() const
{
	return parameters;
}

// Set an individual parameter
void URL::SetParameter(const String& key, const String& value)
{
	parameters[key] = value;
	url_dirty = true;
}

// Set all parameters
void URL::SetParameters(const Parameters& _parameters)
{
	parameters = _parameters;
	url_dirty = true;
}

// Clear the parameters
void URL::ClearParameters()
{
	parameters.clear();
}

// Returns the URL's path, file name and extension.
String URL::GetPathedFileName() const
{
	String pathed_file_name = path;
	
	// Append the file name.
	pathed_file_name += file_name;
	
	// Append the extension.
	if (!extension.Empty())
	{
		pathed_file_name.Append(".");
		pathed_file_name += extension;
	}
	
	return pathed_file_name;
}

String URL::GetQueryString() const
{
	String query_string;
	
	int count = 0;
	for ( Parameters::const_iterator itr = parameters.begin(); itr != parameters.end(); ++itr )
	{
		query_string += ( count == 0 ) ? "" : "&";
		
		query_string += (*itr).first;
		query_string += "=";
		query_string += (*itr).second;
		
		count++;
	}
	
	return query_string;
}

// Less-than operator for use as a key in STL containers.
bool URL::operator<(const URL& rhs) const
{
	if (url_dirty)
		ConstructURL();
	if (rhs.url_dirty)
		rhs.ConstructURL();
	
	return url < rhs.url;
}

void URL::ConstructURL() const
{
	url = "";

	// Append the protocol.
	if (!protocol.Empty())	
	{
		url = protocol;
		url.Append("://");
	}

	// Append login and password
	if (!login.Empty())
	{
		url.Append( login );
		if (!password.Empty())
		{		
			url.Append( ":" );
			url.Append( password );
		}
		url.Append( "@" );
	}
	ROCKET_ASSERTMSG( password.Empty() || ( !password.Empty() && !login.Empty() ), "Can't have a password without a login!" );

	// Append the host.
	url += host;
	
	// Only check ports if there is some host/protocol part
	if ( !url.Empty() )
	{		
		if (port > 0)
		{
			ROCKET_ASSERTMSG( !host.Empty(), "Can't have a port without a host!" );
			char port_string[16];
			sprintf(port_string, ":%d/", port);
			url.Append(port_string);
		}
		else
		{
			url.Append("/");
		}
	}

	// Append the path.
	if (!path.Empty())
	{
		url += path;
	}

	// Append the file name.
	url += file_name;

	// Append the extension.
	if (!extension.Empty())
	{
		url.Append(".");
		url += extension;
	}
	
	// Append parameters
	if (!parameters.empty())
	{
		url += "?";
		url += GetQueryString();		
	}
	
	url_dirty = false;
}

}
}
