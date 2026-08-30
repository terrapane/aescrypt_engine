/*
 *  string_stream_buffer.h
 *
 *  Copyright (C) 2024, 2026
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements a simple custom StringStreamBuffer object
 *      used to facilitate testing.  While it is referred to as
 *      a "string" buffer, it can receive a span of characters or uint8_t.
 *      The buffer provided must live at least as long as this object.
 *
 *  Portability Issues:
 *      None.
 */

#pragma once

#include <sstream>
#include <span>
#include <string>
#include <cstdint>

// Define the StringStreamBuffer class used for reading the buffer content
class StringStreamBuffer : public std::streambuf
{
    public:
        explicit StringStreamBuffer(std::span<const char> buffer)
        {
            // Internal const_cast is encapsulated here and safe because
            // setp() is not called; the stream cannot write
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
            auto *p = const_cast<char *>(buffer.data());
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            setg(p, p, p + buffer.size());
        }

        explicit StringStreamBuffer(std::span<char> buffer)
        {
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            setg(buffer.data(), buffer.data(), buffer.data() + buffer.size());
            setp(buffer.data(), buffer.data() + buffer.size());
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        explicit StringStreamBuffer(std::span<const std::uint8_t> buffer) :
            StringStreamBuffer(
                std::span(reinterpret_cast<const char *>(buffer.data()),
                          buffer.size()))
        {
        }

        explicit StringStreamBuffer(std::span<std::uint8_t> buffer) :
            StringStreamBuffer(
                std::span(reinterpret_cast<char *>(buffer.data()),
                          buffer.size()))
        {
        }

        explicit StringStreamBuffer(std::string &buffer) :
            StringStreamBuffer(
                std::span(reinterpret_cast<char *>(buffer.data()),
                          buffer.size()))
        {
        }

        explicit StringStreamBuffer(const std::string &buffer) :
            StringStreamBuffer(
                std::span(reinterpret_cast<const char *>(buffer.data()),
                          buffer.size()))
        {
        }

    protected:
        pos_type seekoff(off_type off,
                         std::ios_base::seekdir dir,
                         std::ios_base::openmode which =
                             std::ios_base::in | std::ios_base::out) override
        {
            if (pbase() == nullptr) which &= ~std::ios_base::out;

            if (dir == std::ios_base::cur)
            {
                if ((which & std::ios_base::in) != 0)
                {
                    gbump(static_cast<int>(off));
                }
                if ((which & std::ios_base::out) != 0)
                {
                    pbump(static_cast<int>(off));
                }
            }
            else if (dir == std::ios_base::end)
            {
                if ((which & std::ios_base::in) != 0)
                {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    setg(eback(), egptr() + off, egptr());
                }
                if ((which & std::ios_base::out) != 0)
                {
                    pbump(static_cast<int>(epptr() - pptr() + off));
                }
            }
            else if (dir == std::ios_base::beg)
            {
                if ((which & std::ios_base::in) != 0)
                {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    setg(eback(), eback() + off, egptr());
                }
                if ((which & std::ios_base::out) != 0)
                {
                    pbump(static_cast<int>(pbase() - pptr() + off));
                }
            }
            else
            {
                // Return error on invalid seekdir
                return static_cast<off_type>(-1);
            }

            // Return active stream head position based on operation type
            if ((which & std::ios_base::in) != 0) return gptr() - eback();
            if ((which & std::ios_base::out) != 0) return pptr() - pbase();

            return static_cast<off_type>(-1);
        }

        pos_type seekpos(pos_type pos,
                         std::ios_base::openmode which =
                             std::ios_base::in | std::ios_base::out) override
        {
            return seekoff(pos, std::ios_base::beg, which);
        }
};
