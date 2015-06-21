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

#include <QNetworkReply>
#include <QWebFrame>

#include "pandemonium-database.h"
#include "pandemonium-kernel-url.h"

pandemonium_kernel_url::pandemonium_kernel_url
(const QString &url, const int depth, QObject *parent):QObject(parent)
{
  m_depth = depth;
  m_url = m_urlToLoad = QUrl::fromUserInput(url);
  connect(&m_webView,
	  SIGNAL(loadFinished(bool)),
	  this,
	  SLOT(slotLoadFinished(bool)));
  connect(m_webView.page()->networkAccessManager(),
	  SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)),
	  this,
	  SLOT(slotSslErrors(QNetworkReply *, const QList<QSslError> &)));
  m_webView.load(m_urlToLoad);
}

pandemonium_kernel_url::~pandemonium_kernel_url()
{
}

void pandemonium_kernel_url::slotLoadFinished(bool ok)
{
  if(!ok)
    return;

  pandemonium_database::markUrlAsVisited(m_urlToLoad);

  QWebFrame *mainFrame = m_webView.page()->mainFrame();

  if(mainFrame)
    {
      QList<QString> list;
      QMultiMap<QString, QString> map(mainFrame->QWebFrame::metaData());

      list << map.values("description")
	   << map.values("keywords");
      pandemonium_database::saveUrlMetaData(list,
					    m_webView.title(),
					    m_urlToLoad);
    }
}

void pandemonium_kernel_url::slotSslErrors(QNetworkReply *reply,
					   const QList<QSslError> &errors)
{
  Q_UNUSED(errors);

  if(reply)
    reply->ignoreSslErrors();
}
