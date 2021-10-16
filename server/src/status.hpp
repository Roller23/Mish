#if !defined(__STATUS_)
#define __STATUS_

#include <string>

// HTTP status codes as registered with IANA.
// See: https://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml

class Status {
  public:
    static const int Continue           = 100; // RFC 7231, 6.2.1
    static const int SwitchingProtocols = 101; // RFC 7231, 6.2.2
    static const int Processing         = 102; // RFC 2518, 10.1
    static const int EarlyHints         = 103; // RFC 8297

    static const int OK                   = 200; // RFC 7231, 6.3.1
    static const int Created              = 201; // RFC 7231, 6.3.2
    static const int Accepted             = 202; // RFC 7231, 6.3.3
    static const int NonAuthoritativeInfo = 203; // RFC 7231, 6.3.4
    static const int NoContent            = 204; // RFC 7231, 6.3.5
    static const int ResetContent         = 205; // RFC 7231, 6.3.6
    static const int PartialContent       = 206; // RFC 7233, 4.1
    static const int MultiStatus          = 207; // RFC 4918, 11.1
    static const int AlreadyReported      = 208; // RFC 5842, 7.1
    static const int IMUsed               = 226; // RFC 3229, 10.4.1

    static const int MultipleChoices   = 300; // RFC 7231, 6.4.1
    static const int MovedPermanently  = 301; // RFC 7231, 6.4.2
    static const int Found             = 302; // RFC 7231, 6.4.3
    static const int SeeOther          = 303; // RFC 7231, 6.4.4
    static const int NotModified       = 304; // RFC 7232, 4.1
    static const int UseProxy          = 305; // RFC 7231, 6.4.5
    static const int TemporaryRedirect = 307; // RFC 7231, 6.4.7
    static const int PermanentRedirect = 308; // RFC 7538, 3

    static const int BadRequest                   = 400; // RFC 7231, 6.5.1
    static const int Unauthorized                 = 401; // RFC 7235, 3.1
    static const int PaymentRequired              = 402; // RFC 7231, 6.5.2
    static const int Forbidden                    = 403; // RFC 7231, 6.5.3
    static const int NotFound                     = 404; // RFC 7231, 6.5.4
    static const int MethodNotAllowed             = 405; // RFC 7231, 6.5.5
    static const int NotAcceptable                = 406; // RFC 7231, 6.5.6
    static const int ProxyAuthRequired            = 407; // RFC 7235, 3.2
    static const int RequestTimeout               = 408; // RFC 7231, 6.5.7
    static const int Conflict                     = 409; // RFC 7231, 6.5.8
    static const int Gone                         = 410; // RFC 7231, 6.5.9
    static const int LengthRequired               = 411; // RFC 7231, 6.5.10
    static const int PreconditionFailed           = 412; // RFC 7232, 4.2
    static const int RequestEntityTooLarge        = 413; // RFC 7231, 6.5.11
    static const int RequestURITooLong            = 414; // RFC 7231, 6.5.12
    static const int UnsupportedMediaType         = 415; // RFC 7231, 6.5.13
    static const int RequestedRangeNotSatisfiable = 416; // RFC 7233, 4.4
    static const int ExpectationFailed            = 417; // RFC 7231, 6.5.14
    static const int Teapot                       = 418; // RFC 7168, 2.3.3
    static const int MisdirectedRequest           = 421; // RFC 7540, 9.1.2
    static const int UnprocessableEntity          = 422; // RFC 4918, 11.2
    static const int Locked                       = 423; // RFC 4918, 11.3
    static const int FailedDependency             = 424; // RFC 4918, 11.4
    static const int TooEarly                     = 425; // RFC 8470, 5.2.
    static const int UpgradeRequired              = 426; // RFC 7231, 6.5.15
    static const int PreconditionRequired         = 428; // RFC 6585, 3
    static const int TooManyRequests              = 429; // RFC 6585, 4
    static const int RequestHeaderFieldsTooLarge  = 431; // RFC 6585, 5
    static const int UnavailableForLegalReasons   = 451; // RFC 7725, 3

