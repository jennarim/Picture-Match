# Picture Match

The implemented OpenGL application involves the following:
1. Texture Mapping
2. Texture Transformation (Rotation)
3. View Control

# Demo
<img src="https://user-images.githubusercontent.com/20725513/73604361-93abb400-455d-11ea-94fa-720686595aef.gif">

# Introduction
"Picture Match" is a game similar to pin-the-tail-on-the-donkey in that it involves placing face parts such as eyes, nose, mouth to a blank face while guessing their correct location. 
The player is allowed to rotate face pieces (images) and place them at the location they guess fits best.
1. The picture of the desired image is initially shown to the player, alongside other instructions.
<img width="200" alt="1" src="https://user-images.githubusercontent.com/20725513/72637158-a5bb0f00-392e-11ea-9317-ca8660fe1dc4.png">
2. The face parts are placed and rotated randomly outside the face the user should drag the parts to.
<img width="200" alt="2" src="https://user-images.githubusercontent.com/20725513/72637258-e1ee6f80-392e-11ea-9d71-b3f4cc571f1c.png">
3. The user can rotate any of the face parts as they select it.
<img width="200" alt="3" src="https://user-images.githubusercontent.com/20725513/72637270-ec106e00-392e-11ea-84ce-c80e8c49986a.png">
4. Each face part will be bound to follow the user’s cursor.  The moment the user clicks inside the face, then the user has finalized the location.
<img width="200" alt="4" src="https://user-images.githubusercontent.com/20725513/72637287-f6cb0300-392e-11ea-8791-98ddc7b8900f.png">
5. Once all face parts have been finalized, the final result is shown.
<img width="200" alt="5" src="https://user-images.githubusercontent.com/20725513/72637307-fe8aa780-392e-11ea-8cd8-5b98e1d38ab3.png">
6. The user can press R to restart, and continue to play and admire the result.
<img width="200" alt="6" src="https://user-images.githubusercontent.com/20725513/72637323-08140f80-392f-11ea-96af-079b1895853b.png">

# Implementation
## Transformation
**Translation** Each time the user clicks on a face part, it follows the cursor.

**Rotation** The user can rotate each face part using the J/K keys; this rotation is done around the texture’s center.

## View Control
Resizing the application window maintains the vector graphics as is. 
