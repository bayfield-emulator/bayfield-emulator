## How to run the graphics demo (on Linux):
1. Ensure you have libSDL2.0 installed. 
2.     make
3.     ./demo <path to rom>

## How to run the graphics demo (on something else):
1. Don't.
2. If you have to, you're on your own. I have never had much luck with SDL on other platforms.

### Why did the demo drop bitmaps next to it?
Those are dumps of the buffers. You can compare the Window buffer with the Backgroud buffer to see if the Window is being drawn properly.
