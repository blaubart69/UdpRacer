///////////////////////////////////////////////////////////////////////////////////////////////////
//
//               U D P  R A C E R
//
//
//  FILE:        udpracer.c
//
//  PURPOSE:     A test udp server link implementation
//
//
//               Copyright (c) 2013 M. B., Vienna (AT)
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#define  UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used 
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "commom.h"
#include "udprcmsg.h"

#include <mstcpip.h>

// defines

#define NUMBER_OF_MAX_CHAIN     63
#define NUMBER_OF_RECV_BUFFERS  80
//efine NUMBER_OF_WORK_THREADS  16
#define NUMBER_OF_CONCURRENT     0

#define MAX_ULONG 0xFFFFFFFF

// types

typedef __declspec( align( 64 )) struct _RECV {
    WSAOVERLAPPED WsaOv;  // must be first !
    WSABUF        WsaBuf;
    SOCKET        hSocket;
    SOCKADDR_IN   RecvAddrIn;
    DWORD         RecvFlags;
    BOOL          fReceive;
} RECV;

typedef struct _DATA {
    DWORD         Id;
    DWORD         Hops;
    DWORD         Next;
    DWORD         Last;
    SOCKADDR_IN   NextAddrIn[ NUMBER_OF_MAX_CHAIN ];
} DATA;

// functions

PRIVATE VOID    Message( DWORD MsgId, ... );
PRIVATE BOOL    GetUL( ULONG* pResult, LPWSTR s );
PRIVATE LPWSTR* CommandLineToArgv2( LPCWSTR CmdLine, INT *argc );

DWORD WINAPI IoThread( LPVOID lpParam );
BOOL  WINAPI CtrlHandler( DWORD dwCtrlType );

// global

HANDLE  _hExit = NULL;
RECV    _Recv[ NUMBER_OF_RECV_BUFFERS ];

__declspec( align( 64 )) LONG volatile _Packets;

