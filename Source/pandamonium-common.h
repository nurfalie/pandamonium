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

#ifndef _pandamonium_common_h_
#define _pandamonium_common_h_

extern "C"
{
#include <signal.h>
}

#include <QDir>
#include <QNetworkProxy>
#include <QSettings>

#ifdef Q_OS_MAC
#if QT_VERSION >= 0x050000
#include "CocoaInitializer.h"
#endif
#endif

#define pandamonium_VERSION_STR "2015.10.22"

class pandamonium_common
{
 public:
  static QNetworkProxy proxy(void)
  {
    QNetworkProxy proxy;
    QSettings settings;
    int index = settings.value("pandamonium_proxy_type", -1).toInt();

    proxy.setHostName(settings.value("pandamonium_proxy_address").toString());
    proxy.setPassword
      (settings.value("pandamonium_proxy_password").toString());
    proxy.setPort
      (static_cast<quint16> (settings.value("pandamonium_proxy_port").
			     toInt()));

    if(index == 0)
      proxy.setType(QNetworkProxy::HttpProxy);
    else if(index == 1)
      proxy.setType(QNetworkProxy::Socks5Proxy);
    else
      proxy.setType(QNetworkProxy::NoProxy);

    proxy.setUser(settings.value("pandamonium_proxy_user").toString());
    return proxy;
  }

  static QString homePath(void)
  {
    QByteArray homepath(qgetenv("PANDAMONIUM_HOME"));

    if(homepath.isEmpty())
#ifdef Q_OS_WIN32
      return QDir::currentPath() + QDir::separator() + ".pandamonium";
#else
      return QDir::homePath() + QDir::separator() + ".pandamonium";
#endif
      else
	{
	  QFileInfo fileInfo
	    (homepath.mid(0, pandamonium_home_maximum_length));

	  if(!fileInfo.isReadable() || !fileInfo.isWritable())
	    {
	      qDebug() << "Assigning " << QDir::tempPath()
		       << " as pandamonium's home!";
	      return QDir::tempPath();
	    }
	  else
	    return fileInfo.absoluteFilePath();
	}
  }

  static void prepareSignalHandler(void (*signal_handler) (int))
  {
    QList<int> list;
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC) || defined(Q_OS_UNIX)
    struct sigaction act;
#endif
    list << SIGABRT
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC) || defined(Q_OS_UNIX)
	 << SIGBUS
#endif
	 << SIGFPE
	 << SIGILL
	 << SIGINT
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC) || defined(Q_OS_UNIX)
	 << SIGQUIT
#endif
	 << SIGSEGV
	 << SIGTERM;

    while(!list.isEmpty())
      {
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC) || defined(Q_OS_UNIX)
	act.sa_handler = signal_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(list.takeFirst(), &act, 0);
#else
	signal(list.takeFirst(), signal_handler);
#endif
      }
  }

  static const int pandamonium_home_maximum_length = 256;
  static const qint64 maximum_database_size = static_cast<qint64>
    (2147483648LL);

 private:
  pandamonium_common(void)
  {
  }
};

#endif
