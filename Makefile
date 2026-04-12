CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3

SRCS = src/core/main.cpp \
       src/core/App.cpp \
       src/core/integrator.cpp \
       src/core/background.cpp \
       src/core/camera.cpp \
       src/core/film.cpp \
       src/core/image_io.cpp \
       src/core/parser.cpp \
       msg_system/error.cpp \
       lib_tinyxml2/tinyxml2.cpp

OBJS = $(SRCS:.cpp=.o)

TARGET = raytmath

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)