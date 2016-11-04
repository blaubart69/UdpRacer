///////////////////////////////////////////////////////////////////////////////////////////////////
//
//               U D P  R A C E R
//
//
//  FILE:        udpstart.c
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
#include "udpstmsg.h"

#include <mstcpip.h>

// defines

#define NUMBER_OF_MAX_CHAIN 63

// types

typedef struct _DATA {
    DWORD         Id;
    DWORD         Hops;
    DWORD         Next;
    DWORD         Last;
    SOCKADDR_IN   NextAddrIn[ NUMBER_OF_MAX_CHAIN ];
} DATA;

// functions

PRIVATE BOOL FillData( DATA *Data, LPWSTR *szArglist );
PRIVATE VOID Message( DWORD MsgId, ... );
PRIVATE LPWSTR* CommandLineToArgv2( LPCWSTR CmdLine, INT *argc );

DATA _Data;

//=================================================================================================
VOID rawmain( VOID ) {
//=================================================================================================

    LPWSTR  *szArglist = NULL;
    INT     nArgs, rc = 999;
    WSADATA WsaData;
    SOCKET  hSocket = INVALID_SOCKET;
    WSABUF  WsaBuf;
    DWORD   NumberOfBytesSent;

    WsaData.wVersion = 0;

    WsaBuf.len = sizeof( _Data );
    WsaBuf.buf = (CHAR*)&_Data;

    if (( szArglist = CommandLineToArgv2( GetCommandLine(), &nArgs )) == NULL ) {
        rc = 998;
    } else if ( nArgs <= 1 ) {
        Message( MSGID_USAGE, TEXT( FILE_VERSION_UDPLINKS ), TEXT( __DATE__ ), TEXT( COPYRIGHTTEXT ));
    } else if ( WSAStartup( MAKEWORD( 2, 2 ), &WsaData ) != 0 ) {
        Message( MSGID_ERROR_WINAPI_S, TEXT( __FILE__ ), __LINE__
               , L"WSAStartup", L"2.2", WSAGetLastError());
        WsaData.wVersion = 0;
        rc = 1;
    } else if ( ! FillData( &_Data, szArglist + 1 )) {
        ;
    } else if (( hSocket = WSASocket( AF_INET
                                    , SOCK_DGRAM
                                    , IPPROTO_UDP
                                    , NULL
                                    , 0
                                    , 0 )) == INVALID_SOCKET ) {
        Message( MSGID_ERROR_WINAPI_S, TEXT( __FILE__ ), __LINE__
               , L"WSASocket", L"INET/UDP", WSAGetLastError());
        rc = 2;
    } else if ( WSASendTo( hSocket
                         , &WsaBuf
                         , 1
                         , &NumberOfBytesSent
                         , 0
                         , (SOCKADDR*)_Data.NextAddrIn
                         , sizeof( _Data.NextAddrIn[ 0 ] )
                         , NULL
                         , NULL ) != 0 ) { 
        Message( MSGID_ERROR_WINAPI_U, TEXT( __FILE__ ), __LINE__
               , L"WSASendTo", WsaBuf.len, WSAGetLastError());
        rc = 3;
    } else {
        rc = 0;
    } /* endif */

    if ( hSocket != INVALID_SOCKET ) closesocket( hSocket );
    if ( WsaData.wVersion != 0 ) WSACleanup();
    if ( szArglist != NULL ) HeapFree( GetProcessHeap(), 0, szArglist );

    ExitProcess( rc );
}

//-------------------------------------------------------------------------------------------------
PRIVATE BOOL FillData( DATA *Data, LPWSTR *szArglist ) {
//-------------------------------------------------------------------------------------------------

    DWORD i = 0;

    for (;;) {

        INT AddrLen = sizeof( Data->NextAddrIn[ i ] );

        Data->NextAddrIn[ i ].sin_family = AF_INET;

        if ( WSAStringToAddress( *szArglist
                               , AF_INET
                               , NULL
                               , (SOCKADDR*)( Data->NextAddrIn + i )
                               , &AddrLen ) != 0 ) {
            Message( MSGID_ERROR_WINAPI_S, TEXT( __FILE__ ), __LINE__
                   , L"WSAStringToAddress", *szArglist, WSAGetLastError());
            return FALSE;
        } else if ( szArglist[ 1 ] == NULL ) {
            Data->NextAddrIn[ i ].sin_port = htons( 5001 );
            break;
        } else if ( i >= NUMBER_OF_MAX_CHAIN ) {
            return FALSE;
        } else {
            Data->NextAddrIn[ i ].sin_port = htons( 5001 );
            szArglist++;
            i++;
        } /* endif */

    } /* endfor */

    Data->Hops = 0;
    Data->Last = i + 1;
    Data->Next = 0;
    Data->Id   = 0x6619FEAF;

    return TRUE;
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
