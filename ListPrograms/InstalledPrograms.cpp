#include "InstalledPrograms.h"
#include <algorithm>
#include <regex>

static wstring GetInstallerKeyNameFromGuid(wstring GuidName);
static void AddToList(vector<Software>* TheList, Software software);


Software::Software(wstring name, wstring version, wstring location, wstring icon, Arch arch): DisplayName(name), Version(version), InstallLocation(location), Icon(icon), Architecture(arch)
{
}

InstalledPrograms::InstalledPrograms(void)
{
}


InstalledPrograms::~InstalledPrograms(void)
{
}



vector<Software>* InstalledPrograms::GetInstalledPrograms(bool IncludeUpdates)
{
	return GetInstalledProgramsImp(IncludeUpdates);
}

vector<Software>* InstalledPrograms::GetInstalledProgramsImp(bool IncludeUpdates)
{
    vector<Software>* SoftwareList = new vector<Software>();
	
	RegistryKey* ClassesKey = RegistryKey::HKLM().OpenSubKey(L"Software\\Classes\\Installer\\Products");

	// The Classes\\Installer\\Products key is shared 
	// Documentation Here http://msdn.microsoft.com/en-us/library/windows/desktop/aa384253(v=vs.85).aspx

	RegistryKey* Wow64UninstallKey = RegistryKey::HKLM().OpenSubKey32(L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
	SoftwareList = GetUninstallKeyPrograms(Wow64UninstallKey, ClassesKey, SoftwareList, IncludeUpdates);

	RegistryKey* UninstallKey = RegistryKey::HKLM().OpenSubKey64(L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
	SoftwareList = GetUninstallKeyPrograms(UninstallKey, ClassesKey, SoftwareList, IncludeUpdates);

	vector<wstring> subkeys = RegistryKey::HKU().GetSubKeyNames();
	
	for(vector<wstring>::iterator it = subkeys.begin(); it!=subkeys.end(); it++)
	{
		// These Two Keys are Both Shared
		// Documentation Here http://msdn.microsoft.com/en-us/library/windows/desktop/aa384253(v=vs.85).aspx

		wstring uninstallsubs = (*it) + L"\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
		RegistryKey* UninstallKey = RegistryKey::HKU().OpenSubKey(uninstallsubs);
		SoftwareList = GetUninstallKeyPrograms(UninstallKey, ClassesKey, SoftwareList, IncludeUpdates);
		if(UninstallKey)
			delete UninstallKey;

		wstring installersubs = (*it) + L"\\Software\\Microsoft\\Installer\\Products";
		RegistryKey* InstallerKey = RegistryKey::HKU().OpenSubKey(installersubs);
		SoftwareList = GetUserInstallerKeyPrograms(InstallerKey, SoftwareList);
		if(InstallerKey)
			delete InstallerKey;
	}

	if(UninstallKey)
		delete UninstallKey;
	if(Wow64UninstallKey)
		delete Wow64UninstallKey;
	if(ClassesKey)
		delete ClassesKey;
	std::sort(SoftwareList->begin(), SoftwareList->end(), [](Software s1, Software s2)->bool{std::transform(s1.DisplayName.begin(), s1.DisplayName.end(), s1.DisplayName.begin(), ::tolower);std::transform(s2.DisplayName.begin(), s2.DisplayName.end(), s2.DisplayName.begin(), ::tolower);return s1.DisplayName.compare(s2.DisplayName)<0;} );
	return SoftwareList;

}

vector<Software>* InstalledPrograms::GetUserInstallerKeyPrograms(RegistryKey* uInstallerKey, vector<Software>* ExistingProgramList)
{
	if(uInstallerKey==NULL)
		return ExistingProgramList;

	RegistryKey * UserData32 = RegistryKey::HKLM().OpenSubKey32(L"Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData");
	RegistryKey * UserData64 = RegistryKey::HKLM().OpenSubKey64(L"Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData");

	if(UserData32==NULL && UserData64==NULL)
		return ExistingProgramList;


	RegistryKey* UserDatas[2] = {UserData32, UserData64};
	for(int two = 0; two < 2; two++)
	{
		RegistryKey * UserData = UserDatas[two];
		if(UserData==NULL)
			continue;

		vector<wstring> subkeys = uInstallerKey->GetSubKeyNames();
		for(vector<wstring>::iterator it = subkeys.begin(); it!=subkeys.end(); it++)
		{
			vector<wstring> userDataSubKeys = UserData->GetSubKeyNames();
			bool ProductFound = false;
			for(vector<wstring>::iterator userdatait = userDataSubKeys.begin(); userdatait!=userDataSubKeys.end(); userdatait++) 
			{
				//Ignore the LocalSystem account
				if (userdatait->compare(L"S-1-5-18")==0) {
					continue;
				}
			
				RegistryKey * ProductsKey = RegistryKey::HKLM().OpenSubKey(wstring(L"Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\").append(*userdatait).append(L"\\Products"),UnKnown);
                
				if ((ProductsKey != NULL)) 
				{
					vector<wstring> LmProductGuids = ProductsKey->GetSubKeyNames();
					for(vector<wstring>::iterator productit = LmProductGuids.begin(); productit!=LmProductGuids.end(); productit++) 
					{
						if (productit->compare(*it)==0) 
						{	
							RegistryKey * UserDataProgramKey = RegistryKey::HKLM().OpenSubKey(wstring(L"Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\").append(*userdatait).append(L"\\Products\\").append(*productit).append(L"\\InstallProperties"),UnKnown);
							if(UserDataProgramKey!=NULL)
							{
								if ( UserDataProgramKey->GetValue(L"SystemComponent").compare(L"")==0 || _wtoi(UserDataProgramKey->GetValue(L"SystemComponent").c_str())!=1 ) 
								{
									RegistryKey * temp = uInstallerKey->OpenSubKey(*it, uInstallerKey->KeyArch);

									// Name 
									wstring Name1 = temp->GetValue(L"ProductName");
									wstring Name2 = UserDataProgramKey->GetValue(L"DisplayName");
									wstring Name = L"";
									if(Name1.compare(L"")==0)
										Name = Name2;
									else
										Name = Name1;

									// Version
									wstring ProgVersion = UserDataProgramKey->GetValue(L"DisplayVersion");

									// InstallLocation
									wstring InstallLocation = UserDataProgramKey->GetValue(L"InstallLocation");

									// Icon
									wstring Icon1 = temp->GetValue(L"ProductIcon");
									wstring Icon2 = UserDataProgramKey->GetValue(L"DisplayIcon");
									wstring Icon = L"";
									if(Icon1.compare(L"")==0)
										Icon = Icon2;
									else
										Icon = Icon1;

									if ( Name.compare(L"")!=0 ) 
									{
										AddToList(ExistingProgramList, Software(Name,ProgVersion,InstallLocation,Icon,UserData->KeyArch));
										ProductFound = true;
									}
									delete temp;
								}
								delete UserDataProgramKey;
							}
							break;
						}
					}
					if(ProductFound) 
					{
						break; // TODO: might not be correct. Was : Exit For
					}
					delete ProductsKey;
				}
			}
		}
		delete UserData;
	}
    return ExistingProgramList;
}

vector<Software>* InstalledPrograms::GetUninstallKeyPrograms(RegistryKey* UninstallKey, RegistryKey* ClassesKey, vector<Software>* ExistingProgramList, bool IncludeUpdates)
{
    //Make sure the key exists
    if (UninstallKey != NULL)
	{
        //Loop through all subkeys (each one represents an installed program)
		vector<wstring> UninstallSubKeys = UninstallKey->GetSubKeyNames();
		for(vector<wstring>::iterator SubKeyName = UninstallSubKeys.begin(); SubKeyName!=UninstallSubKeys.end(); SubKeyName++) 
		{
			RegistryKey * CurrentSubKey = UninstallKey->OpenSubKey(*SubKeyName, UninstallKey->KeyArch);
			if (CurrentSubKey == NULL)
				continue;

            int IsSystemComponent = 1;
            bool ErrorCheckingSystemComponent = false;
            
			wstring issystemcomponent = CurrentSubKey->GetValue(L"SystemComponent");
			if( issystemcomponent.compare(L"")==0 || _wtoi(issystemcomponent.c_str())!=1 )
				IsSystemComponent = 0;
            
            if (IsSystemComponent != 1) 
			{
                //If the WindowsInstaller flag is set then add the key name to our list of Windows Installer GUIDs
                if (CurrentSubKey->GetValue(L"WindowsInstaller").compare(L"")==0 || _wtoi(CurrentSubKey->GetValue(L"WindowsInstaller").c_str())!= 1) 
				{
                    wregex WindowsUpdateRegEx(L"KB[0-9]{6}$");
                    
					// Release Type
					wstring ProgramReleaseType = CurrentSubKey->GetValue(L"ReleaseType");

					// Version
                    wstring ProgVersion = CurrentSubKey->GetValue(L"DisplayVersion");
					// Name 
					wstring Name = CurrentSubKey->GetValue(L"DisplayName");
					// InstallLocation
					wstring InstallLocation = CurrentSubKey->GetValue(L"InstallLocation");
                    // Icon
					wstring Icon = CurrentSubKey->GetValue(L"DisplayIcon");

					//Check to see if this program is classed as an update
					if (regex_match(*SubKeyName, WindowsUpdateRegEx) == true || \
						CurrentSubKey->GetValue(L"ParentKeyName").compare(L"") != 0 ||\
						ProgramReleaseType.compare(L"Security Update")==0 ||\
						ProgramReleaseType.compare(L"Update Rollup")==0 ||\
						ProgramReleaseType.compare(L"Hotfix")==0 )
					{
                        if (IncludeUpdates) 
						{
                            //Add the program to our list if we are including updates in this search
                            if ( Name.compare(L"")!=0 ) 
							{
                                AddToList(ExistingProgramList, Software(Name,ProgVersion,InstallLocation,Icon,UninstallKey->KeyArch));
                            }
                        }
                    } 
					else 
					{
						wstring uninstallvalue = CurrentSubKey->GetValue(L"UninstallString");
						if (uninstallvalue.compare(L"")!=0) 
						{
                            if ( Name.compare(L"")!=0 ) 
							{
								AddToList(ExistingProgramList, Software(Name,ProgVersion,InstallLocation,Icon,UninstallKey->KeyArch));
                            }
                        }
                    }
                } 

				//If WindowsInstaller
				else 
				{
                    // Name
					wstring Name1 = L"";
					wstring  Icon1 = L"";
                    wstring MsiKeyName = GetInstallerKeyNameFromGuid(*SubKeyName);
					RegistryKey * CrGuidKey = ClassesKey->OpenSubKey(MsiKeyName,ClassesKey->KeyArch);
                    if (CrGuidKey != NULL)
					{
                        Name1 = CrGuidKey->GetValue(L"ProductName");
						Icon1 = CrGuidKey->GetValue(L"ProductIcon");
						delete CrGuidKey;
                    }
					wstring Name2 = CurrentSubKey->GetValue(L"DisplayName");
					wstring Name = L"";
					if(Name1.compare(L"")==0)
						Name = Name2;
					else
						Name = Name1;

                    // Version
					wstring ProgVersion = CurrentSubKey->GetValue(L"DisplayVersion");
                    

					// InstallLocation
					wstring InstallLocation = CurrentSubKey->GetValue(L"InstallLocation");

					// Icon
					wstring Icon2 = CurrentSubKey->GetValue(L"DisplayIcon");
					wstring Icon = L"";
					if(Icon1.compare(L"")==0)
						Icon = Icon2;
					else
						Icon = Icon1;

					if (Name.compare(L"")!=0)
					{
                        AddToList(ExistingProgramList, Software(Name,ProgVersion,InstallLocation,Icon,UninstallKey->KeyArch));
                    }
                }
            }
        }
    }
    return ExistingProgramList;
}







// Util Functions

static void AddToList(vector<Software>* TheList, Software software)
{
	int index = -1;
	for(int i = 0; i<TheList->size(); i++)
	{
		if(TheList->at(i).DisplayName.compare(software.DisplayName)==0)
		{
			index = i;
			break;
		}
	}
	if(index == -1)
		TheList->push_back(software);
	else
	{
		Software duplicate = TheList->at(index);
		
		// Merge Architecture
		if( software.Architecture!=UnKnown && duplicate.Architecture!=UnKnown && duplicate.Architecture!=software.Architecture )
		{
			TheList->push_back(software);
			return;
		}
		else
		{
			if(software.Architecture==UnKnown)
				software.Architecture = duplicate.Architecture;
		}

		// Merge Icon
		if(software.Icon.compare(L"")!=0 && duplicate.Icon.compare(L"")!=0 && software.Icon.compare(duplicate.Icon)!=0)
		{
			TheList->push_back(software);
			return;
		}
		else
		{
			if(software.Icon.compare(L"")==0)
				software.Icon = duplicate.Icon;
		}

		// Merge Location
		if(software.InstallLocation.compare(L"")!=0 && duplicate.InstallLocation.compare(L"")!=0 && software.InstallLocation.compare(duplicate.InstallLocation)!=0)
		{
			TheList->push_back(software);
			return;
		}
		else
		{
			if(software.InstallLocation.compare(L"")==0)
				software.InstallLocation = duplicate.InstallLocation;
		}

		// Merge Version
		if(software.Version.compare(L"")!=0 && duplicate.Version.compare(L"")!=0 && software.Version.compare(duplicate.Version)!=0)
		{
			TheList->push_back(software);
			return;
		}
		else
		{
			if(software.Version.compare(L"")==0)
				software.Version = duplicate.Version;
		}
		TheList->erase(TheList->begin()+index);
		TheList->push_back(software);
	}
}

static wstring ReverseString(wstring input);
static void replaceAll(wstring& str, const wstring& from, const wstring& to);
static vector<wstring> split( const wstring& text, wchar_t delimiter );

static wstring GetInstallerKeyNameFromGuid(wstring GuidName)
{
	replaceAll(GuidName, L"{", L"");
	replaceAll(GuidName, L"}", L"");
	vector<wstring> MsiNameParts = split(GuidName, L'-');
    wstring MsiName;
    //Just reverse the first 3 parts
    for (int i = 0; i <= 2; i++) {
        MsiName.append(ReverseString(MsiNameParts[i]));
    }
    //For the last 2 parts, reverse each character pair
    for (int j = 3; j <= 4; j++) {
        for (int i = 0; i <= MsiNameParts[j].length() - 1; i++) {
            MsiName.append(wstring(1,MsiNameParts[j].c_str()[i + 1]));
            MsiName.append(wstring(1,MsiNameParts[j].c_str()[i]));
            i += 1;
        }
    }
    return MsiName;
}

static wstring ReverseString(wstring input)
{
	std::reverse(input.begin(), input.end());
	return input;
}

static void replaceAll(wstring& str, const wstring& from, const wstring& to) 
{
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != wstring::npos) 
	{
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

static vector<wstring> split( const wstring& text, wchar_t delimiter )
{
    vector<wstring> result;
 
    wstring::size_type start = 0;
    wstring::size_type end   = text.find( delimiter, start );
 
    while( end != wstring::npos )
    {
        wstring token = text.substr( start, end - start );
 
        result.push_back( token );
 
        start = end + 1;
        end   = text.find( delimiter, start );
    }
 
    result.push_back( text.substr( start ) );
 
    return result;
}