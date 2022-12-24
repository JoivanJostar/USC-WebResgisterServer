# FILES

serverM.cpp: the cpp source code file of Main Server

serverC.cpp: the cpp source code file of Credential server

serverCS.cpp: the cpp source code file of backend  serverCS

serverEE.cpp: the cpp source code file of backend  serverEE

client.cpp: the source code file of client.



cred.txt: the credit file contains some encrypted username and password

cred_unencrypted.txt:  the plain text for cred.txt

cs.txt: the course information of CS

ee.txt: the course information of EE



common.h:  it defines some common functions that some of server codes or client code may use. like pack the data, unpack data, split a string.  This file will be included by all other cpp files.

common.cpp: the implementation of the functions defined in common.h.



Makefile

Readme.md



# COMPILE & RUN

make all

./serverM

./serverC

./serverCS

./serverEE

./client

# DATA FORMAT

The request data format from the client to the Main server is as follows:

[ Request Head :8 bytes ] [ payload data ]

The Request Head is 8 bytes, could be "LOGIN" or "QUERY". "LOGIN" means requesting to authorization.

"QUERY" means querying the  course information.

The payload data is multiple strings separated by '\0'.

e.g. If a client input the username "james" and password "2kAnsa7s)" . The request data is:

['L','O','G','I','N','\0','\0','\0','j','a','m','e','s','\0','2','k','A','n','s','a','7','s',')','\0']

if a client want to query the Professor of EE450, the request data is:

['Q','U','E','R','Y','\0','\0','\0','E','E','4','5','0','\0','P','r','o','f','e','s','s','o','r','\0']



the data format  that serverM to serverC or serverEE/CS is similar to the above.

The data format of all responses is very simple, that information which needs to be displayed by the client is directly used as the response data. This is friendly for the client.



# Project Feature

This project realizes the function of querying multiple course information at one time.

e.g: 

Please enter the course code to query: EE450 CS100 CS310 EE520
james sent a request with multiple CourseCode to the main server.
CourseCode: Credits, Professor, Days, Course Name
EE450: 4, Ali Zahid, Tue;Thu, Introduction to Computer Networks
CS100: 4, Sathyanaraya Raghavachary, Tue;Thu, Explorations in Computing
CS310: 4, Chao Wang, Mon;Wed, Software Engineering
EE520: 4, Todd Brun, Tue;Thu, Introduction to Quantum Information Processing





I use "short connection" method to  implement the interaction between the client and the server , that is, each time the client needs to request a service, it needs to establish a new connection with the server. During the life cycle of this connection, the client and the server will only interact once. The server distinguishes between authorization service and query service according to the service type in the data  head sent by the client. Therefore, a new port number will be dynamically allocated for each operation of the client.

Long connection (there are multiple interactions in the process of a connection) The two nodes that need to continuously maintain communication with each other are in a alive state, and the interaction process is relatively complex. Short connection is suitable for situations where the interaction process is simple, such as Http. Because the situation of this assignment is not complicated and the interaction process is relatively simple, I used the short connection design scheme.



## Cases where the program may fail

This program does not check the validity of the client's input perfectly.

The following conditions may cause the program to fail:

1. the username or password contains  character ','

2. The category is not entered as shown in the prompt. 

   eg: the Category is case sensitive. If you want to input "Credit" but input "credit" , the server could not query anything.

   e.g:  extra space for category "CourseName". If you input "Course Name", the server could not query anything.