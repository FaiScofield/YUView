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

#include <QCoreApplication>

#include "common/Typedef.h"
#include "common/Logger.h"
#include "ui/YUViewApplication.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#if ENABLE_SPDLOG
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#endif

int main(int argc, char *argv[])
{
  QString logLevelStr;
  QString logFileStr("log/yuview.log");
  for (int i = 1; i < argc; ++i)
  {
    QString arg(argv[i]);
    if (arg.startsWith("--loglevel="))
    {
      logLevelStr = arg.mid(11);
    }
    else if (arg == "--logfile")
    {
      logFileStr = "logs/yuview.log";
    }
    else if (arg.startsWith("--logfile="))
    {
      logFileStr = arg.mid(10);
    }
  }

#if ENABLE_SPDLOG
  // init spdlog logger
  try
  {
    // set log level from command line argument
    spdlog::level::level_enum logLevel = spdlog::level::debug;
    if (!logLevelStr.isEmpty())
    {
      if (logLevelStr == "trace")
        logLevel = spdlog::level::trace;
      else if (logLevelStr == "debug")
        logLevel = spdlog::level::debug;
      else if (logLevelStr == "info")
        logLevel = spdlog::level::info;
      else if (logLevelStr == "warning")
        logLevel = spdlog::level::warn;
      else if (logLevelStr == "error")
        logLevel = spdlog::level::err;
      else if (logLevelStr == "fatal")
        logLevel = spdlog::level::critical;
      else
        qDebug() << "Unknown log level:" << logLevelStr << ", using default (debug)";
    }

    // console sink
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%P-%t] [%^%l%$] %v");
    console_sink->set_level(logLevel);

    // file sink
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFileStr.toStdString(), true);
    file_sink->set_level(spdlog::level::trace);

    // multi-sink logger
    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
    auto logger = std::make_shared<spdlog::logger>("yuview", sinks.begin(), sinks.end());
    spdlog::set_default_logger(logger);
    // spdlog::set_level(logLevel);

    LOGI("=== YUView Start ===");
    LOGI("spdlog log level: {}", spdlog::level::to_string_view(logLevel));
  }
  catch (const std::exception &e)
  {
    qDebug() << "spdlog init failed: " << e.what();
  }
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // DPI support
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);    // DPI support
#endif
  QCoreApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false);
  QCoreApplication::setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false);

  qRegisterMetaType<recacheIndicator>("recacheIndicator");

  YUViewApplication app(argc, argv);

  return app.returnCode;
}
