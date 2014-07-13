// OwnerDrawnListControl.cpp	- Implementation of COwnerDrawnListItem and COwnerDrawnListControl
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2004 Bernhard Seifert
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
#include "windirstat.h"
#include "treemap.h"		// CColorSpace
#include ".\ownerdrawnlistcontrol.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	const INT TEXT_X_MARGIN = 6;	// Horizontal distance of the text from the edge of the item rectangle

	const UINT LABEL_INFLATE_CX = 3;// How much the label is enlarged, to get the selection and focus rectangle
	const UINT LABEL_Y_MARGIN = 2;

	const UINT GENERAL_INDENT = 5;
}

/////////////////////////////////////////////////////////////////////////////

COwnerDrawnListItem::COwnerDrawnListItem( ) {
	}

COwnerDrawnListItem::~COwnerDrawnListItem( ) {
	}

void COwnerDrawnListItem::DrawColorWithTransparentBackground( _In_ CRect& rcRest, _In_ CImageList* il, _In_ CDC* pdc ) const {
	// Draw the color with transparent background
	auto thisHeight = rcRest.bottom - rcRest.top;
	CPoint pt( rcRest.left, rcRest.top + thisHeight / 2 - thisHeight / 2 );

	il->SetBkColor( CLR_NONE );
	il->Draw( pdc, 0, pt, ILD_NORMAL );
	
	}


void COwnerDrawnListItem::DrawHighlightedItemSelectionBackground( _In_ CRect& rcLabel, _In_ CRect& rc, _In_ COwnerDrawnListControl* list, _In_ CDC* pdc, _Inout_ COLORREF& textColor ) const {
	// Color for the text in a highlighted item (usually white)
	textColor = list->GetHighlightTextColor( );

	CRect selection = rcLabel;
	// Depending on "FullRowSelection" style
	if ( list->IsFullRowSelection( ) ) {
		selection.right = rc.right;
		}
	// Fill the selection rectangle background (usually dark blue)
	pdc->FillSolidRect( selection, list->GetHighlightColor( ) );
	
	}

void COwnerDrawnListItem::DrawLabel( _In_ COwnerDrawnListControl *list, _In_ CImageList *il, _In_ CDC *pdc, _In_ CRect& rc, _In_ const UINT state, _Inout_opt_ INT *width, _Inout_ INT *focusLeft, _In_ const bool indent ) const {
	/*
	  Draws an item label (icon, text) in all parts of the WinDirStat view. The rest is drawn by DrawItem()
	  */
	ASSERT_VALID( pdc );
	ASSERT( list != NULL );
	ASSERT( il != NULL );

	CRect rcRest = rc;
	// Increase indentation according to tree-level
	if ( indent ) {
		rcRest.left += GENERAL_INDENT;
		}

	// Prepare to draw the file/folder icon
	ASSERT( GetImage( ) < il->GetImageCount( ) );

	IMAGEINFO ii;
	il->GetImageInfo( GetImage( ), &ii );

	CRect rcImage( ii.rcImage );

	if ( width == NULL ) {
		DrawColorWithTransparentBackground( rcRest, il, pdc );
		}

	// Decrease size of the remainder rectangle from left
	rcRest.left += ( rcImage.right - rcImage.left );

	CSelectObject sofont( pdc, list->GetFont( ) );

	rcRest.DeflateRect( list->GetTextXMargin( ), 0 );

	auto rcLabel = rcRest;
	auto temp = GetText( 0 );
	ASSERT( rcLabel.Width( ) > 0 );
	ASSERT( rcLabel.right > rcLabel.left );
	pdc->DrawText( temp, rcLabel, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP );

	rcLabel.InflateRect( LABEL_INFLATE_CX, 0 );
	rcLabel.top = rcRest.top + LABEL_Y_MARGIN;
	rcLabel.bottom = rcRest.bottom - LABEL_Y_MARGIN;

	CSetBkMode bk( pdc, TRANSPARENT );
	COLORREF textColor = GetSysColor( COLOR_WINDOWTEXT );
	if ( width == NULL && ( state & ODS_SELECTED ) != 0 && ( list->HasFocus( ) || list->IsShowSelectionAlways( ) ) ) {
		DrawHighlightedItemSelectionBackground( rcLabel, rc, list, pdc, textColor );
		}
	else {
		textColor = GetItemTextColor( );
		// Use the color designated for this item
		// This is currently only for encrypted and compressed items
		}

	// Set text color for device context
	CSetTextColor stc( pdc, textColor );

	if ( width == NULL ) {
		// Draw the actual text	
		pdc->DrawText( GetText( 0 ), rcRest, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP );
		}

	rcLabel.InflateRect( 1, 1 );

	*focusLeft = rcLabel.left;

	if ( ( state & ODS_FOCUS ) != 0 && list->HasFocus( ) && width == NULL && !list->IsFullRowSelection( ) ) {
		pdc->DrawFocusRect( rcLabel );
		}

	if ( width == NULL ) {
		DrawAdditionalState( pdc, rcLabel );
		}

	rcLabel.left = rc.left;
	rc = rcLabel;

	if ( width != NULL ) {
		*width = ( rcLabel.right - rcLabel.left ) + 5; // Don't know, why +5
		}
	}

