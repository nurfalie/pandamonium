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
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QProcess>
#include <QSettings>
#include <QSqlQuery>
#include <QtDebug>
#include <QtMath>

#include "pandemonium-common.h"
#include "pandemonium-database.h"
#include "pandemonium-gui.h"

pandemonium_gui::pandemonium_gui(void):QMainWindow()
{
  QDir().mkdir(pandemonium_common::homePath());
  m_discoveredLinksLastDateTime = 0;
  m_ui.setupUi(this);
  connect(&m_highlightTimer,
	  SIGNAL(timeout(void)),
	  this,
	  SLOT(slotHighlightTimeout(void)));
  connect(&m_kernelDatabaseTimer,
	  SIGNAL(timeout(void)),
	  this,
	  SLOT(slotKernelDatabaseTimeout(void)));
  connect(&m_tableListTimer,
	  SIGNAL(timeout(void)),
	  this,
	  SLOT(slotTableListTimeout(void)));
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
  connect(m_ui.kernel_load_interval,
	  SIGNAL(valueChanged(const QString &)),
	  this,
	  SLOT(slotLoadIntervalChanged(const QString &)));
  connect(m_ui.kernel_path,
	  SIGNAL(returnPressed(void)),
	  this,
	  SLOT(slotSaveKernelPath(void)));
  connect(m_ui.list_discovered_urls,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotListDiscoveredUrls(void)));
  connect(m_ui.list_search_urls,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotListSearchUrls(void)));
  connect(m_ui.monitor_kernel,
	  SIGNAL(toggled(bool)),
	  this,
	  SLOT(slotMonitorKernel(bool)));
  connect(m_ui.page,
	  SIGNAL(currentIndexChanged(int)),
	  this,
	  SLOT(slotPageChanged(int)));
  connect(m_ui.page_limit,
	  SIGNAL(currentIndexChanged(const QString &)),
	  this,
	  SLOT(slotSavePageLimit(const QString &)));
  connect(m_ui.periodically_list_discovered_urls,
	  SIGNAL(toggled(bool)),
	  this,
	  SLOT(slotSavePeriodic(bool)));
  connect(m_ui.periodically_list_search_urls,
	  SIGNAL(toggled(bool)),
	  this,
	  SLOT(slotSavePeriodic(bool)));
  connect(m_ui.proxy_information,
	  SIGNAL(toggled(bool)),
	  this,
	  SLOT(slotProxyInformationToggled(bool)));
  connect(m_ui.remove_all_discovered_urls,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemoveAllDiscovered(void)));
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
  connect(m_ui.toggle_all,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotToggleDiscovered(void)));
  connect(m_ui.toggle_none,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotToggleDiscovered(void)));
  m_highlightTimer.start(2500);
  m_kernelDatabaseTimer.start(2500);
  m_tableListTimer.setInterval(10000);
  m_ui.search_urls->setColumnHidden
    (m_ui.search_urls->columnCount() - 1, true); // url_hash

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
  ** Restore interface settings.
  */

  int index = m_ui.page_limit->findText
    (settings.value("pandemonium_page_limit", "1000").toString());

  if(index >= 0)
    m_ui.page_limit->setCurrentIndex(index);

  m_ui.periodically_list_discovered_urls->setChecked
    (settings.value("pandemonium_periodically_list_discovered_urls").
     toBool());
  m_ui.periodically_list_search_urls->setChecked
    (settings.value("pandemonium_periodically_list_search_urls").toBool());

  /*
  ** Restore kernel settings.
  */

  m_ui.kernel_load_interval->setValue
    (settings.value("pandemonium_kernel_load_interval").toDouble());
  m_ui.kernel_path->setText
    (settings.value("pandemonium_kernel_path").toString());
  m_ui.monitor_kernel->setChecked
    (settings.value("pandemonium_monitor_kernel").toBool());

  /*
  ** Restore proxy settings.
  */

  m_ui.proxy_address->setText(settings.value("pandemonium_proxy_address").
			      toString().trimmed());
  m_ui.proxy_password->setText(settings.value("pandemonium_proxy_password").
			       toString());
  m_ui.proxy_port->setValue(settings.value("pandemonium_proxy_port").toInt());
  index = settings.value("pandemonium_proxy_type").toInt();

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

  QApplication::setOverrideCursor(Qt::BusyCursor);
  slotListSearchUrls();
  QApplication::restoreOverrideCursor();
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

