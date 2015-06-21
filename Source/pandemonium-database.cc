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

#include <QCryptographicHash>
#include <QSqlQuery>
#include <QUrl>
#include <QtDebug>

#include "pandemonium-common.h"
#include "pandemonium-database.h"
#include "pandemonium-gui.h"

QReadWriteLock pandemonium_database::s_dbIdLock;
quint64 pandemonium_database::s_dbId = 0;

QList<QPair<QString, int> > pandemonium_database::searchUrls(void)
{
  QList<QPair<QString, int> > list;
  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandemonium_common::homePath() + QDir::separator() +
       "pandemonium_search_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.setForwardOnly(true);

	if(query.exec("SELECT search_depth, url "
		      "FROM pandemonium_search_urls"))
	  while(query.next())
	    {
	      QPair<QString, int> pair;

	      pair.first = query.value(1).toString();
	      pair.second = query.value(0).toInt();
	      list << pair;
	    }
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  return list;
}

QPair<QSqlDatabase, QString> pandemonium_database::database(void)
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

bool pandemonium_database::isKernelActive(void)
{
  QPair<QSqlDatabase, QString> pair;
  bool active = false;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandemonium_common::homePath() + QDir::separator() +
       "pandemonium_kernel_command.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.setForwardOnly(true);

	if(query.exec("SELECT COUNT(*) FROM pandemonium_kernel_command"))
	  if(query.next())
	    if(query.value(0).toLongLong() > 0)
	      active = true;
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  return active;
}

bool pandemonium_database::shouldTerminateKernel(const qint64 process_id)
{
  QPair<QSqlDatabase, QString> pair;
  bool terminate = false;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandemonium_common::homePath() + QDir::separator() +
       "pandemonium_kernel_command.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.setForwardOnly(true);

	query.prepare("SELECT command FROM pandemonium_kernel_command "
		      "WHERE kernel_process_id = ?");
	query.bindValue(0, process_id);

	if(query.exec())
	  {
	    if(query.next())
	      {
		if(query.value(0).toString().trimmed() == "terminate")
		  terminate = true;
	      }
	    else
	      {
		query.exec("DELETE FROM pandemonium_kernel_command");
		terminate = true;
	      }
	  }
	else
	  terminate = true;
      }
    else
      terminate = true;

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  return terminate;
}

qint64 pandemonium_database::kernelProcessId(void)
{
  QPair<QSqlDatabase, QString> pair;
  qint64 process_id = 0;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandemonium_common::homePath() + QDir::separator() +
       "pandemonium_kernel_command.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.setForwardOnly(true);

	if(query.exec("SELECT kernel_process_id FROM "
		      "pandemonium_kernel_command"))
	  if(query.next())
	    process_id = query.value(0).toLongLong();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  return process_id;
}

void pandemonium_database::addSearchUrl(const QString &str)
{
  QUrl url(QUrl::fromUserInput(str.trimmed()));

  if(url.isEmpty())
    return;
  else if(!url.isValid())
    return;

  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandemonium_common::homePath() + QDir::separator() +
       "pandemonium_search_urls.db");

    if(pair.first.open())
      {
#if QT_VERSION >= 0x050100
	QCryptographicHash hash(QCryptographicHash::Sha3_512);
#elif QT_VERSION >= 0x050000
	QCryptographicHash hash(QCryptographicHash::Sha512);
#else
	QCryptographicHash hash(QCryptographicHash::Sha1);
#endif
	QSqlQuery query(pair.first);

	hash.addData(url.toEncoded());
	query.prepare("INSERT OR REPLACE INTO pandemonium_search_urls "
		      "(url, url_hash) "
		      "VALUES(?, ?)");
	query.bindValue(0, url.toString());
	query.bindValue(1, hash.result().toHex().constData());
	query.exec();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
}

