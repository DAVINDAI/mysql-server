# Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

ADD_SUBDIRECTORY(libmysqlgcs)

SET(GROUP_REPLICATION_SOURCES
  src/applier.cc
  src/auto_increment.cc
  src/certifier.cc
  src/channel_observation_manager.cc
  src/delayed_plugin_initialization.cc
  src/gcs_event_handlers.cc
  src/gcs_logger.cc
  src/gcs_operations.cc
  src/gcs_plugin_messages.cc
  src/gcs_view_modification_notifier.cc
  src/group_partition_handling.cc
  src/member_info.cc
  src/observer_server_actions.cc
  src/observer_server_channels.cc
  src/observer_server_state.cc
  src/observer_trans.cc
  src/pipeline_factory.cc
  src/pipeline_stats.cc
  src/plugin.cc
  src/plugin_psi.cc
  src/plugin_utils.cc
  src/ps_information.cc
  src/read_mode_handler.cc
  src/recovery.cc
  src/recovery_channel_state_observer.cc
  src/applier_channel_state_observer.cc
  src/asynchronous_channels_state_observer.cc
  src/recovery_message.cc
  src/recovery_state_transfer.cc
  src/replication_threads_api.cc
  src/single_primary_message.cc
  src/member_version.cc
  src/compatibility_module.cc
  src/handlers/applier_handler.cc
  src/handlers/certification_handler.cc
  src/handlers/event_cataloger.cc
  src/sql_service/sql_resultset.cc
  src/sql_service/sql_service_context.cc
  src/sql_service/sql_service_context_base.cc
  src/sql_service/sql_service_interface.cc
  src/sql_service/sql_command_test.cc
  src/sql_service/sql_service_command.cc
  src/services/notification/notification.cc
  src/services/registry.cc
)

SET(GMS_LISTENER_SERVICE_TEST_SOURCES
    src/services/notification/impl/gms_listener_test.cc)

ADD_DEFINITIONS(-DMYSQL_SERVER)

IF(WITH_UNIT_TESTS)
  ADD_CONVENIENCE_LIBRARY(gr_unit_test_resource
    src/gcs_plugin_messages.cc
    src/member_info.cc
    src/member_version.cc
    src/compatibility_module.cc
  )
  SET_TARGET_PROPERTIES(gr_unit_test_resource
                        PROPERTIES
                        COMPILE_DEFINITIONS "DISABLE_PSI_MUTEX")
ENDIF()

### Configuration ###

# declare the plugin itself
MYSQL_ADD_PLUGIN(group_replication
                 ${GROUP_REPLICATION_SOURCES}
                 ${GMS_LISTENER_SERVICE_TEST_SOURCES}
                 LINK_LIBRARIES
                 ${LZ4_LIBRARY}
                 ${SSL_LIBRARIES}
                 ${GCS_LIBRARY}
                 MODULE_ONLY MODULE_OUTPUT_NAME "group_replication")

### INSTALLATION ###

IF(INSTALL_MYSQLTESTDIR)
  INSTALL(
    FILES tests/mtr/my.cnf
          tests/mtr/rpl_1slave_base.cnf
    DESTINATION "${INSTALL_MYSQLTESTDIR}/suite/group_replication"
    COMPONENT Test
  )
  INSTALL(
    DIRECTORY tests/mtr/t tests/mtr/r tests/mtr/inc tests/mtr/extra
    DESTINATION "${INSTALL_MYSQLTESTDIR}/suite/group_replication"
    USE_SOURCE_PERMISSIONS
    COMPONENT Test
    PATTERN "collections" EXCLUDE
  )
ENDIF()