void pandemonium_gui::populateDiscovered(void)
{
  QApplication::setOverrideCursor(Qt::BusyCursor);
  m_ui.discovered_urls->clearContents();
  m_ui.discovered_urls->scrollToTop();
  m_ui.discovered_urls->setRowCount(0);

  QList<QUrl> list;
  int row = 0;
  quint64 limit = static_cast<quint64> (m_ui.page_limit->currentText().
					toInt());

  list = pandemonium_database::
    visitedLinks(limit,
		 static_cast<quint64> (limit * m_ui.page->currentIndex()));
  m_ui.discovered_urls->setRowCount(list.size());

  while(!list.isEmpty())
    {
      QCheckBox *checkBox = new QCheckBox();
      QTableWidgetItem *item = new QTableWidgetItem
	(list.takeFirst().toString());

      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      m_ui.discovered_urls->setCellWidget(row, 0, checkBox);
      m_ui.discovered_urls->setItem(row, 1, item);
      row += 1;
    }

  QApplication::restoreOverrideCursor();
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
      QList<QString> list;

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
  m_ui.monitor_kernel->setChecked(false);
  pandemonium_database::recordKernelDeactivation();
}

void pandemonium_gui::slotDepthChanged(const QString &text)
{
  QComboBox *comboBox = qobject_cast<QComboBox *> (sender());

  if(!comboBox)
    return;

  pandemonium_database::saveDepth(text, comboBox->property("url_hash"));
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

  if(m_ui.monitor_kernel->isChecked())
    slotActivateKernel();

  m_ui.kernel_pid->setText
    (QString::number(pandemonium_database::kernelProcessId()));

  QPair<quint64, quint64> numbers
    (pandemonium_database::unvisitedAndVisitedNumbers());

  m_ui.discovered_statistics->setText
    (tr("<b>Total URLs:</b> %1. "
	"<b>Unvisited URLs:</b> %2. "
	"<b>Visited URLs:</b> %3. "
	"<b>Percent unvisited:</b> %4%.").
     arg(numbers.first + numbers.second).
     arg(numbers.first).arg(numbers.second).
     arg(QString::
	 number(100 *
		static_cast<double> (numbers.first)
		/ (qMax(static_cast<quint64> (1),
			numbers.first +
			numbers.second)), 'f', 2)));
}

void pandemonium_gui::slotListDiscoveredUrls(void)
{
  QApplication::setOverrideCursor(Qt::BusyCursor);

  QPair<quint64, quint64> numbers
    (pandemonium_database::unvisitedAndVisitedNumbers());
  int i = 1;

  m_ui.page->clear();

  do
    {
      m_ui.page->addItem(tr("Page %1").arg(i));

      if(i > qCeil(numbers.first / static_cast<quint64> (m_ui.
							 page_limit->
							 currentText().
							 toInt())))
	break;

      i += 1;
    }
  while(true);

  QApplication::restoreOverrideCursor();
  populateDiscovered();
}

void pandemonium_gui::slotListSearchUrls(void)
{
  QApplication::setOverrideCursor(Qt::BusyCursor);

  QModelIndexList list
    (m_ui.search_urls->selectionModel()->selectedRows(m_ui.search_urls->
						      columnCount() - 1));
  QStringList selected;

  while(!list.isEmpty())
    selected << list.takeFirst().data().toString();

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

	if(query.exec("SELECT meta_data_only, search_depth, url, url_hash "
		      "FROM pandemonium_search_urls "
		      "ORDER BY url"))
	  while(query.next())
	    {
	      QCheckBox *checkBox = new QCheckBox();
	      QComboBox *comboBox = new QComboBox();
	      QTableWidgetItem *item = 0;
	      int index = 0;

	      checkBox->setChecked(query.value(0).toInt());
	      checkBox->setProperty("url_hash", query.value(3));
	      checkBox->setToolTip
		(tr("If enabled, only meta-data words will be saved. "
		    "Otherwise, all site words will be saved."));
	      connect(checkBox,
		      SIGNAL(toggled(bool)),
		      this,
		      SLOT(slotMetaDataOnly(bool)));
	      comboBox->addItem("-1");
	      comboBox->setProperty("url_hash", query.value(3));
	      index = comboBox->findText(query.value(1).toString());

	      if(index >= 0)
		comboBox->setCurrentIndex(index);

	      m_ui.search_urls->setRowCount(row + 1);
	      m_ui.search_urls->setCellWidget(row, 0, checkBox);
	      m_ui.search_urls->setCellWidget(row, 1, comboBox);
	      item = new QTableWidgetItem(query.value(2).toString());
	      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	      m_ui.search_urls->setItem(row, 2, item);
	      item = new QTableWidgetItem(query.value(3).toString());
	      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	      m_ui.search_urls->setItem(row, 3, item);

	      if(selected.contains(item->text()))
		{
		  QModelIndex index
		    (m_ui.search_urls->model()->index(row, 0));

		  m_ui.search_urls->selectionModel()->select
		    (index,
		     QItemSelectionModel::Rows |
		     QItemSelectionModel::SelectCurrent);
		}

	      row += 1;
	      connect(comboBox,
		      SIGNAL(currentIndexChanged(const QString &)),
		      this,
		      SLOT(slotDepthChanged(const QString &)));
	    }

	m_ui.search_urls->resizeColumnToContents(0);
	m_ui.search_urls->resizeColumnToContents(1);
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  QApplication::restoreOverrideCursor();
}

