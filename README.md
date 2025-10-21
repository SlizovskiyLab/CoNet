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



### Manually Run
g++ -std=c++17 -Wall -Iinclude -Ithird_party src/*.cpp -o CoNet.exe

./CoNet.exe


cd viz
python3 -m http.server 8080

Check 
http://localhost:8080/index.html


# AMR-GraphNet: Network-based Visualization of ARG–MGE Colocalization Dynamics

AMR-GraphNet is a framework for modeling and visualizing **antimicrobial resistance (AMR) gene – mobile genetic element (MGE) colocalization** across **fecal microbiota transplantation (FMT)** samples.  
It enables researchers to track **temporal changes** in ARG–MGE interactions across donor, pre-FMT, and post-FMT microbiomes using an **interactive graph-based view**.

---

## Project Overview

The pipeline takes annotated **metagenomic assembly files** as input, identifies **colocalized ARG–MGE pairs**, constructs a **biological network**, and exports both static and interactive visualizations.



## Features



---

## Installation

### Prerequisites