    static const int InternalServerError           = 500; // RFC 7231, 6.6.1
    static const int NotImplemented                = 501; // RFC 7231, 6.6.2
    static const int BadGateway                    = 502; // RFC 7231, 6.6.3
    static const int ServiceUnavailable            = 503; // RFC 7231, 6.6.4
    static const int GatewayTimeout                = 504; // RFC 7231, 6.6.5
    static const int HTTPVersionNotSupported       = 505; // RFC 7231, 6.6.6
    static const int VariantAlsoNegotiates         = 506; // RFC 2295, 8.1
    static const int InsufficientStorage           = 507; // RFC 4918, 11.5
    static const int LoopDetected                  = 508; // RFC 5842, 7.2
    static const int NotExtended                   = 510; // RFC 2774, 7
    static const int NetworkAuthenticationRequired = 511; // RFC 6585, 6

    static std::string to_string(const int code) {
      switch (code) {
        case Continue:
          return "Continue";
        case SwitchingProtocols:
          return "Switching Protocols";
        case Processing:
          return "Processing";
        case EarlyHints:
          return "Early Hints";
        case OK:
          return "OK";
        case Created:
          return "Created";
        case Accepted:
          return "Accepted";
        case NonAuthoritativeInfo:
          return "Non-Authoritative Information";
        case NoContent:
          return "No Content";
        case ResetContent:
          return "Reset Content";
        case PartialContent:
          return "Partial Content";
        case MultiStatus:
          return "Multi-Status";
        case AlreadyReported:
          return "Already Reported";
        case IMUsed:
          return "IM Used";
        case MultipleChoices:
          return "Multiple Choices";
        case MovedPermanently:
          return "Moved Permanently";
        case Found:
          return "Found";
        case SeeOther:
          return "See Other";
        case NotModified:
          return "Not Modified";
        case UseProxy:
          return "Use Proxy";
        case TemporaryRedirect:
          return "Temporary Redirect";
        case PermanentRedirect:
          return "Permanent Redirect";
        case BadRequest:
          return "Bad Request";
        case Unauthorized:
          return "Unauthorized";
        case PaymentRequired:
          return "Payment Required";
        case Forbidden:
          return "Forbidden";
        case NotFound:
          return "Not Found";
        case MethodNotAllowed:
          return "Method Not Allowed";
        case NotAcceptable:
          return "Not Acceptable";
        case ProxyAuthRequired:
          return "Proxy Authentication Required";
        case RequestTimeout:
          return "Request Timeout";
        case Conflict:
          return "Conflict";
        case Gone:
          return "Gone";
        case LengthRequired:
          return "Length Required";
        case PreconditionFailed:
          return "Precondition Failed";
        case RequestEntityTooLarge:
          return "Request Entity Too Large";
        case RequestURITooLong:
          return "Request URI Too Long";
        case UnsupportedMediaType:
          return "Unsupported Media Type";
        case RequestedRangeNotSatisfiable:
          return "Requested Range Not Satisfiable";
        case ExpectationFailed:
          return "Expectation Failed";
        case Teapot:
          return "I'm a teapot";
        case MisdirectedRequest:
          return "Misdirected Request";
        case UnprocessableEntity:
          return "Unprocessable Entity";
        case Locked:
          return "Locked";
        case FailedDependency:
          return "Failed Dependency";
        case TooEarly:
          return "Too Early";
        case UpgradeRequired:
          return "Upgrade Required";
        case PreconditionRequired:
          return "Precondition Required";
        case TooManyRequests:
          return "Too Many Requests";
        case RequestHeaderFieldsTooLarge:
          return "Request Header Fields Too Large";
        case UnavailableForLegalReasons:
          return "Unavailable For Legal Reasons";
        case InternalServerError:
          return "Internal Server Error";
        case NotImplemented:
          return "Not Implemented";
        case BadGateway:
          return "Bad Gateway";
        case ServiceUnavailable:
          return "Service Unavailable";
        case GatewayTimeout:
          return "Gateway Timeout";
        case HTTPVersionNotSupported:
          return "HTTP Version Not Supported";
        case VariantAlsoNegotiates:
          return "Variant Also Negotiates";
        case InsufficientStorage:
          return "Insufficient Storage";
        case LoopDetected:
          return "Loop Detected";
        case NotExtended:
          return "Not Extended";
        case NetworkAuthenticationRequired:
          return "Network Authentication Required";
      }
      return "Unknown";
    }
};

#endif // __STATUS_