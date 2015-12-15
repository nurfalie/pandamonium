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
**    derived from pandamonium without specific prior written permission.
**
** pandamonium IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** pandamonium, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <QCryptographicHash>
#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUrl>
#include <QtDebug>

#include "pandamonium-common.h"
#include "pandamonium-database.h"

QReadWriteLock pandamonium_database::s_dbIdLock;
quint64 pandamonium_database::s_dbId = 0;

QHash<QString, QString> pandamonium_database::exportDefinition(void)
{
  QHash<QString, QString> hash;
  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_export_definition.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.setForwardOnly(true);

	if(query.exec("SELECT * FROM pandamonium_export_definition"))
	  if(query.next())
	    for(int i = 0; i < query.record().count(); i++)
	      hash[query.record().fieldName(i)] =
		query.value(i).toString();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  return hash;
}

QList<QList<QVariant> > pandamonium_database::parsedLinks(const quint64 limit,
							  const quint64 offset)
{
  QList<QList<QVariant> > list;
  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_parsed_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	/*
	** Oh no! Not a parameter.
	*/

	query.setForwardOnly(true);
	query.prepare
	  (QString("SELECT title, url FROM pandamonium_parsed_urls "
		   "ORDER BY time_inserted DESC LIMIT %1 OFFSET %2").
	   arg(limit).
	   arg(offset));

	if(query.exec())
	  while(query.next())
	    {
	      QUrl url(QUrl::fromEncoded(query.value(1).toByteArray()));

	      if(!url.isEmpty())
		if(url.isValid())
		  {
		    QList<QVariant> values;

		    values << query.value(0).toString() << url;
		    list << values;
		  }
	    }
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  return list;
}

QList<QList<QVariant> > pandamonium_database::searchUrls(void)
{
  QList<QList<QVariant> > list;
  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_search_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.setForwardOnly(true);

	if(query.exec("SELECT paused, request_interval, search_depth, url "
		      "FROM pandamonium_search_urls"))
	  while(query.next())
	    {
	      QList<QVariant> values;

	      values << query.value(0).toInt();
	      values << query.value(1).toDouble();
	      values << query.value(2).toInt();
	      values << QUrl::fromEncoded(query.value(3).toByteArray());
	      list << values;
	    }
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  return list;
}

QPair<QSqlDatabase, QString> pandamonium_database::database(void)
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

QPair<quint64, quint64> pandamonium_database::unvisitedAndVisitedNumbers(void)
{
  QPair<QSqlDatabase, QString> pair;
  QPair<quint64, quint64> numbers;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_visited_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.setForwardOnly(true);

	if(query.exec("SELECT COUNT(*), 'u' FROM pandamonium_visited_urls "
		      "WHERE visited = 0 UNION "
		      "SELECT COUNT(*), 'v' FROM pandamonium_visited_urls "
		      "WHERE visited = 1 ORDER BY 2"))
	  {
	    query.next();
	    numbers.first = query.value(0).toULongLong();
	    query.next();
	    numbers.second = query.value(0).toULongLong();
	  }
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  return numbers;
}

QUrl pandamonium_database::unvisitedChildUrl(const QUrl &url)
{
  QPair<QSqlDatabase, QString> pair;
  QUrl new_url;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_visited_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.setForwardOnly(true);
	query.prepare
	  ("SELECT url, visited FROM pandamonium_visited_urls "
	   "WHERE url LIKE ? AND visited = 0");
	query.bindValue
	  (0, (pandamonium_common::toEncoded(url) + "%").constData());

	if(query.exec())
	  if(query.next())
	    new_url = QUrl::fromEncoded(query.value(0).toByteArray());
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  return new_url;
}

