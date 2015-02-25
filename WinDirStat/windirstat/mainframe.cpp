// mainframe.cpp	- Implementation of CMySplitterWnd, CPacmanControl and CMainFrame
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_MAINFRAME_CPP
#define WDS_MAINFRAME_CPP

#include "graphview.h"
#include "dirstatview.h"


//encourage inter-procedural optimization (and class-hierarchy analysis!)
#include "ownerdrawnlistcontrol.h"
#include "TreeListControl.h"
#include "item.h"
#include "typeview.h"

#include "pagetreemap.h"
#include "pagegeneral.h"
#include "mainframe.h"
#include "dirstatdoc.h"
#include "options.h"
#include "windirstat.h"


#include "globalhelpers.h"
#include "ScopeGuard.h"

namespace {
	// This must be synchronized with the IDR_MAINFRAME menu
	enum TOPLEVELMENU {
		TLM_FILE,
		TLM_EDIT,
		//TLM_CLEANUP,
		TLM_TREEMAP,
		//TLM_REPORT,
		TLM_VIEW,
		TLM_HELP
		};


	void failed_to_open_clipboard( ) {
		displayWindowsMsgBoxWithError( );
		displayWindowsMsgBoxWithMessage( L"Cannot open the clipboard." );
		TRACE( _T( "Cannot open the clipboard!\r\n" ) );
		}

	class COpenClipboard {
		public:
		COpenClipboard( const COpenClipboard& in ) = delete;
		COpenClipboard& operator=( const COpenClipboard& in ) = delete;

		COpenClipboard( CWnd* const owner, const bool empty ) : m_open { owner->OpenClipboard( ) } {
			if ( !m_open ) {
				failed_to_open_clipboard( );
				return;
				}
			if ( empty ) {
				if ( !EmptyClipboard( ) ) {
					displayWindowsMsgBoxWithError( );
					displayWindowsMsgBoxWithMessage( L"Cannot empty the clipboard." );
					TRACE( _T( "Cannot empty the clipboard!\r\n" ) );
					}
				}
			}
		~COpenClipboard( ) {
			if ( m_open ) {
				VERIFY( CloseClipboard( ) );
				}
			}
		private:
		const BOOL m_open;
		};

	const UINT indicators[ ] = { ID_SEPARATOR, ID_INDICATOR_MEMORYUSAGE };
	
	template<size_t count>
	void SetIndicators( CStatusBar& status_bar, const UINT( &indicators_array )[ count ] ) {
		static_assert( sizeof( indicators_array ) == ( count * sizeof( UINT ) ), "Bad SetIndicators argument!" );
		VERIFY( status_bar.SetIndicators( indicators_array, count ) );
		}

	const rsize_t debug_str_size = 100u;
	
	void debug_output_searching_time( _In_ const double searchingTime ) {
		_Null_terminated_ wchar_t searching_done_str[ debug_str_size ] = { 0 };
		const auto printf_res_1 = _snwprintf_s( searching_done_str, debug_str_size, _TRUNCATE, L"WDS: searching time: %f\r\n", searchingTime );
		ASSERT( printf_res_1 != -1 );
		OutputDebugStringW( searching_done_str );

#ifndef DEBUG
		UNREFERENCED_PARAMETER( printf_res_1 );
#endif
		}
	
	void debug_output_frequency( _In_ const std::int64_t m_frequency ) {
		_Null_terminated_ wchar_t freq_str[ debug_str_size ] = { 0 };
		const auto printf_res_3 = _snwprintf_s( freq_str, debug_str_size, _TRUNCATE, L"WDS: timing frequency: %lld\r\n", m_frequency );
		ASSERT( printf_res_3 != -1 );
		OutputDebugStringW( freq_str );
#ifndef DEBUG
		UNREFERENCED_PARAMETER( printf_res_3 );
#endif
		}

	void debug_output_time_to_draw_empty_window( _In_ const double timeToDrawEmptyWindow ) {
		_Null_terminated_ wchar_t drawing_done_str[ debug_str_size ] = { 0 };
		const auto printf_res_4 = _snwprintf_s( drawing_done_str, debug_str_size, _TRUNCATE, L"WDS: time to draw window:   %f\r\n", timeToDrawEmptyWindow );
		ASSERT( printf_res_4 != -1 );
		OutputDebugStringW( drawing_done_str );
#ifndef DEBUG
		UNREFERENCED_PARAMETER( printf_res_4 );
#endif
		}

	void output_debugging_info( _In_ const double searchingTime, _In_ const std::int64_t frequency, _In_ const double timeToDrawEmptyWindow ) {
		debug_output_searching_time( searchingTime );
		debug_output_frequency( frequency );
		debug_output_time_to_draw_empty_window( timeToDrawEmptyWindow );
		}

	}



/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( COptionsPropertySheet, CPropertySheet )

