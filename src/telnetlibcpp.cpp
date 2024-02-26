#include "telnetlibcpp.hpp"
#include <boost/locale.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>


TelnetlibCpp::TelnetlibCpp(const std::string& host, serverpp::port_identifier port)
    :work_(boost::asio::make_work_guard(io_context_)),
     socket_{std::move(connectAndMoveSocket(host, port))},
     telnet_session_{socket_} {

}

boost::asio::ip::tcp::socket TelnetlibCpp::connectAndMoveSocket(const std::string& host, serverpp::port_identifier port) {
    boost::asio::ip::tcp::socket socket(io_context_);
    boost::asio::ip::tcp::resolver resolver(io_context_);
    boost::system::error_code ec; // Variable to store error code

    // Use resolver to get endpoint iterator
    auto endpoints = resolver.resolve(host, std::to_string(port), ec);
    if (ec) {
        std::cerr << "Failed to resolve host '" << host << "' on port " << port << ": " << ec.message() << std::endl;
        throw std::runtime_error("Failed to connect to '" + host + "' on port " + std::to_string(port) + ": " + ec.message());
    }

    // Attempt to connect to the resolved endpoint
    boost::asio::connect(socket, endpoints, ec);
    if (ec) {
        throw std::runtime_error("Failed to connect to '" + host + "' on port " + std::to_string(port) + ": " + ec.message());
    }

    std::cout << "Successfully connected to '" << host << "' on port " << port << std::endl;
    return socket; // Socket is moved out of the function
}

std::vector<std::string> TelnetlibCpp::Readout() {
    std::string s;
    std::vector<std::string> lines;

    while (true) {
        std::vector<std::uint8_t> byte_data = read_some();
        std::string chunk(byte_data.begin(), byte_data.end());

        s += chunk;

        size_t start_pos = 0;
        size_t pos;
        while ((pos = s.find('\n', start_pos)) != std::string::npos) {
            if (pos > start_pos) { // Ignore empty lines
                std::string line(s.begin() + start_pos, s.begin() + pos);
                line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
                line.erase(std::remove(line.begin(), line.end(), '\x00'), line.end());
                
                if (line.size())
                    lines.push_back(std::move(line)); // Use move semantics to avoid copying
            }
            start_pos = pos + 1;
        }
        s.erase(s.begin(), s.begin() + start_pos); // Only erase the processed part

        if (s.find('>') != std::string::npos) {
            return lines; // Assuming lines are already filtered correctly
        }
    }
}

/* Readout function alternate implementation. This one is tested more but seems to be a bit slower. 
std::vector<std::string> TelnetlibCpp::Readout() {
    std::string s;
    std::vector<std::string> lines;
    while (true) {
        std::vector<std::uint8_t> byte_data = read_some();
        const char* begin = reinterpret_cast<const char*>(byte_data.data());
        const char* end = begin + byte_data.size();
        std::string chunk = boost::locale::conv::to_utf<char>(begin, end, "UTF-8");

        s += chunk;

        size_t pos = 0;
        std::string delimiter = "\n";
        while ((pos = s.find(delimiter)) != std::string::npos) {
            std::string line = s.substr(0, pos);
            // Remove carriage return characters from the line
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
            if (!line.empty()) {
                lines.push_back(line);
            }
            s.erase(0, pos + delimiter.length());
        }

        // Adjust the condition to trim and check the residual string
        if (s.find('>') != std::string::npos) {
            // Final condition met, filter out any null characters and return
            std::vector<std::string> filtered_lines;
            for (auto& line : lines) {
                std::string filtered_line;
                std::copy_if(line.begin(), line.end(), std::back_inserter(filtered_line), [](char c) { return c != '\x00'; });
                filtered_lines.push_back(filtered_line);
            }
            return filtered_lines;
        }
    }
}
*/


std::vector<std::string> TelnetlibCpp::Exec(const std::string& cmd, const std::vector<std::string>& args) {
    std::string text = cmd;
    for (const auto& arg : args) {
        if (!arg.empty()) {
            text += ' ' + arg;
        }
    }
    text += '\n';

    // write(Cmd), write directly to avoid unnecessary conversions 
    telnetpp::bytes byte_data(reinterpret_cast<const std::uint8_t*>(text.data()), text.size());
    telnetpp::element elem = byte_data;
    telnet_session_.write(elem); 
    //-----------------------------------------------------------------------------------------

    // Read and return the output
    return Readout();
}

std::vector<serverpp::byte> TelnetlibCpp::read_some() {
    incoming_data_ = false; // Ensure the flag is reset before starting the async operation
    receivedData_.clear(); // Clear previous data
    // Return a copy of the data or move receivedData as appropriate
    telnet_session_.async_read([this](gsl::span<const serverpp::byte> data) {
        if (!data.empty()) {
            // Directly assign the data since receivedData is a member variable
            receivedData_.assign(data.begin(), data.end());
        }
        incoming_data_ = true; // Indicate that data has been received or the operation is complete
    });
    // Efficiently wait for the async operation to complete using io_context.run_one()
    while (!incoming_data_) {
        io_context_.run_one();
    }
    // Return a copy of the data or move receivedData as appropriate
    return receivedData_;
}

void TelnetlibCpp::write(const std::vector<serverpp::byte>& data) {
    // Assuming 'serverpp::byte' is compatible with 'telnetpp::byte'
    // Wrap the data in a 'telnetpp::bytes' variant to create a 'telnetpp::element'
    telnetpp::bytes byte_data(data.data(), data.size());
    telnetpp::element elem = byte_data;  // Implicitly converts to variant

    // Now call the write method with the constructed element
    telnet_session_.write(elem);
}

void TelnetlibCpp::write_raw_sequence(const std::vector<serverpp::byte>& data){
    if (socket_.is_alive()){
      telnetpp::bytes byte_data(data.data(), data.size());
      socket_.write(byte_data);// Send unescaped data directly to the socket.
    }
}


void TelnetlibCpp::run(){
    io_context_.run();
}