void COwnerDrawnListItem::DrawSelection( _In_ COwnerDrawnListControl *list, _In_ CDC *pdc, _In_ CRect rc, _In_ const UINT state ) const {
	//ASSERT_VALID( pdc );//has already been verified by all callers!!
	ASSERT( list != NULL );
	if ( !list->IsFullRowSelection( ) ) {
		return;
		}
	if ( !list->HasFocus( ) && !list->IsShowSelectionAlways( ) ) {
		return;
		}
	if ( ( state & ODS_SELECTED ) == 0 ) {
		return;
		}

	rc.DeflateRect( 0, LABEL_Y_MARGIN );
	pdc->FillSolidRect( rc, list->GetHighlightColor( ) );
	}

void COwnerDrawnListItem::DrawPercentage( _In_ CDC *pdc, _In_ CRect rc, _In_ const DOUBLE fraction, _In_ const COLORREF color ) const {
	ASSERT_VALID( pdc );
	const INT LIGHT = 198;	// light edge
	const INT DARK = 118;	// dark edge
	const INT BG = 225;		// background (lighter than light edge)

	const COLORREF light	= RGB(LIGHT, LIGHT, LIGHT);
	const COLORREF dark		= RGB(DARK, DARK, DARK);
	const COLORREF bg		= RGB(BG, BG, BG);

	CRect rcLeft = rc;
	rcLeft.right = ( INT ) ( rcLeft.left + rc.Width( ) * fraction );

	CRect rcRight = rc;
	rcRight.left = rcLeft.right;

	if ( rcLeft.right > rcLeft.left ) {
		pdc->Draw3dRect( rcLeft, light, dark );
		}
	rcLeft.DeflateRect(1, 1);
	if ( rcLeft.right > rcLeft.left ) {
		pdc->FillSolidRect( rcLeft, color );
		}
	if ( rcRight.right > rcRight.left ) {
		pdc->Draw3dRect( rcRight, light, light );
		}
	rcRight.DeflateRect(1, 1);
	if ( rcRight.right > rcRight.left ) {
		pdc->FillSolidRect( rcRight, bg );
		}
	}

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( COwnerDrawnListControl, CSortingListControl )

COwnerDrawnListControl::COwnerDrawnListControl( LPCTSTR name, INT rowHeight ) : CSortingListControl( name ) {
	ASSERT( rowHeight > 0 );
	m_rowHeight = rowHeight;
	m_showGrid = false;
	m_showStripes = false;
	m_showFullRowSelection = false;

	InitializeColors( );
	}

COwnerDrawnListControl::~COwnerDrawnListControl( ) {
	}


void COwnerDrawnListControl::OnColumnsInserted( ) {
	/*
	  This method MUST be called BEFORE the Control is shown.
	*/
	// The pacmen shall not draw over our header control.
	ModifyStyle( 0, WS_CLIPCHILDREN );

	// Where does the 1st Item begin vertically?
	if ( GetItemCount( ) > 0 ) {
		CRect rc;
		GetItemRect( 0, rc, LVIR_BOUNDS );
		m_yFirstItem = rc.top;
		}
	else {
		InsertItem( 0, _T( "_tmp" ), 0 );
		CRect rc;
		GetItemRect( 0, rc, LVIR_BOUNDS );
		DeleteItem( 0 );
		m_yFirstItem = rc.top;
		}

	LoadPersistentAttributes( );
	}

