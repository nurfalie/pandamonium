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

#ifndef _pandemonium_kernel_url_h_
#define _pandemonium_kernel_url_h_

#include <QObject>
#include <QNetworkReply>
#include <QSslError>
#include <QTimer>
#include <QUrl>

class QNetworkReply;

class pandemonium_kernel_url: public QObject
{
  Q_OBJECT

 public:
  pandemonium_kernel_url(const QUrl &url,
			 const bool paused,
			 const int depth,
			 QObject *parent);
  ~pandemonium_kernel_url();
  void setPaused(const bool paused);

 private:
  QByteArray m_content;
  QTimer m_abortTimer;
  QTimer m_loadNextTimer;
  QUrl m_url;
  QUrl m_urlToLoad;
  bool m_isLoaded;
  bool m_paused;
  int m_depth;
  void connectReplySignals(QNetworkReply *reply);
  void parseContent(void);

 private slots:
  void slotAbortTimeout(void);
  void slotDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
  void slotError(QNetworkReply::NetworkError code);
  void slotLoadNext(void);
  void slotReplyFinished(void);
  void slotSslErrors(const QList<QSslError> &errors);
};

#endif
