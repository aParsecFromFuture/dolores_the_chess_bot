# dolores_the_chess_bot

It is a improved version of my latest chess engine, you can find it among my repositories

WHAT'S NEW?  
1.Thread count is increased 1 to 16 (Multithread approach)  
2.Now everything is a lookup table!  
3.All the data structures converted to 1D array, even the chess board.  
4.Much lesser and readable codes! Line count is decreased from 750 to 400  
5.Lesser function call, more macro. It will save the cost of function calls  
6.Now the engine is about 20x faster  

Uses minimax and alpha-beta pruning algorithms  
It calculates next movement in 5 second when tree depth is lower than 6 at beginning and middle of the game  
It calculates next movement in 5 second when tree depth is lower than 7 at endings of the game  