void COwnerDrawnListControl::SysColorChanged( ) {
	InitializeColors( );
	}

INT COwnerDrawnListControl::GetRowHeight( ) {
	return m_rowHeight;
	}

void COwnerDrawnListControl::ShowGrid( _In_ const bool show ) {
	m_showGrid = show;
	if ( IsWindow( m_hWnd ) ) {
		InvalidateRect( NULL );
		}
	}

void COwnerDrawnListControl::ShowStripes( _In_ const bool show ) {
	m_showStripes = show;
	if ( IsWindow( m_hWnd ) ) {
		InvalidateRect( NULL );
		}
	}

void COwnerDrawnListControl::ShowFullRowSelection( _In_ const bool show ) {
	m_showFullRowSelection = show;
	if ( IsWindow( m_hWnd ) ) {
		InvalidateRect( NULL );
		}
	}

bool COwnerDrawnListControl::IsFullRowSelection( ) const {
	return m_showFullRowSelection;
	}

// Normal window background color
COLORREF COwnerDrawnListControl::GetWindowColor( ) const {
	return m_windowColor;
	}

// Shaded window background color (for stripes)
COLORREF COwnerDrawnListControl::GetStripeColor( ) const {
	return m_stripeColor;
	}

// Highlight color if we have no focus
COLORREF COwnerDrawnListControl::GetNonFocusHighlightColor( ) const {
	return RGB( 190, 190, 190 );//RGB(120, 120, 120): more contrast
	}
	
// Highlight text color if we have no focus
COLORREF COwnerDrawnListControl::GetNonFocusHighlightTextColor( ) const {
	return RGB(0,0,0); // RGB(255,255,255): more contrast
	}

COLORREF COwnerDrawnListControl::GetHighlightColor( ) {
	if ( HasFocus( ) ) {
		return GetSysColor( COLOR_HIGHLIGHT );
		}
	else {
		return GetNonFocusHighlightColor( );
		}
	}

COLORREF COwnerDrawnListControl::GetHighlightTextColor( ) {
	if ( HasFocus( ) ) {
		return GetSysColor( COLOR_HIGHLIGHTTEXT );
		}
	else {
		return GetNonFocusHighlightTextColor( );
		}
	}

bool COwnerDrawnListControl::IsItemStripeColor( _In_ const INT i ) const {
	return ( m_showStripes && ( i % 2 != 0 ) );
	}

bool COwnerDrawnListControl::IsItemStripeColor( _In_ const COwnerDrawnListItem *item ) {
	return IsItemStripeColor(FindListItem(item));
	}

COLORREF COwnerDrawnListControl::GetItemBackgroundColor( _In_ const INT i ) {
	return ( IsItemStripeColor( i ) ? GetStripeColor( ) : GetWindowColor( ) );
	}

COLORREF COwnerDrawnListControl::GetItemBackgroundColor( _In_ const COwnerDrawnListItem *item ) {
	return GetItemBackgroundColor( FindListItem( item ) );
	}

COLORREF COwnerDrawnListControl::GetItemSelectionBackgroundColor( _In_ const INT i ) {
	bool selected = ( GetItemState( i, LVIS_SELECTED ) & LVIS_SELECTED ) != 0;
	if ( selected && IsFullRowSelection( ) && ( HasFocus( ) || IsShowSelectionAlways( ) ) ) {
		return GetHighlightColor( );
		}
	else {
		return GetItemBackgroundColor( i );
		}
	}

COLORREF COwnerDrawnListControl::GetItemSelectionBackgroundColor( _In_ const COwnerDrawnListItem *item ) {
	return GetItemSelectionBackgroundColor(FindListItem(item));
	}

COLORREF COwnerDrawnListControl::GetItemSelectionTextColor( _In_ const INT i ) {
	bool selected = (GetItemState(i, LVIS_SELECTED) & LVIS_SELECTED) != 0;
	if ( selected && IsFullRowSelection( ) && ( HasFocus( ) || IsShowSelectionAlways( ) ) ) {
		return GetHighlightTextColor( );
		}
	else {
		return GetSysColor( COLOR_WINDOWTEXT );
		}
	}

INT COwnerDrawnListControl::GetTextXMargin( ) {
	return TEXT_X_MARGIN;
	}

