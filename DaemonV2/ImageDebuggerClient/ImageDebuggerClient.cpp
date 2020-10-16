// ImageDebuggerClient.cpp : Defines the exported functions for the DLL application.
//

//#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
//#include <windows.h>
//#include <winsock2.h>
#include <stdio.h>
#include <stdarg.h>

#

#include "ImageDebuggerClient.h"

API bool IDBG_PrintString(int socket, const char* format, ...);


char exe_path[1024];
//DWORD exe_port;
//
//bool GetExecutablePath(void)
//{
//	HKEY hKey;
//	LSTATUS status = RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\TJaworski\\ImageDebugger", NULL,  KEY_ALL_ACCESS, &hKey);
//	if (status != ERROR_SUCCESS)
//	{
//		return false;
//	}
//
//	DWORD pType = REG_SZ;
//	DWORD pSize = 1023;
//	RegQueryValueExA(hKey, "Path", NULL, &pType, (LPBYTE)exe_path, &pSize);
//	if (status != ERROR_SUCCESS)
//		return false;
//
//	pType = REG_DWORD;
//	pSize = 4;
//	RegQueryValueExA(hKey, "Port", NULL, &pType, (LPBYTE)&exe_port, &pSize);
//	if (status != ERROR_SUCCESS)
//		return false;
//
//	RegCloseKey(hKey);
//	return true;
//}

#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

static int client_socket = -1;

void IDBG_Disconect() {
    if (client_socket == -1)
        return;

    ::close(client_socket);
    client_socket = -1;
}

int IDBG_Connection(const char* ip_address, unsigned short port)
{
    if (client_socket != -1)
        return client_socket;
//	WSAData dummy;
//	WSAStartup(MAKEWORD(2, 0), &dummy);

	// czy jest proces?
//    STARTUPINFOA si;
//    PROCESS_INFORMATION pi;

//    ZeroMemory( &si, sizeof(si) );
//    si.cb = sizeof(si);
//    ZeroMemory( &pi, sizeof(pi) );
//
//	HWND h = FindWindowA(NULL, "Image Debugger :: Server");
//	if (h == 0)
//		CreateProcessA(NULL, exe_path, NULL, NULL, false, 0, NULL, NULL, &si, &pi);


	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = ntohs(port);
	addr.sin_addr.s_addr = inet_addr(ip_address);

	int s = ::socket(addr.sin_family, SOCK_STREAM, 0);

	if (s == -1)
		return -1;

	if (::connect(s, (sockaddr*)&addr, sizeof(sockaddr_in)) != 0)
		return -1;

	client_socket = s;
	return s;
}

bool IDBG_PrintString(int s, const char* format, ...)
{
	va_list arglist;
	va_start(arglist, format);

	char temp[1024];

	vsnprintf(temp, 1023, format, arglist);

	int to_send = strlen(temp);
	const char* ptr  = temp;

	while(to_send > 0)
	{
		int sent = ::send(s, ptr, strlen(ptr), 0);
		if (sent <= 0) {
		    IDBG_Disconect();
		    return false;
		}


		to_send -= sent;
		ptr += sent;
	}

	return true;
}
/*
API bool IDBG_ShowImage(const char* name, arma::fmat& mat, const char* palette)
{
	arma::fmat q = mat.st();
	q.eval();
	return IDBG_ShowImage(name, mat.n_rows, mat.n_cols, q.mem, "Float", palette);
}

API bool IDBG_ShowImage(const char* name, arma::mat& mat, const char* palette)
{
	arma::fmat q = arma::conv_to<arma::fmat>::from(mat).st();
	q.eval();
	return IDBG_ShowImage(name, mat.n_rows, mat.n_cols, q.mem, "Float", palette);
}


API bool IDBG_ShowImage(const char* name, arma::Mat<arma::u16>& mat, const char* palette)
{
	arma::Mat<arma::u16> q = mat.st();
	q.eval();
	return IDBG_ShowImage(name, mat.n_rows, mat.n_cols, q.memptr(), "U16", palette);
}
*/

 bool IDBG_ShowImage(const char* name, int h, int w, const void* data, const char* data_type, const char* palette) {
    int s = IDBG_Connection("127.0.0.1", 7391);
    if (s == -1)
        return false;

    if (palette == NULL)
        palette = "Gray";

    if (name == NULL)
        name = "";

    bool result;
    if (!nprintf(s, "name %s\n", name))
        return false;
    if (!nprintf(s, "height %d\n", h))
        return false;
    if (!nprintf(s, "width %d\n", w))
        return false;
    if (!nprintf(s, "ctype %s\n", data_type))
        return false;
    if (!nprintf(s, "itype %s\n", "Luminance"))
        return false;
    if (!nprintf(s, "palette %s\n", palette))
        return false;
    if (!nprintf(s, "data\n"))
        return false;

    int to_send = w * h;

    if (strcmp(data_type, "Float") == 0)
        to_send *= sizeof(float);
    if (strcmp(data_type, "U32") == 0)
        to_send *= sizeof(unsigned int);
    if (strcmp(data_type, "U16") == 0)
        to_send *= sizeof(unsigned short);
    if (strcmp(data_type, "U8") == 0)
        to_send *= sizeof(unsigned char);


    while (to_send > 0) {
        int sent = ::send(s, (char *) data, to_send, 0);
        if (sent <= 0) {
            IDBG_Disconect();
            return false;
        }

        to_send -= sent;
        data = (unsigned char *) data + sent;
    }

    return true;
}