BOOL COptionsPropertySheet::OnInitDialog( ) {
	const BOOL bResult = CPropertySheet::OnInitDialog( );

	RECT rc;

	/*
_AFXWIN_INLINE void CWnd::GetWindowRect(LPRECT lpRect) const
	{ ASSERT(::IsWindow(m_hWnd)); ::GetWindowRect(m_hWnd, lpRect); }
	*/

	ASSERT( ::IsWindow( m_hWnd ) );

	//If [GetWindowRect] succeeds, the return value is nonzero.
	VERIFY( ::GetWindowRect( m_hWnd, &rc ) );

	WTL::CPoint pt = CRect( rc ).TopLeft( );
	
	CPersistence::GetConfigPosition( pt );
	CRect rc2( pt, CRect( rc ).Size( ) );

	ASSERT( m_pCtrlSite == NULL );

	ASSERT( ::IsWindow( m_hWnd ) );

	//If [MoveWindow] succeeds, the return value is nonzero.
	VERIFY( ::MoveWindow( m_hWnd, rc2.left, rc2.top, ( rc2.right - rc2.left ), ( rc2.bottom - rc2.top ), TRUE ) );

	VERIFY( SetActivePage( CPersistence::GetConfigPage( GetPageCount( ) - 1 ) ) );
	return bResult;
	}

BOOL COptionsPropertySheet::OnCommand( _In_ WPARAM wParam, _In_ LPARAM lParam ) {
	CPersistence::SetConfigPage( GetActiveIndex( ) );

	CRect rc;
	GetWindowRect( rc );
	CPersistence::SetConfigPosition( rc.TopLeft( ) );
	ASSERT( m_pCtrlSite == NULL );
	//INT cmd = LOWORD( wParam );
	return CPropertySheet::OnCommand( wParam, lParam );
	}

/////////////////////////////////////////////////////////////////////////////




BEGIN_MESSAGE_MAP( CMySplitterWnd, CSplitterWnd )
	ON_WM_SIZE( )
END_MESSAGE_MAP( )

//CMySplitterWnd::CMySplitterWnd( _In_z_ PCWSTR const name ) : m_persistenceName( name ), m_splitterPos( 0.5 ), m_wasTrackedByUser( false ), m_userSplitterPos( 0.5 ) {
//	CPersistence::GetSplitterPos( m_persistenceName, m_wasTrackedByUser, m_userSplitterPos );
//	}


void CMySplitterWnd::StopTracking( _In_ BOOL bAccept ) {
	CSplitterWnd::StopTracking( bAccept );
	if ( !bAccept ) {
		return;
		}
	RECT rcClient = { 0, 0, 0, 0 };
	ASSERT( ::IsWindow( m_hWnd ) );
	
	//"If [GetClientRect] succeeds, the return value is nonzero. To get extended error information, call GetLastError."
	VERIFY( ::GetClientRect( m_hWnd, &rcClient ) );
	
	auto guard = WDS_SCOPEGUARD_INSTANCE( [ &] { CPersistence::SetSplitterPos( m_persistenceName, m_wasTrackedByUser, m_userSplitterPos ); } );

	INT dummy = 0;
	if ( GetColumnCount( ) > 1 ) {
		INT cxLeft = 0;
		GetColumnInfo( 0, cxLeft, dummy );

		if ( ( rcClient.right - rcClient.left ) > 0 ) {
			m_splitterPos = static_cast< DOUBLE >( cxLeft ) / static_cast< DOUBLE >( rcClient.right - rcClient.left );
			}
		m_wasTrackedByUser = true;
		m_userSplitterPos = m_splitterPos;
		return;
		}
	INT cyUpper = 0;
	GetRowInfo( 0, cyUpper, dummy );
	if ( ( rcClient.bottom - rcClient.top ) > 0 ) {
		m_splitterPos = static_cast< DOUBLE >( cyUpper ) / static_cast< DOUBLE >( rcClient.bottom - rcClient.top );
		}
	m_wasTrackedByUser = true;
	m_userSplitterPos = m_splitterPos;
	}

void CMySplitterWnd::SetSplitterPos( _In_ const DOUBLE pos ) {
	m_splitterPos = pos;
	RECT rcClient = { 0, 0, 0, 0 };

	ASSERT( ::IsWindow( m_hWnd ) );
	//"If [GetClientRect] succeeds, the return value is nonzero. To get extended error information, call GetLastError."
	VERIFY( ::GetClientRect( m_hWnd, &rcClient ) );

	//auto splitter_persist = scopeGuard( [&]{ CPersistence::SetSplitterPos( m_persistenceName, m_wasTrackedByUser, m_userSplitterPos ); }, __FILE__, __FUNCSIG__, __LINE__ );
	//WDS_SCOPEGUARD_INSTANCE
	auto splitter_persist = WDS_SCOPEGUARD_INSTANCE( [&]{ CPersistence::SetSplitterPos( m_persistenceName, m_wasTrackedByUser, m_userSplitterPos ); } );
	if ( GetColumnCount( ) > 1 ) {
		ASSERT( m_pColInfo != NULL );
		if ( m_pColInfo == NULL ) {
			return;
			}
		const auto cxLeft = static_cast< INT >( pos * ( rcClient.right - rcClient.left ) );
		if ( cxLeft >= 0 ) {
			SetColumnInfo( 0, cxLeft, 0 );
			RecalcLayout( );
			return;
			}
		return;
		}
	ASSERT( m_pRowInfo != NULL );
	if ( m_pRowInfo == NULL ) {
		return;
		}
	const auto cyUpper = static_cast< INT >( pos * ( rcClient.bottom - rcClient.top ) );
	if ( cyUpper >= 0 ) {
		SetRowInfo( 0, cyUpper, 0 );
		RecalcLayout( );
		return;
		}
	}


