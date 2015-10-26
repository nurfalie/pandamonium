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

#include <QTextDocument>
#include <QTimer>
#include <QtDebug>

#include "pandamonium-common.h"
#include "pandamonium-database.h"
#include "pandamonium-kernel.h"
#include "pandamonium-kernel-url.h"

static bool sortStringListByLength(const QString &a, const QString &b)
{
  return a.length() > b.length();
}

pandamonium_kernel_url::pandamonium_kernel_url
(const QUrl &url, const bool paused, const double request_interval,
 const int search_depth, QObject *parent):
  QObject(parent)
{
  m_abortTimer.setInterval(10000);
  m_isLoaded = false;
  m_paused = paused;
  m_requestInterval = qBound(0.100, request_interval, 100.00);
  m_searchDepth = search_depth;
  m_url = m_urlToLoad = url;
  connect(&m_abortTimer,
	  SIGNAL(timeout(void)),
	  this,
	  SLOT(slotAbortTimeout(void)));
  connect(&m_loadNextTimer,
	  SIGNAL(timeout(void)),
	  this,
	  SLOT(slotLoadNext(void)));

  if(!m_paused)
    {
      m_abortTimer.start();

      QNetworkReply *reply = pandamonium_kernel::get(QNetworkRequest(m_url));

      reply->setParent(this);
      connectReplySignals(reply);
    }

  m_loadNextTimer.setInterval(static_cast<int> (1000 * m_requestInterval));
}

pandamonium_kernel_url::~pandamonium_kernel_url()
{
}

void pandamonium_kernel_url::connectReplySignals(QNetworkReply *reply)
{
  if(!reply)
    return;

  connect(reply,
	  SIGNAL(downloadProgress(qint64, qint64)),
	  this,
	  SLOT(slotDownloadProgress(qint64, qint64)),
	  Qt::UniqueConnection);
  connect(reply,
	  SIGNAL(error(QNetworkReply::NetworkError)),
	  this,
	  SLOT(slotError(QNetworkReply::NetworkError)),
	  Qt::UniqueConnection);
  connect(reply,
	  SIGNAL(finished(void)),
	  this,
	  SLOT(slotReplyFinished(void)),
	  Qt::UniqueConnection);
  connect(reply,
	  SIGNAL(sslErrors(const QList<QSslError> &)),
	  this,
	  SLOT(slotSslErrors(const QList<QSslError> &)),
	  Qt::UniqueConnection);
}

void pandamonium_kernel_url::parseContent(void)
{
  /*
  ** Let's discover all links.
  */

  QString description("");
  QString title("");
  QStringList words;
  bool metaDataOnly = pandamonium_database::isUrlMetaDataOnly(m_url);
  int s = -1;

  if(metaDataOnly)
    {
      QString content("");

      s = m_content.toLower().indexOf("<meta");

      while(s >= 0)
	{
	  QByteArray meta;
	  int e = m_content.indexOf(">", s);

	  if(e >= s - 1)
	    meta = m_content.mid(s, e - s + 1);
	  else
	    break;

	  QByteArray bytes(meta);

	  bytes.replace(" ", "");

	  if(bytes.toLower().contains("name=\"description\"") ||
	     bytes.toLower().contains("name=\"keywords\""))
	    {
	      int s = meta.toLower().indexOf("content");

	      if(s >= 0)
		{
		  int e = -1;

		  s = meta.indexOf("\"", s);
		  e = meta.indexOf("\"", s + 1);

		  if(e >= s)
		    {
		      content.append
			(QString::fromUtf8(meta.mid(s + 1, e - s - 1).
					   constData()));
		      content.append(" ");
		    }
		}
	    }

	  s = m_content.toLower().indexOf("<meta", e);
	}

      words = content.split(QRegExp("\\W+"), QString::SkipEmptyParts);
    }
  else
    {
      QTextDocument textDocument;

      textDocument.setHtml(m_content);
      words = textDocument.toPlainText().
	split(QRegExp("\\W+"), QString::SkipEmptyParts);
      qSort(words.begin(), words.end(), sortStringListByLength);
    }

  while(!words.isEmpty())
    if(!description.contains(words.first()))
      {
	description.append(words.takeFirst());
	description.append(" ");
      }
    else
      words.takeFirst();

  if((s = m_content.toLower().indexOf("<title>")) >= 0)
    {
      int e = m_content.toLower().indexOf("</title>");

      if(e >= s + 7)
	title = QString::fromUtf8
	  (m_content.mid(s + 7, e - s - 7).trimmed().constData());
    }

  pandamonium_database::saveUrlMetaData(description, title, m_urlToLoad);
  s = m_content.toLower().indexOf("<a");

  while(s >= 0)
    {
      QByteArray a;
      int e = m_content.toLower().indexOf("</a>", s);

      if(e >= s - 4)
	a = m_content.mid(s, e - s + 4);
      else
	break;

      s = m_content.toLower().indexOf("<a", e);

      /*
      ** a = <a ...>...</a>
      */

      if(a.toLower().contains("href"))
	{
	  QByteArray href;
	  QUrl url;
	  int e = -1;
	  int s = a.toLower().indexOf("href");

	  s = a.indexOf("\"", s);
	  e = a.indexOf("\"", s + 1);
	  href = a.mid(s + 1, e - s - 1);
	  url = QUrl::fromEncoded(href);

	  if(href.startsWith("/"))
	    {
	      url = m_url;
	      url = url.resolved(QUrl::fromEncoded(href));
	    }
	  else if(href.startsWith("//"))
	    url.setScheme(m_url.scheme());

	  if(url.toString().startsWith(m_url.toString()))
	    pandamonium_database::markUrlAsVisited(url, false);
	}
    }

  m_content.clear();
}