//=================================================================================================
VOID rawmain( VOID ) {
//=================================================================================================

    LPWSTR    *szArglist = NULL;
    INT       nArgs, rc = 999;
    WSADATA   WsaData;
    SOCKET    hSocket = INVALID_SOCKET;
    SOCKADDR_IN SockAddrIn;
    HANDLE    hCompletionPort = NULL;
    HANDLE    *hThread;
    DWORD     dwThreadId;
    ULONG     i, nThreads;

    WsaData.wVersion = 0;

    nThreads = 4;
    _Packets = 0;

    ZeroMemory( &SockAddrIn, sizeof( SockAddrIn ));
    SockAddrIn.sin_family      = AF_INET;
    SockAddrIn.sin_addr.s_addr = INADDR_ANY;
    SockAddrIn.sin_port        = htons( 5001 );

    if (( szArglist = CommandLineToArgv2( GetCommandLine(), &nArgs )) == NULL ) {
        rc = 998;
    } else if ( nArgs < 1 && nArgs > 2 ) {
        Message( MSGID_USAGE, TEXT( FILE_VERSION_UDPLINKS ), TEXT( __DATE__ ), TEXT( COPYRIGHTTEXT ));
    } else if (( nArgs == 2 ) && ( ! GetUL( &nThreads, szArglist[ 1 ] ))) {
        Message( MSGID_USAGE, TEXT( FILE_VERSION_UDPLINKS ), TEXT( __DATE__ ), TEXT( COPYRIGHTTEXT ));
    } else if (( _hExit = CreateEvent( NULL, TRUE, FALSE, NULL )) == NULL ) {
        Message( MSGID_ERROR_WINAPI_S, TEXT( __FILE__ ), __LINE__
               , L"CreateEvent", L"Manual", GetLastError());
        rc = 1;
    } else if ( ! SetConsoleCtrlHandler( CtrlHandler, TRUE )) {
        Message( MSGID_ERROR_WINAPI_S, TEXT( __FILE__ ), __LINE__
               , L"SetConsoleCtrlHandler", L"", GetLastError());
        rc = 1;
    } else if ( WSAStartup( MAKEWORD( 2, 2 ), &WsaData ) != 0 ) {
        Message( MSGID_ERROR_WINAPI_S, TEXT( __FILE__ ), __LINE__
               , L"WSAStartup", L"2.2", WSAGetLastError());
        WsaData.wVersion = 0;
        rc = 1;
    } else if (( hSocket = WSASocket( AF_INET
                                    , SOCK_DGRAM
                                    , IPPROTO_UDP
                                    , NULL
                                    , 0
                                    , WSA_FLAG_OVERLAPPED )) == INVALID_SOCKET ) {
        Message( MSGID_ERROR_WINAPI_S, TEXT( __FILE__ ), __LINE__
               , L"WSASocket", L"INET/UDP", WSAGetLastError());
        rc = 2;
    } else if ( bind( hSocket, (SOCKADDR*)&SockAddrIn, sizeof( SockAddrIn )) != 0 ) {
        Message( MSGID_ERROR_WINAPI_U, TEXT( __FILE__ ), __LINE__
               , L"bind", 5001, WSAGetLastError());
        rc = 3;
#ifdef SOCKOPT
    } else if ( setsockopt( hSocket, SOL_SOCKET, SO_SNDBUF, (char *)&Zero, sizeof( Zero )) != 0 ) {
        Message( MSGID_ERROR_WINAPI_S, TEXT( __FILE__ ), __LINE__
               , L"setsockopt", L"SO_SNDBUF", WSAGetLastError());
        rc = 4;
    } else if ( setsockopt( hSocket, SOL_SOCKET, SO_RCVBUF, (char *)&Zero, sizeof( Zero )) != 0 ) {
        Message( MSGID_ERROR_WINAPI_S, TEXT( __FILE__ ), __LINE__
               , L"setsockopt", L"SO_RCVBUF", WSAGetLastError());
        rc = 4;
#endif
    } else if (( hCompletionPort = CreateIoCompletionPort( (HANDLE)hSocket
                                                         , NULL
                                                         , 1
                                                         , NUMBER_OF_CONCURRENT )) == NULL ) {
        Message( MSGID_ERROR_WINAPI_U, TEXT( __FILE__ ), __LINE__
               , L"CreateIoCompletionPort", NUMBER_OF_CONCURRENT, GetLastError());
        rc = 5;
    } else {

        // --- let's queue a lot of receives ------------------------------------------------------

        for ( i = 0; i < NUMBER_OF_RECV_BUFFERS; i++ ) {

            DWORD NumberOfBytesRecvd;
            INT   RecvAddrInSize = sizeof( _Recv[ 0 ].RecvAddrIn );

            ZeroMemory( &( _Recv[ i ].WsaOv ), sizeof( _Recv[ i ].WsaOv ));

            _Recv[ i ].hSocket = hSocket;  // don't use hEvent
            _Recv[ i ].fReceive = TRUE;
            _Recv[ i ].WsaBuf.len = sizeof( DATA );

            if (( _Recv[ i ].WsaBuf.buf = VirtualAlloc( NULL
                                                           , _Recv[ i ].WsaBuf.len
                                                           , MEM_COMMIT
                                                           , PAGE_READWRITE )) == NULL ) {
                Message( MSGID_ERROR_WINAPI_U, TEXT( __FILE__ ), __LINE__
                       , L"VirtualAlloc", i, WSAGetLastError());
            } else if (( WSARecvFrom( hSocket
                                    , &( _Recv[ i ].WsaBuf )
                                    , 1
                                    , &NumberOfBytesRecvd
                                    , &( _Recv[ i ].RecvFlags )
                                    , (SOCKADDR*)&( _Recv[ i ].RecvAddrIn )
                                    , &RecvAddrInSize
                                    , &( _Recv[ i ].WsaOv )
                                    , NULL ) != 0 ) && ( WSAGetLastError() != WSA_IO_PENDING )) {
                Message( MSGID_ERROR_WINAPI_U, TEXT( __FILE__ ), __LINE__
                       , L"WSARecvFrom", i, WSAGetLastError());
            } /* endif */
        } /* endfor */

        // --- let's create a number of working threads -------------------------------------------

        hThread = _alloca( nThreads * sizeof( hThread[ 0 ] ));

        for ( i = 0; i < nThreads; i++ ) {
            if (( hThread[ i ] = CreateThread( NULL
                                             , 0
                                             , IoThread
                                             , (LPVOID)hCompletionPort
                                             , 0
                                             , &dwThreadId )) == NULL ) {
                Message( MSGID_ERROR_WINAPI_U, TEXT( __FILE__ ), __LINE__
                       , L"CreateThread", i, GetLastError());
            } /* endif */
        } /* endfor */

        // --- wait and report performance --------------------------------------------------------

        while ( WaitForSingleObject( _hExit, 1000L ) == WAIT_TIMEOUT ) {
            Message( MSGID_INFO_PKTS, _InterlockedExchange( &_Packets, 0 ));
        } /* endwhile */
        
        // --- send a exitmessage to the worker ---------------------------------------------------

        for ( i = 0; i < nThreads; i++ ) {
            PostQueuedCompletionStatus( hCompletionPort, 0, 0, NULL );
        } /* endfor */

        // --- wait for all worker threads (but not more than one second) -------------------------

        WaitForMultipleObjects( nThreads
                              , hThread
                              , TRUE
                              , 1000 );

        // --- close a lot of handles -------------------------------------------------------------

        for ( i = 0; i < nThreads; i++ ) {
            if ( hThread[ i ] != NULL ) {
                CloseHandle( hThread[ i ] );
            } /* endif */
        } /* endfor */

        // --- free memory ------------------------------------------------------------------------

        for ( i = 0; i < NUMBER_OF_RECV_BUFFERS; i++ ) {
            if ( _Recv[ i ].WsaBuf.buf != NULL ) {
                VirtualFree( _Recv[ i ].WsaBuf.buf, 0, MEM_RELEASE );
            } /* endif */
        } /* endfor */

    } /* endif */

    if ( hCompletionPort  != NULL           ) CloseHandle( hCompletionPort );
    if ( hSocket          != INVALID_SOCKET ) closesocket( hSocket );
    if ( WsaData.wVersion != 0              ) WSACleanup();
    if ( _hExit           != NULL           ) CloseHandle( _hExit );
    if ( szArglist        != NULL           ) HeapFree( GetProcessHeap(), 0, szArglist );

    ExitProcess( rc );
}

