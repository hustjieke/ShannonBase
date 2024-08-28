/* Copyright (c) 2022, 2024, Oracle and/or its affiliates.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is designed to work with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have either included with
  the program or referenced in the documentation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License, version 2.0, for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef MYSQL_SERVER_METRICS_INSTRUMENT_SERVICE_IMP_H
#define MYSQL_SERVER_METRICS_INSTRUMENT_SERVICE_IMP_H

#include <mysql/components/services/psi_metric_service.h>
#include <mysql/plugin.h>

/**
  @file storage/perfschema/pfs_metrics_service_imp.h
  The performance schema implementation of server metrics instrument service.
*/
extern SERVICE_TYPE(psi_metric_v1)
    SERVICE_IMPLEMENTATION(performance_schema, psi_metric_v1);

void initialize_mysql_server_metrics_instrument_service();
void cleanup_mysql_server_metrics_instrument_service();

void pfs_register_meters_v1(PSI_meter_info_v1 *info, size_t count);
void pfs_unregister_meters_v1(PSI_meter_info_v1 *info, size_t count);
void pfs_register_change_notification_v1(
    meter_registration_changes_v1_t callback);
void pfs_unregister_change_notification_v1(
    meter_registration_changes_v1_t callback);
void pfs_send_change_notification_v1(const char *meter, MeterNotifyType change);

#endif /* MYSQL_SERVER_METRICS_INSTRUMENT_SERVICE_IMP_H */
