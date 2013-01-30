#include <stdlib.h> // Include the stdlib header for the exit() function.
#include <stdio.h> //Include for file functions and debugging process
#include <math.h> //Include the math header for abs()
#include <string.h> //Include to use strcpy()
#include <allegro.h> // Include the allegro header file.

//Declare colours as constants
#define COLOUR_NO_CURSOR makecol(204, 204, 255)
#define COLOUR_CURSOR makecol(255, 153, 153)

//function prototypes
void instructions();
void playGame();
void highScore(int score, int MinScoreElement, int scores[], char names[][10]);
void GameOver(int score, bool won);
void DisplayHighscore();
void OverWriteHighScore(int MinScoreElement, int NewScore, char NewName[10], int scores[], char names[][10]);
void increment_speed_counter();
void pause();
void NextWave();
void Win(int score);

volatile long speed_counter = 0; //speed counter to maintain game speed

int main(int argc, char *argv[])
{
    allegro_init(); // Initialize Allegro
    set_window_title("Jason's Tower Defence"); //windows bar title                         
    install_keyboard(); // Initialize keyboard routines
    install_mouse(); // Initialize mouse routines
    install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, 0); //Initialize sound routines

    LOCK_VARIABLE(speed_counter); // which regulates the game's speed 
    LOCK_FUNCTION(increment_speed_counter);
    install_int_ex(increment_speed_counter, BPS_TO_TIMER(60)); //Set our BPS
  
    set_color_depth(desktop_color_depth()); // Set the color depth
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, 660,600,0,0); // Change our graphics mode to 640x480
    
    //show and focus the mouse cursor
    show_mouse(screen);
    set_mouse_sprite_focus(0,0); 
  
    //load ALL bitmaps necessary for displaying the main menu
    BITMAP * play;
    play = load_bitmap("play.bmp", NULL);
    BITMAP * instruction;
    instruction = load_bitmap("instructions.bmp", NULL);
    BITMAP * credit;
    credit = load_bitmap("credit.bmp", NULL);
    BITMAP * quit;
    quit = load_bitmap("quit.bmp", NULL);
    BITMAP * main;
    main = load_bitmap("Main Screen.bmp", NULL);
    BITMAP * buffer;
    buffer = create_bitmap(SCREEN_W, SCREEN_H);
    
    //music
    set_volume(255,255);
    MIDI * music = NULL; //declare midi    
    music = load_midi("pokemon.mid"); //load midi
    play_looped_midi(music,1,-1); //play
    
    //keep looping the main menu unless quit button is clicked
    while (1){
        
        //preparing the buffer
        acquire_screen();
        draw_sprite(buffer, main, 0, 0); //background
        
        //changes the colour of the button to pink when cursor is moved over it        
        if (mouse_x >= 190 && mouse_x <= 170+play->w && mouse_y >= 275 && mouse_y <= 255+play->h)
           rectfill(buffer, 190, 275, 170+play->w, 255+play->h, COLOUR_CURSOR);
        else
           rectfill(buffer, 190, 275, 170+play->w, 255+play->h, COLOUR_NO_CURSOR);
           
        if (mouse_x >= 190 && mouse_x <= 170+instruction->w && mouse_y >= 350 && mouse_y <= 330+instruction->h)
           rectfill(buffer, 190, 350, 170+instruction->w, 330+instruction->h, COLOUR_CURSOR);
        else
           rectfill(buffer, 190, 350, 170+instruction->w, 330+instruction->h, COLOUR_NO_CURSOR);
    
        if (mouse_x >= 190 && mouse_x <= 170+credit->w && mouse_y >= 425 && mouse_y <= 405+instruction->h)
           rectfill(buffer, 190, 425, 170+credit->w, 405+credit->h, COLOUR_CURSOR);
        else
           rectfill(buffer, 190, 425, 170+credit->w, 405+credit->h, COLOUR_NO_CURSOR);
        
        if (mouse_x >= 190 && mouse_x <= 170+quit->w && mouse_y >= 500 && mouse_y <= 480+instruction->h) 
           rectfill(buffer, 190, 500, 170+quit->w, 480 + quit->h, COLOUR_CURSOR);
        else
           rectfill(buffer, 190, 500, 170+quit->w, 480 + quit->h, COLOUR_NO_CURSOR);
           
        //display the option buttons for menu
        draw_sprite(buffer, play, 180, 265);   
        draw_sprite(buffer, instruction, 180, 340);
        draw_sprite(buffer, credit, 180, 415);
        draw_sprite(buffer, quit, 180, 490);
        
        //draw buffer to screen
        draw_sprite(screen, buffer, 0, 0);
        release_screen();
        
        //calls separate functions depending on where the button is clicked
        if (mouse_b & 1){
              if (mouse_x >= 190 && mouse_x <= 170+play->w && mouse_y >= 275 && mouse_y <= 255+play->h)
                    playGame();
              else if (mouse_x >= 190 && mouse_x <= 170+instruction->w && mouse_y >= 350 && mouse_y <= 330+instruction->h)
                    instructions();
              else if (mouse_x >= 190 && mouse_x <= 170+credit->w && mouse_y >= 425 && mouse_y <= 405+credit->h)
                    DisplayHighscore();
              else if (mouse_x >= 190 && mouse_x <= 170+quit->w && mouse_y >= 500 && mouse_y <= 480+quit->h) 
                    return 0;
        }
    }
    
    //destroy bitmaps
    destroy_bitmap(play);
    destroy_bitmap(instruction);
    destroy_bitmap(credit);
    destroy_bitmap(quit);
    destroy_bitmap(main);
    destroy_bitmap(buffer);
    
    //destroy midi
    destroy_midi(music);                    
   return 0;
}
END_OF_MAIN()