INT COwnerDrawnListControl::GetGeneralLeftIndent( ) {
	return GENERAL_INDENT;
	}

COwnerDrawnListItem *COwnerDrawnListControl::GetItem( _In_ const INT i ) {
	auto item = ( COwnerDrawnListItem * ) GetItemData( i );
	return item;
	}

INT COwnerDrawnListControl::FindListItem( _In_ const COwnerDrawnListItem *item ) {
	auto fi = zeroInitLVFINDINFO( );
	fi.flags = LVFI_PARAM;
	fi.lParam = ( LPARAM ) item;

	auto i = ( INT ) FindItem( &fi );

	return i;
	}

void COwnerDrawnListControl::InitializeColors( ) {
	// I try to find a good contrast to COLOR_WINDOW (usually white or light grey).
	// This is a result of experiments. 

	const DOUBLE diff = 0.07;		// Try to alter the brightness by diff.
	const DOUBLE threshold = 1.04;	// If result would be brighter, make color darker.
	m_windowColor = GetSysColor( COLOR_WINDOW );

	auto b = CColorSpace::GetColorBrightness( m_windowColor );

	if ( b + diff > threshold ) {
		b -= diff;
		}
	else {
		b += diff;
		if ( b > 1.0 ) {
			b = 1.0;
			}
		}

	m_stripeColor = CColorSpace::MakeBrightColor( m_windowColor, b );
	}

void COwnerDrawnListControl::DoDrawSubItemBecauseItCannotDrawItself( _In_ COwnerDrawnListItem* item, _In_ INT subitem, _In_ CDC& dcmem, _In_ CRect& rcDraw, _In_ LPDRAWITEMSTRUCT& pdis, _In_ bool showSelectionAlways, _In_ bool bIsFullRowSelection ) {
	item->DrawSelection( this, &dcmem, rcDraw, pdis->itemState );
	auto rcText = rcDraw;
	rcText.DeflateRect( TEXT_X_MARGIN, 0 );
	CSetBkMode bk( &dcmem, TRANSPARENT );
	CSelectObject sofont( &dcmem, GetFont( ) );
	auto s = item->GetText( subitem );
	UINT align = IsColumnRightAligned( subitem ) ? DT_RIGHT : DT_LEFT;


	//--------------------------------------
	// Get the correct color in case of compressed or encrypted items
	auto textColor = item->GetItemTextColor( );

	if ( ( pdis->itemState & ODS_SELECTED ) && ( showSelectionAlways || HasFocus( )) && ( bIsFullRowSelection ) ) {
		textColor = GetItemSelectionTextColor( pdis->itemID );
		}
	//--------------------------------------

	// Set the text color
	CSetTextColor tc( &dcmem, textColor );
	// Draw the (sub)item text
	dcmem.DrawText( s, rcText, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP | align );
	// Test: dcmem.FillSolidRect(rcDraw, 0);

	}

