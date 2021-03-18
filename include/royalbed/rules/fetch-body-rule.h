#pragma once

#include <cstddef>
#include <memory>
#include <string_view>

#include <corvusoft/restbed/rule.hpp>
#include <corvusoft/restbed/session.hpp>

namespace royalbed {

class FetchBodyRule final : public restbed::Rule
{
public:
    static constexpr std::size_t defaultSizeLimit = 1'000'000;

    explicit FetchBodyRule(std::string_view contentType, std::size_t sizeLimit = defaultSizeLimit);
    ~FetchBodyRule() override;

    bool condition(std::shared_ptr<restbed::Session> session) override;
    void action(std::shared_ptr<restbed::Session> session,
                const std::function<void(const std::shared_ptr<restbed::Session>)>& callback) override;

private:
    const std::string m_contentType;
    const std::size_t m_sizeLimit;
};

}   // namespace royalbed
