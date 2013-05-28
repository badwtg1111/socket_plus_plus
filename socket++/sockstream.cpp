// sockstream.C -*- C++ -*- socket library
// Copyright (C) 2002 Herbert Straub for my changes, see ChangeLog.
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
//
// You can simultaneously read and write into
// a sockbuf just like you can listen and talk
// through a telephone. Hence, the read and the
// write buffers are different. That is, they do not
// share the same memory.
// 
// Read:
// gptr() points to the start of the get area.
// The unread chars are gptr() - egptr().
// base() points to the read buffer
// 
// eback() is set to base() so that pbackfail()
// is called only when there is no place to
// putback a char. And pbackfail() always returns EOF.
// 
// Write:
// pptr() points to the start of the put area
// The unflushed chars are pbase() - pptr()
// pbase() points to the write buffer.
// epptr() points to the end of the write buffer.
// 
// Output is flushed whenever one of the following conditions
// holds:
// (1) pptr() == epptr()
// (2) EOF is written
// (3) linebuffered and '\n' is written
// 
// Unbuffered:
// Input buffer size is assumed to be of size 1 and output
// buffer is of size 0. That is, egptr() <= base()+1 and
// epptr() == pbase().
//
// Version: 1.2 2002-07-25 Herbert Straub 
// 	Improved Error Handling - extending the sockerr class by cOperation


#include <socket++/sockstream.h>
#ifdef HAVE_SSTREAM
#include <sstream>
#else
#include <strstream>
#endif
#include <string>
#if defined(__APPLE__)
#typedef int socklen_t;
#endif

#if		defined(OS_UNIX)
EXTERN_C_BEGIN
#	include <sys/time.h>
#	include <sys/socket.h>
#	include <sys/ioctl.h>
#	include <unistd.h>
#	include <errno.h>
EXTERN_C_END
#elif	defined(OS_WIN32)
#	include <io.h>
#	define socklen_t int

#	define EWOULDBLOCK			WSAEWOULDBLOCK
#	define EINPROGRESS			WSAEINPROGRESS
#	define EALREADY				WSAEALREADY
#	define ENOTSOCK				WSAENOTSOCK
#	define EDESTADDRREQ			WSAEDESTADDRREQ
#	define EMSGSIZE				WSAEMSGSIZE
#	define EPROTOTYPE			WSAEPROTOTYPE
#	define ENOPROTOOPT			WSAENOPROTOOPT
#	define EPROTONOSUPPORT		WSAEPROTONOSUPPORT
#	define ESOCKTNOSUPPORT		WSAESOCKTNOSUPPORT
#	define EOPNOTSUPP			WSAEOPNOTSUPP
#	define EPFNOSUPPORT			WSAEPFNOSUPPORT
#	define EAFNOSUPPORT			WSAEAFNOSUPPORT
#	define EADDRINUSE			WSAEADDRINUSE
#	define EADDRNOTAVAIL		WSAEADDRNOTAVAIL
#	define ENETDOWN				WSAENETDOWN
#	define ENETUNREACH			WSAENETUNREACH
#	define ENETRESET			WSAENETRESET
#	define ECONNABORTED			WSAECONNABORTED
#	define ECONNRESET			WSAECONNRESET
#	define ENOBUFS				WSAENOBUFS
#	define EISCONN				WSAEISCONN
#	define ENOTCONN				WSAENOTCONN
#	define ESHUTDOWN			WSAESHUTDOWN
#	define ETOOMANYREFS			WSAETOOMANYREFS
#	define ETIMEDOUT			WSAETIMEDOUT
#	define ECONNREFUSED			WSAECONNREFUSED
#	define ELOOP				WSAELOOP
#	define EHOSTDOWN			WSAEHOSTDOWN
#	define EHOSTUNREACH			WSAEHOSTUNREACH
#	define EPROCLIM				WSAEPROCLIM
#	define EUSERS				WSAEUSERS
#	define EDQUOT				WSAEDQUOT
#	define EISCONN				WSAEISCONN
#	define ENOTCONN				WSAENOTCONN
#	define ECONNRESET			WSAECONNRESET
#	define ECONNREFUSED			WSAECONNREFUSED
#	define ETIMEDOUT			WSAETIMEDOUT
#	define EADDRINUSE			WSAEADDRINUSE
#	define EADDRNOTAVAIL		WSAEADDRNOTAVAIL
#	define EWOULDBLOCK			WSAEWOULDBLOCK
#endif

#ifndef BUFSIZ
#  define BUFSIZ 1024
#endif

#if		defined(OS_UNIX)
#ifdef FD_ZERO
#  undef FD_ZERO	// bzero causes so much trouble to us
#endif
#define FD_ZERO(p) (memset ((p), 0, sizeof *(p)))
#endif

using namespace std;
namespace sockstream {
	sockaddr::sockaddr() {
	}
	sockaddr::~sockaddr() {
	}
	template<class charT, class Traits>
	basic_sockbuf<charT, Traits>::basic_sockbuf(
		typename const basic_sockbuf<charT, Traits>::socket& sd
	) {
		rep = new basic_sockbuf<charT, Traits>::sockcnt(sd.sock);
		char_type*	gbuf	= new char_type[BUFSIZ];
		char_type*	pbuf	= new char_type[BUFSIZ];
		setg(gbuf, gbuf + BUFSIZ, gbuf + BUFSIZ);
		setp(pbuf, pbuf + BUFSIZ);
		rep->gend = gbuf + BUFSIZ;
		rep->pend = pbuf + BUFSIZ;
	}	   

