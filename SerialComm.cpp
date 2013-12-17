
#include "SerialComm.h"

SerialComm::SerialComm()   
{   
	m_hCom = 0;   
	m_lpszErrorMessage[0] = '\0';   
	ZeroMemory(&m_ov, sizeof(m_ov));   
}   


SerialComm::~SerialComm() {   
	close();   
}   

char*   SerialComm::GetErrorMessage( void ) {   
	return m_lpszErrorMessage;   
}   


int SerialComm::open( char* lpszPortNum,    
						DWORD  dwBaudRate,    
						BYTE   byParity,   
						BYTE   byStopBits,   
						BYTE   byByteSize ) {   
							DCB  dcb;   //串口设备控制块 Device Control Block   
							BOOL bSuccess;   

							// m_hCom即为函数返回的串口的句柄   
							m_hCom = CreateFileA( lpszPortNum,           // pointer to name of the file   
								GENERIC_READ|GENERIC_WRITE, // 允许读写。   
								0,                     // 通讯设备必须以独占方式打开。   
								NULL,                  // 无安全属性，表示该串口不可   
								// 被子程序继承。   
								OPEN_EXISTING,         // 通讯设备已存在。   
								FILE_FLAG_OVERLAPPED,  // 使用异步方式 overlapped I/O。   
								NULL);                 // 通讯设备不能用模板打开。   
							if ( m_hCom == INVALID_HANDLE_VALUE ) {   
								SerialComm::ErrorToString("RS232::open()   CreateFile() failed, invalid handle value");   
								return FALSE;   
							}   

							// 与串口相关的参数非常多，当需要设置串口参数时，通常是先取得串口   
							// 的参数结构，修改部分参数后再将参数结构写入   
							bSuccess = GetCommState(m_hCom, &dcb);   
							if ( !bSuccess ) {   
								SerialComm::close();   
								SerialComm::ErrorToString("RS232::open()   GetCommState() failed");   
								return FALSE;   
							}   
							dcb.BaudRate = dwBaudRate;   // 串口波特率。   
							dcb.Parity   = byParity;     // 校验方式，值0~4分别对应无校验、奇   
							// 校验、偶校验、校验、置位、校验清零。    
							dcb.fParity = 0;             // 为1的话激活奇偶校验检查。   
							dcb.ByteSize = byByteSize;   // 一个字节的数据位个数，范围是5~8。   
							dcb.StopBits = byStopBits;   // 停止位个数，0~2分别对应1位、1.5位、   
							// 2位停止位。   
							if ( !bSuccess ) {   
								SerialComm::close();   
								SerialComm::ErrorToString("RS232::open()   SetCommState() failed");   
								return FALSE;   
							}   

							return TRUE;   
}   


DWORD SerialComm::output( LPCVOID pdata, DWORD   len ) {   
	BOOL  bSuccess;   
	DWORD written = 0;   

	if ( len < 1 )   
		return 0;   
	// create event for overlapped I/O   
	m_ov.hEvent = CreateEventA( NULL,   // pointer to security attributes    
		FALSE,  // flag for manual-reset event    
		FALSE,  // flag for initial state    
		"");    // pointer to event-object name    
	if ( m_ov.hEvent == INVALID_HANDLE_VALUE ) {   
		SerialComm::ErrorToString( "RS232::output()   CreateEvent() failed" );   
		return -1;   
	}   
	bSuccess = WriteFile( m_hCom,   // handle to file to write to     
		pdata,    // pointer to data to write to file    
		len,      // number of bytes to write    
		&written, // pointer to number of bytes written    
		&m_ov );  // pointer to structure needed for overlapped I/O   
	// 如果函数执行成功的话检查written的值为写入的字节数，WriteFile函数执行完毕后    
	// 自行填充的，利用此变量的填充值可以用来检查该函数是否将所有的数据成功写入串口   
	if ( SerialComm::IsNT() ) {   
		bSuccess = GetOverlappedResult( m_hCom, &m_ov, &written, TRUE );   
		if ( !bSuccess ) {   
			CloseHandle( m_ov.hEvent );   
			SerialComm::ErrorToString( "RS232::output()   GetOverlappedResult() failed" );
			return -1;   
		}   
	}   
	else if ( len != written ) {   
		CloseHandle( m_ov.hEvent );   
		SerialComm::ErrorToString( "RS232::output()   WriteFile() failed" );
		return -1;   
	}   
	CloseHandle( m_ov.hEvent );   
	return written;   
}   



