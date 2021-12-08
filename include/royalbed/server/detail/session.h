#include <cstdint>
#include <memory>
#include <string>

#include "nhope/io/pushback-reader.h"
#include "royalbed/server/request-context.h"
#include "royalbed/server/router.h"
#include "spdlog/logger.h"

#include "nhope/async/ao-context-close-handler.h"
#include "nhope/async/ao-context.h"
#include "nhope/io/io-device.h"

namespace royalbed::server::detail {

class SessionCallback
{
public:
    virtual ~SessionCallback() = default;

    virtual void sessionFinished(bool keepAlive) = 0;
};

struct SessionParams
{
    std::uint32_t num;

    nhope::PushbackReader& in;
    nhope::Writter& out;
    std::shared_ptr<spdlog::logger> log;

    Router& router;
    SessionCallback& callback;
};

void startSession(nhope::AOContext& aoCtx, SessionParams&& params);

}   // namespace royalbed::server::detail
