#include <string>

#include <corvusoft/restbed/request.hpp>
#include <nlohmann/json.hpp>

#include <royalbed/body.h>
#include <royalbed/http-error.h>

#include <gtest/gtest.h>

namespace {

using namespace royalbed;
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

    restbed::Request req;
    req.set_header("Content-Type", "application/json");
    req.set_body(nlohmann::to_string(json(etalon)));

    auto body = Body<TestStruct>(req);
    EXPECT_EQ(body.get(), etalon);
}

TEST(Body, InvalidContentType)   // NOLINT
{
    restbed::Request req;
    req.set_header("Content-Type", "application/json");
    req.set_body("InvalidBody");

    EXPECT_THROW(Body<TestStruct>{req}, HttpError);   // NOLINT
}

TEST(Body, InvalidBody)   // NOLINT
{
    restbed::Request req;
    req.set_header("Content-Type", "application/json");
    req.set_body("InvalidBody");

    EXPECT_THROW(Body<TestStruct>{req}, HttpError);   // NOLINT
}
