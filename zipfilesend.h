#ifndef ZIPFILESEND_H
#define ZIPFILESEND_H

#include <QWidget>
#include <QFileInfo>
#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QThreadPool>  // 整个电脑的磁盘扫描需要很多时间,开启多线程的方式处理
#include <QRunnable>
#include <QProcess>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>




class MyRun;
class ZipFileSend : public QWidget
{
    Q_OBJECT

public:
    ZipFileSend(QWidget *parent = 0);
    ~ZipFileSend();
    void readSet();
    void readPath(QString path);
    bool copyFile(QString src_file,QString dest_file ); //按类型
    bool copyFile2(QString src_file,QString dest_file );//按时间
    QString getDestFileName(const QString& src_file_name);
    QStringList getDisk();
    void run();
    void uploadZip();
    friend class MyRun;
private:
    //=====磁盘压缩需要的信息
    QString backup_path;
    QStringList absolute_files_name;
    QStringList accept_file_types;
    QString accept_file_time;
    QString choose_date_or_type;
    QString filter_disk;
    //=====ftp连接信息======
    QString ftp_host,ftp_port,ftp_user,ftp_passwd,ftp_path;
};

class MyRun: public QRunnable
{
public:
    MyRun(QString path,ZipFileSend* zip);
    ZipFileSend* zip;
protected:
    virtual void run();
private:
    QString path;
};

#endif // ZIPFILESEND_H
