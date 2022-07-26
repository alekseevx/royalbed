#include <exception>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include "nhope/async/ao-context.h"
#include "nhope/async/timer.h"
#include "nhope/io/io-device.h"
#include "nhope/io/sock-addr.h"
#include "nhope/io/tcp.h"
#include "nhope/io/pushback-reader.h"
#include "nhope/io/string-reader.h"

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

inline nhope::PushbackReaderPtr inputStream(nhope::AOContext& aoCtx, std::string request)
{
    using namespace nhope;
    return PushbackReader::create(aoCtx, StringReader::create(aoCtx, std::move(request)));
}

class EchoSock final : public nhope::TcpSocket
{
public:
    explicit EchoSock(nhope::AOContext& parent, const std::string& data)
      : m_aoCtx(parent)
    {
        m_buffer.insert(m_buffer.end(), data.begin(), data.end());
    }

    void write(gsl::span<const std::uint8_t> data, nhope::IOHandler handler) override
    {
        m_aoCtx.exec([this, n = data, handler = std::move(handler)] {
            m_buffer.insert(m_buffer.end(), n.begin(), n.end());
            handler(nullptr, n.size());
        });
    }

    void read(gsl::span<std::uint8_t> buf, nhope::IOHandler handler) override
    {
        m_aoCtx.exec([this, buf, handler = std::move(handler)] {
            auto m = std::min(buf.size(), m_buffer.size());
            m_buffer.erase(m_buffer.begin(), m_buffer.begin() + m);
            handler(nullptr, m);
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

    static nhope::TcpSocketPtr create(nhope::AOContext& parent, const std::string& data)
    {
        return std::make_unique<EchoSock>(parent, data);
    }

private:
    std::vector<std::uint8_t> m_buffer;
    nhope::AOContext m_aoCtx;
};
