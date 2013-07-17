/*
Program Title: Game.h
Author: Darren Glyn Roberts
Date: 07/03/2013
Version: 1.0
*/

#ifndef GAME_H
#define GAME_H
#define levelov 150 // Final level, will change as I add more levels!

class game
{
    private: // Private variables
        short env[20][16]; // Array holding environment variables
        bool finish[20][16]; // Array holding finish positions
        short playerxy[2]; // Array holding player position
        GLuint level; // Level number

         
    public: // Public variables
        game(); // game constructor
        bool loadlevel(); // Load a level
        void nextlvl(); // Load next level
        void prevlvl();  // Load previous level
        void draw(GLuint* texture, GLfloat move_x, GLfloat move_y, GLfloat move_z); // Draw the maze
        void drawplayer(GLdouble, GLdouble, GLuint); // Draw player
        void drawSkyBox(GLuint texture1, GLuint texture2, GLuint texture3, GLuint texture4, GLuint texture5, GLuint texture6); // Draw skybox
            
        void setxy(int x, int y) // Set player coordinates
        {
             playerxy[0] += x; 
             playerxy[1] += y;
        }
        
        short block(int x, int y) // Check block
        {
              return env[playerxy[0] + x] [playerxy[1] + y];
        }
        
        void setblock(int x, int y, short ID) // Set block type and location
        {
             env[playerxy[0] + x][playerxy[1] + y] = ID;
        }
        
        bool finished(); // Level completed?
        
        int lvl() // Return level number
        {
            return level;
        }
};
#endif
