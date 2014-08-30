//http://blog.airesoft.co.uk/code/ntdll.h
//http://gate.upm.ro/os/LABs/Windows_OS_Internals_Curriculum_Resource_Kit-ACADEMIC/WindowsResearchKernel-WRK/WRK-v1.2/base/ntos/io/iomgr/dir.c
//http://fy.chalmers.se/~appro/LD_*-gallery/statpatch.c

//On second thought: you don't need the DDK!
//NOTE: the WDK is not the DDK!
//Some important files are in the DDK!
//It's a bit hard to find the DDK!
//I found it here: https://dev.windowsphone.com/en-us/OEM/docs/Getting_Started/Preparing_for_Windows_Phone_development?logged_in=1




#pragma warning( push, 3 )

#pragma warning( disable : 4514 )
#pragma warning( disable : 4710 )
#include "NT_Q_DIR_TEST_2.h"


#pragma warning( pop )


enum CmdParseResult {
	DISPLAY_USAGE,
	LIST_DIR,
	DEL_FILE,
	ENUM_DIR
	};

const DOUBLE getAdjustedTimingFrequency( ) {
	LARGE_INTEGER timingFrequency;
	BOOL res1 = QueryPerformanceFrequency( &timingFrequency );
	if ( !res1 ) {
		std::wcout << L"QueryPerformanceFrequency failed!!!!!! Disregard any timing data!!" << std::endl;
		}
	const DOUBLE adjustedTimingFrequency = ( DOUBLE( 1.00 ) / DOUBLE( timingFrequency.QuadPart ) );
	return adjustedTimingFrequency;
	}


NtdllWrap::NtdllWrap( ) {
	ntdll = GetModuleHandle( L"C:\\Windows\\System32\\ntdll.dll" );
	if ( ntdll ) {
		auto success = NtQueryDirectoryFile.init( GetProcAddress( ntdll, "NtQueryDirectory" ) );
		if ( !success ) {
			std::wcout << L"Couldn't find NtQueryDirectoryFile in ntdll.dll!" << std::endl;
			}
		}
	else {
		std::wcout << L"Couldn't load ntdll.dll!" << std::endl;
		}
	}

_Success_( return != LONG_MAX ) NTSTATUS NTAPI ntQueryDirectoryFile_f::operator()( _In_ HANDLE FileHandle, _In_opt_ HANDLE Event, _In_opt_ PVOID ApcRoutine, _In_opt_ PVOID ApcContext, _Out_  IO_STATUS_BLOCK* IoStatusBlock, _Out_  PVOID FileInformation, _In_ ULONG Length, _In_ FILE_INFORMATION_CLASS FileInformationClass, _In_ BOOLEAN ReturnSingleEntry, _In_opt_ PUNICODE_STRING FileName, _In_ BOOLEAN RestartScan ) {
	if ( ntQueryDirectoryFuncPtr ) {
		return ntQueryDirectoryFuncPtr( FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, Length, FileInformationClass, ReturnSingleEntry, FileName, RestartScan );
		}
	return LONG_MAX;
	}


void DeleteFileByHandle( _In_ HANDLE hFile ) {
	assert( false );
	return;
	IO_STATUS_BLOCK iosb = { 0 };

	struct FILE_DISPOSITION_INFORMATION {
		BOOLEAN  DeleteFile;
		} fdi = { TRUE };

	HMODULE hNtdll = GetModuleHandle( L"C:\\Windows\\System32\\ntdll.dll" );
	if ( hNtdll ) {
		pfnSetInfoFile ntSetInformationFile = ( pfnSetInfoFile ) GetProcAddress( hNtdll, "NtSetInformationFile" );
		if ( ntSetInformationFile ) {
			ntSetInformationFile( hFile, &iosb, &fdi, sizeof( fdi ), FileDispositionInformation );
			}
		}
	}

