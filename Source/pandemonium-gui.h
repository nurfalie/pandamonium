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

#ifndef _pandemonium_gui_h_
#define _pandemonium_gui_h_

#include <QMainWindow>
#include <QTimer>

#include "ui_pandemonium.h"
#include "ui_pandemonium_export_definition.h"
#include "ui_pandemonium_statistics.h"
#include "ui_pandemonium_statusbar.h"

class pandemonium_gui: public QMainWindow
{
  Q_OBJECT

 public:
  pandemonium_gui(void);
  ~pandemonium_gui();

 private:
  QMainWindow *m_exportMainWindow;
  QMainWindow *m_statisticsMainWindow;
  QTimer m_highlightTimer;
  QTimer m_kernelDatabaseTimer;
  QTimer m_tableListTimer;
  QWidget *m_sbWidget;
  Ui_pandemonium_mainwindow m_ui;
  Ui_pandemonium_export m_uiExport;
  Ui_pandemonium_statistics m_uiStatistics;
  Ui_pandemonium_statusbar m_sb;
  uint m_parsedLinksLastDateTime;
  bool areYouSure(const QString &text);
  void center(QWidget *child, QWidget *parent);
  void closeEvent(QCloseEvent *event);
  void populateParsed(void);
  void processExportDatabase(const QString &path);
  void saveKernelPath(const QString &path);

 private slots:
  void slotAbout(void);
  void slotActivateKernel(void);
  void slotAddSearchUrl(void);
  void slotDeactivateKernel(void);
  void slotDeleteExportedUrlsCheckBoxClicked(bool state);
  void slotDepthChanged(const QString &text);
  void slotExport(void);
  void slotExportCheckBoxClicked(bool state);
  void slotExportDefinition(void);
  void slotExportTableSelected(void);
  void slotHighlightTimeout(void);
  void slotKernelDatabaseTimeout(void);
  void slotKernelToolButtonClicked(void);
  void slotListParsedUrls(void);
  void slotListSearchUrls(void);
  void slotLoadIntervalChanged(const QString &text);
  void slotMetaDataOnly(bool state);
  void slotMonitorKernel(bool state);
  void slotPageChanged(int index);
  void slotPause(bool state);
  void slotProxyInformationToggled(bool state);
  void slotQuit(void);
  void slotRemoveAllParsedUrls(void);
  void slotRemoveSelectedParsedUrls(void);
  void slotRemoveSelectedSearchUrls(void);
  void slotRemoveUnvisitedVisitedUrls(void);
  void slotSaveExportDefinition(void);
  void slotSaveKernelPath(void);
  void slotSavePageLimit(const QString &text);
  void slotSavePeriodic(bool state);
  void slotSaveProxyInformation(void);
  void slotSelectExportDatabase(void);
  void slotSelectKernelPath(void);
  void slotShowStatisticsWindow(void);
  void slotTabIndexChanged(int index);
  void slotTableListTimeout(void);
  void slotToggleParsed(void);
};

#endif
