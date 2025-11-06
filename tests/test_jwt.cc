// Copyright (C) 2025 Ian Torres <iantorres@outlook.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <gtest/gtest.h>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <engine/cipher.hpp>
#include <engine/encoding.hpp>
#include <engine/errors/parse_error.hpp>
#include <engine/errors/signature_error.hpp>
#include <engine/jwt.hpp>
#include <engine/support.hpp>

using namespace engine;

TEST(jwt, can_be_generated) {
  bool throws = false;
  try {
    const std::string _key = base64_decode(generate_sha_256());
    const uuid _id = boost::uuids::random_generator()();
    const auto _jwt = jwt::make(_id, _key);
  } catch (...) {
    throws = true;
  }
  ASSERT_FALSE(throws);
}

TEST(jwt, throws_error_on_invalid_token) {
  const std::string _key = base64_decode(generate_sha_256());
  const std::string _bearer = "Bearer a.b.c";

  bool throws = false;
  try {
    const auto _jwt = jwt::from(_bearer, _key);
  } catch (...) {
    throws = true;
  }

  ASSERT_TRUE(throws);
}

TEST(jwt, can_parse_tokens) {
  const std::string _key = base64url_decode("-66WcolkZd8-oHejFFj1EUhxg3-8UWErNkgMqCwLDEI");
  const std::string _bearer =
      "Bearer "
      "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
      "eyJzdWIiOiI4MDdkOWEyNy04MjI2LTQ4OWUtOGZmNC1kY2ZkOTAyY2NkZTYiLCJpYXQiOjE3"
      "NjI0NDUwNDcsImp0aSI6ImM0NDQ3NTY0LTRhYzktNGU1Yy1hZTE1LWJkMTk2Y2VlMjliYiJ9"
      ".2dV1qpXyN0S9VWiYzB92x7w1EG9R7I_jWn9C9ppfgow";
  bool throws = false;
  try {
    const auto _jwt = jwt::from(_bearer, _key);
    ASSERT_EQ(to_string(_jwt->get_id()), "c4447564-4ac9-4e5c-ae15-bd196cee29bb");
    ASSERT_EQ(to_string(_jwt->get_sub()), "807d9a27-8226-489e-8ff4-dcfd902ccde6");
    ASSERT_EQ(_jwt->get_payload().at("iat").as_int64(), 1762445047);
  } catch (errors::parse_error &) {
    throws = true;
  } catch (errors::signature_error &) {
    throws = true;
  }
  ASSERT_FALSE(throws);
}
