#include "program.h"

#include <Windows.h>
#include <QRegExp>

QSoftware::QSoftware(const Software& software)
{
    // Arch
    switch(software.Architecture)
    {
    case UnKnown:
        this->Arch = ARCH_UNKNOWN;
    case X86:
        this->Arch = ARCH_X86;
    case X64:
        this->Arch = ARCH_X64;
    default:
        this->Arch = ARCH_UNKNOWN;
    }
    // DisplayName
    this->DisplayName = QString::fromStdWString(software.DisplayName);
    // InstallLocation
    this->InstalledLocation = QString::fromStdWString(software.InstallLocation);
    // Version
    this->Version = QString::fromStdWString(software.Version);

    // Create Icon from the Icon String
    // Examine the Icon String.
    QString IconString = QString::fromStdWString(software.Icon);
    int index = 0;

    QRegExp regex("(.+)\\,(\\d+)");
    if(regex.exactMatch(IconString))
    {
        IconString = regex.capturedTexts()[1];
        index = regex.capturedTexts()[2].toInt();
    }
    if(IconString.startsWith("\"")&&IconString.endsWith("\""))
    {
        int size = IconString.size();
        IconString = IconString.left(size - 1).right(size - 2);
    }

    WCHAR out[MAX_PATH];
    GetLongPathNameW(IconString.toStdWString().c_str(), out, MAX_PATH);

    QPixmap SoftwareIcon;
    HICON icon;
    UINT Unused;
    UINT ret = PrivateExtractIconsW(out, index, 48, 48, &icon, &Unused, 1, 0);
    if( ret == 1 )
        SoftwareIcon = QPixmap::fromWinHICON(icon);

    // Icon
    this->Icon = SoftwareIcon;
}


QList<QSoftware*>* QSoftware::SoftwaresOnPC()
{
    QList<QSoftware*>* SoftwareList = new QList<QSoftware*>();
    vector<Software>* list = InstalledPrograms::GetInstalledPrograms(false);
    for(vector<Software>::iterator it = list->begin(); it != list->end(); it++)
    {
        QSoftware* soft = new QSoftware(*it);
        SoftwareList->append(soft);
    }
    delete list;
    return SoftwareList;
}
