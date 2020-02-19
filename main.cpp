#include <iostream>
#include "./parser.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;


#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include <sstream>

int main() {
    ps_parser parser;
    if(!parser)
        return 1;
    /*
    do {
        info i = parser.line();
        std::cout << i.command << " " << i.pid << " " << i.cpu << " " << i.mem << std::endl;
    } while(parser);
    */
    
    json j;
    j["pi"] = 3.141;
    cout << j << endl;
    
    	try 
	{
		curlpp::Cleanup cleaner;
        ostringstream s;
		s << curlpp::options::Url("http://my-json-server.typicode.com/skatsev/testproject/posts/1");
        j = json::parse(s.str());
        cout << j << endl;

	}

	catch ( curlpp::LogicError & e ) 
	{
		std::cout << e.what() << std::endl;
	}

	catch ( curlpp::RuntimeError & e )
	{
		std::cout << e.what() << std::endl;
	}
}
