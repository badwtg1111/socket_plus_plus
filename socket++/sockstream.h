// sockstream.h -*- C++ -*- socket library
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
//
// Version: 1.2 2002-07-25 Herbert Straub 
// 	Improved Error Handling - extending the sockerr class by cOperation
// 2003-03-06 Herbert Straub
// 	adding sockbuf::getname und setname (sockname)
// 	sockbuf methods throw method name + sockname

#ifndef _SOCKSTREAM_H
#define	_SOCKSTREAM_H

#include <iostream>
#include <iomanip>
#include <cstddef>
#include <ctype.h>
#include <string>
#include <cstdio>
#include <stdexcept>
#include <socket++/autosense.h>

#if		defined(OS_WIN32)
#	if		defined(SOCKSTREAM_EXPORTS)
#		define SOCKSTREAM_API __declspec(dllexport)
#	elif	defined(SOCKSTREAM_IMPORTS)
#		define SOCKSTREAM_API __declspec(dllimport)
#	endif

#	ifndef SOCKSTREAM_API
#	define SOCKSTREAM_API
#	endif
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR 
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL 
#define NOUSER
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#	define __wtypes_h__
#	define __RPCNDR_H__
#	define IN
#	define OUT
#	include <WinSock2.h>
#	pragma comment(lib, "ws2_32.lib")

/*
#ifdef errno
#undef errno
#define errno (WSAGetLastError())
#endif
*/
#elif	defined(OS_UNIX)

#	include <sys/types.h>
#	include <sys/uio.h>
#	include <sys/socket.h>
#	define SOCKET int
#	define SOCKET_ERROR -1

#if		defined(OS_LINUX)
#  define MSG_MAXIOVLEN	 16
#endif

#else
#error "unknown OS"
#endif

using namespace std;
namespace sockstream {

	class SOCKSTREAM_API sockaddr {
	public:
		sockaddr();
		virtual	~sockaddr();

		virtual	operator	void*() const = 0;
		virtual	operator	::sockaddr*() const = 0;
		virtual int			size() const = 0;
		virtual int			family() const = 0;
		virtual	::sockaddr*	addr() const = 0;
	};

	// socket buffer class
	template<class charT, class Traits>
	class SOCKSTREAM_API basic_sockbuf : public basic_streambuf<charT, Traits> { //streambuf
	public:
		enum type {
			sock_stream			= SOCK_STREAM,
			sock_dgram			= SOCK_DGRAM,
			sock_raw			= SOCK_RAW,
			sock_rdm			= SOCK_RDM,
			sock_seqpacket		= SOCK_SEQPACKET
		};
		enum option {
			so_debug			= SO_DEBUG,
			so_reuseaddr		= SO_REUSEADDR,
			so_keepalive		= SO_KEEPALIVE,
			so_dontroute		= SO_DONTROUTE,
			so_broadcast		= SO_BROADCAST,
			so_linger			= SO_LINGER,
			so_oobinline		= SO_OOBINLINE,
			so_sndbuf			= SO_SNDBUF,
			so_rcvbuf			= SO_RCVBUF,
			so_error			= SO_ERROR,
			so_type				= SO_TYPE
		};	
		enum level {
			sol_socket			= SOL_SOCKET
		};
		enum msgflag {
			msg_oob				= MSG_OOB,
			msg_peek			= MSG_PEEK,
			msg_dontroute		= MSG_DONTROUTE,
#if !(defined(OS_BSD) || defined(OS_MACOS))
			msg_maxiovlen		= MSG_MAXIOVLEN
#endif
		};
#if		defined(OS_UNIX)
		enum shuthow {
			shut_read,
			shut_write,
			shut_readwrite
		};
#elif	defined(OS_WIN32)
		enum shuthow {
			shut_read			= SD_RECEIVE,
			shut_write			= SD_SEND,
			shut_readwrite		= SD_BOTH,
		};
#endif
		enum {
			somaxconn			= SOMAXCONN
		};
		struct socklinger {
			int	l_onoff;	// option on/off
			int	l_linger;	// linger time

			socklinger(int a, int b) :
				l_onoff(a),
				l_linger(b)	{
			}
		};

		typedef typename Traits::char_type	char_type;
		typedef streampos					pos_type;
		typedef streamoff					off_type;
		typedef typename Traits::int_type	int_type;

