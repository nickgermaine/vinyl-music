#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlPropertyMap>
#include <QString>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <taglib/taglib.h>
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/taglib_config.h>
#include <taglib/tpropertymap.h>
#include <qdebug.h>
#include <qqml.h>
#include <QQuickView>
#include <QDateTime>
#include <QQmlContext>
#include <QFileInfo>
#include <QDir>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QList>
#include <QObject>
#include <albumobject.h>
#include <artistobject.h>
#include <QStandardPaths>
#include <main.h>



QList<QObject*> getAlbums(QSqlDatabase db){
    QList<QObject*> albumList;

    if(db.open()){
        QSqlQuery getAllAlbums;
        getAllAlbums.prepare("select * FROM Albums");
        if(getAllAlbums.exec()){
            while(getAllAlbums.next()){
                QString album = getAllAlbums.value(1).toString();
                QString artist = getAllAlbums.value(2).toString();
                QString art = getAllAlbums.value(3).toString();
                std::cout << "Album: " << album.toStdString() << std::endl;

                albumList.append(new AlbumObject(album, artist, art));

            }
        }
    }
    return albumList;
}

QList<QObject*> getArtists(QSqlDatabase db){
    QList<QObject*> artistList;
    if(db.open()){
        QSqlQuery getAllAlbums;
        getAllAlbums.prepare("select * FROM Artists");
        if(getAllAlbums.exec()){
            while(getAllAlbums.next()){
                QString artist = getAllAlbums.value(2).toString();
                artistList.append(new ArtistObject(artist));

            }
        }
    }
    return artistList;

}


QList<QObject*> getAllSongs(QSqlDatabase db){
    QList<QObject*> songList;

    if(db.open()){
        // DB is open; lets get all songs
        QSqlQuery getSongs;
        getSongs.prepare("Select * FROM Songs");
        if(getSongs.exec()){
            while(getSongs.next()){

                QString title = getSongs.value(2).toString();
                QString path = getSongs.value(1).toString();
                QString album = getSongs.value(4).toString();
                QString artist = getSongs.value(3).toString();
                QString art = getSongs.value(5).toString();

                //std::cout << art.toStdString() << std::endl;
                songList.append(new SongObject(path, QString::fromStdString(title.toStdString()), album, artist, art));
                //songList.append(new SongObject(getSongs.value(2).fromValue, getSongs.value(1).fromValue, getSongs.value(4).fromValue, getSongs.value(3).fromValue));
            }
        }
    }
    songList[0]->setObjectName("allSongObjects");
    return songList;
}

void addSongsToDatabase(QDir dir, TagLib::String path, QString newpath, QString filename, QSqlDatabase db){
    // Get TagLib instance of song by path
    TagLib::FileRef f(path.toCString());
    TagLib::Tag *tag = f.tag();
    // std::cout << "Adding " << tag->title() << " " << tag->artist() << " to the database." << std::endl;
    QString artist = tag->artist().toCString();
    QString title = tag->title().toCString();
    QString album = tag->album().toCString();
    //QString art = tag->

    if(db.open()){
        QSqlQuery qry;
        qry.prepare("CREATE TABLE IF NOT EXISTS Songs(id INTEGER PRIMARY KEY AUTOINCREMENT, path TEXT, title TEXT, artist TEXT, album TEXT, art TEXT)");

        if(qry.exec()){
            //std::cout << "Created table" << std::endl;
        }

        QSqlQuery createAlbums;
        createAlbums.prepare("CREATE TABLE IF NOT EXISTS Albums(id INTEGER PRIMARY KEY AUTOINCREMENT, album TEXT, artist TEXT, art TEXT)");

        if(createAlbums.exec()){
            // We created the table if it didnt exist... move along -->
        }

        // Check if item already exists for this path
        QSqlQuery getAlbum;
        getAlbum.prepare("select * from Albums where album='" + album + "'");
        if(getAlbum.exec()){
            if(getAlbum.first()){

            }else{
                QString art = "placeholder";
                bool recursive=true;
                bool symlinks=false;
                dir.setSorting( QDir::Name );
                QDir::Filters df = QDir::Files | QDir::NoDotAndDotDot;
                if (recursive) df |= QDir::Dirs;
                if (not symlinks) df |= QDir::NoSymLinks;
                QStringList qsl = dir.entryList(df, QDir::Name | QDir::DirsFirst);

                // For each: if folder, run this method, if mp3, add to DB
                foreach (const QString &entry, qsl) {
                    QFileInfo finfo(dir, entry);

                     if (finfo.completeSuffix()=="jpg"){
                            art = finfo.absoluteFilePath();
                     }

                }

                QSqlQuery insertAlbum;
                insertAlbum.prepare("INSERT INTO Albums (id, album, artist, art) VALUES (NULL,'"+ album +"', '"+ artist +"', '"+ art +"')");
                if(insertAlbum.exec()){
                    // We inserted the album  :)
                }
            }
        }

        QSqlQuery createArtist;
        createArtist.prepare("CREATE TABLE IF NOT EXISTS Artists(id INTEGER PRIMARY KEY AUTOINCREMENT, artist TEXT),");

        if(createArtist.exec()){
            // We created the table if it didnt exist... move along -->
        }

        // Check if item already exists for this path
        QSqlQuery getArtist;
        getArtist.prepare("select * from Artists where artist='" + artist + "'");
        if(getArtist.exec()){
            if(getArtist.first()){

            }else{

                QSqlQuery insertArtist;
                insertArtist.prepare("INSERT INTO Artists (id, artist) VALUES (NULL,'"+ artist +"')");
                if(insertArtist.exec()){
                    // We inserted the album  :)
                }
            }
        }

        QSqlQuery getSong;
        getSong.prepare("SELECT * FROM Songs WHERE path='" + newpath + "'");
        if(getSong.exec()){
            // If .first == true, it already exists
           if(getSong.first()){
                while(getSong.next()){
                    //std::cout << getSong.value(1).toString().toUtf8().constData() << std::endl;
                }
            }else{
               QString art = "placeholder";
               bool recursive=true;
               bool symlinks=false;
               dir.setSorting( QDir::Name );
               QDir::Filters df = QDir::Files | QDir::NoDotAndDotDot;
               if (recursive) df |= QDir::Dirs;
               if (not symlinks) df |= QDir::NoSymLinks;
               QStringList qsl = dir.entryList(df, QDir::Name | QDir::DirsFirst);

               // For each: if folder, run this method, if mp3, add to DB
               foreach (const QString &entry, qsl) {
                   QFileInfo finfo(dir, entry);

                    if (finfo.completeSuffix()=="jpg"){
                           art = finfo.absoluteFilePath();
                    }

               }
                // If not, we add it.
                QSqlQuery insertqry;
                if(tag->title().isEmpty()){
                    std::cout << "Empty title"
;                    title = filename;
                }
                insertqry.prepare("INSERT INTO Songs (id, path, title, artist, album, art) VALUES "
                                  "(NULL, '"+ newpath +"', '"+ title +"', '"+ artist +"', '"+ album +"', '"+art+"')");
                if(insertqry.exec()){
                    std::cout << "Inserted data successfully" << std::endl;
                }else{
                     std::cout << "Data not inserted" << std::endl;
                }
            }
        }


    }else{
        // If we couldn't open the database it means the entire program is broken and will need a rewrite
        //std::cout << "Couldnt open database" << std::endl;
    }
}

