/*
  influxdb-cpp -- 💜 C++ client for InfluxDB.

  Copyright (c) 2010-2018 <http://ez8.co> <orca.zhang@yahoo.com>
  This library is released under the MIT License.

  Please see LICENSE file or visit https://github.com/orca-zhang/influxdb-cpp for details.
 */

// This library was modified from source to allow hex_char_array_t. And disable warnings. And allow ownership of sockets. And toggle awaiting responses. And fix upload bugs.

#ifndef INFLUXDB_CPP_HPP
#define INFLUXDB_CPP_HPP

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough="

#include <sstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <numeric>

// for testing
#include <iostream>

// for rclcpp
#include "rclcpp/measuring/hash_to_chars.hpp"
using rclcpp::hex_char_array_t;

#define DEFAULT_PRECISION 5

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
    #include <algorithm>
    #pragma comment(lib, "ws2_32")
    typedef struct iovec { void* iov_base; size_t iov_len; } iovec;
    inline __int64 writev(int sock, struct iovec* iov, int cnt) {
        __int64 r = send(sock, (const char*)iov->iov_base, iov->iov_len, 0);
        return (r < 0 || cnt == 1) ? r : r + writev(sock, iov + 1, cnt - 1);
    }
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/uio.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #define closesocket close
#endif

