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

#include <QComboBox>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLCDNumber>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QSettings>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QtCore/qmath.h>
#include <QtDebug>

#include <limits>

#include "pandamonium-common.h"
#include "pandamonium-database.h"
#include "pandamonium-gui.h"

pandamonium_gui::pandamonium_gui(void):QMainWindow()
{
  QDir().mkdir(pandamonium_common::homePath());
  m_parsedLinksLastDateTime = 0;
  m_brokenLinksWindow = new QMainWindow(this);
  m_exportMainWindow = new QMainWindow(this);
  m_statisticsMainWindow = new QMainWindow(this);
  m_statisticsMainWindow->setWindowFlags
    (windowFlags() | Qt::WindowStaysOnTopHint);
  m_ui.setupUi(this);
  m_sbWidget = new QWidget(this);
  m_sb.setupUi(m_sbWidget);
  statusBar()->addPermanentWidget(m_sbWidget, 100);
  statusBar()->setStyleSheet("QStatusBar::item {"
			     "border: none; "
			     "}");
  statusBar()->setMaximumHeight(m_sbWidget->height());
  m_uiBrokenLinks.setupUi(m_brokenLinksWindow);
  m_uiExport.setupUi(m_exportMainWindow);
  m_uiStatistics.setupUi(m_statisticsMainWindow);
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
  connect(m_sb.kernel,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotKernelToolButtonClicked(void)));
  connect(m_ui.action_About,
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotAbout(void)));
  connect(m_ui.action_Broken_Links,
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotShowBrokenLinksWindow(void)));
  connect(m_ui.action_Export_Definition,
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotExportDefinition(void)));
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
  connect(m_ui.delete_exported_urls,
	  SIGNAL(toggled(bool)),
	  this,
	  SLOT(slotDeleteExportedUrlsCheckBoxClicked(bool)));
  connect(m_ui.export_toggled,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotExport(void)));
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
  connect(m_ui.remove_selected_parsed_urls,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemoveSelectedParsedUrls(void)));
  connect(m_ui.save_proxy_information,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSaveProxyInformation(void)));
  connect(m_ui.select_kernel_path,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSelectKernelPath(void)));
  connect(m_ui.tab_widget,
	  SIGNAL(currentChanged(int)),
	  this,
	  SLOT(slotTabIndexChanged(int)));
  connect(m_ui.toggle_all,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotToggleParsed(void)));
  connect(m_ui.toggle_none,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotToggleParsed(void)));
  connect(m_uiBrokenLinks.action_Close,
	  SIGNAL(triggered(void)),
	  m_brokenLinksWindow,
	  SLOT(close(void)));
  connect(m_uiBrokenLinks.page,
	  SIGNAL(currentIndexChanged(int)),
	  this,
	  SLOT(slotPageChanged(int)));
  connect(m_uiBrokenLinks.refresh,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRefreshBrokenUrls(void)));
  connect(m_uiBrokenLinks.remove_all,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemoveAllBrokenUrls(void)));
  connect(m_uiExport.action_Close,
	  SIGNAL(triggered(void)),
	  m_exportMainWindow,
	  SLOT(close(void)));
  connect(m_uiExport.save,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSaveExportDefinition(void)));
  connect(m_uiExport.select,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSelectExportDatabase(void)));
  connect(m_uiStatistics.action_Close,
	  SIGNAL(triggered(void)),
	  m_statisticsMainWindow,
	  SLOT(close(void)));
  m_highlightTimer.start(2500);
  m_kernelDatabaseTimer.start(2500);
  m_tableListTimer.setInterval(5000);
  m_ui.search_urls->setColumnHidden
    (m_ui.search_urls->columnCount() - 1, true); // url_hash

  QLocale locale;

  m_uiStatistics.database_limits->setText
    (tr("A limit of %1 bytes is imposed on "
	"pandamonium_parsed_urls.db and pandamonium_visited_urls.db.").
     arg(locale.toString(pandamonium_common::maximum_database_size)));

  QPixmap pixmap(":/pandamonium-logo.png");

  pixmap = pixmap.scaled
    (256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  m_uiStatistics.pixmap->setPixmap(pixmap);

  QSettings settings;

  restoreGeometry(settings.value("pandamonium_mainwindow").toByteArray());
  settings.remove("pandamonium_kernel_load_interval");
  show();
  statusBar()->showMessage(tr("Creating databases..."));
  statusBar()->repaint();
  QApplication::setOverrideCursor(Qt::BusyCursor);
  pandamonium_database::createdb();
  QApplication::restoreOverrideCursor();
  statusBar()->clearMessage();

  /*
  ** Restore interface settings.
  */

  m_ui.delete_exported_urls->setChecked
    (settings.value("pandamonium_delete_exported_urls").toBool());

  int index = m_ui.page_limit->findText
    (settings.value("pandamonium_page_limit", "1000").toString());

  if(index >= 0)
    m_ui.page_limit->setCurrentIndex(index);

  m_ui.periodically_list_parsed_urls->setChecked
    (settings.value("pandamonium_periodically_list_parsed_urls").
     toBool());
  m_ui.periodically_list_search_urls->setChecked
    (settings.value("pandamonium_periodically_list_search_urls").toBool());

  /*
  ** Restore kernel settings.
  */

  m_ui.kernel_path->setText
    (settings.value("pandamonium_kernel_path").toString());
  m_ui.monitor_kernel->setChecked
    (settings.value("pandamonium_monitor_kernel").toBool());

  /*
  ** Restore proxy settings.
  */

  m_ui.proxy_address->setText(settings.value("pandamonium_proxy_address").
			      toString().trimmed());
  m_ui.proxy_password->setText(settings.value("pandamonium_proxy_password").
			       toString());
  m_ui.proxy_port->setValue(settings.value("pandamonium_proxy_port").toInt());
  index = settings.value("pandamonium_proxy_type").toInt();

  if(index < 0 || index > 1)
    index = 0;

  m_ui.proxy_type->setCurrentIndex(index);
  m_ui.proxy_user->setText
    (settings.value("pandamonium_proxy_user").toString());

  foreach(QString key, settings.allKeys())
    if(key.startsWith("pandamonium_proxy"))
      {
	m_ui.proxy_information->setChecked(true);
	break;
      }

  QApplication::setOverrideCursor(Qt::BusyCursor);
  slotListSearchUrls();
  QApplication::restoreOverrideCursor();
}

