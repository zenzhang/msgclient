// Error.h

#ifndef _STREAMSDK_ERROR_H_
#define _STREAMSDK_ERROR_H_

namespace streamsdk
{
    namespace error {

        enum errors
        {
            success = 0,
            bind_udp_port_failed ,
            support_method,
            not_name_param,
            user_exist_before,
            connect_server_failed,
            json_format_error,
        };

        namespace detail {

            class http_category
                : public boost::system::error_category
            {
            public:
                http_category()
                {
                    register_category(*this);
                }

                const char* name() const BOOST_SYSTEM_NOEXCEPT
                {
                    return "sinamessage";
                }

                    std::string message(int value) const
                {
                    switch (value) {
                        // 翻译streamsdk的错误码
                    case success:
                        return "success";
                    case bind_udp_port_failed:
                        return "bind_udp_port_failed";
                    case support_method:
                        return "support_method";
                    case not_name_param:
                        return "not_name_param";
                    case user_exist_before:
                        return "user_exist_before";
                    case connect_server_failed:
                        return "connect_server_failed";
                    case json_format_error:
                        return "json_format_error";
                    default:
                        return "other error";
                    }
                }
            };

        } // namespace detail

        inline const boost::system::error_category & get_category()
        {
            static detail::http_category instance;
            return instance;
        }

        inline boost::system::error_code make_error_code(
            errors e)
        {
            return boost::system::error_code(
                static_cast<int>(e), get_category());
        }

    } // namespace error

} // namespace streamsdk

namespace boost
{
    namespace system
    {

        template<>
        struct is_error_code_enum<streamsdk::error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using streamsdk::error::make_error_code;
#endif

    }
}

#endif // _STREAMSDK_HTTP_ERROR_H_
