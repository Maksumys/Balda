//
// Created by Максим Кузин on 16/11/18.
//

#include "daemon.h"

#include <exception>
#include <iostream>

int main( int argc, const char *argv[] )
{
    if( argc < 2 )
        throw std::exception();

    std::cout << "Running daemon..." << std::endl;

    daemon dm;
    dm.run( argv[ 2 ] );
}
