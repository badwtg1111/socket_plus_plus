/*** Disclaimer ******************************************************************/
/*  																			 */
/*  Win32 Named Pipe Wrapper						  (C) Clemens Fischer 2001   */
/*  																			 */
/*  THE CODE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,  				 */
/*  EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO WARRANTIES 		 */
/*  OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. 					 */
/*  IN NO EVENT WILL THE AUTHOR OR AUTHORS BE LIABLE TO YOU FOR ANY DAMAGES,	 */
/*  INCLUDING INCIDENTAL OR CONSEQUENTIAL DAMAGES, ARISING OUT OF THE USE   	 */
/*  OF THE CODE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.			 */
/*  YOU ACKNOWLEDGE THAT YOU HAVE READ THIS LICENSE, UNDERSTAND IT AND AGREE	 */
/*  TO BE BOUND BY ITS TERMS AS THE COMPLETE AND EXCLUSIVE STATEMENT OF 		 */
/*  THE AGREEMENT BETWEEN US, SUPERSEDING ANY PROPOSAL OR PRIOR AGREEMENT,  	 */
/*  ORAL OR WRITTEN, AND ANY OTHER COMMUNICATIONS BETWEEN US RELATING   		 */
/*  TO THE SUBJECT MATTER OF THIS LICENSE.  									 */
/*  																			 */
/*********************************************************************************/

#ifndef		__PIPE_CLASS__
#define		__PIPE_CLASS__
#include <string>
#include <windows.h>
#define		PIPE_MAX_READ_BUFFER_SIZE		0xFFF

// Pipe errors
namespace fifostream {

	enum {
		PIPE_ERROR_SUCCESS				= 0x40000000,		// Success
		PIPE_ERROR_NAME_ALREADY_EXISTS,						// The specified pipe name already exists
		PIPE_ERROR_NAME_REGISTRATION_FAILED,				// The pipe name registration failed 
		PIPE_ERROR_NAME_DOES_NOT_EXISTS,					// The specified pipe name doesn´t exists
		PIPE_ERROR_NAME_UNREGISTRATIONS_FAILED, 			// The pipe name unregistration failed 
		PIPE_ERROR_CREATION_FAILED,							// Pipe creation failed
		PIPE_ERROR_CALLBACK,								// Error related to backpipe
		PIPE_ERROR_READ_FAILED,								// Error while reading from pipe
		PIPE_ERROR_WRITE_FAILED,							// Error while Wrinting to pipe
		PIPE_CREATE_ALREADY_CALLED							// create was already called
	};


	enum AccessMode {
		PIPE_READ												= 0x01,			// Grant client read access
		PIPE_WRITE													// Grant client write access
	};

	typedef		UINT(*PIPE_CALLBACK)	(LPVOID pParam, LPVOID Buffer, DWORD dwLength);

	class FIFO {
	private :
		std::string			subkey;
		int						m_LastError;

		std::string			pipename;			
		DWORD					access;
		bool					m_bServerSide;
		PIPE_CALLBACK			backpipe;
		LPVOID					m_pCallBackParam;

		bool					m_bcreateCalled;
		bool					m_bcloseCalled;
		bool					m_bPipeRegistered;

		HANDLE					readside;
		HANDLE					writeside;
		HANDLE					m_hClientReadPipe;
		HANDLE					m_hClientWritePipe;
		HANDLE					mutex;

		bool*					m_pbVaildPtr;
		bool					m_bVaildThread;
		bool					m_bRunThread;

		// Private operations 

		bool					registerme(void);
		bool					deregister(void);
		bool					update(void);
		bool					retrieve(void);

		bool					callback(void);
		void					SetLastError(int nErrorCode);
		static UINT				CallBackThread(LPVOID pParam);

	public :
		FIFO(void);		
		virtual ~FIFO(void);

		bool					create(std::string PipeName,
									DWORD dwDesiredAccess = PIPE_READ | PIPE_WRITE,
									bool bServerSide = TRUE,
									PIPE_CALLBACK CallBackFunc = NULL,
									LPVOID pCallBackParam = NULL);
		bool					read(LPVOID Buffer, DWORD nNumberOfBytesToRead, DWORD* lpNumberOfBytesRead);
		bool					write(LPVOID Buffer, DWORD nNumberOfBytesToWrite);
		void					close(void);
		int						GetLastError(void);
	};
}
#endif