void firstMusicScan(QDir d, QSqlDatabase db, bool recursive=true, bool symlinks=false ) {
    // Get top directory (~/Music by default), and recursively scan for mp3
    d.setSorting( QDir::Name );
    QDir::Filters df = QDir::Files | QDir::NoDotAndDotDot;
    if (recursive) df |= QDir::Dirs;
    if (not symlinks) df |= QDir::NoSymLinks;
    QStringList qsl = d.entryList(df, QDir::Name | QDir::DirsFirst);

    // For each: if folder, run this method, if mp3, add to DB
    foreach (const QString &entry, qsl) {
        QFileInfo finfo(d, entry);
        if ( finfo.isDir() ) {
            QDir sd(finfo.absoluteFilePath());
            firstMusicScan(sd, db);
        } else {
            if (finfo.completeSuffix()=="mp3")
                addSongsToDatabase(finfo.dir(), finfo.absoluteFilePath().toStdString(), QString(finfo.absoluteFilePath()), finfo.fileName(), db);
        }
    }
}

void initialQuery(){
    // Open DB connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setHostName("localhost");
    db.setDatabaseName("vinylmusic");
    std::cout << "Loading database data. " << std::endl;

    if(db.open()){
        // Create Settings table if it isnt there
        QSqlQuery qry;
        qry.prepare("CREATE TABLE IF NOT EXISTS Settings(id INTEGER PRIMARY KEY AUTOINCREMENT, setting TEXT, value TEXT)");
        if(qry.exec()){
            std::cout << "Created table" << std::endl;
        }else{
            std::cout << "Didnt create table" << std::endl;
        }
    }else{
        std::cout << "Couldnt open database" << std::endl;
    }

    // Uncomment for debug purposes
    /*
    QSqlQuery getSettings;
    getSettings.prepare("Select * FROM Settings");
    if(getSettings.exec()){
        while(getSettings.next()){
            std::cout << getSettings.value(1).toString().toUtf8().constData() << std::endl;
        }
    }
    */
}

int main(int argc, char *argv[]){

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    if (app.arguments().size() > 1) {
        // If Vinyl Music was opened with a song path:
        QFileInfo finfo;
        finfo.setFile(app.arguments().at(1));
        engine.rootContext()->setContextProperty("loadedFileFolder", QString(finfo.path()));
    }

    // Initialize the database if not exists, and set Settings table
    initialQuery();

    const QStringList musicLocations = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    QString musicLocation = musicLocations.isEmpty() ?
                QDir::homePath() + QLatin1String("/Music") : musicLocations.first();

    // Open DB and perform initial music dir scan/build Songs table
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setHostName("localhost");
    db.setDatabaseName("vinylmusic");
    std::cout << "Loading database data. " << std::endl;
    firstMusicScan(QDir(musicLocation), db);

    QString stream_directory = musicLocation + QLatin1String("/streams");


    // Create path variables accessible in QML:
    engine.rootContext()->setContextProperty("homeDirectory", musicLocation);
    engine.rootContext()->setContextProperty("streamDirectory", stream_directory);
    engine.rootContext()->setContextProperty("allSongObjects", QVariant::fromValue(getAllSongs(db)));
    engine.rootContext()->setContextProperty("allArtists", QVariant::fromValue(getArtists(db)));
    engine.rootContext()->setContextProperty("allAlbums", QVariant::fromValue(getAlbums(db)));


    // Create view from main.qml:
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    // Run the app
    return app.exec();
}


