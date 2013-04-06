#include <iostream>
#include "InstalledPrograms.h"

using namespace std;

int main()
{
	vector<Software>* list = InstalledPrograms::GetInstalledPrograms(false);
	for(vector<Software>::iterator iter = list->begin(); iter!=list->end(); iter++)
	{
		wcout<<iter->DisplayName<<L"  "<<iter->InstallLocation<<L"  "<<iter->Version<<L"  "<<iter->Icon<<L"  "<<iter->Architecture<<endl;
	}
}