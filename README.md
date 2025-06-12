# CoNet

To run the program

### Linux/macOS
Step 1: Create and enter build directory
mkdir -p build
cd build

Step 2: Generate Makefiles with CMake
```
cmake ..
```

Step 3: Compile the project
```
cmake --build .
```

Step 4: Run the executable
```
./CoNet
```

### Windows
Step 1: Create and enter build directory
mkdir -p build
cd build

Step 2: Generate Makefiles with CMake
```
cmake .. -G "MinGW Makefiles"
```

Step 3: Compile the project
```
mingw32-make
```

Step 4: Run the executable
```
./CoNet.exe
```
