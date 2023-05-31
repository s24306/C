# Basic jpeg to ASCII converter

Run:
```
make
```
to create the executable.

To run the program:
1. If you're using Linux
```
./ascii -i [name of your jpeg file] -o [desired name of the text file] (optional)-t [how many times to scale down the image]
```
2. If you're using Windows
```
./writeASCII.exe -i [name of your jpeg file] -o [desired name of the text file] (optional)-t [how many times to scale down the image]
```

Example input:

![MTG icon](https://github.com/s24306/C/blob/master/Img-to-ASCII-art/icon.jpg?raw=true)

Example output:

![ASCII MTG icon](https://github.com/s24306/C/blob/master/Img-to-ASCII-art/ASCIIicon.png?raw=true)

Update 31.05.23:
- Added the ability to scale down the image by t times. The -t option is optional, and the default value is 1.
