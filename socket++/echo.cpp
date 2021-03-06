// echo.C -*- C++ -*- socket library
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11



#include <socket++/echo.h>
//#include <sockstream/fork.h>
#include <stdlib.h>
#if		defined(OS_WIN32)
#include <windows.h>
#define sleep Sleep
#endif
using namespace std;

namespace echo {
	void server::serverbuf::serve(int portno) {
		if (protocol_name()) {
			if (portno < 0)
				sockinetbuf::bind((unsigned long) INADDR_ANY, "echo", protocol_name());
			else if (portno <= 1024) {
				sockinetbuf::bind();
				cout << "echo@" << localhost() << ":" << localport() << endl;
			} else {
				sockinetbuf::bind((unsigned long) INADDR_ANY, portno);
				cout << "echo@" << localhost() << ":" << portno << endl;
			}

			// act as a server now
			listen(sockbuf::somaxconn);

			// commit suicide when we receive SIGTERM
	//		Fork::suicide_signal(SIGTERM);

			for (; ;) {
				sockbuf	s	= accept();

	//			Fork	f	(1, 1); // kill my children when I get terminated.

	//			if (f.is_child()) {
					char	buf[1024];
					int		rcnt;

					while ((rcnt = s.read(buf, 1024)) > 0)
						while (rcnt != 0) {
							int	wcnt	= s.write(buf, rcnt);
							if (wcnt == -1)
								throw sockerr(errno);
							rcnt -= wcnt;
						}
					sleep(300);
					exit(0);
	//			}
			}
		}
	}
}

