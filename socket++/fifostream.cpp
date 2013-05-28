#include <sockstream/fifostream.h>
using namespace std;
namespace fifostream {
	FIFO::FIFO(void) {
		m_LastError = PIPE_ERROR_SUCCESS;
		subkey = "";

		m_bcreateCalled = FALSE;
		m_bcloseCalled = FALSE;
		m_bPipeRegistered = FALSE;

		readside = NULL;
		writeside = NULL;	
		m_hClientReadPipe = NULL;
		m_hClientWritePipe = NULL;
		mutex = NULL;

		m_bVaildThread = FALSE;
		m_bRunThread = TRUE;
	}
	FIFO::~FIFO(void) {
		close();
	}
	/*** create **********************************************************************/
	/*																				 */
	/*  Return Value:   true if successful. 										 */
	/*																				 */
	/*  Parameters: 	PipeName		   string containing the pipe name.		 */
	/*  				dwDesiredAccess    Specifies the type of access.			 */
	/*  				bServerSide 	   Specifies server/client side.			 */
	/*  				CallBackFunc	   Pointer to backpipe functions.   		 */
	/*  				pCallBackParam     Pointer to a single parameter value  	 */
	/*  								   passed to the backpipe function. 		 */
	/*																				 */
	/*  Description:	creates and registers the neccessary pipe(s).   			 */
	/*																				 */
	/*********************************************************************************/

