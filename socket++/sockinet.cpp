// sockinet.C  -*- C++ -*- socket library
// Copyright (C) 2002 Herbert Straub
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
// 
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11
// 2002-07-25 Version 1.2 (C) Herbert Straub
// 	Adding improved Error Handling in sockerr class
// 	sockinetaddr::setport if the first character of the port parameter is a 
// 		digit, then the parameter is interpreted as a number
// 2002-07-28 Version 1.2 (C) Herbert Straub
//  Eliminating sorry_about_global_temp inititialisation. This don't work
//  in combination with NewsCache. My idea is: initializing the classes with (0)
//  and in the second step call basic_ios<charT, Traits>::init (basic_sockinetbuf<charT, Traits> *) and basic_iosockstream<charT, Traits>::init ...
//  The constructors of basic_isockinet<charT, Traits>, basic_osockinet<charT, Traits> and basic_iosockinet<charT, Traits> are changed.


#include <socket++/sockinet.h>

#if		defined(OS_UNIX)
#if		defined(OS_MACOS)
typedef int	socklen_t;
#endif
EXTERN_C_BEGIN
#	include <netdb.h>
#	include <sys/time.h>
#	include <sys/socket.h>
#	include <stdlib.h>
#	include <unistd.h>
#	include <errno.h>
#	include <netinet/tcp.h>
EXTERN_C_END
#elif	defined(OS_WIN32)
#	define socklen_t int
#	define EADDRNOTAVAIL				WSAEADDRNOTAVAIL
#	define EADDRINUSE					WSAEADDRINUSE
#	define ENOPROTOOPT					WSAENOPROTOOPT
#endif

void herror(const char*);
using namespace std;
namespace sockstream {
	sockinetaddr::~sockinetaddr() {
	}
	sockinetaddr::sockinetaddr() {
		sin_family = sockinetaddr::af_inet;
		sin_addr.s_addr = htonl(INADDR_ANY);
		sin_port = 0;
	}

	sockinetaddr::sockinetaddr(unsigned long addr, int port_no){
		sin_family = sockinetaddr::af_inet;
		sin_addr.s_addr = htonl(addr);
		sin_port = htons(port_no);
	}

	sockinetaddr::sockinetaddr(unsigned long addr, const char* sn, const char* pn){
		sin_family = sockinetaddr::af_inet;
		sin_addr.s_addr = htonl(addr); // Added by cgay@cs.uoregon.edu May 29, 1993
		setport(sn, pn);
	}

	sockinetaddr::sockinetaddr(const char* host_name, int port_no){
		sin_family = sockinetaddr::af_inet;
		setaddr(host_name);
		sin_port = htons(port_no);
	}

	sockinetaddr::sockinetaddr(const char* hn, const char* sn, const char* pn){
		sin_family = sockinetaddr::af_inet;
		setaddr(hn);
		setport(sn, pn);
	}

	sockinetaddr::sockinetaddr(const sockinetaddr& sina) {
		sin_family = sina.sin_family;
		sin_addr.s_addr = sina.sin_addr.s_addr;
		sin_port = sina.sin_port;
	}   

	sockinetaddr::operator ::sockaddr*() const {
		return (::sockaddr*)(static_cast<const sockaddr_in*>(this));
	}
	::sockaddr*	sockinetaddr::addr() const {
		return (::sockaddr*)(static_cast<const sockaddr_in*>(this));
	}
	void sockinetaddr::setport(const char* sn, const char* pn) {
		if (isdigit(*sn)) {
			sin_port = htons(atoi(sn));
		} else {
			servent*sp	= getservbyname(sn, pn);
			if (sp == 0)
				throw sockerr(EADDRNOTAVAIL, "sockinetaddr::setport");
			sin_port = sp->s_port;
		}
	}

	int sockinetaddr::getport() const {
		return ntohs(sin_port);
	}

