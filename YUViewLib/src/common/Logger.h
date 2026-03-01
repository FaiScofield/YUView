/*  This file is part of YUView - The YUV player with advanced analytics toolset
 *   <https://github.com/IENT/YUView>
 *   Copyright (C) 2026 Vance <wfyash@163.com>
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

#pragma once

#ifndef YUVIEW_LOGGER_H
#define YUVIEW_LOGGER_H

#if ENABLE_SPDLOG
#include <spdlog/spdlog.h>

#define LOGT(fmt, ...) spdlog::trace(fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) spdlog::debug(fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) spdlog::info(fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) spdlog::warn(fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) spdlog::error(fmt, ##__VA_ARGS__)
#define LOGC(fmt, ...) spdlog::critical(fmt, ##__VA_ARGS__)

// #define DEBUG qDebug

// spdlog 间接包含 Windows 头文件，其定义了臭名昭著的 IN 和 OUT 宏，
// 这导致代码中的 ZoomMode::IN 和 ZoomMode::OUT 在预处理阶段被宏替换为空，从而产生语法错误。
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

#else

#define LOGT(fmt, ...) ((void)0)
#define LOGD(fmt, ...) ((void)0)
#define LOGI(fmt, ...) ((void)0)
#define LOGW(fmt, ...) ((void)0)
#define LOGE(fmt, ...) ((void)0)
#define LOGC(fmt, ...) ((void)0)

// #define DEBUG NullStreamLogger
#endif

#define LOGO(fmt, ...) ((void)0)


/* a null stream target for debug message output */
class NullStream
{
public:
  template <typename T> inline NullStream &operator<<(const T &) { return *this; }
};

static inline NullStream NullStreamLogger()
{
  return NullStream();
}

#endif /* YUVIEW_LOGGER_H */