pandamonium_gui::~pandamonium_gui()
{
}

bool pandamonium_gui::areYouSure(const QString &text)
{
  QMessageBox mb(this);

#ifdef Q_OS_MAC
#if QT_VERSION < 0x050000
  mb.setAttribute(Qt::WA_MacMetalStyle, true);
#endif
#endif
  mb.setIcon(QMessageBox::Question);
  mb.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
  mb.setText(text);
  mb.setWindowModality(Qt::WindowModal);
  mb.setWindowTitle(tr("pandamonium: Confirmation"));

  if(mb.exec() != QMessageBox::Yes)
    return false;
  else
    return true;
}

void pandamonium_gui::center(QWidget *child, QWidget *parent)
{
  if(!child || !parent)
    return;

  QPoint p(0, 0);
  int X = 0;
  int Y = 0;

  p = parent->pos();

  if(parent->width() >= child->width())
    X = p.x() + (parent->width() - child->width()) / 2;
  else
    X = p.x() - (child->width() - parent->width()) / 2;

  if(parent->height() >= child->height())
    Y = p.y() + (parent->height() - child->height()) / 2;
  else
    Y = p.y() - (child->height() - parent->height()) / 2;

  child->move(X, Y);
}

void pandamonium_gui::closeEvent(QCloseEvent *event)
{
  QSettings settings;

  settings.setValue("pandamonium_mainwindow", saveGeometry());
  QMainWindow::closeEvent(event);
}

