#pragma once

#include <serverpp/core.hpp>
#include <serverpp/tcp_socket.hpp>
#include <telnetpp/telnetpp.hpp>
#include <serverpp/tcp_server.hpp>
#include <functional>
#include <memory>

/**
 * @brief A class used as a wrapper for the telnetpp C++ library. Contains necessary functions to 
 * integrate with the OpenOCD.py file.
 */
class TelnetlibCpp{
    public:
        /**
         * @brief Construct a new Telnetlib Cpp object. Establish a connection to the specified host
         * and port.
         * @param host host name. 
         * @param port port number, usually 4440.
         */
        TelnetlibCpp(const std::string& host,serverpp::port_identifier port);

        /**
         * @brief Read data from the telnetSession and return as a vector. To comply with the OpenOCD.py wrapper.
         * @return std::vector<std::string> 
         */
        std::vector<std::string> Readout();

        /**
         * @brief Run the command provided with the arguments. This is to comply with the OpenOCD.py wrapper.
         * @param cmd 
         * @param args 
         * @return std::vector<std::string> Readout when the required commands are run
         */
        std::vector<std::string> Exec(const std::string& cmd, const std::vector<std::string>& args);

        /**
         * @brief Read at least one byte of cooked data unless EOF is hit, blocking if no data is available.
         * @return std::vector<serverpp::byte> 
         */
        std::vector<serverpp::byte> read_some();

        /**
         * @brief Write a byte string to the socket, doubling any IAC characters. This can block if the connection is blocked.
         * @param data data to write. 
         */
        void write(const std::vector<serverpp::byte>& data);

        /**
         * @brief Writes data directly to the tcp socket.
         * @param data 
         */
        void write_raw_sequence(const std::vector<serverpp::byte>& data);

        /**
         * @brief This function is used if a server is created and needs to be run. (Not currently used).
         */
        void run();
    private:
        /**
         * @brief Private function used in the constructor to construct the tcp_socket variable. It establihes connection
         * and returns the socket.
         * @param host 
         * @param port 
         * @return boost::asio::ip::tcp::socket 
         */
        boost::asio::ip::tcp::socket connectAndMoveSocket(const std::string& host, serverpp::port_identifier port);

        /// @brief Used to control async operations.
        boost::asio::io_context io_context_;
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_;

        /// @brief Internetl socket to establish a connection to a port and to be used with the telnet::session class. 
        serverpp::tcp_socket socket_;

        /// @brief Used for writing and reading telnet protocols. 
        telnetpp::session telnet_session_;

        /// @brief Flag used for syncrhonisation attempts.
        bool incoming_data_{};

        /// @brief Variable to store received data from async read function. 
        std::vector<serverpp::byte> receivedData_;
};
