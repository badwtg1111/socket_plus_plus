// protocol.h -*- C++ -*- socket library
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <socket++/sockinet.h>
namespace sockstream {
class SOCKSTREAM_API protocol : public iosockstream {
public:
	enum transport {
		nil		= 0,
		tcp		= sockbuf::sock_stream,
		udp		= sockbuf::sock_dgram
	};
	static const char*		protocol_name(transport t);

	class SOCKSTREAM_API serverbuf : public sockinetbuf {
	private:
		transport			pn;
		void				bind(sockaddr& sa)		{ sockbuf::bind(sa);	}

	public:
		serverbuf(sockinetbuf& si);
		serverbuf(transport pname);

		void				bind();
		const char*			protocol_name() const;

		virtual void		serve(int portno = -1) = 0;
		virtual const char*	rfc_name() const = 0;
		virtual const char*	rfc_doc() const = 0;
	};
	class SOCKSTREAM_API clientbuf : public sockinetbuf {
	private:
		transport			pn;
		void				connect(sockaddr& sa)	{ sockbuf::connect(sa); }

	public:
		clientbuf(sockinetbuf& si);
		clientbuf(transport pname);

		void				connect();
		void				connect(unsigned long addr);
		void				connect(const char* host);
		void				connect(const char* host, int portno);

		const char*			protocol_name() const;

		virtual const char*	rfc_name() const = 0;
		virtual const char*	rfc_doc() const = 0;
	};

	public:
		protocol();
};
}
#endif // PROTOCOL_H