void playGame(){   
     //play start game music
     MIDI*StartGame;
     StartGame = load_midi("baby.mid");
     play_looped_midi(StartGame,1,-1);
     
     //create and load bitmaps needed for the game screen
     BITMAP*buffer;
     BITMAP*map;
     BITMAP*balloon;
     BITMAP*kangaroo;
     BITMAP*allowed;
     BITMAP*disallowed;
     BITMAP*pop;
     map = load_bitmap("map.bmp", NULL);
     balloon = load_bitmap("baloons.bmp", NULL);
     buffer = create_bitmap(SCREEN_W, SCREEN_H);
     kangaroo = load_bitmap("kangaroo1.bmp", NULL);
     allowed = load_bitmap("allowed.bmp", NULL);
     disallowed = load_bitmap("NotAllowed.bmp", NULL);
     pop = load_bitmap("pop.bmp", NULL);
     
     //Defines structure
     struct coordinates{
            int x;
            int y;
     };
     
     coordinates Balloon[100];
     coordinates Kangaroo[30];
     
     //declare and initialize variables
     int timer_counter[100] = {0}, i, j, lastBalloon = 0, total = 9, total_counter = 0;
     int TotalKangaroos = 0, NextKangaroo = 0, popped_timer[100] = {0}, delayTimer[30];
     int score = 0, money = 40, lives = 25, increment = 1, level = 1, spacing = 90, cost = 20;
     bool finished[100] = {0}, mouseClick = 0, popped[100] = {0};
     
     for (i = 0; i<=29; i++){ //initialize an array of variables
         Kangaroo[i].x = 550;
         Kangaroo[i].y = 10;
         delayTimer[i] = 3999;
     }
     
     for (i = 0; i <= 99; i++){
         Balloon[i].x = 160;
         Balloon[i].y = 0;
     }
     
     //loops game until the ESC key is pressed
     while (!key[KEY_ESC]){
                      
           //update the x and y position of all the balloons
           for (i = 0; i<= lastBalloon; i++){
               if (!finished[i]){
                   if (timer_counter[i] >= 0/ increment && timer_counter[i] <=20/increment)
                      Balloon[i].y+= increment;
                   else if (timer_counter[i] > 20/increment && timer_counter[i] <= 155/increment)
                      Balloon[i].x-= increment;
                   else if (timer_counter[i] > 155/increment && timer_counter[i] <= 400/increment)
                      Balloon[i].y+= increment;
                   else if (timer_counter[i] > 400/increment && timer_counter[i] <= 530/increment)
                      Balloon[i].x+= increment;
                   else if (timer_counter[i] > 530/increment && timer_counter[i] <= 635/increment)
                      Balloon[i].y-= increment;
                   else if (timer_counter[i] > 635/increment && timer_counter[i] <= 830/increment)
                      Balloon[i].x+= increment;
                   else if (timer_counter[i] > 830/increment && timer_counter[i] <= 1065/increment)
                      Balloon[i].y+= increment;
                   else if (timer_counter[i] > 1065/increment && timer_counter[i] <= 1390/increment)
                      Balloon[i].x-= increment;
                   else if (timer_counter[i] > 1390/increment && timer_counter[i] <= 1530/increment)
                      Balloon[i].y+= increment;
                   else if (timer_counter[i] > 1530/increment && timer_counter[i] <= 1990/increment)
                      Balloon[i].x+= increment;
                   else if (timer_counter[i] > 1990/increment && timer_counter[i] <= 2500/increment)
                      Balloon[i].y-= increment;
                   else if (timer_counter[i] > 2500/increment && timer_counter[i] <= 2700/increment)
                      Balloon[i].x-= increment;
                   else if (timer_counter[i] > 2700/increment && timer_counter[i] <= 2750/increment)
                      Balloon[i].y-= increment;
                   else{
                      finished[i] = 1; //balloon has reached the end
                      score--; //decrease the score by 1
                      lives--;
                   } 
               }
               
               timer_counter[i]++; //a timer to keep track of the movements of each balloon
               
               if (timer_counter[i] == spacing && i != total)
                  lastBalloon++; //send a new balloon from the starting position
           }            
           
           acquire_screen();

           draw_sprite(buffer, map, 0, 0); //create the map background
           draw_sprite(buffer, kangaroo, 550, 10); //draw the kangaroo weapon on the right
           
           //updates new kangaroo when mouse clicks and drags the icon
           if (mouse_b & 1 && mouse_x > 550 && mouse_x < 660 && mouse_y > 10 && mouse_y < 10+kangaroo->h && money >= cost){
                 mouseClick = 1;
                 Kangaroo[NextKangaroo].x = mouse_x - (kangaroo->w)/2;
                 Kangaroo[NextKangaroo].y = mouse_y - (kangaroo->h)/2;
           }
           
           if (mouseClick == 1){
                 Kangaroo[NextKangaroo].x = mouse_x - (kangaroo->w)/2;
                 Kangaroo[NextKangaroo].y = mouse_y - (kangaroo->h)/2;
                 
                 //checks the position of the kangaroo so that it can be ONLY
                 //placed on the grass area of the map
                 if ((mouse_x - kangaroo->w/2 > 200 && mouse_x + kangaroo->w/2 < 290 && mouse_y - kangaroo->h/2 > 0 && mouse_y + kangaroo->h/2 < 170) ||
                 (mouse_x - kangaroo->w/2 > 75 && mouse_x + kangaroo->w/2 < 480 && mouse_y - kangaroo->h/2 > 80 && mouse_y + kangaroo->h/2 < 170) ||
                 (mouse_x - kangaroo->w/2 > 75 && mouse_x + kangaroo->w/2 < 155 && mouse_y - kangaroo->h/2 > 80 && mouse_y + kangaroo->h/2 < 265) ||
                 (mouse_x - kangaroo->w/2 > 395 && mouse_x + kangaroo->w/2 < 480 && mouse_y - kangaroo->h/2 > 80 && mouse_y + kangaroo->h/2 < 535) ||
                 (mouse_x - kangaroo->w/2 > 75 && mouse_x + kangaroo->w/2 < 480 && mouse_y - kangaroo->h/2 > 450 && mouse_y + kangaroo->h/2 < 535) ||
                 (mouse_x - kangaroo->w/2 > 200 && mouse_x + kangaroo->w/2 < 350 && mouse_y - kangaroo->h/2 > 215 && mouse_y + kangaroo->h/2 < 400) ||
                 (mouse_x - kangaroo->w/2 > 0 && mouse_x + kangaroo->w/2 < 350 && mouse_y - kangaroo->h/2 > 315 && mouse_y + kangaroo->h/2 < 400)) 
                     draw_sprite(buffer, allowed, mouse_x - allowed->w/2, mouse_y - allowed->h/2); //blue circle for allowed position
                 else
                     draw_sprite(buffer, disallowed, mouse_x - disallowed->w/2, mouse_y - disallowed->h/2); //red circle for disallowed position
           }
                   
           //once the mouse is let go, the kangaroos position is permanent 
           if (!(mouse_b & 1) && mouseClick == 1 ){
                 mouseClick = 0;
                 
                 if ((mouse_x - kangaroo->w/2 > 200 && mouse_x + kangaroo->w/2 < 290 && mouse_y - kangaroo->h/2 > 0 && mouse_y + kangaroo->h/2 < 170) ||
                 (mouse_x - kangaroo->w/2 > 75 && mouse_x + kangaroo->w/2 < 480 && mouse_y - kangaroo->h/2 > 80 && mouse_y + kangaroo->h/2 < 170) ||
                 (mouse_x - kangaroo->w/2 > 75 && mouse_x + kangaroo->w/2 < 155 && mouse_y - kangaroo->h/2 > 80 && mouse_y + kangaroo->h/2 < 265) ||
                 (mouse_x - kangaroo->w/2 > 395 && mouse_x + kangaroo->w/2 < 480 && mouse_y - kangaroo->h/2 > 80 && mouse_y + kangaroo->h/2 < 535) ||
                 (mouse_x - kangaroo->w/2 > 75 && mouse_x + kangaroo->w/2 < 480 && mouse_y - kangaroo->h/2 > 450 && mouse_y + kangaroo->h/2 < 535) ||
                 (mouse_x - kangaroo->w/2 > 200 && mouse_x + kangaroo->w/2 < 350 && mouse_y - kangaroo->h/2 > 215 && mouse_y + kangaroo->h/2 < 400) ||
                 (mouse_x - kangaroo->w/2 > 0 && mouse_x + kangaroo->w/2 < 350 && mouse_y - kangaroo->h/2 > 315 && mouse_y + kangaroo->h/2 < 400)){         
                        NextKangaroo++;
                        TotalKangaroos++;
                        money-=cost;
                 }
                 
                 else {
                     Kangaroo[NextKangaroo].x = 550;
                     Kangaroo[NextKangaroo].y = 10;
                 }
                     
                     
           }
           
           //draw all the kangaroos that are already positioned      
           for (i = 0; i <= TotalKangaroos; i++)
               draw_sprite(buffer, kangaroo, Kangaroo[i].x, Kangaroo[i].y);        
            

           //if the balloon is within the kangaroos range, it will POP 
           for (j = 0; j <= NextKangaroo - 1; j++){
               for (i = 0; i<=lastBalloon; i++){
                   if (delayTimer[j] != 4000 && i<= 9)
                            delayTimer[j]= delayTimer[j] + 1;
                   if (Balloon[i].x + balloon->w > Kangaroo[j].x - 50 && Balloon[i].x < Kangaroo[j].x + 50 + kangaroo->w && Balloon[i].y + balloon->h > Kangaroo[j].y - 50 && Balloon[i].y < Kangaroo[j].y + 50 + kangaroo->h && !finished[i]){                   
                        if (delayTimer[j] == 4000){
                            popped[i] = 1;
                            score+= 2; //increase score
                            money++; //increase money
                            finished[i] = 1;
                            delayTimer[j] = 0;
                            break;
                            break;
                        }
                   }
               }
           }
           
           for (i = 0; i<= lastBalloon; i++){
               if (finished[i] == 1)
                  total_counter++; //counts number of balloons that are no longer in the maze
           } 
           
           for (i = 0; i<=lastBalloon; i++){
               if (finished[i] == 0)
                  draw_sprite(buffer, balloon, Balloon[i].x, Balloon[i].y); //display the balloon if it's not popped
               else if (popped[i] == 1 && popped_timer[i] < 30 && total_counter != total + 1){
                  draw_sprite(buffer, pop, Balloon[i].x, Balloon[i].y); //display the pop bitmap
                  popped_timer[i] ++;   
               }               
           }
           
           //display updated information
           textprintf_ex(buffer, font, 565, 390, makecol(0, 0, 0), -1, "LEVEL: %d", level);
           textprintf_ex(buffer, font, 565, 430, makecol(0, 0, 0), -1, "LIVES: %d", lives);
           textprintf_ex(buffer, font, 565, 470, makecol(0, 0, 0), -1, "SCORE: %d", score);
           textprintf_ex(buffer, font, 565, 550, makecol(0, 0, 0), -1, "MONEY: $%d", money);
           textprintf_ex(buffer, font, 565, 510, makecol(0, 0, 0), -1, "COST:  $%d", cost);
           textprintf_ex(buffer, font, 555, 320, makecol(0, 0, 255), -1, "Press 'P' to");
           textprintf_ex(buffer, font, 575, 340, makecol(0, 0, 255), -1, " PAUSE");
           textprintf_ex(buffer, font, 553, 90, makecol(0, 0, 0), -1, "Click n' Drag");
           textprintf_ex(buffer, font, 556, 130, makecol(255, 0, 0), -1, "Press ESC to");
           textprintf_ex(buffer, font, 580, 140, makecol(255, 0, 0), -1, " QUIT");
 
           draw_sprite(screen, buffer, 0, 0); //output buffer to screen   
           
           release_screen();
           
           if (lives == 0){ //game over
               GameOver(score, 0); //call game over function
               break;
           }
           

               
           if (total_counter == total + 1){ //if all the balloons are finished, reset everything
               if (level == 10){
                  Win(score); //WON THE GAME
                  break;
               }
               total += 5; //next level has 5 more total balloons
               level++;
               if (level>=5)
                  increment = level/3 + 1; //increase balloon speed
               spacing-= 4;
               cost += 5;
               for (i = 0; i<= 29; i++){                  
                   lastBalloon = 0;                  
                   delayTimer[i] = 3999;
               }
               for (i = 0; i<= 99; i++){ //reset arrays
                   Balloon[i].x = 160;
                   Balloon[i].y = 0;
                   finished[i] = 0;
                   popped[i] = 0;
                   popped_timer[i] = 0;
                   timer_counter[i] = 0;
               }
               NextWave();
           }
     
           total_counter = 0; //reset the counter   
               
           if (key[KEY_P])
               pause(); //pause if 'p' is pressed
           
           //ready warning
           if (total == 9 && timer_counter[0] == 1){    
              allegro_message("GET READY! GAME IS ABOUT TO START IN 3 SECONDS! MOVE THOSE KANGAROOS ON THE RIGHT AS FAST AS YOU CAN! :)");   
              NextWave();
           }
     }
     
     //destroy bitmaps
     destroy_bitmap(buffer);
     destroy_bitmap(map);
     destroy_bitmap(balloon);
     destroy_bitmap(kangaroo);
     destroy_bitmap(allowed);
     destroy_bitmap(disallowed);
     destroy_bitmap(pop);
}END_OF_FUNCTION(playGame);

