# Scalable-Server-for-Multiple-Client-Management
Overview:
  1. TCP Socket Based Client and Server Application.
  2. Multiple Clients were created on a single server.
  3. Server recieves the client request and queues them in a queue.
  4. Server has worker threads pool. 
  5. Worker thread get items from the queue, work on the items and store the results back in the item.
  6. Clients can poll and view the current state of the requests.
  
Project in Detail
CLIENT
  1. Clients are command line applications.
  2. Commands that are sent to server:
    1. pingSites www.google.com,www.cnn.com,www.espn.com
        1. The List of websites are variable and has maximum count of 10. 
        2. Server returns a handle to these requests and is printed on the command line.
    2. showHandles
        1. This command returns the handles of different requests made by all the clients of the server.
    3. showHandleStatus Handle
        1. This command sends a specific handle to the server
        2. the server returns the status of the request and client prints the status.

SERVER
  1. Server recieves a connect requests for each client and immediately spawns a Linux pthread to handle requests from client
  2. All commands other than "pingSites" are handled directly by per client thread.
  3. When pingSites command is recieved, the client request is queued to a queue for each site.
      1. Server also returns a unique handle for this request.
      2. Note: The queue is a queue of site names for worker threads to work on.
  4. There are 5 worker threads which are dedicated to retrieving site name from the queue.
      1. Worker Thread retrives the site name from the queue and open the TCP socket to the site.
      2. Worker thread measures the time it takes between open and successful connection.
      3. Worker thread then closes the TCP socket to the site.
      4. This is done for 10 times and calculates the average, minimum and maximum response times to the site obtained.
      5. Worker thread goes to access the next item from the queue.
      6. If nothing is there in the queue worker thread just blocked.
      
COMPILATION
  >gcc server.c -o server -lpthread
  >gcc client.c -o client

EXECUTION
  1. For execution of the codes one must open different linux command terminals. 
  2. One for server and another for different clients. The server code must be executed first. 
  3. The command can be run as follow: >./server
  4. As the server code starts running, the client code must be executed in another terminal as follows: >./client 127.0.0.1
  5. where 127.0.0.1 is the local IP-address.
  6. Give the commands as exactly shown above.