//void CMySplitterWnd::RestoreSplitterPos( _In_ const DOUBLE default_pos ) {
//	SetSplitterPos( ( m_wasTrackedByUser ) ? m_userSplitterPos : default_pos );
//	}

void CMySplitterWnd::OnSize( const UINT nType, const INT cx, const INT cy ) {
	auto guard = WDS_SCOPEGUARD_INSTANCE( [&]{ CSplitterWnd::OnSize( nType, cx, cy ); } );
	if ( GetColumnCount( ) > 1 ) {
		const INT cxLeft = static_cast< INT >( cx * m_splitterPos );
		if ( cxLeft > 0 ) {
			SetColumnInfo( 0, cxLeft, 0 );
			return;
			}
		return;
		}
	const INT cyUpper = static_cast< INT >( cy * m_splitterPos );
	if ( cyUpper > 0 ) {
		SetRowInfo( 0, cyUpper, 0 );
		return;
		}
	}

LRESULT CDeadFocusWnd::OnKeyDown( UINT /*nMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled ) {
	const UINT nChar = static_cast<UINT>( wParam );
	if ( nChar == VK_TAB ) {
		m_frameptr->MoveFocus( LOGICAL_FOCUS::LF_DIRECTORYLIST );
		bHandled = TRUE;
		return 0;
		}
	bHandled = FALSE;
	return 0;
	}

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE( CMainFrame, CFrameWnd )

BEGIN_MESSAGE_MAP( CMainFrame, CFrameWnd )
	ON_WM_CREATE( )
	ON_MESSAGE( WM_ENTERSIZEMOVE, &( CMainFrame::OnEnterSizeMove ) )
	ON_MESSAGE( WM_EXITSIZEMOVE, &( CMainFrame::OnExitSizeMove ) )
	ON_WM_CLOSE( )
	ON_WM_INITMENUPOPUP( )
	ON_UPDATE_COMMAND_UI( ID_INDICATOR_MEMORYUSAGE, &( CMainFrame::OnUpdateMemoryUsage ) )
	ON_WM_SIZE( )
	ON_UPDATE_COMMAND_UI( ID_VIEW_SHOWTREEMAP, &( CMainFrame::OnUpdateViewShowtreemap ) )
	ON_COMMAND( ID_VIEW_SHOWTREEMAP, &( CMainFrame::OnViewShowtreemap ) )
	ON_UPDATE_COMMAND_UI( ID_VIEW_SHOWFILETYPES, &( CMainFrame::OnUpdateViewShowfiletypes ) )
	ON_COMMAND( ID_VIEW_SHOWFILETYPES, &( CMainFrame::OnViewShowfiletypes ) )
	ON_COMMAND( ID_CONFIGURE, &( CMainFrame::OnConfigure ) )
	ON_WM_DESTROY( )
	//ON_COMMAND(ID_TREEMAP_HELPABOUTTREEMAPS, OnTreemapHelpabouttreemaps)
	ON_WM_SYSCOLORCHANGE( )
END_MESSAGE_MAP( )

CMainFrame* CMainFrame::_theFrame;

_Ret_maybenull_
CMainFrame* CMainFrame::GetTheFrame( ) {
	return _theFrame;
	}


INT CMainFrame::OnCreate( const LPCREATESTRUCT lpCreateStruct ) {
	/*
	Initializes the MAIN frame - wherein the rectangular layout, the list of files, and the list of file types are.
	Initializes a few related things, such as the memory display.
	*/

	_theFrame = this;
	m_appptr = GetApp( );
	if ( CFrameWnd::OnCreate( lpCreateStruct ) == -1 ) {
		return -1;
		}


	VERIFY( m_wndStatusBar.Create( this ) );
	SetIndicators( m_wndStatusBar, indicators );

	RECT rc = { 0, 0, 0, 0 };

	m_wndDeadFocus.Create( m_hWnd, rc, _T( "_deadfocus" ), WS_CHILD, 0, dead_focus_wnd::IDC_DEADFOCUS, NULL );
	//m_wndDeadFocus.Create( this );

	EnableDocking( CBRS_ALIGN_ANY );

	LoadBarState( CPersistence::GetBarStateSection( ) );
	ShowControlBar( &m_wndStatusBar, CPersistence::GetShowStatusbar( ), false );
	
	TRACE( _T( "sizeof CItemBranch: %I64u\r\n" ), static_cast< std::uint64_t >( sizeof( CItemBranch ) ) );
#ifdef DISPLAY_FINAL_CITEMBRANCH_SIZE
#ifndef DEBUG
	if ( IsDebuggerPresent( ) ) {
		const auto size_citembranch = std::to_wstring( sizeof( CItemBranch ) );
		const std::wstring size_text( L"sizeof CItemBranch: " + size_citembranch );
		displayWindowsMsgBoxWithMessage( size_text.c_str( ) );
		}
#endif
#endif
	return 0;
	}