LONGLONG WcsToLLDec( _In_z_ const wchar_t* pNumber, _In_ wchar_t* endChar ) {
	LONGLONG temp = 0;
	while ( iswdigit( *pNumber ) ) {
		temp *= 10;
		temp += ( *pNumber - L'0' );
		++pNumber;
		}
	if ( endChar ) {
		*endChar = *pNumber;
		}
	return temp;
	}

CmdParseResult ParseCmdLine( _In_ int argc, _In_ _In_reads_( argc ) wchar_t** argv, _In_opt_ LONGLONG* fileId ) {
	if ( ( argc < 2 ) || ( ( argv[ 1 ][ 0 ] != L'/' ) && ( argv[ 1 ][ 0 ] != L'-' ) ) ) {
		return DISPLAY_USAGE;
		}
	wchar_t lowerFirstArgChar = towlower( argv[ 1 ][ 1 ] );
	// argv[2] is optional
	if ( lowerFirstArgChar == L'l' ) {
		if ( ( argc == 2 ) || ( PathIsDirectory( argv[ 2 ] ) ) ) {
			return LIST_DIR;
			}
		else return DISPLAY_USAGE;
		}
	else if ( lowerFirstArgChar == L'd' ) {
		wchar_t endChar = 0;
		if ( fileId != NULL ) {
			if ( ( argc < 4 ) || ( ( *fileId = WcsToLLDec( argv[ 3 ], &endChar ), endChar != 0 ) ) ) {
				return DISPLAY_USAGE;
				}
			else {
				return DEL_FILE;
				}
			}
		}
	else if ( lowerFirstArgChar == L'e' ) {
		return ENUM_DIR;
		}
	return DISPLAY_USAGE;
	}



