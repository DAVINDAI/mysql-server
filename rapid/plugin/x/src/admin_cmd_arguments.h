/*
 * Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0,
 * as published by the Free Software Foundation.
 *
 * This program is also distributed with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms,
 * as designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have included with MySQL.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
 */

#ifndef X_SRC_ADMIN_CMD_ARGUMENTS_H_
#define X_SRC_ADMIN_CMD_ARGUMENTS_H_

#include <string>
#include <vector>

#include "plugin/x/src/admin_cmd_handler.h"

namespace xpl {

class Admin_command_arguments_list
    : public Admin_command_handler::Command_arguments {
 public:
  explicit Admin_command_arguments_list(const List &args);

  Admin_command_arguments_list &string_arg(const char *name,
                                           std::string *ret_value,
                                           const bool optional) override;
  Admin_command_arguments_list &string_list(const char *name,
                                            std::vector<std::string> *ret_value,
                                            const bool optional) override;
  Admin_command_arguments_list &sint_arg(const char *name, int64_t *ret_value,
                                         const bool optional) override;
  Admin_command_arguments_list &uint_arg(const char *name, uint64_t *ret_value,
                                         const bool optional) override;
  Admin_command_arguments_list &bool_arg(const char *name, bool *ret_value,
                                         const bool optional) override;
  Admin_command_arguments_list &docpath_arg(const char *name,
                                            std::string *ret_value,
                                            const bool optional) override;
  Admin_command_arguments_list &object_list(
      const char *name, std::vector<Command_arguments *> *ret_value,
      const bool optional, unsigned expected_members_count) override;

  bool is_end() const override;
  const ngs::Error_code &end() override;
  const ngs::Error_code &error() const override { return m_error; }

 protected:
  bool check_scalar_arg(const char *argname,
                        Mysqlx::Datatypes::Scalar::Type type,
                        const char *type_name, const bool optional);
  void arg_type_mismatch(const char *argname, int argpos, const char *type);

  const List &m_args;
  List::const_iterator m_current;
  ngs::Error_code m_error;
  int m_args_consumed;
};

class Admin_command_arguments_object
    : public Admin_command_handler::Command_arguments {
 public:
  using Object = ::Mysqlx::Datatypes::Object;
  explicit Admin_command_arguments_object(const List &args);
  explicit Admin_command_arguments_object(const Object &obj);

  Admin_command_arguments_object &string_arg(const char *name,
                                             std::string *ret_value,
                                             const bool optional) override;
  Admin_command_arguments_object &string_list(
      const char *name, std::vector<std::string> *ret_value,
      const bool optional) override;
  Admin_command_arguments_object &sint_arg(const char *name, int64_t *ret_value,
                                           const bool optional) override;
  Admin_command_arguments_object &uint_arg(const char *name,
                                           uint64_t *ret_value,
                                           const bool optional) override;
  Admin_command_arguments_object &bool_arg(const char *name, bool *ret_value,
                                           const bool optional) override;
  Admin_command_arguments_object &docpath_arg(const char *name,
                                              std::string *ret_value,
                                              const bool optional) override;
  Admin_command_arguments_object &object_list(
      const char *name, std::vector<Command_arguments *> *ret_value,
      const bool optional, unsigned expected_members_count) override;

  bool is_end() const override;
  const ngs::Error_code &end() override;
  const ngs::Error_code &error() const override { return m_error; }

 private:
  using Any = ::Mysqlx::Datatypes::Any;
  using Object_field_list =
      ::google::protobuf::RepeatedPtrField<Object::ObjectField>;

  template <typename H>
  void get_scalar_arg(const char *name, const bool optional, H *handler);
  template <typename H>
  void get_scalar_value(const Any &value, H *handler);
  const Object::ObjectField *get_object_field(const char *name, const bool optional);
  void set_error(const char *name);
  Admin_command_arguments_object *add_sub_object(const Object &object);

  const bool m_args_empty;
  const bool m_is_object;
  const Object &m_object;
  ngs::Error_code m_error;
  int m_args_consumed;
  std::vector<std::shared_ptr<Admin_command_arguments_object> > m_sub_objects;
};

}  // namespace xpl

#endif  // X_SRC_ADMIN_CMD_ARGUMENTS_H_
