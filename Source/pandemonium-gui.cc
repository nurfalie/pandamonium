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

extern "C"
{
#include <math.h>
}

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
#include <QtDebug>

#include <limits>

#include "pandemonium-common.h"
#include "pandemonium-database.h"
#include "pandemonium-gui.h"

pandemonium_gui::pandemonium_gui(void):QMainWindow()
{
  QDir().mkdir(pandemonium_common::homePath());
  m_parsedLinksLastDateTime = 0;
  m_exportMainWindow = new QMainWindow(this);
  m_statisticsMainWindow = new QMainWindow(this);
  m_statisticsMainWindow->setWindowFlags
    (windowFlags() | Qt::WindowStaysOnTopHint);
  m_ui.setupUi(this);
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
  connect(m_ui.action_About,
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotAbout(void)));
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

  m_ui.delete_exported_urls->setChecked
    (settings.value("pandemonium_delete_exported_urls").toBool());

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
}

pandemonium_gui::~pandemonium_gui()
{
}

bool pandemonium_gui::areYouSure(const QString &text)
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
  mb.setWindowTitle(tr("pandemonium: Confirmation"));

  if(mb.exec() != QMessageBox::Yes)
    return false;
  else
    return true;
}

void pandemonium_gui::center(QWidget *child, QWidget *parent)
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

  QList<QList<QVariant> > list;
  int row = 0;
  quint64 limit = static_cast<quint64> (m_ui.page_limit->currentText().
					toInt());

  list = pandemonium_database::parsedLinks
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

