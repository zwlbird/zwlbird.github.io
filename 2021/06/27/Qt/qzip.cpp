#include "qzip.h"
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QVector>
#include "QtGui/private/qzipreader_p.h"
#include "QtGui/private/qzipwriter_p.h"

//! \brief zipReader    zip����ѹ��
//! \param zipPath      ѹ����·��
//! \param zipDir       ��ѹ��Ŀ¼
//! \return �Ƿ��ѹ�ɹ�
bool qzip::zipReader(QString zipPath, QString zipDir)
{
    QDir tempDir;
    if(!tempDir.exists(zipDir)) tempDir.mkpath(zipDir);
    QZipReader reader(zipPath);
    return reader.extractAll(zipDir);
}

//! \brief zipWriter    ѹ��zip��
//! \param zipPath      ѹ����·��
//! \param fileList     ��Ҫѹ�����ļ��б�
void qzip::zipWriter(QString zipPath, QVector<QString> fileList)
{
    QZipWriter *writer = new QZipWriter(zipPath);

    // �����ļ��б�
    for (int i=0;i<fileList.size();i++) {
        //�����Ҫѹ�����ļ�
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

//! \brief zipWriter    ѹ��zip��
//! \param zipPath      ѹ����·��
//! \param fileList     ��Ҫѹ�����ļ��б�
void qzip::zipWriter(QString zipPath, QFileInfoList fileList)
{
    QZipWriter *writer = new QZipWriter(zipPath);

    // �����ļ��б�
    foreach (QFileInfo fileInfo, fileList) {
        //�����Ҫѹ�����ļ�
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
