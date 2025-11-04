# Chess Base

A simple recreation of the classic Pong game made in Unity.


## FEN String Support
- When I began my approach I started with making it iterate through each character of the string and that each slash would mean that it is a new row.
- For the numbers in the string representing empty space, I simply used its ascii value to turn it from a character to an int and simply advance the column amount by that much.
- I then simply would check if the character was uppercase by using the built in isUpper() and would set a boolean based on that to see if the piece was black or white.
- Then finally I would set the piece type based off the character, and advance to the next column.
