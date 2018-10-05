User Manual for Operating Thomas Vy’s CPSC 441 Assignment 1 (Bad Spelling Proxy) 
How to Compile and Run 
To compile Assignment, I would suggest that you use command line and navigate to the directory where the “Server.cpp” file is located and enter the following command to prompt the C++ compiler: 
g++ -Wall -o server Server.cpp 
If everything goes well then, a server.exe should be created and you’ll be able to start the server by invoking the following command: 
./server.exe 
Note: Make sure that the port 12345 is accessible through the firewall. University of Calgary should be fine since I demoed it there and it worked fine as of October 3rd, 2018. 
Now, with the server running, you should connect your browser to the proxy by going to the proxy options of your browser and adding the IP address of the machine the proxy is running on (Can be found by using ipconfig or just use localhost or 127.0.0.1) and the port 12345. 
Features 
Features for this proxy includes a service that retrieves your desired content with your normal HTTP requests but changes the webpage data by changing characters in the page while retaining the format of the page (gifs, html format, and other formats stay the same); the proxy will only change the data if the HTTP response code form the server is 200, or else the proxy doesn’t manipulate the data. The proxy can only handle webpages that are less than 10kB in size (The size can be changed in the source code if the user wants – change the BUFFER_SIZE macro to the number of your choosing).  
Testing 
Testing was done on October 3, 2018 in T05 with Reza at 12:45pm on my personal computer(Windows 10) using Cygwin as the terminal. 
GET HTTP request works well but other HTTP request like POST were not tested. I did not implement any bonus features in the assignment.  