#include <string>

#include "nhope/async/ao-context.h"
#include "nhope/async/thread-executor.h"
#include "nhope/io/string-reader.h"

#include "spdlog/spdlog.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "royalbed/server/request-context.h"
#include "royalbed/server/router.h"

#include <royalbed/common/body.h>

namespace {

using namespace royalbed::common;
using nlohmann::json;

struct TestStruct
{
    int val1;
    std::string val2;

    bool operator==(const TestStruct& other) const
    {
        return val1 == other.val1 && val2 == other.val2;
    }
};

void from_json(const nlohmann::json& json, TestStruct& value)
{
    json.at("val1").get_to(value.val1);
    json.at("val2").get_to(value.val2);
}

void to_json(nlohmann::json& json, const TestStruct& value)
{
    json = {{"val1", value.val1}, {"val2", value.val2}};
}

}   // namespace

TEST(Body, ValidBody)   // NOLINT
{
    const TestStruct etalon = {100, "text text"};
    auto body = Body<TestStruct>(etalon);
    EXPECT_EQ(body.get(), etalon);
}

TEST(Body, InvalidBody)   // NOLINT
{
    nhope::ThreadExecutor th;
    nhope::AOContext ao(th);

    Request req;
    req.headers.emplace("Content-Type", "application/json");
    req.body = nhope::StringReader::create(ao, "InvalidBody");
    EXPECT_THROW(royalbed::common::parseBody<TestStruct>(req.headers, {1, 3, 3}), HttpError);   // NOLINT
}

TEST(Body, InvalidContentType)   // NOLINT
{
    nhope::ThreadExecutor th;
    nhope::AOContext ao(th);
    const TestStruct etalon = {100, "text text"};

    Request req;
    req.headers.emplace("Content-Type", "application/jpeg");
    req.body = nhope::StringReader::create(ao, nlohmann::to_string(json(etalon)));
    EXPECT_THROW(royalbed::common::extractBodyType(req.headers), HttpError);   // NOLINT
}