void instructions(){
     BITMAP * instructions;
     instructions = load_bitmap("InstructionsPage.bmp", NULL);
     while (!key[KEY_ENTER]){
         draw_sprite(screen, instructions, 0, 0); //displays instruction bitmap until enter key is pressed
     }   
     destroy_bitmap(instructions);
}END_OF_FUNCTION(Instructions);

void GameOver(int score, bool won){
    
     if (won == 0){
         //load midi and stop the original track
         stop_midi();
         MIDI*GameOver1;
         GameOver1 = load_midi("Mockingbird.mid");
         play_looped_midi(GameOver1,1,-1);
         
         BITMAP*GameOver;
         GameOver = load_bitmap("GameOver.bmp", NULL);
         draw_sprite(screen, GameOver, 0, 0);   
         
         clear_keybuf();
         readkey(); //wait for key press 
         destroy_bitmap(GameOver);
     }
     
     //declare and initialize variables necessary for the high score system
     int minScore, i, recordedScore[10], counter = 0, minScoreElement;
     char name[10][10], Continue;
     
     //point to text file
     FILE*file;
     file = fopen("Highscore.txt", "r");
     
     //scan and assign prerecorded scores until End Of File is found
     do {
          fscanf(file, "%d", &recordedScore[counter]);
          fscanf(file, "%s", &name[counter]);   
          counter++;
     }while (fscanf(file, "%c", &Continue) != EOF && counter <= 11); 
     fclose(file);
     
     //find the lowest score
     minScore = recordedScore[0];
     minScoreElement = 0;
     for (i = 0; i<= counter - 1 ; i++){
         if (recordedScore[i] < minScore){
            minScore = recordedScore[i];
            minScoreElement = i;
         }
     }

     //tests to see if score makes the top 10
     if (counter >= 11 && score <= minScore)
         allegro_message("Sorry, you did not make the highscore, but you are encouraged to play again! :)");
     else
         highScore(score, minScoreElement, recordedScore, name); //call highscore function
}END_OF_FUNCTION(GameOver);