void pandamonium_kernel_url::setPaused(const bool paused)
{
  m_paused = paused;

  if(m_paused)
    m_loadNextTimer.stop();
  else
    {
      if(!m_loadNextTimer.isActive())
	m_loadNextTimer.start();
    }
}

void pandamonium_kernel_url::setRequestInterval(const double request_interval)
{
  m_requestInterval = qBound(0.100, request_interval, 100.00);

  if(m_loadNextTimer.interval() !=
     static_cast<int> (1000 * m_requestInterval))
    m_loadNextTimer.start(static_cast<int> (1000 * m_requestInterval));
}

void pandamonium_kernel_url::slotAbortTimeout(void)
{
  if(!m_isLoaded)
    {
      m_content.clear();

      QNetworkReply *reply = findChild<QNetworkReply *> ();

      if(reply)
	{
	  qDebug() << "Aborting " << reply->url() << "!";
	  reply->deleteLater();
	}
    }
}

void pandamonium_kernel_url::slotDownloadProgress
(qint64 bytesReceived, qint64 bytesTotal)
{
  Q_UNUSED(bytesReceived);
  Q_UNUSED(bytesTotal);

  QNetworkReply *reply = qobject_cast<QNetworkReply *> (sender());

  if(reply)
    m_content.append(reply->readAll());
}

void pandamonium_kernel_url::slotError(QNetworkReply::NetworkError code)
{
  m_abortTimer.stop();
  m_content.clear();
  m_isLoaded = true;

  QNetworkReply *reply = qobject_cast<QNetworkReply *> (sender());

  if(reply)
    {
      pandamonium_database::recordBrokenUrl
	(reply->errorString(), reply->url(), m_url);
      qDebug() << "Network error " << code << "!" << reply->url();
      reply->deleteLater();
    }
  else
    qDebug() << "Network error " << code << "!";
}

void pandamonium_kernel_url::slotLoadNext(void)
{
  if(m_paused)
    return;

  if(!findChildren<QNetworkReply *> ().isEmpty())
    return;

  QUrl url(pandamonium_database::unvisitedChildUrl(m_url));

  if(url.isEmpty() || !url.isValid())
    url = m_url; // Restart.

  if(!url.isEmpty())
    if(url.isValid())
      if(url.scheme().toLower().trimmed() == "http" ||
	 url.scheme().toLower().trimmed() == "https")
	{
	  m_abortTimer.start();
	  m_content.clear();
	  m_isLoaded = false;
	  m_urlToLoad = url;

	  QNetworkReply *reply =
	    pandamonium_kernel::get(QNetworkRequest(m_urlToLoad));

	  reply->setParent(this);
	  connectReplySignals(reply);
	}
}

void pandamonium_kernel_url::slotReplyFinished(void)
{
  m_abortTimer.stop();
  m_isLoaded = true;
  pandamonium_database::markUrlAsVisited(m_urlToLoad, true);

  QNetworkReply *reply = qobject_cast<QNetworkReply *> (sender());
  bool redirect = false;

  if(reply)
    {
      QUrl redirectUrl
	(reply->attribute(QNetworkRequest::
			  RedirectionTargetAttribute).toUrl());

      reply->deleteLater();

      if(!redirectUrl.isEmpty())
	if(redirectUrl.isValid())
	  {
	    redirect = true;
	    redirectUrl = m_url.resolved(redirectUrl);
	    reply = pandamonium_kernel::get(QNetworkRequest(redirectUrl));
	    reply->setParent(this);
	    connectReplySignals(reply);
	  }
    }

  if(!redirect)
    parseContent();
}

void pandamonium_kernel_url::slotSslErrors(const QList<QSslError> &errors)
{
  Q_UNUSED(errors);

  QNetworkReply *reply = qobject_cast<QNetworkReply *> (sender());

  if(reply)
    reply->ignoreSslErrors();
}