void COwnerDrawnListControl::DrawItem( _In_ LPDRAWITEMSTRUCT pdis ) {
	auto item = ( COwnerDrawnListItem * ) ( pdis->itemData );
	auto pdc = CDC::FromHandle( pdis->hDC );
	auto bIsFullRowSelection = IsFullRowSelection( );
	ASSERT_VALID( pdc );
	CRect rcItem( pdis->rcItem );
	if ( m_showGrid ) {
		rcItem.bottom--;
		rcItem.right--;
		}

	CDC dcmem; //compiler seems to vectorize this!
	/*
	CDC dcmem vectorization summary:
	VXORPS xmm0 against itself thrice - breaks dependencies {
	vxorps	xmm0, xmm0, xmm0
	}
	Creates object on stack {
	lea	ecx, DWORD PTR _dcmem$[esp+420]
	vmovdqu	XMMWORD PTR _dcmem$[esp+420], xmm0
	}
	Calls CDC constructor

	VXORPS -> vectorized XOR (on floating point)
	VMOVDQU -> Move Unaligned Double Quadword
	*/

	dcmem.CreateCompatibleDC( pdc );
	CBitmap bm;
	bm.CreateCompatibleBitmap( pdc, ( rcItem.right - rcItem.left ), ( rcItem.bottom - rcItem.top ) );
	CSelectObject sobm( &dcmem, &bm );

	dcmem.FillSolidRect( rcItem - rcItem.TopLeft( ), GetItemBackgroundColor( pdis->itemID ) ); //NOT vectorized!

	bool drawFocus = ( pdis->itemState & ODS_FOCUS ) != 0 && HasFocus( ) && bIsFullRowSelection; //partially vectorized

	CArray<INT, INT> order;
	std::vector<INT> orderVec;
	
	bool showSelectionAlways = IsShowSelectionAlways( );
	auto thisHeaderCtrl = GetHeaderCtrl( );//HORRENDOUSLY slow. Pessimisation of memory access, iterates (with a for loop!) over a map. MAXIMUM branch prediction failures! Maximum Bad Speculation stalls!

	orderVec.reserve( thisHeaderCtrl->GetItemCount( ) );
	order.SetSize( thisHeaderCtrl->GetItemCount( ) );
	thisHeaderCtrl->GetOrderArray( order.GetData( ), order.GetSize( ) );////TODO: BAD IMPLICIT CONVERSION HERE!!! BUGBUG FIXME

#ifdef DEBUG
	for ( INT i = 0; i < order.GetSize( ) - 1; ++i ) {
		if ( i != order[ i ] ) {
			TRACE( _T( "order[%i]: %i \r\n" ), i, order[ i ] );
			}
		}
#endif

	CRect rcFocus = rcItem;
	rcFocus.DeflateRect( 0, LABEL_Y_MARGIN - 1 );

	auto thisLoopSize = order.GetSize( );
	for ( INT i = 0; i < thisLoopSize; i++ ) {
		ASSERT( order[ i ] == i );
		auto subitem = order[ i ];
		CRect rc = GetWholeSubitemRect( pdis->itemID, subitem );
		CRect rcDraw = rc - rcItem.TopLeft( );
		INT focusLeft = rcDraw.left;
		if ( !item->DrawSubitem( subitem, &dcmem, rcDraw, pdis->itemState, NULL, &focusLeft ) ) {//if DrawSubItem returns true, item draws self. Therefore `!item->DrawSubitem` is true when item DOES NOT draw self
			DoDrawSubItemBecauseItCannotDrawItself( item, subitem, dcmem, rcDraw, pdis, showSelectionAlways, bIsFullRowSelection );
			}

		if ( focusLeft > rcDraw.left ) {
			if ( drawFocus && i > 0 ) {
				pdc->DrawFocusRect( rcFocus );
				}
			rcFocus.left = focusLeft;
			}
		rcFocus.right = rcDraw.right;
		pdc->BitBlt( ( rcItem.left + rcDraw.left ), ( rcItem.top + rcDraw.top ), ( rcDraw.right - rcDraw.left ), ( rcDraw.bottom - rcDraw.top ), &dcmem, rcDraw.left, rcDraw.top, SRCCOPY );
		}
	if ( drawFocus ) {
		pdc->DrawFocusRect( rcFocus );
		}
	}

bool COwnerDrawnListControl::IsColumnRightAligned( _In_ const INT col ) {
	HDITEM hditem = zeroInitHDITEM( );

	hditem.mask = HDI_FORMAT;
	GetHeaderCtrl( )->GetItem( col, &hditem );
	return ( hditem.fmt & HDF_RIGHT ) != 0;
	}

CRect COwnerDrawnListControl::GetWholeSubitemRect(_In_ const INT item, _In_ const INT subitem)
{
	CRect rc;
	rc.bottom = 0;
	rc.left   = 0;
	rc.right  = 0;
	rc.top    = 0;

	if ( subitem == 0 ) {
		// Special case column 0:
		// If we did GetSubItemRect(item 0, LVIR_LABEL, rc) and we have an image list, then we would get the rectangle excluding the image.
		HDITEM hditem = zeroInitHDITEM( );

		hditem.mask = HDI_WIDTH;
		GetHeaderCtrl( )->GetItem( 0, &hditem );

		VERIFY( GetItemRect( item, rc, LVIR_LABEL ) );
		rc.left = rc.right - hditem.cxy;
		}
	else {
		VERIFY( GetSubItemRect( item, subitem, LVIR_LABEL, rc ) );
		}

	if ( m_showGrid ) {
		rc.right--;
		rc.bottom--;
		}
	return rc;
}

bool COwnerDrawnListControl::HasFocus()
{
	return ::GetFocus() == m_hWnd;
}

