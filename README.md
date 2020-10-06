# ESPRandomMatrix
Random animations, images and more on a RGB led matrix
======

This project is a mashup of three projects / examples for PxMatrix library. This display changes every 20 seconds between the following three display modes:

1. Tetris clock  
   As seen in https://github.com/witnessmenow/WiFi-Tetris-Clock/
2. Animated images  
   As seen in the https://github.com/2dom/PxMatrix/tree/master/examples/black_lives example. Displays a new image animation from the list of hardcoded images.
3. Aurora animation  
   As seen in https://github.com/2dom/PxMatrix/tree/master/examples/Aurora_Demo example. Displays a new random animation from the list of animations.

## Setup
Copy the _credentials.sample.h to _credentials.h and enter your WiFi credentials.  
If you want to add you own image animations. Have a look at the GIF animation converter here: https://toblum.github.io/gif2carray to create you own AnimData.h file.

## Source projects
Code is derived from: 
- https://github.com/2dom/PxMatrix/tree/master/examples
- https://github.com/witnessmenow/WiFi-Tetris-Clock/