	void sockinetaddr::setaddr(const char* host_name) {
		if ((sin_addr.s_addr = inet_addr(host_name)) == INADDR_NONE) {
			hostent*hp	= gethostbyname(host_name);
			if (hp == 0)
				throw sockerr(EADDRNOTAVAIL, "sockinetaddr::setaddr");
			memcpy(&sin_addr, hp->h_addr, hp->h_length);
			sin_family = hp->h_addrtype;
		} else {
			sin_family = sockinetaddr::af_inet;
		}
	}

	const char* sockinetaddr::gethostname() const {
		if (sin_addr.s_addr == htonl(INADDR_ANY)) {
			static char	hostname[64];
			if (::gethostname(hostname, 63) == -1)
				return "";
			return hostname;
		}

		hostent*hp	= gethostbyaddr((const char*) &sin_addr, sizeof(sin_addr), family());
		if (hp == 0) {
			return "";
		}
		if (hp->h_name) {
			return hp->h_name;
		}
		return "";
	}
	sockinetaddr::operator void*() const {
		return addr_in();
	}

	sockaddr_in*sockinetaddr::addr_in() const {
		return (sockaddr_in *) this;
	}
	int sockinetaddr::size() const {
		return sizeof(sockaddr_in);
	}
	int	sockinetaddr::family() const {
		return sin_family;
	}

	template<class charT, class Traits>
	basic_sockinetbuf<charT, Traits>::basic_sockinetbuf<charT, Traits>(const basic_sockinetbuf<charT, Traits>& si) :
		basic_sockbuf<charT, Traits>(si)	{
	}
	template<class charT, class Traits>
	basic_sockinetbuf<charT, Traits>::basic_sockinetbuf<charT, Traits>(const basic_sockbuf<charT, Traits>::socket& sd) :
		basic_sockbuf<charT, Traits>(sd.sock) {
	}

	template<class charT, class Traits>
	basic_sockinetbuf<charT, Traits>::basic_sockinetbuf<charT, Traits>(basic_sockbuf<charT, Traits>::type ty, int proto) :
		basic_sockbuf<charT, Traits>(af_inet, ty, proto)	{
	}
	template<class charT, class Traits>
	basic_sockinetbuf<charT, Traits>::~basic_sockinetbuf<charT, Traits>() {
	}

	template<class charT, class Traits>
	sockinetaddr basic_sockinetbuf<charT, Traits>::localaddr() const {
		sockinetaddr	sin;
		int				len	= sin.size();
		if (::getsockname(rep->sock, sin.addr(), (socklen_t *) // LN
			& len) == -1) {
			throw sockerr(errno, "basic_sockinetbuf<charT, Traits>::localaddr");
		}
		return sin;
	}

	template<class charT, class Traits>
	int	basic_sockinetbuf<charT, Traits>::localport() const {
		sockinetaddr	sin	= localaddr();
		if (sin.family() != af_inet) {
			return -1;
		}
		return sin.getport();
	}

	template<class charT, class Traits>
	const char*	basic_sockinetbuf<charT, Traits>::localhost() const {
		sockinetaddr	sin	= localaddr();
		if (sin.family() != af_inet) {
			return "";
		}
		return sin.gethostname();
	}

	template<class charT, class Traits>
	sockinetaddr basic_sockinetbuf<charT, Traits>::peeraddr() const {
		sockinetaddr	sin;
		int				len	= sin.size();
		if (::getpeername(rep->sock, sin.addr(), (socklen_t *) // LN
			& len) == -1) {
			throw sockerr(errno, "basic_sockinetbuf<charT, Traits>::peeraddr");
		}
		return sin;
	}

	template<class charT, class Traits>
	int	basic_sockinetbuf<charT, Traits>::peerport() const {
		sockinetaddr	sin	= peeraddr();
		if (sin.family() != af_inet) {
			return -1;
		}
		return sin.getport();
	}

	template<class charT, class Traits>
	const char*	basic_sockinetbuf<charT, Traits>::peerhost() const {
		sockinetaddr	sin	= peeraddr();
		if (sin.family() != af_inet) {
			return "";
		}
		return sin.gethostname();
	}