void highScore(int score, int MinScoreElement, int scores[], char names[][10]){
     //declare and initialize variables 
     int counter = 0;
     char name[10] = " ";
     bool invalid = 0;
     name[0] = '\0';
     clear_keybuf();
     
     //load bitmaps
     BITMAP*highScore;
     BITMAP*buffer;
     buffer = create_bitmap(SCREEN_W, SCREEN_H);
     highScore = load_bitmap("HighScore.bmp", NULL);
     
     //reads in user key inputs until escap key or enter key is pressed
     while (!key[KEY_ENTER] && !key[KEY_ESC] || (counter == 1 || counter == 0)){
           //update the name input box after every key press
           acquire_screen();
           draw_sprite(buffer, highScore, 0, 0);
           textprintf_ex(buffer, font, 440, 510, makecol(0, 0, 0), -1, "%d", score);
           textprintf_ex(buffer, font, 430 - 7*counter/2, 425, makecol(255, 255, 255), -1, "%s", name); 
           draw_sprite(screen, buffer, 0, 0);
           release_screen();
           
           //reads in key press
           name[counter] = readkey()%256;
           
           //if enter or esc is pressed without a name, show invalid input   
           if (key[KEY_ENTER] || key[KEY_ESC]){
               if (counter == 0){
                   allegro_message("Please enter a name");
                   name[counter] = '\0';
               }
               else
                   break;
           }
           else if (key[KEY_BACKSPACE]){ //deletes character if backspace is pressed
              name[counter] = '\0';
              if (counter > 0){
                 name[counter-1] = '\0';
                 counter--;
              }
           }
           else  if (counter > 7){ //max character limit
               allegro_message("Max Character Limit : 8");
               name[counter] = '\0';
           }            
           else {
                name[counter+1] = '\0'; //NULL value for end of string
                counter++;
           }
           
           //change spacebar to underscore
           if (name[counter-1] == ' ')
              name[counter-1] = '_';
           

           name[counter] = '\0';
     }
     
     //destroy bitmaps
     destroy_bitmap(buffer);
     destroy_bitmap(highScore);
     OverWriteHighScore(MinScoreElement, score, name, scores, names); //replace lowest score in txt with new highscore
     DisplayHighscore(); //display highscores in order
}END_OF_FUNCTION(Highscore);