bool COwnerDrawnListControl::IsShowSelectionAlways()
{
	return (GetStyle() & LVS_SHOWSELALWAYS) != 0;
}

INT COwnerDrawnListControl::GetSubItemWidth(_In_ COwnerDrawnListItem *item, _In_ const INT subitem)
{
	if ( item == NULL ) {
		return -1;
		}
	INT width = 0;

	CClientDC dc( this );
	CRect rc( 0, 0, 1000, 1000 );
	
	INT dummy = rc.left;
	if ( item->DrawSubitem( subitem, &dc, rc, 0, &width, &dummy ) ) {
		return width;
		}

	CString s = item->GetText( subitem );
	if ( s.IsEmpty( ) ) {
		return 0;
		}

	CSelectObject sofont( &dc, GetFont( ) );
	UINT align = IsColumnRightAligned( subitem ) ? DT_RIGHT : DT_LEFT;
	dc.DrawText( s, rc, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP | align );

	rc.InflateRect( TEXT_X_MARGIN, 0 );
	return rc.Width( );
}

void COwnerDrawnListControl::buildArrayFromItemsInHeaderControl( _In_ CArray<INT, INT>& columnOrder, _Inout_ CArray<INT, INT>& vertical ) {
	vertical.SetSize( GetHeaderCtrl( )->GetItemCount( ) + 1 );
	
	INT x = -GetScrollPos( SB_HORZ );
	auto hdi = zeroInitHDITEM( );

	hdi.mask = HDI_WIDTH;
	for ( INT i = 0; i < GetHeaderCtrl( )->GetItemCount( ); i++ ) {
		GetHeaderCtrl( )->GetItem( columnOrder[ i ], &hdi );
		x += hdi.cxy;
		vertical[ i ] = x;
		}
	}




BEGIN_MESSAGE_MAP(COwnerDrawnListControl, CSortingListControl)
	ON_WM_ERASEBKGND()
	ON_NOTIFY(HDN_DIVIDERDBLCLICKA, 0, OnHdnDividerdblclick)
	ON_NOTIFY(HDN_DIVIDERDBLCLICKW, 0, OnHdnDividerdblclick)
	ON_WM_VSCROLL()
	ON_NOTIFY(HDN_ITEMCHANGINGA, 0, OnHdnItemchanging)
	ON_NOTIFY(HDN_ITEMCHANGINGW, 0, OnHdnItemchanging)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()