namespace influxdb_cpp {
    struct server_info {
        std::string host_;
        int port_;
        bool await_post_response_;
        std::string org_;
        std::string bkt_;
        std::string tkn_;
        struct addrinfo hints_, *res_=NULL;
        server_info(const std::string& host, int port, const std::string& org, const std::string& token, const std::string& bucket = "", bool await_post_response = true) {
            port_ = port;
            org_  = org;
            tkn_  = token;
            bkt_  = bucket;
            await_post_response_ = await_post_response;

            // please reference the IBM documentation for IPv4/IPv6 with questions
            // https://www.ibm.com/docs/en/i/7.2?topic=clients-example-ipv4-ipv6-client
            int resp = 0;

            struct in6_addr serveraddr;
            memset(&hints_, 0x00, sizeof(hints_));
            hints_.ai_flags    = AI_NUMERICSERV;
            hints_.ai_family   = AF_UNSPEC;
            hints_.ai_socktype = SOCK_STREAM;

            // check to see if the address is a valid IPv4 address
            resp = inet_pton(AF_INET, host.c_str(), &serveraddr);
            if (resp == 1){
                hints_.ai_family = AF_INET; // IPv4
                hints_.ai_flags |= AI_NUMERICHOST;
            
            // not a valid IPv4 -> check to see if address is a valid IPv6 address
            } else {
                resp = inet_pton(AF_INET6, host.c_str(), &serveraddr);
                if (resp == 1) {
                    hints_.ai_family = AF_INET6;  // IPv6
                    hints_.ai_flags |= AI_NUMERICHOST;

                }
            }

            resp = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints_, &res_);
            if (resp != 0) {
                std::cerr << "Host not found --> " << gai_strerror(resp) << std::endl;
                if (resp == EAI_SYSTEM)
                    std::cerr << "getaddrinfo() failed" << std::endl;
                exit(1);
            }

        }
    };

    struct influx_socket {
        int sock_;
        influx_socket() = delete;
        influx_socket(const server_info& si) {
            std::cout << " Begin influx socket open ... \n";
            // open the socket
            int sock = socket(si.res_->ai_family, si.res_->ai_socktype, si.res_->ai_protocol);
            if (sock < 0) {
                closesocket(sock);
                throw std::invalid_argument("influx_socket: socket() failed, the internal socket was closed.");
            }
            std::cout << " Socket created ... \n";

            // connect to the server
            int ret_code = connect(sock, si.res_->ai_addr, si.res_->ai_addrlen);
            if (ret_code < 0)
            {
                closesocket(sock);
                throw std::invalid_argument("influx_socket: connect() failed, the internal socket was closed.");
            }
            std::cout << " Socket connected!\n";

            sock_ = sock;
        }

        ~influx_socket() {
#ifdef _WIN32
            int shutHow = SD_BOTH;
#else 
            int shutHow = SHUT_RDWR;
#endif
            shutdown(sock_, shutHow);
            closesocket(sock_);
        }
    };

    namespace detail {
        struct meas_caller;
        struct tag_caller;
        struct field_caller;
        struct ts_caller;
        struct inner {
            static int http_request(const influx_socket&, const char*, const char*, const std::string&, const std::string&, const server_info&, std::string*);
            static inline unsigned char to_hex(unsigned char x) { return  x > 9 ? x + 55 : x + 48; }
            static void url_encode(std::string& out, const std::string& src);
        };
    }

    inline int flux_query(std::string& resp, const std::string& query, const server_info& si, const influx_socket& is) {

        // query JSON body
        std::stringstream body;
        body << "{\"query\": \"";
        body << query;
        body << "\", \"type\": \"flux\" }";

        return detail::inner::http_request(is, "POST", "query", "", body.str(), si, &resp);
    }

    struct builder {
        detail::tag_caller& meas(const std::string& m) {

            if (lines_.tellp() == std::streampos(0)) { // the stream has not been used yet, it is fresh.
                lines_.imbue(std::locale("C"));
                lines_.clear();
                return _m(m);
            } else { 
                // we are adding more measurements after already doing some work in the past, treat as ts_caller.meas().
                // The (short) function is duplicated instead of casting to the type and calling it, due to circular dependency issues.
                //
                // This is productive here, because the standard use-case of batching measurements looks like this:
                // .meas().field().timestamp().meas() ...
                // The timestamp() call will yield a ts_caller and go to the correct function.
                // However, if the building is distributed, for example:
                // 
                // auto memberVariable = builder();
                // void modifyBuilderA() { /* add to builder, then... */; maybe_upload();  }
                // void modifyBuilderB() { /* add to builder, then... */; maybe_upload(); }
                // void maybe_upload() { if (some_condition) { memberVariable.post_http(); } }
                //
                // Where A and B are called arbitrarily many times in an arbitrary order...
                //
                // Then there are difficulties. Because the type is internally changing in the method chaining,
                // The member variable cannot change from builder to ts_caller, without unions or hacking in polymorphism.
                // Additionally, re-assigning to the member variable at all is pain due to move semantics with stringstream.
                //
                // Hence this branch of the function lets users of the base type (i.e.. distributed building) continue building their request body correctly.

                lines_ << '\n'; 
                return _m(m);
            }
        }

        size_t getBufferSize() {
            std::streampos currentPos = lines_.tellp();
            lines_.seekp(0, std::ios::beg);
            std::streampos startPos = lines_.tellp();
            lines_.seekp(currentPos, std::ios::beg); // restore the stream to normal.

            return static_cast<size_t>(currentPos - startPos);
        }
        void clear() { 
            lines_.str({}); // set the buffer to a new (empty) string.
            lines_.clear();  // clear the error flags. Clear on its own does NOT empty a stringstream.
        }
    protected:
        detail::tag_caller& _m(const std::string& m) {
            _escape(m, ", ");
            return (detail::tag_caller&)*this;
        }
        detail::tag_caller& _t(const std::string& k, const std::string& v) {
            lines_ << ',';
            _escape(k, ",= ");
            lines_ << '=';
            _escape(v, ",= ");
            return (detail::tag_caller&)*this;
        }
        detail::tag_caller& _t(const std::string& k, const hex_char_array_t& v) {
            lines_ << ',';
            _escape(k, ",= ");
            lines_ << '=';
            lines_ << v;
            return (detail::tag_caller&)*this;
        }
        detail::field_caller& _f_s(char delim, const std::string& k, const std::string& v) {
#ifdef INFLUXCPP2_DEBUG
            std::cout << "KV: " << k << " " << v << std::endl;
#endif
            lines_ << delim;
            lines_ << std::fixed;
            _escape(k, ",= ");
            lines_ << "=\"";
            _escape(v, "\"");
            lines_ << '\"';
            return (detail::field_caller&)*this;
        }
        detail::field_caller& _f_h(char delim, const std::string& k, const hex_char_array_t& v) {
            lines_ << delim;
            lines_ << std::fixed;
            _escape(k, ",= ");
            lines_ << "=\"";
            lines_ << v; // no need to escape a hex string
            lines_ << '\"';
            return (detail::field_caller&)*this;
        }
        detail::field_caller& _f_i(char delim, const std::string& k, long long v) {
            lines_ << delim;
            lines_ << std::fixed;
            _escape(k, ",= ");
            lines_ << '=';
            lines_ << v << 'i';
            return (detail::field_caller&)*this;
        }
        detail::field_caller& _f_f(char delim, const std::string& k, double v, int prec) {
            lines_ << delim;
            _escape(k, ",= ");
            lines_ << std::fixed;
            lines_.precision(prec);
            lines_ << '=' << v;
            return (detail::field_caller&)*this;
        }
        detail::field_caller& _f_b(char delim, const std::string& k, bool v) {
            lines_ << delim;
            _escape(k, ",= ");
            lines_ << std::fixed;
            lines_ << '=' << (v ? 't' : 'f');
            return (detail::field_caller&)*this;
        }
        detail::ts_caller& _ts(long long ts) {
            lines_ << ' ' << ts;
            return (detail::ts_caller&)*this;
        }
        int _post_http(const server_info& si, const influx_socket& is, std::string* resp) {
            return detail::inner::http_request(is, "POST", "write", "", lines_.str(), si, resp);
        }
        void _escape(const std::string& src, const char* escape_seq) {
            size_t pos = 0, start = 0;
            while((pos = src.find_first_of(escape_seq, start)) != std::string::npos) {
                lines_.write(src.c_str() + start, pos - start);
                lines_ << '\\' << src[pos];
                start = ++pos;
            }
            lines_.write(src.c_str() + start, src.length() - start);
        }

        std::stringstream lines_;
    };

    namespace detail {
        struct tag_caller : public builder {
            detail::tag_caller& tag(const std::string& k, const std::string& v)       { return _t(k, v); }
            detail::tag_caller& tag(const std::string& k, const hex_char_array_t& v)  { return _t(k, v); }
            detail::field_caller& field(const std::string& k, const std::string& v)   { return _f_s(' ', k, v); }
            detail::field_caller& field(const std::string& k, const hex_char_array_t& v) { return _f_h(' ', k, v); }
            detail::field_caller& field(const std::string& k, bool v)                 { return _f_b(' ', k, v); }
            detail::field_caller& field(const std::string& k, short v)                { return _f_i(' ', k, v); }
            detail::field_caller& field(const std::string& k, int v)                  { return _f_i(' ', k, v); }
            detail::field_caller& field(const std::string& k, long v)                 { return _f_i(' ', k, v); }
            detail::field_caller& field(const std::string& k, long long v)            { return _f_i(' ', k, v); }
            detail::field_caller& field(const std::string& k, double v, int prec = DEFAULT_PRECISION) { return _f_f(' ', k, v, prec); }
        private:
            detail::tag_caller& meas(const std::string& m);
        };
        struct ts_caller : public builder {
            detail::tag_caller& meas(const std::string& m)                            { lines_ << '\n'; return _m(m); }
            int post_http(const server_info& si, const influx_socket& is, std::string* resp = NULL)            { return _post_http(si, is, resp); }
        };
        struct field_caller : public ts_caller {
            detail::field_caller& field(const std::string& k, const std::string& v)   { return _f_s(',', k, v); }
            detail::field_caller& field(const std::string& k, const hex_char_array_t& v) { return _f_h(',', k, v); }
            detail::field_caller& field(const std::string& k, bool v)                 { return _f_b(',', k, v); }
            detail::field_caller& field(const std::string& k, short v)                { return _f_i(',', k, v); }
            detail::field_caller& field(const std::string& k, int v)                  { return _f_i(',', k, v); }
            detail::field_caller& field(const std::string& k, long v)                 { return _f_i(',', k, v); }
            detail::field_caller& field(const std::string& k, long long v)            { return _f_i(',', k, v); }
            detail::field_caller& field(const std::string& k, double v, int prec = 2) { return _f_f(',', k, v, prec); }
            detail::ts_caller& timestamp(unsigned long long ts)                       { return _ts(ts); }
        };
        inline void inner::url_encode(std::string& out, const std::string& src) {
            size_t pos = 0, start = 0;
            while((pos = src.find_first_not_of("abcdefghijklmnopqrstuvwxyqABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.~", start)) != std::string::npos) {
                out.append(src.c_str() + start, pos - start);
                if(src[pos] == ' ')
                    out += "+";
                else {
                    out += '%';
                    out += to_hex((unsigned char)src[pos] >> 4);
                    out += to_hex((unsigned char)src[pos] & 0xF);
                }
                start = ++pos;
            }
            out.append(src.c_str() + start, src.length() - start);
        }
        inline int inner::http_request(const influx_socket& socket, const char* method, const char* uri,
            const std::string& querystring, const std::string& body, const server_info& si, std::string* resp) {
            std::string header;
            constexpr int iovec_len = 2;
            struct iovec iv[iovec_len];
            int ret_code = 0, content_length = 0, len = 0;
            char ch;
            unsigned char chunked = 0;

            int sock = socket.sock_; // copy of a file descriptor, use responsibly.

            header.resize(len = 0x100);

            for(;;) {
                iv[0].iov_len = snprintf(&header[0], len,
                    "%s /api/v2/%s?org=%s&bucket=%s%s HTTP/1.1\r\nHost: %s\r\nAuthorization:Token %s\r\nContent-Length: %d\r\n\r\n",
                    method, uri, si.org_.c_str(), si.bkt_.c_str(),
                    querystring.c_str(), si.host_.c_str(), si.tkn_.c_str(), (int)body.length());

#ifdef INFLUXCPP2_DEBUG
                    std::cout << printf(&header[0], len,
                    "%s /api/v2/%s?org=%s&bucket=%s%s HTTP/1.1\r\nHost: %s\r\nAuthorization:Token %s\r\nContent-Length: %d\r\n\r\n",
                    method, uri, si.org_.c_str(), si.bkt_.c_str(),
                    querystring.c_str(), si.host_.c_str(), si.tkn_.c_str(), (int)body.length()) << std::endl;
#endif
                if((int)iv[0].iov_len >= len)
                    header.resize(len *= 2);
                else
                    break;
            }
            iv[0].iov_base = &header[0];
            iv[1].iov_base = (void*)&body[0];
            iv[1].iov_len = body.length();

            // A loop is required to handle large writes. 
            // There is no guarantee enough is written after the first writev.
            // The send buffer size is dependent on the user's machine.
            // For example, it might be 21888 while a buffer larger than this needs to be sent.
            const int total_bytes = std::accumulate(iv, iv + iovec_len, 0, [](int acc, struct iovec iov){ return acc + iov.iov_len; });
            ssize_t total_written = 0;
            struct iovec * buffers = iv; // allow pointing past processed buffers.
            int processed_buffers = 0;
            do {
                // 1. do the write
                int buffers_remaining = iovec_len - processed_buffers;
                const ssize_t result = writev(sock, buffers, buffers_remaining);
//                std::cout << "socket (" << sock << "): " << result << " from writev, sending: " << total_bytes << " bytes\n";
                // 2. check for errors: -1 can be returned, and the specific error is indicated by errno.
                if (result < 0) {
                    std::cout << "influxdb upload error (" << strerror(errno) << ")\n";
                    ret_code = -6;
                    goto END;
                }

                // <!> updates the loop guard variable.
                total_written += result;
//                std::cout << total_written << " written, total: " << total_bytes << "\n";
                // early exit potential.
                if (total_written >= total_bytes) { 
                    break; 
                }

                // 3. adjust buffer pointers, since we are not done yet due to no early exit.
                size_t to_offset = static_cast<size_t>(result); // should be safe since result >= 0.
                while (to_offset >= buffers->iov_len) {
                    to_offset -= buffers->iov_len; // to_offset >= 0.
                    buffers += 1; // point to the next buffer.
                    processed_buffers += 1; // bookkeeping for writev.
                }

                // to_offset < buffers->iov_len, so adjust the struct to point to the remaining content.
                size_t remaining = buffers[0].iov_len - to_offset;
                void * buffer_ptr = buffers[0].iov_base;
                // so shift the pointer forward and reduce the length.
                buffers[0].iov_base = static_cast<void *>(static_cast<char *>(buffer_ptr) + to_offset);
                buffers[0].iov_len = remaining;
            } while (total_written < total_bytes);

            if (! si.await_post_response_) { // this avoids spending many MS waiting for a response, crucial option for some, as it lowers the time of this call down to microseconds.
                ret_code = 0;
                goto END;
            }

            iv[0].iov_len = len;

#define _NO_MORE() (len >= (int)iv[0].iov_len && \
    (iv[0].iov_len = recv(sock, &header[0], header.length(), len = 0)) == size_t(-1))
#define _GET_NEXT_CHAR() (ch = _NO_MORE() ? 0 : header[len++])
#define _LOOP_NEXT(statement) for(;;) { if(!(_GET_NEXT_CHAR())) { ret_code = -7; goto END; } statement }
#define _UNTIL(c) _LOOP_NEXT( if(ch == c) break; )
#define _GET_NUMBER(n) _LOOP_NEXT( if(ch >= '0' && ch <= '9') n = n * 10 + (ch - '0'); else break; )
#define _GET_CHUNKED_LEN(n, c) _LOOP_NEXT( if(ch >= '0' && ch <= '9') n = n * 16 + (ch - '0'); \
            else if(ch >= 'A' && ch <= 'F') n = n * 16 + (ch - 'A') + 10; \
            else if(ch >= 'a' && ch <= 'f') n = n * 16 + (ch - 'a') + 10; else {if(ch != c) { ret_code = -8; goto END; } break;} )
