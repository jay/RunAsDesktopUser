#pragma once

// ------------------------------------------------------------------------------------------
// Structure and operators to insert a zero-filled hex-formatted number into a std::ostream.
struct HEX
{
	HEX(unsigned long num, unsigned long fieldwidth = 8, bool bUpcase = false)
		: m_num(num), m_width(fieldwidth), m_upcase(bUpcase)
		{}

	unsigned long m_num;
	unsigned long m_width;
	bool m_upcase;
};

inline ostream& operator << ( ostream& os, const HEX & h )
{
	int fmt = os.flags();
	char fillchar = os.fill('0');
	os << "0x" << hex << (h.m_upcase ? uppercase : nouppercase) << setw(h.m_width) << h.m_num ;
	os.fill(fillchar);
	os.flags(fmt);
	return os;
}

inline wostream& operator << ( wostream& os, const HEX & h )
{
	int fmt = os.flags();
	wchar_t fillchar = os.fill(L'0');
	os << L"0x" << hex << (h.m_upcase ? uppercase : nouppercase) << setw(h.m_width) << h.m_num ;
	os.fill(fillchar);
	os.flags(fmt);
	return os;
}


// ------------------------------------------------------------------------------------------

// Convert an error code to corresponding text, returning it as a std::wstring.

inline std::wstring SysErrorMessageWithCode(DWORD dwErrCode /*= GetLastError()*/)
{
	LPWSTR pszErrMsg = NULL;
	wstringstream sRetval;
	DWORD flags =
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_FROM_SYSTEM ;

	if ( FormatMessageW(
		flags,
		NULL, 
		dwErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPWSTR) &pszErrMsg,
		0,
		NULL ) )
	{
		sRetval << pszErrMsg << L" (Error # " << dwErrCode << L" = " << HEX(dwErrCode) << L")";
		LocalFree(pszErrMsg);
	}
	else
	{
		sRetval << L"Error # " << dwErrCode << L" (" << HEX(dwErrCode) << L")";
	}
	return sRetval.str();
}

