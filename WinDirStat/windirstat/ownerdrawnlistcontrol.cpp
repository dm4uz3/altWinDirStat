// OwnerDrawnListControl.cpp	- Implementation of COwnerDrawnListItem and COwnerDrawnListCtrl
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


#pragma once

#include "stdafx.h"

#include "treemap.h"		// CColorSpace
#include "ownerdrawnlistcontrol.h"
#include "globalhelpers.h"



INT COwnerDrawnListItem::CompareS( _In_ const COwnerDrawnListItem* const other, _In_ const SSorting& sorting ) const {
	auto r = Compare( other, sorting.column1 );
	if ( abs( r ) < 2 && !sorting.ascending1 ) {
		r = -r;
		}
	
	if (r == 0 && sorting.column2 != sorting.column1) {
		r = Compare( other, sorting.column2 );
		if ( abs( r ) < 2 && !sorting.ascending2 ) {
			r = -r;
			}
		}
	return r;
	}

void COwnerDrawnListItem::DrawColorTranspBackground( _In_ CRect& rcRest, _In_ CImageList* const il, _In_ CDC& pdc ) const {
	// Draw the color with transparent background
	auto thisHeight = rcRest.Height( );
	CPoint pt( rcRest.left, rcRest.top + thisHeight / 2 - thisHeight / 2 );

	il->SetBkColor( CLR_NONE );
	VERIFY( il->Draw( &pdc, 0, pt, ILD_NORMAL ) );
	
	}


void COwnerDrawnListItem::DrawHighlightSelectBackground( _In_ const CRect& rcLabel, _In_ const CRect& rc, _In_ const COwnerDrawnListCtrl* const list, _In_ CDC& pdc, _Inout_ COLORREF& textColor ) const {
	// Color for the text in a highlighted item (usually white)
	textColor = list->GetHighlightTextColor( );

	auto selection = rcLabel;
	// Depending on "FullRowSelection" style
	if ( list->m_showFullRowSelection ) {
		selection.right = rc.right;
		}
	// Fill the selection rectangle background (usually dark blue)
	pdc.FillSolidRect( selection, list->GetHighlightColor( ) );
	
	}

void COwnerDrawnListItem::AdjustLabelForMargin( _In_ const CRect& rcRest, _Inout_ CRect& rcLabel ) const {
	rcLabel.InflateRect( LABEL_INFLATE_CX, 0 );
	rcLabel.top    = rcRest.top + static_cast<LONG>( LABEL_Y_MARGIN );
	rcLabel.bottom = rcRest.bottom - static_cast<LONG>( LABEL_Y_MARGIN );
	}

