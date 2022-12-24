CXX      = g++
CXXFLAGS = -g3 -Wall -Wextra -Wpedantic -Wshadow 
LDFLAGS  = -g3 
all: serverM serverC serverCS serverEE client
serverM: common.o serverM.o
	$(CXX) $(CXXFLAGS) -o serverM $^ 
serverM.o: serverM.cpp common.h
	$(CXX) $(CXXFLAGS) -c serverM.cpp
common.o: common.cpp common.h
	$(CXX) $(CXXFLAGS) -c common.cpp
serverC: serverC.o common.o
	$(CXX) $(CXXFLAGS) -o serverC $^ 
serverC.o: serverC.cpp common.h
	$(CXX) $(CXXFLAGS) -c serverC.cpp	

serverCS: serverCS.o common.o
	$(CXX) $(CXXFLAGS) -o serverCS $^ 
serverCS.o: serverCS.cpp common.h
	$(CXX) $(CXXFLAGS) -c serverCS.cpp
serverEE: serverEE.o common.o
	$(CXX) $(CXXFLAGS) -o serverEE $^ 
serverEE.o: serverEE.cpp common.h
	$(CXX) $(CXXFLAGS) -c serverEE.cpp	
	
client: client.o common.o
	$(CXX) $(CXXFLAGS) -o client $^ 		
	
clean: 
	rm *.o