void pandemonium_gui::processExportDatabase(const QString &path)
{
  QPair<QSqlDatabase, QString> pair;

  {
    pair = pandemonium_database::database();
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

void pandemonium_gui::saveKernelPath(const QString &path)
{
  QSettings settings;

  settings.setValue("pandemonium_kernel_path", path);
  m_ui.kernel_path->selectAll();
}

void pandemonium_gui::slotAbout(void)
{
  QMessageBox mb(this);

  mb.setIconPixmap(QPixmap(":/nuvola/www.png"));
  mb.setStandardButtons(QMessageBox::Ok);
  mb.setText
    (QString("<html>Version %1.<br>"
	     "Qt %2.<br>"
	     "Please visit <a href='https://github.com/textbrowser/"
	     "pandemonium/blob/master/Documentation/RELEASE-NOTES.html'>"
	     "https://github.com/textbrowser/"
	     "pandemonium/blob/master/Documentation/RELEASE-NOTES.html</a> "
	     "for release information.</html>").
     arg(PANDEMONIUM_VERSION_STR).
     arg(QT_VERSION_STR));
  mb.setTextFormat(Qt::RichText);
  mb.setWindowTitle(tr("pandemonium: About"));
  mb.exec();
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

void pandemonium_gui::slotDeleteExportedUrlsCheckBoxClicked(bool state)
{
  QSettings settings;

  settings.setValue("pandemonium_delete_exported_urls", state);
}

void pandemonium_gui::slotDepthChanged(const QString &text)
{
  QComboBox *comboBox = qobject_cast<QComboBox *> (sender());

  if(!comboBox)
    return;

  pandemonium_database::saveDepth(text, comboBox->property("url_hash"));
}

void pandemonium_gui::slotExport(void)
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
  dialog.setLabelText(tr("Exporting URL(s)..."));
  dialog.setMaximum(m_ui.parsed_urls->rowCount());
  dialog.setMinimum(0);
  dialog.setWindowModality(Qt::ApplicationModal);
  dialog.setWindowTitle(tr("Exporting URL(s)..."));
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

      pandemonium_database::exportUrl
	(item->text(), m_ui.delete_exported_urls->isChecked());
    }

  if(m_ui.delete_exported_urls->isChecked())
    slotListParsedUrls();
}

void pandemonium_gui::slotExportCheckBoxClicked(bool state)
{
  if(state)
    m_ui.periodically_list_parsed_urls->setChecked(false);
}

void pandemonium_gui::slotExportDefinition(void)
{
  QApplication::setOverrideCursor(Qt::BusyCursor);

  QHash<QString, QString> hash(pandemonium_database::exportDefinition());

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

void pandemonium_gui::slotExportTableSelected(void)
{
  int row = m_uiExport.tables_table->currentRow();

  if(row < 0)
    return;

  QTableWidgetItem *item = m_uiExport.tables_table->item(row, 0);

  if(!item)
    return;

  QPair<QSqlDatabase, QString> pair;

  {
    pair = pandemonium_database::database();
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
  int percent = static_cast<int>
    (100 *
     static_cast<double> (numbers.first) /
     static_cast<double> (qMax(static_cast<quint64> (1),
			       numbers.first +
			       numbers.second)));
  quint64 ldpm = 0;
  uint t_now = QDateTime::currentDateTime().toTime_t() / 60;

  /*
  ** Static variables.
  */

  static quint64 links_before = 0;
  static uint t_before = 0;

  if(numbers.first + numbers.second > links_before)
    ldpm = (numbers.first + numbers.second - links_before) /
      qMax(static_cast<quint64> (1), static_cast<quint64> (t_now - t_before));
  else
    ldpm = (links_before - (numbers.first + numbers.second)) /
      qMax(static_cast<quint64> (1), static_cast<quint64> (t_now - t_before));

  links_before = numbers.first + numbers.second;
  t_before = t_now;
  statistics << "Links Discovered Per Minute"
	     << "Parsed URL(s)"
	     << "Percent Remaining"
	     << "Remaining URL(s)"
	     << "Total URL(s) Discovered";
  values << ldpm
	 << pandemonium_database::parsedLinksCount()
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

  list << (pandemonium_common::homePath() + QDir::separator() +
	   "pandemonium_parsed_urls.db")
       << (pandemonium_common::homePath() + QDir::separator() +
	   "pandemonium_visited_urls.db");

  for(int i = 0; i < list.size(); i++)
    {
      QFileInfo fileInfo(list.at(i));
      QString toolTip("");
      int percent = 0;

      percent = static_cast<int>
	(100 * (static_cast<double> (fileInfo.size()) /
		static_cast<double> (pandemonium_common::
				     maximum_database_size)));
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
  quint64 i = 1;

  m_ui.page->clear();

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
  if(!areYouSure(tr("Are you sure that you wish to remove all of "
		    "the parsed URL(s)?")))
    return;

  QFile::remove(pandemonium_common::homePath() + QDir::separator() +
		"pandemonium_parsed_urls.db");
  slotListParsedUrls();
}

void pandemonium_gui::slotRemoveSelectedParsedUrls(void)
{
  QModelIndexList indexes
    (m_ui.parsed_urls->selectionModel()->selectedRows(1)); // URL

  if(indexes.isEmpty())
    return;

  if(!areYouSure(tr("Are you sure that you wish to remove the selected "
		    "URL(s)?")))
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
      pandemonium_database::removeParsedUrls(list);
      slotListParsedUrls();
    }
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
  if(!areYouSure(tr("Are you sure that you wish to remove "
		    "pandemonium_visited_urls.db?")))
    return;

  QFile::remove(pandemonium_common::homePath() + QDir::separator() +
		"pandemonium_visited_urls.db");
}

void pandemonium_gui::slotSaveExportDefinition(void)
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

  if(!pandemonium_database::saveExportDefinition(hash))
    QMessageBox::critical
      (this, tr("pandemonium: Error"),
       tr("An error occurred while attempting to save the "
	  "export definition. Please verify that you have unique values "
	  "for all of the database fields."));
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

void pandemonium_gui::slotSelectExportDatabase(void)
{
  QFileDialog dialog(m_exportMainWindow);

  dialog.setWindowTitle(tr("pandemonium: Select Export Database"));
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
  center(m_statisticsMainWindow, this);
  m_statisticsMainWindow->show();
}

void pandemonium_gui::slotTabIndexChanged(int index)
{
  if(index == 0)
    {
      m_ui.kernel_box_grid_layout->addWidget(m_ui.kernel_pid, 1, 1);
      m_ui.kernel_box_grid_layout->addWidget(m_ui.activate_kernel, 1, 2);
      m_ui.kernel_box_grid_layout->addWidget(m_ui.deactivate_kernel, 1, 3);
    }
  else
    {
      statusBar()->addWidget(m_ui.kernel_pid);
      statusBar()->addWidget(m_ui.activate_kernel);
      statusBar()->addWidget(m_ui.deactivate_kernel);
    }
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
