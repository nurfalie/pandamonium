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
#include <QSettings>

#include "pandemonium.h"

pandemonium::pandemonium(void):QMainWindow()
{
  QDir().mkdir(homePath());
  m_ui.setupUi(this);
  connect(m_ui.action_Quit,
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(close(void)));

  QSettings settings;

  restoreGeometry(settings.value("pandemonium_mainwindow").toByteArray());
  show();
}

pandemonium::~pandemonium()
{
}

QString pandemonium::homePath(void)
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

void pandemonium::closeEvent(QCloseEvent *event)
{
  QSettings settings;

  settings.setValue("pandemonium_mainwindow", saveGeometry());
  QMainWindow::closeEvent(event);
}
