/*
  Copyright (c) 2023, 2024, Oracle and/or its affiliates.

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
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "classic_auth_sha256_password_sender.h"

#include "auth_digest.h"
#include "classic_frame.h"
#include "hexify.h"
#include "mysql/harness/logging/logging.h"
#include "mysqld_error.h"  // mysql-server error-codes

IMPORT_LOG_FUNCTIONS()

// sender

stdx::expected<Processor::Result, std::error_code> AuthSha256Sender::process() {
  switch (stage()) {
    case Stage::Init:
      return init();
    case Stage::Response:
      return response();
    case Stage::PublicKey:
      return public_key();
    case Stage::Error:
      return error();
    case Stage::Ok:
      return ok();
    case Stage::Done:
      return Result::Done;
  }

  harness_assert_this_should_not_execute();
}

stdx::expected<Processor::Result, std::error_code> AuthSha256Sender::init() {
  auto &dst_conn = connection()->server_conn();
  auto &dst_channel = dst_conn.channel();
  auto &dst_protocol = dst_conn.protocol();

  if (dst_channel.ssl() || password_.empty()) {
    if (auto &tr = tracer()) {
      tr.trace(
          Tracer::Event().stage("sha256_password::sender::plaintext_password"));
    }

    auto send_res = ClassicFrame::send_msg(
        dst_conn, classic_protocol::borrowed::message::client::AuthMethodData{
                      password_ + '\0'});
    if (!send_res) return send_server_failed(send_res.error());

    stage(Stage::Response);
  } else {
    if (auto &tr = tracer()) {
      tr.trace(
          Tracer::Event().stage("sha256_password::sender::public_key_request"));
    }
    auto send_res = Auth::send_public_key_request(dst_channel, dst_protocol);
    if (!send_res) return send_server_failed(send_res.error());

    stage(Stage::PublicKey);
  }

  return Result::SendToServer;
}

stdx::expected<Processor::Result, std::error_code>
AuthSha256Sender::response() {
  // ERR|OK|EOF|other
  auto &src_conn = connection()->server_conn();
  auto &src_channel = src_conn.channel();
  auto &src_protocol = src_conn.protocol();

  // ensure the recv_buf has at last frame-header (+ msg-byte)
  auto read_res = ClassicFrame::ensure_has_msg_prefix(src_conn);
  if (!read_res) return recv_server_failed(read_res.error());

  const uint8_t msg_type = src_protocol.current_msg_type().value();

  enum class Msg {
    Ok = ClassicFrame::cmd_byte<classic_protocol::message::server::Ok>(),
    Error = ClassicFrame::cmd_byte<classic_protocol::message::server::Error>(),
  };

  switch (Msg{msg_type}) {
    case Msg::Ok:
      stage(Stage::Ok);
      return Result::Again;
    case Msg::Error:
      stage(Stage::Error);
      return Result::Again;
  }

  // if there is another packet, dump its payload for now.
  const auto &recv_buf = src_channel.recv_plain_view();

  // get as much data of the current frame from the recv-buffers to log it.
  (void)ClassicFrame::ensure_has_full_frame(src_conn);

  log_debug("received unexpected message from server in sha256-auth:\n%s",
            mysql_harness::hexify(recv_buf).c_str());
  return recv_server_failed(make_error_code(std::errc::bad_message));
}

stdx::expected<Processor::Result, std::error_code>
AuthSha256Sender::public_key() {
  auto &dst_conn = connection()->server_conn();
  auto &dst_channel = dst_conn.channel();
  auto &dst_protocol = dst_conn.protocol();

  auto msg_res = ClassicFrame::recv_msg<
      classic_protocol::borrowed::message::server::AuthMethodData>(dst_conn);
  if (!msg_res) return recv_server_failed(msg_res.error());

  auto msg = *msg_res;

  auto pubkey_res = Auth::public_key_from_pem(msg.auth_method_data());
  if (!pubkey_res) return recv_server_failed(pubkey_res.error());

  // discard _after_ msg. is used as msg borrows from the dst_channel's
  // recv_buffer.
  discard_current_msg(dst_conn);

  auto nonce = initial_server_auth_data_;

  // if there is a trailing zero, strip it.
  if (nonce.size() == Auth::kNonceLength + 1 &&
      nonce[Auth::kNonceLength] == 0x00) {
    nonce = nonce.substr(0, Auth::kNonceLength);
  }

  auto encrypted_res =
      Auth::rsa_encrypt_password(*pubkey_res, password_, nonce);
  if (!encrypted_res) return send_server_failed(encrypted_res.error());

  auto send_res =
      Auth::send_encrypted_password(dst_channel, dst_protocol, *encrypted_res);
  if (!send_res) return send_server_failed(send_res.error());

  stage(Stage::Response);

  return Result::SendToServer;
}

stdx::expected<Processor::Result, std::error_code> AuthSha256Sender::ok() {
  stage(Stage::Done);

  if (auto &tr = tracer()) {
    tr.trace(Tracer::Event().stage("sha256_password::sender::ok"));
  }

  return Result::Again;
}

stdx::expected<Processor::Result, std::error_code> AuthSha256Sender::error() {
  stage(Stage::Done);

  if (auto &tr = tracer()) {
    tr.trace(Tracer::Event().stage("sha256_password::sender::error"));
  }

  return Result::Again;
}
