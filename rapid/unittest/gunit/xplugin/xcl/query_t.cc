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

#include <stdexcept>
#include <memory>

#include "plugin/x/client/mysqlxclient/xquery_result.h"
#include "plugin/x/client/xcontext.h"
#include "plugin/x/client/xquery_result_impl.h"
#include "unittest/gunit/xplugin/xcl/message_helpers.h"
#include "unittest/gunit/xplugin/xcl/mock/protocol.h"
#include "unittest/gunit/xplugin/xcl/mock/query_instance.h"


namespace xcl {
namespace test {

using ::testing::_;
using ::testing::Mock;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::Throw;

const Query_instances::Instance_id TEST_INSTANCE_ID = 1001;
const xcl::XProtocol::Handler_id   TEST_NOTICE_HANDLER_ID = 1002;

class Query_test_suite : public ::testing::Test {
 public:
  using Mock_query_instances_ptr = std::shared_ptr<Mock_query_instances>;
  using Mock_protocol_ptr        = std::shared_ptr<Mock_protocol>;
  using Context_ptr              = std::shared_ptr<Context>;

 public:
  void SetUp() override {
    ON_CALL(*m_mock_protocol, recv_single_message_raw(_, _))
      .WillByDefault(Throw(std::logic_error("Unexpected mock calls")));

    EXPECT_CALL(*m_mock_protocol, add_notice_handler(
        _,
        Handler_position::Begin,
        Handler_priority_medium))
            .WillOnce(Return(TEST_NOTICE_HANDLER_ID));
    EXPECT_CALL(*m_mock_query_instances, instances_fetch_begin())
        .WillOnce(Return(TEST_INSTANCE_ID));

    m_sut.reset(new Query_result(
        m_mock_protocol,
        m_mock_query_instances.get(),
        m_context));
  }

  void verifyMocks() {
    Mock::VerifyAndClearExpectations(m_mock_query_instances.get());
    Mock::VerifyAndClearExpectations(m_mock_protocol.get());
  }

  Context_ptr m_context =
      std::make_shared<Context>();

  Mock_query_instances_ptr m_mock_query_instances =
      std::make_shared<StrictMock<Mock_query_instances>>();

  Mock_protocol_ptr m_mock_protocol =
      std::make_shared<StrictMock<Mock_protocol>>();

  std::unique_ptr<XQuery_result> m_sut;
};


class Query_non_active_test_suite : public Query_test_suite {
 public:
  void SetUp() override {
    Query_test_suite::SetUp();

    EXPECT_CALL(*m_mock_query_instances, is_instance_active(TEST_INSTANCE_ID)).
        WillRepeatedly(Return(false));
  }
};

TEST_F(Query_non_active_test_suite, last_insert_id_not_initialized) {
  uint64_t out_result;

  ASSERT_FALSE(m_sut->try_get_last_insert_id(&out_result));
  ASSERT_FALSE(m_sut->try_get_last_insert_id(nullptr));
}

TEST_F(Query_non_active_test_suite, affected_rows_not_initialized) {
  uint64_t out_result;

  ASSERT_FALSE(m_sut->try_get_affected_rows(&out_result));
  ASSERT_FALSE(m_sut->try_get_affected_rows(nullptr));
}

TEST_F(Query_non_active_test_suite, info_message_not_initialized) {
  std::string out_result;

  ASSERT_FALSE(m_sut->try_get_info_message(&out_result));
  ASSERT_FALSE(m_sut->try_get_info_message(nullptr));
}

TEST_F(Query_non_active_test_suite, get_warnings_not_initialized) {
  ASSERT_EQ(0, m_sut->get_warnings().size());
}

TEST_F(Query_non_active_test_suite, get_metadata_previous_not_finished) {
  XError out_error;

  ASSERT_EQ(0, m_sut->get_metadata(&out_error).size());
  ASSERT_EQ(0, m_sut->get_metadata(nullptr).size());
  ASSERT_EQ(CR_X_LAST_COMMAND_UNFINISHED, out_error.error());
}

TEST_F(Query_non_active_test_suite, get_next_row2_previous_not_finished) {
  const XRow  *out_xrow;
  XError out_error;

  ASSERT_FALSE(m_sut->get_next_row(&out_xrow, &out_error));
  ASSERT_EQ(CR_X_LAST_COMMAND_UNFINISHED, out_error.error());

  ASSERT_FALSE(m_sut->get_next_row(nullptr, nullptr));
}

TEST_F(Query_non_active_test_suite, get_next_row1_previous_not_finished) {
  XError out_error;

  ASSERT_EQ(nullptr, m_sut->get_next_row(&out_error));
  ASSERT_EQ(CR_X_LAST_COMMAND_UNFINISHED, out_error.error());

  ASSERT_EQ(nullptr, m_sut->get_next_row(nullptr));
}

TEST_F(Query_non_active_test_suite, get_next_row_raw_previous_not_finished) {
  XError out_error;

  ASSERT_EQ(nullptr, m_sut->get_next_row_raw(&out_error).get());
  ASSERT_EQ(CR_X_LAST_COMMAND_UNFINISHED, out_error.error());

  ASSERT_EQ(nullptr, m_sut->get_next_row_raw(nullptr).get());
}

TEST_F(Query_non_active_test_suite, next_resultset_previous_not_finished) {
  XError out_error;

  ASSERT_FALSE(m_sut->next_resultset(&out_error));
  ASSERT_EQ(CR_X_LAST_COMMAND_UNFINISHED, out_error.error());
}

TEST_F(Query_non_active_test_suite, has_resultset_previous_not_finished) {
  XError out_error;

  ASSERT_FALSE(m_sut->has_resultset(&out_error));
  ASSERT_EQ(CR_X_LAST_COMMAND_UNFINISHED, out_error.error());
}

class Query_active_test_suite : public Query_test_suite {
 public:
  using Message_id = xcl::XProtocol::Server_message_type_id;

