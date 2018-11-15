#include <simple-web-server/server_https.hpp>
#include <simple-web-server/crypto.hpp>

#include <boost/filesystem.hpp>

#include "json.hpp"

#include <algorithm>
#include <fstream>
#include <vector>

using namespace std;

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using json = nlohmann::json;

int main() {

	HttpServer server;
	server.config.port = 8080;


	server.default_resource["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		try
		{
            auto j3 = json::parse( request->content.string() );
            response->write( j3.dump() );
            std::cout << j3 << std::endl;
		} catch( ... )
		{

		}
	};

	server.default_resource["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		try {
			auto web_root_path = boost::filesystem::canonical("../web");
			auto path = boost::filesystem::canonical(web_root_path / request->path);

			if(distance(web_root_path.begin(), web_root_path.end()) > distance(path.begin(), path.end()) ||
			   !equal(web_root_path.begin(), web_root_path.end(), path.begin()))
				throw invalid_argument("path must be within root path");
			if(boost::filesystem::is_directory(path))
				path /= "index.html";

			SimpleWeb::CaseInsensitiveMultimap header;


			auto ifs = make_shared<ifstream>();
			ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);

			if(*ifs) {
				auto length = ifs->tellg();
				ifs->seekg(0, ios::beg);

				header.emplace("Content-Length", to_string(length));
				response->write(header);

				class FileServer {
				public:
					static void read_and_send(const shared_ptr<HttpServer::Response> &response, const shared_ptr<ifstream> &ifs) {
						static vector<char> buffer(131072);
						streamsize read_length;
						if((read_length = ifs->read(&buffer[0], static_cast<streamsize>(buffer.size())).gcount()) > 0) {
							response->write(&buffer[0], read_length);
							if(read_length == static_cast<streamsize>(buffer.size())) {
								response->send([response, ifs](const SimpleWeb::error_code &ec) {
									if(!ec)
										read_and_send(response, ifs);
									else
										cerr << "Connection interrupted" << endl;
								});
							}
						}
					}
				};
				FileServer::read_and_send(response, ifs);
			}
			else
				throw invalid_argument("could not read file");
		}
		catch(const exception &e) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request, "Could not open path " + request->path + ": " + e.what());
		}
	};

	server.on_error = [](shared_ptr<HttpServer::Request> /*request*/, const SimpleWeb::error_code & /*ec*/) {
	};

	thread server_thread([&server]() {
		server.start();
	});

	server_thread.join();
}