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
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#endif

/**
 * @brief Show or hide the console window on Windows
 * @param show True to show console, false to hide
 */
void setConsoleVisible(bool show)
{
#if defined(_WIN32)
  HWND consoleWindow = GetConsoleWindow();
  if (consoleWindow != nullptr && IsWindow(consoleWindow)) {
    ShowWindow(consoleWindow, show ? SW_SHOW : SW_HIDE);
    LOGI("Show console window success.");
  }
  else
    LOGW("Failed to get console window handle!");
#endif
}

/**
 * @brief Print help message to console
 */
void printHelp()
{
  printf("YUView - The YUV player with advanced analytics toolset\n\n");
  printf("Usage: YUView [options]\n\n");
  printf("Options:\n");
  printf("  -h, --help              Show this help message\n");
  printf("  --console               Keep the console window visible (default: hidden)\n");
  printf("  --loglevel=<level>      Set log level (trace, debug, info, warn, error, critical) (default: debug)\n");
  printf("  --logfile[=<path>]      Set log file path (default: log/yuview.log)\n");
  printf("\n");
}

int main(int argc, char *argv[])
{
  bool showConsole = false;
  bool showHelp = false;
  QString logLevelStr;
  QString logFileStr("log/yuview.log");

  // Parse command line arguments
  for (int i = 1; i < argc; ++i)
  {
    QString arg(argv[i]);
    if (arg == "-h" || arg == "--help")
      showHelp = true;
    else if (arg == "--console")
      showConsole = true;
    else if (arg.startsWith("--loglevel="))
      logLevelStr = arg.mid(11);
    else if (arg == "--logfile")
      logFileStr = "logs/yuview.log";
    else if (arg.startsWith("--logfile="))
      logFileStr = arg.mid(10);
  }

  // Show help and exit if requested
  if (showHelp)
  {
    setConsoleVisible(true);
    printHelp();
    return 0;
  }

#if ENABLE_SPDLOG
  // init spdlog logger
  try
  {
    // set log level from command line argument
    spdlog::level::level_enum logLevel = spdlog::level::debug;
    if (!logLevelStr.isEmpty())
      logLevel = spdlog::level::from_str(logLevelStr.toStdString());

    // console sink
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  #ifndef NDEBUG
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%5P-%-5t] [%^%-5l%$] %@ %v");
  #else
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%5P-%-5t] [%^%-5l%$] %v");
  #endif
    console_sink->set_level(logLevel);

    // file sink
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFileStr.toStdString(), true);
    file_sink->set_level(spdlog::level::trace);

    // multi-sink logger
    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
    auto logger = std::make_shared<spdlog::logger>("yuview", sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace);
    spdlog::set_default_logger(logger);

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

  // Hide console by default on Windows
  setConsoleVisible(showConsole);

  YUViewApplication app(argc, argv);

  return app.returnCode;
}