void CMainFrame::InitialShowWindow( ) {
	auto wp = zero_init_struct<WINDOWPLACEMENT>( );
	wp.length = sizeof( wp );
	VERIFY( GetWindowPlacement( &wp ) );
	CPersistence::GetMainWindowPlacement( wp );
	//MakeSaneShowCmd( wp.showCmd );
	if ( wp.showCmd != SW_SHOWMAXIMIZED ) {
		wp.showCmd = SW_SHOWNORMAL;
		}
	VERIFY( SetWindowPlacement( &wp ) );
	}

void CMainFrame::OnClose( ) {
	TRACE( _T( "CMainFrame::OnClose!\r\n" ) );
	const auto qpc_1 = help_QueryPerformanceCounter( );
	WTL::CWaitCursor wc;

	// It's too late, to do this in OnDestroy(). Because the toolbar, if undocked, is already destroyed in OnDestroy(). So we must save the toolbar state here in OnClose().
	SaveBarState( CPersistence::GetBarStateSection( ) );
	CPersistence::SetShowStatusbar( ( m_wndStatusBar.GetStyle( ) bitand WS_VISIBLE ) != 0 );

#ifdef _DEBUG
	// avoid memory leaks and show hourglass while deleting the tree
	VERIFY( GetDocument( )->OnNewDocument( ) );
#endif

	const auto Document = GetDocument( );
	if ( Document != NULL ) {
		Document->ForgetItemTree( );
		}
	CFrameWnd::OnClose( );
	const auto qpc_2 = help_QueryPerformanceCounter( );
	const auto qpf = help_QueryPerformanceFrequency( );
	const auto timing = ( static_cast< double >( qpc_2.QuadPart - qpc_1.QuadPart ) * ( static_cast< double >( 1.0 ) / static_cast< double >( qpf.QuadPart ) ) );
	ASSERT( timing != 0 );
#ifndef DEBUG
	UNREFERENCED_PARAMETER( timing );
#endif
	TRACE( _T( "OnClose timing: %f\r\n" ), timing );
	//auto pmc = zeroInitPROCESS_MEMORY_COUNTERS( );
	auto pmc = zero_init_struct<PROCESS_MEMORY_COUNTERS>( );
	pmc.cb = sizeof( pmc );

	if ( GetProcessMemoryInfo( GetCurrentProcess( ), &pmc, sizeof( pmc ) ) ) {
		TRACE( _T( "GetProcessMemoryInfo: %I64u\r\n" ), static_cast< std::uint64_t >( pmc.WorkingSetSize ) );
		}
	else {
		TRACE( _T( "GetProcessMemoryInfo failed!!\r\n" ) );
		}
	}

void CMainFrame::OnDestroy( ) {
	//auto wp = zeroInitWINDOWPLACEMENT( );
	auto wp = zero_init_struct<WINDOWPLACEMENT>( );
	GetWindowPlacement( &wp );
	CPersistence::SetMainWindowPlacement( wp );
	const auto TypeView = GetTypeView( );
	const auto GraphView = GetGraphView( );
	if ( TypeView != NULL ) {
		CPersistence::SetShowFileTypes( TypeView->m_showTypes );
		}
	if ( GraphView != NULL ) {
		CPersistence::SetShowTreemap( GraphView->m_showTreemap );
		}
	CFrameWnd::OnDestroy( );
	}

BOOL CMainFrame::OnCreateClient( LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext ) {
	const SIZE GraphView_size = { 100, 100 };
	const SIZE DirstatView_size = { 700, 500 };
	const SIZE TypeView_size = { 100, 500 };

	VERIFY( m_wndSplitter.CreateStatic( this, 2, 1 ) );
	VERIFY( m_wndSplitter.CreateView( 1, 0, RUNTIME_CLASS( CGraphView ), GraphView_size, pContext ) );

	VERIFY( m_wndSubSplitter.CreateStatic( &m_wndSplitter, 1, 2, WS_CHILD | WS_VISIBLE | WS_BORDER, static_cast< UINT >( m_wndSplitter.IdFromRowCol( 0, 0 ) ) ) );
	VERIFY( m_wndSubSplitter.CreateView( 0, 0, RUNTIME_CLASS( CDirstatView ), DirstatView_size, pContext ) );
	VERIFY( m_wndSubSplitter.CreateView( 0, 1, RUNTIME_CLASS( CTypeView ), TypeView_size, pContext ) );

	//MinimizeGraphView( );
	m_wndSplitter.SetSplitterPos( 1.0 );
	//MinimizeTypeView ( );
	m_wndSubSplitter.SetSplitterPos( 1.0 );

	const auto TypeView = GetTypeView( );
	const auto GraphView = GetGraphView( );
	if ( TypeView != NULL ) {
		TypeView->ShowTypes( CPersistence::GetShowFileTypes( ) );
		}
	if ( GraphView != NULL ) {
		GraphView->m_showTreemap = CPersistence::GetShowTreemap( );
		}
	return TRUE;
	}