void pandamonium_gui::populateBroken(void)
{
  QApplication::setOverrideCursor(Qt::BusyCursor);

  m_uiBrokenLinks.table->clearContents();
  m_uiBrokenLinks.table->setRowCount(0);

  QPair<QSqlDatabase, QString> pair;

  {
    pair = pandamonium_database::database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_broken_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);
	int row = 0;
	quint64 limit = static_cast<quint64> (m_uiBrokenLinks.
					      page_limit->currentText().
					      toInt());
	quint64 offset = static_cast<quint64> (limit * m_uiBrokenLinks.
					       page->currentIndex());

	m_uiBrokenLinks.table->setSortingEnabled(false);
	query.setForwardOnly(true);
	query.prepare
	  (QString("SELECT url_parent, url, error_string, url_hash "
		   "FROM pandamonium_broken_urls ORDER BY url_parent "
		   "LIMIT %1 OFFSET %2").arg(limit).arg(offset));

	if(query.exec())
	  while(query.next())
	    {
	      m_uiBrokenLinks.table->setRowCount(row + 1);

	      QTableWidgetItem *item = 0;

	      item = new QTableWidgetItem(query.value(0).toString());
	      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	      m_uiBrokenLinks.table->setItem(row, 0, item);
	      item = new QTableWidgetItem(query.value(1).toString());
	      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	      m_uiBrokenLinks.table->setItem(row, 1, item);
	      item = new QTableWidgetItem(query.value(2).toString());
	      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	      m_uiBrokenLinks.table->setItem(row, 2, item);
	      item = new QTableWidgetItem(query.value(3).toString());
	      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	      m_uiBrokenLinks.table->setItem(row, 3, item);
	      row += 1;
	    }

	m_uiBrokenLinks.table->setSortingEnabled(true);
	m_uiBrokenLinks.table->horizontalHeader()->
	  setSortIndicator(0, Qt::AscendingOrder);
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  QApplication::restoreOverrideCursor();
}

void pandamonium_gui::populateParsed(void)
{
  QApplication::setOverrideCursor(Qt::BusyCursor);
  m_ui.parsed_urls->clearContents();
  m_ui.parsed_urls->scrollToTop();
  m_ui.parsed_urls->setRowCount(0);

  QList<QList<QVariant> > list;
  int row = 0;
  quint64 limit = static_cast<quint64> (m_ui.page_limit->currentText().
					toInt());

  list = pandamonium_database::parsedLinks
    (limit, static_cast<quint64> (limit * m_ui.page->currentIndex()));
  m_ui.parsed_urls->setRowCount(list.size());

  while(!list.isEmpty())
    {
      QCheckBox *checkBox = new QCheckBox();
      QList<QVariant> values(list.takeFirst());
      QTableWidgetItem *item = 0;

      item = new QTableWidgetItem(values.value(1).toUrl().toString());
      checkBox->setToolTip
	(tr("If pressed, the periodic populating of the "
	    "table will be disabled."));
      connect(checkBox,
	      SIGNAL(clicked(bool)),
	      this,
	      SLOT(slotExportCheckBoxClicked(bool)));
      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      m_ui.parsed_urls->setCellWidget(row, 0, checkBox);
      m_ui.parsed_urls->setItem(row, 1, item);
      item = new QTableWidgetItem(values.value(0).toString());
      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      m_ui.parsed_urls->setItem(row, 2, item);
      row += 1;
    }

  m_ui.parsed_urls->resizeColumnToContents(0);
  QApplication::restoreOverrideCursor();
}