		typedef int							seekdir;
		enum {
			eof = EOF
		}; 

		struct socket {
			SOCKET	sock;
			socket(SOCKET d) :	sock(d)	{}
			friend ostream&operator << (ostream& os, const socket& s) {
				os << static_cast<unsigned int>(s.sock);
				return os;
			}
		};

	protected:
		struct sockcnt {
			SOCKET	sock;
			int		cnt;
			int		stmo; // -1==block, 0==poll, >0 == waiting time in secs
			int		rtmo; // -1==block, 0==poll, >0 == waiting time in secs
			bool	oob;	// check for out-of-band byte while reading
			void*	gend; // end of input buffer
			void*	pend; // end of output buffer

			sockcnt(SOCKET s) :
				sock(s),
				cnt(1),
				stmo(-1),
				rtmo(-1),
				oob(false),
				gend(0),
				pend(0)
			{
			}
		};

		sockcnt*			rep;  // counts the # refs to sock
		string				sockname; // name of sockbuf - Herbert Straub

#if 0
		virtual sockbuf*  	setbuf (char_type* s, int_type* n);
		virtual pos_type  	seekoff (off_type off,	seekdir way, ios::openmode which = ios::in|ios::out);
		virtual pos_type  	seekpos (pos_type sp, ios::openmode which = ios::in|ios::out);
#endif

		virtual int			sync();

		virtual int			showmanyc() const;
		virtual streamsize	xsgetn(char_type* s, streamsize n);
		virtual int_type	underflow();
		virtual int_type	uflow();

		virtual int_type	pbackfail(int_type c = eof);

		virtual streamsize	xsputn(const char_type* s, streamsize n);
		virtual int_type	overflow(int_type c = eof);

	public:
		basic_sockbuf(const socket& sd);
		basic_sockbuf(int domain, type, int proto);
		basic_sockbuf(const basic_sockbuf&);
		virtual~basic_sockbuf();

		SOCKET				sd() const { return rep->sock; }
		int					pubsync() { return sync(); }
		virtual bool		is_open() const;

		virtual void		bind(const sockaddr&);
		virtual void		connect(const sockaddr&);
		virtual bool		open(const sockaddr&);
		virtual bool		close();

		void				listen(int num = somaxconn);
		virtual socket		accept();
		virtual socket		accept(sockaddr& sa);

		int					read(void* buf, int len);
		int					recv(void* buf, int len, int msgf = 0);
		int					recv(std::string& buf, int msgf = 0);
		int					recv(std::wstring& buf, int msgf = 0);
		int					recvfrom(sockaddr& sa, void* buf, int len, int msgf = 0);

#if	!defined(OS_LINUX) && !defined(OS_WIN32)
		int					recvmsg(msghdr* msg, int msgf = 0);
		int					sendmsg(msghdr* msg, int msgf = 0);
#endif

		int					write(const void* buf, int len);
		int					send(const void* buf, int len, int msgf = 0);
		int					send(const std::string& buf, int msgf = 0);
		int					send(const std::wstring& buf, int msgf = 0);
		int					sendto(sockaddr& sa, const void* buf, int len, int msgf = 0);

		int					sendtimeout(int wp = -1);
		int					recvtimeout(int wp = -1);
		int					is_readready(int wp_sec, int wp_usec = 0) const;
		int					is_writeready(int wp_sec, int wp_usec = 0) const;
		int					is_exceptionpending(int wp_sec, int wp_usec = 0) const;

		void				shutdown(shuthow sh);

		int					getopt(int op, void* buf, int len, int level = sol_socket) const;
		void				setopt(int op, void* buf, int len, int level = sol_socket) const;

		type				gettype() const;
		int					clearerror() const;
		bool				debug() const;
		bool				debug(bool set) const;
		bool				reuseaddr() const;
		bool				reuseaddr(bool set) const;
		bool				keepalive() const;
		bool				keepalive(bool set) const;
		bool				dontroute() const;
		bool				dontroute(bool set) const;
		bool				broadcast() const;
		bool				broadcast(bool set) const;
		bool				oobinline() const;
		bool				oobinline(bool set) const;
		bool
		inline				oob() const { return rep->oob; }
		bool				oob(bool b);
		int					sendbufsz() const;
		int					sendbufsz(int sz)   const;
		int					recvbufsz() const;
		int					recvbufsz(int sz)   const;
		socklinger			linger() const;
		socklinger			linger(socklinger opt) const;
		socklinger			linger(int onoff, int tm) const;