	template<class charT, class Traits>
	void basic_sockinetbuf<charT, Traits>::bind_until_success(int portno)
		// a. bind to (INADDR_ANY, portno)
		// b. if success return
		// c. if failure and errno is EADDRINUSE, portno++ and go to step a.
	{
		for (; ;) {
			try {
				bind(portno++);
			} catch (sockerr e) {
				//  	if (e.errno () != EADDRINUSE) throw;
				if (e.serrno() != EADDRINUSE)
					throw; // LN
				continue;
			}
			break;
		}
	}

	template<class charT, class Traits>
	void basic_sockinetbuf<charT, Traits>::bind(sockaddr& sa) {
		basic_sockbuf<charT, Traits>::bind(sa);
	}

	template<class charT, class Traits>
	void basic_sockinetbuf<charT, Traits>::bind(int port_no) {
		sockinetaddr	sa	((long unsigned int) // LN
						INADDR_ANY, port_no);
		sockaddr_in*p	= (sockaddr_in*)&sa;
		bind(sa);
	}

	template<class charT, class Traits>
	void basic_sockinetbuf<charT, Traits>::bind(unsigned long addr, int port_no)
		// address and portno are in host byte order
	{
		sockinetaddr	sa	(addr, port_no);
		bind(sa);
	}

	template<class charT, class Traits>
	void basic_sockinetbuf<charT, Traits>::bind(const char* host_name, int port_no) {
		sockinetaddr	sa	(host_name, port_no);
		bind(sa);
	}

	template<class charT, class Traits>
	void basic_sockinetbuf<charT, Traits>::bind(unsigned long addr, const char* service_name, const char* protocol_name) {
		sockinetaddr	sa	(addr, service_name, protocol_name);
		bind(sa);
	}

	template<class charT, class Traits>
	void basic_sockinetbuf<charT, Traits>::bind(const char* host_name, const char* service_name, const char* protocol_name) {
		sockinetaddr	sa	(host_name, service_name, protocol_name);
		bind(sa);
	}

	template<class charT, class Traits>
	void basic_sockinetbuf<charT, Traits>::connect(sockaddr& sa) {
		basic_sockbuf<charT, Traits>::connect(sa);
	}

	template<class charT, class Traits>
	void basic_sockinetbuf<charT, Traits>::connect(unsigned long addr, int port_no) {
		// address and portno are in host byte order
		sockinetaddr	sa	(addr, port_no);
		connect(sa);
	}

	template<class charT, class Traits>
	void basic_sockinetbuf<charT, Traits>::connect(const char* host_name, int port_no) {
		sockinetaddr	sa	(host_name, port_no);
		connect(sa);
	}

	template<class charT, class Traits>
	void basic_sockinetbuf<charT, Traits>::connect(unsigned long addr, const char* service_name, const char* protocol_name) {
		sockinetaddr	sa	(addr, service_name, protocol_name);
		connect(sa);
	}

	template<class charT, class Traits>
	void basic_sockinetbuf<charT, Traits>::connect(const char* host_name, const char* service_name, const char* protocol_name) {
		sockinetaddr	sa	(host_name, service_name, protocol_name);
		connect(sa);
	}

	template<class charT, class Traits>
	bool
	basic_sockinetbuf<charT, Traits>::open(sockaddr& sa) {
		basic_sockbuf<charT, Traits>::connect(sa);
		return true;
	}

	template<class charT, class Traits>
	bool
	basic_sockinetbuf<charT, Traits>::open(unsigned long addr, int port_no) {
		// address and portno are in host byte order
		sockinetaddr	sa	(addr, port_no);
		connect(sa);
		return true;
	}

	template<class charT, class Traits>
	bool
	basic_sockinetbuf<charT, Traits>::open(const char* host_name, int port_no) {
		sockinetaddr	sa	(host_name, port_no);
		connect(sa);
		return true;
	}

	template<class charT, class Traits>
	bool
	basic_sockinetbuf<charT, Traits>::open(unsigned long addr, const char* service_name, const char* protocol_name) {
		sockinetaddr	sa	(addr, service_name, protocol_name);
		connect(sa);
		return true;
	}

