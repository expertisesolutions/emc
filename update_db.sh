#!/bin/bash
# id3.sh
# Carlos Carvalho <carloslack@gmail.com>
#
# SQLite3 create/insert for testing purposes.
#
# There are still some bugs but the purpose is only to have some data
# written in database se we can perform some queries from application.
# The point is to have have data making sense and inserted correctly, if
# we miss something it isn't a big deal.
#

DB=./src/emc.db
tracks=$HOME/Music
t_artists="artists"
t_albums="albums"
t_tracks="tracks"

if [ -f $DB ] ; then
   rm -rf $DB
fi

exists()
{
   exe=$(which $1)
   if [ ! -f "$exe" ] ; then
      >&2 echo "error: $1 not found"
      echo "exit"
   fi
   echo $exe
}

sql=$(exists 'sqlite3')
id3=$(exists 'id3v2')

#Create empty database or exit if sqlite3 not found
$sql $DB ".dump" >/dev/null 2>&1
if [ $? != 0 ] ; then
   >&2 echo "error creating database"
fi

#Exit if not found
$id3 >/dev/null 2>&1

$sql $DB "CREATE TABLE $t_artists (id INTEGER PRIMARY KEY, name TEXT NOT NULL);"
$sql $DB "CREATE TABLE $t_albums (id INTEGER PRIMARY KEY, id_artist INTEGER, name TEXT NOT NULL, genre TEXT DEFAULT NULL, year INTEGER DEFAULT 0, image BLOB DEFAULT 0);"
$sql $DB "CREATE TABLE $t_tracks (id INTEGER PRIMARY KEY, id_artist INTEGER, id_album INTEGER, name TEXT NOT NULL, track INTEGER, file TEXT NOT NULL);"

results()
{
   $sql $DB ".dump"
}

artist_get()
{
   _ret=$(cat .tmpfile |grep '^TPE2' |cut -d ":" -f2| cut -d "/" -f1 |sed -e 's/^ //g;s/^  //g;s/\x27//g')
   if [ -z "$artist" ] ; then
     _ret=$(cat .tmpfile |grep '^TPE1' |cut -d ":" -f2| cut -d "/" -f1 |sed -e 's/^ //g;s/^  //g;s/\x27//g')
   fi

   echo $_ret;
}

#create artists
artist_insert()
{
   echo "<Artist Insert>"
   find $tracks -name "*.mp3" |while read line ; do
      $id3 -l "$line" > .tmpfile
      artist=$(artist_get)
      if [ ! -z "$artist" ] ; then
         row=$($sql $DB "SELECT * FROM $t_artists WHERE name = '$artist' COLLATE NOCASE;") 2>/dev/null
         if [ -z "$row" ] ; then
            $sql -echo $DB "INSERT INTO $t_artists (name) VALUES('$artist');" 2>/dev/null
         fi
      fi
   done
   echo "</Artist Insert>"
}

#create albums
album_insert()
{
   echo "<Album Insert>"
   find $tracks -name "*.mp3" |while read line ; do
      $id3 -l "$line" > .tmpfile
      artist=$(artist_get)
      if [ ! -z "$artist" ] ; then
         name=$(cat .tmpfile |grep '^TALB' |cut -d ":" -f2| sed -e 's/^ //g;s/^  //g;s/\x27//g')
         id_artist=$($sql $DB "SELECT id FROM $t_artists WHERE name = '$artist' COLLATE NOCASE;") 2>/dev/null
         row=$($sql $DB "SELECT * FROM $t_albums WHERE name = '$name' COLLATE NOCASE AND id_artist = "$id_artist";") 2>/dev/null
         if [ -z "$row" ] ; then
            p=$(dirname "$line")
            cd "$p"
            list=$(ls *.jpg 2>/dev/null)
            status=$(echo $?)
            array=($list)
            cd - >/dev/null 2>&1
            genre=$(cat .tmpfile |grep '^TCON' |cut -d ":" -f2| sed -e 's/^ //g;s/^  //g;s/\x27//g')
            year=$(cat .tmpfile |grep '^TYER' |cut -d ":" -f2| sed -e 's/^ //g;s/^  //g;s/\x27//g')
            if [ -z "$year" ] ; then
              year=0
            fi
#            if [ $status -eq 0 ] ; then
#               p=$(dirname "$line")
               # readfile() && writefile() are extensions
#               $sql -echo $DB "INSERT INTO $t_albums (id_artist, name, genre, year, image) VALUES($id_artist, '$name', '$genre', $year, readfile('$p/${array[0]}'));" 2>/dev/null
#            else
               $sql -echo $DB "INSERT INTO $t_albums (id_artist, name, genre, year) VALUES($id_artist, '$name', '$genre', $year);" 2>/dev/null
#            fi
         fi

      fi
   done
   echo "</Album Insert>"
}

#create tracks
track_insert()
{
   echo "<Track Insert>"
   find $tracks -name "*.mp3" |while read line ; do
      $id3 -l "$line" > .tmpfile
      artist=$(artist_get)
      if [ ! -z "$artist" ] ; then
         name_album=$(cat .tmpfile |grep '^TALB' |cut -d ":" -f2| sed -e 's/^ //g;s/^  //g;s/\x27//g')
         name_track=$(cat .tmpfile |grep '^TIT2' |cut -d ":" -f2| sed -e 's/^ //g;s/^  //g;s/\x27//g')
         n_track=$(cat .tmpfile |grep '^TRCK' |cut -d ":" -f2| sed -e 's/^ //g;s/^  //g;s/\x27//g' |cut -d "/" -f1)
         id_artist=$($sql $DB "SELECT id FROM $t_artists WHERE name = '$artist' COLLATE NOCASE;") 2>/dev/null
         id_album=$($sql $DB "SELECT id FROM $t_albums WHERE id_artist = "$id_artist" AND name = '$name_album' COLLATE NOCASE;") 2>/dev/null
         if [ ! -z $id_album ] ; then #XXX
            row=$($sql $DB "SELECT * FROM $t_tracks WHERE name = '$name_track' AND id_artist = "$id_artist" AND id_album = "$id_album";") 2>/dev/null
            if [ -z "$row" ] ; then
               $sql -echo $DB "INSERT INTO $t_tracks (id_artist, id_album, name, track, file) VALUES($id_artist, $id_album, '$name_track', $n_track, '$line');" 2>/dev/null
            fi
         fi
      fi
   done
   echo "</Track Insert>"
}

artist_insert
album_insert
track_insert
results

rm -rf .tmpfile 2>/dev/null
#EOF