#define _(c) if((_GET_NEXT_CHAR()) != c) break;
#define __(c) if((_GET_NEXT_CHAR()) != c) { ret_code = -9; goto END; }

            if(resp) resp->clear();

            _UNTIL(' ')_GET_NUMBER(ret_code)
            for(;;) {
                _UNTIL('\n')
                switch(_GET_NEXT_CHAR()) {
                    case 'C':_('o')_('n')_('t')_('e')_('n')_('t')_('-')
                        _('L')_('e')_('n')_('g')_('t')_('h')_(':')_(' ')
                        _GET_NUMBER(content_length)
                        break;
                    case 'T':_('r')_('a')_('n')_('s')_('f')_('e')_('r')_('-')
                        _('E')_('n')_('c')_('o')_('d')_('i')_('n')_('g')_(':')
                        _(' ')_('c')_('h')_('u')_('n')_('k')_('e')_('d')
                        chunked = 1;
                        break;
                    case '\r':__('\n')
                        switch(chunked) {
                            do {__('\r')__('\n')
                            case 1:
                                _GET_CHUNKED_LEN(content_length, '\r')__('\n')
                                if(!content_length) {
                                    __('\r')__('\n')
                                    goto END;
                                }
                            case 0:
                                while(content_length > 0 && !_NO_MORE()) {
                                    content_length -= (iv[1].iov_len = std::min(content_length, (int)iv[0].iov_len - len));
                                    if(resp) resp->append(&header[len], iv[1].iov_len);
                                    len += iv[1].iov_len;
                                }
                            } while(chunked);
                        }
                        goto END;
                }
                if(!ch) {
                    ret_code = -10;
                    goto END;
                }
            }
            ret_code = -11;
        END:
            return ret_code / 100 == 2 ? 0 : ret_code;
#undef _NO_MORE
#undef _GET_NEXT_CHAR
#undef _LOOP_NEXT
#undef _UNTIL
#undef _GET_NUMBER
#undef _GET_CHUNKED_LEN
#undef _
#undef __
        }
    }
}

#pragma GCC diagnostic pop

#endif // INFLUXDB_CPP_HPP
