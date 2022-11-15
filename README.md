# **DiscussionForum**


A C++ MultiThreaded TCP Server and Test-Harness to test the servers
through-put.

With the built executables of both these programs being so small in
size, the .exe's have been provided.

```
Assignment Title: Discussion Forum 

Module: Concurrent and Parallel Systems - Level 6

Module Code: 55-600738

Author: Alexander Ashley (B7034209@exchange.shu.ac.uk)
```

## **Table of Contents**
- [**DiscussionForum**](#discussionforum)
  - [**Table of Contents**](#table-of-contents)
  - [**Building both the TCP Server & Test-Harness**](#building-both-the-tcp-server--test-harness)
  - [**Running the TCP Server**](#running-the-tcp-server)
  - [**Running the Test-Harness**](#running-the-test-harness)
___


## **Building both the TCP Server & Test-Harness**
  - Open the DiscussionForm.sln solution in Visual Studio 2019.
  - Change your solution configuration from **Debug** to **Release**
    along the top of the VS toolbar.
  - Right click the solution in Solution Explorer and select build
    solution.

This will build both the TCP Server & Test-Harness and produce
exectuables under `./x64/Release/`


## **Running the TCP Server**
From the root folder (containing the **DiscussionForm.sln**)
navigate to `./x64/Release/`

This can then be ran by either
double clicking **TCPServer.exe** or by opening this directory
in a terminal and running **"./TCPServer.exe"**.

## **Running the Test-Harness**
From the root folder (containing the **DiscussionForm.sln**)
navigate to `./x64/Release/`

The Test-Harness **requires 5 arguments** to run, so this must be 
ran from a terminal.

`TestHarnessClient.exe server_ip number_of_poster_threads number_of_reader_threads time_duration throttle(0|1)`

| Argument                  | Description   |
| --------------------------|-------------  |
| server_ip                 | IP of server (localhost or 127.0.0.1 if running locally).
| number_of_poster_threads  | The number of threads performing POST operations      
| number_of_reader_threads  | The number of threads performing READ operations    
| time_duration             | How long you want the Test-Harness to run for **(seconds)**
| throttle                  |  **0**: No Throttle <br/> **1**: throttle to 1,000 requests per second

An example run for testing the through-put on 5 poster threads & 5 reader threads, for 30 seconds with no throttling, might look something like this:

`TestHarnessClient.exe localhost 5 5 30 0`