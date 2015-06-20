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
#include <QFileDialog>
#include <QSettings>
#include <QtDebug>

#include "pandemonium.h"
#include "pandemonium_createdb.h"

pandemonium::pandemonium(void):QMainWindow()
{
  QDir().mkdir(homePath());
  m_ui.setupUi(this);
  connect(m_ui.action_Quit,
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(close(void)));
  connect(m_ui.kernel_path,
	  SIGNAL(returnPressed(void)),
	  this,
	  SLOT(slotSaveKernelPath(void)));
  connect(m_ui.proxy_information,
	  SIGNAL(toggled(bool)),
	  this,
	  SLOT(slotProxyInformationToggled(bool)));
  connect(m_ui.save_proxy_information,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSaveProxyInformation(void)));
  connect(m_ui.select_kernel_path,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSelectKernelPath(void)));

  QSettings settings;

  restoreGeometry(settings.value("pandemonium_mainwindow").toByteArray());
  show();
  statusBar()->showMessage(tr("Creating databases..."));
  statusBar()->repaint();
  QApplication::setOverrideCursor(Qt::BusyCursor);
  pandemonium_createdb::createdb();
  QApplication::restoreOverrideCursor();
  statusBar()->clearMessage();

  /*
  ** Restore kernel path.
  */

  m_ui.kernel_path->setText
    (settings.value("pandemonium_kernel_path").toString());

  /*
  ** Restore proxy settings.
  */

  m_ui.proxy_address->setText(settings.value("pandemonium_proxy_address").
			      toString().trimmed());
  m_ui.proxy_password->setText(settings.value("pandemonium_proxy_password").
			       toString());
  m_ui.proxy_port->setValue(settings.value("pandemonium_proxy_port").toInt());

  int index = settings.value("pandemonium_proxy_type").toInt();

  if(index < 0 || index > 1)
    index = 0;

  m_ui.proxy_type->setCurrentIndex(index);
  m_ui.proxy_user->setText
    (settings.value("pandemonium_proxy_user").toString());

  foreach(QString key, settings.allKeys())
    if(key.startsWith("pandemonium_proxy"))
      {
	m_ui.proxy_information->setChecked(true);
	break;
      }
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

void pandemonium::saveKernelPath(const QString &path)
{
  QSettings settings;

  settings.setValue("pandemonium_kernel_path", path);
  m_ui.kernel_path->selectAll();
}

void pandemonium::slotProxyInformationToggled(bool state)
{
  if(!state)
    {
      m_ui.proxy_address->clear();
      m_ui.proxy_password->clear();
      m_ui.proxy_port->setValue(m_ui.proxy_port->minimum());
      m_ui.proxy_type->setCurrentIndex(0);
      m_ui.proxy_user->clear();

      QSettings settings;

      settings.remove("pandemonium_proxy_address");
      settings.remove("pandemonium_proxy_password");
      settings.remove("pandemonium_proxy_port");
      settings.remove("pandemonium_proxy_type");
      settings.remove("pandemonium_proxy_user");
    }
}

void pandemonium::slotSaveKernelPath(void)
{
  saveKernelPath(m_ui.kernel_path->text());
}

void pandemonium::slotSaveProxyInformation(void)
{
  QSettings settings;

  settings.setValue
    ("pandemonium_proxy_address", m_ui.proxy_address->text().trimmed());
  settings.setValue
    ("pandemonium_proxy_password", m_ui.proxy_password->text());
  settings.setValue
    ("pandemonium_proxy_port", m_ui.proxy_port->value());
  settings.setValue
    ("pandemonium_proxy_type", m_ui.proxy_type->currentIndex());
  settings.setValue
    ("pandemonium_proxy_user", m_ui.proxy_user->text());
}

void pandemonium::slotSelectKernelPath(void)
{
  QFileDialog dialog;

  dialog.setWindowTitle(tr("pandemonium: Select Kernel Path"));
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setDirectory(QDir::homePath());
  dialog.setLabelText(QFileDialog::Accept, tr("&Select"));
  dialog.setAcceptMode(QFileDialog::AcceptOpen);
#ifdef Q_OS_MAC
#if QT_VERSION < 0x050000
  dialog.setAttribute(Qt::WA_MacMetalStyle, false);
#endif
#endif

  if(dialog.exec() == QDialog::Accepted)
    {
      m_ui.kernel_path->setText(dialog.selectedFiles().value(0));
      saveKernelPath(dialog.selectedFiles().value(0));
    }
}