void CMainFrame::RestoreTypeView( ) {
	const auto thisTypeView = GetTypeView( );
	if ( thisTypeView == NULL ) {
		return;
		}
	if ( thisTypeView->m_showTypes ) {
		m_wndSubSplitter.RestoreSplitterPos( 0.72 );
		VERIFY( thisTypeView->RedrawWindow( ) );
		}
	}


void CMainFrame::RestoreGraphView( ) {
	const auto thisGraphView = GetGraphView( );
	if ( thisGraphView == NULL ) {
		return;
		}
	if ( !( thisGraphView->m_showTreemap ) ) {
		return;
		}
	m_wndSplitter.RestoreSplitterPos( 0.4 );

	QPC_timer timer_draw_empty_view;
	timer_draw_empty_view.begin( );
	thisGraphView->DrawEmptyView( );
	timer_draw_empty_view.end( );
	const DOUBLE timeToDrawEmptyWindow = timer_draw_empty_view.total_time_elapsed( );
	TRACE( _T( "Done drawing empty view. Timing: %f\r\nDrawing treemap...\r\n" ), timeToDrawEmptyWindow );
	QPC_timer timer_draw_treemap;
	timer_draw_treemap.begin( );
	VERIFY( thisGraphView->RedrawWindow( ) );
	timer_draw_treemap.end( );

	const DOUBLE timeToDrawWindow = timer_draw_treemap.total_time_elapsed( );
	TRACE( _T( "Finished drawing treemap! Timing: %f\r\n" ), timeToDrawWindow );
	const auto comp_file_timing = GetDocument( )->m_compressed_file_timing;
	const auto searchingTime = GetDocument( )->m_searchTime;
	ASSERT( searchingTime > 0 );
	output_debugging_info( searchingTime, timer_draw_treemap.m_frequency, timeToDrawEmptyWindow );
	const auto avg_name_leng = GetDocument( )->m_rootItem->averageNameLength( );
	ASSERT( timeToDrawWindow != 0 );
	m_lastSearchTime = searchingTime;
	if ( m_lastSearchTime == -1 ) {
		m_lastSearchTime = searchingTime;
		ASSERT( m_lastSearchTime >= comp_file_timing );
		WriteTimeToStatusBar( timeToDrawWindow, m_lastSearchTime, avg_name_leng, comp_file_timing );//else the search time compounds whenever the time is written to the status bar
		}
	else {
		WriteTimeToStatusBar( timeToDrawWindow, m_lastSearchTime, avg_name_leng, comp_file_timing );
		}
	}

_Must_inspect_result_ _Ret_maybenull_ CDirstatView* CMainFrame::GetDirstatView( ) const {
	const auto pWnd = m_wndSubSplitter.GetPane( 0, 0 );
	return STATIC_DOWNCAST( CDirstatView, pWnd );
	}

//cannot be defined in header.
_Must_inspect_result_ _Ret_maybenull_ CGraphView* CMainFrame::GetGraphView( ) const {
	const auto pWnd = m_wndSplitter.GetPane( 1, 0 );
	return STATIC_DOWNCAST( CGraphView, pWnd );
	}

_Must_inspect_result_ _Ret_maybenull_ CTypeView* CMainFrame::GetTypeView( ) const {
	const auto pWnd = m_wndSubSplitter.GetPane( 0, 1 );
	return STATIC_DOWNCAST( CTypeView, pWnd );
	}

LRESULT CMainFrame::OnEnterSizeMove( const WPARAM, const LPARAM ) {
	const auto GraphView = GetGraphView( );
	if ( GraphView != NULL ) {
		GraphView->SuspendRecalculation( true );
		}
	return 0;
	}

LRESULT CMainFrame::OnExitSizeMove( const WPARAM, const LPARAM ) {
	const auto GraphView = GetGraphView( );
	if ( GraphView != NULL ) {
		GraphView->SuspendRecalculation( false );
		}
	return 0;
	}

