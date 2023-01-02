#include "NetWorkUpdata.h"

NetWorkUpdata::NetWorkUpdata(std::atomic<bool>& is_ok, json& json)
	: is_ok(is_ok), m_json(json)
{
}

NetWorkUpdata::~NetWorkUpdata()
{
}

void NetWorkUpdata::Run()
{
	// ºóÌ¨
	httplib::Client cli("http://songshu007.gitee.io");
	httplib::Result res = cli.Get("/backstage/2048/");

	if (res)
	{
		int begin = res->body.find("[####");
		int end = res->body.find("####]");

		if (begin >= 0 && end >= 0)
		{
			std::string data = res->body.substr(begin + 5, end - begin - 5);
			std::cout << data << std::endl;

			m_json = json::parse(data);

			is_ok = true;
		}
	}
	else
	{
		is_ok = false;
	}

}