DWORD SerialComm::input( LPVOID pdest, DWORD  len, DWORD  dwMaxWait ) {   
	BOOL  bSuccess;   
	DWORD result = 0,   
		read   = 0, // num read bytes   
		mask   = 0; // a 32-bit variable that receives a mask    
	// indicating the type of event that occurred   
	if ( len < 1 )   
		return(0);   
	// create event for overlapped I/O   
	m_ov.hEvent = CreateEventA( NULL,    // pointer to security attributes    
		FALSE,   // flag for manual-reset event    
		FALSE,   // flag for initial state    
		"" );    // pointer to event-object name    
	if ( m_ov.hEvent == INVALID_HANDLE_VALUE ) {   
		SerialComm::ErrorToString( "RS232::input()   CreateEvent() failed" );   
		return -1;   
	}   
	// Specify here the event to be enabled   
	bSuccess = SetCommMask( m_hCom, EV_RXCHAR );   
	if ( ! bSuccess ) {   
		CloseHandle(m_ov.hEvent);   
		SerialComm::ErrorToString("RS232::input()   SetCommMask() failed");   
		return -1;   
	}   
	// WaitForSingleObject   
	bSuccess = WaitCommEvent(m_hCom, &mask, &m_ov);   
	if ( !bSuccess ) {   
		int err = GetLastError();   
		if ( err == ERROR_IO_PENDING ) {   
			result = WaitForSingleObject(m_ov.hEvent, dwMaxWait);  //wait dwMaxWait   
			// milli seconds before returning   
			if ( result == WAIT_FAILED ) {   
				CloseHandle(m_ov.hEvent);   
				SerialComm::ErrorToString( "RS232::input()   WaitForSingleObject() failed" );   
				return -1;   
			}   
		}   
	}   
	// The specified event occured?   
	if ( mask & EV_RXCHAR )    
	{   
		bSuccess = ReadFile( m_hCom, // handle of file to read    
			pdest,  // address of buffer that receives data    
			len,    // number of bytes to read    
			&read,  // address of number of bytes read    
			&m_ov); // address of structure for data    
		if ( SerialComm::IsNT() ) {   
			bSuccess = GetOverlappedResult(m_hCom, &m_ov, &read, TRUE);   
			if ( !bSuccess ) {   
				CloseHandle( m_ov.hEvent );   
				SerialComm::ErrorToString( "RS232::input()   GetOverlappedResult() failed" );   
				return -1;   
			}   
		}   
		else if ( !bSuccess ) {   
			CloseHandle(m_ov.hEvent);   
			SerialComm::ErrorToString( "RS232::input()   ReadFile() failed" );   
			return -1;   
		}   
	}   
	else {   
		CloseHandle(m_ov.hEvent);   
		wsprintfA( m_lpszErrorMessage, "RS232::input()   No EV_RXCHAR occured\n" );   
		return -1;   
	}   
	CloseHandle(m_ov.hEvent);   
	return read;   
}   


void SerialComm::close( void ) {   
	if ( m_hCom > 0 )   
		CloseHandle( m_hCom );   
	m_hCom = 0;    
}   


VOID SerialComm::ErrorToString( char* lpszMessage ) {   
	LPVOID lpMessageBuffer;   
	DWORD  error = GetLastError();   

	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |   
		FORMAT_MESSAGE_FROM_SYSTEM,      // source and processing options   
		NULL,                            // pointer to message source   
		error,                           // requested message identifie   
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // the user default language.   
		( LPTSTR ) &lpMessageBuffer,     // pointer to message buffer   
		0,                               // maximum size of message buffer   
		NULL);                           // address of array of message inserts    

	// and copy it in our error string   
	wsprintfA(m_lpszErrorMessage,"%s: (%d) %s\n", lpszMessage, error, lpMessageBuffer);   

	LocalFree(lpMessageBuffer);   
}   


BOOL SerialComm::IsNT( void ) {   
	OSVERSIONINFO osvi;   

	osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );   
	GetVersionEx( &osvi );   
	if ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT )   
		return TRUE;   
	else   
		return FALSE;   
}   

bool SerialComm::isAvailable()
{
	return (m_hCom != INVALID_HANDLE_VALUE)&&(m_hCom != NULL);
}