	template<class charT, class Traits>
	bool
	basic_sockinetbuf<charT, Traits>::open(const char* host_name, const char* service_name, const char* protocol_name) {
		sockinetaddr	sa	(host_name, service_name, protocol_name);
		connect(sa);
		return true;
	}
	template<class charT, class Traits>
	bool
	basic_sockinetbuf<charT, Traits>::close() {
		return true;
	}

	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>::socket
	basic_sockinetbuf<charT, Traits>::accept() {
		return basic_sockbuf<charT, Traits>::accept();
	}

	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>::socket
	basic_sockinetbuf<charT, Traits>::accept(sockaddr& sa) {
		return basic_sockbuf<charT, Traits>::accept(sa);
	}

	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>::socket
	basic_sockinetbuf<charT, Traits>::accept(unsigned long addr, int port_no) {
		sockinetaddr	sa	(addr, port_no);
		return accept(sa);
	}

	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>::socket
	basic_sockinetbuf<charT, Traits>::accept(const char* host_name, int port_no) {
		sockinetaddr	sa	(host_name, port_no);
		return accept(sa);
	}

	template<class charT, class Traits>
	bool
	basic_sockinetbuf<charT, Traits>::tcpnodelay() const {
		struct protoent*proto	= getprotobyname("tcp");
		if (proto == 0) {
			throw sockerr(ENOPROTOOPT, "basic_sockinetbuf<charT, Traits>::tcpnodelay");
		}

		int	old	= 0;
		getopt(TCP_NODELAY, &old, sizeof(old), proto->p_proto);
		return old != 0;
	}

	template<class charT, class Traits>
	bool
	basic_sockinetbuf<charT, Traits>::tcpnodelay(bool set) const {
		struct protoent*proto	= getprotobyname("tcp");
		if (proto == 0) {
			throw sockerr(ENOPROTOOPT, "basic_sockinetbuf<charT, Traits>::tcpnodelay");
		}

		int	old	= 0;
		int	opt	= set;
		getopt(TCP_NODELAY, &old, sizeof(old), proto->p_proto);
		setopt(TCP_NODELAY, &opt, sizeof(opt), proto->p_proto);
		return old != 0;
	}

	int sockinit::count	= 0;
	sockinit::sockinit() {
		int rc;
        if (count++ == 0) {
#if		defined(OS_WIN32)
			WSADATA wsaData;
			rc = ::WSAStartup(WINSOCK_VERSION, &wsaData);
			if  (rc == SOCKET_ERROR) {
				rc = WSAGetLastError();
			}
#endif
		}
	}
	sockinit::~sockinit() {
		int rc;
#if		defined(OS_WIN32)
		if (--count == 0) {
			rc = ::WSACleanup();
			if  (rc == SOCKET_ERROR) {
				rc = WSAGetLastError();
			}
		}
#endif
	}
	template<class charT, class Traits>
	basic_isockinet<charT, Traits>::basic_isockinet(typename const basic_sockbuf<charT, Traits>::socket& sd)
	{
		basic_sockinetbuf<charT, Traits>*	t	= new basic_sockinetbuf<charT, Traits>(sd);

		basic_ios<charT, Traits>::init(t);
		basic_isockstream<charT, Traits>::init(t);
	}

	template<class charT, class Traits>
	basic_isockinet<charT, Traits>::basic_isockinet(typename basic_sockbuf<charT, Traits>::type ty, int proto)
	{
		basic_sockinetbuf<charT, Traits>*	t	= new basic_sockinetbuf<charT, Traits>(ty, proto);

		basic_ios<charT, Traits>::init(t);
		basic_isockstream<charT, Traits>::init(t);
	}

	template<class charT, class Traits>
	basic_isockinet<charT, Traits>::basic_isockinet(typename const basic_sockinetbuf<charT, Traits>& sb)
	{
		basic_sockinetbuf<charT, Traits>*	t	= new basic_sockinetbuf<charT, Traits>(sb);

		basic_ios<charT, Traits>::init(t);
		basic_isockstream<charT, Traits>::init(t);
	}

