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

#include <QApplication>
#include <QNetworkAccessManager>
#include <QtDebug>

#include "pandemonium-common.h"
#include "pandemonium-database.h"
#include "pandemonium-kernel.h"

static pandemonium_kernel *s_kernel = 0;

pandemonium_kernel::pandemonium_kernel(void):QObject()
{
  s_kernel = this;
  m_networkAccessManager = new QNetworkAccessManager(this);
  m_networkAccessManager->setProxy(pandemonium_common::proxy());
  connect(&m_controlTimer,
	  SIGNAL(timeout(void)),
	  this,
	  SLOT(slotControlTimeout(void)));
  connect(&m_rovingTimer,
	  SIGNAL(timeout(void)),
	  this,
	  SLOT(slotRovingTimeout(void)));
  m_controlTimer.start(2500);
  m_rovingTimer.start(2500);
  pandemonium_database::createdb();
  pandemonium_database::recordKernelProcessId(QApplication::applicationPid());
}

pandemonium_kernel::~pandemonium_kernel()
{
  s_kernel = 0;
  pandemonium_database::recordKernelDeactivation
    (QApplication::applicationPid());
  QApplication::quit();
}

QNetworkReply *pandemonium_kernel::get(const QNetworkRequest &request)
{
  return s_kernel->m_networkAccessManager->get(request);
}

void pandemonium_kernel::slotControlTimeout(void)
{
  if(pandemonium_database::
     shouldTerminateKernel(QApplication::applicationPid()))
    deleteLater();

  m_networkAccessManager->setProxy(pandemonium_common::proxy());
}

void pandemonium_kernel::slotRovingTimeout(void)
{
  QList<QPair<QUrl, int> > list(pandemonium_database::searchUrls());
  QList<QUrl> urls;

  for(int i = 0; i < list.size(); i++)
    {
      QPair<QUrl, int> pair(list.at(i));

      if(pair.first.isEmpty())
	continue;
      else if(!pair.first.isValid())
	continue;

      urls << pair.first;

      if(!m_searchUrls.contains(pair.first))
	{
	  QPointer<pandemonium_kernel_url> url = new pandemonium_kernel_url
	    (pair.first, pair.second, this);
	  m_searchUrls[pair.first] = url;
	}
    }

  QMutableHashIterator<QUrl, QPointer<pandemonium_kernel_url> >
    it(m_searchUrls);

  while(it.hasNext())
    {
      it.next();

      if(!urls.contains(it.key()))
	{
	  qDebug() << "Removing search URL " << it.key() << ".";

	  if(it.value())
	    it.value()->deleteLater();

	  it.remove();
	}
    }
}