//=================================================================================================
BOOL WINAPI CtrlHandler( DWORD dwCtrlType ) {
//=================================================================================================

    BOOL fHandled;

    if ( dwCtrlType == CTRL_C_EVENT ) {
        fHandled = SetEvent( _hExit ) ? TRUE : FALSE;
    } else {
        fHandled = FALSE;
    } /* endif */

    return fHandled;
}

//=================================================================================================
DWORD WINAPI IoThread( LPVOID lpParam ) {
//=================================================================================================

    HANDLE hCompletionPort = (HANDLE)lpParam;
    BOOL   bSuccess;
    DWORD  dwIoSize, dwCompletionKey;
    RECV   *Recv;
    DATA   *Data;

    for (;;) {

        // --- wait for a job ---------------------------------------------------------------------

        bSuccess = GetQueuedCompletionStatus( hCompletionPort
                                            , &dwIoSize
                                            , &dwCompletionKey
                                            , (OVERLAPPED**)&Recv
                                            , INFINITE );

        // --- check for termination --------------------------------------------------------------

        if ( dwCompletionKey == 0 ) {

            break;

        } else if ( dwIoSize != sizeof( *Data )) {

            ;

        } else if ( Recv->fReceive ) {

            DWORD NumberOfBytesSent;

            Recv->fReceive = FALSE;

            Data = (DATA*)Recv->WsaBuf.buf;

            Data->Hops++;
            Data->Next = ( Data->Next + 1 ) % Data->Last;

            _InterlockedIncrement( &_Packets );

            if (( WSASendTo( Recv->hSocket
                           , &( Recv->WsaBuf )
                           , 1
                           , &NumberOfBytesSent
                           , 0
                           , (SOCKADDR*)&( Data->NextAddrIn[ Data->Next ] )
                           , sizeof( Data->NextAddrIn[ 0 ] )
                           , &( Recv->WsaOv )
                           , NULL ) != 0 ) && ( WSAGetLastError() != WSA_IO_PENDING )) { 
                Message( MSGID_ERROR_WINAPI_U, TEXT( __FILE__ ), __LINE__
                       , L"WSASendTo", -1, WSAGetLastError());
            } /* endif */

        } else {

            DWORD NumberOfBytesRecvd;
            INT   RecvAddrInSize = sizeof( Recv->RecvAddrIn );

            Recv->fReceive = TRUE;

            Data = (DATA*)Recv->WsaBuf.buf;

            if (( WSARecvFrom( Recv->hSocket
                             , &( Recv->WsaBuf )
                             , 1
                             , &NumberOfBytesRecvd
                             , &( Recv->RecvFlags )
                             , (SOCKADDR*)&( Recv->RecvAddrIn )
                             , &RecvAddrInSize
                             , &( Recv->WsaOv )
                             , NULL ) != 0 ) && ( WSAGetLastError() != WSA_IO_PENDING )) {
                Message( MSGID_ERROR_WINAPI_U, TEXT( __FILE__ ), __LINE__
                       , L"WSARecvFrom", -1, WSAGetLastError());
            } /* endif */

        } /* endif */

    } /* endfor */

    return 0;
}

