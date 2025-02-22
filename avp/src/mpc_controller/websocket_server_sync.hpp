#pragma once
//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket server, synchronous
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

typedef struct _msg_t {
    bool ready;
    float steer;
    float accel;
} ws_msg_t;
typedef struct _ws_args_t {
    ws_msg_t *input;
    ws_msg_t *output; 
} ws_args_t;



static void
do_session(tcp::socket socket, ws_args_t* args)
{

    ws_msg_t *input = args->input;
    ws_msg_t *output = args->output;
    try
    {
        // Construct the stream by moving in the socket
        websocket::stream<tcp::socket> ws{std::move(socket)};
        // Set a decorator to change the Server of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res)
            {
                res.set(http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-server-sync");
            }));

        ws.binary(true);
        printf("waiting for accept\n");
        ws.accept();
    
        for(;;)
        {
            beast::flat_buffer ready_buffer; 
            beast::flat_buffer buffer; 
            ws.read(ready_buffer);
            printf("got ready\n");
            while(input->ready == false) {
                usleep(1);
            }

            void *ptr = (void *)malloc(sizeof(float)*2);
            memcpy(ptr, &input->steer, sizeof(float));
            memcpy(ptr+sizeof(float), &input->accel, sizeof(float));

            auto write_buffer = boost::asio::buffer(ptr, sizeof(float)*2);
            ws.write(write_buffer);
            input->ready = false;
            free(ptr);
            
            ws.read(buffer);
            auto b = beast::buffers_to_string(buffer.data());
            memcpy(&output->steer, b.c_str(), sizeof(float));
            memcpy(&output->accel, b.c_str()+sizeof(float), sizeof(float));
            output->ready = true;
        }
    }
    catch(beast::system_error const& se)
    {
        // This indicates that the session was closed
        if(se.code() != websocket::error::closed)
            std::cerr << "Error: " << se.code().message() << std::endl;
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

}

static void *start(void *_args) {
    printf("Running start\n");
    
    
    ws_args_t *args = (ws_args_t *)_args;
    try {
        auto const address = net::ip::make_address("0.0.0.0");
        auto const port = static_cast<unsigned short>(9001);

        printf("ioc\n");
        net::io_context ioc{1};

        printf("acceptor\n");
        // The acceptor receives incoming connections
        tcp::acceptor acceptor{ioc, {address, port}};
        for(;;)
        {

            printf("socket\n");
            // This will receive the new connection
            tcp::socket socket{ioc};

            // Block until we get a connection
            printf("Waiting for conncetion\n");
            acceptor.accept(socket);


            printf("Starting session\n");
            // Launch the session, transferring ownership of the socket
            std::thread(
                &do_session,
                std::move(socket), args).detach();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << "Error: " << e.what() << std::endl;
    }

}
