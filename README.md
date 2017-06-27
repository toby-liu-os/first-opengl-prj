## First OpenGL Project

[first-opengl-prj](https://github.com/tobyliu-sw/first-opengl-prj) is used as an example of simple OpenGL project, which uses vertex and fragment shader to draw triangles.

### System environment

1. Ubuntu 16.04 as VMWare guest OS
2. built-in gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.4)
3. cmake version 3.5.1 
```
sudo apt-get install cmake
```

### Required library:

1. [GLFW](http://www.glfw.org/): version 3.1
```
sudo apt-get install libglfw3-dev
```
2. [glm](https://github.com/g-truc/glm/): latest version on GitHub
```
- cd ~
- git clone https://github.com/g-truc/glm.git
- cd /usr/local/include/
- sudo ln -s ~/glm/glm
```

### Build:
```
./build.sh
```

### Run:
```
./build/first-opengl
```
