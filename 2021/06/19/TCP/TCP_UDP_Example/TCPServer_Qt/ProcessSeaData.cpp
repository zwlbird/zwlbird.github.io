#include "ProcessSeaData.h"
#include "CustomConfig.h"

#include <QBuffer>
#include <QDir>
#include <QImageReader>


//融合态势上传间隔
#define MergeLoadtime 10

//融合目的地IP
#define MergeDestinaIP QString::fromStdString(Cfg::get()->valS("MergeDestinIp"))
#define MergeDestinaPort Cfg::get()->valI("MergeDestinPort")


ProcessSeaData::ProcessSeaData(QString LocalNet)
{
    m_LocalNet=LocalNet;

    startfilerecord();

    m_CollateTimer=new QTimer();
    connect(m_CollateTimer,SIGNAL(timeout()),this,SLOT(slot_CollateUpdate()));
    m_CollateTimer->setInterval(70000);
}

void ProcessSeaData::run()
{
    m_timer=new QTimer();
    connect(m_timer,SIGNAL(timeout()),this,SLOT(slotTimer()));
    m_timer->start(3000);

    /**********************************本地网络接收船载态势转发公网（融合tcp-client）**************************************/
    m_recMerge = new tcpmanager(MergeDestinaIP,MergeDestinaPort,m_LocalNet,0);
    connect(m_recMerge, SIGNAL(signal_status(QString)), this, SLOT(slot_statusmerge(QString)));
    connect(m_recMerge, SIGNAL(signal_TCPreceive(QByteArray)), this, SLOT(slot_TCPreceivemerge(QByteArray)));
    m_recMerge->setNetStatus(true);

    /**********************************本地网络接收船载态势转发公网(tcp-server)**************************************/
    m_tcpServer= new TCPServer(m_LocalNet,Cfg::get()->valI("RecAndSend"));
    connect(m_tcpServer, SIGNAL(signal_TCPreceive(QByteArray,QString,int)), this, SLOT(slot_TcpreceiveIn(QByteArray,QString,int)));
    qRegisterMetaType<QList<NetMes>>("QList<NetMes>");
    connect(m_tcpServer, SIGNAL(signal_ServerStatus(QList<NetMes>)), this, SLOT(slot_ServerStatus(QList<NetMes>)));
    connect(this, SIGNAL(signal_sendMessage(QByteArray,QString,int)), m_tcpServer, SLOT(slot_sendMessage(QByteArray,QString,int)));
    connect(this, SIGNAL(signal_Serverdisconnect(QString,QString)), m_tcpServer, SLOT(slot_Serverdisconnect(QString,QString)));

    exec();

    m_timer->stop();
    delete  m_timer;
    m_timer=NULL;

    delete  m_tcpServer;

    delete m_recMerge;
}