void qDirFile( const wchar_t* dir, std::vector<std::wstring>& dirs, std::uint64_t& numItems, const bool writeToScreen, pfnQueryDirFile ntQueryDirectoryFile, std::wstring& curDir, HANDLE hDir, std::vector<UCHAR>& idInfo ) {
	//I do this to ensure there are NO issues with incorrectly sized buffers or mismatching parameters (or any other bad changes)
	const FILE_INFORMATION_CLASS InfoClass = FileIdFullDirectoryInformation;
	typedef FILE_ID_FULL_DIR_INFORMATION THIS_FILE_INFORMATION_CLASS;
	typedef THIS_FILE_INFORMATION_CLASS* PTHIS_FILE_INFORMATION_CLASS;

	IO_STATUS_BLOCK iosb = { static_cast< ULONG_PTR >( 0 ) };

	NTSTATUS stat = STATUS_PENDING;
	if ( writeToScreen ) {
		std::wcout << L"Files in directory " << dir << L'\n';
		std::wcout << L"      File ID       |       File Name\n";
		}
	assert( idInfo.size( ) > 1 );
	auto buffer = &( idInfo[ 0 ] );
	//++numItems;
	auto sBefore = stat;
	stat = ntQueryDirectoryFile( hDir, NULL, NULL, NULL, &iosb, ( &idInfo[ 0 ] ), idInfo.size( ), InfoClass, FALSE, NULL, TRUE );
	assert( stat != sBefore );
	while ( stat == STATUS_BUFFER_OVERFLOW ) {
		idInfo.erase( idInfo.begin( ), idInfo.end( ) );
		idInfo.resize( idInfo.size( ) * 2 );
		buffer = ( &idInfo[ 0 ] );
		stat = ntQueryDirectoryFile( hDir, NULL, NULL, NULL, &iosb, buffer, idInfo.size( ), InfoClass, FALSE, NULL, TRUE );
		}
	
	auto bufSizeWritten = iosb.Information;
	assert( NT_SUCCESS( stat ) );

	//This zeros just enough of the idInfo buffer ( after the end of valid data ) to halt the forthcoming while loop at the last valid data. This saves the effort of zeroing larger parts of the buffer.
	for ( size_t i = bufSizeWritten; i < bufSizeWritten + ( sizeof( THIS_FILE_INFORMATION_CLASS ) + ( MAX_PATH * sizeof( UCHAR ) ) * 2 ); ++i ) {
		if ( i == idInfo.size( ) ) {
			break;
			}
		idInfo.at( i ) = 0;
		}
	

	auto pFileInf = reinterpret_cast<PTHIS_FILE_INFORMATION_CLASS>( &idInfo[ 0 ] );

	assert( pFileInf != NULL );
	std::vector<std::wstring> breadthDirs;
	std::vector<WCHAR> fNameVect;
	while ( NT_SUCCESS( stat ) && ( pFileInf != NULL ) ) {
		//PFILE_ID_BOTH_DIR_INFORMATION pFileInf = ( FILE_ID_BOTH_DIR_INFORMATION* ) buffer;

		assert( pFileInf->FileNameLength > 1 );
		if ( pFileInf->FileName[ 0 ] == L'.' && ( pFileInf->FileName[ 1 ] == 0 || ( pFileInf->FileName[ 1 ] == '.' ) ) ) {
			//continue;
			goto nextItem;
			}
		
		++numItems;
		if ( writeToScreen || ( pFileInf->FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {//I'd like to avoid building a null terminated string unless it is necessary
			fNameVect.clear( );
			fNameVect.reserve( ( pFileInf->FileNameLength / sizeof( WCHAR ) ) + 1 );
			PWCHAR end = pFileInf->FileName + ( pFileInf->FileNameLength / sizeof( WCHAR ) );
			fNameVect.insert( fNameVect.end( ), pFileInf->FileName, end );
			fNameVect.emplace_back( L'\0' );
			PWSTR fNameChar = &( fNameVect[ 0 ] );

			if ( writeToScreen ) {

				std::wcout << std::setw( std::numeric_limits<LONGLONG>::digits10 ) << pFileInf->FileId.QuadPart << L"    " << std::setw( 0 ) << curDir << L"\\" << fNameChar;
				auto state = std::wcout.fail( );
				if ( state != 0 ) {
					std::wcout.clear( );
					std::wcout << std::endl << L"std::wcout.fail( ): " << state << std::endl;
					}
				}
			if ( pFileInf->FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

				if ( curDir.compare( curDir.length( ) - 2, 2, L"\\" ) == 0 ) {
					DebugBreak( );
					}
				if ( curDir.back( ) != L'\\' ) {
					breadthDirs.emplace_back( curDir + L"\\" + fNameChar + L"\\" );
					}
				else {
					breadthDirs.emplace_back( curDir + fNameChar + L"\\" );
					}
				//std::wstring dirstr = curDir + L"\\" + fNameChar + L"\\";
				//breadthDirs.emplace_back( dirstr );
				//numItems += ListDirectory( dirstr.c_str( ), dirs, idInfo, writeToScreen );
				}
			}

	nextItem:
		//stat = ntQueryDirectoryFile( hDir, NULL, NULL, NULL, &iosb, &idInfo[ 0 ], idInfo.size( ), FileIdBothDirectoryInformation, TRUE, NULL, FALSE );
		if ( writeToScreen ) {
			std::wcout << L"\t\tpFileInf: " << pFileInf << L", pFileInf->NextEntryOffset: " << pFileInf->NextEntryOffset << L", pFileInf + pFileInf->NextEntryOffset " << ( pFileInf + pFileInf->NextEntryOffset ) << std::endl;
			}
		pFileInf = ( pFileInf->NextEntryOffset != 0 ) ? reinterpret_cast<PTHIS_FILE_INFORMATION_CLASS>( reinterpret_cast<std::uint64_t>( pFileInf ) + ( static_cast<std::uint64_t>( pFileInf->NextEntryOffset ) ) ) : NULL;
		}

	for ( auto& aDir : breadthDirs ) {
		numItems += ListDirectory( aDir.c_str( ), dirs, idInfo, writeToScreen );
		}
	assert( ( pFileInf == NULL ) || ( !NT_SUCCESS( stat ) ) );
	}

uint64_t ListDirectory( _In_z_ const wchar_t* dir, _Inout_ std::vector<std::wstring>& dirs, _Inout_ std::vector<UCHAR>& idInfo, _In_ const bool writeToScreen ) {
	std::wstring curDir;
	std::uint64_t numItems = 0;
	if ( !dir ) {
		curDir.resize( GetCurrentDirectoryW( 0, NULL ) );
		GetCurrentDirectoryW( static_cast<DWORD>( curDir.size( ) ), &curDir[ 0 ] );
		dir = curDir.c_str( );
		}
	else {
		curDir = dir;
		}
	HANDLE hDir = CreateFile( dir, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
	if ( hDir == INVALID_HANDLE_VALUE ) {
		DWORD err = GetLastError( );
		std::wcout << L"Failed to open directory " << dir << L" because of error " << err << std::endl;
		return numItems;
		}
	HMODULE hNtDll = GetModuleHandle( L"C:\\Windows\\System32\\ntdll.dll" );
	if ( hNtDll == NULL ) {
		return numItems;
		}
	pfnQueryDirFile ntQueryDirectoryFile = ( pfnQueryDirFile ) GetProcAddress( hNtDll, "NtQueryDirectoryFile" );
	if ( !ntQueryDirectoryFile ) {
		std::wcout << L"Couldn't find NtQueryDirectoryFile in ntdll.dll!" << std::endl;
		return numItems;
		}
	
	qDirFile( dir, dirs, numItems, writeToScreen, ntQueryDirectoryFile, curDir, hDir, idInfo );
	if ( writeToScreen ) {
		std::wcout << std::setw( std::numeric_limits<LONGLONG>::digits10 ) << numItems << std::setw( 0 ) << L" items in directory " << dir << std::endl;
		}
	CloseHandle( hDir );
	return numItems + dirs.size( );
	}

void DelFile( _In_ WCHAR fileVolume, _In_ LONGLONG fileId ) {
	assert( false );
	return;
	HMODULE hNtdll = GetModuleHandleW( L"C:\\Windows\\System32\\ntdll.dll" );
	if ( hNtdll == NULL ) {
		return;
		}
	pfnOpenFile ntCreateFile = ( pfnOpenFile ) GetProcAddress( hNtdll, "NtCreateFile" );
	WCHAR volumePath[ ] = { L'\\', L'\\', L'.', L'\\', fileVolume, L':', 0 };
	HANDLE hVolume = CreateFileW( volumePath, 0, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0x80, NULL );
	if ( hVolume == INVALID_HANDLE_VALUE ) {
		DWORD err = GetLastError( );
		std::wcout << L"Failed to open volume " << fileVolume << L": because of error " << err << L'\n';
		return;
		}
	HANDLE hFile = NULL;
	IO_STATUS_BLOCK iosb = { 0 };
	OBJECT_ATTRIBUTES oa = { sizeof( oa ), 0 };
	UNICODE_STRING name;
	name.Buffer = ( PWSTR ) &fileId;
	name.Length = name.MaximumLength = sizeof( fileId );
	oa.ObjectName = &name;
	oa.RootDirectory = hVolume;
	NTSTATUS stat = ntCreateFile(
		&hFile, GENERIC_READ | GENERIC_WRITE | DELETE, &oa, &iosb, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN, FILE_NON_DIRECTORY_FILE | FILE_OPEN_BY_FILE_ID, NULL, 0 );
	CloseHandle( hVolume );
	if ( !NT_SUCCESS( stat ) ) {
		std::cout << "Failed to delete file because of error " << std::hex << stat << '\n';
		}
	else {
		BYTE buffer[ ( ( sizeof( WCHAR ) * MAX_PATH ) + sizeof( ULONG ) ) ] = { 0 };
		GetFileInformationByHandleEx( hFile, FileNameInfo, buffer, sizeof( buffer ) );
		FILE_NAME_INFO* pFni = ( FILE_NAME_INFO* ) buffer;
		std::wcout << L"Deleting " << pFni->FileName << L'\n';
		DeleteFileByHandle( hFile );
		CloseHandle( hFile );
		std::cout << "File deleted\n";
		}
	}

void RecurseListDirectory( _In_z_ const wchar_t* dir, _Inout_ std::vector<std::wstring>& dirs, _In_ const bool writeToScreen ) {
	std::vector<UCHAR> idInfo( ( sizeof( FILE_ID_BOTH_DIR_INFORMATION ) + ( MAX_PATH * sizeof( UCHAR ) ) ) * 500000 );
	std::uint64_t items = 0;
	std::vector<std::wstring> downDirs;
	for ( auto& aDir : dirs ) {
		//auto aDir = dirs.at( dirs.size( ) - 1 );
		//dirs.pop_back( );
		items += ListDirectory( aDir.c_str( ), downDirs, idInfo, writeToScreen );
		++items;
		}
	std::wcout << L"Total items in directory tree of " << dir << L": " << items << std::endl;
	}

int __cdecl wmain( _In_ int argc, _In_ _Deref_prepost_count_( argc ) wchar_t** argv ) {
		{
		LONGLONG fileId = 0;
		CmdParseResult res = ParseCmdLine( argc, argv, &fileId );
		std::vector<std::wstring> dirs;
		dirs.reserve( 1000 );
		for ( size_t i = 0; i < argc; ++i ) {
			std::wcout << L"arg # " << i << L": " << argv[ i ] << std::endl;
			}
		if ( res == DISPLAY_USAGE ) {
			std::cout <<
				"Usage:\n"
				"FileId /list <C:\\Path\\To\\Directory>\n"
				"FileId /delete <volume> FileId\n"
				"\twhere <volume> is the drive letter of the drive the file id is on";
			return -1;
			}
		if ( res == LIST_DIR ) {
			dirs.emplace_back( argv[ 2 ] );
			//ListDirectory( argc < 3 ? NULL : argv[ 2 ], dirs, true );
			}
		if ( res == ENUM_DIR ) {
			std::wstring _path( argv[ 2 ] );
			std::wcout << L"Arg: " << _path << std::endl;
			std::wstring nativePath = L"\\\\?\\" + _path;
			std::wcout << L"'native' path: " << nativePath << std::endl;
			dirs.emplace_back( nativePath );
			LARGE_INTEGER startTime = { 0 };
			LARGE_INTEGER endTime = { 0 };
	
			//std::int64_t fileSizeTotal = 0;
			auto adjustedTimingFrequency = getAdjustedTimingFrequency( );
			BOOL res2 = QueryPerformanceCounter( &startTime );

			RecurseListDirectory( nativePath.c_str( ), dirs, false );

			BOOL res3 = QueryPerformanceCounter( &endTime );
	
			if ( ( !res2 ) || ( !res3 ) ) {
				std::wcout << L"QueryPerformanceCounter Failed!!!!!! Disregard any timing data!!" << std::endl;
				}
			auto totalTime = ( endTime.QuadPart - startTime.QuadPart ) * adjustedTimingFrequency;

			std::wcout << L"Time in seconds: " << totalTime << std::endl;
			
			//ListDirectory( argc < 3 ? NULL : argv[ 2 ], dirs, false );
			}
		else // DEL_FILE
			{
			return 0;
			DelFile( argv[ 2 ][ 0 ], fileId );
			
			}
		auto state = std::wcout.fail( );
		std::wcout.clear( );
		std::wcout << L"std::wcout.fail( ): " << state << std::endl;
		return 0;
		}
	}