	template<class charT, class Traits>
	basic_isockinet<charT, Traits>::~basic_isockinet<charT, Traits>() {
		delete basic_ios<charT, Traits>::rdbuf();
	}

	template<class charT, class Traits>
	basic_osockinet<charT, Traits>::basic_osockinet(typename const basic_sockbuf<charT, Traits>::socket& sd)
	{
		basic_sockinetbuf<charT, Traits>*	t	= new basic_sockinetbuf<charT, Traits>(sd);

		basic_ios<charT, Traits>::init(t);
		basic_osockstream<charT, Traits>::init(t);
	}

	template<class charT, class Traits>
	basic_osockinet<charT, Traits>::basic_osockinet(typename basic_sockbuf<charT, Traits>::type ty, int proto)
	{
		basic_sockinetbuf<charT, Traits>*	t	= new basic_sockinetbuf<charT, Traits>(ty, proto);

		basic_ios<charT, Traits>::init(t);
		basic_osockstream<charT, Traits>::init(t);
	}

	template<class charT, class Traits>
	basic_osockinet<charT, Traits>::basic_osockinet(typename const basic_sockinetbuf<charT, Traits>& sb) 
	{
		basic_sockinetbuf<charT, Traits>*	t	= new basic_sockinetbuf<charT, Traits>(sb);

		basic_ios<charT, Traits>::init(t);
		basic_osockstream<charT, Traits>::init(t);
	}

	template<class charT, class Traits>
	basic_osockinet<charT, Traits>::~basic_osockinet<charT, Traits>() {
		delete basic_ios<charT, Traits>::rdbuf();
	}

	template<class charT, class Traits>
	basic_iosockinet<charT, Traits>::basic_iosockinet(typename const basic_sockbuf<charT, Traits>::socket& sd)
	{
		basic_sockinetbuf<charT, Traits>*	t	= new basic_sockinetbuf<charT, Traits>(sd);

		basic_ios<charT, Traits>::init(t);
		basic_iosockstream<charT, Traits>::init(t);
	}

	template<class charT, class Traits>
	basic_iosockinet<charT, Traits>::basic_iosockinet(typename basic_sockbuf<charT, Traits>::type ty, int proto)
	{
		basic_sockinetbuf<charT, Traits>*	t	= new basic_sockinetbuf<charT, Traits>(ty, proto);

		basic_ios<charT, Traits>::init(t);
		basic_iosockstream<charT, Traits>::init(t);
	}

	template<class charT, class Traits>
	basic_iosockinet<charT, Traits>::basic_iosockinet(typename const basic_sockinetbuf<charT, Traits>& sb)
	{
		basic_sockinetbuf<charT, Traits>*	t	= new basic_sockinetbuf<charT, Traits>(sb);

		basic_ios<charT, Traits>::init(t);
		basic_iosockstream<charT, Traits>::init(t);
	}

	template<class charT, class Traits>
	basic_iosockinet<charT, Traits>::~basic_iosockinet() {
		delete basic_ios<charT, Traits>::rdbuf();
	}

	template SOCKSTREAM_API basic_sockinetbuf<char, char_traits<char> >;
	template SOCKSTREAM_API basic_isockinet<char, char_traits<char> >;
	template SOCKSTREAM_API basic_osockinet<char, char_traits<char> >;
	template SOCKSTREAM_API basic_iosockinet<char, char_traits<char> >;

	template SOCKSTREAM_API basic_sockinetbuf<wchar_t, char_traits<wchar_t> >;
	template SOCKSTREAM_API basic_isockinet<wchar_t, char_traits<wchar_t> >;
	template SOCKSTREAM_API basic_osockinet<wchar_t, char_traits<wchar_t> >;
	template SOCKSTREAM_API basic_iosockinet<wchar_t, char_traits<wchar_t> >;


}