bool pandamonium_database::isKernelActive(void)
{
  QPair<QSqlDatabase, QString> pair;
  bool active = false;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_kernel_command.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.setForwardOnly(true);

	if(query.exec("SELECT COUNT(*) FROM pandamonium_kernel_command"))
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

bool pandamonium_database::isUrlMetaDataOnly(const QUrl &url)
{
  bool state = true;
  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_search_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.setForwardOnly(true);
	query.prepare("SELECT meta_data_only "
		      "FROM pandamonium_search_urls WHERE url = ?");
	query.bindValue(0, pandamonium_common::toEncoded(url));

	if(query.exec())
	  if(query.next())
	    state = query.value(0).toInt();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  return state;
}

bool pandamonium_database::saveExportDefinition
(const QHash<QString, QString> &hash)
{
  QPair<QSqlDatabase, QString> pair;
  bool ok = false;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_export_definition.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.prepare("INSERT OR REPLACE INTO pandamonium_export_definition("
		      "database_path, database_table, "
		      "field_content, "
		      "field_description, field_title, field_url) "
		      "VALUES(?, ?, ?, ?, ?, ?)");
	query.bindValue(0, hash["database_path"]);
	query.bindValue(1, hash["database_table"]);
	query.bindValue(2, hash["field_content"]);
	query.bindValue(3, hash["field_description"]);
	query.bindValue(4, hash["field_title"]);
	query.bindValue(5, hash["field_url"]);
	ok = query.exec();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  return ok;
}

bool pandamonium_database::shouldTerminateKernel(const qint64 process_id)
{
  QPair<QSqlDatabase, QString> pair;
  bool terminate = false;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_kernel_command.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.setForwardOnly(true);
	query.prepare("SELECT command FROM pandamonium_kernel_command "
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
		query.exec("DELETE FROM pandamonium_kernel_command");
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

qint64 pandamonium_database::kernelProcessId(void)
{
  QPair<QSqlDatabase, QString> pair;
  qint64 process_id = 0;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_kernel_command.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.setForwardOnly(true);

	if(query.exec("SELECT kernel_process_id FROM "
		      "pandamonium_kernel_command"))
	  if(query.next())
	    process_id = query.value(0).toLongLong();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  return process_id;
}


qint64 pandamonium_database::parsedLinksCount(void)
{
  QPair<QSqlDatabase, QString> pair;
  qint64 count = 0;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_parsed_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.setForwardOnly(true);

	if(query.exec("SELECT COUNT(*) FROM pandamonium_parsed_urls"))
	  if(query.next())
	    count = query.value(0).toLongLong();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  return count;
}

void pandamonium_database::addSearchUrl(const QString &str)
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
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_search_urls.db");

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

	hash.addData(pandamonium_common::toEncoded(url));
	query.prepare("INSERT OR REPLACE INTO pandamonium_search_urls"
		      "(url, url_hash) "
		      "VALUES(?, ?)");
	query.bindValue(0, pandamonium_common::toEncoded(url));
	query.bindValue(1, hash.result().toHex().constData());
	query.exec();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
}

void pandamonium_database::createdb(void)
{
  QStringList fileNames;

  fileNames << "pandamonium_broken_urls.db"
	    << "pandamonium_export_definition.db"
	    << "pandamonium_kernel_command.db"
	    << "pandamonium_parsed_urls.db"
	    << "pandamonium_search_urls.db"
	    << "pandamonium_visited_urls.db";

  foreach(QString fileName, fileNames)
    {
      QPair<QSqlDatabase, QString> pair;

      {
	pair = database();
	pair.first.setDatabaseName
	  (pandamonium_common::homePath() + QDir::separator() + fileName);

	if(pair.first.open())
	  {
	    QSqlQuery query(pair.first);

	    if(fileName == "pandamonium_broken_urls.db")
	      query.exec
		("CREATE TABLE IF NOT EXISTS pandamonium_broken_urls("
		 "error_string TEXT NOT NULL, "
		 "url TEXT NOT NULL, "
		 "url_hash TEXT NOT NULL PRIMARY KEY, "
		 "url_parent TEXT NOT NULL)");
	    else if(fileName == "pandamonium_export_definition.db")
	      {
		query.exec
		  ("CREATE TABLE IF NOT EXISTS pandamonium_export_definition("
		   "database_path TEXT NOT NULL, "
		   "database_table TEXT NOT NULL, "
		   "field_content BLOB NOT NULL, "
		   "field_description TEXT NOT NULL, "
		   "field_title TEXT NOT NULL, "
		   "field_url TEXT NOT NULL, "
		   "PRIMARY KEY(database_path, "
		   "database_table, field_description, field_title, "
		   "field_url))");
		query.exec
		  ("CREATE TRIGGER IF NOT EXISTS "
		   "pandamonium_export_definition_trigger "
		   "BEFORE INSERT ON pandamonium_export_definition "
		   "BEGIN "
		   "DELETE FROM pandamonium_export_definition;"
		   "END");
	      }
	    else if(fileName == "pandamonium_kernel_command.db")
	      {
		query.exec
		  ("CREATE TABLE IF NOT EXISTS pandamonium_kernel_command("
		   "command TEXT NOT NULL CHECK "
		   "(command IN ('rove', 'terminate')), "
		   "kernel_process_id INTEGER NOT NULL PRIMARY KEY)");
		query.exec
		  ("CREATE TRIGGER IF NOT EXISTS "
		   "pandamonium_kernel_command_trigger "
		   "BEFORE INSERT ON pandamonium_kernel_command "
		   "BEGIN "
		   "DELETE FROM pandamonium_kernel_command;"
		   "END");
	      }
	    else if(fileName == "pandamonium_parsed_urls.db")
	      query.exec
		("CREATE TABLE IF NOT EXISTS pandamonium_parsed_urls("
		 "content BLOB TEXT NOT NULL, "
		 "description TEXT NOT NULL, " // Not a BLOB?
		 "time_inserted INTEGER NOT NULL, "
		 "title TEXT NOT NULL, "
		 "url TEXT NOT NULL PRIMARY KEY)");
	    else if(fileName == "pandamonium_search_urls.db")
	      query.exec
		("CREATE TABLE IF NOT EXISTS pandamonium_search_urls("
		 "meta_data_only INTEGER NOT NULL DEFAULT 1, "
		 "paused INTEGER NOT NULL DEFAULT 0, "
		 "request_interval REAL NOT NULL DEFAULT 0.50, "
		 "search_depth INTEGER NOT NULL DEFAULT -1, "
		 "url TEXT NOT NULL, "
		 "url_hash TEXT NOT NULL PRIMARY KEY)");
	    else if(fileName == "pandamonium_visited_urls.db")
	      query.exec
		("CREATE TABLE IF NOT EXISTS pandamonium_visited_urls("
		 "url TEXT NOT NULL PRIMARY KEY, "
		 "visited INTEGER NOT NULL DEFAULT 0)");
	  }

	pair.first.close();
	pair.first = QSqlDatabase();
      }

      QSqlDatabase::removeDatabase(pair.second);
    }
}

void pandamonium_database::exportUrl
(const QString &str, const bool shouldDelete)
{
  /*
  ** First, let's retrieve the export definition.
  */

  QHash<QString, QString> hash(exportDefinition());

  if(hash.isEmpty())
    return;

  /*
  ** Now, let's retrieve the URL's data.
  */

  QPair<QSqlDatabase, QString> pair;
  QList<QVariant> values;
  bool ok = false;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_parsed_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.setForwardOnly(true);
	query.prepare("SELECT content, description, title FROM "
		      "pandamonium_parsed_urls WHERE url = ?");
	query.bindValue(0, pandamonium_common::toEncoded(QUrl(str)));

	if(query.exec())
	  if(query.next())
	    {
	      ok = true;
	      values << query.value(0).toByteArray()
		     << query.value(1).toString().trimmed()
		     << query.value(2).toString().trimmed()
		     << str;
	    }

	if(!ok)
	  qDebug() << str << QUrl(str) << query.lastError();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);

  if(!ok)
    return;
  else
    ok = false;

  /*
  ** Now, let's write the URL to the export database.
  */

  {
    pair = database();
    pair.first.setDatabaseName(hash.value("database_path"));

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.prepare(QString("INSERT OR REPLACE INTO %1("
			      "%2, %3, %4, %5) VALUES(?, ?, ?, ?)").
		      arg(hash.value("database_table")).
		      arg(hash.value("field_content")).
		      arg(hash.value("field_description")).
		      arg(hash.value("field_title")).
		      arg(hash.value("field_url")));
	query.bindValue(0, values.value(0).toByteArray());
	query.bindValue(1, values.value(1).toString().toUtf8());
	query.bindValue(2, values.value(2).toString().toUtf8());
	query.bindValue
	  (3, pandamonium_common::toEncoded(QUrl::fromEncoded(str.toUtf8())));
	ok = query.exec();

	if(!ok)
	  qDebug() << query.lastError();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);

  /*
  ** Finally, remove the URL.
  */

  if(ok)
    if(shouldDelete)
      removeParsedUrls(QStringList() << str);
}

void pandamonium_database::markUrlAsVisited
(const QUrl &url, const bool visited)
{
  QFileInfo fileInfo
    (pandamonium_common::homePath() + QDir::separator() +
     "pandamonium_visited_urls.db");

  if(fileInfo.size() >= pandamonium_common::maximum_database_size)
    return;

  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName(fileInfo.absoluteFilePath());

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	if(visited)
	  query.prepare("INSERT OR REPLACE INTO pandamonium_visited_urls"
			"(url, visited) "
			"VALUES(?, ?)");
	else
	  {
	    query.exec("PRAGMA synchronous = OFF");
	    query.prepare("INSERT INTO pandamonium_visited_urls"
			  "(url, visited) "
			  "VALUES(?, ?)");
	  }

	query.bindValue(0, pandamonium_common::toEncoded(url));
	query.bindValue(1, visited ? 1 : 0);
	query.exec();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
}

void pandamonium_database::recordBrokenUrl(const QString &error_string,
					   const QUrl &child_url,
					   const QUrl &parent_url)
{
  if(child_url.isEmpty() || !child_url.isValid() ||
     parent_url.isEmpty() || !parent_url.isValid())
    return;

  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_broken_urls.db");

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

	hash.addData(pandamonium_common::toEncoded(child_url));
	query.prepare("INSERT OR REPLACE INTO pandamonium_broken_urls"
		      "(error_string, url, url_hash, url_parent) "
		      "VALUES(?, ?, ?, ?)");
	query.bindValue(0, error_string.trimmed());
	query.bindValue(1, pandamonium_common::toEncoded(child_url));
	query.bindValue(2, hash.result().toHex().constData());
	query.bindValue(3, pandamonium_common::toEncoded(parent_url));
	query.exec();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
}

void pandamonium_database::recordKernelDeactivation(const qint64 process_id)
{
  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_kernel_command.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	if(process_id == 0)
	  query.exec("DELETE FROM pandamonium_kernel_command");
	else
	  {
	    query.prepare("DELETE FROM pandamonium_kernel_command "
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

void pandamonium_database::recordKernelProcessId(const qint64 process_id)
{
  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_kernel_command.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);
	bool ok = true;

	query.setForwardOnly(true);

	if(query.exec("SELECT command FROM pandamonium_kernel_command"))
	  if(query.next())
	    if(query.value(0).toString().trimmed() == "terminate")
	      ok = false;

	if(ok)
	  {
	    query.prepare("INSERT INTO pandamonium_kernel_command"
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

void pandamonium_database::removeBrokenUrls(const QStringList &list)
{
  if(list.isEmpty())
    return;

  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_broken_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.exec("PRAGMA secure_delete = ON");

	foreach(QString str, list)
	  {
	    query.prepare("DELETE FROM pandamonium_broken_urls "
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

void pandamonium_database::removeParsedUrls(const QStringList &list)
{
  if(list.isEmpty())
    return;

  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_parsed_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.exec("PRAGMA secure_delete = ON");

	foreach(QString str, list)
	  {
	    query.prepare("DELETE FROM pandamonium_parsed_urls "
			  "WHERE url = ?");
	    query.bindValue(0, pandamonium_common::toEncoded(QUrl(str)));
	    query.exec();
	  }
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
}

void pandamonium_database::removeSearchUrls(const QStringList &list)
{
  if(list.isEmpty())
    return;

  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_search_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.exec("PRAGMA secure_delete = ON");

	foreach(QString str, list)
	  {
	    query.prepare("DELETE FROM pandamonium_search_urls "
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

void pandamonium_database::saveRequestInterval(const QString &request_interval,
					       const QVariant &url_hash)
{
  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_search_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.prepare("UPDATE pandamonium_search_urls "
		      "SET request_interval = ? "
		      "WHERE url_hash = ?");
	query.bindValue(0, request_interval.toDouble());
	query.bindValue(1, url_hash.toString());
	query.exec();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
}

void pandamonium_database::saveSearchDepth(const QString &search_depth,
					   const QVariant &url_hash)
{
  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_search_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.prepare("UPDATE pandamonium_search_urls "
		      "SET search_depth = ? "
		      "WHERE url_hash = ?");
	query.bindValue(0, search_depth.toInt());
	query.bindValue(1, url_hash.toString());
	query.exec();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
}

void pandamonium_database::saveUrlMetaData(const QByteArray &content,
					   const QString &description,
					   const QString &title,
					   const QUrl &url)
{
  QFileInfo fileInfo
    (pandamonium_common::homePath() + QDir::separator() +
     "pandamonium_parsed_urls.db");

  if(fileInfo.size() >= pandamonium_common::maximum_database_size)
    return;

  QPair<QSqlDatabase, QString> pair;

  {
    pair = database();
    pair.first.setDatabaseName(fileInfo.absoluteFilePath());

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.prepare("INSERT OR REPLACE INTO pandamonium_parsed_urls"
		      "(content, description, time_inserted, title, url)"
		      "VALUES(?, ?, ?, ?, ?)");
	query.bindValue(0, content);

	if(description.trimmed().isEmpty())
	  query.bindValue(1, pandamonium_common::toEncoded(url));
	else
	  query.bindValue(1, description.trimmed());

	query.bindValue(2, QDateTime::currentDateTime().toTime_t());

	if(title.trimmed().isEmpty())
	  query.bindValue(3, pandamonium_common::toEncoded(url));
	else
	  query.bindValue(3, title.trimmed());

	query.bindValue(4, pandamonium_common::toEncoded(url));
	query.exec();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
}
