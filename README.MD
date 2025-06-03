# 简介

美国南加州大学（USC) 计算机系的网络编程大作业，一个简易的Web registration system，在Linux系统上用Socket编程实现基本的登录认证，课程信息查询等操作。

主要编写了4个服务器程序和1个客户端程序。

服务器架构：1个主服务器M+1个认证服务器C+2个课程信息查询服务器（CS计算机系和EE电子系）

客户端：一个简单的负责交互的控制台客户端

思路：TCP短链接的设计思路，客户端每次交互只向服务器发送一个请求，包含需要那种服务，登录 or 查询

主服务器M根据请求头的类别信息，转发客户端的请求到正确的后端服务器。后端服务器完成具体的业务查询，然后把结果返回给主服务器，再转发给客户端。这里主服务器相当于一个反向代理服务器。

项目里面有两个文件夹，因为当时接单的时候，碰巧一次接了两份，为了对客户负责，所以分别用C语言和C++都实现了一遍。

# 作业要求

 **Socket Programming Project**

**Problem Statement:** 

Web registration system has been a critical system for USC students. It is the key place for students to plan their future course of study and plays an important role in students’ academic success. Imagine that one day the web registration system is gone and tens of thousands of students are left unknown about what to choose for the next semester. A course might be over-crowded with hundreds of students because students don’t know how many students have already registered and the administrator might have to randomly select hundreds of people to drop from that course. Or you can imagine on the first day of the semester, the web registration system is suddenly down, all students are anxiously refreshing their webpage. And another thing to consider is the security, keep in mind that our web registration system should have a kind of authorization, for example username and password. Otherwise, it will be a popular hoax among students to drop courses for others. Thus a secure, reliable, functional and informative web registration system is vital for our school. As a networking course, we will try to approach this problem with some simplification. We will make the assumption that only one student will access the web registration system each time, and there are only two departments of courses to choose from. We will also introduce a very simple authorization schema. 

In this project, you will implement a simple web registration system for USC. Specifically, a student will use the client to access the central web registration server, which will forward their requests to the department servers in each department. For each department, the department server will store the information of the courses offered in this department. Additionally, a credential server will be used to verify the identity of the student. 

There are total 5 communication end-points: 

- ●  Client:      used by a student to access the registration system. 
- ●  Main      server (serverM): coordinate with the backend servers. 
- ●  Credential      server (serverC): verify the identity of the student. 
- ●  Department      server(s) (serverCS and serverEE)): store the information of courses      offered by this department. 

详细内容见word文档。Socket Programming Project.docx