	template<class charT, class Traits>
	basic_sockbuf<charT, Traits>::basic_sockbuf(
		int domain, typename basic_sockbuf<charT, Traits>::type st, int proto) :
		rep(0)
	{
		SOCKET	soc	= ::socket(domain, st, proto);

		if (soc == SOCKET_ERROR)
#ifndef OS_WIN32
			throw sockerr(errno, "sockbuf::sockbuf");
#else
			throw sockerr(WSAGetLastError(), "sockbuf::sockbuf");
#endif

		rep = new basic_sockbuf<charT, Traits>::sockcnt(soc);

		charT*	gbuf	= new charT[BUFSIZ];
		charT*	pbuf	= new charT[BUFSIZ];
		setg(gbuf, gbuf + BUFSIZ, gbuf + BUFSIZ);
		setp(pbuf, pbuf + BUFSIZ);
		rep->gend = gbuf + BUFSIZ;
		rep->pend = pbuf + BUFSIZ;
	}

	template<class charT, class Traits>
	basic_sockbuf<charT, Traits>::basic_sockbuf(
		const basic_sockbuf<charT, Traits>& sb) :
		//streambuf (sb),
		rep(sb.rep)
	{
		// the streambuf::streambuf (const streambuf&) is assumed
		// to haved handled pbase () and gbase () correctly.

		rep->cnt++;
	}

	/*sockbuf& sockbuf::operator = (const sockbuf& sb)
	{
	if (this != &sb && rep != sb.rep && rep->sock != sb.rep->sock) {
		streambuf::operator = (sb);
		this->sockbuf::~sockbuf();

		// the streambuf::operator = (const streambuf&) is assumed
		// to have handled pbase () and gbase () correctly.
		rep  = sb.rep;
		rep->cnt++;
	}
	return *this;
	}*/