//-------------------------------------------------------------------------------------------------
PRIVATE VOID Message( DWORD MsgId, ... ) {
//-------------------------------------------------------------------------------------------------

    HANDLE hStdOut;
    DWORD  FileType, NumberOfCharsWritten;
    DWORD  MsgCntX;
    LPTSTR MsgBufX = NULL;
    DWORD  MsgCntY;
    LPSTR  MsgBufY = NULL;

    va_list vaList;
    va_start( vaList, MsgId );

    if (( MsgCntX = FormatMessage(   FORMAT_MESSAGE_ALLOCATE_BUFFER
                                   | FORMAT_MESSAGE_FROM_HMODULE
                                 , GetModuleHandle( NULL )
                                 , MsgId
                                 , 0
                                 , (LPTSTR)&MsgBufX
                                 , 0
                                 , &vaList )) == 0 ) {
        Beep( 2000, 100 );
    } else if (( hStdOut = GetStdHandle( STD_OUTPUT_HANDLE )) == INVALID_HANDLE_VALUE ) {
        Beep( 2000, 100 );
    } else if (( FileType = GetFileType( hStdOut )) == FILE_TYPE_CHAR ) {
        WriteConsole( hStdOut, MsgBufX, MsgCntX, &NumberOfCharsWritten, NULL );
    } else if (( MsgBufY = HeapAlloc( GetProcessHeap()
                                    , 0
                                    , MsgCntY = MsgCntX * 3 )) == NULL ) {
        Beep( 2000, 100 );
    } else if (( MsgCntY = WideCharToMultiByte( GetConsoleOutputCP()
                                              , 0
                                              , MsgBufX
                                              , MsgCntX
                                              , MsgBufY
                                              , MsgCntY
                                              , NULL
                                              , NULL )) == 0 ) {
        Beep( 2000, 100 );
    } else {
        WriteFile( hStdOut, MsgBufY, MsgCntY, &NumberOfCharsWritten, NULL );
    } /* endif */

    if ( MsgBufY != NULL ) HeapFree( GetProcessHeap(), 0, MsgBufY );
    if ( MsgBufX != NULL ) LocalFree( MsgBufX );

    va_end( vaList );
}

