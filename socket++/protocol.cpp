// protocol.h -*- C++ -*- socket library
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11

#include <socket++/protocol.h>

#if		defined(OS_WIN32)
#	define EPROTONOSUPPORT				WSAEPROTONOSUPPORT
#endif

namespace sockstream {
	protocol::protocol() :
		std::ios(0),
		iosockstream(NULL)
	{
	}  // NULL seems like a very bad idea
	const char*protocol::protocol_name(transport t) {
		char*	ret	= "";
		if (t == protocol::tcp) {
			ret = "tcp";
		}
		if (t == protocol::udp) {
			ret = "udp";
		}
		return ret;
	}

	protocol::serverbuf::serverbuf(sockinetbuf& si) :
		sockinetbuf(si),
		pn(protocol::nil)
	{
	}
	protocol::serverbuf::serverbuf(transport pname) :
		sockinetbuf((sockbuf::type) pname, 0),
		pn(pname)
	{
	}
	void protocol::serverbuf::bind() {
		serve();
	}
	const char*protocol::serverbuf::protocol_name() const {
		return protocol::protocol_name(pn);
	}

	void protocol::clientbuf::connect() {
		if (pn == protocol::nil) {
			throw sockerr(EPROTONOSUPPORT);
		}
		sockinetbuf::connect(localhost(), rfc_name(), protocol_name());
	}

	void protocol::clientbuf::connect(unsigned long addr)
		// addr is in host byte order
	{
		if (pn == protocol::nil) {
			throw sockerr(EPROTONOSUPPORT);
		}
		sockinetbuf::connect(addr, rfc_name(), protocol_name());
	}

	void protocol::clientbuf::connect(const char* host) {
		if (pn == protocol::nil) {
			throw sockerr(EPROTONOSUPPORT);
		}
		sockinetbuf::connect(host, rfc_name(), protocol_name());
	}

	void protocol::clientbuf::connect(const char* host, int portno) {
		if (pn == protocol::nil) {
			throw sockerr(EPROTONOSUPPORT);
		}
		sockinetbuf::connect(host, portno);
	}
	protocol::clientbuf::clientbuf(sockinetbuf& si) :
		sockinetbuf(si),
		pn(protocol::nil)
	{
	}
	protocol::clientbuf::clientbuf(transport pname) :
		sockinetbuf((sockbuf::type) pname, 0),
		pn(pname)
	{
	}
	const char*protocol::clientbuf::protocol_name() const {
		return protocol::protocol_name(pn);
	}
}
