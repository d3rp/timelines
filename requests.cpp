#include "requests.h"
#include <cpr/cpr.h>

std::string
request()
{
	cpr::Response r = cpr::Get(cpr::Url{ "https://en.wikipedia.org/w/api.php"},
        cpr::Parameters{
            {"action", "query"},
			{"list", "search"},
			{"srsearch", "platon"},
			{"format", "json"}
        });
    //cpr::Response r = cpr::Get(cpr::Url{ "https://t.co" });
    //,
    //    cpr::Authentication{ "user", "pass" },
    //    cpr::Parameters{ {"anon", "true"}, {"key", "value"} });

//    r.status_code;                  // 200
//    r.header["content-type"];       // application/json; charset=utf-8
//    r.text;                         // JSON text string
    return r.text;
}

TEST_CASE("testing curl functionality")
{
    auto s = request();
    CHECK(s == "foo");
}
