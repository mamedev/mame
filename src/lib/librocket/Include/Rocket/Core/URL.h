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

#ifndef ROCKETCOREURL_H
#define ROCKETCOREURL_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/String.h>
#include <map>

namespace Rocket {
namespace Core {

/**
	@author Peter Curry
 */

class ROCKETCORE_API URL
{
public:
	/// Constructs an empty URL.
	URL();
	/// Constructs a new URL from the given string.
	URL(const String& url);
	/// Constructs a new URL from the given string. A little more scripting
	/// engine friendly.
	URL(const char* url);
	/// Destroys the URL.
	~URL();

	/// Assigns a new URL to the object. This will return false if the URL
	/// is malformed.
	bool SetURL(const String& url);
	/// Returns the entire URL.
	const String& GetURL() const;

	/// Sets the URL's protocol.
	bool SetProtocol(const String& protocol);
	/// Returns the protocol this URL is utilising.
	const String& GetProtocol() const;

	/// Sets the URL's login
	bool SetLogin( const String& login );
	/// Returns the URL's login
	const String& GetLogin() const;

	/// Sets the URL's password
	bool SetPassword( const String& password );
	/// Returns the URL's password
	const String& GetPassword() const;

	/// Sets the URL's host.
	bool SetHost(const String& host);
	/// Returns the URL's host.
	const String& GetHost() const;

	/// Sets the URL's port number.
	bool SetPort(int port);
	/// Returns the URL's port number.
	int GetPort() const;

	/// Sets the URL's path.
	bool SetPath(const String& path);
	/// Prefixes the URL's existing path with the given prefix.
	bool PrefixPath(const String& prefix);
	/// Returns the URL's path.
	const String& GetPath() const;

	/// Sets the URL's file name.
	bool SetFileName(const String& file_name);
	/// Returns the URL's file name.
	const String& GetFileName() const;

	/// Sets the URL's file extension.
	bool SetExtension(const String& extension);
	/// Returns the URL's file extension.
	const String& GetExtension() const;
	
	/// Access the url parameters
	typedef std::map< String, String > Parameters;
	const Parameters& GetParameters() const;
	void SetParameter(const String& name, const String& value);
	void SetParameters( const Parameters& parameters );
	void ClearParameters();
	
	/// Returns the URL's path, file name and extension.
	String GetPathedFileName() const;
	/// Builds and returns a url query string ( key=value&key2=value2 )		
	String GetQueryString() const;

	/// Less-than operator for use as a key in STL containers.
	bool operator<(const URL& rhs) const;

private:
	void ConstructURL() const;

	mutable String url;
	String protocol;
	String login;
	String password;
	String host;		
	String path;
	String file_name;
	String extension;

	Parameters parameters;

	int port;
	mutable int url_dirty;
};

}
}

#endif
