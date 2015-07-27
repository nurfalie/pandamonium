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

#include <QApplication>
#include <QNetworkAccessManager>
#include <QtDebug>

#include "pandamonium-common.h"
#include "pandamonium-database.h"
#include "pandamonium-kernel.h"

static pandamonium_kernel *s_kernel = 0;

pandamonium_kernel::pandamonium_kernel(void):QObject()
{
  s_kernel = this;
  m_networkAccessManager = new QNetworkAccessManager(this);
  m_networkAccessManager->setProxy(pandamonium_common::proxy());
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
  pandamonium_database::createdb();
  pandamonium_database::recordKernelProcessId(QApplication::applicationPid());
}

pandamonium_kernel::~pandamonium_kernel()
{
  s_kernel = 0;
  pandamonium_database::recordKernelDeactivation
    (QApplication::applicationPid());
  QApplication::quit();
}

QNetworkReply *pandamonium_kernel::get(const QNetworkRequest &request)
{
  return s_kernel->m_networkAccessManager->get(request);
}

void pandamonium_kernel::slotControlTimeout(void)
{
  if(pandamonium_database::
     shouldTerminateKernel(QApplication::applicationPid()))
    deleteLater();

  m_networkAccessManager->setProxy(pandamonium_common::proxy());
}

void pandamonium_kernel::slotRovingTimeout(void)
{
  QList<QList<QVariant> > list(pandamonium_database::searchUrls());
  QList<QUrl> urls;

  for(int i = 0; i < list.size(); i++)
    {
      QList<QVariant> values(list.at(i)); /*
					  ** 0 - paused
					  ** 1 - request_interval
					  ** 2 - search_depth
					  ** 3 - url
					  */
      QUrl url(values.value(3).toUrl());

      if(url.isEmpty())
	continue;
      else if(!url.isValid())
	continue;

      urls << url;

      if(!m_searchUrls.contains(url))
	{
	  QPointer<pandamonium_kernel_url> u = new pandamonium_kernel_url
	    (url,
	     values.value(0).toBool(),
	     values.value(1).toDouble(),
	     values.value(2).toInt(),
	     this);

	  m_searchUrls[url] = u;
	}
      else
	{
	  QPointer<pandamonium_kernel_url> u(m_searchUrls.value(url, 0));

	  if(u)
	    {
	      u->setPaused(values.value(0).toBool());
	      u->setRequestInterval(values.value(1).toDouble());
	    }
	}
    }

  QMutableHashIterator<QUrl, QPointer<pandamonium_kernel_url> >
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
