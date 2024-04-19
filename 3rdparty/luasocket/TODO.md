## FIX

http was preserving old host header during redirects
fix smtp.send hang on source error
add create field to FTP and SMTP and fix HTTP ugliness
clean timeout argument to open functions in SMTP, HTTP and FTP
eliminate globals from namespaces created by module().
url.absolute was not working when base_url was already parsed
http.request was redirecting even when the location header was empty
tcp{client}:shutdown() was checking for group instead of class.
tcp{client}:send() now returns i+sent-1...
get rid of a = socket.try() in the manual, except for protected cases. replace it with assert.
get rid of "base." kludge in package.loaded
check all "require("http")" etc in the manual.
make sure sock_gethostname.*  only return success if the hp is not null!
change 'l' prefix in C libraries to 'c' to avoid clash with LHF libraries
    don't forget the declarations in luasocket.h and mime.h!!!
setpeername was using udp{unconnected}
fixed a bug in http.lua that caused some requests to fail (Florian Berger)
fixed a bug in select.c that prevented sockets with descriptor 0 from working (Renato Maia)
fixed a "bug" that caused dns.toip to crash under uLinux
fixed a "bug" that caused a crash in gethostbyname under VMS
DEBUG and VERSION became _DEBUG and _VERSION
send returns the right value if input is "". Alexander Marinov


## WISH

... as an l-value to get all results of a function call?
at least ...[i] and #...
extend to full tuples?

__and __or __not metamethods

lua_tostring, lua_tonumber, lua_touseradta etc push values in stack
__tostring,__tonumber, __touserdata metamethods are checked
and expected to push an object of correct type on stack

lua_rawtostring, lua_rawtonumber, lua_rawtouserdata don't
push anything on stack, return data of appropriate type,
skip metamethods and throw error if object not of exact type

package.findfile exported
module not polluting the global namespace

coxpcall with a coroutine pool for efficiency (reusing coroutines)

exception mechanism formalized? just like the package system was.

a nice bitlib in the core


## TODO

- bizarre default values for getnameinfo should throw error instead!

> It's just too bad it can't talk to gmail -
> reason 1: they absolutely want TLS
> reason 2: unlike all the other SMTP implementations, they
> don't
> tolerate missing < > around adresses

- document the new bind and connect behavior.
- shouldn't we instead make the code compatible to Lua 5.2
  without any compat stuff, and use a compatibility layer to
  make it work on 5.1?
- add what's new to manual
- should there be an equivalent to tohostname for IPv6?
- should we add service name resolution as well to getaddrinfo?
- Maybe the sockaddr to presentation conversion should be done with getnameinfo()?

- add http POST sample to manual
  people keep asking stupid questions
- documentation of dirty/getfd/setfd is problematic because of portability
  same for unix and serial.
  what to do about this? add a stronger disclaimer?
- fix makefile with decent defaults?

## Done:

- added IPv6 support to getsockname
- simplified getpeername implementation
- added family to return of getsockname and getpeername
  and added modification to the manual to describe

- connect and bind try all adresses returned by getaddrinfo
- document headers.lua?
- update copyright date everywhere?
- remove RCSID from files?
- move version to 2.1 rather than 2.1.1?
- fixed url package to support ipv6 hosts
- changed domain to family
- implement getfamily methods.

- remove references to Lua 5.0 from documentation, add 5.2?
- update lua and luasocket version in samples in documentation
- document ipv5_v6only default option being set?
- document tcp6 and udp6
- document dns.getaddrinfo
- documented zero-sized datagram change?
  no.
- document unix socket and serial socket? add raw support?
  no.
- document getoption
- merge luaL_typeerror into auxiliar to avoid using luaL prefix?










replace \r\n with \0xD\0xA in everything
New mime support

ftp send should return server replies?
make sure there are no object files in the distribution tarball
http handling of 100-continue, see DB patch
DB ftp.lua bug.
test unix.c to return just a function and works with require"unix"
get rid of setmetatable(, nil) since packages don't need this anymore in 5.1
compat-5.1 novo
ajeitar pra lua-5.1

adicionar exemplos de expans�o: pipe, local, named pipe
testar os options!


- Thread-unsafe functions to protect
    gethostbyname(), gethostbyaddr(), gethostent(),
inet_ntoa(), strerror(),

