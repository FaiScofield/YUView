/*  This file is part of YUView - The YUV player with advanced analytics toolset
 *   <https://github.com/IENT/YUView>
 *   Copyright (C) 2015  Institut für Nachrichtentechnik, RWTH Aachen University, GERMANY
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the
 *   OpenSSL library under certain conditions as described in each
 *   individual source file, and distribute linked combinations including
 *   the two.
 *
 *   You must obey the GNU General Public License in all respects for all
 *   of the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the
 *   file(s), but you are not obligated to do so. If you do not wish to do
 *   so, delete this exception statement from your version. If you delete
 *   this exception statement from all source files in the program, then
 *   also delete it here.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "SingleInstanceHandler.h"
#include "common/Logger.h"


singleInstanceHandler::singleInstanceHandler(QObject *parent) : QObject(parent)
{
  connect(&server, &QLocalServer::newConnection, this, &singleInstanceHandler::newConnection);
}

singleInstanceHandler::~singleInstanceHandler()
{
  server.close();
}

void singleInstanceHandler::newConnection()
{
  socket = server.nextPendingConnection();
  connect(socket.data(), &QLocalSocket::readyRead, this, &singleInstanceHandler::readyRead);
}

void singleInstanceHandler::readyRead()
{
  QByteArray  tmp      = socket->readAll();
  QStringList fileList = QString(tmp).split("\n");
  if (fileList.length() > 0)
  {
    QStringList openFiles;
    for (QString s : fileList)
    {
      if (!s.isEmpty())
      {
        LOGD("singleInstanceHandler::readyRead got file %s", s.toLatin1().data());
        openFiles.append(s);
      }
    }

    emit newAppStarted(openFiles);
  }

  socket->close();
  socket->deleteLater();
}

bool singleInstanceHandler::isRunning(QString name, QStringList args)
{
  LOGD("singleInstanceHandler::isRunning {}", name.toStdString());

  QLocalSocket socket;
  socket.connectToServer(name, QLocalSocket::ReadWrite);

  if (socket.waitForConnected())
  {
    LOGD(
        "singleInstanceHandler::isRunning Connected to other instance. Sending data.");

    QByteArray buffer;
    for (auto &item : args)
      buffer.append(QString(item + "\n").toLatin1());
    socket.write(buffer);
    socket.waitForBytesWritten();
    return true;
  }

  LOGD("singleInstanceHandler::isRunning we are the first session - {}",
                       socket.errorString().toStdString());
  return false;
}

void singleInstanceHandler::listen(QString name)
{
  server.removeServer(name);
  server.listen(name);

  LOGD("singleInstanceHandler::listen name {} - {}",
                       name.toStdString(),
                       server.errorString().toStdString());
}