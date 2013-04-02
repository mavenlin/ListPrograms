#ifndef REGISTRY_KEY
#define REGISTRY_KEY

#include <Windows.h>
#include <string>
#include <vector>

using namespace std;

enum Arch
{
	UnKnown = 0,
	X86 = 1,
	X64 = 2
};

class RegistryKey
{
public:
	~RegistryKey(void);
	RegistryKey* OpenSubKey64(wstring subkey);
	RegistryKey* OpenSubKey32(wstring subkey);
	RegistryKey* OpenSubKey(wstring subkey);
	RegistryKey* OpenSubKey(wstring subkey, Arch a);

	vector<wstring> GetSubKeyNames();
	wstring GetValue(wstring query);

	static RegistryKey& HKLM();
	static RegistryKey& HKU();

	Arch KeyArch; // Indicate whether the Key is a 64 bit or 32 bit Key.
private:
	HKEY hkey;
	RegistryKey(HKEY, Arch); // The constructor is private, not explicit initilization is available.
};

#endif