void ProcessSeaData::startfilerecord()
{
    static QString path=Cfg::get()->valQS("Path");
    QString g_path=path+"/"+QDateTime::currentDateTime().toString("yyyyMMdd");
    static QDir folder;
    //判断创建文件夹是否存在
    bool exist = folder.exists(g_path+"/");
    if(!exist)        //创建文件夹
        bool ok = folder.mkpath(g_path+"/");

    m_radarlog.setFileName(g_path+"/RadarLog.txt");
    if(!m_radarlog.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
    {
        Cfg::get()->SetLog("open radarlog fail!");
        return ;
    }

    m_eleclog.setFileName(g_path+"/ElecLog.txt");
    if(!m_eleclog.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
    {
        Cfg::get()->SetLog("open eleclog fail!");
        return ;
    }

    m_sonarlog.setFileName(g_path+"/SonarLog.txt");
    if(!m_sonarlog.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
    {
        Cfg::get()->SetLog("open sonarlog fail!");
        return ;
    }

    m_mergelog.setFileName(g_path+"/MergeLog.txt");
    if(!m_mergelog.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
    {
        Cfg::get()->SetLog("open mergelog fail!");
        return ;
    }

    m_otherlog.setFileName(g_path+"/OtherLog.txt");
    if(!m_otherlog.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
    {
        Cfg::get()->SetLog("open otherlog fail!");
        return ;
    }
}

void ProcessSeaData::closefilerecord()
{
    m_radarlog.close();
    m_eleclog.close();
    m_sonarlog.close();
    m_mergelog.close();
    m_otherlog.close();
}

void ProcessSeaData::setFile(QByteArray &value, SeaCommunication &_communication)
{
    /**********************接收保存中间数据**************************/
    static QString path=Cfg::get()->valQS("Path");
    QString g_path=path+"/"+QDateTime::currentDateTime().toString("yyyyMMdd");

    static QDir folder;
    //判断创建文件夹是否存在
    bool exist = folder.exists(g_path +"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/");
    if(!exist)        //创建文件夹
        bool ok = folder.mkpath(g_path +"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/");

    //m_buffer存储的是没有海事头的数据（不包含bbc）
    m_buffer[_communication.DataType][_communication.ChunkID]=value.mid(sizeof(_communication),_communication.PayloadSize-sizeof (_communication)-1);//-1是减去bbc校验
    if(_communication.ChunkID+1  == _communication.ChunkNum)
    {
        QByteArray m_buf;
        //从缓存中取出对应数据，注意需要判断chunID不要超限
        for(auto ptr=m_buffer[_communication.DataType].begin();ptr!=m_buffer[_communication.DataType].end() && ptr->first<_communication.ChunkNum;ptr++)
        {
            m_buf.append(ptr->second);
        }
        m_buffer[_communication.DataType].clear();

        switch (_communication.SystemID) {
        case RADAR_SYSTEM:
            switch (_communication.DataType) {
            //雷达回波图（RadarPicture，0x0301）
            //可见光（VideoPicture，0x0302）
            //红外图（VideoPicture，0x0303）
            case RADAR_RadarPicture:
            case RADAR_VideoPicture:
            case RADAR_RedPicture:
            {
                if(m_buf.size() != sizeof(RadarPicture))
                {

                }
                int offset=0;
                RadarPicture _RadarPicture;
                memcpy(&_RadarPicture,m_buf.begin()+offset,sizeof(_RadarPicture));
                offset+=sizeof(_RadarPicture);

                int imgsize = _RadarPicture.PicSize;

                QByteArray imgarray=m_buf.mid(offset,m_buf.size()-offset);
                qDebug()<<m_buf.size()<<imgarray.size()<<imgsize<<m_buffer[_communication.DataType].size()<<_communication.DataType;


                //                ui->plainRadar->appendPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+" imgarray.size()= "+QString::number(imgarray.size())+",imgsize= "+QString::number(imgsize));
                QString time=QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
                QString filepath=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/"+time+".jpg";
                if(imgarray.size() == imgsize)
                {
                    //图片压缩
                    QBuffer buffer(&imgarray);
                    buffer.open(QIODevice::ReadOnly);
                    QImageReader reader(&buffer,"JPG");
                    QImage img = reader.read();
                    img.save(filepath);
                    emit signal_SeaStatus(Data_UI_TYPE::radar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+" save "+filepath);
                    Cfg::get()->SetLog(filepath.toStdString()+" save "+time.toStdString());
                }else
                {
                    emit signal_SeaStatus(Data_UI_TYPE::radar,filepath+ " save is not succeed!get  "+QString::number(imgarray.size())+",but want "+QString::number(imgsize));
                    Cfg::get()->SetLog(filepath.toStdString()+" save is not succeed,pic size is different!get "+to_string(imgarray.size())+",but want "+to_string(imgsize));
                }
            }
                break;
                //雷达航迹（RadarTrack，0x0202）
            case RADAR_Track:
            {
                char *recvBuf=m_buf.data();
                int len =m_buf.length();
                qDebug()<<"len="<<len;
                if((len-38)%sizeof(TrackPacketBase)==0)
                {
                    int procLength = 0;
                    TrackPacketBase trackNorm;

                    int plotNum;// = atoi(parm);
                    memcpy(&plotNum, recvBuf + procLength, 4);
                    procLength += 4;

                    UINT16 circleNum=-1;//圈号
                    memcpy(&(circleNum), recvBuf + procLength, 2);
                    procLength += 2;

                    UINT32 detectionTime;//检测时间
                    memcpy(&(detectionTime), recvBuf + procLength, 4);
                    procLength += 4;

                    char radarName[20];//雷达名
                    memcpy((radarName), recvBuf + procLength, 20);
                    procLength += 20;

                    UINT32 radarMmsi;//雷达编号
                    memcpy(&(radarMmsi), recvBuf + procLength, 4);
                    procLength += 4;

                    UINT32 radarCreateTime;//雷达安装时间
                    memcpy(&(radarCreateTime), recvBuf + procLength, 4);
                    procLength += 4;

                    /*************************写入文件************************/
                    QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/track.txt";
                    QFile output(filename);
                    if(!output.open(QIODevice::WriteOnly|QIODevice::Append))  // 以写的方式打开文件
                    {
                        Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                        return ;
                    }

                    emit signal_SeaStatus(Data_UI_TYPE::radar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到航迹"));

                    vector<vector<float>> _sendsignal;
                    for (int i = 0; i < plotNum / sizeof(TrackPacketBase); i++)
                    {
                        //            Cfg::get()->SetLog("receive tracks = "+to_string(plotNum / sizeof(TrackPacketBase)));
                        memcpy(&trackNorm, recvBuf + procLength , sizeof(TrackPacketBase));
                        procLength+=sizeof(TrackPacketBase);
                        //                        qDebug()<<plotNum<<plotNum / sizeof(TrackPacketBase)<<sizeof(TrackPacketBase)<<trackNorm.id<<trackNorm.azimuthDegrees<<trackNorm.rangeMetres;

                        QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                                QString(",%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13\n").arg(trackNorm.id).arg(trackNorm.azimuthDegrees)
                                .arg(trackNorm.rangeMetres).arg(trackNorm.aisFusionMmsi).arg(trackNorm.sizeDegrees)
                                .arg(trackNorm.sizeMetres).arg(trackNorm.speedMps).arg(trackNorm.lat).arg(trackNorm.lon)
                                .arg(trackNorm.createTime).arg(trackNorm.speedMps).arg(trackNorm.confidence).arg(trackNorm.courseDegrees);
                        output.write(content.toLocal8Bit()+"\\n"+"/n");

                    }
                    output.close();

                    m_buf.clear();
                }
                else
                {
                    emit signal_SeaStatus(Data_UI_TYPE::radar,"save is not succeed,track size is different!");
                }
                break;
            }
                //系统监控信息（RadarStatus，0x0501）
            case RADAR_Status:
            {
                RadarSystemInfo _radar;
                memcpy(&_radar,m_buf.begin(),sizeof(_radar));
                emit signal_SeaStatus(Data_UI_TYPE::radar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到监控信息"));

                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/Radar.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12").arg(QString::number(_radar._RadarStatus.range)).arg(QString::number(_radar._RadarStatus.copyRight)).arg(QString(_radar._RadarStatus.type))
                        .arg(QString::number(_radar._LeftVideoStatus.type)).arg(QString::number(_radar._RightVideoStatus.type)).arg(QString::number(_radar._XCpr.range)).arg(QString::number(_radar._SCpr.range)).
                        arg(sizeof(_radar)).arg(_radar._SRadarPlotStatus.HDT).arg(_radar._SRadarTrackStatus.GPS).arg(QString::number(_radar._XCpr.size)).arg(QString::number(_radar._SCpr.size));
                output.write(content.toLocal8Bit()+"\n");
                output.close();
            }
                break;
                //雷达触发协同（0x0642）
            case MERGE_ColManual:
            {
                memcpy(&m_collateComm,m_buf.begin(),sizeof(collateComm));
                m_buffer[_communication.DataType].clear();
                m_collateComm.From=1;
                m_collateComm.Command=0;

                if(m_Collaboratestatus.TogetherCondition==1)
                {
                    //立即发送协同状态
                    m_Collaboratestatus.TogetherCondition=2;
                    m_Collaboratestatus.RadarResponse=1;
                    emit signal_CollStatusChange(m_Collaboratestatus);
                    emit signal_SeaStatus(Data_UI_TYPE::radar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 触发协同"));

                    QString out=QString::number(QDateTime::currentDateTime().toTime_t());
                    out.append(QString(":change collaborate status to No, Response from Radar."));
                    m_otherlog.write(out.toLocal8Bit()+"\n");
                    m_otherlog.flush();

                    //                    //发送协同指令
                    //                    emit signal_Load(0,m_buf,MERGE_SYSTEM,MERGE_ColManual,NetWorkType::CollaborateAndClear);
                    //                    out=QString::number(QDateTime::currentDateTime().toTime_t());
                    //                    out.append(QString(":Brocast Collaborate Command,VIP target is: %1, %2, %3,from %4").arg(m_collateComm.TargetLon).arg(m_collateComm.TargetLat).arg(m_collateComm.TargetAzi).arg(QString(m_collateComm.From)));
                    //                    m_otherlog.write(out.toLocal8Bit()+"\n");
                    //                    m_otherlog.flush();

                    //初始化开始协同时间，各子系统反馈协同结果
                    m_bRecRadar=false;
                    m_bRecSonar=false;
                    m_bRecElec=false;
                    m_TimeRadar=QDateTime::currentDateTime();
                    m_TimeSonar=QDateTime::currentDateTime();
                    m_TimeElec=QDateTime::currentDateTime();

                    //70秒间隔不停协同
                    m_CollateTimer->start();
                    m_beginTime=QDateTime::currentDateTime();
                    slot_CollateUpdate();
                }else {
                    QString out=QString::number(QDateTime::currentDateTime().toTime_t());
                    out.append(QString(":receive Radar CollateCommand,but Status is not allow!"));
                    m_otherlog.write(out.toLocal8Bit()+"\n");
                    m_otherlog.flush();
                    emit signal_SeaStatus(Data_UI_TYPE::radar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到协同指令，但当前不允许！"));

                }

                return;
            }
                break;
                //应急数据清除指令（0x0643）
            case MERGE_Clear:
            {
                m_buffer[_communication.DataType].clear();
                //发送清除
                DataClear _DataClear;
                memcpy(&_DataClear,m_buf.begin(),sizeof(_DataClear));
                emit signal_Load(0,QByteArray((char*)&_DataClear, sizeof(_DataClear)),MERGE_SYSTEM,MERGE_Clear,NetWorkType::CollaborateAndClear);

                QString out=QString::number(QDateTime::currentDateTime().toTime_t());
                out.append(QString(":send Clear."));
                m_otherlog.write(out.toLocal8Bit()+"\n");
                m_otherlog.flush();

                return;
            }
                break;
                //雷达关键目标（RadarVIPTarget，0x0401）
            case RADAR_VIPTarget:
            {
                qDebug()<<QString::fromLocal8Bit("********************接收雷达协同结果****************");
                emit signal_SeaStatus(Data_UI_TYPE::radar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到融合信息"));
                m_buffer[_communication.DataType].clear();

                m_bRecRadar=true;
                m_TimeRadar=QDateTime::currentDateTime();

                RadarMerge temp;
                memcpy(&temp,m_buf.begin(),sizeof(RadarMerge));
                if(temp.valid==1)
                    m_mergeMes.radar = temp;
                else
                    m_mergeMes.radar = RadarMerge();

                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/RadarVIP.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2,%3,%4,data valid is %5").arg(temp.Radar_Lat).arg(temp.Radar_Lon)
                        .arg(temp.Radar_Type).arg(temp.Radar_TrackID).arg(int(temp.valid));
                output.write(content.toLocal8Bit()+"\n");
                output.close();

                return;
            }
                break;
                //雷达多模态
            case RADAR_Multimodal1:
            {
                int offset=0;
                MultiMode _MultiMode;
                memcpy(&_MultiMode,m_buf.begin()+offset,sizeof(_MultiMode));
                offset+=sizeof(_MultiMode);

                int _wantsize=_MultiMode.VideoPicSize+_MultiMode.RadarPicSize+sizeof(_MultiMode);
                if(m_buf.size()!=_wantsize)
                {
                    qDebug()<<m_buf.size()<<_wantsize<<m_buffer[_communication.DataType].size()<<_communication.DataType;

                    emit signal_SeaStatus(Data_UI_TYPE::radar, "multi mode save is not succeed!get  "+QString::number(m_buf.size())+",but want "+QString::number(_wantsize));
                    Cfg::get()->SetLog("multi mode save is not succeed,pic size is different!get "+to_string(m_buf.size())+",but want "+to_string(_wantsize));

                }else
                {
                    QString time=QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
                    QString filepath=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/video_"+time+".jpg";
                    QByteArray imgarray=m_buf.mid(offset,_MultiMode.VideoPicSize);
                    //图片压缩
                    QBuffer buffer(&imgarray);
                    buffer.open(QIODevice::ReadOnly);
                    QImageReader reader(&buffer,"PNG");
                    QImage img = reader.read();
                    img.save(filepath);
                    offset+=_MultiMode.VideoPicSize;
                    emit signal_SeaStatus(Data_UI_TYPE::radar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+" save "+filepath);
                    Cfg::get()->SetLog(filepath.toStdString()+" save "+time.toStdString());

                    filepath=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/radar_"+time+".jpg";
                    imgarray=m_buf.mid(offset,_MultiMode.VideoPicSize);
                    //图片压缩
                    QBuffer buffer1(&imgarray);
                    buffer1.open(QIODevice::ReadOnly);
                    QImageReader reader1(&buffer1,"PNG");
                    QImage img1 = reader1.read();
                    img1.save(filepath);
                    emit signal_SeaStatus(Data_UI_TYPE::radar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+" save "+filepath);
                    Cfg::get()->SetLog(filepath.toStdString()+" save "+time.toStdString());

                    /*************************写入文件************************/
                    QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/RadarMulti.txt";
                    QFile output(filename);
                    if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                    {
                        Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                        return ;
                    }
                    QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                            QString(": track id = %1,latlon =%2,%3,radarclass=%4,videoclass=%5")
                            .arg(_MultiMode.ID).arg(_MultiMode.Latitude)
                            .arg(_MultiMode.Lontitude).arg(_MultiMode.Radar_Class).arg(_MultiMode.Video_Class);
                    output.write(content.toLocal8Bit()+"\n");
                    output.close();
                }
            }
                break;
            default:
                emit signal_SeaStatus(Data_UI_TYPE::radar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+":Radar-->DataType is Error,get DataType is "+QString::number(_communication.DataType));
                break;
            }
            break;
        case ELEC_SYSTEM:
            switch (_communication.DataType)
            {
            //AIS解译态势信息（0x0231）
            case ELEC_AIS:
            {
                AISPostureData _AIS;
                //                qDebug()<<"sizeof (AISPostureData)="<<sizeof (AISPostureData);
                //                qDebug()<<"_communication.PayloadSize="<<m_buf.size()<<_communication.PayloadSize;
                //                QString disp_string,S;
                //                for(int i=0;i<m_buf.size();i++)
                //                {
                //                    S.sprintf("0x%02x, ", (unsigned char)m_buf.at(i));
                //                    disp_string += S;
                //                }
                //                qDebug()<<"m_buf = "<<disp_string;
                memcpy(&_AIS,m_buf.begin(),sizeof(_AIS));
                //                qDebug()<<"AIS="<<_AIS.m_sigType<<_AIS.m_time<<_AIS.m_msgID<< QString(QLatin1String((char*)_AIS.m_target_id))<<_AIS.m_static_info.m_imo_id<<_AIS.m_dynamic_info.m_target_longitude<<_AIS.m_dynamic_info.m_target_latitude;
                emit signal_SeaStatus(Data_UI_TYPE::Elec,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到AIS信息"));
                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/AIS.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                string targetid;
                targetid.assign(_AIS.m_target_id,_AIS.m_target_id+9);
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2,%3,%4,%5,%6,%7,%8,%9,%10").arg(_AIS.m_dynamic_info.m_target_longitude).arg(_AIS.m_dynamic_info.m_target_latitude)
                        .arg(_AIS.m_dynamic_info.m_ship_real_course).arg(_AIS.m_dynamic_info.m_cog).arg(_AIS.m_dynamic_info.m_ship_speed)
                        .arg(_AIS.m_dynamic_info.m_locate_precision).arg(int(_AIS.m_dynamic_info.m_ship_status))
                        .arg(QString::fromStdString(targetid)).arg(_AIS.m_msgID).arg(_AIS.m_time);
                output.write(content.toLocal8Bit()+"\n");
                output.close();
            }
                break;
                //ADS-B解译态势信息（0x0232）
            case ELEC_ADS_B:
            {
                ADSBPostureData m_ADSBPostureData;
                memcpy(&m_ADSBPostureData,m_buf.begin(),sizeof(m_ADSBPostureData));
                qDebug()<<"ADSB="<<m_ADSBPostureData.m_azimuth<<m_ADSBPostureData.m_longitude<<m_ADSBPostureData.m_latitude<<m_ADSBPostureData.m_sigType;
                emit signal_SeaStatus(Data_UI_TYPE::Elec,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到ADS-B信息"));
                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/ADSB.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2").arg(m_ADSBPostureData.m_longitude).arg(m_ADSBPostureData.m_latitude);
                output.write(content.toLocal8Bit()+"\n");
                output.close();
            }
                break;
                //ACARS解译态势信息（0x0233）
            case ELEC_ACARS:
            {
                ACARSPostureData _ACARS;
                memcpy(&_ACARS,m_buf.begin(),sizeof(_ACARS));
                qDebug()<<"_ACARS="<<_ACARS.m_target_latitude<<_ACARS.m_target_longitude;
                emit signal_SeaStatus(Data_UI_TYPE::Elec,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到ACARS信息"));
                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/ACARS.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2").arg(_ACARS.m_target_latitude).arg(_ACARS.m_target_longitude);
                output.write(content.toLocal8Bit()+"\n");
                output.close();
            }
                break;
                //重点雷达信号（0x0211）
                //海事头+(北斗头+数据)+bbc
            case ELEC_Radar:
            {
                BDCommunication _BDCommunication;
                memcpy(&_BDCommunication,m_buf.begin(),sizeof(_BDCommunication));

                QByteArray _Radarbuffer=m_buf.mid(sizeof(_BDCommunication),m_buf.size()-sizeof(_BDCommunication));
                //                qDebug()<<"_Radarbuffer.size()="<<_Radarbuffer.size()<<m_buf.size()<<sizeof(_BDCommunication)<<m_buf.size()-sizeof(_BDCommunication);
                KeyRadarData _KeyRadarData;
                memcpy(&_KeyRadarData,_Radarbuffer.begin(),sizeof(_KeyRadarData));
                //                qDebug()<<"_ACARS="<<_ACARS.m_target_latitude<<_ACARS.m_target_longitude;
                emit signal_SeaStatus(Data_UI_TYPE::Elec,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到重点雷达信号"));


                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/KeyRadarData.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2,%3,%4,%5,%6").arg(int(_KeyRadarData.m_signal_type)).arg(_KeyRadarData.m_lat).arg(_KeyRadarData.m_lon)
                        .arg(_KeyRadarData.m_freq).arg(_KeyRadarData.m_width).arg(_KeyRadarData.m_first_capture_time);
                output.write(content.toLocal8Bit()+"\n");

                if(m_Collaboratestatus.TogetherCondition!=2)
                    emit signal_Load(1,_Radarbuffer,ELEC_SYSTEM,ELEC_Radar,NetWorkType::BeiDou);
                else
                {
                    content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                            QString::fromLocal8Bit(" 接收到重点雷达信号，但是处于协同状态，不允许发送。");
                    output.write(content.toLocal8Bit()+"\n");
                    emit signal_SeaStatus(Data_UI_TYPE::Elec,content);
                }
                output.close();


                return;
            }
                break;
                //IFF信号重点雷达信号（0x0212）
            case ELEC_IFF:
            {
                BDCommunication _BDCommunication;
                memcpy(&_BDCommunication,m_buf.begin(),sizeof(_BDCommunication));

                QByteArray Iffbuffer=m_buf.mid(sizeof(_BDCommunication),m_buf.size()-sizeof(_BDCommunication));
                KeyIffData _KeyIffData;
                memcpy(&_KeyIffData,Iffbuffer,sizeof(_KeyIffData));
                //                qDebug()<<"_ACARS="<<_ACARS.m_target_latitude<<_ACARS.m_target_longitude;
                emit signal_SeaStatus(Data_UI_TYPE::Elec,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到IFF信号"));
                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/KeyIffData.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2,%3,%4,%5,%6").arg(int(_KeyIffData.m_signal_type)).arg(_KeyIffData.m_lat).arg(_KeyIffData.m_lon)
                        .arg(_KeyIffData.m_src_no).arg(int(_KeyIffData.m_mode_id)).arg(_KeyIffData.m_first_capture_time);
                output.write(content.toLocal8Bit()+"\n");
                if(m_Collaboratestatus.TogetherCondition!=2)
                    emit signal_Load(1,Iffbuffer,ELEC_SYSTEM,ELEC_IFF,NetWorkType::BeiDou);
                else
                {
                    content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                            QString::fromLocal8Bit(" 接收到IFF信号，但是处于协同状态，不允许发送。");
                    output.write(content.toLocal8Bit()+"\n");
                    emit signal_SeaStatus(Data_UI_TYPE::Elec,content);
                }
                output.close();
                return;
            }
                break;
                //反潜浮标信号（0x0213）
            case ELEC_FQFB:
            {
                BDCommunication _BDCommunication;
                memcpy(&_BDCommunication,m_buf.begin(),sizeof(_BDCommunication));

                QByteArray Iffbuffer=m_buf.mid(sizeof(_BDCommunication),m_buf.size()-sizeof(_BDCommunication));
                FanQianData _FanQianData;
                memcpy(&_FanQianData,Iffbuffer.begin(),sizeof(_FanQianData));
                //                qDebug()<<"_ACARS="<<_ACARS.m_target_latitude<<_ACARS.m_target_longitude;
                emit signal_SeaStatus(Data_UI_TYPE::Elec,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到反潜浮标信号"));
                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/FanQianData.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2,%3,%4,%5,%6").arg(_FanQianData.position).arg(_FanQianData.latitude).arg(_FanQianData.longtitude)
                        .arg(_FanQianData.time).arg(_FanQianData.sourceNum).arg(int(_FanQianData.signalType));
                output.write(content.toLocal8Bit()+"\n");
                if(m_Collaboratestatus.TogetherCondition!=2)
                    emit signal_Load(1,Iffbuffer,ELEC_SYSTEM,ELEC_FQFB,NetWorkType::BeiDou);
                else
                {
                    content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                            QString::fromLocal8Bit(" 接收到反潜浮标信号，但是处于协同状态，不允许发送。");
                    output.write(content.toLocal8Bit()+"\n");
                    emit signal_SeaStatus(Data_UI_TYPE::Elec,content);
                }
                output.close();
                return;
            }
                break;
                //数据链信号（0x0214）
            case ELEC_SJL:
            {
                BDCommunication _BDCommunication;
                memcpy(&_BDCommunication,m_buf.begin(),sizeof(_BDCommunication));

                QByteArray Iffbuffer=m_buf.mid(sizeof(_BDCommunication),m_buf.size()-sizeof(_BDCommunication));
                KeyDataLinkData _KeyDataLinkData;
                memcpy(&_KeyDataLinkData,Iffbuffer.begin(),sizeof(_KeyDataLinkData));
                //                qDebug()<<"_ACARS="<<_ACARS.m_target_latitude<<_ACARS.m_target_longitude;
                emit signal_SeaStatus(Data_UI_TYPE::Elec,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到数据链信号"));
                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/KeyDataLinkData.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2,%3,%4,%5,%6").arg(_KeyDataLinkData.m_radiant_ID).arg(_KeyDataLinkData.latitude).arg(_KeyDataLinkData.longtitude)
                        .arg(_KeyDataLinkData.m_freq).arg(_KeyDataLinkData.m_azimuth).arg(_KeyDataLinkData.m_self_address);
                output.write(content.toLocal8Bit()+"\n");
                if(m_Collaboratestatus.TogetherCondition!=2)
                    emit signal_Load(1,Iffbuffer,ELEC_SYSTEM,ELEC_SJL,NetWorkType::BeiDou);
                else
                {
                    content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                            QString::fromLocal8Bit(" 接收到数据链信号，但是处于协同状态，不允许发送。");
                    output.write(content.toLocal8Bit()+"\n");
                    emit signal_SeaStatus(Data_UI_TYPE::Elec,content);
                }
                output.close();
                return;
            }
                break;
                //无线电系统运维信息（0x0511）
            case ELEC_Status:
            {
                HealthData _HealthData;
                memcpy(&_HealthData,m_buf.begin(),sizeof(_HealthData));
                emit signal_SeaStatus(Data_UI_TYPE::Elec,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到监控信息"));

                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/Elec.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2,%3,%4,%5,%6").arg(_HealthData.m_sub_health).arg(_HealthData.m_usw_detect_health)
                        .arg(_HealthData.m_dianzi_detect_health).arg(_HealthData.m_satellite_health)
                        .arg(_HealthData.m_sub_parameter.No1copyright).arg(_HealthData.m_sub_parameter.No2copyright);
                output.write(content.toLocal8Bit()+"\n");
                output.close();
            }
                break;
                //无线电关键目标（ElecVIPTarget，0x0411）
            case ELEC_Merge:
            {
                emit signal_SeaStatus(Data_UI_TYPE::Elec,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到融合信息"));

                BDCommunication _BDCommunication;
                memcpy(&_BDCommunication,m_buf.begin(),sizeof(_BDCommunication));

                QByteArray Iffbuffer=m_buf.mid(sizeof(_BDCommunication),m_buf.size()-sizeof(_BDCommunication));

                m_bRecElec=true;
                m_TimeElec=QDateTime::currentDateTime();
                ElecMerge temp;
                memcpy(&temp,Iffbuffer.begin(),sizeof(ElecMerge));
                //                if(temp.valid==1)
                m_mergeMes.elec = temp;
                //                else
                //                    m_mergeMes.elec = ElecMerge();

                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/ElecVIP.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2,%3,%4,%5,%6,valid is %7").arg(temp.Elec_ID).arg(temp.Elec_Lat)
                        .arg(temp.Elec_Lon).arg(temp.Elec_azi).arg(temp.Elec_fps).arg(temp.Elec_Type).arg(int(temp.valid));
                output.write(content.toLocal8Bit()+"\n");
                output.close();

                return;
                qDebug()<<"****ELEC_Merge:"<<QString::fromLatin1(m_buf)<<m_buf.size()<<sizeof(ElecMerge)<<QString::number(temp.Elec_Lon)<<temp.Elec_Lat;
            }
                break;
                //无线电触发协同（0x0642）
            case MERGE_ColManual:
            {

            }
                break;
            default:
                QString out=QString::number(QDateTime::currentDateTime().toTime_t());
                out.append(":get DataType Error, is "+QString::number(_communication.DataType)+"\n");
                m_eleclog.write(out.toLocal8Bit());
                m_eleclog.flush();
                emit signal_SeaStatus(Data_UI_TYPE::Elec,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+":Elec-->DataType is Error,get DataType is "+QString::number(_communication.DataType));
                break;
            }
            break;
        case SONAR_SYSTEM:
            switch(_communication.DataType){
            //侦听声纳目标信息（SoundSonarTarget ，0x0221）
            case SONAR_SoundSonarTarget:
            {
                MyStructSonar _SoundSonarTarget;
                memcpy(&_SoundSonarTarget,m_buf.begin(),sizeof(_SoundSonarTarget));
                emit signal_SeaStatus(Data_UI_TYPE::Sonar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 侦听声纳目标ID= ")+QString::number(_SoundSonarTarget.nID));

                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/Sonar.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2,%3,%4,%5,%6").arg(_SoundSonarTarget.nID).arg(_SoundSonarTarget.nNumTarget)
                        .arg(_SoundSonarTarget.nTargetType).arg(_SoundSonarTarget.dAzimuthDegrees).arg(_SoundSonarTarget.dCurrentDistance).arg(_SoundSonarTarget.nComSignalsType);
                output.write(content.toLocal8Bit()+"\n");
                output.close();
            }
                break;
                //成像声纳识别 0x0222
            case SONAR_ImageSonarRecognition:
            {
                StSonarImageRecognitionInfo _ImageSonarRecognition;
                memcpy(&_ImageSonarRecognition,m_buf.begin(),sizeof(_ImageSonarRecognition));
                emit signal_SeaStatus(Data_UI_TYPE::Sonar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 成像声呐目标ID= ")+QString::number(_ImageSonarRecognition.nID));

                qDebug()<<"_SonarPicture.TimeID:"<<_ImageSonarRecognition.TimeID;
                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/Sonar.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2,%3,%4,%5,%6").arg(_ImageSonarRecognition.nID).arg(_ImageSonarRecognition.dLat)
                        .arg(_ImageSonarRecognition.dLon).arg(_ImageSonarRecognition.classID).arg(_ImageSonarRecognition.confidence).arg(_ImageSonarRecognition.TimeID);
                output.write(content.toLocal8Bit()+"\n");
                output.close();
            }
                break;
                //周边海域水声频段背景
            case SONAR_FrequencyBackground:
            {
                FrequencyBackground _FrequencyBackground;
                memcpy(&_FrequencyBackground,m_buf.begin(),sizeof(_FrequencyBackground));
                emit signal_SeaStatus(Data_UI_TYPE::Sonar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 周边海域水声频段背景= ")+QString::number(_FrequencyBackground.ID));

                qDebug()<<"_SonarPicture.TimeID:"<<_FrequencyBackground.ID;
                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/Sonar.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2,%3").arg(_FrequencyBackground.ID).arg(_FrequencyBackground.Lat)
                        .arg(_FrequencyBackground.Lon);
                output.write(content.toLocal8Bit()+"\n");
                output.close();
            }
                break;
                //信号侦听声纳：
                //方位历程图（0x0321）、Lofar谱图（0x0322）、Demon谱图（0x0323）
                //成像声纳：
                //成像声纳目标图（0x0324）、海底地形图（0x0325）
            case  SONAR_AzimuthMap:
            case  SONAR_LofarSpectrogram:
            case  SONAR_DemonSpectrogram:
            case  SONAR_SonarPicture:
            case  SONAR_TopographicMap:
            {
                int offset=0;
                StSonarPictureHead _SonarPicture;
                memcpy(&_SonarPicture,m_buf.begin()+offset,sizeof(_SonarPicture));
                offset+=sizeof(_SonarPicture);

                int imgsize = _SonarPicture.picSize;

                QByteArray imgarray=m_buf.mid(offset,m_buf.size()-offset);
                qDebug()<<m_buf.size()<<imgarray.size()<<imgsize<<m_buffer[_communication.DataType].size()<<_communication.DataType;


                //                ui->plainSonar->appendPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+" imgarray.size()= "+QString::number(imgarray.size())+",imgsize= "+QString::number(imgsize));
                QString time=QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
                QString filepath=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/"+time+".jpg";
                if(imgarray.size() == imgsize)
                {
                    //图片压缩
                    QBuffer buffer(&imgarray);
                    buffer.open(QIODevice::ReadOnly);
                    QImageReader reader(&buffer,"JPG");
                    QImage img = reader.read();
                    img.save(filepath);
                    emit signal_SeaStatus(Data_UI_TYPE::Sonar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+" save "+filepath+",imgsize="+QString::number(imgsize)+",packet is "+QString::number(_communication.ChunkNum));
                    Cfg::get()->SetLog(filepath.toStdString()+" save "+time.toStdString());
                }else
                {
                    emit signal_SeaStatus(Data_UI_TYPE::Sonar,filepath+ " save is not succeed!get  "+QString::number(imgarray.size())+",but want "+QString::number(imgsize));
                    Cfg::get()->SetLog(filepath.toStdString()+" save is not succeed,pic size is different!get "+to_string(imgarray.size())+",but want "+to_string(imgsize));
                }
                break;
            }
                //系统监控信息（SonarStatus，0x0521）
            case  SONAR_SonarStatus:
            {
                StSonarSystemInfo _SonarPicture;
                memcpy(&_SonarPicture,m_buf.begin(),sizeof(_SonarPicture));
                emit signal_SeaStatus(Data_UI_TYPE::Sonar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到系统监控信息"));

                //                qDebug()<<"_SonarPicture.TimeID:"<<_SonarPicture.TimeID;
                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/Sonar.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11").arg(_SonarPicture.DemonStatus).arg(_SonarPicture.LofarStatus)
                        .arg(_SonarPicture.SystemStatus).arg(_SonarPicture.ResultsUpload).arg(_SonarPicture.AzimuthMapStatus)
                        .arg(_SonarPicture.ImageSonarStatus).arg(_SonarPicture.SoundSonarStatus).arg(_SonarPicture.ImageSensorStatus)
                        .arg(_SonarPicture.ImageSoftwareStatus1).arg(_SonarPicture.AzimuthSoftwareStatus).arg(_SonarPicture.Size);
                output.write(content.toLocal8Bit()+"\n");
                output.close();
            }
                break;
                //声纳关键目标（SonarVIPTarget，0x0421）
            case SONAR_SonarVIPTarget:
            {
                qDebug()<<QString::fromLocal8Bit("********************接收声纳协同结果****************");
                emit signal_SeaStatus(Data_UI_TYPE::Sonar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到融合信息"));
                m_buffer[_communication.DataType].clear();

                m_bRecSonar=true;
                m_TimeSonar=QDateTime::currentDateTime();

                SonarMerge temp;
                memcpy(&temp,m_buf.begin(),sizeof(SonarMerge));
                //                if(temp.valid==1)
                m_mergeMes.Sonar = temp;
                //                else
                //                    m_mergeMes.Sonar = SonarMerge();

                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/SonarVIP.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2,%3,%4,%5,%6,data valid is %7").arg(temp.Sonar_ID).arg(temp.Sonar_Azi)
                        .arg(temp.Sonar_Type).arg(temp.Sonar_ImageID).arg(temp.Sonar_ImageLat).arg(temp.Sonar_ImageLon).arg(int(temp.valid));
                output.write(content.toLocal8Bit()+"\n");
                output.close();
                return;
            }
                break;
                //声纳触发协同（0x0642）
            case MERGE_ColManual:
            {
                qDebug()<<QString::fromLocal8Bit("********************接收声纳协同请求****************");
                memcpy(&m_collateComm,m_buf.begin(),sizeof(collateComm));
                m_buffer[_communication.DataType].clear();
                m_collateComm.From=3;
                m_collateComm.Command=0;

                emit signal_SeaStatus(Data_UI_TYPE::Sonar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 触发协同"));

                if(m_Collaboratestatus.TogetherCondition==1)
                {
                    //立即发送协同状态
                    m_Collaboratestatus.TogetherCondition=2;
                    m_Collaboratestatus.SonarResponse=1;
                    emit signal_CollStatusChange(m_Collaboratestatus);

                    QString out=QString::number(QDateTime::currentDateTime().toTime_t());
                    out.append(QString(":change collaborate status to No, Response from Sonar."));
                    m_otherlog.write(out.toLocal8Bit()+"\n");
                    m_otherlog.flush();

                    //                    //发送协同指令
                    //                    emit signal_Load(0,m_buf,MERGE_SYSTEM,MERGE_ColManual,NetWorkType::CollaborateAndClear);
                    //                    out=QString::number(QDateTime::currentDateTime().toTime_t());
                    //                    out.append(QString(":Brocast Collaborate Command,VIP target is: %1, %2, %3,from %4").arg(m_collateComm.TargetLon).arg(m_collateComm.TargetLat).arg(m_collateComm.TargetAzi).arg(QString(m_collateComm.From)));
                    //                    m_otherlog.write(out.toLocal8Bit()+"\n");
                    //                    m_otherlog.flush();

                    //初始化开始协同时间，各子系统反馈协同结果
                    m_bRecRadar=false;
                    m_bRecSonar=false;
                    m_bRecElec=false;
                    m_TimeRadar=QDateTime::currentDateTime();
                    m_TimeSonar=QDateTime::currentDateTime();
                    m_TimeElec=QDateTime::currentDateTime();

                    //70秒间隔不停协同
                    m_CollateTimer->start();
                    m_beginTime=QDateTime::currentDateTime();
                    slot_CollateUpdate();
                }else {
                    QString out=QString::number(QDateTime::currentDateTime().toTime_t());
                    out.append(QString(":receive Sonar CollateCommand,but Status is not allow!"));
                    m_otherlog.write(out.toLocal8Bit()+"\n");
                    m_otherlog.flush();
                }

                return;
            }
                break;
            default:
                emit signal_SeaStatus(Data_UI_TYPE::Sonar,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+":Sonar-->DataType is Error,get DataType is "+QString::number(_communication.DataType));
                break;
            }
            break;
        case MERGE_SYSTEM:
            switch(_communication.DataType){
            //态势数据（0x0101）
            case  MERGE_Status:
            {
                memcpy(&m_sail,m_buf.begin(),sizeof(SaiLMessage));
                //                qDebug()<<_communication.SystemID<<_communication.DataType<<sail.HDT_<<sail.Lat_<<sail.Lon_<<sail.nTime_<<_communication.PayloadSize<<value.size()<<sizeof (SeaCommunication)<<sizeof (SaiLMessage);
                /**********************转发**************************/
                static QDateTime _lasttime=QDateTime::currentDateTime();
                if(_lasttime.secsTo(QDateTime::currentDateTime())>MergeLoadtime)
                {
                    /*************************写入文件************************/
                    QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/merge.txt";
                    QFile output(filename);
                    if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                    {
                        Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                        return ;
                    }
                    QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                            QString(",%1,%2,%3,%4,%5,%6,%7").arg(m_sail.nTime_).arg(m_sail.Lat_).arg(m_sail.Lon_)
                            .arg(m_sail.HDT_).arg(m_sail.WindSpeed_).arg(m_sail.WindSpeed_).arg(m_sail.nBoat_ID_);
                    output.write(content.toLocal8Bit()+"\n");
                    output.close();


                    emit signal_SeaStatus(Data_UI_TYPE::Merge,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+":Merge-->"+
                                          QString::number(m_sail.nTime_)+","+QString::number(m_sail.HDT_)+","+QString::number(m_sail.Lat_)+","+QString::number(m_sail.Lon_));
                    _lasttime=QDateTime::currentDateTime();
                    break;
                }else
                {
                    m_buffer[_communication.DataType].clear();
                    return;
                }
            }
                break;
                //融合系统运维信息（MergeSystemStatus，0x0531）
            case  MERGE_SystemStatus:
            {
                MergeSystemInfo _MergeSysStatus;
                memcpy(&_MergeSysStatus,m_buf.begin(),sizeof(_MergeSysStatus));
                emit signal_SeaStatus(Data_UI_TYPE::Merge,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到系统监控信息"));

                //                qDebug()<<"_SonarPicture.TimeID:"<<_SonarPicture.TimeID;
                /*************************写入文件************************/
                QString filename=g_path+"/"+QString::number(_communication.SystemID)+"-"+QString::number(_communication.DataType)+"/MergeStatus.txt";
                QFile output(filename);
                if(!output.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))  // 以写的方式打开文件
                {
                    Cfg::get()->SetLog("open "+filename.toStdString()+" fail!");
                    return ;
                }
                QString content=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                        QString(",%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,%18,%19,%20,%21,%22,%23,%24,%25").arg(_MergeSysStatus._mergeSys.Sea_lost).arg(_MergeSysStatus._mergeSys.Sea_delay).arg(_MergeSysStatus._mergeSys.wifi_lost)
                        .arg(_MergeSysStatus._mergeSys.wifi_delay).arg(_MergeSysStatus._mergeSys.Net_global).arg(_MergeSysStatus._mergeSys.Com_global).arg(_MergeSysStatus._mergeSys.radarsize).arg(_MergeSysStatus._mergeSys.elecsize)
                        .arg(_MergeSysStatus._mergeSys.mergesize).arg(_MergeSysStatus._mergeSys.sonarsize).arg(int(_MergeSysStatus._mergeSys.zjxx.power[0])).arg(int(_MergeSysStatus._mergeSys.zjxx.power[1]))
                        .arg(int(_MergeSysStatus._mergeSys.zjxx.power[2])).arg(int(_MergeSysStatus._mergeSys.zjxx.power[3])).arg(int(_MergeSysStatus._mergeSys.zjxx.power[4])).arg(int(_MergeSysStatus._mergeSys.zjxx.power[5]))
                        .arg((_MergeSysStatus._mergeSys.zjxx.elect).to_ulong()).arg((_MergeSysStatus._mergeSys.zjxx.state).to_ulong()) .arg((_MergeSysStatus._mergeSys.zjxx.income).to_ulong()).arg((_MergeSysStatus._mergeSys.zjxx.icstate).to_ulong())
                        .arg(QString::number(_MergeSysStatus._boat.AIS_recv_interval)).arg(_MergeSysStatus._boat.GPS_recv_interval).arg(_MergeSysStatus._boat.HDT_recv_interval).arg(_MergeSysStatus._boat.MWV_recv_interval).arg(_MergeSysStatus._boat.SAIL_recv_interval);
                output.write(content.toLocal8Bit()+"\n");
                output.close();
            }
                break;
                //任务规划回复（0x0632）
            case MERGE_TargetRep:
            {
                emit signal_Load(1,m_buf,MERGE_SYSTEM,MERGE_TargetRep,NetWorkType::BeiDou);
                m_buffer[_communication.DataType].clear();
                emit signal_SeaStatus(Data_UI_TYPE::Merge,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromLocal8Bit(" 接收到任务规划回复"));

                QString out=QString::number(QDateTime::currentDateTime().toTime_t());
                out.append(QString(":Task Response is %1.").arg(QString::fromLocal8Bit(m_buf)));
                m_otherlog.write(out.toLocal8Bit()+"\n");
                m_otherlog.flush();

                return;
            }
                break;
            default:
                emit signal_SeaStatus(Data_UI_TYPE::Merge,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+":Merge-->DataType is Error,get DataType is "+QString::number(_communication.DataType));
                break;
            }
            break;
        default:
            emit signal_SeaStatus(Data_UI_TYPE::Com,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+":SystemID is Error,get DataType is "+QString::number(_communication.SystemID));
            break;
        }
    }

    emit signal_Load(3,value,0,0,NetWorkType::GlobalNetWork);
}


void ProcessSeaData::slot_TcpreceiveIn(QByteArray value,QString ip,int port)
{

    //////////////////剔除无线电初始包
    int _Header;
    memcpy(&_Header,value,4);
    if(_Header==0x55555555)
    {
        return;
    }

    static QString _lastdate=QDateTime::currentDateTime().toString("yyyyMMdd");
    if(_lastdate!=QDateTime::currentDateTime().toString("yyyyMMdd"))
    {
        closefilerecord();
        startfilerecord();
    }
    _lastdate=QDateTime::currentDateTime().toString("yyyyMMdd");

    /**********************接收(确保是一个包一个包处理)**************************/
    m_logmutex.lock();
    QString mapkey=ip+":"+QString::number(port);
    map<QString,recData>::iterator ptr;
    if(m_MesData.find(mapkey)!=m_MesData.end())
    {
        ptr=m_MesData.find(mapkey);
    }else
    {
        m_MesData[mapkey]=recData();
        ptr=m_MesData.find(mapkey);
    }

    ptr->second.time=QDateTime::currentDateTime();
    ptr->second.data.append(value);//分包

    while (ptr->second.data.size()>=sizeof(SeaCommunication))
    {
        SeaCommunication _communication;
        QByteArray m_buf;
        memcpy(&_communication,ptr->second.data,sizeof(SeaCommunication));
        //        qDebug()<<"_communication.SystemID====="<<_communication.PacketHead<<_communication.PayloadSize
        //               <<_communication.ChunkID<<_communication.ChunkNum<<_communication.SystemID<<_communication.DataType<<ptr->second.data.size()<<value.size();

        if(_communication.PayloadSize > ptr->second.data.size())
            break;//分包，等待后续数据
        else if(_communication.PayloadSize<=0 || _communication.PayloadSize > 1429)
        {
            ptr->second.data.clear();
        }
        //需要提供带头的value（包含bbc），并且剔除已经拿走的数据
        m_buf.append(ptr->second.data.mid(0,_communication.PayloadSize));
        ptr->second.data.remove(0,_communication.PayloadSize);//粘包

        /**********************日志**************************/
        QString out=QString::number(QDateTime::currentDateTime().toTime_t());
        out.append(QString(":%1,receive %2,All chunk is %3,current chunk is %4,size is %5.").arg(_communication.DataTime).arg(_communication.DataType).arg(_communication.ChunkNum).arg(_communication.ChunkID).arg(_communication.PayloadSize));
        switch (_communication.SystemID) {
        case RADAR_SYSTEM:
            m_radarlog.write(out.toLocal8Bit()+"\n");
            m_radarlog.flush();
            setFile(m_buf,_communication);
            //            ui->plainRadar->appendPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+":Radar-->"+out+";");
            break;
        case ELEC_SYSTEM:
            m_eleclog.write(out.toLocal8Bit()+"\n");
            m_eleclog.flush();
            setFile(m_buf,_communication);
            //            ui->plainElec->appendPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+":Elec-->"+out+";");
            break;
        case SONAR_SYSTEM:
            m_sonarlog.write(out.toLocal8Bit()+"\n");
            m_sonarlog.flush();
            setFile(m_buf,_communication);
            //            ui->plainSonar->appendPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+":Sonar-->"+out+";");
            break;
        case MERGE_SYSTEM:
            m_mergelog.write(out.toLocal8Bit()+"\n");
            m_mergelog.flush();
            setFile(m_buf,_communication);
            //            ui->plainMerge->appendPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+":Merge-->"+out+";");
            break;
        default:
            emit signal_SeaStatus(Data_UI_TYPE::Com,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+":SystemID is Error,get DataType is "+QString::number(_communication.SystemID));
            break;
        }
    }
    m_logmutex.unlock();
}

void ProcessSeaData::slot_ServerStatus(QList<NetMes> t)
{
    //    qDebug()<<"---------------------------";
    for(int i = 0; i < t.count(); i++)
    {
        NetMes _net= t.at(i);
        //        qDebug()<<"++++++++++++++++++=="<<_net.m_clientIP+":"+QString::number(_net.m_clientPort);
    }
    emit signal_ServerStatus(t);
}

void ProcessSeaData::slotTimer()
{
    m_logmutex.lock();
    /**********************剔除超时缓存数据**************************/
    //    if(!m_MesData.empty())
    for(auto pt=m_MesData.begin();pt!=m_MesData.end();)
    {
        qDebug()<<"m_MesData:"<<pt->first<<""<<pt->second.data.size();
        QString output="m_MesData:"+pt->first+","+QString::number(pt->second.data.size());
        //            if(QDateTime::currentDateTime().toTime_t()%30==0)
        {
            Cfg::get()->SetLog(output.toStdString());
        }
        if(abs(pt->second.time.secsTo(QDateTime::currentDateTime()))>10)
        {
            //            pt->second.data.clear();
            pt=m_MesData.erase(pt);
            continue;
        }else if(pt->second.data.size()>100000)
        {
            pt=m_MesData.erase(pt);
            continue;
        }
        pt++;
    }
    m_logmutex.unlock();
    for(auto pt=m_buffer.begin();pt!=m_buffer.end();pt++)
    {
        qDebug()<<"m_buffer:"<<pt->first<<""<<pt->second.size();

        QString output="m_buffer:"+QString::number(pt->first)+","+QString::number(pt->second.size());
        //        if(QDateTime::currentDateTime().toTime_t()%30==0)
        {
            Cfg::get()->SetLog(output.toStdString());
        }
    }
}

void ProcessSeaData::slot_sendMerge()
{

    qDebug()<<"***********"<<m_TimeRadar.secsTo(QDateTime::currentDateTime());
    //    emit signal_SeaStatus(Data_UI_TYPE::Merge,QString("%1,%2,%3,%4,%5,%6").arg(int(m_bRecRadar)).arg(int(m_bRecSonar)).arg(int(m_bRecElec))
    //                          .arg(int(m_TimeRadar.secsTo(QDateTime::currentDateTime()))).arg(int(m_TimeSonar.secsTo(QDateTime::currentDateTime())))
    //                          .arg(int(m_TimeElec.secsTo(QDateTime::currentDateTime()))));

    if((m_bRecRadar && m_bRecSonar && m_bRecElec) ||
            (!m_bRecRadar && m_TimeRadar.secsTo(QDateTime::currentDateTime())>60) ||
            (!m_bRecSonar && m_TimeSonar.secsTo(QDateTime::currentDateTime())>60) ||
            (!m_bRecElec && m_TimeElec.secsTo(QDateTime::currentDateTime())>60)
            )
    {
        m_mergeMes.Time=m_CollateTime;
        //        m_mergeMes.Lat=m_sail.Lat_;
        //        m_mergeMes.Lon=m_sail.Lon_;
        QByteArray array_data;
        array_data.append((char*)&m_mergeMes, sizeof(m_mergeMes));

        //发送融合消息
        emit signal_Load(1,array_data,MERGE_SYSTEM,MERGE_VIPTarget,NetWorkType::BeiDou);

        QString out=QString::number(QDateTime::currentDateTime().toTime_t());
        out.append(QString(":send Merge data to land,current data have radar: %1;Sonar: %2;Elec: %3."
                           "Radar Data is:Radar_Lat=%4,Radar_Lon=%5,Radar_Type=%6,Radar_TrackID=%7,data valid is %8;"
                           "Sonar Data is %9,%10,%11,%12,%13,%14,data valid is %15;"
                           "Elec Data is %16,%17,%18,%19,%20,%21,valid is %22,get time is %23")
                   .arg(m_bRecRadar).arg(m_bRecSonar).arg(m_bRecElec).arg(m_mergeMes.radar.Radar_Lat).arg(m_mergeMes.radar.Radar_Lon)
                   .arg(m_mergeMes.radar.Radar_Type).arg(m_mergeMes.radar.Radar_TrackID).arg(int(m_mergeMes.radar.valid))
                   .arg(m_mergeMes.Sonar.Sonar_ID).arg(m_mergeMes.Sonar.Sonar_Azi).arg(m_mergeMes.Sonar.Sonar_Type).arg(m_mergeMes.Sonar.Sonar_ImageID).arg(m_mergeMes.Sonar.Sonar_ImageLat).arg(m_mergeMes.Sonar.Sonar_ImageLon).arg(int(m_mergeMes.Sonar.valid))
                   .arg(m_mergeMes.elec.Elec_ID).arg(m_mergeMes.elec.Elec_Lat).arg(m_mergeMes.elec.Elec_Lon).arg(m_mergeMes.elec.Elec_azi).arg(m_mergeMes.elec.Elec_fps).arg(m_mergeMes.elec.Elec_Type).arg(int(m_mergeMes.elec.valid))
                   .arg(QString::number(m_mergeMes.Time)));
        m_otherlog.write(out.toLocal8Bit()+"\n");
        m_otherlog.flush();



        emit signal_SeaStatus(Data_UI_TYPE::Merge,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+
                              QString::fromLocal8Bit(":上报融合数据，数据来源为雷达：%1，声纳%2，无线电%3").arg(m_bRecRadar).arg(m_bRecSonar).arg(m_bRecElec));
        qDebug()<<QString::fromLocal8Bit("********************发送协同数据****************");


        /**********************中间件入库****************************/
        QString str= "MergeMes:insert into MergeMes" + QString("(Time,Radar_Lon,Radar_Lat,Radar_Type,Radar_TrackID,Radar_valid,Elec_Type,Elec_ID,Elec_Lon,Elec_Lat,Elec_time,Elec_fps,Elec_azi,Elec_valid,Sonar_Azi,Sonar_Type,Sonar_SignalType,Sonar_ID,Sonar_ImageLat,Sonar_ImageLon,Sonar_ImageType,Sonar_ImageID,Sonar_valid) values (%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,%18,%19,%20,%21,%22,%23);")
                .arg(m_mergeMes.Time).arg(m_mergeMes.radar.Radar_Lon).arg(m_mergeMes.radar.Radar_Lat).arg(m_mergeMes.radar.Radar_Type).arg(m_mergeMes.radar.Radar_TrackID).arg(int(m_mergeMes.radar.valid))
                .arg(int(m_mergeMes.elec.Elec_Type)).arg(m_mergeMes.elec.Elec_ID).arg(m_mergeMes.elec.Elec_Lon).arg(m_mergeMes.elec.Elec_Lat).arg(m_mergeMes.elec.time).arg(m_mergeMes.elec.Elec_fps).arg(m_mergeMes.elec.Elec_azi).arg(int(m_mergeMes.elec.valid))
                .arg(m_mergeMes.Sonar.Sonar_Azi).arg(int(m_mergeMes.Sonar.Sonar_Type)).arg(int(m_mergeMes.Sonar.Sonar_SignalType)).arg(m_mergeMes.Sonar.Sonar_ID).arg(m_mergeMes.Sonar.Sonar_ImageLat).arg(m_mergeMes.Sonar.Sonar_ImageLon).arg(int(m_mergeMes.Sonar.Sonar_ImageType))
                .arg(m_mergeMes.Sonar.Sonar_ImageID).arg(int(m_mergeMes.Sonar.valid));
        qDebug()<<str;
        //        Cfg::get()->SetLog(str.toStdString());

        emit signal_Load(4,str.toLocal8Bit(),0,0,NetWorkType::GlobalNetWork);
        //        m_udp->writeDatagram(str.toLocal8Bit(),str.size(),QHostAddress(QString::fromStdString(Cfg::get()->valS("MergeDestinIp"))),34184);

        m_mergeMes.clear();
    }
}

void ProcessSeaData::slot_CollateUpdate()
{
    //发送融合信息
    slot_sendMerge();

    //初始化开始协同时间，各子系统反馈协同结果
    m_bRecRadar=false;
    m_bRecSonar=false;
    m_bRecElec=false;
    m_TimeRadar=QDateTime::currentDateTime();
    m_TimeSonar=QDateTime::currentDateTime();
    m_TimeElec=QDateTime::currentDateTime();
    m_CollateTime=QDateTime::currentDateTime().toTime_t();
    m_collateComm.CurTime = m_CollateTime;

    qDebug()<<"******slot_CollateUpdate******"<<m_beginTime.secsTo(QDateTime::currentDateTime());
    if(m_beginTime.secsTo(QDateTime::currentDateTime())>600)
    {
        emit signal_SeaStatus(Data_UI_TYPE::Merge,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+QString::fromLocal8Bit(":协同600秒停止;"));
        qDebug()<<QString::fromLocal8Bit("********************协同600秒停止****************");
        //停止
        m_CollateTimer->stop();

        //立即发送协同状态
        m_Collaboratestatus.TogetherCondition=1;
        m_Collaboratestatus.ElecResponse=0;
        m_Collaboratestatus.RadarResponse=0;
        m_Collaboratestatus.SonarResponse=0;
        emit signal_CollStatusChange(m_Collaboratestatus);

        //发送结束协同指令
        m_collateComm.Command=1;
        emit signal_Load(0,QByteArray((char*)&m_collateComm, sizeof(m_collateComm)),MERGE_SYSTEM,MERGE_ColManual,NetWorkType::CollaborateAndClear);
        QString out=QString::number(QDateTime::currentDateTime().toTime_t());
        out.append(QString(":Timer Brocast Collaborate Command,VIP target is: %1, %2, %3, %4 from %5,Command is %6,%7,%8,%9,%10")
                   .arg(m_collateComm.TargetLon).arg(m_collateComm.TargetLat).arg(m_collateComm.TargetAzi).arg((m_collateComm.TargetRange)).arg(int(m_collateComm.From))
                   .arg(int(m_collateComm.Command)).arg(int(m_collateComm.Level)).arg(m_collateComm.CurTime).arg(m_collateComm.CircleAzi).arg(m_collateComm.CircleRradius));
        m_otherlog.write(out.toLocal8Bit()+"\n");
        m_otherlog.flush();

        out=QString::number(QDateTime::currentDateTime().toTime_t());
        out.append(QString(":change collaborate status to Yes."));
        m_otherlog.write(out.toLocal8Bit()+"\n");
        m_otherlog.flush();

        return;
    }

    emit signal_SeaStatus(Data_UI_TYPE::Merge,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                          +QString::fromLocal8Bit(":发送协同指令,发起方为：%1").arg(int(m_collateComm.From)));
    qDebug()<<QString::fromLocal8Bit("********************发送协同指令****************");
    //发送协同指令
    emit signal_Load(0,QByteArray((char*)&m_collateComm, sizeof(m_collateComm)),MERGE_SYSTEM,MERGE_ColManual,NetWorkType::CollaborateAndClear);
    QString out=QString::number(QDateTime::currentDateTime().toTime_t());
    out.append(QString(":Timer Brocast Collaborate Command,VIP target is: %1, %2, %3, %4 from %5,Command is %6,%7,%8,%9,%10,Time is %11")
               .arg(m_collateComm.TargetLon).arg(m_collateComm.TargetLat).arg(m_collateComm.TargetAzi).arg((m_collateComm.TargetRange)).arg(int(m_collateComm.From))
               .arg(int(m_collateComm.Command)).arg(int(m_collateComm.Level)).arg(m_collateComm.CurTime).arg(m_collateComm.CircleAzi).arg(m_collateComm.CircleRradius).arg(QString::number(m_CollateTime)));
    m_otherlog.write(out.toLocal8Bit()+"\n");
    m_otherlog.flush();
}

/**********************************本地网络接收转发公网TCP（融合）**************************************/
void ProcessSeaData::slot_statusmerge(QString v)
{
    emit signal_SeaStatus(Data_UI_TYPE::Merge,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+":Merge-->"+v);
}

void ProcessSeaData::slot_TCPreceivemerge(QByteArray value)
{
    slot_TcpreceiveIn(value,MergeDestinaIP,MergeDestinaPort);
}
