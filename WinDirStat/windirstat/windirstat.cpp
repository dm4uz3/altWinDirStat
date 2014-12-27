// windirstat.cpp	- Implementation of CDirstatApp and some globals
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2005 Bernhard Seifert
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: bseifert@users.sourceforge.net, bseifert@daccord.net
//
// Last modified: $Date$

#include "stdafx.h"
#include "graphview.h"
#include "SelectDrivesDlg.h"
#include "dirstatdoc.h"
#include "options.h"
#include "windirstat.h"
#include "mainframe.h"
#include "globalhelpers.h"

CMainFrame* GetMainFrame( ) {
	// Not: `return (CMainFrame *)AfxGetMainWnd();` because CWinApp::m_pMainWnd is set too late.
	return CMainFrame::GetTheFrame( );
	}

CDirstatApp* GetApp( ) {
	return static_cast< CDirstatApp* >( AfxGetApp( ) );
	}


namespace {
#ifdef DEBUG
	void setFlags( ) {
		const auto flag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
		TRACE( _T( "CrtDbg state: %i\r\n\t_CRTDBG_ALLOC_MEM_DF: %i\r\n\t_CRTDBG_CHECK_CRT_DF: %i\r\n\t_CRTDBG_LEAK_CHECK_DF: %i\r\n\t_CRTDBG_DELAY_FREE_MEM_DEF: %i\r\n" ), flag, ( flag & _CRTDBG_ALLOC_MEM_DF ), ( flag & _CRTDBG_CHECK_CRT_DF ), ( flag & _CRTDBG_LEAK_CHECK_DF ), ( flag & _CRTDBG_DELAY_FREE_MEM_DF ) );
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF /*| _CRTDBG_CHECK_CRT_DF*/ );
		const auto flag2 = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
		TRACE( _T( "CrtDbg state: %i\r\n\t_CRTDBG_ALLOC_MEM_DF: %i\r\n\t_CRTDBG_CHECK_CRT_DF: %i\r\n\t_CRTDBG_LEAK_CHECK_DF: %i\r\n\t_CRTDBG_DELAY_FREE_MEM_DEF: %i\r\n" ), flag2, ( flag2 & _CRTDBG_ALLOC_MEM_DF ), ( flag2 & _CRTDBG_CHECK_CRT_DF ), ( flag2 & _CRTDBG_LEAK_CHECK_DF ), ( flag2 & _CRTDBG_DELAY_FREE_MEM_DF )  );
		}
#endif

	PCWSTR about_text = L"\r\naltWinDirStat - a fork of 'WinDirStat' Windows Directory Statistics\r\n\r\nShows where all your disk space has gone\r\nand helps you clean it up.\r\n\r\n(originally)Re-programmed for MS Windows by\r\nBernhard Seifert,\r\n\r\nbased on Stefan Hundhammer's KDE (Linux) program KDirStat\r\n(http://kdirstat.sourceforge.net/).\r\n\r\n\r\n\r\n\r\n\r\nLATER modified by Alexander Riccio\r\n\r\nabout.me/ariccio or ariccio.com\r\nsee gpl-2.0.txt for license ( GNU GENERAL PUBLIC LICENSE Version 2, June 1991 )";

	}


// CDirstatApp

