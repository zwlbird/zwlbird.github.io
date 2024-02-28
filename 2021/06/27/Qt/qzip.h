#ifndef QZIP_H
#define QZIP_H

#include <QFileInfoList>
#include <QObject>

class qzip : public QObject
{
    Q_OBJECT
public:
    explicit qzip(QObject *parent = nullptr);

    static bool zipReader(QString zipPath="test.zip", QString zipDir="");
    static void zipWriter(QString zipPath, QVector<QString> fileList);
    static void zipWriter(QString zipPath="test.zip", QFileInfoList fileList=QFileInfoList());
signals:

public slots:
};

#endif // QZIP_H
