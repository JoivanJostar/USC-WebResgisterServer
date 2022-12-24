## FILES

client.c: the c source code file of client.

serverM.c: the c source code file of Main Server

serverC.c: the c source code file of serverC

serverCS.c: the c source code file of serverCS

serverEE.c: the c source code file of serverEE

cred.txt: the credit file of encrypted username and password. This will be read by serverC

cred_unencrypted.txt:  the unencrypted username and password .

cs.txt: the course information of CS department. This will be read by serverCS

ee.txt: the course information of EE department. This will be read by serverEE



## DATA PACKAGE 

All of the data transform between nodes are in this common format:

| 4bytes Head n| string0| string 1| string2 ......| string n|

Data head is 4 bytes, it means the number of strings in payload body.

(1)When the client send username and password to server,

the data is : |2|username|password|

and then serverM just forward it to serverC.

the response of serverC are in 3 cases: 

1. |1| PASS| : login successfully
2. |1| NO_USER| : no this username
3. |1| NO_MATCH|: password does not match

and this will be forward to client by serverM

(2)When client send the query for 1 course, the data is like:

​	|3| SINGLE|course_code| category|

where SINGLE means this query is for 1 course whose code is course_code, and query the category.

then serverM will forward it in this format:

​	|2|course_code|category|  (just remove the SINGLE lable)

the result from serverXX is in this format:

​	|1| the xx of xx is xx|           or            |1| Didn't find xxx|

this message is easy be forward to client, and print them on screen.

(3)When client send the query for many courses at a time, the data is like:

​	|N| MULTY|course_code_1|course_code_2|........|course_code_N|

and serverM will analyse the context of data sent by client, send request to correct backend serverCS/EE  in the order of course_code, and forward the response from serverCS/EE.



## Cases where the program may fail

1. input too long string ( the length is more than 100) will cause the overflow of buffer.
2. the backend server  be killed while client is interact with it. 