	template<class charT, class Traits>
	basic_sockbuf<charT, Traits>::~basic_sockbuf() {
		overflow(eof); // flush write buffer
		if (--rep->cnt == 0) {
			delete[] pbase();
			delete[] eback();
#ifndef OS_WIN32
			int	c	= close(rep->sock);
#else
			int	c	= closesocket(rep->sock);
#endif
			delete rep;
			int rc;
			if (c == SOCKET_ERROR) {
#ifndef OS_WIN32
				throw sockerr(errno, "sockbuf::~sockbuf", sockname.c_str());
#else
				rc = WSAGetLastError();
				if  (rc != WSANOTINITIALISED) {//FIXME 
					throw sockerr(WSAGetLastError(), "sockbuf::~sockbuf", sockname.c_str());
				}
#endif
			}
		}
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::is_open() const {
		// if socket is still connected to the peer, return true
		// else return false
		return false;
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::sync() {
		// we never return -1 because we throw sockerr
		// exception in the event of an error.
		if (pptr() && pbase() < pptr() && pptr() <= epptr()) {
			// we have some data to flush
			try {
				write(pbase(), static_cast<int>(pptr() - pbase()));
			} catch (int wlen) {
				// write was not completely successful
#ifdef HAVE_SSTREAM
				stringstream	sb;
#else
				strstream		sb;
#endif
				string			err	("sockbuf::sync");
				err += "(" + sockname + ")";
				if (wlen) {
					// reposition unwritten chars
					charT*	pto		= pbase();
					charT*	pfrom	= pbase() + wlen;
					int		len		= static_cast<int>(pptr() - pbase() - wlen);
					while (pfrom < pptr())
						*pto++ = *pfrom++;
					setp(pbase(), (charT *) rep->pend);
					pbump(len);
					sb << " wlen=(" << wlen << ")";
					err += sb.rdbuf()->str();
				}
				throw sockerr(errno, err.c_str());
			}

			setp(pbase(), (charT *) rep->pend);
		}

		// we cannot restore input data back to the socket stream
		// thus we do not do anything on the input stream

		return 0;
	}

	template<class charT, class Traits>
	int	basic_sockbuf<charT, Traits>::showmanyc() const {
		// return the number of chars in the input sequence
		if (gptr() && gptr() < egptr()) {
			return static_cast<int>(egptr() - gptr());
		}
		return 0;
	}

	template<class charT, class Traits>
	typename Traits::int_type
	basic_sockbuf<charT, Traits>::underflow() {
		if (gptr() == 0) {
			return eof;
		} // input stream has been disabled

		if (gptr() < egptr()) {
			return (unsigned char) * gptr();
		} // eof is a -ve number; make it
		// unsigned to be diff from eof

		int	rlen	= read(eback(), static_cast<int>((char*) rep->gend - (char*) eback()));

		if (rlen == 0) {
			return eof;
		}

		setg(eback(), eback(), eback() + rlen);
		return (unsigned char) * gptr();
	}

	template<class charT, class Traits>
	typename Traits::int_type
	basic_sockbuf<charT, Traits>::uflow() {
		int_type	ret	= underflow();
		if (ret == eof) {
			return eof;
		}

		gbump(1);
		return ret;
	}

	template<class charT, class Traits>
	streamsize
	basic_sockbuf<charT, Traits>::xsgetn(char_type* s, streamsize n) {
		int	rval	= showmanyc();
		if (rval >= n) {
			memcpy(s, gptr(), n * sizeof(char_type));
			gbump(n);
			return n;
		}

		memcpy(s, gptr(), rval * sizeof(char_type));
		gbump(rval);

		if (underflow() != eof) {
			return rval + xsgetn(s + rval, n - rval);
		}

		return rval;
	}

	template<class charT, class Traits>
	typename Traits::int_type
	basic_sockbuf<charT, Traits>::pbackfail(typename Traits::int_type c) {
		return eof;
	}

	template<class charT, class Traits>
	typename Traits::int_type
	basic_sockbuf<charT, Traits>::overflow(typename Traits::int_type c)
		// if pbase () == 0, no write is allowed and thus return eof.
		// if c == eof, we sync the output and return 0.
		// if pptr () == epptr (), buffer is full and thus sync the output,
		//  					   insert c into buffer, and return c.
		// In all cases, if error happens, throw exception.
	{
		if (pbase() == 0) {
			return eof;
		}

		if (c == eof) {
			return sync();
		}

		if (pptr() == epptr()) {
			sync();
		}
		*pptr() = (char_type) c;
		pbump(1);
		return c;
	}

	template<class charT, class Traits>
	streamsize
	basic_sockbuf<charT, Traits>::xsputn(const char_type* s, streamsize n) {
		int	wval	= static_cast<int>(epptr() - pptr());
		if (n <= wval) {
			memcpy(pptr(), s, n * sizeof(char_type));
			pbump(n);
			return n;
		}

		memcpy(pptr(), s, wval * sizeof(char_type));
		pbump(wval);

		if (overflow() != eof) {
			return wval + xsputn(s + wval, n - wval);
		}

		return wval;
	}

	template<class charT, class Traits>
	void
	basic_sockbuf<charT, Traits>::bind(const sockaddr& sa) {
		if (::bind(rep->sock, sa.addr(), sa.size()) == -1) {
			throw sockerr(errno, "sockbuf::bind", sockname.c_str());
		}
	}

	template<class charT, class Traits>
	void
	basic_sockbuf<charT, Traits>::connect(const sockaddr& sa) {
		if (::connect(rep->sock, sa.addr(), sa.size()) == -1) {
			throw sockerr(errno, "sockbuf::connect", sockname.c_str());
		}
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::open(const sockaddr& sa) {
		if (::connect(rep->sock, sa.addr(), sa.size()) == -1) {
			return false;
			throw sockerr(errno, "sockbuf::connect", sockname.c_str());
		}
		return true;
	}
	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::close() {
		return true;
	}
	template<class charT, class Traits>
	void
	basic_sockbuf<charT, Traits>::listen(int num) {
		if (::listen(rep->sock, num) == -1) {
			throw sockerr(errno, "sockbuf::listen", sockname.c_str());
		}
	}

	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>::socket
	basic_sockbuf<charT, Traits>::accept(sockaddr& sa) {
		int	len	= sa.size();
		SOCKET	soc	= -1;
		if ((soc = ::accept(rep->sock, sa.addr(), (socklen_t *) // LN
	& len)) == -1) {
			throw sockerr(errno, "sockbuf::socket", sockname.c_str());
		}
		return socket(soc);
	}

	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>::socket
	basic_sockbuf<charT, Traits>::accept() {
		SOCKET	soc	= -1;
		if ((soc = ::accept(rep->sock, 0, 0)) == -1) {
			throw sockerr(errno, "sockbuf::socket", sockname.c_str());
		}
		return socket(soc);
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::read(void* buf, int len) {
		if (rep->rtmo != -1 && is_readready(rep->rtmo) == 0) {
			throw sockerr(ETIMEDOUT, "sockbuf::read", sockname.c_str());
		}

		if (rep->oob && atmark()) {
			throw sockoob();
		}

		int	rval	= 0;
		if ((rval = ::read(static_cast<int>(rep->sock), buf, len)) == -1) {
			throw sockerr(errno, "sockbuf::read", sockname.c_str());
		}
		return rval;
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::recv(void* buf, int len, int msgf) {
		if (rep->rtmo != -1 && is_readready(rep->rtmo) == 0) {
			throw sockerr(ETIMEDOUT, "sockbuf::recv", sockname.c_str());
		}

		if (rep->oob && atmark()) {
			throw sockoob();
		}

		int	rval	= 0;
		if ((rval = ::recv(rep->sock, (char *) buf, len, msgf)) == -1) {
			throw sockerr(errno, "sockbuf::recv", sockname.c_str());
		}
		return rval;
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::recv(string& buf, int msgf) {
		if (rep->rtmo != -1 && is_readready(rep->rtmo) == 0) {
			throw sockerr(ETIMEDOUT, "sockbuf::recv", sockname.c_str());
		}

		if (rep->oob && atmark()) {
			throw sockoob();
		}

		int	rval	= 0;
		unsigned long size;
		rval = ::recv(rep->sock, reinterpret_cast<char*>(&size), sizeof(unsigned long), msgf);
		if  (rval == 0)
			return rval;
        if  (rval != sizeof(unsigned long))
			throw sockerr(errno, "sockbuf::recv", sockname.c_str());

		size = ntohl(size);
		buf.resize(size);
		rval = ::recv(rep->sock, const_cast<char*>(buf.data()), size, msgf);
		return rval;
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::recv(wstring& buf, int msgf) {
		if (rep->rtmo != -1 && is_readready(rep->rtmo) == 0) {
			throw sockerr(ETIMEDOUT, "sockbuf::recv", sockname.c_str());
		}

		if (rep->oob && atmark()) {
			throw sockoob();
		}

		int	rval	= 0;
		unsigned long size;
		rval = ::recv(rep->sock, reinterpret_cast<char*>(&size), sizeof(unsigned long), msgf);
		if  (rval == 0)
			return rval;
        if  (rval != sizeof(unsigned long))
			throw sockerr(errno, "sockbuf::recv", sockname.c_str());

		size = ntohl(size);
		buf.resize(size / sizeof(wchar_t));
		rval = ::recv(
			rep->sock,
			const_cast<char*>(reinterpret_cast<const char*>(buf.data())),
			size, msgf);
		return rval;
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::recvfrom(sockaddr& sa, void* buf, int len, int msgf) {
		if (rep->rtmo != -1 && is_readready(rep->rtmo) == 0) {
			throw sockerr(ETIMEDOUT, "sockbuf::recvfrom", sockname.c_str());
		}

		if (rep->oob && atmark()) {
			throw sockoob();
		}

		int	rval	= 0;
		int	sa_len	= sa.size();

		if ((rval = ::recvfrom(rep->sock, (char *) buf, len, msgf, sa.addr(), (socklen_t *) // LN
	& sa_len)) == -1) {
			throw sockerr(errno, "sockbuf::recvfrom", sockname.c_str());
		}
		return rval;
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::write(const void* buf, int len)
		// upon error, write throws the number of bytes writen so far instead
		// of sockerr.
	{
		char*	pbuf	= (char*) buf;
		if (rep->stmo != -1 && is_writeready(rep->stmo) == 0) {
			throw sockerr(ETIMEDOUT, "sockbuf::write", sockname.c_str());
		}

		int	wlen	= 0;
		while (len > 0) {
			int	wval	= ::write(static_cast<int>(rep->sock), pbuf + wlen, len);
			if (wval == -1)
				throw wlen;
			len -= wval;
			wlen += wval;
		}
		return wlen; // == len if every thing is all right
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::send(const void* buf, int len, int msgf)
		// upon error, write throws the number of bytes writen so far instead
		// of sockerr.
	{
		char*	pbuf	= (char*) buf;
		if (rep->stmo != -1 && is_writeready(rep->stmo) == 0) {
			throw sockerr(ETIMEDOUT, "sockbuf::send", sockname.c_str());
		}

		int	wlen	= 0;
		while (len > 0) {
			int	wval	= ::send(rep->sock, pbuf + wlen, len, msgf);
			if (wval == -1)
				throw wlen;
			len -= wval;
			wlen += wval;
		}
		return wlen;
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::send(const std::string& buf, int msgf)
		// upon error, write throws the number of bytes writen so far instead
		// of sockerr.
	{
		if (rep->stmo != -1 && is_writeready(rep->stmo) == 0) {
			throw sockerr(ETIMEDOUT, "sockbuf::send", sockname.c_str());
		}

		//
		// send string::soze() on a 2 bytes unsigned integer
		//
		int	wlen		= 0;
		unsigned long	size = htonl(buf.size());	//FIXME use a portable nat4 type
		int	wval		= ::send(
			rep->sock,
			reinterpret_cast<const char*>(&size),
			sizeof(unsigned long),
			msgf
		);
		if (wval == -1)
			throw wlen;


		//
		// send string::data()
		//
		int len = buf.size();
		char*	pbuf	= (char*) buf.data();
		while (len > 0) {
			wval	= ::send(rep->sock, pbuf + wlen, len, msgf);
			if (wval == -1)
				throw wlen;
			len -= wval;
			wlen += wval;
		}
		return wlen;
	}
	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::send(const std::wstring& buf, int msgf)
		// upon error, write throws the number of bytes writen so far instead
		// of sockerr.
	{
		if (rep->stmo != -1 && is_writeready(rep->stmo) == 0) {
			throw sockerr(ETIMEDOUT, "sockbuf::send", sockname.c_str());
		}

		//
		// send string::soze() on a 2 bytes unsigned integer
		//
		int	wlen		= 0;
		int len			= buf.size() * sizeof(wchar_t);
		unsigned long	size = htonl(len);	//FIXME use a portable nat4 type
		int	wval		= ::send(
			rep->sock,
			reinterpret_cast<const char*>(&size),
			sizeof(unsigned long),
			msgf
		);
		if (wval == -1)
			throw wlen;


		//
		// send string::data()
		//
		char*	pbuf	= (char*) buf.data();
		while (len > 0) {
			wval	= ::send(rep->sock, pbuf + wlen, len, msgf);
			if (wval == -1)
				throw wlen;
			len -= wval;
			wlen += wval;
		}
		return wlen;
	}
	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::sendto(sockaddr& sa, const void* buf, int len, int msgf)
		// upon error, write throws the number of bytes writen so far instead
		// of sockerr.
	{
		char*	pbuf	= (char*) buf;
		if (rep->stmo != -1 && is_writeready(rep->stmo) == 0) {
			throw sockerr(ETIMEDOUT, "sockbuf::sendto", sockname.c_str());
		}

		int	wlen	= 0;
		while (len > 0) {
			int	wval	= ::sendto(rep->sock, pbuf + wlen, len, msgf, sa.addr(), sa.size());
			if (wval == -1)
				throw wlen;
			len -= wval;
			wlen += wval;
		}
		return wlen;
	}

	#if	!defined(__linux__) && !defined(OS_WIN32)
	// does not have sendmsg or recvmsg


	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::recvmsg(msghdr* msg, int msgf) {
		if (rep->rtmo != -1 && is_readready(rep->rtmo) == 0) {
			throw sockerr(ETIMEDOUT, "sockbuf::recvmsg", sockname.c_str());
		}

		if (rep->oob && atmark()) {
			throw sockoob();
		}

		int	rval	= ::recvmsg(rep->sock, msg, msgf);
		if (rval == -1) {
			throw sockerr(errno, "sockbuf::recvmsg", sockname.c_str());
		}
		return rval;
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::sendmsg(msghdr* msg, int msgf)
		// upon error, write throws the number of bytes writen so far instead
		// of sockerr.
	{
		if (rep->stmo != -1 && is_writeready(rep->stmo) == 0) {
			throw sockerr(ETIMEDOUT, "sockbuf::sendmsg", sockname.c_str());
		}

		int	wlen	= ::sendmsg(rep->sock, msg, msgf);
		if (wlen == -1) {
			throw 0;
		}
		return wlen;
	}
	#endif // !__linux__ && !OS_WIN32

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::sendtimeout(int wp) {
		int	oldstmo	= rep->stmo;
		rep->stmo = (wp < 0) ? -1 : wp;
		return oldstmo;
	}

	template<class charT, class Traits>
	int	basic_sockbuf<charT, Traits>::recvtimeout(int wp) {
		int	oldrtmo	= rep->rtmo;
		rep->rtmo = (wp < 0) ? -1 : wp;
		return oldrtmo;
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::is_readready(int wp_sec, int wp_usec) const {
		fd_set	fds;
		FD_ZERO(&fds);
		FD_SET(rep->sock, &fds);

		timeval	tv;
		tv.tv_sec = wp_sec;
		tv.tv_usec = wp_usec;

		int	ret	= select(static_cast<int>(rep->sock) + 1, &fds, 0, 0, (wp_sec == -1) ? 0 : &tv);
		if (ret == -1) {
			throw sockerr(errno, "sockbuf::is_readready", sockname.c_str());
		}
		return ret;
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::is_writeready(int wp_sec, int wp_usec) const {
		fd_set	fds;
		FD_ZERO(&fds);
		FD_SET(rep->sock, &fds);

		timeval	tv;
		tv.tv_sec = wp_sec;
		tv.tv_usec = wp_usec;

		int	ret	= select(static_cast<int>(rep->sock)+ 1, 0, &fds, 0, (wp_sec == -1) ? 0 : &tv);
		if (ret == -1) {
			throw sockerr(errno, "sockbuf::is_writeready", sockname.c_str());
		}
		return ret;
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::is_exceptionpending(int wp_sec, int wp_usec) const {
		fd_set	fds;
		FD_ZERO(&fds);
		FD_SET(rep->sock, &fds);

		timeval	tv;
		tv.tv_sec = wp_sec;
		tv.tv_usec = wp_usec;

		int	ret	= select(static_cast<int>(rep->sock) + 1, 0, 0, &fds, (wp_sec == -1) ? 0 : &tv);
		if (ret == -1) {
			throw sockerr(errno, "sockbuf::is_exceptionpending", sockname.c_str());
		}
		return ret;
	}

	template<class charT, class Traits>
	void
	basic_sockbuf<charT, Traits>::shutdown(shuthow sh) {
		switch (sh) {
			case shut_read:
				delete[] eback();
				setg(0, 0, 0);
				break;
			case shut_write:
				delete[] pbase();
				setp(0, 0);
				break;
			case shut_readwrite:
				shutdown(shut_read);
				shutdown(shut_write);
				break;
		}
		if (::shutdown(rep->sock, sh) == -1) {
			throw sockerr(errno, "sockbuf::shutdown", sockname.c_str());
		}
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::getopt(int op, void* buf, int len, int level) const {
		if (::getsockopt(rep->sock, level, op, (char *) buf, (socklen_t *) // LN
	& len) == -1) {
			throw sockerr(errno, "sockbuf::getopt", sockname.c_str());
		}
		return len;
	}

	template<class charT, class Traits>
	void
	basic_sockbuf<charT, Traits>::setopt(int op, void* buf, int len, int level) const {
		if (::setsockopt(rep->sock, level, op, (char *) buf, len) == -1) {
			throw sockerr(errno, "sockbuf::setopt", sockname.c_str());
		}
	}

	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>::type
	basic_sockbuf<charT, Traits>::gettype() const {
		int	ty	= 0;
		getopt(so_type, &ty, sizeof(ty));
		return basic_sockbuf<charT, Traits>::type(ty);
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::clearerror() const {
		int	err	= 0;
		getopt(so_error, &err, sizeof(err));
		return err;
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::debug() const {
		int	old	= 0;
		getopt(so_debug, &old, sizeof(old));
		return old != 0;
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::debug(bool set) const {
		int	old	= 0;
		int	opt	= set;
		getopt(so_debug, &old, sizeof(old));
		setopt(so_debug, &opt, sizeof(opt));
		return old != 0;
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::reuseaddr() const {
		int	old	= 0;
		getopt(so_reuseaddr, &old, sizeof(old));
		return old != 0;
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::reuseaddr(bool set) const {
		int	old	= 0;
		int	opt	= set;
		getopt(so_reuseaddr, &old, sizeof(old));
		setopt(so_reuseaddr, &opt, sizeof(opt));
		return old != 0;
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::keepalive() const {
		int	old	= 0;
		getopt(so_keepalive, &old, sizeof(old));
		return old != 0;
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::keepalive(bool set) const {
		int	old	= 0;
		int	opt	= set;
		getopt(so_keepalive, &old, sizeof(old));
		setopt(so_keepalive, &opt, sizeof(opt));
		return old != 0;
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::dontroute() const {
		int	old	= 0;
		getopt(so_dontroute, &old, sizeof(old));
		return old != 0;
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::dontroute(bool set) const {
		int	old	= 0;
		int	opt	= set;
		getopt(so_dontroute, &old, sizeof(old));
		setopt(so_dontroute, &opt, sizeof(opt));
		return old != 0;
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::broadcast() const {
		int	old	= 0;
		getopt(so_broadcast, &old, sizeof(old));
		return old != 0;
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::broadcast(bool set) const {
		int	old	= 0;
		int	opt	= set;
		getopt(so_broadcast, &old, sizeof(old));
		setopt(so_broadcast, &opt, sizeof(opt));
		return old != 0;
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::oobinline() const {
		int	old	= 0;
		getopt(so_oobinline, &old, sizeof(old));
		return old != 0;
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::oobinline(bool set) const {
		int	old	= 0;
		int	opt	= set;
		getopt(so_oobinline, &old, sizeof(old));
		setopt(so_oobinline, &opt, sizeof(opt));
		return old != 0;
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::oob(bool b) {
		bool	old	= rep->oob;
		rep->oob = b;
		return old;
	}

	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>::socklinger
	basic_sockbuf<charT, Traits>::linger() const {
		socklinger	old	(0, 0);
		getopt(so_linger, &old, sizeof(old));
		return old;
	}

	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>::socklinger
	basic_sockbuf<charT, Traits>::linger(socklinger opt) const {
		socklinger	old	(0, 0);
		getopt(so_linger, &old, sizeof(old));
		setopt(so_linger, &opt, sizeof(opt));
		return old;
	}
	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>::socklinger
	basic_sockbuf<charT, Traits>::linger(int onoff, int tm) const {
		return linger(socklinger(onoff, tm));
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::sendbufsz() const {
		int	old	= 0;
		getopt(so_sndbuf, &old, sizeof(old));
		return old;
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::sendbufsz(int sz) const {
		int	old	= 0;
		getopt(so_sndbuf, &old, sizeof(old));
		setopt(so_sndbuf, &sz, sizeof(sz));
		return old;
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::recvbufsz() const {
		int	old	= 0;
		getopt(so_rcvbuf, &old, sizeof(old));
		return old;
	}

	template<class charT, class Traits>
	int	basic_sockbuf<charT, Traits>::recvbufsz(int sz) const {
		int	old	= 0;
		getopt(so_rcvbuf, &old, sizeof(old));
		setopt(so_rcvbuf, &sz, sizeof(sz));
		return old;
	}

	template<class charT, class Traits>
	bool
	basic_sockbuf<charT, Traits>::atmark() const
		// return true, if the read pointer for socket points to an
		// out of band data
	{
#ifndef OS_WIN32
		int	arg;
		if (::ioctl(rep->sock, SIOCATMARK, &arg) == -1) {
			throw sockerr(errno, "sockbuf::atmark", sockname.c_str());
		}
#else
		unsigned long arg = 0;
		if (::ioctlsocket(rep->sock, SIOCATMARK, &arg) == SOCKET_ERROR) {
			throw sockerr(WSAGetLastError(), "sockbuf::atmark", sockname.c_str());
		}
#endif // !OS_WIN32
		return arg != 0;
	}

#ifndef OS_WIN32
	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::pgrp() const
		// return the process group id that would receive SIGIO and SIGURG
		// signals
	{
		int	arg;
		if (::ioctl(rep->sock, SIOCGPGRP, &arg) == -1) {
			throw sockerr(errno, "sockbuf::pgrp", sockname.c_str());
		}
		return arg;
	}

	template<class charT, class Traits>
	int
	basic_sockbuf<charT, Traits>::pgrp(int new_pgrp) const
		// set the process group id that would receive SIGIO and SIGURG signals.
		// return the old pgrp
	{
		int	old	= pgrp();
		if (::ioctl(rep->sock, SIOCSPGRP, &new_pgrp) == -1) {
			throw sockerr(errno, "sockbuf::pgrp", sockname.c_str());
		}
		return old;
	}

	template<class charT, class Traits>
	void
	basic_sockbuf<charT, Traits>::closeonexec(bool set) const
		// if set is true, set close on exec flag
		// else clear close on exec flag
	{
		if (set) {
			if (::ioctl(rep->sock, FIOCLEX, 0) == -1) {
				throw sockerr(errno, "sockbuf::closeonexec", sockname.c_str());
			}
		} else {
			if (::ioctl(rep->sock, FIONCLEX, 0) == -1) {
				throw sockerr(errno, "sockbuf::closeonexec", sockname.c_str());
			}
		}
	}
#endif // !OS_WIN32

	template<class charT, class Traits>
	long
	basic_sockbuf<charT, Traits>::nread() const
		// return how many chars are available for reading in the recvbuf of
		// the socket.
	{
		long	arg;
#ifndef OS_WIN32  
		if (::ioctl(rep->sock, FIONREAD, &arg) == -1) {
			throw sockerr(errno, "sockbuf::nread", sockname.c_str());
		}
#else
		if (::ioctlsocket(rep->sock, FIONREAD, (unsigned long *) &arg) == SOCKET_ERROR) {
			throw sockerr(WSAGetLastError(), "sockbuf::nread", sockname.c_str());
		}
#endif // !OS_WIN32
		return arg;
	}

	template<class charT, class Traits>
	long
	basic_sockbuf<charT, Traits>::howmanyc() const
		// return how many chars are available for reading in the input buffer
		// and the recvbuf of the socket.
	{
		return showmanyc() + nread();
	}

	template<class charT, class Traits>
	void
	basic_sockbuf<charT, Traits>::nbio(bool set) const
		// if set is true, set socket to non-blocking io. Henceforth, any
		// write or read operation will not wait if write or read would block.
		// The read or write operation will result throwing a sockerr
		// exception with errno set to  EWOULDBLOCK.
	{
	#ifndef OS_WIN32
		int	arg	= set;
		if (::ioctl(rep->sock, FIONBIO, &arg) == -1) {
			throw sockerr(errno, "sockbuf::nbio", sockname.c_str());
		}
	#else
		unsigned long arg = (set) ? 1 : 0;
		if (::ioctlsocket(rep->sock, FIONBIO, &arg) == -1) {
			throw sockerr(WSAGetLastError(), "sockbuf::nbio", sockname.c_str());
		}
	#endif // !OS_WIN32
	}

	#ifndef OS_WIN32
	template<class charT, class Traits>
	void
	basic_sockbuf<charT, Traits>::async(bool set) const
		// if set is true, set socket for asynchronous io. If any io is
		// possible on the socket, the process will get SIGIO
	{
		int	arg	= set;
		if (::ioctl(rep->sock, FIOASYNC, &arg) == -1) {
			throw sockerr(errno, "sockbuf::async", sockname.c_str());
		}
	}
	#endif // !OS_WIN32

	template<class charT, class Traits>
	basic_isockstream<charT, Traits>::basic_isockstream(
		typename basic_sockbuf<charT, Traits>* sb
	) :
		basic_ios<charT, Traits>(sb),
		basic_istream<charT, Traits>(sb) {
	}
	template<class charT, class Traits>
	basic_isockstream<charT, Traits>::~basic_isockstream() {
	}

	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>*
	basic_isockstream<charT, Traits>::rdbuf() {
		return (basic_sockbuf<charT, Traits> *) std::basic_ios<charT, Traits>::rdbuf();
	}
	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>*
	basic_isockstream<charT, Traits>::operator ->() {
		return rdbuf();
	}
	template<class charT, class Traits>
	basic_osockstream<charT, Traits>::basic_osockstream(
		typename basic_sockbuf<charT, Traits>* sb) :
		basic_ios<charT, Traits>(sb),
		basic_ostream<charT, Traits>(sb) {
	}
	template<class charT, class Traits>
	basic_osockstream<charT, Traits>::~basic_osockstream() {
	}

	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>*
	basic_osockstream<charT, Traits>::rdbuf() {
		return (basic_sockbuf<charT, Traits> *) basic_ios<charT, Traits>::rdbuf();
	}
	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>*
	basic_osockstream<charT, Traits>::operator ->() {
		return rdbuf();
	}
	template<class charT, class Traits>
	typename basic_osockstream<charT, Traits>&
	crlf(typename basic_osockstream<charT, Traits>& o) {
		o << "\r\n";
		o.rdbuf()->pubsync();
		return o;
	}

	template<class charT, class Traits>
	typename basic_osockstream<charT, Traits>&
	lfcr(typename basic_osockstream<charT, Traits>& o) {
		o << "\n\r";
		o.rdbuf()->pubsync();
		return o;
	}
	template<class charT, class Traits>
	basic_iosockstream<charT, Traits>::basic_iosockstream(
		typename basic_sockbuf<charT, Traits>* sb) :
		basic_ios<charT, Traits>(sb),
		basic_iostream<charT, Traits>(sb) {
	}
	template<class charT, class Traits>
	basic_iosockstream<charT, Traits>::~basic_iosockstream() {
	}

	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>*
	basic_iosockstream<charT, Traits>::rdbuf() {
		return (basic_sockbuf<charT, Traits>*) basic_ios<charT, Traits>::rdbuf();
	}
	template<class charT, class Traits>
	typename basic_sockbuf<charT, Traits>*
	basic_iosockstream<charT, Traits>::operator ->() {
		return rdbuf();
	}
	sockerr::sockerr(int e, const char* operation ) :
		err(e)	{
		if (operation != NULL) {
			text = operation;
		}
	}
	sockerr::sockerr(int e, const char* operation, const char* specification) :
		err(e)	{
		if (operation != NULL) {
			text = operation;
		}
		if (specification != NULL) {
			text += "(";
			text += specification;
			text += ")";
		}
	}
	sockerr::sockerr(int e, const string& operation) :
		err(e)	{
		text = operation;
	}
	sockerr::sockerr(const sockerr& O) :
		exception	(O) {
		err = O.err;
		text = O.text;
	}

	const char*sockerr::what() const {
		return "sockerr";
	}
	const char*sockerr::operation() const {
		return text.c_str();
	}
	int	sockerr::serrno() const {
		return err;
	} // LN
	bool sockerr::error(int eno) const {
		return eno == err;
	}

	const char*sockerr::errstr() const {
#if		defined(OS_WIN32)
		const char* literal[] = {
			"",
		};
		return ""; // TODO
#else
		return SYS_ERRLIST[err];
#endif
	}

	bool	sockerr::io() const {		// recoverable io error.
		switch (err) {
			case EWOULDBLOCK:
			case EINPROGRESS:
			case EALREADY:
				return true;
		}
		return false;
	}

	bool sockerr::arg() const {			// recoverable argument error.
		switch (err) {
			case ENOTSOCK:
			case EDESTADDRREQ:
			case EMSGSIZE:
			case EPROTOTYPE:
			case ENOPROTOOPT:
			case EPROTONOSUPPORT:
			case ESOCKTNOSUPPORT:
			case EOPNOTSUPP:
			case EPFNOSUPPORT:
			case EAFNOSUPPORT:
			case EADDRINUSE:
			case EADDRNOTAVAIL:
				return true;
		}
		return false;
	}

	bool sockerr::op() const {		// operational error encountered 
		switch (err) {
			case ENETDOWN:
			case ENETUNREACH:
			case ENETRESET:
			case ECONNABORTED:
			case ECONNRESET:
			case ENOBUFS:
			case EISCONN:
			case ENOTCONN:
			case ESHUTDOWN:
			case ETOOMANYREFS:
			case ETIMEDOUT:
			case ECONNREFUSED:
			case ELOOP:
			case ENAMETOOLONG:
			case EHOSTDOWN:
			case EHOSTUNREACH:
			case ENOTEMPTY:
#if !defined(OS_LINUX) // LN
			case EPROCLIM:
#	endif
			case EUSERS:
			case EDQUOT:
				return true;
		}
		return false;
	}

	bool sockerr::conn() const {
		switch (err) {
			case EISCONN:
			case ENOTCONN:
			case ECONNRESET:
			case ECONNREFUSED:
			case ETIMEDOUT:
			case EPIPE:
				return true;
		}
		return false;
	}

	bool sockerr::addr() const	{
		switch (err) {
			case EADDRINUSE:
			case EADDRNOTAVAIL:
				return true;
		}
		return false;
	}

	bool sockerr::benign() const {
		switch (err) {
			case EINTR:
			case EWOULDBLOCK:
				// On FreeBSD (and probably on Linux too) 
				// EAGAIN has the same value as EWOULDBLOCK
#if !defined(OS_BSD) && !defined(OS_LINUX) && !defined(OS_MACOS) // LN
			case EAGAIN:
#endif
				return true;
		}
		return false;
	}

	template SOCKSTREAM_API basic_sockbuf<char, char_traits<char> >;
	template SOCKSTREAM_API basic_isockstream<char, char_traits<char> >;
	template SOCKSTREAM_API basic_osockstream<char, char_traits<char> >;
	template SOCKSTREAM_API basic_iosockstream<char, char_traits<char> >;

	template SOCKSTREAM_API basic_sockbuf<wchar_t, char_traits<wchar_t> >;
	template SOCKSTREAM_API basic_isockstream<wchar_t, char_traits<wchar_t> >;
	template SOCKSTREAM_API basic_osockstream<wchar_t, char_traits<wchar_t> >;
	template SOCKSTREAM_API basic_iosockstream<wchar_t, char_traits<wchar_t> >;

}

#if	defined(_WIN32)
#include <windows.h>
bool APIENTRY
DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}
#endif