void CMainFrame::CopyToClipboard( _In_ const std::wstring psz ) const {
	COpenClipboard clipboard( const_cast< CMainFrame* >( this ), true );
	const rsize_t strSizeInBytes = ( ( psz.length( ) + 1 ) * sizeof( WCHAR ) );

	const HGLOBAL handle_globally_allocated_memory = GlobalAlloc( GMEM_MOVEABLE bitand GMEM_ZEROINIT, strSizeInBytes );
	if ( handle_globally_allocated_memory == NULL ) {
		displayWindowsMsgBoxWithMessage( global_strings::global_alloc_failed );
		TRACE( L"%s\r\n", global_strings::global_alloc_failed );
		return;
		}

	const auto lp = GlobalLock( handle_globally_allocated_memory );
	if ( lp == NULL ) {
		displayWindowsMsgBoxWithMessage( L"GlobalLock failed!" );
		return;
		}

	auto strP = static_cast< PWSTR >( lp );

	const HRESULT strCopyRes = StringCchCopyW( strP, ( psz.length( ) + 1 ), psz.c_str( ) );
	if ( !SUCCEEDED( strCopyRes ) ) {
		if ( strCopyRes == STRSAFE_E_INVALID_PARAMETER ) {
			displayWindowsMsgBoxWithMessage( std::move( std::wstring( global_strings::string_cch_copy_failed ) + std::wstring( L"(STRSAFE_E_INVALID_PARAMETER)" ) ) );
			}
		if ( strCopyRes == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			displayWindowsMsgBoxWithMessage( std::move( std::wstring( global_strings::string_cch_copy_failed ) + std::wstring( L"(STRSAFE_E_INSUFFICIENT_BUFFER)" ) ) );
			}
		else {
			displayWindowsMsgBoxWithMessage( global_strings::string_cch_copy_failed );
			}
		const BOOL unlock_res = GlobalUnlock( handle_globally_allocated_memory );
		strP = NULL;
		const auto last_err = GetLastError( );
		if ( unlock_res == 0 ) {
			ASSERT( last_err == NO_ERROR );
#ifndef DEBUG
			UNREFERENCED_PARAMETER( last_err );
#endif

			}
		return;
		}

	if ( GlobalUnlock( handle_globally_allocated_memory ) == 0 ) {
		const auto err = GetLastError( );
		if ( err != NO_ERROR ) {
			displayWindowsMsgBoxWithMessage( L"GlobalUnlock failed!" );
			return;
			}
		}
	//wtf is going on here?
	UINT uFormat = CF_TEXT;
	uFormat = CF_UNICODETEXT;

	if ( NULL == SetClipboardData( uFormat, handle_globally_allocated_memory ) ) {
		displayWindowsMsgBoxWithMessage( global_strings::cannot_set_clipboard_data );
		TRACE( L"%s\r\n", global_strings::cannot_set_clipboard_data );
		return;
		}
	}

void CMainFrame::OnInitMenuPopup( CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu ) {
	CFrameWnd::OnInitMenuPopup( pPopupMenu, nIndex, bSysMenu );
	}


_At_( lf, _Pre_satisfies_( ( lf == LOGICAL_FOCUS::LF_NONE ) || ( lf == LOGICAL_FOCUS::LF_DIRECTORYLIST ) || ( lf == LOGICAL_FOCUS::LF_EXTENSIONLIST ) ) )
void CMainFrame::SetLogicalFocus( _In_ const LOGICAL_FOCUS lf ) {
	if ( lf != m_logicalFocus ) {
		m_logicalFocus = lf;
		SetSelectionMessageText( );

		//reinterpret_cast< CDocument* >( GetDocument( ) )->UpdateAllViews( NULL, HINT_SELECTIONSTYLECHANGED );
		GetDocument( )->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_SELECTIONSTYLECHANGED );
		}
	}
_At_( lf, _Pre_satisfies_( ( lf == LOGICAL_FOCUS::LF_NONE ) || ( lf == LOGICAL_FOCUS::LF_DIRECTORYLIST ) || ( lf == LOGICAL_FOCUS::LF_EXTENSIONLIST ) ) )
void CMainFrame::MoveFocus( _In_ const LOGICAL_FOCUS lf ) {
	if ( lf == LOGICAL_FOCUS::LF_NONE ) {
		SetLogicalFocus( LOGICAL_FOCUS::LF_NONE );
		m_wndDeadFocus.SetFocus( );
		}
	else if ( lf == LOGICAL_FOCUS::LF_DIRECTORYLIST ) {
		const auto DirstatView = GetDirstatView( );
		if ( DirstatView != NULL ) {
			DirstatView->SetFocus( );
			}
		}
	else if ( lf == LOGICAL_FOCUS::LF_EXTENSIONLIST ) {
		const auto TypeView = GetTypeView( );
		if ( TypeView != NULL ) {
			TypeView->SetFocus( );
			}
		}
	}

size_t CMainFrame::getExtDataSize( ) const {
	const auto Document = GetDocument( );
	if ( Document != NULL ) {
		return Document->GetExtensionRecords( )->size( );
		}
	return 0;
	}