void COwnerDrawnListItem::DrawLabel( _In_ COwnerDrawnListCtrl* const list, _In_ CDC& pdc, _In_ CRect& rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const bool indent ) const {
	/*
	  Draws an item label (icon, text) in all parts of the WinDirStat view. The rest is drawn by DrawItem()
	  */

	const auto tRc = rc;
	auto rcRest = rc;
	// Increase indentation according to tree-level
	if ( indent ) {
		rcRest.left += GENERAL_INDENT;
		}

	CSelectObject sofont( pdc, *( list->GetFont( ) ) );

	rcRest.DeflateRect( TEXT_X_MARGIN, 0 );

	auto rcLabel = rcRest;
		{
		
		const rsize_t pszSize = MAX_PATH;
		wchar_t psz_col_name_text[ pszSize ] = { 0 };
		rsize_t sizeNeeded = 0;
		rsize_t chars_written = 0;
		HRESULT res = GetText_WriteToStackBuffer( column::COL_NAME, psz_col_name_text, pszSize, sizeNeeded, chars_written );
		if ( SUCCEEDED( res ) ) {
			pdc.DrawTextW( psz_col_name_text, static_cast<int>( chars_written ), rcLabel, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP );//DT_CALCRECT modifies rcLabel!!!
			}
		else {
			if ( sizeNeeded < ( 2 * MAX_PATH ) ) {
				const rsize_t pszSize_2 = ( MAX_PATH * 2 );
				wchar_t psz_col_name_text_2[ pszSize_2 ] = { 0 };
				rsize_t chars_written_2 = 0;
				HRESULT res_2 = GetText_WriteToStackBuffer( column::COL_NAME, psz_col_name_text_2, pszSize_2, sizeNeeded, chars_written_2 );
				if ( SUCCEEDED( res_2 ) ) {
					pdc.DrawTextW( psz_col_name_text_2, static_cast<int>( chars_written_2 ), rcLabel, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP );//DT_CALCRECT modifies rcLabel!!!
					}
				else {
					goto draw_text_with_heap_memory;
					}
				}
			else {
	
	draw_text_with_heap_memory:
				const auto temp( GetText( column::COL_NAME ) );//COL_NAME

				pdc.DrawTextW( temp.c_str( ), static_cast<int>( temp.length( ) ), rcLabel, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP );//DT_CALCRECT modifies rcLabel!!!
				}
			}
	}
	AdjustLabelForMargin( rcRest, rcLabel );

	CSetBkMode bk( pdc, TRANSPARENT );
	auto textColor = GetSysColor( COLOR_WINDOWTEXT );
	if ( width == NULL && ( state bitand ODS_SELECTED ) != 0 && ( list->HasFocus( ) || list->IsShowSelectionAlways( ) ) ) {
		DrawHighlightSelectBackground( rcLabel, rc, list, pdc, textColor );
		}
	else {
		textColor = GetItemTextColor( ); // Use the color designated for this item. This is currently only for encrypted and compressed items
		}

	// Set text color for device context
	CSetTextColor stc( pdc, textColor );

	if ( width == NULL ) {
			{
			const rsize_t pszSize = MAX_PATH;
			wchar_t psz_col_name_text[ pszSize ] = { 0 };
			rsize_t sizeNeeded = 0;
			rsize_t chars_written = 0;
			auto res = GetText_WriteToStackBuffer( column::COL_NAME, psz_col_name_text, pszSize, sizeNeeded, chars_written );



			//for testing `/analyze`:
			//wchar_t psz_bullshit[ 10 ] = { 0 };
			//GetText_WriteToStackBuffer( 0, nullptr, 15, sizeNeeded );
			//GetText_WriteToStackBuffer( 0, psz_bullshit, 15, sizeNeeded );

			if ( SUCCEEDED( res ) ) {
				pdc.DrawTextW( psz_col_name_text, static_cast<int>( chars_written ), rcRest, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP );
				}
			else {
				
				// Draw the actual text	
				const auto draw_text( GetText( column::COL_NAME ) );
				pdc.DrawTextW( draw_text.c_str( ), static_cast<int>( draw_text.length( ) ), rcRest, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP );
				}
			}
		}

	rcLabel.InflateRect( 1, 1 );

	*focusLeft = rcLabel.left;

	if ( ( state bitand ODS_FOCUS ) != 0 && list->HasFocus( ) && width == NULL && !list->m_showFullRowSelection ) {
		pdc.DrawFocusRect( rcLabel );
		}


	rcLabel.left = rc.left;
	rc = rcLabel;
	if ( width != NULL ) {
		*width = ( rcLabel.Width( ) ) + 5; // +5 because GENERAL_INDENT?
		}
	}

void COwnerDrawnListItem::DrawSelection( _In_ const COwnerDrawnListCtrl* const list, _In_ CDC& pdc, _Inout_ CRect rc, _In_ const UINT state ) const {
	//ASSERT_VALID( pdc );//has already been verified by all callers!!
	ASSERT( list != NULL );
	if ( !list->m_showFullRowSelection ) {
		return;
		}
	if ( !list->HasFocus( ) && !list->IsShowSelectionAlways( ) ) {
		return;
		}
	if ( ( state bitand ODS_SELECTED ) == 0 ) {
		return;
		}

	rc.DeflateRect( 0, LABEL_Y_MARGIN );
	pdc.FillSolidRect( rc, list->GetHighlightColor( ) );
	}