 public:
  void SetUp() override {
    Query_test_suite::SetUp();

    EXPECT_CALL(*m_mock_query_instances,
                is_instance_active(TEST_INSTANCE_ID)).
                    WillRepeatedly(Return(true));
  }

  void expect_query_finish() {
    EXPECT_CALL(*m_mock_query_instances,
                instances_fetch_end());
    EXPECT_CALL(*m_mock_protocol,
                remove_notice_handler(TEST_NOTICE_HANDLER_ID));

    EXPECT_CALL(*m_mock_query_instances,
                is_instance_active(TEST_INSTANCE_ID)).
                    WillRepeatedly(Return(true));
  }

  template <typename Message_type>
  void expect_recv_message(const std::string &msg_text) {
    auto *message = new Message_from_str<Message_type>(msg_text);

    const auto &message_options = message->descriptor()->options();
    const auto server_id = static_cast<int>(
        message_options.GetExtension(Mysqlx::server_message_id));

    EXPECT_CALL(*m_mock_protocol, recv_single_message_raw(_, _)).
        WillOnce(Invoke(
            [message, server_id](
                xcl::XProtocol::Server_message_type_id *out_mid,
                XError *out_error) -> xcl::XProtocol::Message* {
      if (out_mid)
        *out_mid = static_cast<Message_id>(server_id);

      if (out_error)
        *out_error = XError();

      return message;
    }));
  }

  template <typename Message_type>
  void expect_recv_message() {
    auto *message = new Message_type(Server_message<Message_type>::make_required());

    const auto &message_options = message->descriptor()->options();
    const auto server_id = static_cast<int>(
        message_options.GetExtension(Mysqlx::server_message_id));

    EXPECT_CALL(*m_mock_protocol, recv_single_message_raw(_, _)).
        WillOnce(Invoke(
            [message, server_id](
                xcl::XProtocol::Server_message_type_id *out_mid,
                XError *out_error) -> xcl::XProtocol::Message* {
      if (out_mid)
        *out_mid = static_cast<Message_id>(server_id);

      if (out_error)
        *out_error = XError();

      return message;
    }));
  }

