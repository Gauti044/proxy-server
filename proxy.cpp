#include <iostream>
#include <fstream>
#include <map>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <sstream>
#include <chrono>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

std::map<std::string, std::string> cache_file;
std::map<std::string, std::string> block_sites;
std::vector<std::thread> servicing_threads;
std::mutex mtx;  // Mutex for thread safety
bool runn = true;

// Function to load cached and blocked sites from file
void loadFiles() {
    std::ifstream cacheFile("cached_sites.txt");
    std::ifstream blockFile("block_sites.txt");

    if (cacheFile.is_open()) {
        std::string line;
        while (std::getline(cacheFile, line)) {
            cache_file[line] = line;  // Assuming cached content is just URL for simplicity
        }
        cacheFile.close();
    } else {
        std::cout << "Cached sites file not found, creating new one." << std::endl;
    }

    if (blockFile.is_open()) {
        std::string line;
        while (std::getline(blockFile, line)) {
            block_sites[line] = line;
        }
        blockFile.close();
    } else {
        std::cout << "Blocked sites file not found, creating new one." << std::endl;
    }
}

// Function to save cached and blocked sites to file
void saveFiles() {
    std::ofstream cacheFile("cached_sites.txt");
    std::ofstream blockFile("block_sites.txt");

    if (cacheFile.is_open()) {
        for (const auto& entry : cache_file) {
            cacheFile << entry.first << std::endl;
        }
        cacheFile.close();
    }

    if (blockFile.is_open()) {
        for (const auto& entry : block_sites) {
            blockFile << entry.first << std::endl;
        }
        blockFile.close();
    }
}

// Function to handle incoming requests from clients (simulated)
void handleRequest(int clientSocket) {
    std::this_thread::sleep_for(std::chrono::seconds(1));  // Simulate handling request
    std::string message = "Request Handled by Server\n";
    send(clientSocket, message.c_str(), message.length(), 0);
    close(clientSocket);
}

// Function to listen for incoming connections
void listenForConnections(int port) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Binding failed!" << std::endl;
        close(serverSocket);
        return;
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Listening failed!" << std::endl;
        close(serverSocket);
        return;
    }

    std::cout << "Waiting for client on port " << port << "..." << std::endl;

    while (runn) {
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == -1) {
            std::cerr << "Failed to accept client connection." << std::endl;
            continue;
        }

        std::lock_guard<std::mutex> lock(mtx);
        servicing_threads.push_back(std::thread(handleRequest, clientSocket));
    }

    close(serverSocket);
}

// Function to handle dynamic management of proxy via console
void manageProxy() {
    while (runn) {
        std::cout << "\n\t\tPROXY SERVER MENU:\n\nEnter a new site to block, or type:\n\"blocked\" to see blocked sites,\n\"cached\" to see cached sites,\n\"close\" to close server.\n";
        std::string cmd;
        std::getline(std::cin, cmd);

        if (cmd == "blocked") {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "\nCurrently Blocked Sites: \n";
            for (const auto& entry : block_sites) {
                std::cout << entry.first << std::endl;
            }
        } else if (cmd == "cached") {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "\nCurrently Cached Sites: \n";
            for (const auto& entry : cache_file) {
                std::cout << entry.first << std::endl;
            }
        } else if (cmd == "close") {
            runn = false;
            break;
        } else {
            std::lock_guard<std::mutex> lock(mtx);
            block_sites[cmd] = cmd;
            std::cout << "\n" << cmd << " blocked successfully!!\n";
        }
    }
}

int main() {
    // Load cached and blocked sites from files
    loadFiles();

    // Start listening for incoming connections on a separate thread
    std::thread listeningThread(listenForConnections, 6969);

    // Start the console management menu in the main thread
    manageProxy();

    // Wait for all threads to finish
    listeningThread.join();

    // Save cached and blocked sites before closing the server
    saveFiles();
    std::cout << "Server closed." << std::endl;

    return 0;
}