BOOL COwnerDrawnListControl::OnEraseBkgnd( CDC* pDC ) {
	ASSERT_VALID( pDC );
	ASSERT( GetHeaderCtrl( )->GetItemCount( ) > 0 );

	// We should recalculate m_yFirstItem here (could have changed e.g. when the XP-Theme changed).
	if ( GetItemCount( ) > 0 ) {
		CRect rc;
		GetItemRect( GetTopIndex( ), rc, LVIR_BOUNDS );
		m_yFirstItem = rc.top;
		}
	// else: if we did the same thing as in OnColumnsCreated(), we get repaint problems.

	const COLORREF gridColor = RGB( 212, 208, 200 );

	CRect rcClient;
	GetClientRect( rcClient );

	CRect rcHeader;
	GetHeaderCtrl( )->GetWindowRect( rcHeader );
	ScreenToClient( rcHeader );

	CRect rcBetween = rcClient;// between header and first item
	rcBetween.top = rcHeader.bottom;
	rcBetween.bottom = m_yFirstItem;
	pDC->FillSolidRect( rcBetween, gridColor );

	CArray<INT, INT> columnOrder;
	columnOrder.SetSize( GetHeaderCtrl( )->GetItemCount( ) );
	GetColumnOrderArray( columnOrder.GetData( ), columnOrder.GetSize( ) );//TODO: BAD IMPLICIT CONVERSION HERE!!! BUGBUG FIXME

	CArray<INT, INT> vertical;

	buildArrayFromItemsInHeaderControl( columnOrder, vertical );

	if ( m_showGrid ) {
		CPen pen( PS_SOLID, 1, gridColor );
		CSelectObject sopen( pDC, &pen );

		auto rowHeight = GetRowHeight( );
		for ( auto y = ( m_yFirstItem + rowHeight - 1 ); y < rcClient.bottom; y += rowHeight ) {
			ASSERT( rowHeight == GetRowHeight( ) );
			pDC->MoveTo( rcClient.left, y );
			pDC->LineTo( rcClient.right, y );
			}

		auto verticalSize = vertical.GetSize( );
		for ( INT i = 0; i < verticalSize; i++ ) {
			ASSERT( verticalSize == vertical.GetSize( ) );
			pDC->MoveTo( ( vertical[ i ] - 1 ), rcClient.top );
			pDC->LineTo( ( vertical[ i ] - 1 ), rcClient.bottom );
			}
		}

	const INT gridWidth = m_showGrid ? 1 : 0;
	const COLORREF bgcolor = GetSysColor( COLOR_WINDOW );

	const INT lineCount = GetCountPerPage( ) + 1;
	const INT firstItem = GetTopIndex( );
	const INT lastItem = min( firstItem + lineCount, GetItemCount( ) ) - 1;

	ASSERT( GetItemCount( ) == 0 || firstItem < GetItemCount( ) );
	ASSERT( GetItemCount( ) == 0 || lastItem < GetItemCount( ) );
	ASSERT( GetItemCount( ) == 0 || lastItem >= firstItem );

	const INT itemCount= lastItem - firstItem + 1;

	CRect fill;
	fill.left = vertical[ vertical.GetSize( ) - 1 ];
	fill.right = rcClient.right;
	fill.top = m_yFirstItem;
	fill.bottom = fill.top + GetRowHeight( ) - gridWidth;
	for ( INT i = 0; i < itemCount; i++ ) {
		pDC->FillSolidRect( fill, bgcolor );
		fill.OffsetRect( 0, GetRowHeight( ) );
		}

	auto rowHeight = GetRowHeight( );
	INT top = fill.top;
	while (top < rcClient.bottom) {
		fill.top = top;
		fill.bottom = top + GetRowHeight( ) - gridWidth;
		
		INT left = 0;
		auto verticalSize = vertical.GetSize( );
		for ( INT i = 0; i < verticalSize; i++ ) {
			ASSERT( verticalSize == vertical.GetSize( ) );
			fill.left = left;
			fill.right = vertical[ i ] - gridWidth;
			pDC->FillSolidRect( fill, bgcolor );
			left = vertical[ i ];
			}
		fill.left = left;
		fill.right = rcClient.right;
		pDC->FillSolidRect( fill, bgcolor );

		ASSERT( rowHeight == GetRowHeight( ) );
		top += rowHeight;
		}
	return true;
	}

void COwnerDrawnListControl::OnHdnDividerdblclick( NMHDR *pNMHDR, LRESULT *pResult ) {
	CWaitCursor wc;
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	INT subitem= phdr->iItem;

	AdjustColumnWidth(subitem);

	*pResult = 0;
	}

void COwnerDrawnListControl::AdjustColumnWidth( _In_ const INT col ) {
	CWaitCursor wc;

	INT width = 10;
	auto itemCount = GetItemCount( );
	for ( INT i = 0; i < itemCount; i++ ) {
		ASSERT( itemCount == GetItemCount( ) );
		auto w = GetSubItemWidth( GetItem( i ), col );
		if ( w > width ) {
			width = w;
			}
		}
	SetColumnWidth( col, width + 5 );
	}

void COwnerDrawnListControl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);

	// Owner drawn list controls with LVS_EX_GRIDLINES don't repaint correctly when scrolled (under Windows XP). So we fource a complete repaint here.
	InvalidateRect(NULL);
}

void COwnerDrawnListControl::OnHdnItemchanging(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	// Unused: LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	Default();
	InvalidateRect( NULL );
	*pResult = 0;
}


// $Log$
// Revision 1.13  2004/11/15 00:29:25  assarbad
// - Minor enhancement for the coloring of compressed/encrypted items when not in "select full row" mode
//
// Revision 1.12  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.11  2004/11/12 00:47:42  assarbad
// - Fixed the code for coloring of compressed/encrypted items. Now the coloring spans the full row!
//
// Revision 1.10  2004/11/07 23:28:14  assarbad
// - Partial implementation for coloring of compressed/encrypted files
//
// Revision 1.9  2004/11/07 00:06:34  assarbad
// - Fixed minor bug with ampersand (details in changelog.txt)
//
// Revision 1.8  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