void pandemonium_database::createdb(void)
{
  QStringList fileNames;

  fileNames << "pandemonium_discovered_urls.db"
	    << "pandemonium_kernel_command.db"
	    << "pandemonium_search_urls.db";

  foreach(QString fileName, fileNames)
    {
      QPair<QSqlDatabase, QString> pair;

      {
	pair = database();
	pair.first.setDatabaseName
	  (pandemonium_common::homePath() + QDir::separator() + fileName);

	if(pair.first.open())
	  {
	    QSqlQuery query(pair.first);

	    if(fileName == "pandemonium_discovered_urls.db")
	      query.exec
		("CREATE TABLE IF NOT EXISTS pandemonium_discovered_urls("
		 "description TEXT NOT NULL, "
		 "title TEXT NOT NULL, "
		 "url TEXT NOT NULL PRIMARY KEY)");
	    else if(fileName == "pandemonium_kernel_command.db")
	      {
		query.exec
		  ("CREATE TABLE IF NOT EXISTS pandemonium_kernel_command("
		   "command TEXT NOT NULL, "
		   "kernel_process_id INTEGER NOT NULL PRIMARY KEY)");
		query.exec
		  ("CREATE TRIGGER IF NOT EXISTS "
		   "pandemonium_kernel_command_trigger "
		   "BEFORE INSERT ON pandemonium_kernel_command "
		   "BEGIN "
		   "DELETE FROM pandemonium_kernel_command;"
		   "END");
	      }
	    else
	      query.exec
		("CREATE TABLE IF NOT EXISTS pandemonium_search_urls("
		 "search_depth INTEGER NOT NULL DEFAULT 15, "
		 "url TEXT NOT NULL, "
		 "url_hash TEXT NOT NULL PRIMARY KEY)");
	  }

	pair.first.close();
	pair.first = QSqlDatabase();
      }

      QSqlDatabase::removeDatabase(pair.second);
    }
}

void pandemonium_database::recordKernelDeactivation(const qint64 process_id)
{
  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandemonium_common::homePath() + QDir::separator() +
       "pandemonium_kernel_command.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	if(process_id == 0)
	  query.exec("DELETE FROM pandemonium_kernel_command");
	else
	  {
	    query.prepare("DELETE FROM pandemonium_kernel_command "
			  "WHERE kernel_process_id = ?");
	    query.bindValue(0, process_id);
	    query.exec();
	  }
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
}

void pandemonium_database::recordKernelProcessId(const qint64 process_id)
{
  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandemonium_common::homePath() + QDir::separator() +
       "pandemonium_kernel_command.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);
	bool ok = true;

	query.setForwardOnly(true);

	if(query.exec("SELECT command FROM pandemonium_kernel_command"))
	  if(query.next())
	    if(query.value(0).toString().trimmed() == "terminate")
	      ok = false;

	if(ok)
	  {
	    query.prepare("INSERT INTO pandemonium_kernel_command"
			  "(command, kernel_process_id) "
			  "VALUES(?, ?)");
	    query.bindValue(0, "rove");
	    query.bindValue(1, process_id);
	    query.exec();
	  }
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
}

void pandemonium_database::removeSearchUrls(const QStringList &list)
{
  if(list.isEmpty())
    return;

  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandemonium_common::homePath() + QDir::separator() +
       "pandemonium_search_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.exec("PRAGMA secure_delete = ON");

	foreach(QString str, list)
	  {
	    query.prepare("DELETE FROM pandemonium_search_urls "
			  "WHERE url_hash = ?");
	    query.bindValue(0, str);
	    query.exec();
	  }
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
}

void pandemonium_database::saveDepth(const QString &depth,
				     const QVariant &url_hash)
{
  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandemonium_common::homePath() + QDir::separator() +
       "pandemonium_search_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.prepare("UPDATE pandemonium_search_urls "
		      "SET search_depth = ? "
		      "WHERE url_hash = ?");
	query.bindValue(0, depth.toInt());
	query.bindValue(1, url_hash.toString());
	query.exec();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
}
