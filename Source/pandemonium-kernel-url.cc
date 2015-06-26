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

#include <QTextDocument>
#include <QTimer>
#include <QtDebug>

#include "pandemonium-common.h"
#include "pandemonium-database.h"
#include "pandemonium-kernel.h"
#include "pandemonium-kernel-url.h"

static bool sortStringListByLength(const QString &a, const QString &b)
{
  return a.length() > b.length();
}

pandemonium_kernel_url::pandemonium_kernel_url
(const QUrl &url, const bool paused, const int depth, QObject *parent):
  QObject(parent)
{
  m_abortTimer.setInterval(10000);
  m_depth = depth;
  m_isLoaded = false;
  m_paused = paused;
  m_url = m_urlToLoad = url;
  connect(&m_abortTimer,
	  SIGNAL(timeout(void)),
	  this,
	  SLOT(slotAbortTimeout(void)));
  connect(&m_loadNextTimer,
	  SIGNAL(timeout(void)),
	  this,
	  SLOT(slotLoadNext(void)));
  m_abortTimer.start();

  if(!m_paused)
    {
      QNetworkReply *reply = pandemonium_kernel::get
	(QNetworkRequest(m_url));

      reply->setParent(this);
      connectReplySignals(reply);
    }

  QSettings settings;
  double interval = settings.value("pandemonium_kernel_load_interval").
    toDouble();

  interval = qBound(0.50, interval, 100.00);
  m_loadNextTimer.setInterval(1000 * interval);
}

pandemonium_kernel_url::~pandemonium_kernel_url()
{
}

void pandemonium_kernel_url::connectReplySignals(QNetworkReply *reply)
{
  if(!reply)
    return;

  connect(reply,
	  SIGNAL(downloadProgress(qint64, qint64)),
	  this,
	  SLOT(slotDownloadProgress(qint64, qint64)));
  connect(reply,
	  SIGNAL(error(QNetworkReply::NetworkError)),
	  this,
	  SLOT(slotError(QNetworkReply::NetworkError)));
  connect(reply,
	  SIGNAL(finished(void)),
	  this,
	  SLOT(slotReplyFinished(void)));
  connect(reply,
	  SIGNAL(sslErrors(const QList<QSslError> &)),
	  this,
	  SLOT(slotSslErrors(const QList<QSslError> &)));
}

void pandemonium_kernel_url::parseContent(void)
{
  /*
  ** Let's discover all links.
  */

  QString description("");
  QString title("");
  QStringList words;
  bool metaDataOnly = pandemonium_database::isUrlMetaDataOnly(m_url);
  int s = -1;

  if(metaDataOnly)
    {
      QString content("");

      s = m_content.indexOf("<meta");

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

	  if(bytes.contains("name=\"description\"") ||
	     bytes.contains("name=\"keywords\""))
	    {
	      int s = meta.indexOf("content");

	      if(s >= 0)
		{
		  int e = -1;

		  s = meta.indexOf("\"", s);
		  e = meta.indexOf("\"", s + 1);

		  if(e >= s)
		    {
		      content.append(meta.mid(s + 1, e - s - 1));
		      content.append(" ");
		    }
		}
	    }

	  s = m_content.indexOf("<meta", e);
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

  description = qCompress(description.trimmed().toUtf8(), 9);
  pandemonium_database::saveUrlMetaData(description, title, m_urlToLoad);

  if((s = m_content.indexOf("<title>")) >= 0)
    {
      int e = m_content.indexOf("</title>");

      if(e >= s + 7)
	title = m_content.mid(s + 7, e - s - 7).trimmed();
    }

  s = m_content.indexOf("<a");

  while(s >= 0)
    {
      QByteArray a;
      int e = m_content.indexOf("</a>", s);

      if(e >= s - 4)
	a = m_content.mid(s, e - s + 4);
      else
	break;

      s = m_content.indexOf("<a", e);

      /*
      ** a = <a ...>...</a>
      */

      if(a.contains("href"))
	{
	  QByteArray href;
	  QUrl url;
	  int e = -1;
	  int s = a.indexOf("href");

	  s = a.indexOf("\"", s);
	  e = a.indexOf("\"", s + 1);
	  href = a.mid(s + 1, e - s - 1);
	  url = QUrl(href);

	  if(href.startsWith("/"))
	    {
	      url = m_url;
	      url = url.resolved(QUrl(href));
	    }
	  else if(href.startsWith("//"))
	    url.setScheme(m_url.scheme());

	  if(url.toString().startsWith(m_url.toString()))
	    pandemonium_database::markUrlAsVisited(url, false);
	}
    }

  m_content.clear();
}

void pandemonium_kernel_url::setPaused(const bool paused)
{
  m_paused = paused;

  if(!m_paused)
    if(!m_loadNextTimer.isActive())
      m_loadNextTimer.start();
}

void pandemonium_kernel_url::slotAbortTimeout(void)
{
  if(!m_isLoaded)
    {
      m_content.clear();

      QNetworkReply *reply = findChild<QNetworkReply *> ();

      if(reply)
	reply->deleteLater();
    }
}

void pandemonium_kernel_url::slotDownloadProgress
(qint64 bytesReceived, qint64 bytesTotal)
{
  Q_UNUSED(bytesReceived);
  Q_UNUSED(bytesTotal);

  QNetworkReply *reply = qobject_cast<QNetworkReply *> (sender());

  if(reply)
    m_content.append(reply->readAll());
}

void pandemonium_kernel_url::slotError(QNetworkReply::NetworkError code)
{
  m_abortTimer.stop();
  m_content.clear();
  m_isLoaded = true;

  QNetworkReply *reply = qobject_cast<QNetworkReply *> (sender());

  if(reply)
    {
      qDebug() << "Network error " << code << "!" << reply->url();
      reply->deleteLater();
    }
  else
    qDebug() << "Network error " << code << "!";
}

void pandemonium_kernel_url::slotLoadNext(void)
{
  if(m_paused)
    return;

  QUrl url(pandemonium_database::unvisitedChildUrl(m_url));

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
	    pandemonium_kernel::get(QNetworkRequest(m_urlToLoad));

	  reply->setParent(this);
	  connectReplySignals(reply);
	}
}

void pandemonium_kernel_url::slotReplyFinished(void)
{
  m_abortTimer.stop();
  m_isLoaded = true;
  pandemonium_database::markUrlAsVisited(m_urlToLoad, true);

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
	    reply = pandemonium_kernel::get(QNetworkRequest(redirectUrl));
	    reply->setParent(this);
	    connectReplySignals(reply);
	  }
    }

  if(!redirect)
    {
      parseContent();

      QSettings settings;
      double interval = settings.value("pandemonium_kernel_load_interval").
	toDouble();

      interval = qBound(0.50, interval, 100.00);
      m_loadNextTimer.start(1000 * interval);
    }
}

void pandemonium_kernel_url::slotSslErrors(const QList<QSslError> &errors)
{
  Q_UNUSED(errors);

  QNetworkReply *reply = qobject_cast<QNetworkReply *> (sender());

  if(reply)
    reply->ignoreSslErrors();
}