void CMainFrame::valid_timing_to_write( _In_ const double populate_timing, _In_ const double draw_timing, _In_ const double average_extension_length, _In_ const double enum_timing, _In_ const double compressed_file_timing, _In_ const double total_time, _In_ const rsize_t ext_data_size, _In_ const double file_name_length, _Out_ _Post_z_ _Pre_writable_size_( buffer_size_init ) PWSTR buffer_ptr, const rsize_t buffer_size_init ) {
	const HRESULT fmt_res = StringCchPrintfW( buffer_ptr, buffer_size_init, _T( "File enumeration took %.3f sec. NTFS compressed file size processing took: %.3f sec. Drawing took %.3f sec. Populating 'file types' took %.3f sec. Total: %.4f sec. # file types: %u. Avg name len: %.2f. Avg extension len: %.2f." ), enum_timing, compressed_file_timing, draw_timing, populate_timing, total_time, unsigned( ext_data_size ), file_name_length, average_extension_length );
	ASSERT( SUCCEEDED( fmt_res ) );
	if ( SUCCEEDED( fmt_res ) ) {
		SetMessageText( buffer_ptr );
		m_drawTiming = buffer_ptr;
		return;
		}
	if ( fmt_res == STRSAFE_E_END_OF_FILE ) {
		SetMessageText( L"Couldn't set message text: STRSAFE_E_END_OF_FILE" );
		return;
		}
	if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		SetMessageText( L"Couldn't set message text: STRSAFE_E_INSUFFICIENT_BUFFER" );
		return;
		}
	if ( fmt_res == STRSAFE_E_INVALID_PARAMETER ) {
		SetMessageText( L"Couldn't set message text: STRSAFE_E_INVALID_PARAMETER" );
		return;
		}
	SetMessageText( L"Couldn't set message text, unknown error!" );

	}

void CMainFrame::invalid_timing_to_write( _In_ const double average_extension_length, _In_ const double enum_timing, _In_ const rsize_t ext_data_size, _Out_ _Post_z_ _Pre_writable_size_( buffer_size_init ) PWSTR buffer_ptr, const rsize_t buffer_size_init ) {
	const HRESULT fmt_res = StringCchPrintfW( buffer_ptr, buffer_size_init, _T( "I had trouble with QueryPerformanceCounter, and can't provide timing. # file types: %u. Avg name len: %.2f. Avg extension len: %.2f." ), unsigned( ext_data_size ), enum_timing, average_extension_length );
	ASSERT( SUCCEEDED( fmt_res ) );
	if ( SUCCEEDED( fmt_res ) ) {
		SetMessageText( buffer_ptr );
		m_drawTiming = buffer_ptr;
		return;
		}
	if ( fmt_res == STRSAFE_E_END_OF_FILE ) {
		SetMessageText( L"Couldn't set message text: STRSAFE_E_END_OF_FILE" );
		return;
		}
	if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		SetMessageText( L"Couldn't set message text: STRSAFE_E_INSUFFICIENT_BUFFER" );
		return;
		}
	if ( fmt_res == STRSAFE_E_INVALID_PARAMETER ) {
		SetMessageText( L"Couldn't set message text: STRSAFE_E_INVALID_PARAMETER" );
		return;
		}
	SetMessageText( L"Couldn't set message text, unknown error!" );
	}


_Pre_satisfies_( searchTiming >= compressed_file_timing )
void CMainFrame::WriteTimeToStatusBar( _In_ const double drawTiming, _In_ const DOUBLE searchTiming, _In_ const DOUBLE fileNameLength, _In_ const DOUBLE compressed_file_timing ) {
	/*
	  Negative values are assumed to be erroneous.
	*/
	ASSERT( searchTiming >= compressed_file_timing );
	const rsize_t buffer_size_init = 512u;

	//TODO: why are we using heap here?
	std::unique_ptr<_Null_terminated_ wchar_t[ ]> buffer_uniq_ptr = std::make_unique<_Null_terminated_ wchar_t[ ]>( buffer_size_init );

	PWSTR buffer_ptr = buffer_uniq_ptr.get( );

	DOUBLE populateTiming_scopeholder = -1;
	DOUBLE averageExtLeng_scopeholder = -1;
	const auto TypeView = GetTypeView( );
	if ( TypeView != NULL ) {
		populateTiming_scopeholder = TypeView->m_extensionListControl.m_adjustedTiming;
		averageExtLeng_scopeholder = TypeView->m_extensionListControl.m_averageExtensionNameLength;
		}
	else {
		//no typeview, no population done!
		populateTiming_scopeholder = 0.00;
		}

	const DOUBLE populateTiming = populateTiming_scopeholder;
	const DOUBLE averageExtLeng = averageExtLeng_scopeholder;
	ASSERT( searchTiming >= compressed_file_timing );
	const auto enum_timing = ( searchTiming - compressed_file_timing );
	ASSERT( searchTiming >= enum_timing );

	const auto extDataSize = getExtDataSize( );
	
	ASSERT( searchTiming > 0.00f );
	ASSERT( drawTiming > 0.00f );
	ASSERT( populateTiming > 0.00f );
	m_drawTiming.clear( );
	const auto total_time = ( searchTiming + drawTiming + populateTiming );
	if ( ( searchTiming >= 0.00 ) && ( drawTiming >= 0.00 ) && ( populateTiming >= 0.00 ) ) {
		valid_timing_to_write( populateTiming, drawTiming, averageExtLeng, enum_timing, compressed_file_timing, total_time, extDataSize, fileNameLength, buffer_ptr, buffer_size_init );
		return;
		}

	invalid_timing_to_write( averageExtLeng, enum_timing, extDataSize, buffer_ptr, buffer_size_init );
	return;
	}

