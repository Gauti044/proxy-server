To run your C++ implementation of an HTTP Proxy Server, follow these detailed steps:

1. Set Up Your Development Environment
Ensure that you have a C++ compiler installed. If you are using a Linux system, g++ (GNU C++ Compiler) is commonly used. On Windows, you can use MinGW or Visual Studio. For macOS, Xcode command line tools will work.
For Linux:

Install g++ via package manager if not installed:
bash
Copy code
sudo apt update
sudo apt install g++
For Windows:

Install MinGW (Minimalist GNU for Windows) or use Visual Studio's built-in C++ compiler.
For macOS:

Install Xcode command-line tools if you don't already have them:
bash
Copy code
xcode-select --install
2. Prepare the Code Files
Ensure that your C++ code (e.g., proxy_server.cpp) is saved in a directory.
Your proxy server code might require some additional files like cached_sites.txt and block_sites.txt to store the cached files and blocked sites, respectively.
Ensure the following files exist:

proxy_server.cpp (your main C++ code).
cached_sites.txt (a file to store cached sites).
block_sites.txt (a file to store sites to be blocked).
A directory to hold the cached files (e.g., cached/).
3. Compile the Code
Open a terminal or command prompt and navigate to the directory containing your proxy_server.cpp.
On Linux/macOS:

bash
Copy code
cd /path/to/your/code
g++ -o proxy_server proxy_server.cpp
On Windows (using MinGW):

bash
Copy code
cd \path\to\your\code
g++ -o proxy_server proxy_server.cpp
On Windows (using Visual Studio Command Prompt):

bash
Copy code
cd \path\to\your\code
cl proxy_server.cpp
This command will compile your code and generate an executable named proxy_server.

4. Configure Your Browser to Use the Proxy
After the proxy server is compiled successfully, configure your web browser (e.g., Firefox) to use the proxy server you just created.
For Firefox:

Open Firefox and go to Preferences/Settings.
Scroll down and click on Settings under Network Settings.
Select Manual proxy configuration.
In the HTTP Proxy field, enter localhost (or the IP address of the machine running the proxy if it's a different machine).
Set the Port to 6969 (or the port number your proxy server is listening on).
Ensure Use this proxy server for all protocols is checked.
5. Run the Proxy Server
Go to the terminal/command prompt again and execute the compiled proxy_server program.
On Linux/macOS:

bash
Copy code
./proxy_server
On Windows:

bash
Copy code
proxy_server.exe
The server will start listening on localhost:6969 for incoming HTTP/HTTPS requests. You should see the proxy server running in the terminal/command prompt.

6. Test the Proxy Server
Open your browser and attempt to browse a website. If everything is set up correctly, the request will be routed through your proxy server.
The proxy server should handle the requests:
If the requested URL is cached, it will serve the cached file.
If the URL is not cached, it will fetch the content from the origin server and store it in the cache.
Try accessing a blocked site (as defined in block_sites.txt). The proxy server should block access to the site.
7. Monitor Logs/Console Output
The proxy server will print log information to the terminal or command prompt, showing what requests are being made and how it is handling them.
You can monitor the output for any errors or confirmation messages, such as "Connection Established" for HTTPS requests.
8. Handle Errors/Debugging
If there are any errors during compilation, check the error messages. They might indicate issues such as:
Missing libraries (e.g., <iostream>, <string>, <unordered_map>).
Syntax errors in the C++ code.
You can also check the runtime output to identify issues such as:
Incorrect file paths for cached files or blocked sites.
Incorrect proxy server port configuration.
9. Optional: Run the Proxy Server on a Different Machine
If you want to test the proxy server running on a different machine, use the IP address of the machine running the proxy instead of localhost in the browser’s proxy settings.
Make sure that your firewall allows inbound connections on the proxy server port (6969 by default).
10. Shut Down the Proxy Server
When you are done, you can stop the proxy server by closing the terminal or command prompt, or by pressing Ctrl+C in the terminal.
