//
// Created by dev on 16.11.18.
//

#ifndef BALDA_SERVER_HPP
#define BALDA_SERVER_HPP

#include "src/exception.hpp"

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

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using json = nlohmann::json;

class Command
{
public:
	std::uint64_t id{ 0 };
	std::uint64_t state{ 0 };
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

class server
{
public:
	server()
	{
		using std::chrono_literals::operator""s;

		logger_initialize( "balda" );

		http_server.config.port = 8080;

		BOOST_LOG_TRIVIAL( info ) << "Initialize server address: " << http_server.config.address
								  << " port: " << http_server.config.port;

		sessions_check = std::move( std::thread( [ & ](){
			while( !exit )
			{
				for( auto &session : sessions )
				{

				}
				std::this_thread::sleep_for( 1s );
			}
		} ) );

		http_server.default_resource["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
												  std::shared_ptr<HttpServer::Request> request) {
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

		http_server.default_resource["GET"] = [](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			try {
				auto web_root_path = boost::filesystem::canonical("../web");
				auto path = boost::filesystem::canonical(web_root_path / request->path);

				if(std::distance(web_root_path.begin(), web_root_path.end()) > std::distance(path.begin(), path.end()) ||
				   !std::equal(web_root_path.begin(), web_root_path.end(), path.begin()))
					throw std::invalid_argument("path must be within root path");
				if(boost::filesystem::is_directory(path))
					path /= "index.html";

				SimpleWeb::CaseInsensitiveMultimap header;


				auto ifs = std::make_shared<std::ifstream>();
				ifs->open(path.string(), std::ifstream::in | std::ios::binary | std::ios::ate);

				if(*ifs) {
					auto length = ifs->tellg();
					ifs->seekg(0, std::ios::beg);

					header.emplace("Content-Length", to_string(length));
					response->write(header);

					class FileServer {
					public:
						static void read_and_send(const std::shared_ptr<HttpServer::Response> &response, const std::shared_ptr<std::ifstream> &ifs) {
							static std::vector<char> buffer(131072);
							std::streamsize read_length;
							if((read_length = ifs->read(&buffer[0], static_cast<std::streamsize>(buffer.size())).gcount()) > 0) {
								response->write(&buffer[0], read_length);
								if(read_length == static_cast<std::streamsize>(buffer.size())) {
									response->send([response, ifs](const SimpleWeb::error_code &ec) {
										if(!ec)
											read_and_send(response, ifs);
										else
											std::cerr << "Connection interrupted" << std::endl;
									});
								}
							}
						}
					};
					FileServer::read_and_send(response, ifs);
				}
				else
					throw std::invalid_argument("could not read file");
			}
			catch(const std::exception &e) {
				response->write(SimpleWeb::StatusCode::client_error_bad_request, "Could not open path " + request->path + ": " + e.what());
			}
		};

		http_server.on_error = [](std::shared_ptr<HttpServer::Request> /*request*/, const SimpleWeb::error_code & /*ec*/) {
		};

		server_thread = std::move( std::thread( [&]()
												{
													http_server.start();
												} ) );
	}

	~server()
	{
		server_thread.join();
		exit = true;
		sessions_check.join();
	}

protected:
	HttpServer http_server;

	std::map< boost::uuids::uuid, std::chrono::steady_clock::time_point > sessions;

	std::thread sessions_check;
	std::thread server_thread;

	bool exit{ false };
};


#endif //BALDA_SERVER_HPP