	bool
	FIFO::create(string PipeName,
				DWORD dwDesiredAccess,
				bool bServerSide,
				PIPE_CALLBACK CallBackFunc,
				LPVOID pCallBackParam) {
		bool				bCreationSucceeded;
		SECURITY_ATTRIBUTES	SecAttrib;

		if (m_bcreateCalled) {
			SetLastError(PIPE_CREATE_ALREADY_CALLED);

			// create pipe failed 

			return FALSE;
		}

		pipename = PipeName;			
		access = dwDesiredAccess;
		m_bServerSide = bServerSide;
		backpipe = CallBackFunc;
		m_pCallBackParam = pCallBackParam;
		m_bcreateCalled = TRUE;

		// Check if server side

		if (m_bServerSide) {
			if (registerme()) {
				// Pipe registration succeeded

				bCreationSucceeded = FALSE;

				SecAttrib.nLength = sizeof(SECURITY_ATTRIBUTES);
				SecAttrib.lpSecurityDescriptor = NULL;
				SecAttrib.bInheritHandle = TRUE;

				if (dwDesiredAccess & PIPE_READ) {
					bCreationSucceeded = (CreatePipe(&m_hClientReadPipe, &writeside, &SecAttrib, 0)) ? TRUE : FALSE;
				}

				if (dwDesiredAccess & PIPE_WRITE) {
					bCreationSucceeded = (CreatePipe(&readside, &m_hClientWritePipe, &SecAttrib, 0)) ? TRUE : FALSE;
				}

				if (bCreationSucceeded && update() && callback()) {
					return TRUE;
				} else {
					SetLastError(PIPE_ERROR_CREATION_FAILED);
					deregister();
				}
			}
		} else {
			if (retrieve() && callback()) {
				return TRUE;
			}
		}

		// Pipe creation failed

		return FALSE;
	}
	void
	FIFO::close(void) {
		if (m_bcloseCalled) {
			return;
		}

		m_bcloseCalled = TRUE;
		m_bRunThread = FALSE;

		deregister();

		CloseHandle(m_hClientWritePipe);
		CloseHandle(m_hClientReadPipe);
		CloseHandle(writeside);
		CloseHandle(readside);
		CloseHandle(mutex);

		if (m_bVaildThread) {
			*m_pbVaildPtr = FALSE;
		}
	}
	bool
	FIFO::read(LPVOID Buffer, DWORD nNumberOfBytesToRead, DWORD* lpNumberOfBytesRead) {
		if (ReadFile(readside, Buffer, nNumberOfBytesToRead, lpNumberOfBytesRead, NULL)) {
			// Read pipe succeeded

			return TRUE;
		}

		SetLastError(PIPE_ERROR_READ_FAILED);

		// Read pipe failed

		return FALSE;
	}
	bool
	FIFO::write(LPVOID Buffer, DWORD nNumberOfBytesToWrite) {
		DWORD	nNumberOfBytesWritten;

		if (WriteFile(writeside, Buffer, nNumberOfBytesToWrite, &nNumberOfBytesWritten, NULL)) {
			if (nNumberOfBytesWritten == nNumberOfBytesToWrite) {
				// Read write succeeded

				return TRUE;
			}
		}

		SetLastError(PIPE_ERROR_WRITE_FAILED);

		// Write pipe failed 

		return FALSE;
	}
	int
	FIFO::GetLastError(void) {
		// Return latest error value 

		return m_LastError;
	}
	bool
	FIFO::registerme(void) {
		HKEY	hRegKey;
		DWORD	dwDisposition;
		DWORD	dwValuePId;
		DWORD	dwValueDef;
		DWORD	dwDataLen;
		HANDLE	hProcess;
		HANDLE	hMutex;

		// create registry key

		subkey = "SOFTWARE\\";
		subkey += "\\";
		subkey += pipename;

		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, subkey.c_str(), 0, "", 0, KEY_ALL_ACCESS, NULL, &hRegKey, &dwDisposition) ==
			ERROR_SUCCESS) {
			if (dwDisposition == REG_OPENED_EXISTING_KEY) {
				dwDataLen = sizeof(DWORD);

				if (RegQueryValueEx(hRegKey, "PID", 0, NULL, (BYTE *) &dwValuePId, &dwDataLen) == ERROR_SUCCESS) {
					// Check process id &  mutex

					if ((hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwValuePId)) != NULL) {
						CloseHandle(hProcess);

						if ((hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, (LPCSTR) pipename.c_str())) != NULL) {
							CloseHandle(hMutex);

							SetLastError(PIPE_ERROR_NAME_ALREADY_EXISTS);
							RegCloseKey(hRegKey);

							// Pipe registration failed 

							return FALSE;
						}
					}
				}
			}

			// create mutex

			if ((mutex = CreateMutex(NULL, TRUE, (LPCSTR) (pipename.c_str()))) != NULL) {
				dwValuePId = GetCurrentProcessId();
				dwValueDef = (DWORD) INVALID_HANDLE_VALUE;

				// Save data

				if ((RegSetValueEx(
					hRegKey, "PID",
					0,
					REG_DWORD,
					(BYTE *) &dwValuePId,
					sizeof(DWORD)) & RegSetValueEx(hRegKey,
					"HRP",
					0,
					REG_DWORD,
					(BYTE *) &dwValueDef,
					sizeof(DWORD)) & RegSetValueEx(hRegKey,
					"HWP",
					0,
					REG_DWORD,
					(BYTE *) &dwValueDef,
					sizeof(DWORD))) ==ERROR_SUCCESS) {
					RegCloseKey(hRegKey);
					m_bPipeRegistered = TRUE;

					// Pipe registration succeeded

					return TRUE;
				}
			}

			RegCloseKey(hRegKey);
		}

		SetLastError(PIPE_ERROR_NAME_REGISTRATION_FAILED);

		// Pipe registration failed 

		return FALSE;
	}

	bool
	FIFO::deregister(void) {
		// Delete registry key

		if (m_bPipeRegistered && (RegDeleteKey(HKEY_LOCAL_MACHINE, subkey.c_str()) == ERROR_SUCCESS)) {
			// Pipe unregistration succeeded

			m_bPipeRegistered = FALSE;
			return TRUE;
		}

		SetLastError(PIPE_ERROR_NAME_UNREGISTRATIONS_FAILED);

		// Pipe unregistration failed 

		return FALSE;
	}
	bool
	FIFO::update(void) {
		HKEY	hRegKey;
		DWORD	dwDisposition;

		// Open registry key 

		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, subkey.c_str(), 0, "", 0, KEY_ALL_ACCESS, NULL, &hRegKey, &dwDisposition) ==
			ERROR_SUCCESS) {
			if ((RegSetValueEx(hRegKey, "HRP", 0, REG_DWORD, (BYTE *) &m_hClientReadPipe, sizeof(DWORD)) & RegSetValueEx(hRegKey,
																														 "HWP",
																														 0,
																														 REG_DWORD,
																														 (BYTE *) &m_hClientWritePipe,
																														 sizeof(DWORD))) ==
				ERROR_SUCCESS) {
				// Registry update succeeded

				RegCloseKey(hRegKey);

				return TRUE;
			}

			RegCloseKey(hRegKey);
		}

		// Registry update failed 

		return FALSE;
	}
	bool
	FIFO::retrieve(void) {
		HKEY	hRegKey;
		DWORD	dwValuePId;
		DWORD	dwReadPipe;	
		DWORD	dwWritePipe;
		HANDLE	hProcess;
		DWORD	dwDataLen;
		HANDLE	hMutex;

		// Open registry key

		subkey = "SOFTWARE\\";
		subkey += "\\";
		subkey += pipename;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey.c_str(), 0, KEY_ALL_ACCESS, &hRegKey) == ERROR_SUCCESS) {
			dwDataLen = sizeof(DWORD);

			if (RegQueryValueEx(hRegKey, "PID", 0, NULL, (BYTE *) &dwValuePId, &dwDataLen) == ERROR_SUCCESS) {
				// Check process id & mutex 

				if ((hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwValuePId)) != NULL) {
					if ((hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, (LPCSTR) (pipename.c_str()))) != NULL) {
						if ((RegQueryValueEx(hRegKey, "HRP", 0, NULL, (BYTE *) &dwReadPipe, &dwDataLen) & RegQueryValueEx(hRegKey,
							"HWP",
							0,
							NULL,
							(BYTE *) &dwWritePipe,
							&dwDataLen)) ==	ERROR_SUCCESS) {
							if (dwReadPipe != NULL) {
								if (!DuplicateHandle(hProcess,
													(HANDLE) dwReadPipe,
													GetCurrentProcess(),
													&readside,
													0,
													FALSE,
													DUPLICATE_SAME_ACCESS)) {
									readside = INVALID_HANDLE_VALUE;
								}
							}

							if (dwWritePipe != NULL) {
								if (!DuplicateHandle(hProcess,
													(HANDLE) dwWritePipe,
													GetCurrentProcess(),
													&writeside,
													0,
													FALSE,
													DUPLICATE_SAME_ACCESS)) {
									writeside = INVALID_HANDLE_VALUE;
								}
							}

							if ((readside != INVALID_HANDLE_VALUE) && (writeside != INVALID_HANDLE_VALUE)) {
								CloseHandle(hProcess);
								CloseHandle(hMutex);
								RegCloseKey(hRegKey);

								// Retrieve pipe succeeded

								return TRUE;
							}
						}

						SetLastError(PIPE_ERROR_CREATION_FAILED);
						CloseHandle(hProcess);
						CloseHandle(hMutex);
						RegCloseKey(hRegKey);

						// Retrieve pipe failed 

						return FALSE;
					}
				}

				CloseHandle(hProcess);
			}

			RegCloseKey(hRegKey);
		}

		SetLastError(PIPE_ERROR_NAME_DOES_NOT_EXISTS);

		// Retrieve pipe failed 

		return FALSE;
	}
	UINT
	FIFO::CallBackThread(LPVOID pParam) {
		FIFO*		pThis		= (FIFO*) pParam;
		bool		bValidPtr	= TRUE;
		BYTE		Buffer[PIPE_MAX_READ_BUFFER_SIZE];
		DWORD		dwBytesRead;

		// Init

		pThis->m_bVaildThread = TRUE;
		pThis->m_pbVaildPtr = &bValidPtr;

		while (pThis->m_bRunThread && pThis->read(&Buffer[0], PIPE_MAX_READ_BUFFER_SIZE, &dwBytesRead))
			if (bValidPtr) {
				pThis->backpipe(pThis->m_pCallBackParam, &Buffer[0], dwBytesRead);
			}

		// Leave thread

		if (bValidPtr) {
			pThis->m_bVaildThread = FALSE;
		}

		return 0;
	}
	bool
	FIFO::callback(void) {
		if (!backpipe) {
			return TRUE;
		}

#if 0
		if (AfxBeginThread((AFX_THREADPROC) CallBackThread, (void *)this)) {
			// callback succeeded

			return TRUE;
		}

		SetLastError(PIPE_ERROR_CALLBACK);

		// callback failed 
#endif
		return FALSE;
	}
	void
	FIFO::SetLastError(int nErrorCode) {
		// Set last erroer value

		m_LastError = nErrorCode;
	}
}
