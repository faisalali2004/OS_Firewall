CXX = g++
CXXFLAGS = -std=c++17 -Wall -Icore -Igui -Iprocess_monitor
LDFLAGS = -lsqlite3 -lnetfilter_queue -lQt5Widgets -lQt5Core -lQt5Gui -lpthread

# Find all source files
CORE_SRC = $(wildcard core/*.cpp)
GUI_SRC = $(wildcard gui/*.cpp)
PROC_SRC = $(wildcard process_monitor/*.cpp)
SRC = main.cpp $(CORE_SRC) $(GUI_SRC) $(PROC_SRC)

# Output
TARGET = firewall

all: $(TARGET)

$(TARGET): $(SRC)
    $(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
    rm -f $(TARGET) *.o core/*.o gui/*.o process_monitor/*.o

.PHONY: all clean