//------------------------------------------------------------------------------------------------
PRIVATE BOOL GetUL( ULONG* pResult, LPWSTR s ) {
//------------------------------------------------------------------------------------------------

    WCHAR c;
    ULONG rc = 0, base;

    // this is a "special" strtoul.

    c = *s++;

    if ( c == L'\0' ) return FALSE;

    if ( c == L'0' && ( *s == L'x' || *s == L'X' )) {
        c = s[ 1 ]; s += 2;
        base = 16;
    } else if ( c == L'0' && ( *s == L'b' || *s == L'B' )) {
        c = s[ 1 ]; s += 2;
        base = 2;
    } else if ( c == L'#' ) {
        c = *s++;
        base = 16;
    } else {
        base = ( c == L'0' ) ? 8 : 10;
    } /* endif */

    while ( c != L'\0' ) {
        if ( rc > ( MAX_ULONG / base )) return FALSE;
        else if ( c >= L'0' && c <= L'1' ) rc = rc * base + ( c - L'0' );
        else if ( base ==  2 ) return FALSE;
        else if ( c >= L'2' && c <= L'7' ) rc = rc * base + ( c - L'0' );
        else if ( base ==  8 ) return FALSE;
        else if ( c >= L'8' && c <= L'9' ) rc = rc * base + ( c - L'0' );
        else if ( base == 10 ) return FALSE;
        else if ( c >= L'A' && c <= L'F' ) rc = rc * base + ( c - L'A' ) + 10;
        else if ( c >= L'a' && c <= L'f' ) rc = rc * base + ( c - L'a' ) + 10;
        else return FALSE;
        c = *s++;
    } /* endfor */

    *pResult = rc;

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
PRIVATE LPWSTR* CommandLineToArgv2( LPCWSTR CmdLine, INT *argc ) {
//-------------------------------------------------------------------------------------------------

    //
    // This function is almost identical to the CommandLineToArgvW from the shell32.dll
    // Since this is the only function from this DLL we break this dependency and implemented
    // our own function.
    //
    // The special interpretation of backslash characters has NOT be implemented and we use the
    // HeapAlloc() functions, so use HeapFree() instead of LocalFree().
    //

    LPWSTR  *argv;
    LPWSTR  arg_v;
    ULONG   arg_c;
    ULONG   len;
    WCHAR   a;
    ULONG   i;

    BOOLEAN InQuot;
    BOOLEAN InText;

    len = (ULONG)lstrlen( CmdLine );

    // --- we calculate a upper boundary of argc ---

    i = ((( len + 1 ) / 2 ) + 1 ) * sizeof( argv[ 0 ] );
    //                        ^---- for argv[argc]=NULL
    //                  ^---------- for every arg you need min 2 char (argument und deliminator)
    //            ^---------------- round up

    // --- allocate the copy of the commandline and the argv array at once ---

    argv = (LPWSTR*)HeapAlloc( GetProcessHeap(), 0, i + ( len + 2 ) * sizeof( CmdLine[ 0 ] ));

    if ( argv == NULL ) {

        Message( MSGID_ERROR_WINAPI_U, TEXT( __FILE__ ), __LINE__
               , L"HeapAlloc", i + ( len + 2 ) * sizeof( CmdLine[ 0 ] ), ERROR_NOT_ENOUGH_MEMORY );
        return NULL;

    } /* endif */

    // --- the copy of the commandline tokens starts after the argv array ---

    arg_v = (LPWSTR)( (PUCHAR)argv + i );
    arg_c = 0;

    argv[ 0 ] = arg_v;

    InQuot = FALSE;
    InText = FALSE;

    i = 0;

    while (( a = CmdLine[ i ] ) != L'\0' ) {

        if ( InQuot ) {

            if ( a == L'\"' ) {

                // --- special "" handling ---

                if ( CmdLine[ i + 1 ] == L'\"' ) {
                    *( arg_v++ ) = a; /* Double quote inside quoted string */
                    i++;
                } else {
                    InQuot = FALSE;
                } /* endif */

            } else {
                *( arg_v++ ) = a;
            } /* endif */

        } else {

            switch ( a ) {

                case L' ' :  // "white"
                case L'\t':
                case L'\n':
                case L'\r': {
                    if ( InText ) {
                        *( arg_v++ ) = L'\0';
                        InText = FALSE;
                    } /* endif */
                    break;
                }

                case L'\"': {
                    InQuot = TRUE;
                    if ( ! InText ) {
                        argv[ arg_c++ ] = arg_v;
                        InText = TRUE;
                    } /* endif */
                    break;
                }

                default: {
                    if ( ! InText ) {
                        argv[ arg_c++ ] = arg_v;
                        InText = TRUE;
                    } /* endif */
                    *( arg_v++ ) = a;
                    break;
                }

            } /* endswitch */
        } /* endif */

        i++;
    }

    // --- terminate the last token ---

    *arg_v = L'\0';

    // --- fill in additional empty argument ---

    argv[ arg_c ] = NULL;

    // --- give back the local results ---

    (*argc) = arg_c;

    return argv;
}

// EOF
