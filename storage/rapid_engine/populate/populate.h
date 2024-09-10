/**
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

   The fundmental code for imcs. The chunk is used to store the data which
   transfer from row-based format to column-based format.

   Copyright (c) 2023, Shannon Data AI and/or its affiliates.

   The fundmental code for imcs. The chunk is used to store the data which
   transfer from row-based format to column-based format.
*/

#ifndef __SHANNONBASE_POPULATE_H__
#define __SHANNONBASE_POPULATE_H__
#include <memory>
#include <unordered_map>

#include "my_inttypes.h"
#include "storage/rapid_engine/include/rapid_const.h"
#include "storage/rapid_engine/populate/log_commons.h"

class IB_thread;

namespace ShannonBase {
namespace Populate {

#define log_rapid_pop_mutex_enter(log) mutex_enter(&((log).rapid_populator_mutex))

#define log_rapid_pop_mutex_enter_nowait(log) mutex_enter_nowait(&((log).rapid_populator_mutex))

#define log_rapid_pop_mutex_exit(log) mutex_exit(&((log).rapid_populator_mutex))

#define log_rapid_pop_mutex_own(log) (mutex_own(&((log).rapid_populator_mutex)) || !Populator::log_rapid_is_active())

typedef struct mtr_log_rec_t {
  std::unique_ptr<uchar[]> data;
  size_t size;

  mtr_log_rec_t(size_t s) : data(std::make_unique<uchar[]>(s)), size(s) {}

  mtr_log_rec_t(mtr_log_rec_t &&other) noexcept : data(std::move(other.data)), size(other.size) { other.size = 0; }

  mtr_log_rec_t &operator=(mtr_log_rec_t &&other) noexcept {
    if (this != &other) {
      data = std::move(other.data);
      size = other.size;
      other.size = 0;
    }
    return *this;
  }

  // Deleted copy constructor and copy assignment operator to prevent copying
  mtr_log_rec_t(const mtr_log_rec_t &) = delete;
  mtr_log_rec_t &operator=(const mtr_log_rec_t &) = delete;

  ~mtr_log_rec_t() {}
} mtr_log_rec;

// pop change buffer size
extern ulonglong sys_population_buffer_sz;
// flag of pop change thread. true is running, set to false to stop
extern std::atomic<bool> sys_pop_started;
/** a buffer to store redo log records, then parses these records. if we
use std::map, that will occure corruption at rb tree reblance. if we impl
copy cotr or assingment cotr, it's still corruption, due to access
empty address. Therefore, here, we use hash map to store it, and other reason:
the RB tree will do re-blancing when the size of items exceed a threshold, that's
performance issue. in future, we will use co-rountine to process every item by a
co-routine to promot the performance.
*/
extern std::unordered_map<uint64_t, mtr_log_rec> sys_pop_buff;

class Populator {
 public:
  static bool log_pop_thread_is_active();
  static void start_change_populate_threads();
  static void end_change_populate_threads();
  static void rapid_print_thread_info(FILE *file);
};

}  // namespace Populate
}  // namespace ShannonBase
#endif  //__SHANNONBASE_POPULATE_H__