		bool				atmark() const;  
		long				nread() const;
		long				howmanyc() const;
		void				nbio(bool set = true) const;
		inline void			setname(const char* name);
		inline void			setname(const string& name);
		inline const string&getname();

#if		!defined(OS_WIN32)
		void				async(bool set = true) const;
		int					pgrp() const;
		int					pgrp(int new_pgrp) const;
		void				closeonexec(bool set = true) const;
#endif
	};
};
namespace sockstream {
	template<class charT, class Traits>
	class SOCKSTREAM_API basic_isockstream : public basic_istream<charT, Traits> {
	protected:
		//  					  isockstream (): istream(rdbuf()), ios (0) {}
	public:
		basic_isockstream(typename basic_sockbuf<charT, Traits>* sb = 0) ;
		virtual~basic_isockstream();

		basic_sockbuf<charT, Traits>*rdbuf();
		basic_sockbuf<charT, Traits>*operator ->();
	};

	template<class charT, class Traits>
	class SOCKSTREAM_API basic_osockstream : public basic_ostream<charT, Traits> {
	protected:
		//  					osockstream (): ostream(static_cast<>rdbuf()), ios (0) {}
	public:
		basic_osockstream(typename basic_sockbuf<charT, Traits>* sb = 0);
		virtual~basic_osockstream() ;

		basic_sockbuf<charT, Traits>*rdbuf();
		basic_sockbuf<charT, Traits>*operator ->();
	};

	template<class charT, class Traits>
	class SOCKSTREAM_API basic_iosockstream : public basic_iostream<charT, Traits> {
	protected:
//		basic_iosockstream();

	public:
		basic_iosockstream(typename basic_sockbuf<charT, Traits>* sb = 0);
		virtual~basic_iosockstream();

		basic_sockbuf<charT, Traits>*rdbuf();
		basic_sockbuf<charT, Traits>*operator ->();

	};

	// manipulators
	template<class charT, class Traits>
	extern basic_osockstream<charT, Traits>&	crlf(basic_osockstream<charT, Traits>&);
	template<class charT, class Traits>
	extern basic_osockstream<charT, Traits>&	lfcr(basic_osockstream<charT, Traits>&);

	// inline
	template<class charT, class Traits>
	void basic_sockbuf<charT, Traits>::setname(const char* name)		{ sockname = name;	}
	template<class charT, class Traits>
	void basic_sockbuf<charT, Traits>::setname(const string& name)		{ sockname = name;	}
	template<class charT, class Traits>
	const string&	basic_sockbuf<charT, Traits>::getname()				{ return sockname;	}

	class SOCKSTREAM_API sockerr : public std::exception {
		int			err;
		string		text;
	public:
		sockerr(int e, const char* operation = NULL);
		sockerr(int e, const char* operation, const char* specification);
		sockerr(int e, const string& operation);
		sockerr(const sockerr& O);

		const char* what() const;
		const char* operation() const;

		//  int errno () const { return err; }
		int			serrno() const;
		const char*	errstr() const;
		bool		error(int eno) const;

		bool		io() const; // non-blocking and interrupt io recoverable error.
		bool		arg() const; // incorrect argument supplied. recoverable error.
		bool		op() const; // operational error. recovery difficult.

		bool		conn() const;   // connection error
		bool		addr() const;   // address error
		bool		benign() const; // recoverable read/write error like EINTR etc.
	};

	class SOCKSTREAM_API sockoob {
	public:
		const char*what() const {
			return "sockoob";
		}
	};  
}
namespace sockstream {
typedef basic_sockbuf<char, char_traits<char> >				sockbuf;
typedef basic_isockstream<char, char_traits<char> >			isockstream;
typedef basic_osockstream<char, char_traits<char> >			osockstream;
typedef basic_iosockstream<char, char_traits<char> >		iosockstream;

typedef basic_sockbuf<wchar_t, char_traits<wchar_t> >		wsockbuf;
typedef basic_isockstream<wchar_t, char_traits<wchar_t> >	wisockstream;
typedef basic_osockstream<wchar_t, char_traits<wchar_t> >	wosockstream;
typedef basic_iosockstream<wchar_t, char_traits<wchar_t> >	wiosockstream;
}
#endif	// _SOCKSTREAM_H
