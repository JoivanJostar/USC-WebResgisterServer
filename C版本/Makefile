CXX      = gcc
CXXFLAGS = -g3 -Wall -Wextra -Wpedantic -Wshadow 
LDFLAGS  = -g3 
all: serverM serverC serverEE serverCS client
serverM: serverM.o
	$(CXX) $(CXXFLAGS) -o serverM $^ 
serverM.o: serverM.c
	$(CXX) $(CXXFLAGS) -c serverM.c
serverC: serverC.o
	$(CXX) $(CXXFLAGS) -o serverC $^ 
serverC.o: serverC.c
	$(CXX) $(CXXFLAGS) -c serverC.c
serverCS: serverCS.o
	$(CXX) $(CXXFLAGS) -o serverCS $^ 
serverCS.o: serverCS.c
	$(CXX) $(CXXFLAGS) -c serverCS.c
serverEE: serverEE.o
	$(CXX) $(CXXFLAGS) -o serverEE $^ 
serverEE.o: serverEE.c 
	$(CXX) $(CXXFLAGS) -c serverEE.c
client: client.o
	$(CXX) $(CXXFLAGS) -o client $^ 		
client.o: client.c
	$(CXX) $(CXXFLAGS) -c client.c $^ 		
clean: 
	rm *.o
