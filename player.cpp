/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "player.h"
#include "playlistmodel.h"
#include <QMediaService>
#include <QMediaPlaylist>
#include <QMediaMetaData>
#include <QObject>
#include <QFileInfo>
#include <QTime>
#include <QDir>
#include <QStandardPaths>

Player::Player(QObject *parent)
    : QObject(parent)
{
    m_player = new QMediaPlayer(this);
    m_playlist = new QMediaPlaylist(this);
    m_playlistModel = new PlaylistModel(this);
    open();

    // n????u playlist null ma?? set playlist thi?? ct se?? crash
    if(getPlaylist() != NULL)
        m_player->setPlaylist(m_playlist);

    if (!m_playlist->isEmpty())
        m_playlist->setCurrentIndex(0);
}

Player::~Player()
{
    if(m_player != NULL){
        qDebug() << "delete m_player";
        delete m_player;
    }
    if(m_playlist != NULL){
        qDebug() << "delete m_playlist";
        delete m_playlist;
    }
    if(m_playlistModel != NULL){
        qDebug() << "delete m_playlistModel";
        delete m_playlistModel;
    }
}

// load file .mp3 trong mu??c musicLocation
void Player::open()
{
    QDir directory(QStandardPaths::standardLocations(QStandardPaths::MusicLocation)[0]);
    QFileInfoList musics = directory.entryInfoList(QStringList() << "*.mp3",QDir::Files);
    QList<QUrl> urls;
    for (int i = 0; i < musics.length(); i++){
        urls.append(QUrl::fromLocalFile(musics[i].absoluteFilePath()));
    }
    for(int i = 0; i< urls.size(); i++)
        qDebug() << urls[i].toString();
    addToPlaylist(urls);
}

// add URL va??o playlist
// add SONG va??o playlist model
void Player::addToPlaylist(const QList<QUrl> &urls)
{
    for (auto &url: urls) {
        m_playlist->addMedia(url);
        QString artSong = getAlbumArt(url);
        qDebug() << "url: " << artSong;
        FileRef f(url.toLocalFile().toStdString().c_str());
        Tag *tag = f.tag();
        Song song(QString::fromWCharArray(tag->title().toCWString()),
                  QString::fromWCharArray(tag->artist().toCWString()),
                  url.toDisplayString(), artSong);
        m_playlistModel->addSong(song);
    }
}

// covert th???i gian t??? millisecond sang gi??? ph??t gi??y
QString Player::getTimeInfo(qint64 currentInfo)
{
    QString tStr = "00:00";
    currentInfo = currentInfo/1000;
    qint64 durarion = m_player->duration()/1000;
    if (currentInfo || durarion) {
        QTime currentTime((currentInfo / 3600) % 60, (currentInfo / 60) % 60,
                          currentInfo % 60, (currentInfo * 1000) % 1000);
        QTime totalTime((durarion / 3600) % 60, (m_player->duration() / 60) % 60,
                        durarion % 60, (m_player->duration() * 1000) % 1000);
        QString format = "mm:ss";
        if (durarion > 3600)
            format = "hh::mm:ss";
        tStr = currentTime.toString(format);
    }
    return tStr;
}

// ??i????u khi????n ca??c tra??ng tha??i ch??i cu??a ca??c ba??i ha??t
void Player::playBackModeList(QMediaPlaylist::PlaybackMode mode)
{
    // ch??i ng????u nhi??n
    if(mode == QMediaPlaylist::Random)
        m_playlist->setPlaybackMode(QMediaPlaylist::Random);
    // ch??i l????p la??i m????t ba??i ha??t
    else if(mode == QMediaPlaylist::CurrentItemInLoop)
        m_playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    // ch??i bi??nh th??????ng
    else if(mode == QMediaPlaylist::Sequential)
        m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
    else
        qDebug() << "NOT AVAILABLE MODE!!!";

}

QMediaPlayer *Player::getPlayer()
{
    return m_player;
}

QMediaPlaylist *Player::getPlaylist()
{
    return m_playlist;
}

PlaylistModel *Player::getPlaylistModel()
{
    return m_playlistModel;
}

// tri??ch xu????t album art cu??a ba??i ha??t
QString Player::getAlbumArt(QUrl url)
{
    TagLib::MPEG::File mpegFile(url.toLocalFile().toStdString().c_str());
    TagLib::ID3v2::FrameList Frame ;
    TagLib::ID3v2::AttachedPictureFrame *PicFrame ;
    void *SrcImage ;
    unsigned long Size ;
    FILE *jpegFile;
    jpegFile = fopen(QString(url.fileName()+".jpg").toStdString().c_str(),"wb");
    // ki????n tra xem co?? tag t????n ta??i kh??ng
    if ( mpegFile.ID3v2Tag() )
    {
        Frame = mpegFile.ID3v2Tag()->frameListMap()["APIC"];
        if (!Frame.isEmpty() )
        {
            for(TagLib::ID3v2::FrameList::ConstIterator it = Frame.begin(); it != Frame.end(); ++it)
            {
                PicFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(*it) ;
                if ( PicFrame->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover)
                {
                    Size = PicFrame->picture().size() ;
                    SrcImage = malloc ( Size ) ;
                    if ( SrcImage )
                    {
                        memcpy ( SrcImage, PicFrame->picture().data(), Size ) ;
                        fwrite(SrcImage,Size,1, jpegFile);
                        fclose(jpegFile);
                        free( SrcImage);
                        qDebug() << QUrl::fromLocalFile(url.fileName()+".jpg").toDisplayString();
                        return QUrl::fromLocalFile(url.fileName()+".jpg").toDisplayString();
                    }
                }
            }
        }
    }
    else
    {
        qDebug() <<"id3v2 not present";
        return "qrc:/images/back ground/album_art.png";
    }
    return "qrc:/images/back ground/album_art.png";
}
