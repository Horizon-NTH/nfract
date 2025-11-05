#pragma once

#include <chrono>
#include <filesystem>
#include <initializer_list>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace nfract::test
{
    class ArgvBuilder
    {
    public:
        ArgvBuilder() = default;

        ArgvBuilder(std::initializer_list<std::string_view> args)
        {
            m_storage.reserve(args.size());
            for (const auto arg : args)
            {
                m_storage.emplace_back(arg);
            }
            rebuild_raw();
        }

        explicit ArgvBuilder(std::vector<std::string> args) :
            m_storage(std::move(args))
        {
            rebuild_raw();
        }

        [[nodiscard]] std::span<const char* const> span() const noexcept
        {
            return {m_raw.data(), m_raw.size()};
        }

        [[nodiscard]] const std::vector<std::string>& storage() const noexcept
        {
            return m_storage;
        }

        void push_back(std::string arg)
        {
            m_storage.emplace_back(std::move(arg));
            m_raw.push_back(m_storage.back().c_str());
        }

        void assign(std::vector<std::string> args)
        {
            m_storage = std::move(args);
            rebuild_raw();
        }

    private:
        void rebuild_raw()
        {
            m_raw.clear();
            m_raw.reserve(m_storage.size());
            for (auto& s : m_storage)
            {
                m_raw.push_back(s.c_str());
            }
        }

        std::vector<std::string> m_storage;
        std::vector<const char*> m_raw;
    };

    [[nodiscard]] inline std::filesystem::path make_unique_path(const std::string_view prefix, const std::string_view extension = {})
    {
        thread_local std::mt19937_64 rng{std::random_device{}()};
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        const auto random_value = rng();

        std::string filename(prefix);
        filename.append("-");
        filename.append(std::to_string(timestamp));
        filename.append("-");
        filename.append(std::to_string(random_value));

        return (std::filesystem::temp_directory_path() / filename).replace_extension(extension);
    }

    class TempFileGuard
    {
    public:
        explicit TempFileGuard(std::filesystem::path path) noexcept :
            m_path(std::move(path))
        {
        }

        TempFileGuard(const TempFileGuard&) = delete;
        TempFileGuard& operator=(const TempFileGuard&) = delete;

        TempFileGuard(TempFileGuard&& other) noexcept :
            m_path(std::exchange(other.m_path, {}))
        {
        }

        TempFileGuard& operator=(TempFileGuard&& other) noexcept
        {
            if (this != &other)
            {
                remove();
                m_path = std::exchange(other.m_path, {});
            }
            return *this;
        }

        ~TempFileGuard()
        {
            remove();
        }

        [[nodiscard]] const std::filesystem::path& path() const noexcept
        {
            return m_path;
        }

    private:
        void remove() const noexcept
        {
            if (!m_path.empty())
            {
                std::error_code ec;
                std::filesystem::remove(m_path, ec);
            }
        }

        std::filesystem::path m_path;
    };
}
