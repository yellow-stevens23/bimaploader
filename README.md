# bimaploader
A simple load and display programme for bitmap images.

I started this in order to replace a prewritten library on my OGHouse project. I also wanted to get my head around reading and writting files in C++ using fstream. Most people learn by reading and writting text files and I wanted a bit more of a challenge so here it is. 

There are a few online tutorials on how to load BMPs in C but no so many in C++ and I did not find one in how to display the result! I thought being able to see it might be a good way of testing it. Obviously being able to read and display BMPs is not useful in and of itself but you can use the loader in your own projects

I have manually rearanged the bits from BRG (BMP format) to the more normal RBG. You can do this automatically using OpenGL formating (GL_BGR instead of GL_RGB and GL_BGRA rather than GL_RGBA)  in the texture but I have done it this way as I want to extend this into a conversion programme (possibly writing jpgs) in due course. 

You compile this with gcc by g++ bitmaploader.cpp -o bitmaploader -lGL -lglfw -lGLEW 

run with  ./bitmaploader yourfile.bmp.

I am including 3 test bitmaps all different sizes one with an Alpha Channel. 

