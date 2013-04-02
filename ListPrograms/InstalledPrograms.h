#ifndef INSTALLED_PROGRAMS
#define INSTALLED_PROGRAMS
#include <vector>
#include <Windows.h>
#include "RegistryKey.h"
using namespace std;

class Software
{
public:
	wstring DisplayName;
	wstring InstallLocation;
	wstring Version;
	Arch Architecture; // 32 or 64
	wstring Icon;
	Software(wstring, wstring, wstring, wstring, Arch);
};


class InstalledPrograms
{
public:
	InstalledPrograms(void);
	~InstalledPrograms(void);
	static vector<Software>* GetInstalledPrograms(bool IncludeUpdates);
private:
	static vector<Software>* GetInstalledProgramsImp(bool IncludeUpdates);
	static vector<Software>* GetUninstallKeyPrograms(RegistryKey* UninstallKey, RegistryKey* ClassesKey, vector<Software>*, bool IncludeUpdates);
	static vector<Software>* GetUserInstallerKeyPrograms(RegistryKey* uInstallerKey, vector<Software>* ExistingProgramList);
};

#endif