void OverWriteHighScore(int MinScoreElement, int NewScore, char NewName[10], int scores[], char names[][10]){
     //declare and initialize variables
     int counter = 0, i;
     char Continue, temp[10];
     
     //replace old lowest score and name with NEW highscore and name
     strcpy(temp, NewName);    
     strcpy(names[MinScoreElement - 1], temp);
     scores[MinScoreElement] = NewScore;     
     
     //points to text file again to WRITE
     FILE*overwrite;
     overwrite = fopen("HighScore.txt", "w");
     
     //rewrite textfile with new updated information
     for (i = 0; i<= 9; i++)
         fprintf(overwrite, "%s %d\n", names[i], scores[i+1]);     
     
     fclose(overwrite);
     
}END_OF_FUNCTION(OverWriteHighScore);

void DisplayHighscore(){
     //declare and initialize variables
     int counter = 0, i, j, score[10], tempScore;
     char name[10][10], Continue, tempName[10];
     
     //points to text file to read
     FILE*fptr;
     fptr = fopen("HighScore.txt", "r");
     
     //read in an array of updated scores and names until END OF FILE is scanned
     do {
          fscanf(fptr, "%d", &score[counter]);
          fscanf(fptr, "%s", &name[counter]);   
          counter++;
     }while (fscanf(fptr, "%c", &Continue) != EOF); 
     
     fclose(fptr); //close the pointer
     
     //load bitmaps
     BITMAP*Background;
     BITMAP*buffer;
     Background = load_bitmap("DisplayHighscore.bmp", NULL);
     buffer = create_bitmap(SCREEN_W, SCREEN_H);
     
     //display the background 
     draw_sprite(screen, Background, 0, 0);
     destroy_bitmap(Background); //destroy bitmap
     
     //BUBBLE SORT the highscores by value from smallest to greatest
     for (j = 0; j<= counter-3; j++){
         for (i = 0; i<= counter-3; i++){
             if (score[i+2] < score[i+1]){
                  tempScore = score[i+1];
                  score[i+1] = score[i+2];
                  score[i+2] = tempScore;
                  strcpy(tempName, name[i]);
                  strcpy(name[i], name[i+1]);
                  strcpy(name[i+1], tempName);
             }          
         }
     }
     
     //output highscores in BACKWARD order so it will be highest to lowest
     for (i = counter - 2; i >= 0; i--)
         textprintf_ex(screen, font, 240, 160 + abs(i*30 - 270), makecol(0, 0, 0), -1, "%3d) %-12s- %4d", abs(1+i-11), name[i], score[i+1]);
         
     readkey(); //wait for key press
     //main();
}END_OF_FUNCTION(DisplayHighscore);

