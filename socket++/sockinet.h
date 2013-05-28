// sockinet.h -*- C++ -*- socket library
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11

#ifndef _SOCKINET_H
#define	_SOCKINET_H

#include <socket++/sockstream.h>
#if		!defined(OS_WIN32)
#	include <netinet/in.h>
#endif

namespace sockstream {
	class SOCKSTREAM_API sockinit {
		static int count;
	public:
		sockinit();
		~sockinit();
	};
	class SOCKSTREAM_API sockinetaddr : public sockaddr, public sockaddr_in {
/*
struct sockaddr_in {
        short   sin_family;
        u_short sin_port;
        struct  in_addr sin_addr;
        char    sin_zero[8];
};
*/
	public:
		enum domain {
			af_inet = AF_INET
		};

	protected:
		void	setport(const char* sn, const char* pn = "tcp");
		void	setaddr(const char* hn);
	public:
		sockinetaddr();
		sockinetaddr(unsigned long addr, int port_no = 0);
		sockinetaddr(const char* host_name, int port_no = 0);
		sockinetaddr(unsigned long addr, const char* service_name, const char* protocol_name = "tcp");
		sockinetaddr(const char* host_name, const char* service_name, const char* protocol_name = "tcp");
		sockinetaddr(const sockinetaddr& sina);    
		virtual ~sockinetaddr();

		::sockaddr_in*		addr_in() const;
		int					getport() const;
		const char*			gethostname() const;


		virtual	operator	void*()const;
		virtual	operator	::sockaddr*()const;
		virtual int			size() const ;
		virtual int			family() const;
		virtual ::sockaddr*	addr() const;
	};

	template<class charT, class Traits>
	class SOCKSTREAM_API basic_sockinetbuf : public sockinit, public basic_sockbuf<charT, Traits> {
	public:
		enum domain {
			af_inet = AF_INET
		};

		basic_sockinetbuf(const basic_sockinetbuf<charT, Traits>& si);
		basic_sockinetbuf(typename const basic_sockbuf<charT, Traits>::socket& sd);
		basic_sockinetbuf(typename basic_sockbuf<charT, Traits>::type ty, int proto = 0);
		~basic_sockinetbuf();

		sockinetaddr		localaddr() const;
		int					localport() const;
		const char*			localhost() const;

		sockinetaddr		peeraddr() const;
		int					peerport() const;
		const char*			peerhost() const;

		void				bind_until_success(int portno);

		virtual void		bind(sockaddr& sa);
		void				bind(int port_no = 0); // addr is assumed to be INADDR_ANY
		// and thus defaults to local host

		void				bind(unsigned long addr, int port_no);
		void				bind(const char* host_name, int port_no = 0);
		void				bind(unsigned long addr, const char* service_name, const char* protocol_name = "tcp");
		void				bind(const char* host_name, const char* service_name, const char* protocol_name = "tcp");

		virtual void		connect(sockaddr& sa);
		void				connect(unsigned long addr, int port_no);
		void				connect(const char* host_name, int port_no);
		void				connect(unsigned long addr, const char* service_name, const char* protocol_name = "tcp");
		void				connect(const char* host_name, const char* service_name, const char* protocol_name = "tcp");
		virtual bool		open(sockaddr& sa);
		bool				open(unsigned long addr, int port_no);
		bool				open(const char* host_name, int port_no);
		bool				open(unsigned long addr, const char* service_name, const char* protocol_name = "tcp");
		bool				open(const char* host_name, const char* service_name, const char* protocol_name = "tcp");
		virtual bool		close();

		virtual socket		accept();
		virtual socket		accept(sockaddr& sa);
		socket				accept(unsigned long addr, int port_no);
		socket				accept(const char* host_name, int port_no);

		bool				tcpnodelay() const;
		bool				tcpnodelay(bool set) const;
	};


	template<class charT, class Traits>
	class SOCKSTREAM_API basic_isockinet : public sockinit, public basic_isockstream<charT, Traits> {
	public:
		basic_isockinet(typename const basic_sockbuf<charT, Traits>::socket& sd);
		basic_isockinet(typename const basic_sockinetbuf<charT, Traits>& sb);
		basic_isockinet(typename basic_sockbuf<charT, Traits>::type ty = basic_sockbuf<charT, Traits>::sock_stream, int proto = 0);
		~basic_isockinet();      

		basic_sockinetbuf<charT, Traits>*rdbuf() {
			return (basic_sockinetbuf<charT, Traits> *) std::basic_ios<charT, Traits>::rdbuf();
		}
		basic_sockinetbuf<charT, Traits>*operator ->() {
			return rdbuf();
		}
	};

	template<class charT, class Traits>
	class SOCKSTREAM_API basic_osockinet : public sockinit, public basic_osockstream<charT, Traits> {
	public:
		basic_osockinet(typename const basic_sockbuf<charT, Traits>::socket& sd);
		basic_osockinet(typename const basic_sockinetbuf<charT, Traits>& sb);
		basic_osockinet(typename basic_sockbuf<charT, Traits>::type ty = basic_sockbuf<charT, Traits>::sock_stream, int proto = 0);
		~basic_osockinet();      

		basic_sockinetbuf<charT, Traits>*rdbuf() {
			return (basic_sockinetbuf<charT, Traits> *) std::basic_ios<charT, Traits>::rdbuf();
		}
		basic_sockinetbuf<charT, Traits>*operator ->() {
			return rdbuf();
		}
	};

	template<class charT, class Traits>
	class SOCKSTREAM_API basic_iosockinet : public sockinit, basic_iosockstream<charT, Traits> {
	public:
		basic_iosockinet(typename const basic_sockbuf<charT, Traits>::socket& sd);
		basic_iosockinet(typename const basic_sockinetbuf<charT, Traits>& sb);
		basic_iosockinet(typename basic_sockbuf<charT, Traits>::type ty = basic_sockbuf<charT, Traits>::sock_stream, int proto = 0);
		~basic_iosockinet();     

		basic_sockinetbuf<charT, Traits>*rdbuf() const {
			return (basic_sockinetbuf<charT, Traits> *) std::basic_ios<charT, Traits>::rdbuf();
		}
		basic_sockinetbuf<charT, Traits>*rdbuf() {
			return (basic_sockinetbuf<charT, Traits> *) std::basic_ios<charT, Traits>::rdbuf();
		}
		basic_sockinetbuf<charT, Traits>*operator ->() {
			return rdbuf();
		}
		basic_sockinetbuf<charT, Traits>*operator ->() const {
			return rdbuf();
		}
	};
}
namespace sockstream {
typedef basic_sockinetbuf<char, char_traits<char> >			sockinetbuf;
typedef basic_isockinet<char, char_traits<char> >			isockinet;
typedef basic_osockinet<char, char_traits<char> >			osockinet;
typedef basic_iosockinet<char, char_traits<char> >			iosockinet;

typedef basic_sockinetbuf<wchar_t, char_traits<wchar_t> >	wsockinetbuf;
typedef basic_isockinet<wchar_t, char_traits<wchar_t> >		wisockinet;
typedef basic_osockinet<wchar_t, char_traits<char> >		wosockinet;
typedef basic_iosockinet<wchar_t, char_traits<wchar_t> >	wiosockinet;
}

#endif	// _SOCKINET_H
