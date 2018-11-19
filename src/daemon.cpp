#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <execinfo.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <boost/process/child.hpp>

#include "src/logger.hpp"
#include "exception.hpp"

namespace server
{
	class daemon
	{
	public:
		daemon( )
		{
			logger_initialize( "daemon" );

			int status;
			int pid;

			pid = fork( );

			if( pid == -1 )
			{
				BOOST_LOG_TRIVIAL( error ) << "Start Daemon Error: " << strerror( errno );
				throw server_exception( "Fail to run daemon" );
			} else if (!pid) // если это потомок
			{
				umask( 0 );
				setsid();

				close( STDIN_FILENO );
				close( STDOUT_FILENO );
				close( STDERR_FILENO );

				boost::process::child child( "./balda" );

				if( child.running( ) )
					BOOST_LOG_TRIVIAL( info ) << "Process balda started!" << strerror( errno );
				else
					BOOST_LOG_TRIVIAL( info) << "Fail to start process balda!" << strerror( errno );

				child.detach( );

				BOOST_LOG_TRIVIAL( info ) << "Process balda detached!";

			}
		}
	};
}

int main( int argc, char** argv )
{
	server::daemon dm;
	return 0;
}