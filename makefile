CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

server: main.cpp  ./timer/lst_timer.cpp ./http/http_conn.cpp ./log/log.cpp ./mysqlpool/sqlCon.cpp ./mysqlpool/sqlPool.cpp webserver.cpp config.cpp
	$(CXX) -o server  $^ $(CXXFLAGS) -L /usr/local/lib -Wl,-rpath=/usr/local/lib -lpthread -lmysqlclient -ljsoncpp -I ./ 

clean:
	rm  -r server