void CMainFrame::SetSelectionMessageText( ) {
	switch ( m_logicalFocus ) {
			case LOGICAL_FOCUS::LF_NONE:
				return SetMessageText( m_drawTiming.c_str( ) );
			case LOGICAL_FOCUS::LF_DIRECTORYLIST:
				{
				const auto Document = GetDocument( );
				ASSERT( Document != NULL );
				if ( Document == NULL ) {
					SetMessageText( _T( "No document?" ) );
					return;
					}
				const auto Selection = Document->m_selectedItem;
				if ( Selection == NULL ) {
					SetMessageText( m_drawTiming.c_str( ) );
					return;
					}
				SetMessageText( Selection->GetPath( ).c_str( ) );
				return;
				}
				return;
			case LOGICAL_FOCUS::LF_EXTENSIONLIST:
				return SetMessageText( std::wstring( L'*' + GetDocument( )->m_highlightExtension ).c_str( ) );
		}
	}

void CMainFrame::OnUpdateMemoryUsage( CCmdUI *pCmdUI ) {
	pCmdUI->Enable( true );
	const rsize_t ramUsageStrBufferSize = 50;
	_Null_terminated_ wchar_t ramUsageStr[ ramUsageStrBufferSize ] = { 0 };
	const HRESULT res = m_appptr->GetCurrentProcessMemoryInfo( ramUsageStr, ramUsageStrBufferSize );
	if ( !SUCCEEDED( res ) ) {
		rsize_t chars_written = 0;
		wds_fmt::write_BAD_FMT( ramUsageStr, chars_written );
		}
	pCmdUI->SetText( ramUsageStr );
	}



void CMainFrame::OnSize( const UINT nType, const INT cx, const INT cy ) {
	CFrameWnd::OnSize( nType, cx, cy );

	if ( !IsWindow( m_wndStatusBar.m_hWnd ) ) {
		return;
		}
	CRect rc;
	m_wndStatusBar.GetItemRect( 0, rc );
	}

void CMainFrame::OnUpdateViewShowtreemap( CCmdUI *pCmdUI ) {
	const auto GraphView = GetGraphView( );
	ASSERT( GraphView != NULL );
	if ( GraphView == NULL ) {
		return;
		}
	pCmdUI->SetCheck( GraphView->m_showTreemap );
	}

void CMainFrame::OnViewShowtreemap( ) {
	const auto thisGraphView = GetGraphView( );
	ASSERT( thisGraphView != NULL );
	if ( thisGraphView == NULL ) {
		return;
		}
	thisGraphView->m_showTreemap = !thisGraphView->m_showTreemap;
	if ( thisGraphView->m_showTreemap ) {
		return RestoreGraphView( );
		}
	//MinimizeGraphView( );
	m_wndSplitter.SetSplitterPos( 1.0 );
	}

void CMainFrame::OnUpdateViewShowfiletypes( CCmdUI *pCmdUI ) {
	const auto TypeView = GetTypeView( );
	ASSERT( TypeView != NULL );
	if ( TypeView == NULL ) {
		return;
		}
	pCmdUI->SetCheck( TypeView->m_showTypes );
	}

void CMainFrame::OnViewShowfiletypes( ) {
	const auto thisTypeView = GetTypeView( );
	ASSERT( thisTypeView != NULL );
	if ( thisTypeView == NULL ) {
		return;
		}
	thisTypeView->ShowTypes( !thisTypeView->m_showTypes );
	if ( thisTypeView->m_showTypes ) {
		return RestoreTypeView( );
		}
	//MinimizeTypeView( );
	m_wndSubSplitter.SetSplitterPos( 1.0 );
	}

void CMainFrame::OnConfigure( ) {
	COptionsPropertySheet sheet;

	CPageGeneral  general( m_appptr );
	CPageTreemap  treemap;

	sheet.AddPage( &general );
	sheet.AddPage( &treemap );

	sheet.DoModal( );
	const auto Options = GetOptions( );
	Options->SaveToRegistry( );
	}

void CMainFrame::OnSysColorChange( ) {
	CFrameWnd::OnSysColorChange( );
	const auto DirstatView = GetDirstatView( );
	ASSERT( DirstatView != NULL );
	if ( DirstatView != NULL ) {
		DirstatView->SysColorChanged( );
		}
	
	const auto TypeView = GetTypeView( );
	ASSERT( TypeView != NULL );
	if ( TypeView != NULL ) {
		TypeView->SysColorChanged( );
		}
	
	}


#else

#endif