void increment_speed_counter ()  { //Interrupt Handler
speed_counter++; }
END_OF_FUNCTION(increment_speed_counter);   

void pause(){
     //load bitmaps
     BITMAP*pause;
     pause = load_bitmap("pause.bmp", NULL);
     
     //display pause screen
     draw_sprite(screen, pause, 0, 0);
     clear_keybuf();
     readkey(); //wait for key press to return back to play function
}END_OF_FUNCTION(pause);

void NextWave(){
     int i;
     for (i = 3; i>= 0; i--){
         textprintf_ex(screen, font, 565, 200, makecol(0, 0, 0), -1, "NEXT WAVE"); 
         textprintf_ex(screen, font, 560, 230, makecol(0, 0, 0), -1, "STARTING IN:");
         textprintf_ex(screen, font, 600, 260, makecol(0, 0, 0), -1, "%d", i);
         rest(1000);
         textprintf_ex(screen, font, 600, 260, makecol(255, 255, 255), -1, "%d", i);
     }
}
void Win(int score){
     //load bitmap
     BITMAP*win;
     win = load_bitmap("win.bmp", NULL);
     
     //load music and play it
     stop_midi();
     MIDI*Win;
     Win = load_midi("JustADream.mid");
     play_looped_midi(Win,1,-1);
     
     //display congrats screen
     draw_sprite(screen, win, 0, 0);
     readkey(); //wait for key press to continue
     GameOver(score, 1);
}END_OF_FUNCTION(Win);
