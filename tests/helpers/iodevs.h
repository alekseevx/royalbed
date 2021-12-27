#include <exception>
#include <memory>
#include <system_error>

#include "nhope/async/ao-context.h"
#include "nhope/async/timer.h"
#include "nhope/io/io-device.h"

class IOErrorReader final : public nhope::Reader
{
public:
    explicit IOErrorReader(nhope::AOContext& parent)
      : m_aoCtx(parent)
    {}

    void read(gsl::span<std::uint8_t> /*buf*/, nhope::IOHandler handler) override
    {
        m_aoCtx.exec([handler] {
            const auto errCode = std::make_error_code(std::errc::io_error);
            handler(std::make_exception_ptr(std::system_error(errCode)), {});
        });
    }

    static nhope::ReaderPtr create(nhope::AOContext& parent)
    {
        return std::make_unique<IOErrorReader>(parent);
    }

private:
    nhope::AOContext m_aoCtx;
};

class SlowIODevice final : public nhope::IODevice
{
public:
    explicit SlowIODevice(nhope::AOContext& parent)
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

    static nhope::IODevicePtr create(nhope::AOContext& parent)
    {
        return std::make_unique<SlowIODevice>(parent);
    }

private:
    nhope::AOContext m_aoCtx;
};

class IOErrorWritter final : public nhope::Writter
{
public:
    explicit IOErrorWritter(nhope::AOContext& parent)
      : m_aoCtx(parent)
    {}

    void write(gsl::span<const std::uint8_t> /*data*/, nhope::IOHandler handler) override
    {
        m_aoCtx.exec([handler] {
            const auto errCode = std::make_error_code(std::errc::io_error);
            handler(std::make_exception_ptr(std::system_error(errCode)), {});
        });
    }

    static nhope::WritterPtr create(nhope::AOContext& parent)
    {
        return std::make_unique<IOErrorWritter>(parent);
    }

private:
    nhope::AOContext m_aoCtx;
};