  void verifyQuery_state_is_done(
      const int warnings_count = 0,
      const bool is_last_insert = false,
      const bool is_affected_rows = false,
      const bool is_info_message = false) {
    ASSERT_FALSE(m_sut->has_resultset(nullptr));
    ASSERT_FALSE(m_sut->next_resultset(nullptr));
    ASSERT_EQ(nullptr, m_sut->get_next_row(nullptr));
    ASSERT_EQ(warnings_count, m_sut->get_warnings().size());

    ASSERT_EQ(is_last_insert,   m_sut->try_get_last_insert_id(nullptr));
    ASSERT_EQ(is_affected_rows, m_sut->try_get_affected_rows(nullptr));
    ASSERT_EQ(is_info_message,  m_sut->try_get_info_message(nullptr));
  }
};

class Query_active_destructor_cleanup_test_suite :
    public Query_active_test_suite {};

TEST_F(Query_active_destructor_cleanup_test_suite, consumes_stmt_execute_ok) {
  InSequence s;

  expect_recv_message<::Mysqlx::Sql::StmtExecuteOk>("");
  expect_query_finish();
}

TEST_F(Query_active_destructor_cleanup_test_suite, consumes_done_and_ok) {
  InSequence s;

  expect_recv_message<::Mysqlx::Resultset::FetchDone>("");

  expect_recv_message<::Mysqlx::Sql::StmtExecuteOk>("");
  expect_query_finish();
}

TEST_F(Query_active_destructor_cleanup_test_suite,
       consumes_meta_data_and_done_and_ok) {
  InSequence s;

  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>("type:SINT");
  expect_recv_message<::Mysqlx::Resultset::FetchDone>("");

  expect_recv_message<::Mysqlx::Sql::StmtExecuteOk>("");
  expect_query_finish();
}

TEST_F(Query_active_destructor_cleanup_test_suite,
       consumes_meta_data_and_row_and_done_and_ok) {
  InSequence s;

  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>("type:SINT");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:''");
  expect_recv_message<::Mysqlx::Resultset::FetchDone>("");

  expect_recv_message<::Mysqlx::Sql::StmtExecuteOk>("");
  expect_query_finish();
}

TEST_F(Query_active_destructor_cleanup_test_suite,
       consumes_two_resultsets) {
  InSequence s;

  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>("type:SINT");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:''");
  expect_recv_message<::Mysqlx::Resultset::FetchDoneMoreResultsets>("");

  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>("type:DOUBLE");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:'1111'");
  expect_recv_message<::Mysqlx::Resultset::FetchDone>("");

  expect_recv_message<::Mysqlx::Sql::StmtExecuteOk>("");
  expect_query_finish();
}

TEST_F(Query_active_destructor_cleanup_test_suite,
       consumes_resultset_outparams) {
  InSequence s;

  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>("type:SINT");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:''");
  expect_recv_message<::Mysqlx::Resultset::FetchDoneMoreOutParams>("");

  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>("type:DOUBLE");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:'1111'");
  expect_recv_message<::Mysqlx::Resultset::FetchDone>("");

  expect_recv_message<::Mysqlx::Sql::StmtExecuteOk>("");
  expect_query_finish();
}


TEST_F(Query_active_destructor_cleanup_test_suite,
       consumes_multiple_resultset_outparams) {
  InSequence s;

  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>(
      "type:SINT name:'first'");
  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>(
      "type:UINT name:'second'");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:'1' field:'2'");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:'3' field:'4'");
  expect_recv_message<::Mysqlx::Resultset::FetchDoneMoreResultsets>("");

  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>(
      "type:SINT name:'next_1'");
  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>(
      "type:UINT name:'next_2'");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:'5' field:'6'");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:'8' field:'8'");
  expect_recv_message<::Mysqlx::Resultset::FetchDoneMoreOutParams>("");

  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>("type:DOUBLE");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:'1'");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:'2'");
  expect_recv_message<::Mysqlx::Resultset::FetchDoneMoreOutParams>("");

  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>("type:DOUBLE");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:'1'");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:'2'");
  expect_recv_message<::Mysqlx::Resultset::FetchDone>("");

  expect_recv_message<::Mysqlx::Sql::StmtExecuteOk>("");
  expect_query_finish();
}

TEST_F(Query_active_destructor_cleanup_test_suite, consumes_error) {
  expect_recv_message<::Mysqlx::Error>("code:1 sql_state:'' msg:'error'");
  expect_query_finish();
}

TEST_F(Query_active_destructor_cleanup_test_suite, consumes_error_after_meta) {
  InSequence s;

  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>(
      "type:SINT name:'first'");
  expect_recv_message<::Mysqlx::Error>("code:1 sql_state:'' msg:'error'");
  expect_query_finish();
}

using Unexpected_messages = ::testing::Types<
    ::Mysqlx::Ok,
    ::Mysqlx::Connection::Capabilities,
    ::Mysqlx::Session::AuthenticateOk,
    ::Mysqlx::Session::AuthenticateContinue>;

template <typename T>
class Typed_query_active_test_suite: public Query_active_test_suite {
 public:
  using Unexpected_message = T;
};

TYPED_TEST_CASE(Typed_query_active_test_suite, Unexpected_messages);

TYPED_TEST(Typed_query_active_test_suite,
           destructor_consumes_until_unexpected_message1) {
  InSequence s;

  //unexpected message breaks flow
  this->template expect_recv_message<TypeParam>();
  this->expect_query_finish();
}

TYPED_TEST(Typed_query_active_test_suite,
           destructor_consumes_until_unexpected_message2) {
  InSequence s;

  this->template expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>(
      "type:SINT name:'first'");
  this->template expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>(
      "type:SINT name:'second'");

  //unexpected message breaks flow
  this->template expect_recv_message<TypeParam>();
  this->expect_query_finish();
}

// Issue reproduction
// Query_result was not deregistring its instance after
// reception of StmtExecuteOk
TEST_F(Query_active_test_suite, no_resultset_at_call_has_resultset) {
  InSequence s;

  expect_recv_message<::Mysqlx::Sql::StmtExecuteOk>("");
  expect_query_finish();

  ASSERT_FALSE(m_sut->has_resultset());
  verifyMocks();

  verifyQuery_state_is_done();
}

