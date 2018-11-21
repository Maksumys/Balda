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
#include <boost/asio.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>

#include "src/json.hpp"
#include "src/logger.hpp"
#include "src/balda.hpp"

#include <algorithm>
#include <fstream>
#include <vector>
#include <mutex>

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

class game
{
	class cell
	{
	public:
		std::string ch;
	};
public:
	game( const boost::uuids::uuid &__player1, const boost::uuids::uuid &__player2 ) {

        uuid = boost::uuids::random_generator()();

		player1 = __player1;
		player2 = __player2;

		field.reserve( 5 );
		field.resize( 5 );

		for( auto &line : field ) {
            line.reserve( 5 );
            line.resize( 5 );
		}

        generate_word();
	}

	///TODO:
	void generate_word( ) {
		field[ 2 ][ 0 ].ch = "Д";
		field[ 2 ][ 1 ].ch = "O";
		field[ 2 ][ 2 ].ch = "Ш";
		field[ 2 ][ 3 ].ch = "И";
		field[ 2 ][ 4 ].ch = "К";
	}

	std::string field_string() {

	    std::string str;

	    for( const auto &line : field )
	        for( const auto &elem : line )
	            if( elem.ch.empty() )
	                str += "0";
	            else
	                str += elem.ch;

        return str;
    }

	boost::uuids::uuid player1;
	boost::uuids::uuid player2;

	boost::uuids::uuid uuid;

	std::vector< std::vector< cell > > field;
};

class server
{
public:
	server()
	{
		logger_initialize( "balda" );

		http_server.config.port = 8080;

		BOOST_LOG_TRIVIAL( info ) << "Initialize server address: " << http_server.config.address
								  << " port: " << http_server.config.port;

		boost::asio::post( pool,
		[ & ]( ) {
			using std::chrono::duration_cast;
			using std::chrono::steady_clock;
			using std::chrono::seconds;
			using std::chrono_literals::operator""ms;

			while( !exit ) {
				{
					std::lock_guard< std::mutex > sessions_lock( sessions_mutex );
					for(auto session = sessions.begin(); session != sessions.end(); )
					{
						auto now = steady_clock::now();
						auto interval = duration_cast< seconds >( now - session->second );

						if( interval > SESSIONS_TIMEOUT_LIMIT ) {
							BOOST_LOG_TRIVIAL( info ) << "Session disconnect timeout: " <<
							boost::lexical_cast< boost::uuids::uuid >( session->first );
							session = sessions.erase(session);
						}
						else
							++session;
					}
				}
				std::this_thread::sleep_for( 10000ms );
			}
		} );

		boost::asio::post( pool,
		[ & ]( ) {
			using std::chrono::duration_cast;
			using std::chrono::steady_clock;
			using std::chrono::seconds;
			using std::chrono_literals::operator""ms;
			using boost::lexical_cast;
			using boost::uuids::uuid;

			while( !exit ) {
				{
					std::lock( games_mutex, sessions_mutex );
					std::lock_guard< std::mutex > games_lock( games_mutex, std::adopt_lock );
					std::lock_guard< std::mutex > sessions_lock( sessions_mutex, std::adopt_lock );
					for( auto game = games.begin(); game != games.end(); )
					{
						if( sessions.count( game->second.player1 ) == 0 ) {
							BOOST_LOG_TRIVIAL( info ) << "Game disconnect timeout from user: "
													  << lexical_cast< uuid >( game->second.player1 );
							game = games.erase( game );
						}
						else if( sessions.count( game->second.player2 ) == 0 ) {
							BOOST_LOG_TRIVIAL( info ) << "Game disconnect timeout from user: "
													  << lexical_cast< uuid >( game->second.player2 );
							game = games.erase( game );
						}
						else
							++game;
					}
				}
				std::this_thread::sleep_for( 10000ms );
			}
		} );

		http_server.default_resource["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
												  std::shared_ptr<HttpServer::Request> request) {
			try {

				auto json_req = json::parse( request->content.string() );
				json json_response;

				BOOST_LOG_TRIVIAL( debug ) << "Post request: " << json_req;

				auto cmd = get_cmd( json_req );

				if( cmd.id == 1 ) {
					json_response = event_login( cmd.state );
				}
				else if( cmd.id == 2 ) {

                    if( !json_req.count( "command" ) )
                        throw server_exception( "command not found!" );
                    if( !json_req[ "command" ].count( "id" ) )
                        throw server_exception( "id command not found!" );
                    if( !json_req[ "command" ].count( "state" ) )
                        throw server_exception( "state command not found!" );
                    if( !json_req[ "command" ].count( "uuid1" ) )
                        throw server_exception( "uuid1 command not found!" );
                    if( !json_req[ "command" ].count( "uuid2" ) )
                        throw server_exception( "uuid2 command not found!" );

					json_response = event_start_game( boost::lexical_cast< boost::uuids::uuid >( json_req[ "command" ][ "uuid1" ].get<std::string>() ),
                                                      boost::lexical_cast< boost::uuids::uuid >( json_req[ "command" ][ "uuid2" ].get<std::string>() ),
                                                      cmd.state );
				}
				else
					throw server_exception( "id error!" );

				response->write( json_response.dump() );

			} catch( const std::exception &ex ) {
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

		boost::asio::post(pool, [ & ]( ) {
			http_server.start();
		});
	}

	json event_login( std::uint64_t state )
	{
		if( state == 1 )
		{
			boost::uuids::uuid uuid = boost::uuids::random_generator()();

			BOOST_LOG_TRIVIAL( debug ) << "Generate session: " << uuid;

			sessions[ uuid ] = std::chrono::steady_clock::now();

			return { { "command",
						   {
							   { "id", 1 },
							   { "state", 2 },
							   { "uuid", boost::uuids::to_string( uuid ) }
						   } } };
		} else
			throw server_exception( "state error!" );
	}

	json event_start_game( const boost::uuids::uuid &uuid1, const boost::uuids::uuid &uuid2, std::uint64_t state )
	{
		if( state == 1 )
		{
			if( ( sessions.count( uuid1 ) < 1 ) && ( sessions.count( uuid2 ) < 1 ) )
                throw server_exception( "Sessions not found" );

			sessions[ uuid1 ] = std::chrono::steady_clock::now();
            sessions[ uuid2 ] = std::chrono::steady_clock::now();

            game new_game( uuid1, uuid2 );

			return { { "command",
							 {
									 { "id", 1 },
									 { "state", 2 },
									 { "game_uuid", boost::uuids::to_string( new_game.uuid ) },
                                     { "field", new_game.field_string() }
							 } } };
		} else
			throw server_exception( "state error!" );
	}

	~server()
	{
		exit = true;
		pool.join();
	}

protected:
	HttpServer http_server;

	std::map< boost::uuids::uuid, std::chrono::steady_clock::time_point > sessions;
	std::mutex sessions_mutex;

	std::map< boost::uuids::uuid, game > games;
	std::mutex games_mutex;



	boost::asio::thread_pool pool{ 4 };

	bool exit{ false };
};


#endif //BALDA_SERVER_HPP
