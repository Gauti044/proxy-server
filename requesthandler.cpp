#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include <opencv2/opencv.hpp>

using namespace std;
using boost::asio::ip::tcp;

class Proxy {
public:
    static bool isBlocked(const string& url) {
        // Implement logic to check if a site is blocked
        return false; // placeholder
    }

    static void addCachedPage(const string& url, const string& filePath) {
        // Implement caching logic
    }

    static string getCachedPage(const string& url) {
        // Implement logic to fetch cached page if exists
        return ""; // placeholder
    }
};

class RequestHandler {
private:
    tcp::socket clientSocket;
    boost::asio::streambuf requestBuffer;
    boost::asio::streambuf responseBuffer;
    string cachedFilePath;
    
public:
    RequestHandler(boost::asio::io_service& io_service, tcp::socket&& socket) 
        : clientSocket(std::move(socket)) {}

    void handleRequest() {
        // Read the first line of the request
        boost::asio::read_until(clientSocket, requestBuffer, "\r\n");
        istream requestStream(&requestBuffer);
        string requestLine;
        getline(requestStream, requestLine);

        // Parse the request
        stringstream ss(requestLine);
        string method, url;
        ss >> method >> url;

        cout << "Request received: " << requestLine << endl;

        // Handle URL prepending with http:// if missing
        if (url.substr(0, 4) != "http") {
            url = "http://" + url;
        }

        // Check if the site is blocked
        if (Proxy::isBlocked(url)) {
            cout << "Blocked site requested: " << url << endl;
            blockedSiteRequested();
            return;
        }

        // Handle HTTP vs HTTPS requests
        if (method == "CONNECT") {
            cout << "HTTPS request for: " << url << endl;
            handleHTTPSRequest(url);
        } else {
            // Check if cached page exists
            cachedFilePath = Proxy::getCachedPage(url);
            if (!cachedFilePath.empty()) {
                cout << "Cached copy found for: " << url << endl;
                sendCachedPageToClient();
            } else {
                cout << "HTTP GET for: " << url << endl;
                sendNonCachedToClient(url);
            }
        }
    }

    void sendCachedPageToClient() {
        try {
            ifstream cachedFile(cachedFilePath, ios::binary);
            if (!cachedFile.is_open()) {
                cout << "Error opening cached file: " << cachedFilePath << endl;
                return;
            }

            string contentType = cachedFilePath.substr(cachedFilePath.find_last_of('.') + 1);
            string response = "HTTP/1.0 200 OK\r\nProxy-Agent: ProxyServer/1.0\r\n\r\n";
            boost::asio::write(clientSocket, boost::asio::buffer(response));

            if (contentType == "png" || contentType == "jpg" || contentType == "jpeg" || contentType == "gif") {
                // Sending image file
                vector<char> buffer(4096);
                while (!cachedFile.eof()) {
                    cachedFile.read(buffer.data(), buffer.size());
                    boost::asio::write(clientSocket, boost::asio::buffer(buffer.data(), cachedFile.gcount()));
                }
            } else {
                // Sending text content
                string line;
                while (getline(cachedFile, line)) {
                    boost::asio::write(clientSocket, boost::asio::buffer(line + "\r\n"));
                }
            }
        } catch (exception& e) {
            cerr << "Error sending cached page to client: " << e.what() << endl;
        }
    }

    void sendNonCachedToClient(const string& url) {
        try {
            string fileExtension = url.substr(url.find_last_of(".") + 1);
            string fileName = url.substr(url.find("://") + 3);
            replace(fileName.begin(), fileName.end(), '/', '_');
            fileName += "." + fileExtension;

            // Cache the file
            ofstream cachedFile("cached/" + fileName, ios::binary);
            if (!cachedFile.is_open()) {
                cout << "Error creating cache file" << endl;
                return;
            }

            boost::asio::io_service ioService;
            tcp::resolver resolver(ioService);
            tcp::resolver::query query(url, "80");
            tcp::resolver::iterator endpointIterator = resolver.resolve(query);

            tcp::socket serverSocket(ioService);
            boost::asio::connect(serverSocket, endpointIterator);

            string request = "GET " + url + " HTTP/1.1\r\nHost: " + url + "\r\n\r\n";
            boost::asio::write(serverSocket, boost::asio::buffer(request));

            boost::asio::streambuf responseBuffer;
            boost::asio::read_until(serverSocket, responseBuffer, "\r\n\r\n");

            // Send the response back to the client
            stringstream responseStream;
            responseStream << &responseBuffer;
            boost::asio::write(clientSocket, boost::asio::buffer(responseStream.str()));

            // Read and send the body of the response
            vector<char> buffer(4096);
            while (size_t bytesRead = serverSocket.read_some(boost::asio::buffer(buffer))) {
                boost::asio::write(clientSocket, boost::asio::buffer(buffer.data(), bytesRead));
                cachedFile.write(buffer.data(), bytesRead);
            }

            // Cache the page
            Proxy::addCachedPage(url, "cached/" + fileName);

        } catch (exception& e) {
            cerr << "Error handling non-cached request: " << e.what() << endl;
        }
    }

    void handleHTTPSRequest(const string& url) {
        try {
            string host = url.substr(7);
            stringstream ss(host);
            string hostname;
            int port = 443;

            // Extract host and port
            getline(ss, hostname, ':');
            if (ss >> port) {}

            boost::asio::io_service ioService;
            tcp::socket serverSocket(ioService);
            serverSocket.connect(tcp::endpoint(boost::asio::ip::address::from_string(hostname), port));

            string response = "HTTP/1.0 200 Connection Established\r\nProxy-Agent: ProxyServer/1.0\r\n\r\n";
            boost::asio::write(clientSocket, boost::asio::buffer(response));

            vector<char> buffer(4096);
            while (true) {
                size_t bytesRead = clientSocket.read_some(boost::asio::buffer(buffer));
                serverSocket.write_some(boost::asio::buffer(buffer, bytesRead));
                bytesRead = serverSocket.read_some(boost::asio::buffer(buffer));
                clientSocket.write_some(boost::asio::buffer(buffer, bytesRead));
            }
        } catch (exception& e) {
            cerr << "Error handling HTTPS request: " << e.what() << endl;
        }
    }

    void blockedSiteRequested() {
        try {
            string response = "HTTP/1.0 403 Forbidden\r\nProxy-Agent: ProxyServer/1.0\r\n\r\n";
            boost::asio::write(clientSocket, boost::asio::buffer(response));
        } catch (exception& e) {
            cerr << "Error sending blocked site response: " << e.what() << endl;
        }
    }
};

int main() {
    try {
        boost::asio::io_service io_service;
        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 8080));
        cout << "Proxy server running on port 8080..." << endl;

        while (true) {
            tcp::socket clientSocket(io_service);
            acceptor.accept(clientSocket);
            RequestHandler handler(io_service, std::move(clientSocket));
            handler.handleRequest();
        }
    } catch (exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}
