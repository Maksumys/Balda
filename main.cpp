#include <utility>

#include <simple-web-server/server_https.hpp>
#include <simple-web-server/crypto.hpp>

#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "src/json.hpp"
#include "src/logger.hpp"

#include <algorithm>
#include <fstream>
#include <vector>

using namespace std;

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using json = nlohmann::json;

class Command
{
public:
	std::uint64_t id{ 0 };
	std::uint64_t state{ 0 };
};

class server_exception : public std::exception
{
	std::string err_str;
public:
	explicit server_exception( const std::string &str ) : err_str{ str } { }

	const char* what() const noexcept override
	{
		return err_str.c_str();
	}
};

Command get_cmd( nlohmann::json &json_req )
{
	Command cmd;

	if( !json_req.count( "command" ) )
		throw server_exception( "command not found!" );
	if( !json_req[ "command" ].count( "id" ) )
		throw server_exception( "id command not found!" );
	if( !json_req[ "command" ].count( "state" ) )
		throw server_exception( "state command not found!" );

	cmd.id = json_req[ "command" ][ "id" ].get<std::uint64_t >();
	cmd.state = json_req[ "command" ][ "state" ].get<std::uint64_t >();

	return cmd;
}

std::map< boost::uuids::uuid, std::chrono::steady_clock::time_point > sessions;

int main()
{
    logger_initialize();

	HttpServer server;
	server.config.port = 8080;

    BOOST_LOG_TRIVIAL( info ) << "Initialize server address: " << server.config.address
                              << " port: " << server.config.port;

	bool exit{ false };


	std::thread sessions_check( [ & ](){
		while( !exit )
		{
		    for( auto &session : sessions )
            {

            }
		    std::this_thread::sleep_for( 1s );
		}
	} );

	server.default_resource["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		try
		{
			auto json_req = json::parse( request->content.string() );

            BOOST_LOG_TRIVIAL( debug ) << "Post request: " << json_req;

            auto cmd = get_cmd( json_req );

			if( cmd.id == 1 )
			{
				if( cmd.state == 1 )
				{
					boost::uuids::uuid uuid = boost::uuids::random_generator()();

                    BOOST_LOG_TRIVIAL( debug ) << "Generate session: " << uuid;

					sessions[ uuid ] = std::chrono::steady_clock::now();

					json json_response = { { "command",
                                            {
                                                { "id", 1 },
                                                { "state", 2 },
                                                { "uuid", boost::uuids::to_string( uuid ) }
                                            } }
									     };

					response->write( json_response.dump() );
				} else
					throw server_exception( "state error!" );
			} else
				throw server_exception( "id error!" );
		} catch( const std::exception &ex )
		{
            BOOST_LOG_TRIVIAL( error ) << "Post request error: " << ex.what( );
			response->write( SimpleWeb::StatusCode::client_error_bad_request, ex.what( ) );
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
	exit = true;
	sessions_check.join();
}