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

#include "storage/rapid_engine/populate/populate.h"

#include "current_thd.h"
#include "sql/sql_class.h"
#include "include/os0event.h"
#include "storage/innobase/include/os0thread-create.h"

#include "storage/rapid_engine/populate/log_parser.h"

#ifdef UNIV_PFS_THREAD
mysql_pfs_key_t rapid_populate_thread_key;
#endif /* UNIV_PFS_THREAD */

namespace ShannonBase {
namespace Populate {

std::atomic<bool> sys_pop_started {false};
static IB_thread sys_log_rapid_thread;

ulonglong sys_population_buffer_sz = m_pop_buff_size;
std::unique_ptr<Ringbuffer<uchar>> sys_population_buffer {nullptr};
static ulint sys_rapid_loop_count;

static void parse_log_func (log_t *log_ptr) {
  THD* log_pop_thread_thd {nullptr};
  if (current_thd == nullptr) {
    log_pop_thread_thd = create_internal_thd();
    if (!log_pop_thread_thd) return;
  }

  os_event_reset(log_ptr->rapid_events[0]);
   //here we have a notifiyer, when checkpoint_lsn/flushed_lsn > rapid_lsn to start pop
  while (sys_pop_started.load(std::memory_order_seq_cst)) {
    auto stop_condition = [&](bool wait) {
      if (sys_population_buffer->readAvailable()) {
        return true;
      }
      if (wait) { //do somthing in waiting
      }
      return false;
    };

    os_event_wait_for(log_ptr->rapid_events[0], MAX_LOG_POP_SPIN_COUNT,
                      std::chrono::microseconds{100}, stop_condition);

    sys_rapid_loop_count++;

    auto size = sys_population_buffer->readAvailable();
    byte* from_ptr = sys_population_buffer->peek();
    LogParser parse_log;
    uint parsed_bytes = parse_log.parse_redo(from_ptr, from_ptr + size);
    sys_population_buffer->remove(parsed_bytes);
  } //wile(pop_started)

  os_event_set(log_ptr->rapid_events[0]);
  if (log_pop_thread_thd) {
    destroy_internal_thd(log_pop_thread_thd);
    log_pop_thread_thd = nullptr;
  }
  sys_pop_started.store(false, std::memory_order_seq_cst);
}

bool Populator::log_pop_thread_is_active() {
   return  thread_is_active(sys_log_rapid_thread);
}

void Populator::start_change_populate_threads() {
  if (!Populator::log_pop_thread_is_active()) {
    sys_log_rapid_thread =
      os_thread_create(rapid_populate_thread_key, 0, parse_log_func, log_sys);
    ShannonBase::Populate::sys_pop_started = true;
    sys_log_rapid_thread.start();
  }
}

void Populator::end_change_populate_threads() {
  sys_pop_started.store(false, std::memory_order_seq_cst);
}

void Populator::rapid_print_thread_info(FILE *file){ /* in: output stream */
  fprintf(file, "rapid log pop thread : %s \n"
         "rapid log pop thread loops: " ULINTPF "\n",
         ShannonBase::Populate::sys_pop_started ? "running" : "stopped",
         ShannonBase::Populate::sys_rapid_loop_count);
}

}  // namespace Populate
}  // namespace ShannonBase