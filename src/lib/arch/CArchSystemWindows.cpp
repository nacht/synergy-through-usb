/*
 * synergy -- mouse and keyboard sharing utility
 * Copyright (C) 2012 Bolton Software Ltd.
 * Copyright (C) 2004 Chris Schoeneman
 * 
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file COPYING that should have accompanied this file.
 * 
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CArchSystemWindows.h"
#include "CArchMiscWindows.h"
#include "XArchWindows.h"

#include <tchar.h>
#include <sstream>

static const char* s_settingsKeyNames[] = {
	_T("SOFTWARE"),
	_T("Synergy"),
	NULL
};

//
// CArchSystemWindows
//

CArchSystemWindows::CArchSystemWindows()
{
	// do nothing
}

CArchSystemWindows::~CArchSystemWindows()
{
	// do nothing
}

std::string
CArchSystemWindows::getOSName() const
{
#if WINVER >= _WIN32_WINNT_WIN2K
	OSVERSIONINFOEX info;
#else
	OSVERSIONINFO info;
#endif

	info.dwOSVersionInfoSize = sizeof(info);
	if (GetVersionEx((OSVERSIONINFO*) &info)) {
		switch (info.dwPlatformId) {
		case VER_PLATFORM_WIN32_NT:
			#if WINVER >= _WIN32_WINNT_WIN2K
			if (info.dwMajorVersion == 6) {
				if(info.dwMinorVersion == 0) {
					if (info.wProductType == VER_NT_WORKSTATION) {
						return "Microsoft Windows Vista";
					} else {
						return "Microsoft Windows Server 2008";
					}
				} else if(info.dwMinorVersion == 1) {
					if (info.wProductType == VER_NT_WORKSTATION) {
						return "Microsoft Windows 7";
					} else {
						return "Microsoft Windows Server 2008 R2";
					}
				}
			}
			#endif

			if (info.dwMajorVersion == 5 && info.dwMinorVersion == 2) {
				return "Microsoft Windows Server 2003";
			}
			if (info.dwMajorVersion == 5 && info.dwMinorVersion == 1) {
				return "Microsoft Windows XP";
			}
			if (info.dwMajorVersion == 5 && info.dwMinorVersion == 0) {
				return "Microsoft Windows Server 2000";
			}
			if (info.dwMajorVersion <= 4) {
				return "Microsoft Windows NT";
			}

			{
				std::stringstream result("Microsoft Windows");
				result << info.dwMajorVersion << '.' << info.dwMinorVersion;
				return result.str();
			}
			break;

		case VER_PLATFORM_WIN32_WINDOWS:
			if (info.dwMajorVersion == 4 && info.dwMinorVersion == 0) {
				if (info.szCSDVersion[1] == 'C' ||
					info.szCSDVersion[1] == 'B') {
					return "Microsoft Windows 95 OSR2";
				}
				return "Microsoft Windows 95";
			}
			if (info.dwMajorVersion == 4 && info.dwMinorVersion == 10) {
				if (info.szCSDVersion[1] == 'A') {
					return "Microsoft Windows 98 SE";
				}
				return "Microsoft Windows 98";
			}
			if (info.dwMajorVersion == 4 && info.dwMinorVersion == 90) {
				return "Microsoft Windows ME";
			}
			if (info.dwMajorVersion == 4) {
				return "Microsoft Windows unknown 95 family";
			}
			break;

		default:
			break;
		}
	}
	return "Microsoft Windows <unknown>";
}

std::string
CArchSystemWindows::getPlatformName() const
{
#ifdef _X86_
	if(isWOW64())
		return "x86 (WOW64)";
	else
		return "x86";
#else
#ifdef _AMD64_
	return "x64";
#else
	return "Unknown";
#endif
#endif
}

std::string
CArchSystemWindows::setting(const std::string& valueName) const
{
	HKEY key = CArchMiscWindows::openKey(HKEY_LOCAL_MACHINE, s_settingsKeyNames);
	if (key == NULL)
		return "";

	return CArchMiscWindows::readValueString(key, valueName.c_str());
}

void
CArchSystemWindows::setting(const std::string& valueName, const std::string& valueString) const
{
	HKEY key = CArchMiscWindows::addKey(HKEY_LOCAL_MACHINE, s_settingsKeyNames);
	if (key == NULL)
		throw XArch(std::string("could not access registry key: ") + valueName);
	CArchMiscWindows::setValue(key, valueName.c_str(), valueString.c_str());
}

bool
CArchSystemWindows::isWOW64() const
{
#if WINVER >= _WIN32_WINNT_WINXP
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	HMODULE hModule = GetModuleHandle(TEXT("kernel32"));
	if (!hModule) return FALSE;

	LPFN_ISWOW64PROCESS fnIsWow64Process =
		(LPFN_ISWOW64PROCESS) GetProcAddress(hModule, "IsWow64Process");

	BOOL bIsWow64 = FALSE;
	if(NULL != fnIsWow64Process &&
		fnIsWow64Process(GetCurrentProcess(), &bIsWow64) &&
		bIsWow64)
	{
		return true;
	}
#endif
	return false;
}
