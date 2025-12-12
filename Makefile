# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# Include directories
INCLUDES = -Iraylib/include -Iraylib-cpp/include -Itinyfiledialogs -Itools

# Library directories
LDFLAGS = -Lraylib/lib

# Libraries to link
LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm -lole32 -luuid -lcomdlg32

# Source files
SRC = \
    main.cpp \
	tinyfiledialogs/tinyfiledialogs.c \
    tools/PencilTool.cpp \
	tools/EraserTool.cpp

# Output executable
OUT = ratart.exe

# Default rule
all: $(OUT)

# Linking
$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) $(INCLUDES) $(LDFLAGS) $(LDLIBS) -o $(OUT)

# Clean build files
clean:
	del /Q $(OUT)

