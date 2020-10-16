#if !defined(_IMAGE_DEBUGGER_SERVER_)
#define _IMAGE_DEBUGGER_SERVER_

//#include <armadillo>

//typedef int SOCKET;

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IMAGEDEBUGGERCLIENT_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// IMAGEDEBUGGERCLIENT_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
//#ifdef IMAGEDEBUGGERCLIENT_EXPORTS
//#define API __declspec(dllexport)
//#else
//#define API __declspec(dllimport)
//#endif
#define API

API int IDBG_Connection(const char* ip_address, unsigned short port);
API bool IDBG_ShowImage(const char* name, int h, int w, const void* data, const char* data_type, const char* palette = NULL);
//API bool IDBG_ShowImage(const char* name, arma::fmat& mat, const char* palette = NULL);

//API bool IDBG_ShowImage(const char* name, arma::mat& mat, const char* palette = NULL);
//API bool IDBG_ShowImage(const char* name, arma::Mat<arma::u16>& mat, const char* palette = NULL);


#define nprintf IDBG_PrintString


#endif // _IMAGE_DEBUGGER_SERVER_