void COwnerDrawnListItem::DrawPercentage( _In_ CDC& pdc, _In_ CRect rc, _In_ const DOUBLE fraction, _In_ const COLORREF color ) const {
	//ASSERT_VALID( pdc );
	const INT LIGHT = 198;	// light edge
	const INT DARK = 118;	// dark edge
	const INT BG = 225;		// background (lighter than light edge)

	const COLORREF light = RGB( LIGHT, LIGHT, LIGHT );
	const COLORREF dark  = RGB( DARK, DARK, DARK );
	const COLORREF bg    = RGB( BG, BG, BG );

	auto rcLeft = rc;
	rcLeft.right = static_cast<INT>( rcLeft.left + rc.Width( ) * fraction );

	auto rcRight = rc;
	rcRight.left = rcLeft.right;

	if ( rcLeft.right > rcLeft.left ) {
		pdc.Draw3dRect( rcLeft, light, dark );
		}
	rcLeft.DeflateRect( 1, 1 );
	if ( rcLeft.right > rcLeft.left ) {
		pdc.FillSolidRect( rcLeft, color );
		}
	if ( rcRight.right > rcRight.left ) {
		pdc.Draw3dRect( rcRight, light, light );
		}
	rcRight.DeflateRect( 1, 1 );
	if ( rcRight.right > rcRight.left ) {
		pdc.FillSolidRect( rcRight, bg );
		}
	}

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( COwnerDrawnListCtrl, CSortingListControl )

COwnerDrawnListCtrl::COwnerDrawnListCtrl( _In_z_ PCWSTR name, _In_range_( 0, UINT_MAX ) const UINT rowHeight ) : CSortingListControl( name ), m_rowHeight( rowHeight ), m_showGrid( false ), m_showStripes( false ), m_showFullRowSelection( false ) {
	ASSERT( rowHeight > 0 );
	InitializeColors( );
	}

//COwnerDrawnListCtrl::~COwnerDrawnListCtrl( ) { }

COLORREF COwnerDrawnListItem::GetItemTextColor( const bool defaultTextColor ) const {
	if ( defaultTextColor ) {
		return COwnerDrawnListItem::ItemTextColor( );
		}
	return ItemTextColor( );
	}

bool COwnerDrawnListItem::DrawSubitem_( _In_ _In_range_( 0, 7 ) const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ CRect rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft ) const {
	return DrawSubitem( subitem, pdc, rc, state, width, focusLeft );
	}

INT COwnerDrawnListItem::compare_interface( _In_ const COwnerDrawnListItem* const other, _In_ _In_range_( 0, 7 ) const column::ENUM_COL subitem ) const {
	return Compare( other, subitem );
	}

INT COwnerDrawnListItem::Compare( _In_ const COwnerDrawnListItem* const other, _In_ _In_range_( 0, 7 ) const column::ENUM_COL subitem ) const {
/*
	Return value:
	<= -2:	this is less than other regardless of ascending flag
	-1:		this is less than other
	0:		this equals other
	+1:		this is greater than other
	>= +1:	this is greater than other regardless of ascending flag.
*/

	// Default implementation compares strings
	return signum( GetText( subitem ).compare( other->GetText( subitem ) ) );

	}


COLORREF COwnerDrawnListItem::ItemTextColor( ) const {
	return GetSysColor( COLOR_WINDOWTEXT );
	}

std::wstring COwnerDrawnListItem::GetText( _In_range_( 0, 7 ) const column::ENUM_COL subitem ) const {
	return Text( subitem );
	}


