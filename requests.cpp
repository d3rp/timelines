#include "requests.h"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

std::string
request()
{
    /*
    auto sslOpts = cpr::Ssl(
        cpr::ssl::ALPN(false),
        cpr::ssl::NPN(false),
        cpr::ssl::MaxTLSVersion(),
        cpr::ssl::VerifyHost(false),
        cpr::ssl::VerifyPeer(false),
        cpr::ssl::VerifyStatus(false)
    );
	*/
    //auto sslOpts = cpr::Ssl(cpr::ssl::ALPN{ false }, cpr::ssl::NPN{ false });
    cpr::Response r = cpr::Get(cpr::Url{ "https://www.httpbin.org/get" },
        /*        cpr::Parameters{
                    {"action", "query"},
                    {"list", "search"},
                    {"srsearch", "platon"},
                    {"format", "json"}
               },
               */
        cpr::VerifySsl(false));
    //cpr::Response r = cpr::Get(cpr::Url{ "https://t.co" });
    //,
    //    cpr::Authentication{ "user", "pass" },
    //    cpr::Parameters{ {"anon", "true"}, {"key", "value"} });

//    r.status_code;                  // 200
//    r.header["content-type"];       // application/json; charset=utf-8
//    r.text;                         // JSON text string
    auto j = nlohmann::json::parse(r.text);
    //return j.dump();
    return j.at("Host");
}

TEST_CASE("testing curl functionality")
{
    auto s = request();
    CHECK(s == "foo");
}
