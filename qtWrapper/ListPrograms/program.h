#ifndef PROGRAM_H
#define PROGRAM_H

#include <QPixmap>
#include "InstalledPrograms.h"
#include <QList>

enum ARCH
{
    ARCH_UNKNOWN = 0,
    ARCH_X86 = 1,
    ARCH_X64 = 2
};

class QSoftware
{
public:
    QString DisplayName;
    QString InstalledLocation;
    QString Version;
    ARCH Arch;
    QPixmap Icon;
    QSoftware(const Software&);
    static QList<QSoftware*>* SoftwaresOnPC();
};

#endif // PROGRAM_H
