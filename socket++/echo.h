// echo.h -*- C++ -*- socket library
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11

#ifndef ECHO_H
#define ECHO_H

#include <socket++/protocol.h>
using namespace sockstream;
namespace echo {
	class SOCKSTREAM_API server : public protocol {
	public:
		class SOCKSTREAM_API serverbuf : public protocol::serverbuf {
		public:
			serverbuf(sockinetbuf& si) :
				protocol::serverbuf(si)
			{
			}
			serverbuf(protocol::transport pname) :
				protocol::serverbuf(pname)
			{
			}

			virtual void	serve(int portno = -1);
			virtual const char*	rfc_name() const {	return "echo";	}
			virtual const char*	rfc_doc() const {	return "rfc862";	}
		};

	protected:
		server() :
			std::ios(0)	{
		}
		server(const server& s)	{
		}

	public:
		server(protocol::transport pname) :
			std::ios(0)	{
			std::ios::init(new serverbuf(pname));
		}
		~server() {
			delete std::ios::rdbuf(); std::ios::init(0);
		}

		serverbuf*rdbuf() {	return (serverbuf *) protocol::rdbuf();	}
		serverbuf*operator ->() {	return rdbuf(); }
	};
	class SOCKSTREAM_API client : public protocol {
	public:
		class SOCKSTREAM_API clientbuf : public protocol::clientbuf {
		public:
			clientbuf(sockinetbuf& si) :
				protocol::clientbuf(si)
			{
			}
			clientbuf(protocol::transport pname) :
				protocol::clientbuf(pname)
			{
			}

			virtual const char*	rfc_name() const {	return "echo";	}
			virtual const char*	rfc_doc() const {	return "rfc862";	}
		};

	protected:
		client() :
			std::ios(0)	{
		}

	public:
		client(protocol::transport pname) :
			std::ios(0)	{
			std::ios::init(new clientbuf(pname));
		}
		~client() {
			delete std::ios::rdbuf(); std::ios::init(0);
		}

		clientbuf*rdbuf() {	return (clientbuf *) protocol::rdbuf();	}
		clientbuf*operator ->() {	return rdbuf(); }
	};
}
#endif // !ECHO_H