_Must_inspect_result_
HRESULT COwnerDrawnListItem::GetText_WriteToStackBuffer( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	return Text_WriteToStackBuffer( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
	}

void COwnerDrawnListCtrl::OnColumnsInserted( ) {
	/*
	  This method MUST be called BEFORE the Control is shown.
	*/
	// The pacmen shall not draw over our header control.
	VERIFY( ModifyStyle( 0, WS_CLIPCHILDREN ) );

	// Where does the 1st Item begin vertically?
	if ( GetItemCount( ) > 0 ) {
		CRect rc;
		VERIFY( GetItemRect( 0, rc, LVIR_BOUNDS ) );
		m_yFirstItem = rc.top;
		}
	else {
		InsertItem( 0, _T( "_tmp" ), 0 );
		CRect rc;
		VERIFY( GetItemRect( 0, rc, LVIR_BOUNDS ) );
		VERIFY( DeleteItem( 0 ) );
		m_yFirstItem = rc.top;
		}

	LoadPersistentAttributes( );
	}


//COLORREF COwnerDrawnListCtrl::GetItemSelectionBackgroundColor( _In_ const COwnerDrawnListItem* const item ) const {
//	return GetItemSelectionBackgroundColor( FindListItem( item ) );
//	}

COLORREF COwnerDrawnListCtrl::GetItemSelectionTextColor( _In_ _In_range_( 0, INT_MAX ) const INT i ) const {
	auto selected = ( GetItemState( i, LVIS_SELECTED ) & LVIS_SELECTED ) != 0;
	if ( selected && m_showFullRowSelection && ( HasFocus( ) || IsShowSelectionAlways( ) ) ) {
		return GetHighlightTextColor( );
		}
	return GetSysColor( COLOR_WINDOWTEXT );
	}

COwnerDrawnListItem* COwnerDrawnListCtrl::GetItem( _In_ _In_range_( 0, INT_MAX ) const INT i ) const {
	ASSERT( i < GetItemCount( ) );
	const auto item = reinterpret_cast<COwnerDrawnListItem *>( GetItemData( i ) );
	return item;
	}

_Success_( return != -1 ) _Ret_range_( -1, INT_MAX ) INT COwnerDrawnListCtrl::FindListItem( _In_ const COwnerDrawnListItem* const item ) const {

	auto fi   = zeroInitLVFINDINFO( );
	fi.flags  = LVFI_PARAM;
	fi.lParam = reinterpret_cast<LPARAM>( item );

	auto i = static_cast<INT>( FindItem( &fi ) );

	return i;
	}

void COwnerDrawnListCtrl::InitializeColors( ) {
	// I try to find a good contrast to COLOR_WINDOW (usually white or light grey).
	// This is a result of experiments. 

	const DOUBLE diff      = 0.07;		// Try to alter the brightness by diff.
	const DOUBLE threshold = 1.04;	// If result would be brighter, make color darker.
	m_windowColor          = GetSysColor( COLOR_WINDOW );

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

#ifdef COLOR_DEBUGGING
	TRACE( _T( "Setting m_stripeColor to CColorSpace::MakeBrightColor( m_windowColor: %ld, b: %f )\r\n" ), m_windowColor, b );
#endif
	m_stripeColor = CColorSpace::MakeBrightColor( m_windowColor, b );
#ifdef COLOR_DEBUGGING
	TRACE( _T( "m_stripeColor: %ld\r\n" ), m_stripeColor );
#endif
	}

_Success_( SUCCEEDED( return ) )
HRESULT COwnerDrawnListCtrl::drawSubItem_stackbuffer( _In_ const COwnerDrawnListItem* const item, _In_ CRect& rcText, const int& align, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ CDC& dcmem ) const {
	const rsize_t subitem_text_size = 128;
	wchar_t psz_subitem_formatted_text[ subitem_text_size ] = { 0 };
	rsize_t sizeNeeded = 0;
	rsize_t chars_written = 0;

	const HRESULT res = item->GetText_WriteToStackBuffer( subitem, psz_subitem_formatted_text, subitem_text_size, sizeNeeded, chars_written );
	if ( SUCCEEDED( res ) ) {
		dcmem.DrawTextW( psz_subitem_formatted_text, static_cast<int>( chars_written ), rcText, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP | static_cast<UINT>( align ) );
		return res;
		}
	if ( ( MAX_PATH * 2 ) > sizeNeeded ) {
		const rsize_t subitem_text_size_2 = ( MAX_PATH * 2 );
		wchar_t psz_subitem_formatted_text_2[ subitem_text_size_2 ] = { 0 };
		rsize_t chars_written_2 = 0;
		const HRESULT res_2 = item->GetText_WriteToStackBuffer( subitem, psz_subitem_formatted_text_2, subitem_text_size_2, sizeNeeded, chars_written_2 );
		if ( SUCCEEDED( res_2 ) ) {
			dcmem.DrawTextW( psz_subitem_formatted_text_2, static_cast<int>( chars_written_2 ), rcText, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP | static_cast<UINT>( align ) );
			return res;
			}
		//goto DoDrawSubItemBecauseItCannotDrawItself_drawText_dynamic_memory;
		//DrawText_dynamic( item, rcText, align, subitem, dcmem );
		return res;
		}
	ASSERT( !SUCCEEDED( res ) );
	return res;
	}

void COwnerDrawnListCtrl::DoDrawSubItemBecauseItCannotDrawItself( _In_ const COwnerDrawnListItem* const item, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ CDC& dcmem, _In_ CRect& rcDraw, _In_ const PDRAWITEMSTRUCT& pdis, _In_ const bool showSelectionAlways, _In_ const bool bIsFullRowSelection, const std::vector<bool>& is_right_aligned_cache ) const {
	item->DrawSelection( this, dcmem, rcDraw, pdis->itemState );
	auto rcText = rcDraw;
	rcText.DeflateRect( TEXT_X_MARGIN, 0 );
	CSetBkMode bk( dcmem, TRANSPARENT );
	CSelectObject sofont( dcmem, *( GetFont( ) ) );
	
	//const auto align = IsColumnRightAligned( subitem ) ? DT_RIGHT : DT_LEFT;
	const auto align = is_right_aligned_cache[ static_cast<size_t>( subitem ) ] ? DT_RIGHT : DT_LEFT;

	// Get the correct color in case of compressed or encrypted items
	auto textColor = item->GetItemTextColor( );

	if ( ( pdis->itemState bitand ODS_SELECTED ) && ( showSelectionAlways || HasFocus( ) ) && ( bIsFullRowSelection ) ) {
		textColor = GetItemSelectionTextColor( static_cast<INT>( pdis->itemID ) );
		}

	CSetTextColor tc( dcmem, textColor );

	if ( ( subitem == column::COL_FILES ) || ( subitem == column::COL_ITEMS ) ) {
		//goto DoDrawSubItemBecauseItCannotDrawItself_drawText_dynamic_memory;
		DrawText_dynamic( item, rcText, align, subitem, dcmem );
		return;
		}
	const HRESULT stackbuffer_draw_res = drawSubItem_stackbuffer( item, rcText, align, subitem, dcmem );
	if ( !SUCCEEDED( stackbuffer_draw_res ) ) {
		DrawText_dynamic( item, rcText, align, subitem, dcmem );
		}
	}

void COwnerDrawnListCtrl::DrawText_dynamic( _In_ const COwnerDrawnListItem* const item, _In_ CRect& rcText, const int& align, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ CDC& dcmem ) const {
	// Draw the (sub)item text
	const auto s( item->GetText( subitem ) );
	dcmem.DrawTextW( s.c_str( ), static_cast<int>( s.length( ) ), rcText, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP | static_cast< UINT >( align ) );
	}

void COwnerDrawnListCtrl::DrawItem( _In_ PDRAWITEMSTRUCT pdis ) {
	const auto item = reinterpret_cast< COwnerDrawnListItem *> ( pdis->itemData );
	const auto pdc = CDC::FromHandle( pdis->hDC );
	const auto bIsFullRowSelection = m_showFullRowSelection;
	ASSERT_VALID( pdc );
	CRect rcItem( pdis->rcItem );
	if ( m_showGrid ) {
		rcItem.bottom--;
		rcItem.right--;
		}

	CDC dcmem; //compiler seems to vectorize this!

	VERIFY( dcmem.CreateCompatibleDC( pdc ) );
	CBitmap bm;
	VERIFY( bm.CreateCompatibleBitmap( pdc, ( rcItem.Width( ) ), ( rcItem.Height( ) ) ) );
	CSelectObject sobm( dcmem, bm );

	dcmem.FillSolidRect( rcItem - rcItem.TopLeft( ), GetItemBackgroundColor( static_cast<INT>( pdis->itemID ) ) ); //NOT vectorized!

	const bool drawFocus = ( pdis->itemState bitand ODS_FOCUS ) != 0 && HasFocus( ) && bIsFullRowSelection; //partially vectorized

	CArray<INT, INT> order;
	//std::vector<column::ENUM_COL> orderVec;
	
	const bool showSelectionAlways = IsShowSelectionAlways( );
	const auto thisHeaderCtrl = GetHeaderCtrl( );//HORRENDOUSLY slow. Pessimisation of memory access, iterates (with a for loop!) over a map. MAXIMUM branch prediction failures! Maximum Bad Speculation stalls!

	//orderVec.reserve( static_cast<size_t>( thisHeaderCtrl->GetItemCount( ) ) );
	order.SetSize( thisHeaderCtrl->GetItemCount( ) );
	VERIFY( thisHeaderCtrl->GetOrderArray( order.GetData( ), static_cast<int>( order.GetSize( ) ) )) ;

#ifdef DEBUG
	for ( INT i = 0; i < order.GetSize( ) - 1; ++i ) {
		if ( i != order[ i ] ) {
			TRACE( _T( "order[%i]: %i \r\n" ), i, order[ i ] );
			}
		}
#endif
	std::vector<bool> is_right_aligned_cache;
	const auto thisLoopSize = order.GetSize( );
	for ( INT i = 0; i < thisLoopSize; ++i ) {
		is_right_aligned_cache.push_back( IsColumnRightAligned( i ) );
		}
	auto rcFocus = rcItem;
	rcFocus.DeflateRect( 0, LABEL_Y_MARGIN - 1 );

	
	for ( INT i = 0; i < thisLoopSize; i++ ) {
		//iterate over columns, properly populate fields.
		ASSERT( order[ i ] == i );
		const auto subitem = static_cast<column::ENUM_COL>( order[ i ] );
		const auto rc = GetWholeSubitemRect( static_cast<INT>( pdis->itemID ), subitem );
		CRect rcDraw = rc - rcItem.TopLeft( );
		INT focusLeft = rcDraw.left;
		if ( !item->DrawSubitem_( subitem, dcmem, rcDraw, pdis->itemState, NULL, &focusLeft ) ) {//if DrawSubItem returns true, item draws self. Therefore `!item->DrawSubitem` is true when item DOES NOT draw self
			DoDrawSubItemBecauseItCannotDrawItself( item, subitem, dcmem, rcDraw, pdis, showSelectionAlways, bIsFullRowSelection, is_right_aligned_cache );
			}

		if ( focusLeft > rcDraw.left ) {
			if ( drawFocus && i > 0 ) {
				pdc->DrawFocusRect( rcFocus );
				}
			rcFocus.left = focusLeft;
			}
		rcFocus.right = rcDraw.right;
		VERIFY( pdc->BitBlt( ( rcItem.left + rcDraw.left ), ( rcItem.top + rcDraw.top ), ( rcDraw.Width( ) ), ( rcDraw.Height( ) ), &dcmem, rcDraw.left, rcDraw.top, SRCCOPY ) );
		}
	if ( drawFocus ) {
		pdc->DrawFocusRect( rcFocus );
		}
	//VERIFY( dcmem.DeleteDC( ) );
	}

bool COwnerDrawnListCtrl::IsColumnRightAligned( _In_ const INT col ) const {
	auto hditem = zeroInitHDITEM( );
	hditem.mask   = HDI_FORMAT;
	VERIFY( GetHeaderCtrl( )->GetItem( col, &hditem ) );
	return ( hditem.fmt bitand HDF_RIGHT ) != 0;
	}

CRect COwnerDrawnListCtrl::GetWholeSubitemRect( _In_ const INT item, _In_ const INT subitem ) const {
	CRect rc;
	if ( subitem == 0 ) {
		// Special case column 0:
		// If we did GetSubItemRect(item 0, LVIR_LABEL, rc) and we have an image list, then we would get the rectangle excluding the image.
		HDITEM hditem = zeroInitHDITEM( );

		hditem.mask = HDI_WIDTH;
		VERIFY( GetHeaderCtrl( )->GetItem( 0, &hditem ) );

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

_Success_( return >= 0 ) _Ret_range_( 0, INT_MAX ) _On_failure_( _Ret_range_( -1, -1 ) )
INT COwnerDrawnListCtrl::GetSubItemWidth( _In_ const COwnerDrawnListItem* const item, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem ) const {
	if ( item == NULL ) {
		return -1;
		}
	INT width = 0;

	CClientDC dc( const_cast< COwnerDrawnListCtrl* >( this ) );
	CRect rc( 0, 0, 1000, 1000 );
	
	INT dummy = rc.left;
	if ( item->DrawSubitem_( subitem, dc, rc, 0, &width, &dummy ) ) {
		//ASSERT( item )
		return width;
		}

	const auto s( item->GetText( subitem ) );
	if ( s.empty( ) ) {
		return 0;
		}

	CSelectObject sofont( dc, *( GetFont( ) ) );
	const auto align = IsColumnRightAligned( subitem ) ? DT_RIGHT : DT_LEFT;
	dc.DrawTextW( s.c_str( ), static_cast<int>( s.length( ) ), rc, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP | static_cast<UINT>( align ) );

	rc.InflateRect( TEXT_X_MARGIN, 0 );
	return rc.Width( );
	}

void COwnerDrawnListCtrl::buildArrayFromItemsInHeaderControl( _In_ CArray<INT, INT>& columnOrder, _Inout_ CArray<INT, INT>& vertical ) {
	vertical.SetSize( GetHeaderCtrl( )->GetItemCount( ) + 1 );
	
	auto x   = -GetScrollPos( SB_HORZ );
	auto hdi = zeroInitHDITEM( );

	hdi.mask = HDI_WIDTH;
	for ( INT i = 0; i < GetHeaderCtrl( )->GetItemCount( ); i++ ) {
		VERIFY( GetHeaderCtrl( )->GetItem( columnOrder[ i ], &hdi ) );
		x += hdi.cxy;
		vertical[ i ] = x;
		}
	}

BEGIN_MESSAGE_MAP(COwnerDrawnListCtrl, CSortingListControl)
	ON_WM_ERASEBKGND()
	ON_NOTIFY(HDN_DIVIDERDBLCLICKA, 0, OnHdnDividerdblclick)
	ON_NOTIFY(HDN_DIVIDERDBLCLICKW, 0, OnHdnDividerdblclick)
	ON_WM_VSCROLL()
	ON_NOTIFY(HDN_ITEMCHANGINGA, 0, OnHdnItemchanging)
	ON_NOTIFY(HDN_ITEMCHANGINGW, 0, OnHdnItemchanging)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

BOOL COwnerDrawnListCtrl::OnEraseBkgnd( CDC* pDC ) {
	ASSERT_VALID( pDC );
	ASSERT( GetHeaderCtrl( )->GetItemCount( ) > 0 );

	// We should recalculate m_yFirstItem here (could have changed e.g. when the XP-Theme changed).
	if ( GetItemCount( ) > 0 ) {
		CRect rc;
		VERIFY( GetItemRect( GetTopIndex( ), rc, LVIR_BOUNDS ) );
		m_yFirstItem = rc.top;
		}
	// else: if we did the same thing as in OnColumnsCreated(), we get repaint problems.

	const COLORREF gridColor = RGB( 212, 208, 200 );

	CRect rcClient;
	GetClientRect( rcClient );

	CRect rcHeader;
	GetHeaderCtrl( )->GetWindowRect( rcHeader );
	ScreenToClient( rcHeader );

	auto rcBetween  = rcClient;// between header and first item
	rcBetween.top    = rcHeader.bottom;
	rcBetween.bottom = m_yFirstItem;
	pDC->FillSolidRect( rcBetween, gridColor );

	CArray<INT, INT> columnOrder;
	columnOrder.SetSize( GetHeaderCtrl( )->GetItemCount( ) );
	VERIFY( GetColumnOrderArray( columnOrder.GetData( ), static_cast<int>( columnOrder.GetSize( ) ) ) );

	CArray<INT, INT> vertical;

	buildArrayFromItemsInHeaderControl( columnOrder, vertical );

	if ( m_showGrid ) {
		CPen pen( PS_SOLID, 1, gridColor );
		const CSelectObject sopen( *pDC, pen );

		const auto rowHeight = m_rowHeight;
		for ( auto y = ( m_yFirstItem + rowHeight - 1 ); y < rcClient.bottom; y += rowHeight ) {
			ASSERT( rowHeight == m_rowHeight );
			pDC->MoveTo( rcClient.left, static_cast<INT>( y ) );
			VERIFY( pDC->LineTo( rcClient.right, static_cast<INT>( y ) ) );
			}

		const auto verticalSize = vertical.GetSize( );
		for ( INT i = 0; i < verticalSize; i++ ) {
			ASSERT( verticalSize == vertical.GetSize( ) );
			pDC->MoveTo( ( vertical[ i ] - 1 ), rcClient.top );
			VERIFY( pDC->LineTo( ( vertical[ i ] - 1 ), rcClient.bottom ) );
			}
		}

	const auto bgcolor   = GetSysColor( COLOR_WINDOW );
	const INT gridWidth  = m_showGrid ? 1 : 0;
	const auto lineCount = GetCountPerPage( ) + 1;
	const auto firstItem = GetTopIndex( );
	const INT lastItem   = min( firstItem + lineCount, GetItemCount( ) ) - 1;

	ASSERT( GetItemCount( ) == 0 || firstItem < GetItemCount( ) );
	ASSERT( GetItemCount( ) == 0 || lastItem < GetItemCount( ) );
	ASSERT( GetItemCount( ) == 0 || lastItem >= firstItem );

	const auto itemCount = lastItem - firstItem + 1;

	CRect fill;
	fill.left   = vertical[ vertical.GetSize( ) - 1 ];
	fill.right  = rcClient.right;
	fill.top    = m_yFirstItem;
	fill.bottom = fill.top + static_cast<LONG>( m_rowHeight ) - static_cast<LONG>( gridWidth );
	for ( INT i = 0; i < itemCount; i++ ) {
		pDC->FillSolidRect( fill, bgcolor );
		fill.OffsetRect( 0, static_cast<int>( m_rowHeight ) );
		}

	const auto rowHeight = m_rowHeight;
	auto top = fill.top;
	while ( top < rcClient.bottom ) {
		fill.top    = top;
		fill.bottom = top + static_cast<LONG>( m_rowHeight ) - static_cast<LONG>( gridWidth );
		
		INT left = 0;
		auto verticalSize = vertical.GetSize( );
		for ( INT i = 0; i < verticalSize; i++ ) {
			ASSERT( verticalSize == vertical.GetSize( ) );
			fill.left = left;
			fill.right = vertical[ i ] - gridWidth;
			pDC->FillSolidRect( fill, bgcolor );
			left = vertical[ i ];
			}
		fill.left  = left;
		fill.right = rcClient.right;
		pDC->FillSolidRect( fill, bgcolor );

		ASSERT( rowHeight == m_rowHeight );
		top += rowHeight;
		}
	return true;
	}

void COwnerDrawnListCtrl::OnHdnDividerdblclick( NMHDR* pNMHDR, LRESULT* pResult ) {
	CWaitCursor wc;
	ASSERT( pNMHDR != NULL );
	if ( pNMHDR != NULL ) {
		auto phdr = reinterpret_cast< LPNMHEADER >( pNMHDR );
		const INT subitem = phdr->iItem;
		AdjustColumnWidth( static_cast<column::ENUM_COL>( subitem ) );
		}
	ASSERT( pResult != NULL );
	if ( pResult != NULL ) {
		*pResult = 0;
		}
	}

void COwnerDrawnListCtrl::AdjustColumnWidth( _In_ const column::ENUM_COL col ) {
	CWaitCursor wc;

	INT width = 10;
	const auto itemCount = GetItemCount( );
	for ( INT i = 0; i < itemCount; i++ ) {
		ASSERT( itemCount == GetItemCount( ) );
		auto w = GetSubItemWidth( GetItem( i ), col );
		if ( w > width ) {
			width = w;
			}
		}
	VERIFY( SetColumnWidth( col, width + 5 ) );
	}

void COwnerDrawnListCtrl::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar ) {
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);

	// Owner drawn list controls with LVS_EX_GRIDLINES don't repaint correctly when scrolled (under Windows XP). So we force a complete repaint here.
	InvalidateRect( NULL );
	}

void COwnerDrawnListCtrl::OnHdnItemchanging( NMHDR* /*pNMHDR*/, LRESULT* pResult ) {
	// Unused: LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	Default( );
	InvalidateRect( NULL );
	ASSERT( pResult != NULL );
	if ( pResult != NULL ) {
		*pResult = 0;
		}
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
