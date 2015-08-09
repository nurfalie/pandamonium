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

#ifndef _pandamonium_database_h_
#define _pandamonium_database_h_

#include <QPair>
#include <QReadWriteLock>
#include <QSqlDatabase>
#include <QUrl>

class pandamonium_database
{
 public:
  static QHash<QString, QString> exportDefinition(void);
  static QList<QList<QVariant> > parsedLinks(const quint64 limit,
					     const quint64 offset);
  static QList<QList<QVariant> > searchUrls(void);
  static QPair<QSqlDatabase, QString> database(void);
  static QPair<quint64, quint64> unvisitedAndVisitedNumbers(void);
  static QUrl unvisitedChildUrl(const QUrl &url);
  static bool isKernelActive(void);
  static bool isUrlMetaDataOnly(const QUrl &url);
  static bool saveExportDefinition(const QHash<QString, QString> &hash);
  static bool shouldTerminateKernel(const qint64 process_id);
  static qint64 kernelProcessId(void);
  static qint64 parsedLinksCount(void);
  static void addSearchUrl(const QString &str);
  static void createdb(void);
  static void exportUrl(const QString &str, const bool shouldDelete);
  static void markUrlAsVisited(const QUrl &url, const bool visited);
  static void recordBrokenUrl(const QString &error_string,
			      const QUrl &child_url,
			      const QUrl &parent_url);
  static void recordKernelDeactivation(const qint64 process_id = 0);
  static void recordKernelProcessId(const qint64 process_id);
  static void removeBrokenUrls(const QStringList &list);
  static void removeParsedUrls(const QStringList &list);
  static void removeSearchUrls(const QStringList &list);
  static void saveRequestInterval(const QString &request_interval,
				  const QVariant &url_hash);
  static void saveSearchDepth(const QString &search_depth,
			      const QVariant &url_hash);
  static void saveUrlMetaData(const QString &description,
			      const QString &title,
			      const QUrl &url);

 private:
  pandamonium_database(void)
  {
  }

  static QReadWriteLock s_dbIdLock;
  static quint64 s_dbId;
};

#endif
