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
#include <QSqlRecord>
#include <QtCore/qmath.h>
#include <QtDebug>

#include "pandemonium-common.h"
#include "pandemonium-database.h"
#include "pandemonium-gui.h"

pandemonium_gui::pandemonium_gui(void):QMainWindow()
{
  QDir().mkdir(pandemonium_common::homePath());
  m_parsedLinksLastDateTime = 0;
  m_statisticsMainWindow.setWindowFlags
    (windowFlags() | Qt::WindowStaysOnTopHint);
  m_ui.setupUi(this);
  m_uiStatistics.setupUi(&m_statisticsMainWindow);
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
	  SLOT(slotQuit(void)));
  connect(m_ui.action_Statistics_Window,
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotShowStatisticsWindow(void)));
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
  connect(m_ui.list_parsed_urls,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotListParsedUrls(void)));
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
  connect(m_ui.periodically_list_parsed_urls,
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
  connect(m_ui.purge_unvisited_visited_urls,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemoveUnvisitedVisitedUrls(void)));
  connect(m_ui.remove_all_parsed_urls,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemoveAllParsedUrls(void)));
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
	  SLOT(slotToggleParsed(void)));
  connect(m_ui.toggle_none,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotToggleParsed(void)));
  connect(m_uiStatistics.action_Close,
	  SIGNAL(triggered(void)),
	  &m_statisticsMainWindow,
	  SLOT(close(void)));
  m_highlightTimer.start(2500);
  m_kernelDatabaseTimer.start(2500);
  m_tableListTimer.setInterval(10000);
  m_ui.search_urls->setColumnHidden
    (m_ui.search_urls->columnCount() - 1, true); // url_hash

  QLocale locale;

  m_uiStatistics.database_limits->setText
    (tr("A limit of %1 bytes is imposed on "
	"pandemonium_parsed_urls.db and pandemonium_visited_urls.db.").
     arg(locale.toString(pandemonium_common::maximum_database_size)));

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

  m_ui.periodically_list_parsed_urls->setChecked
    (settings.value("pandemonium_periodically_list_parsed_urls").
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
  m_statisticsMainWindow.show();
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

void pandemonium_gui::populateParsed(void)
{
  QApplication::setOverrideCursor(Qt::BusyCursor);
  m_ui.parsed_urls->clearContents();
  m_ui.parsed_urls->scrollToTop();
  m_ui.parsed_urls->setRowCount(0);

  QList<QUrl> list;
  int row = 0;
  quint64 limit = static_cast<quint64> (m_ui.page_limit->currentText().
					toInt());

  list = pandemonium_database::parsedLinks
    (limit, static_cast<quint64> (limit * m_ui.page->currentIndex()));
  m_ui.parsed_urls->setRowCount(list.size());

  while(!list.isEmpty())
    {
      QCheckBox *checkBox = new QCheckBox();
      QTableWidgetItem *item = new QTableWidgetItem
	(list.takeFirst().toString());

      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      m_ui.parsed_urls->setCellWidget(row, 0, checkBox);
      m_ui.parsed_urls->setItem(row, 1, item);
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
    (this, tr("pandemonium: URL"), tr("&URL"),
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

  /*
  ** Not for statistics.
  */

  m_uiStatistics.statistics->clearContents();
  m_uiStatistics.statistics->setRowCount(0);

  QList<qint64> values;
  QLocale locale;
  QPair<quint64, quint64> numbers
    (pandemonium_database::unvisitedAndVisitedNumbers());
  QStringList statistics;
  int percent = 100 *
    static_cast<double> (numbers.first) / (qMax(static_cast<quint64> (1),
						numbers.first +
						numbers.second));

  statistics << "Parsed URL(s)"
	     << "Percent Remaining"
	     << "Remaining URL(s)"
	     << "Total URL(s) Discovered";
  values << pandemonium_database::parsedLinksCount()
	 << static_cast<qint64> (percent)
	 << numbers.first
	 << numbers.first + numbers.second;
  m_uiStatistics.statistics->setRowCount(statistics.size());

  for(int i = 0; i < statistics.size(); i++)
    {
      QTableWidgetItem *item = 0;

      item = new QTableWidgetItem();
      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      item->setText(statistics.at(i));
      m_uiStatistics.statistics->setItem(i, 0, item);
      item = new QTableWidgetItem();
      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      item->setText(locale.toString(values.at(i)));
      m_uiStatistics.statistics->setItem(i, 1, item);
    }

  m_uiStatistics.statistics->resizeColumnToContents(0);

  QStringList list;

  list << (pandemonium_common::homePath() + QDir::separator() +
	   "pandemonium_parsed_urls.db")
       << (pandemonium_common::homePath() + QDir::separator() +
	   "pandemonium_visited_urls.db");

  for(int i = 0; i < list.size(); i++)
    {
      QFileInfo fileInfo(list.at(i));
      QString toolTip("");
      int percent = 0;

      percent = 
	100 * (static_cast<double> (fileInfo.size()) /
	       pandemonium_common::maximum_database_size);
      toolTip = tr("%1 of %2 bytes consumed.").
	arg(locale.toString(fileInfo.size())).
	arg(locale.toString(pandemonium_common::maximum_database_size));

      if(i == 0)
	{
	  m_uiStatistics.parsed_urls_capacity->setToolTip(toolTip);
	  m_uiStatistics.parsed_urls_capacity->setValue(percent);
	}
      else
	{
	  m_uiStatistics.visited_urls_capacity->setToolTip(toolTip);
	  m_uiStatistics.visited_urls_capacity->setValue(percent);
	}
    }
}

void pandemonium_gui::slotListParsedUrls(void)
{
  QApplication::setOverrideCursor(Qt::BusyCursor);

  QPair<quint64, quint64> numbers
    (pandemonium_database::unvisitedAndVisitedNumbers());
  int i = 1;

  m_ui.page->clear();

  do
    {
      m_ui.page->addItem(tr("Page %1").arg(i));

      if(i > qCeil(numbers.second / static_cast<quint64> (m_ui.
							  page_limit->
							  currentText().
							  toInt())))
	break;

      i += 1;
    }
  while(true);

  QApplication::restoreOverrideCursor();
  populateParsed();
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

	if(query.exec("SELECT meta_data_only, paused, "
		      "search_depth, url, url_hash "
		      "FROM pandemonium_search_urls "
		      "ORDER BY url"))
	  while(query.next())
	    {
	      m_ui.search_urls->setRowCount(row + 1);

	      for(int i = 0; i < query.record().count(); i++)
		{
		  if(i == 0)
		    {
		      QCheckBox *checkBox = new QCheckBox();

		      checkBox->setChecked(query.value(i).toInt());
		      checkBox->setProperty
			("url_hash", query.value(query.record().count() - 1));
		      checkBox->setToolTip
			(tr("If enabled, only meta-data words will "
			    "be saved. "
			    "Otherwise, all site words will be saved."));
		      connect(checkBox,
			      SIGNAL(toggled(bool)),
			      this,
			      SLOT(slotMetaDataOnly(bool)));
		      m_ui.search_urls->setCellWidget(row, i, checkBox);
		    }
		  else if(i == 1)
		    {
		      QCheckBox *checkBox = new QCheckBox();

		      checkBox->setChecked(query.value(i).toInt());
		      checkBox->setProperty
			("url_hash", query.value(query.record().count() - 1));
		      connect(checkBox,
			      SIGNAL(toggled(bool)),
			      this,
			      SLOT(slotPause(bool)));
		      m_ui.search_urls->setCellWidget(row, i, checkBox);
		    }
		  else if(i == 2)
		    {
		      QComboBox *comboBox = new QComboBox();
		      int index = 0;

		      comboBox->addItem("-1");
		      comboBox->setProperty
			("url_hash", query.value(query.record().count() - 1));
		      index = comboBox->findText(query.value(i).toString());

		      if(index >= 0)
			comboBox->setCurrentIndex(index);

		      connect(comboBox,
			      SIGNAL(currentIndexChanged(const QString &)),
			      this,
			      SLOT(slotDepthChanged(const QString &)));
		      m_ui.search_urls->setCellWidget(row, i, comboBox);
		    }
		  else
		    {
		      QTableWidgetItem *item = new QTableWidgetItem
			(query.value(i).toString());

		      item->setFlags
			(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		      m_ui.search_urls->setItem(row, i, item);

		      if(i == query.record().count() - 1)
			if(selected.contains(item->text()))
			  {
			    QModelIndex index
			      (m_ui.search_urls->model()->index(row, 0));

			    m_ui.search_urls->selectionModel()->select
			      (index,
			       QItemSelectionModel::Rows |
			       QItemSelectionModel::SelectCurrent);
			  }
		    }
		}

	      row += 1;
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
  populateParsed();
}

void pandemonium_gui::slotPause(bool state)
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
		      "SET paused = ? "
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

void pandemonium_gui::slotQuit(void)
{
  QApplication::instance()->quit();
}

void pandemonium_gui::slotRemoveAllParsedUrls(void)
{
  QFile::remove(pandemonium_common::homePath() + QDir::separator() +
		"pandemonium_parsed_urls.db");
  slotListParsedUrls();
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

void pandemonium_gui::slotRemoveUnvisitedVisitedUrls(void)
{
  QFile::remove(pandemonium_common::homePath() + QDir::separator() +
		"pandemonium_visited_urls.db");
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

  if(m_ui.periodically_list_parsed_urls ==
     qobject_cast<QCheckBox *> (sender()))
    {
      if(state)
	{
	  m_parsedLinksLastDateTime = 0;

	  if(!m_tableListTimer.isActive())
	    m_tableListTimer.start();
	}
      else
	m_tableListTimer.stop();

      str = "pandemonium_periodically_list_parsed_urls";
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

void pandemonium_gui::slotShowStatisticsWindow(void)
{
  m_statisticsMainWindow.show();
}

void pandemonium_gui::slotTableListTimeout(void)
{
  QFileInfo fileInfo
    (pandemonium_common::homePath() + QDir::separator() +
     "pandemonium_parsed_urls.db");

  if(fileInfo.exists())
    {
      if(fileInfo.lastModified().toTime_t() > m_parsedLinksLastDateTime)
	m_parsedLinksLastDateTime = fileInfo.lastModified().toTime_t();
      else
	return;
    }
  else
    m_parsedLinksLastDateTime = 0;

  if(m_ui.tab_widget->currentIndex() == 0)
    {
      if(m_ui.periodically_list_search_urls->isChecked())
	slotListSearchUrls();
    }
  else
    {
      if(m_ui.periodically_list_parsed_urls->isChecked())
	slotListParsedUrls();
    }
}

void pandemonium_gui::slotToggleParsed(void)
{
  m_tableListTimer.stop();
  m_ui.periodically_list_parsed_urls->setChecked(false);

  QPushButton *pushButton = qobject_cast<QPushButton *> (sender());
  bool state = true;

  if(m_ui.toggle_all == pushButton)
    state = true;
  else
    state = false;

  for(int i = 0; i < m_ui.parsed_urls->rowCount(); i++)
    {
      QCheckBox *checkBox = qobject_cast<QCheckBox *>
	(m_ui.parsed_urls->cellWidget(i, 0));

      if(checkBox)
	checkBox->setChecked(state);
    }
}
