#ifndef A_SHOCK_HTTP_H
#define A_SHOCK_HTTP_H

#include "shock/http.h"

class ShockHTTP;

typedef void (ShockOutput)(ShockHTTP&, void* closure, const char* data, size_t length);
typedef void (ShockDispatch)(ShockHTTP&,
                             void* closure,
                             shock_request_part part,
                             const char* data,
                             size_t length);

class ShockHTTP
{
private:
    shock_http _http;
    ShockDispatch* _dispatch;
    ShockOutput* _output;
    void* _closure;

    static void forwardDispatch(shock_http*,
                                void* closure,
                                shock_request_part part,
                                const char* data,
                                size_t length)
    {
        ShockHTTP* self = (ShockHTTP*)closure;
        self->_dispatch(*self, self->_closure, part, data, length);
    }

    static void forwardOutput(shock_http*,
                              void* closure,
                              const char* data,
                              size_t length)
    {
        ShockHTTP* self = (ShockHTTP*)closure;
        self->_output(*self, self->_closure, data, length);
    }

public:
    ShockHTTP() {}
    ShockHTTP(ShockDispatch* dispatch,
              ShockOutput* output,
              void* closure)
        : _dispatch(dispatch), _output(output), _closure(closure)
    {
        shock_http_init(&_http, forwardDispatch, forwardOutput, this);
    }

    void send(shock_response_part part, const char* data, size_t length)
    {
        shock_http_send(&_http, part, data, length);
    }

    size_t receive(const char* data, size_t length)
    {
        return shock_http_recv(&_http, data, length);
    }

    void drop()
    {
        shock_http_drop(&_http);
    }

    ~ShockHTTP()
    {
        shock_http_fini(&_http);
    }
};

#endif /* A_SHOCK_HTTP_H */
