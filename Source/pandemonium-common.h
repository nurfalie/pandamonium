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

#ifndef _pandemonium_common_h_
#define _pandemonium_common_h_

extern "C"
{
#include <signal.h>
}

#include <QDir>

#define PANDEMONIUM_VERSION_STR "2015.07.04"

class pandemonium_common
{
 public:
  static QString homePath(void)
  {
    QByteArray homepath(qgetenv("PANDEMONIUM_HOME"));

    if(homepath.isEmpty())
#ifdef Q_OS_WIN32
      return QDir::currentPath() + QDir::separator() + ".pandemonium";
#else
      return QDir::homePath() + QDir::separator() + ".pandemonium";
#endif
    else
      return homepath.constData();
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

 private:
  pandemonium_common(void)
  {
  }
};

#endif