BEGIN_MESSAGE_MAP(CDirstatApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
END_MESSAGE_MAP()


WTL::CAppModule _Module;	// add this line
CDirstatApp _theApp;


CDirstatApp::CDirstatApp( ) : m_workingSet( 0 ), m_lastPeriodicalRamUsageUpdate( GetTickCount64( ) ), m_altEncryptionColor( GetAlternativeColor( RGB( 0x00, 0x80, 0x00 ), _T( "AltEncryptionColor" ) ) ) { }

CDirstatApp::~CDirstatApp( ) {
	m_pDocTemplate = { NULL };
	}


void CDirstatApp::UpdateRamUsage( ) {
	CWinThread::OnIdle( 0 );
	}

void CDirstatApp::PeriodicalUpdateRamUsage( ) {
	if ( GetTickCount64( ) - m_lastPeriodicalRamUsageUpdate > RAM_USAGE_UPDATE_INTERVAL ) {
		UpdateRamUsage( );
		m_lastPeriodicalRamUsageUpdate = GetTickCount64( );
		}
	}



// Get the alternative colors for compressed and encrypted files/folders. This function uses either the value defined in the Explorer configuration or the default color values.
_Success_( return != clrDefault ) COLORREF CDirstatApp::GetAlternativeColor( _In_ const COLORREF clrDefault, _In_z_  PCWSTR which ) {
	COLORREF x;
	ULONG cbValue = sizeof( x );
	CRegKey key;

	// Open the explorer key
	key.Open( HKEY_CURRENT_USER, _T( "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer" ), KEY_READ );

	// Try to read the REG_BINARY value
	if ( ERROR_SUCCESS == key.QueryBinaryValue( which, &x, &cbValue ) ) {
		return x;
		}
	return clrDefault;
	}

_Success_( SUCCEEDED( return ) ) HRESULT CDirstatApp::GetCurrentProcessMemoryInfo( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_usage, _In_range_( 50, 64 ) rsize_t strSize ) {
	const auto Memres = UpdateMemoryInfo( );
	if ( !Memres ) {
		write_MEM_INFO_ERR( psz_formatted_usage );
		return STRSAFE_E_INVALID_PARAMETER;
		}
	write_RAM_USAGE( psz_formatted_usage );
	const HRESULT res = FormatBytes( m_workingSet, &( psz_formatted_usage[ 11 ] ), ( strSize - 12 ) );
	if ( !SUCCEEDED( res ) ) {
		return StringCchPrintfW( psz_formatted_usage, strSize, L"RAM Usage: %s", FormatBytes( m_workingSet, GetOptions( )->m_humanFormat ).c_str( ) );
		}
	return res;
	}

_Success_( return == true ) bool CDirstatApp::UpdateMemoryInfo( ) {
	auto pmc = zeroInitPROCESS_MEMORY_COUNTERS( );
	pmc.cb = sizeof( pmc );

	if ( !GetProcessMemoryInfo( GetCurrentProcess( ), &pmc, sizeof( pmc ) ) ) {
		return false;
		}	

	m_workingSet = pmc.WorkingSetSize;

	return true;
	}

BOOL CDirstatApp::InitInstance( ) {
	//Program entry point

	TRACE( _T( "------>Program entry point!<------\r\n" ) );
	//uses ~29K memory
	if ( !SUCCEEDED( CoInitializeEx( NULL, COINIT_APARTMENTTHREADED ) ) ) {
		AfxMessageBox( _T( "CoInitializeEx Failed!" ) );
		return FALSE;
		}
	
#ifdef DEBUG
	setFlags( );
#endif

	// Initialize ATL
	_Module.Init( NULL, AfxGetInstanceHandle( ) );

	VERIFY( CWinApp::InitInstance( ) );
	InitCommonControls( );          // InitCommonControls() is necessary for Windows XP.
	if ( AfxOleInit( ) == FALSE ) { // For SHBrowseForFolder()
		AfxMessageBox( _T( "AfxOleInit Failed!" ) );
		return FALSE;
		}
	
	

	SetRegistryKey( _T( "Seifert" ) );
	//LoadStdProfileSettings( 4 );

	GetOptions( )->LoadFromRegistry( );
	
	m_pDocTemplate = new CSingleDocTemplate { IDR_MAINFRAME, RUNTIME_CLASS( CDirstatDoc ), RUNTIME_CLASS( CMainFrame ), RUNTIME_CLASS( CGraphView ) };
	if ( !m_pDocTemplate ) {
		return FALSE;
		}

	AddDocTemplate( m_pDocTemplate );
	
	CCommandLineInfo cmdInfo;
	ParseCommandLine( cmdInfo );

	m_nCmdShow = SW_HIDE;
	
	if ( !ProcessShellCommand( cmdInfo ) ) {
		return FALSE;
		}

	GetMainFrame( )->InitialShowWindow( );
	m_pMainWnd->UpdateWindow( );

	// When called by setup.exe, windirstat remained in the background, so we do a
	m_pMainWnd->BringWindowToTop( );
	m_pMainWnd->SetForegroundWindow( );
	if ( cmdInfo.m_nShellCommand != CCommandLineInfo::FileOpen ) {
		OnFileOpen( );
		}
	return TRUE;
	}

INT CDirstatApp::ExitInstance( ) {
	// Terminate ATL
	_Module.Term( );	
	const auto retval = CWinApp::ExitInstance( );
	//delete m_pDocTemplate;
	//m_pDocTemplate = NULL;
	//_CrtDumpMemoryLeaks( );
	return retval;
	}

void CDirstatApp::OnAppAbout( ) {
	//CString text;
	//text.FormatMessage( IDS_ABOUT_ABOUTTEXTss );
	//displayWindowsMsgBoxWithMessage( text );
	//StartAboutDialog( );
	displayWindowsMsgBoxWithMessage( std::move( std::wstring( about_text ) ) );
	}

void CDirstatApp::OnFileOpen( ) {
	CSelectDrivesDlg dlg;
	if ( IDOK == dlg.DoModal( ) ) {
		const auto path = EncodeSelection( static_cast<RADIO>( dlg.m_radio ), std::wstring( dlg.m_folderName.GetString( ) ), dlg.m_drives );
		if ( path.find( '|' ) == std::wstring::npos ) {
			m_pDocTemplate->OpenDocumentFile( path.c_str( ), true );
			}
		}
	}

BOOL CDirstatApp::OnIdle( _In_ LONG lCount ) {
	BOOL more = FALSE;
	ASSERT( lCount >= 0 );
	const auto ramDiff = ( GetTickCount64( ) - m_lastPeriodicalRamUsageUpdate );
	auto doc = GetDocument( );
	if ( doc != NULL ) {
		if ( !doc->Work( ) ) {
			ASSERT( doc->m_workingItem != NULL );
			more = TRUE;
			}
		else {
			//more |= CWinThread::OnIdle( 0 );
			}
		}
	
	if ( ramDiff > RAM_USAGE_UPDATE_INTERVAL ) {
		more = CWinApp::OnIdle( lCount );
		if ( !more ) {
			GetApp( )->PeriodicalUpdateRamUsage( );
			}
		else {
			more = CWinThread::OnIdle( 0 );
			}
		}
	return more;
	}

// $Log$
// Revision 1.16  2005/04/17 12:27:21  assarbad
// - For details see changelog of 2005-04-17
//
// Revision 1.15  2005/04/10 16:49:30  assarbad
// - Some smaller fixes including moving the resource string version into the rc2 files
//
// Revision 1.14  2004/12/19 10:52:39  bseifert
// Minor fixes.
//
// Revision 1.13  2004/11/28 14:40:06  assarbad
// - Extended CFileFindWDS to replace a global function
// - Now packing/unpacking the file attributes. This even spares a call to find encrypted/compressed files.
//
// Revision 1.12  2004/11/25 11:58:52  assarbad
// - Minor fixes (odd behavior of coloring in ANSI version, caching of the GetCompressedFileSize API)
//   for details see the changelog.txt
//
// Revision 1.11  2004/11/14 08:49:06  bseifert
// Date/Time/Number formatting now uses User-Locale. New option to force old behavior.
//
// Revision 1.10  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.9  2004/11/10 01:03:00  assarbad
// - Style cleaning of the alternative coloring code for compressed/encrypted items
//
// Revision 1.8  2004/11/08 00:46:26  assarbad
// - Added feature to distinguish compressed and encrypted files/folders by color as in the Windows 2000/XP explorer.
//   Same rules apply. (Green = encrypted / Blue = compressed)
//
// Revision 1.7  2004/11/05 16:53:08  assarbad
// Added Date and History tag where appropriate.
//
