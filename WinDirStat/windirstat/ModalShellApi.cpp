// ModalShellApi.cpp	- Implementation of CModalShellApi
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
#include "ModalShellApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
	enum {
		EMPTY_RECYCLE_BIN,
		DELETE_FILE
		};
	}


CModalShellApi::CModalShellApi()
{
}

void CModalShellApi::DeleteFile( _In_ LPCTSTR fileName, _In_ bool toRecycleBin ) {
	m_operation    = DELETE_FILE;
	m_fileName     = fileName;
	m_toRecycleBin = toRecycleBin;
	DoModal( );
	}

void CModalShellApi::DoOperation( ) {
	if ( m_operation == DELETE_FILE ) {
		return DoDeleteFile( );
		}
	}

void CModalShellApi::DoDeleteFile( ) {
	INT len = m_fileName.GetLength( );
	LPTSTR psz = m_fileName.GetBuffer( len + 2 );
	psz[ len + 1 ] = 0;

	auto sfos   = zeroInitSHFILEOPSTRUCT( );

	sfos.wFunc  = FO_DELETE;
	sfos.pFrom  = psz;
	sfos.fFlags = m_toRecycleBin ? FOF_ALLOWUNDO : 0;

	sfos.hwnd   = *AfxGetMainWnd( );

	( void ) SHFileOperation( &sfos );

	m_fileName.ReleaseBuffer( );
	}

// $Log$
// Revision 1.5  2004/11/13 08:17:07  bseifert
// Remove blanks in Unicode Configuration names.
//
// Revision 1.4  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.3  2004/11/05 16:53:06  assarbad
// Added Date and History tag where appropriate.
//
