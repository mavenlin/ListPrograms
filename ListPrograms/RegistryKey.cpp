#include "RegistryKey.h"


RegistryKey::RegistryKey(HKEY hkey, Arch arch)
{
	this->hkey = hkey;
	this->KeyArch = arch;
}


RegistryKey::~RegistryKey(void)
{
	if( this->hkey == HKEY_LOCAL_MACHINE || this->hkey == HKEY_USERS )
		return;
	RegCloseKey(this->hkey);
}

RegistryKey* RegistryKey::OpenSubKey64(wstring subkey)
{
	HKEY hKey;
	if (RegOpenKeyEx(this->hkey, subkey.c_str(), 0, KEY_READ|KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
	{
		return NULL;
	}
	else
	{
		return new RegistryKey(hKey, X64);
	}
}

RegistryKey* RegistryKey::OpenSubKey32(wstring subkey)
{
	HKEY hKey;
	if (RegOpenKeyEx(this->hkey, subkey.c_str(), 0, KEY_READ|KEY_WOW64_32KEY, &hKey) != ERROR_SUCCESS)
	{
		return NULL;
	}
	else
	{
		return new RegistryKey(hKey, X86);
	}
}

RegistryKey* RegistryKey::OpenSubKey(wstring subkey)
{
	HKEY hKey;
	if (RegOpenKeyEx(this->hkey, subkey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		return NULL;
	}
	else
	{
		return new RegistryKey(hKey, UnKnown);
	}
}

RegistryKey* RegistryKey::OpenSubKey(wstring subkey, Arch a)
{
	HKEY hKey;
	DWORD FLAG;
	if(a==X64)
		FLAG = KEY_WOW64_64KEY;
	else if(a==X86)
		FLAG = KEY_WOW64_32KEY;
	else
		FLAG = 0;

	if (RegOpenKeyEx(this->hkey, subkey.c_str(), 0, KEY_READ|FLAG, &hKey) != ERROR_SUCCESS)
	{
		return NULL;
	}
	else
	{
		return new RegistryKey(hKey, a);
	}
}


RegistryKey& RegistryKey::HKLM()
{
	static RegistryKey Key(HKEY_LOCAL_MACHINE, UnKnown);
	return Key;
}

RegistryKey& RegistryKey::HKU()
{
	static RegistryKey Key(HKEY_USERS, UnKnown);
	return Key;
}

vector<wstring> RegistryKey::GetSubKeyNames()
{
	vector<wstring> ret;
	LONG lRet;
	DWORD dwIndex = 0;
	DWORD cbName = MAX_PATH;
	WCHAR szSubKeyName[MAX_PATH];
	while( (lRet = RegEnumKeyEx(this->hkey, dwIndex, szSubKeyName, &cbName, NULL,NULL, NULL, NULL)) != ERROR_NO_MORE_ITEMS )
	{
		if(lRet == ERROR_SUCCESS)
		{
			ret.push_back(wstring(szSubKeyName));
		}
		cbName = MAX_PATH;
		dwIndex++;
	}
	return ret;
}


wstring RegistryKey::GetValue(wstring query)
{
	WCHAR Value[MAX_PATH];
	DWORD dwSize = sizeof(Value);
	DWORD dwType;
	if (RegQueryValueEx(this->hkey, query.c_str(), NULL, &dwType, (LPBYTE)&Value, &dwSize) == ERROR_SUCCESS)
    {
		if(dwType==REG_DWORD)
		{
			DWORD * ret = (DWORD*)Value;
			return wstring(_itow((*ret),Value,10));
		}
		else if(dwType==REG_SZ)
			return wstring(Value);
		else if(dwType==REG_EXPAND_SZ)
		{
			WCHAR Expanded[MAX_PATH];
			ExpandEnvironmentStrings(Value, Expanded, MAX_PATH);
			return wstring(Expanded);
		}
    }
	else
	{
		return L"";
	}
}