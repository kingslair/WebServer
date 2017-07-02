************************************README*************************************
  MAKE TOOL VERSION           : 3.81
  GNU C++ COMPILER VERSION    : 4.4.7 

  SPECIFICATIONS
  **************
  -> This is a HTTP 1.1 Supporting Server.
  -> Supports GET, HEAD, POST, PUT, HEAD, OPTIONS methods.
  -> Uses Python scripts for testing are avaliable at : codes/testing directory 
     to test the HTTP Server.
  -> Complete Details on Code and Readme are avaliable at the home page of the
     web server i.e www/index2.html, which can be accessed by running the server.

  DIRECTORY STRUCTURE
  *******************
  -> CODES -> bin     (contains both the server and client executable)
           -> gcov    (contains all the gcov coverage files)
           -> headers (contains all the classes)
           -> make    (contains the makefile)
           -> sources (source codes of the program)
           -> objects (object files of source codes)
           -> testing (contains all the python scripts for test automation)

  -> LOG -> log       (contains the log file)
  
  -> CONFIG -> config (contains the configuration file)

  -> TRASH -> directory to hold the deleted data
  
  -> WWW -> directory which hold all the pages and data that are served
            and uploaded by the client.

  COMPILING THE PROGRAM
  **********************
  -> Go to : codes/make
  -> Execute the makefile as :
      -> make
  -> creates all the object files of the source codes and also the executable
     of the server.

  RUNNING THE PROGRAM
  *******************
  -> Go to : codes/bin directory
    -> Run Server executable as :
        -> ./http_server portnumber
        -> Example =  ./http_server 4000
        -> server takes port number as argument

  -> The HTTP Server starts serving the clients.
  -> Common Client include : Standard Web Browser
                           : REST API Clients    
                             Example: 1. DHC Client avaliable on chrome store.
                             Example: 2. Postman also avalable on chrome store.

  GCOV REPORT
  ***********
  -> Go to : codes/make
  -> Execute as :
      -> make gcov_report
  -> generates the gcov report for all the used source code files.

  VALGRIND REPORT
  ***************
  -> Go to : codes/make
  -> Execute as :
      -> make valgrind
  -> Run commands on the Server and use termination signal for the Server to
     generate the valgrind report.

  TESTING
  *******
  -> Start the server in one Command Window with port number 4000.
  -> Open another Command Window and Go to : codes/testing
  -> Execute as :
      -> python python_client.py
      -> This execution takes the port number as 4000 and host as localhost
  -> The output will be displayed on the command line window.

  EXITING THE SERVER
  ******************
  -> For exiting from the Server use the following commands from a new Command
     Line Window.
      -> ps aux | grep username
         -> Type in the System username on which it is logged on and the server
            is running.
      -> kill -s USR1 PID
         -> The first command returns the processid(PID) of the running server.
         -> Replace it in the PID part of the above command.

  BACKGROUND
  **********
  -> Server uses 'select' command to connect multiple clients by 
     IO Multiplexing.
  -> HTTP 1.1 Server
  -> HTTP Server which servers the following method:
      -> GET
      -> PUT
      -> POST
      -> DELETE
      -> HEAD
      -> OPTIONS
  -> Testing on this server is done using Python Scripts.
  -> Python Modules are divided into seperate parts as per the request type.
*******************************************************************************
