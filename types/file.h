#ifndef FILE_H
#define FILE_H

#include <QDebug>
#include <QString>

namespace Telegram {

class File
{
public:
    File(QString aFileId, qint64 aFileSize = -1,  QString filePath = QString()) :
    fileId(aFileId), fileSize(aFileSize), filePath(filePath) {}

    QString fileId;
    qint64 fileSize;
    QString filePath;
};

inline QDebug operator<< (QDebug dbg, const File &file)
{
    dbg.nospace() << qUtf8Printable(QString("Telegram::File(fileId=%1; fileSize=%2; filePath=%3)")
                                    .arg(file.fileId)
                                    .arg(file.fileSize)
                                    .arg(file.filePath));

    return dbg.maybeSpace();
}

}

#endif // FILE_H
