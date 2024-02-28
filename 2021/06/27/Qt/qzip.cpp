#include "qzip.h"
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QVector>
#include "QtGui/private/qzipreader_p.h"
#include "QtGui/private/qzipwriter_p.h"

//! \brief zipReader    zip包解压缩
//! \param zipPath      压缩包路径
//! \param zipDir       解压缩目录
//! \return 是否解压成功
bool qzip::zipReader(QString zipPath, QString zipDir)
{
    QDir tempDir;
    if(!tempDir.exists(zipDir)) tempDir.mkpath(zipDir);
    QZipReader reader(zipPath);
    return reader.extractAll(zipDir);
}

//! \brief zipWriter    压缩zip包
//! \param zipPath      压缩包路径
//! \param fileList     需要压缩的文件列表
void qzip::zipWriter(QString zipPath, QVector<QString> fileList)
{
    QZipWriter *writer = new QZipWriter(zipPath);

    // 遍历文件列表
    for (int i=0;i<fileList.size();i++) {
        //添加需要压缩的文件
        QFile file(fileList[i]);
        if (!file.exists()) continue;
        file.open(QIODevice::ReadOnly);
        writer->addFile(fileList[i], file.readAll());
        file.close();
    }
    writer->close();
    if(writer)
    {
        delete writer;
        writer = nullptr;
    }
}

//! \brief zipWriter    压缩zip包
//! \param zipPath      压缩包路径
//! \param fileList     需要压缩的文件列表
void qzip::zipWriter(QString zipPath, QFileInfoList fileList)
{
    QZipWriter *writer = new QZipWriter(zipPath);

    // 遍历文件列表
    foreach (QFileInfo fileInfo, fileList) {
        //添加需要压缩的文件
        QFile file(fileInfo.filePath());
        if (!file.exists()) continue;
        file.open(QIODevice::ReadOnly);
        writer->addFile(fileInfo.fileName(), file.readAll());
        file.close();
    }
    writer->close();
    if(writer)
    {
        delete writer;
        writer = nullptr;
    }
}

qzip::qzip(QObject *parent) : QObject(parent)
{

}
