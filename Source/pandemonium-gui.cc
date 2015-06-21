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

#include <QComboBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QProcess>
#include <QSettings>
#include <QSqlQuery>
#include <QtDebug>

#include "pandemonium-common.h"
#include "pandemonium-database.h"
#include "pandemonium-gui.h"

pandemonium_gui::pandemonium_gui(void):QMainWindow()
{
  QDir().mkdir(pandemonium_common::homePath());
  m_ui.setupUi(this);
  connect(&m_highlightTimer,
	  SIGNAL(timeout(void)),
	  this,
	  SLOT(slotHighlightTimeout(void)));
  connect(&m_kernelDatabaseTimer,
	  SIGNAL(timeout(void)),
	  this,
	  SLOT(slotKernelDatabaseTimeout(void)));
  connect(m_ui.action_Quit,
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(close(void)));
  connect(m_ui.activate_kernel,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotActivateKernel(void)));
  connect(m_ui.add_search_url,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotAddSearchUrl(void)));
  connect(m_ui.deactivate_kernel,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotDeactivateKernel(void)));
  connect(m_ui.kernel_path,
	  SIGNAL(returnPressed(void)),
	  this,
	  SLOT(slotSaveKernelPath(void)));
  connect(m_ui.list_search_urls,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotListSearchUrls(void)));
  connect(m_ui.proxy_information,
	  SIGNAL(toggled(bool)),
	  this,
	  SLOT(slotProxyInformationToggled(bool)));
  connect(m_ui.remove_search_urls,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemoveSelectedSearchUrls(void)));
  connect(m_ui.save_proxy_information,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSaveProxyInformation(void)));
  connect(m_ui.select_kernel_path,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSelectKernelPath(void)));
  m_highlightTimer.start(2500);
  m_kernelDatabaseTimer.start(2500);
  m_ui.search_urls->setColumnHidden(2, true); // url_hash

  QSettings settings;

  restoreGeometry(settings.value("pandemonium_mainwindow").toByteArray());
  show();
  statusBar()->showMessage(tr("Creating databases..."));
  statusBar()->repaint();
  QApplication::setOverrideCursor(Qt::BusyCursor);
  pandemonium_database::createdb();
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

pandemonium_gui::~pandemonium_gui()
{
}

void pandemonium_gui::closeEvent(QCloseEvent *event)
{
  QSettings settings;

  settings.setValue("pandemonium_mainwindow", saveGeometry());
  QMainWindow::closeEvent(event);
}

void pandemonium_gui::saveKernelPath(const QString &path)
{
  QSettings settings;

  settings.setValue("pandemonium_kernel_path", path);
  m_ui.kernel_path->selectAll();
}

void pandemonium_gui::slotActivateKernel(void)
{
  QString program(m_ui.kernel_path->text());

#ifdef Q_OS_MAC
  if(QFileInfo(program).isBundle())
    {
      QStringList list;

      list << "-a" << program;

      QProcess::startDetached("open", list);
    }
  else
    QProcess::startDetached(program);
#elif defined(Q_OS_WIN32)
  QProcess::startDetached(QString("\"%1\"").arg(program));
#else
  QProcess::startDetached(program);
#endif
}

void pandemonium_gui::slotAddSearchUrl(void)
{
  QString str("");
  bool ok = true;

  str = QInputDialog::getText
    (this, tr("pandemonium: New Search URL"), tr("&URL"),
     QLineEdit::Normal, QString(""), &ok);

  if(!ok)
    return;

  pandemonium_database::addSearchUrl(str);
  slotListSearchUrls();
}

void pandemonium_gui::slotDeactivateKernel(void)
{
  pandemonium_database::recordKernelDeactivation();
}

void pandemonium_gui::slotHighlightTimeout(void)
{
  QColor color;
  QFileInfo fileInfo;
  QPalette palette;

  fileInfo.setFile(m_ui.kernel_path->text());

  if(fileInfo.isExecutable() && fileInfo.size() > 0)
    color = QColor(144, 238, 144);
  else
    color = QColor(240, 128, 128); // Light coral!

  palette.setColor(m_ui.kernel_path->backgroundRole(), color);
  m_ui.kernel_path->setPalette(palette);

  if(m_ui.kernel_pid->text().toLongLong() > 0)
    color = QColor(144, 238, 144);
  else
    color = QColor(240, 128, 128); // Light coral!

  palette.setColor(m_ui.kernel_pid->backgroundRole(), color);
  m_ui.kernel_pid->setPalette(palette);
}

void pandemonium_gui::slotKernelDatabaseTimeout(void)
{
  pandemonium_database::createdb();
  m_ui.kernel_pid->setText
    (QString::number(pandemonium_database::kernelProcessId()));
}

void pandemonium_gui::slotListSearchUrls(void)
{
  QApplication::setOverrideCursor(Qt::BusyCursor);
  m_ui.search_urls->clearContents();
  m_ui.search_urls->setRowCount(0);

  QPair<QSqlDatabase, QString> pair;

  {
    pair = pandemonium_database::database();
    pair.first.setDatabaseName
      (pandemonium_common::homePath() + QDir::separator() +
       "pandemonium_search_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);
	int row = 0;

	query.setForwardOnly(true);

	if(query.exec("SELECT search_depth, url, url_hash "
		      "FROM pandemonium_search_urls "
		      "ORDER BY url"))
	  while(query.next())
	    {
	      QComboBox *comboBox = new QComboBox();
	      QTableWidgetItem *item = 0;
	      int index = 0;

	      comboBox->addItem("5");
	      comboBox->addItem("10");
	      comboBox->addItem("15");
	      index = comboBox->findText(query.value(0).toString());

	      if(index >= 0)
		comboBox->setCurrentIndex(index);

	      m_ui.search_urls->setRowCount(row + 1);
	      m_ui.search_urls->setCellWidget(row, 0, comboBox);
	      item = new QTableWidgetItem(query.value(1).toString());
	      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	      m_ui.search_urls->setItem(row, 1, item);
	      item = new QTableWidgetItem(query.value(2).toString());
	      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	      m_ui.search_urls->setItem(row, 2, item);
	      row += 1;
	    }
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  QApplication::restoreOverrideCursor();
}

void pandemonium_gui::slotProxyInformationToggled(bool state)
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

void pandemonium_gui::slotRemoveSelectedSearchUrls(void)
{
  QModelIndexList indexes
    (m_ui.search_urls->selectionModel()->selectedRows(2));
  QStringList list;

  while(!indexes.isEmpty())
    {
      QModelIndex index(indexes.takeFirst());

      if(index.isValid())
	list << index.data().toString();
    }

  if(!list.isEmpty())
    {
      pandemonium_database::removeSearchUrls(list);
      slotListSearchUrls();
    }
}

void pandemonium_gui::slotSaveKernelPath(void)
{
  saveKernelPath(m_ui.kernel_path->text());
}

void pandemonium_gui::slotSaveProxyInformation(void)
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

void pandemonium_gui::slotSelectKernelPath(void)
{
  QFileDialog dialog(this);

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
