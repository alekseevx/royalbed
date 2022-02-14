#include <exception>
#include <memory>
#include <system_error>

#include "nhope/async/ao-context.h"
#include "nhope/async/timer.h"
#include "nhope/io/io-device.h"
#include "nhope/io/sock-addr.h"
#include "nhope/io/tcp.h"

class SlowSock final : public nhope::TcpSocket
{
public:
    explicit SlowSock(nhope::AOContext& parent)
      : m_aoCtx(parent)
    {}

    void read(gsl::span<std::uint8_t> buf, nhope::IOHandler handler) override
    {
        using namespace std::literals;

        nhope::setTimeout(m_aoCtx, 60min, [handler, n = buf.size()](auto) {
            handler(nullptr, n);
        });
    }

    void write(gsl::span<const std::uint8_t> data, nhope::IOHandler handler) override
    {
        using namespace std::literals;

        nhope::setTimeout(m_aoCtx, 60min, [handler, n = data.size()](auto) {
            handler(nullptr, n);
        });
    }

    [[nodiscard]] nhope::SockAddr localAddress() const override
    {
        return nhope::SockAddr::ipv4("127.0.0.1", 1);
    }

    [[nodiscard]] nhope::SockAddr peerAddress() const override
    {
        return nhope::SockAddr::ipv4("127.0.0.1", 2);
    }

    void shutdown(Shutdown /*unused*/) override
    {}

    static nhope::TcpSocketPtr create(nhope::AOContext& parent)
    {
        return std::make_unique<SlowSock>(parent);
    }

private:
    nhope::AOContext m_aoCtx;
};

class BrokenSock final : public nhope::TcpSocket
{
public:
    explicit BrokenSock(nhope::AOContext& parent)
      : m_aoCtx(parent)
    {}

    void read(gsl::span<std::uint8_t> /*buf*/, nhope::IOHandler handler) override
    {
        m_aoCtx.exec([handler] {
            const auto errCode = std::make_error_code(std::errc::io_error);
            handler(std::make_exception_ptr(std::system_error(errCode)), {});
        });
    }

    void write(gsl::span<const std::uint8_t> /*data*/, nhope::IOHandler handler) override
    {
        m_aoCtx.exec([handler] {
            const auto errCode = std::make_error_code(std::errc::io_error);
            handler(std::make_exception_ptr(std::system_error(errCode)), {});
        });
    }

    [[nodiscard]] nhope::SockAddr localAddress() const override
    {
        return nhope::SockAddr::ipv4("127.0.0.1", 1);
    }

    [[nodiscard]] nhope::SockAddr peerAddress() const override
    {
        return nhope::SockAddr::ipv4("127.0.0.1", 2);
    }

    void shutdown(Shutdown /*unused*/) override
    {}

    static nhope::TcpSocketPtr create(nhope::AOContext& parent)
    {
        return std::make_unique<BrokenSock>(parent);
    }

private:
    nhope::AOContext m_aoCtx;
};

class NullSock final : public nhope::TcpSocket
{
public:
    explicit NullSock(nhope::AOContext& parent)
      : m_aoCtx(parent)
    {}

    void write(gsl::span<const std::uint8_t> data, nhope::IOHandler handler) override
    {
        m_aoCtx.exec([this, n = data.size(), handler = std::move(handler)] {
            handler(nullptr, n);
        });
    }

    void read(gsl::span<std::uint8_t> /*buf*/, nhope::IOHandler handler) override
    {
        m_aoCtx.exec([this, handler = std::move(handler)] {
            handler(nullptr, 0);
        });
    }

    [[nodiscard]] nhope::SockAddr localAddress() const override
    {
        return nhope::SockAddr::ipv4("127.0.0.1", 1);
    }

    [[nodiscard]] nhope::SockAddr peerAddress() const override
    {
        return nhope::SockAddr::ipv4("127.0.0.1", 2);
    }

    void shutdown(Shutdown /*unused*/) override
    {}

    static nhope::TcpSocketPtr create(nhope::AOContext& parent)
    {
        return std::make_unique<BrokenSock>(parent);
    }

private:
    nhope::AOContext m_aoCtx;
};