void pandamonium_gui::processExportDatabase(const QString &path)
{
  QPair<QSqlDatabase, QString> pair;

  {
    pair = pandamonium_database::database();
    pair.first.setDatabaseName(path);

    if(pair.first.open())
      {
	QStringList tables(pair.first.tables());

	disconnect(m_uiExport.tables_table,
		   SIGNAL(itemSelectionChanged(void)),
		   this,
		   SLOT(slotExportTableSelected(void)));
	m_uiExport.fields_table->clearContents();
	m_uiExport.fields_table->setRowCount(0);
	m_uiExport.tables_table->clearContents();
	m_uiExport.tables_table->setRowCount(tables.size());

	for(int i = 0; i < tables.size(); i++)
	  {
	    QTableWidgetItem *item = new QTableWidgetItem();

	    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	    item->setText(tables.at(i));
	    m_uiExport.tables_table->setItem(i, 0, item);
	  }

	m_uiExport.tables_table->sortItems(0);
	connect(m_uiExport.tables_table,
		SIGNAL(itemSelectionChanged(void)),
		this,
		SLOT(slotExportTableSelected(void)));
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
}

void pandamonium_gui::saveKernelPath(const QString &path)
{
  QSettings settings;

  settings.setValue("pandamonium_kernel_path", path);
  m_ui.kernel_path->selectAll();
}

void pandamonium_gui::slotAbout(void)
{
  QMessageBox mb(this);
  QPixmap pixmap(":/pandamonium-logo-font.png");

  pixmap = pixmap.scaled
    (256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  mb.setIconPixmap(pixmap);
  mb.setStandardButtons(QMessageBox::Ok);
  mb.setText
    (QString("<html>Version %1.<br>"
	     "Qt %2.<br>"
	     "Please visit <a href='https://github.com/textbrowser/"
	     "pandamonium/blob/master/Documentation/RELEASE-NOTES.html'>"
	     "https://github.com/textbrowser/"
	     "pandamonium/blob/master/Documentation/RELEASE-NOTES.html</a> "
	     "for release information.</html>").
     arg(pandamonium_VERSION_STR).
     arg(QT_VERSION_STR));
  mb.setTextFormat(Qt::RichText);
  mb.setWindowFlags(Qt::SplashScreen | mb.windowFlags());
  mb.setWindowTitle(tr("pandamonium: About"));
  mb.exec();
}

void pandamonium_gui::slotActivateKernel(void)
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

void pandamonium_gui::slotAddSearchUrl(void)
{
  QString str("");
  bool ok = true;

  str = QInputDialog::getText
    (this, tr("pandamonium: URL"), tr("&URL"),
     QLineEdit::Normal, QString(""), &ok);

  if(!ok)
    return;

  pandamonium_database::addSearchUrl(str);
  slotListSearchUrls();
}

void pandamonium_gui::slotDeactivateKernel(void)
{
  m_ui.monitor_kernel->setChecked(false);
  pandamonium_database::recordKernelDeactivation();
}

void pandamonium_gui::slotDeleteExportedUrlsCheckBoxClicked(bool state)
{
  QSettings settings;

  settings.setValue("pandamonium_delete_exported_urls", state);
}

void pandamonium_gui::slotDepthChanged(const QString &text)
{
  QComboBox *comboBox = qobject_cast<QComboBox *> (sender());

  if(!comboBox)
    return;

  pandamonium_database::saveSearchDepth(text, comboBox->property("url_hash"));
}

void pandamonium_gui::slotExport(void)
{
  if(!areYouSure(tr("Have you prepared an export definition?")))
    return;

  m_ui.periodically_list_parsed_urls->setChecked(false);

  QProgressDialog dialog(this);

#ifdef Q_OS_MAC
#if QT_VERSION < 0x050000
  dialog.setAttribute(Qt::WA_MacMetalStyle, true);
#endif
#endif
  dialog.setLabelText(tr("Exporting URL..."));
  dialog.setMaximum(m_ui.parsed_urls->rowCount());
  dialog.setMinimum(0);
  dialog.setWindowModality(Qt::ApplicationModal);
  dialog.setWindowTitle(tr("Exporting URL..."));
  dialog.show();
  dialog.update();
#ifndef Q_OS_MAC
  QApplication::processEvents();
#endif

  for(int i = 0; i < m_ui.parsed_urls->rowCount(); i++)
    {
      if(dialog.wasCanceled())
	break;

      dialog.setValue(i);
#ifndef Q_OS_MAC
      QApplication::processEvents();
#endif

      QCheckBox *checkBox = qobject_cast<QCheckBox *>
	(m_ui.parsed_urls->cellWidget(i, 0));
      QTableWidgetItem *item = m_ui.parsed_urls->item(i, 1);

      if(!checkBox || !item)
	continue;

      if(!checkBox->isChecked())
	continue;

      pandamonium_database::exportUrl
	(item->text(), m_ui.delete_exported_urls->isChecked());
    }

  if(m_ui.delete_exported_urls->isChecked())
    slotListParsedUrls();
}

void pandamonium_gui::slotExportCheckBoxClicked(bool state)
{
  if(state)
    m_ui.periodically_list_parsed_urls->setChecked(false);
}

void pandamonium_gui::slotExportDefinition(void)
{
  QApplication::setOverrideCursor(Qt::BusyCursor);

  QHash<QString, QString> hash(pandamonium_database::exportDefinition());

  m_uiExport.export_database_path->setText(hash.value("database_path"));
  processExportDatabase(m_uiExport.export_database_path->text());

  for(int i = 0; i < m_uiExport.tables_table->rowCount(); i++)
    {
      QTableWidgetItem *item = m_uiExport.tables_table->item(i, 0);

      if(item)
	if(hash.value("database_table") == item->text())
	  {
	    m_uiExport.tables_table->selectRow(i);
	    break;
	  }
    }

  for(int i = 0; i < m_uiExport.fields_table->rowCount(); i++)
    {
      QComboBox *comboBox = qobject_cast<QComboBox *>
	(m_uiExport.fields_table->cellWidget(i, 2));
      QTableWidgetItem *item = m_uiExport.fields_table->item(i, 0);

      if(!comboBox || !item)
	continue;

      if(hash.value("field_description") == item->text())
	comboBox->setCurrentIndex(1);
      else if(hash.value("field_title") == item->text())
	comboBox->setCurrentIndex(2);
      else if(hash.value("field_url") == item->text())
	comboBox->setCurrentIndex(3);
    }

  center(m_exportMainWindow, this);
  m_exportMainWindow->show();
  QApplication::restoreOverrideCursor();
}

void pandamonium_gui::slotExportTableSelected(void)
{
  int row = m_uiExport.tables_table->currentRow();

  if(row < 0)
    return;

  QTableWidgetItem *item = m_uiExport.tables_table->item(row, 0);

  if(!item)
    return;

  QPair<QSqlDatabase, QString> pair;

  {
    pair = pandamonium_database::database();
    pair.first.setDatabaseName(m_uiExport.export_database_path->text());

    if(pair.first.open())
      {
	m_uiExport.fields_table->clearContents();
	m_uiExport.fields_table->setRowCount(0);

	QSqlRecord record(pair.first.record(item->text()));

	m_uiExport.fields_table->setRowCount(record.count());

	for(int i = 0; i < record.count(); i++)
	  {
	    QComboBox *comboBox = 0;
	    QTableWidgetItem *item = 0;

	    item = new QTableWidgetItem();
	    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	    item->setText(record.fieldName(i));
	    m_uiExport.fields_table->setItem(i, 0, item);
	    item = new QTableWidgetItem();
	    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	    item->setText(QVariant::typeToName(record.field(i).type()));
	    m_uiExport.fields_table->setItem(i, 1, item);
	    comboBox = new QComboBox();
	    comboBox->addItem("-");
	    comboBox->addItem("field_description");
	    comboBox->addItem("field_title");
	    comboBox->addItem("field_url");
	    m_uiExport.fields_table->setCellWidget(i, 2, comboBox);
	  }

	m_uiExport.fields_table->sortItems(0);
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
}

void pandamonium_gui::slotHighlightTimeout(void)
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

  /*
  ** The status bar!
  */

  QIcon icon(":/nuvola/online.png");
  QPixmap pixmap;

  if(m_ui.kernel_pid->text().toLongLong() > 0)
    {
      m_sb.kernel->setToolTip
	(tr("The pandamonium kernel is online. "
	    "Its process identifier is %1. You may deactivate the "
	    "kernel by pressing this tool button.").
	 arg(m_ui.kernel_pid->text()));
      pixmap = icon.pixmap(QSize(16, 16), QIcon::Normal, QIcon::On);
    }
  else
    {
      m_sb.kernel->setToolTip
	(tr("The pandamonium kernel is offline. "
	    "You may activate the kernel by pressing this tool button."));
      pixmap = icon.pixmap(QSize(16, 16), QIcon::Disabled, QIcon::Off);
    }

  m_sb.kernel->setIcon(pixmap);
}

void pandamonium_gui::slotKernelDatabaseTimeout(void)
{
  pandamonium_database::createdb();

  if(m_ui.monitor_kernel->isChecked())
    slotActivateKernel();

  m_ui.kernel_pid->setText
    (QString::number(pandamonium_database::kernelProcessId()));

  /*
  ** Not for statistics.
  */

  m_uiStatistics.statistics->clearContents();
  m_uiStatistics.statistics->setRowCount(0);

  QList<qint64> values;
  QLocale locale;
  QPair<quint64, quint64> numbers
    (pandamonium_database::unvisitedAndVisitedNumbers());
  QStringList statistics;
  int percent = static_cast<int>
    (100 *
     static_cast<double> (numbers.first) /
     static_cast<double> (qMax(static_cast<quint64> (1),
			       numbers.first +
			       numbers.second)));
  uint t_now = QDateTime::currentDateTime().toTime_t();

  /*
  ** Static variables.
  */

  static int c = 0;
  static quint64 p = 0;
  static quint64 previous = 0;
  static quint64 total = 0;
  static uint t_started = t_now;

  c += 1;

  if(c >= 60 / (m_kernelDatabaseTimer.interval() / 1000))
    {
      p = total;
      c = 0;
      total = 0;
    }
  else if(c >= 2)
    {
      if(numbers.first + numbers.second >= previous)
	total += numbers.first + numbers.second - previous;

      previous = numbers.first + numbers.second;
    }

  statistics << "Interface Uptime (Minutes)"
	     << "Pages Discovered Per Minute (PPM)"
	     << "Parsed URLs"
	     << "Percent Remaining"
	     << "Remaining URLs"
	     << "Total URLs Discovered";
  values << (t_now - t_started) / 60
	 << p
	 << pandamonium_database::parsedLinksCount()
	 << static_cast<qint64> (percent)
	 << numbers.first
	 << numbers.first + numbers.second;
  m_uiStatistics.statistics->setRowCount(statistics.size());

  for(int i = 0; i < statistics.size(); i++)
    {
      QLCDNumber *number = 0;
      QTableWidgetItem *item = 0;

      item = new QTableWidgetItem();
      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      item->setText(statistics.at(i));
      m_uiStatistics.statistics->setItem(i, 0, item);
      number = new QLCDNumber();
      number->setDigitCount
	(static_cast<int> (log10(std::numeric_limits<int>::max())));
      number->display(static_cast<int> (values.at(i)));
      number->setAutoFillBackground(true);
      number->setSegmentStyle(QLCDNumber::Flat);
      number->setStyleSheet
	("QLCDNumber{color:rgb(102, 255, 0); background-color:black;}");
      m_uiStatistics.statistics->setCellWidget(i, 1, number);
    }

  m_uiStatistics.statistics->resizeColumnToContents(0);

  QStringList list;

  list << (pandamonium_common::homePath() + QDir::separator() +
	   "pandamonium_parsed_urls.db")
       << (pandamonium_common::homePath() + QDir::separator() +
	   "pandamonium_visited_urls.db");

  for(int i = 0; i < list.size(); i++)
    {
      QFileInfo fileInfo(list.at(i));
      QString toolTip("");
      int percent = 0;

      percent = static_cast<int>
	(100 * (static_cast<double> (fileInfo.size()) /
		static_cast<double> (pandamonium_common::
				     maximum_database_size)));
      toolTip = tr("%1 of %2 bytes consumed.").
	arg(locale.toString(fileInfo.size())).
	arg(locale.toString(pandamonium_common::maximum_database_size));

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

void pandamonium_gui::slotKernelToolButtonClicked(void)
{
  if(m_ui.kernel_pid->text().toLongLong() > 0)
    slotDeactivateKernel();
  else
    slotActivateKernel();
}

void pandamonium_gui::slotListParsedUrls(void)
{
  QApplication::setOverrideCursor(Qt::BusyCursor);
  m_ui.page->clear();

  QPair<quint64, quint64> numbers
    (pandamonium_database::unvisitedAndVisitedNumbers());
  quint64 i = 1;

  do
    {
      m_ui.page->addItem(tr("Page %1").arg(i));

      if(i > numbers.second / static_cast<quint64> (m_ui.
						    page_limit->
						    currentText().
						    toInt()))
	break;

      i += 1;
    }
  while(true);

  QApplication::restoreOverrideCursor();
  populateParsed();
}

void pandamonium_gui::slotListSearchUrls(void)
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
    pair = pandamonium_database::database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_search_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);
	int row = 0;

	query.setForwardOnly(true);

	if(query.exec("SELECT meta_data_only, paused, request_interval, "
		      "search_depth, url, url_hash "
		      "FROM pandamonium_search_urls "
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
		      QDoubleSpinBox *spinBox = new QDoubleSpinBox();

		      spinBox->setMaximum(100.00);
		      spinBox->setMinimum(0.100);
		      spinBox->setValue(query.value(i).toDouble());
		      spinBox->setProperty
			("url_hash", query.value(query.record().count() - 1));
		      connect(spinBox,
			      SIGNAL(valueChanged(const QString &)),
			      this,
			      SLOT(slotLoadIntervalChanged(const QString &)));
		      m_ui.search_urls->setCellWidget(row, i, spinBox);
		    }
		  else if(i == 3)
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
	m_ui.search_urls->resizeColumnToContents(2);
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  QApplication::restoreOverrideCursor();
}

void pandamonium_gui::slotLoadIntervalChanged(const QString &text)
{
  QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *> (sender());

  if(!spinBox)
    return;

  pandamonium_database::saveRequestInterval
    (text, spinBox->property("url_hash"));
}

void pandamonium_gui::slotMetaDataOnly(bool state)
{
  QCheckBox *checkBox = qobject_cast<QCheckBox *> (sender());

  if(!checkBox)
    return;

  QPair<QSqlDatabase, QString> pair;

  {
    pair = pandamonium_database::database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_search_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.prepare("UPDATE pandamonium_search_urls "
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

void pandamonium_gui::slotMonitorKernel(bool state)
{
  QSettings settings;

  settings.setValue("pandamonium_monitor_kernel", state);
}

void pandamonium_gui::slotPageChanged(int index)
{
  Q_UNUSED(index);

  if(m_uiBrokenLinks.page == sender())
    {
      m_uiBrokenLinks.page->repaint();
      populateBroken();
    }
  else
    {
      m_ui.page->repaint();
      populateParsed();
    }
}

void pandamonium_gui::slotPause(bool state)
{
  QCheckBox *checkBox = qobject_cast<QCheckBox *> (sender());

  if(!checkBox)
    return;

  QPair<QSqlDatabase, QString> pair;

  {
    pair = pandamonium_database::database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_search_urls.db");

    if(pair.first.open())
      {
	QSqlQuery query(pair.first);

	query.prepare("UPDATE pandamonium_search_urls "
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

void pandamonium_gui::slotProxyInformationToggled(bool state)
{
  if(!state)
    {
      m_ui.proxy_address->clear();
      m_ui.proxy_password->clear();
      m_ui.proxy_port->setValue(m_ui.proxy_port->minimum());
      m_ui.proxy_type->setCurrentIndex(0);
      m_ui.proxy_user->clear();

      QSettings settings;

      settings.remove("pandamonium_proxy_address");
      settings.remove("pandamonium_proxy_password");
      settings.remove("pandamonium_proxy_port");
      settings.remove("pandamonium_proxy_type");
      settings.remove("pandamonium_proxy_user");
    }
}

void pandamonium_gui::slotQuit(void)
{
  QApplication::instance()->quit();
}

void pandamonium_gui::slotRefreshBrokenUrls(void)
{
  QApplication::setOverrideCursor(Qt::BusyCursor);

  QPair<QSqlDatabase, QString> pair;

  {
    pair = pandamonium_database::database();
    pair.first.setDatabaseName
      (pandamonium_common::homePath() + QDir::separator() +
       "pandamonium_broken_urls.db");

    if(pair.first.open())
      {
	m_uiBrokenLinks.page->clear();

	QSqlQuery query(pair.first);
	quint64 count = 0;
	quint64 i = 1;

	if(query.exec("SELECT COUNT(*) FROM pandamonium_broken_urls"))
	  if(query.next())
	    count = query.value(0).toULongLong();

	do
	  {
	    m_uiBrokenLinks.page->addItem(tr("Page %1").arg(i));

	    if(i > count / static_cast<quint64> (m_uiBrokenLinks.
						 page_limit->
						 currentText().
						 toInt()))
	      break;

	    i += 1;
	  }
	while(true);
      }

    pair.first.close();
    pair.first = QSqlDatabase();
  }

  QSqlDatabase::removeDatabase(pair.second);
  QApplication::restoreOverrideCursor();
  populateBroken();
}

void pandamonium_gui::slotRemoveAllBrokenUrls(void)
{
  if(!areYouSure(tr("Are you sure that you wish to remove all of "
		    "the broken links?")))
    return;

  QFile::remove(pandamonium_common::homePath() + QDir::separator() +
		"pandamonium_broken_urls.db");
  slotRefreshBrokenUrls();
}

void pandamonium_gui::slotRemoveAllParsedUrls(void)
{
  if(!areYouSure(tr("Are you sure that you wish to remove all of "
		    "the parsed URLs?")))
    return;

  QFile::remove(pandamonium_common::homePath() + QDir::separator() +
		"pandamonium_parsed_urls.db");
  slotListParsedUrls();
}

void pandamonium_gui::slotRemoveSelectedParsedUrls(void)
{
  QModelIndexList indexes
    (m_ui.parsed_urls->selectionModel()->selectedRows(1)); // URL

  if(indexes.isEmpty())
    return;

  if(!areYouSure(tr("Are you sure that you wish to remove the selected "
		    "URLs?")))
    return;

  QList<QString> list;

  while(!indexes.isEmpty())
    {
      QModelIndex index(indexes.takeFirst());

      if(index.isValid())
	list << index.data().toString();
    }

  if(!list.isEmpty())
    {
      pandamonium_database::removeParsedUrls(list);
      slotListParsedUrls();
    }
}

void pandamonium_gui::slotRemoveSelectedSearchUrls(void)
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
      pandamonium_database::removeSearchUrls(list);
      slotListSearchUrls();
    }
}

void pandamonium_gui::slotRemoveUnvisitedVisitedUrls(void)
{
  if(!areYouSure(tr("Are you sure that you wish to remove "
		    "pandamonium_visited_urls.db?")))
    return;

  QFile::remove(pandamonium_common::homePath() + QDir::separator() +
		"pandamonium_visited_urls.db");
}

void pandamonium_gui::slotSaveExportDefinition(void)
{
  QHash<QString, QString> hash;
  QTableWidgetItem *item = 0;

  hash["database_path"] = m_uiExport.export_database_path->text();
  item = m_uiExport.tables_table->item
    (m_uiExport.tables_table->currentRow(), 0);

  if(item)
    hash["database_table"] = item->text();

  for(int i = 0; i < m_uiExport.fields_table->rowCount(); i++)
    {
      QComboBox *comboBox = qobject_cast<QComboBox *>
	(m_uiExport.fields_table->cellWidget(i, 2));

      item = m_uiExport.fields_table->item(i, 0);

      if(!comboBox || !item)
	continue;

      hash[comboBox->currentText()] = item->text();
    }

  if(!pandamonium_database::saveExportDefinition(hash))
    QMessageBox::critical
      (this, tr("pandamonium: Error"),
       tr("An error occurred while attempting to save the "
	  "export definition. Please verify that you have unique values "
	  "for all of the database fields."));
}

void pandamonium_gui::slotSaveKernelPath(void)
{
  saveKernelPath(m_ui.kernel_path->text());
}

void pandamonium_gui::slotSavePageLimit(const QString &text)
{
  QSettings settings;

  settings.setValue("pandamonium_page_limit", text);
}

void pandamonium_gui::slotSavePeriodic(bool state)
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

      str = "pandamonium_periodically_list_parsed_urls";
    }
  else
    str = "pandamonium_periodically_list_search_urls";

  settings.setValue(str, state);
}

void pandamonium_gui::slotSaveProxyInformation(void)
{
  QSettings settings;

  settings.setValue
    ("pandamonium_proxy_address", m_ui.proxy_address->text().trimmed());
  settings.setValue
    ("pandamonium_proxy_password", m_ui.proxy_password->text());
  settings.setValue
    ("pandamonium_proxy_port", m_ui.proxy_port->value());
  settings.setValue
    ("pandamonium_proxy_type", m_ui.proxy_type->currentIndex());
  settings.setValue
    ("pandamonium_proxy_user", m_ui.proxy_user->text());
}

void pandamonium_gui::slotSelectExportDatabase(void)
{
  QFileDialog dialog(m_exportMainWindow);

  dialog.setWindowTitle(tr("pandamonium: Select Export Database"));
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
      m_uiExport.export_database_path->setText
		 (dialog.selectedFiles().value(0));
      processExportDatabase(dialog.selectedFiles().value(0));
    }
}

void pandamonium_gui::slotSelectKernelPath(void)
{
  QFileDialog dialog(this);

  dialog.setWindowTitle(tr("pandamonium: Select Kernel Path"));
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

void pandamonium_gui::slotShowBrokenLinksWindow(void)
{
  center(m_brokenLinksWindow, this);
  m_brokenLinksWindow->show();
}

void pandamonium_gui::slotShowStatisticsWindow(void)
{
  center(m_statisticsMainWindow, this);
  m_statisticsMainWindow->show();
}

void pandamonium_gui::slotTabIndexChanged(int index)
{
  Q_UNUSED(index);
}

void pandamonium_gui::slotTableListTimeout(void)
{
  QFileInfo fileInfo
    (pandamonium_common::homePath() + QDir::separator() +
     "pandamonium_parsed_urls.db");

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

void pandamonium_gui::slotToggleParsed(void)
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
