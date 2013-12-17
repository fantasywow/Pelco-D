#pragma once
#include <windows.h>
class SerialComm {   
public:   

	SerialComm(); 
	~SerialComm();   

	int open( char* lpszPortNum = "com1",    // 串口号   
		DWORD  dwBaudRate  = CBR_2400,  // 波特率   
		BYTE   byParity    = NOPARITY,  // 奇偶校验   
		BYTE   byStopBits  = ONESTOPBIT,// 停止位个数   
		BYTE   byByteSize  = 8 );       // 字节长度   

	DWORD   output( LPCVOID pdata, DWORD len );   
	DWORD   input( LPVOID  pdest, DWORD len, DWORD dwMaxWait = 500 );   
	char*   GetErrorMessage( void );   
	void close();   
	bool isAvailable();
private:   
	VOID    ErrorToString( char* lpszMessage );   
	BOOL    IsNT( void );   
	char       m_lpszErrorMessage[256];   
	HANDLE      m_hCom;   // 串口句柄   
	OVERLAPPED  m_ov;     // 包含异步输入输出操作信息的结构   
};   
