/*
** Copyright (c) Alexis Megas.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from pandemonium without specific prior written permission.
**
** PANDEMONIUM IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** PANDEMONIUM, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <QDir>
#include <QSqlQuery>

#include "pandemonium.h"
#include "pandemonium_createdb.h"

QReadWriteLock pandemonium_createdb::s_dbIdLock;
quint64 pandemonium_createdb::s_dbId = 0;

QPair<QSqlDatabase, QString> pandemonium_createdb::database(void)
{
  QPair<QSqlDatabase, QString> pair;
  quint64 dbId = 0;

  QWriteLocker locker(&s_dbIdLock);

  dbId = s_dbId += 1;
  locker.unlock();
  pair.first = QSqlDatabase::addDatabase
    ("QSQLITE", QString("database_%1").arg(dbId));
  pair.second = pair.first.connectionName();
  return pair;
}

void pandemonium_createdb::createdb(void)
{
  QStringList fileNames;

  fileNames << "pandemonium_discovered_urls.db"
	    << "pandemonium_search_urls.db";

  foreach(QString fileName, fileNames)
    {
      QPair<QSqlDatabase, QString> pair;

      {
	pair = database();
	pair.first.setDatabaseName
	  (pandemonium::homePath() + QDir::separator() + fileName);

	if(pair.first.open())
	  {
	    QSqlQuery query(pair.first);

	    if(fileName == "pandemonium_discovered_urls.db")
	      query.exec
		("CREATE TABLE IF NOT EXISTS pandemonium_discovered_urls("
		 "description TEXT NOT NULL, "
		 "title TEXT NOT NULL, "
		 "url TEXT NOT NULL)");
	    else
	      query.exec
		("CREATE TABLE IF NOT EXISTS pandemonium_search_urls("
		 "url TEXT NOT NULL, "
		 "url_hash TEXT NOT NULL PRIMARY KEY)");
	  }

	pair.first.close();
	pair.first = QSqlDatabase();
      }

      QSqlDatabase::removeDatabase(pair.second);
    }
}