void pandemonium_gui::slotLoadIntervalChanged(const QString &text)
{
  QSettings settings;

  settings.setValue("pandemonium_kernel_load_interval", text);
}

void pandemonium_gui::slotMetaDataOnly(bool state)
{
  QCheckBox *checkBox = qobject_cast<QCheckBox *> (sender());

  if(!checkBox)
    return;

  QPair<QSqlDatabase, QString> pair;

  {
    pair = pandemonium_database::database();
    pair.first.setDatabaseName
      (pandemonium_common::homePath() + QDir::separator() +
       "pandemonium_search_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.prepare("UPDATE pandemonium_search_urls "
		      "SET meta_data_only = ? "
		      "WHERE url_hash = ?");
	query.bindValue(0, state ? 1 : 0);
	query.bindValue(1, checkBox->property("url_hash"));
	query.exec();
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
}

void pandemonium_gui::slotMonitorKernel(bool state)
{
  QSettings settings;

  settings.setValue("pandemonium_monitor_kernel", state);
}

void pandemonium_gui::slotPageChanged(int index)
{
  Q_UNUSED(index);
  m_ui.page->repaint();
  populateDiscovered();
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

void pandemonium_gui::slotRemoveAllDiscovered(void)
{
  QFile::remove(pandemonium_common::homePath() + QDir::separator() +
		"pandemonium_discovered_urls.db");
  slotListDiscoveredUrls();
}

void pandemonium_gui::slotRemoveSelectedSearchUrls(void)
{
  QList<QString> list;
  QModelIndexList indexes
    (m_ui.search_urls->selectionModel()->
     selectedRows(m_ui.search_urls->columnCount() - 1));

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

void pandemonium_gui::slotSavePageLimit(const QString &text)
{
  QSettings settings;

  settings.setValue("pandemonium_page_limit", text);
}

void pandemonium_gui::slotSavePeriodic(bool state)
{
  QSettings settings;
  QString str("");

  if(m_ui.periodically_list_discovered_urls ==
     qobject_cast<QCheckBox *> (sender()))
    {
      if(state)
	{
	  m_discoveredLinksLastDateTime = 0;

	  if(!m_tableListTimer.isActive())
	    m_tableListTimer.start();
	}
      else
	m_tableListTimer.stop();

      str = "pandemonium_periodically_list_discovered_urls";
    }
  else
    str = "pandemonium_periodically_list_search_urls";

  settings.setValue(str, state);
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

void pandemonium_gui::slotTableListTimeout(void)
{
  QFileInfo fileInfo
    (pandemonium_common::homePath() + QDir::separator() +
     "pandemonium_discovered_urls.db");

  if(fileInfo.exists())
    {
      if(fileInfo.lastModified().toTime_t() > m_discoveredLinksLastDateTime)
	m_discoveredLinksLastDateTime = fileInfo.lastModified().toTime_t();
      else
	return;
    }
  else
    m_discoveredLinksLastDateTime = 0;

  if(m_ui.tab_widget->currentIndex() == 0)
    {
      if(m_ui.periodically_list_search_urls->isChecked())
	slotListSearchUrls();
    }
  else
    {
      if(m_ui.periodically_list_discovered_urls->isChecked())
	slotListDiscoveredUrls();
    }
}

void pandemonium_gui::slotToggleDiscovered(void)
{
  m_tableListTimer.stop();
  m_ui.periodically_list_discovered_urls->setChecked(false);

  QPushButton *pushButton = qobject_cast<QPushButton *> (sender());
  bool state = true;

  if(m_ui.toggle_all == pushButton)
    state = true;
  else
    state = false;

  for(int i = 0; i < m_ui.discovered_urls->rowCount(); i++)
    {
      QCheckBox *checkBox = qobject_cast<QCheckBox *>
	(m_ui.discovered_urls->cellWidget(i, 0));

      if(checkBox)
	checkBox->setChecked(state);
    }
}