TEST_F(Query_active_test_suite, no_resultset_at_call_get_metadata) {
  InSequence s;
  XError out_error;

  expect_recv_message<::Mysqlx::Sql::StmtExecuteOk>("");
  expect_query_finish();

  ASSERT_EQ(0, m_sut->get_metadata(&out_error).size());
  ASSERT_FALSE(out_error);
  verifyMocks();

  verifyQuery_state_is_done();
}

TEST_F(Query_active_test_suite, no_resultset_at_call_next_resultset) {
  InSequence s;
  XError out_error;

  expect_recv_message<::Mysqlx::Sql::StmtExecuteOk>("");
  expect_query_finish();

  ASSERT_FALSE(m_sut->next_resultset(&out_error));
  ASSERT_FALSE(out_error);
  verifyMocks();

  ASSERT_FALSE(m_sut->has_resultset(&out_error));
  ASSERT_FALSE(m_sut->next_resultset(nullptr));
  ASSERT_EQ(nullptr, m_sut->get_next_row(nullptr));
}

TEST_F(Query_active_test_suite, no_resultset_at_call_get_next_row) {
  InSequence s;
  XError out_error;

  expect_recv_message<::Mysqlx::Sql::StmtExecuteOk>("");
  expect_query_finish();

  ASSERT_EQ(nullptr, m_sut->get_next_row(&out_error));
  ASSERT_FALSE(out_error);
  verifyMocks();

  verifyQuery_state_is_done();
}


TEST_F(Query_active_test_suite, fetch_one_resultset) {
  InSequence s;
  XError out_error;

  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>("type:SINT");
  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>("type:DOUBLE");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:''");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:'1111'");
  expect_recv_message<::Mysqlx::Resultset::FetchDone>("");

  expect_recv_message<::Mysqlx::Sql::StmtExecuteOk>("");

  expect_query_finish();

  ASSERT_TRUE(m_sut->has_resultset(&out_error));
  ASSERT_FALSE(out_error);
  ASSERT_EQ(2, m_sut->get_metadata(&out_error).size());
  ASSERT_FALSE(out_error);

  ASSERT_NE(nullptr, m_sut->get_next_row(&out_error));
  ASSERT_FALSE(out_error);

  ASSERT_NE(nullptr, m_sut->get_next_row(&out_error));
  ASSERT_FALSE(out_error);

  ASSERT_EQ(nullptr, m_sut->get_next_row(&out_error));
  ASSERT_FALSE(out_error);

  ASSERT_FALSE(m_sut->next_resultset(&out_error));
  ASSERT_FALSE(m_sut->has_resultset(&out_error));

  verifyMocks();

  verifyQuery_state_is_done();
}

TEST_F(Query_active_test_suite, fetch_two_resultsetss) {
  InSequence s;
  XError out_error;

  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>("type:SINT");
  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>("type:DOUBLE");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:'' field:''");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:'1111' field:''");
  expect_recv_message<::Mysqlx::Resultset::FetchDoneMoreResultsets>("");

  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>("type:SINT");
  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>("type:DOUBLE");
  expect_recv_message<::Mysqlx::Resultset::ColumnMetaData>("type:UINT");
  expect_recv_message<::Mysqlx::Resultset::Row>("field:'' field:'' field:''");
  expect_recv_message<::Mysqlx::Resultset::FetchDone>("");

  expect_recv_message<::Mysqlx::Sql::StmtExecuteOk>("");

  expect_query_finish();

  // First resultset
  ASSERT_TRUE(m_sut->has_resultset(&out_error));
  ASSERT_FALSE(out_error);
  ASSERT_EQ(2, m_sut->get_metadata(&out_error).size());
  ASSERT_FALSE(out_error);

  ASSERT_NE(nullptr, m_sut->get_next_row(&out_error));
  ASSERT_FALSE(out_error);

  ASSERT_NE(nullptr, m_sut->get_next_row(&out_error));
  ASSERT_FALSE(out_error);

  ASSERT_EQ(nullptr, m_sut->get_next_row(&out_error));
  ASSERT_FALSE(out_error);

  // Second resultset
  ASSERT_TRUE(m_sut->next_resultset(&out_error));
  ASSERT_FALSE(out_error);
  ASSERT_EQ(3, m_sut->get_metadata(&out_error).size());
  ASSERT_FALSE(out_error);

  ASSERT_NE(nullptr, m_sut->get_next_row(&out_error));
  ASSERT_FALSE(out_error);

  ASSERT_EQ(nullptr, m_sut->get_next_row(&out_error));
  ASSERT_FALSE(out_error);

  ASSERT_FALSE(m_sut->next_resultset(&out_error));
  ASSERT_FALSE(m_sut->has_resultset(&out_error));

  verifyMocks();
  verifyQuery_state_is_done();
}

}  // namespace test
}  // namespace xcl
