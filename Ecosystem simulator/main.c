#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_TURNS 6
#define LION_HUNGER_LIMIT 3
#define ZEBRA_HUNGER_LIMIT 3
#define REPRODUCTION_AGE 5
#define GRASS_REGROWTH_PERIOD 5

typedef struct {
    int is_available;       // 1 if grass is available, 0 otherwise
    int regrowth_timer;     // Turns since grass was eaten
} Grass;

typedef struct {
    int age;
    int hunger_counter;
} Lion;

typedef struct {
    int age;
    int hunger_counter;
} Zebra;

typedef struct {
    Lion *lion;             // Pointer to lion in this cell, if any
    Zebra *zebra;           // Pointer to zebra in this cell, if any
    Grass grass;            // Grass status in this cell
} Cell;

// Function prototypes (you can add more)
Cell** read_initial_map(const char *filename, int *height, int *width);
Cell** allocate_grid(int height, int width);
void free_grid(Cell **grid, int height, int width);
void copy_grid(Cell **grid, Cell **grid_buffer, int height, int width);
void display_grid(Cell **grid, int height, int width);
void update_grass(Cell **grid, Cell **grid_buffer, int height, int width);
void reproduction(int i, int j, Cell** grid_buffer, int height, int width);
void move_lions(Cell **grid, Cell **grid_buffer, int height, int width);
void move_zebras(Cell **grid, Cell **grid_buffer, int height, int width);
void simulate(Cell **grid, int height, int width);
void free_memory(Cell **grid, int height, int width);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s sample_map_i.txt\n", argv[0]);
        return EXIT_FAILURE;
    }

    int height, width;
    Cell **grid = read_initial_map(argv[1], &height, &width);
    simulate(grid, height, width);
    free_memory(grid, height, width);
    free_grid(grid, height, width);
    return 0;
}

/*
*Allocates our 2D ecosystem grid to heap
*/
Cell** allocate_grid(int height, int width) {
    // Allocate memory for grid
    int i; // avariable to use it inside of for loop and it loops rows(height)
    Cell** grid = (Cell **) malloc(height * sizeof(Cell*)); //creating grid to allocate rows(height) 

    for(i = 0; i<height; i++){ //to loop the rows in our heap
        grid[i] = malloc(width * sizeof(Cell)); //for every row(height) opening columns.
        grid[i]->lion = NULL; //initializing our struct lion pointer to NULL
        grid[i]->zebra = NULL;//initializing our struct zebra pointer to NULL
    }
    
    return grid; //retuning our heap allocated 2D ecosystem grid
}

/*
* Reads the input file to create 2D ecosystem grid with the informations that lives inside of ecosystem(Lions, Zebras, Grass)
*/
Cell** read_initial_map(const char *filename, int *height, int *width) {
    // Read initial map from file and initialize grid
    int counter = 0; //counter variable to use to track the first two lines(which is given height and width in the first two)
    int i; //a variable to use in for loop to check the zebra and lion
    int j = 0; //a variable to use it to track the height
    char buffer[256]; //variable to keep the input file inside

    FILE *file = fopen(filename, "r"); //opening the file

    while(fgets(buffer,sizeof(buffer), file) != NULL){ 
        counter +=1;
        if(counter == 1){ 
            *height = buffer[0] -'0'; //getting height from file
        }
        else if( counter == 2){
            *width = buffer[0] - '0'; //getting width from file
            break;
        }
        
    }
    Cell** grid = allocate_grid(*height, *width); //allocating 2D ecosystem grid

    while(fgets(buffer, sizeof(buffer), file) != NULL){ //looping through the next lines
        for(i = 0; i< *width; i++){
            if(buffer[i] == 'Z'){ //if the initial grid in the file has zebra at that index
                grid[j][i].zebra = malloc(sizeof(Zebra)); //creating Zebra to that location
                grid[j][i].lion = NULL; //initializing lion to null
                grid[j][i].zebra->age = 0; //initializing the zebra's age to 0
                grid[j][i].zebra->hunger_counter = 0; //initializing zebra's hunger to 0
                grid[j][i].grass.is_available = 0; //initializing grass to isnt available 
                grid[j][i].grass.regrowth_timer = 0; //initializing the grass' growth timer to 0
            }
            else if(buffer[i] == 'L'){  //if the initial grid in the file has zebra at that index
                grid[j][i].lion = malloc(sizeof(Lion)); //creating Lion to that location
                grid[j][i].zebra = NULL; //initializing zebra to null    
                grid[j][i].lion->age = 0; //initializing the lion's age to 0
                grid[j][i].lion->hunger_counter = 0; //initializing lion's hunger to 0
                grid[j][i].grass.is_available = 0; //initializing grass to isnt available 
                grid[j][i].grass.regrowth_timer = 0; //initializing the grass' growth timer to 0
            }
            else if(buffer[i] == 'G'){ //if the initial grid in the file has a grown grass
                grid[j][i].grass.is_available = 1; //initializing grass to make it available
                grid[j][i].lion = NULL; //initializing lion to null
                grid[j][i].zebra = NULL; //initializing zebra to null
            }
            else if (buffer [i] == '-' || buffer[i] == '.'){ //if the initial grid in the file has a non-grown grass
                grid[j][i].grass.is_available = 0; //initializing grass to isnt available 
                grid[j][i].grass.regrowth_timer = 0; //initializing the grass' growth timer to 0
                grid[j][i].lion = NULL; //initializing lion to null
                grid[j][i].zebra = NULL; //initializing zebra to null
            }
            else{
                continue; //skiping to next index
            }
        }
        j += 1; // increasing j to change the row(height)
    }
    fclose(file); //closing the file to not get a memory leak

    return grid;
}

/*
* Frees the grid in the heap.
*/
void free_grid(Cell **grid, int height, int width) {
    // Free memory allocated for grid
    int i; //variable to use it in loop to free each row

    for(i = 0; i< height; i++){
        free(grid[i]); //freeing each row(height)
        grid[i] =NULL; //preventing dangling pointers
    }

    free(grid); //freeing the row(height) pointers
    grid = NULL; //preventing dangling pointers
}

/*
* Copies 2D ecosystem grid_buffer to a grid
*/
void copy_grid(Cell **grid, Cell **grid_buffer, int height, int width) {
    // Copy grid_buffer to grid
    int i; //variable to loop through the rows(height)
    int j; //variable to loop through the columns (width)

    for(i = 0; i< height; i++){ //looping through rows(height)
        for(j=0; j<width; j++){ //looping through columns(width)
            if (grid_buffer[i][j].lion) { //if our original 2D grid_buffer has lion copies it to grid
                grid[i][j].lion = malloc(sizeof(Lion)); //allocating a lion in that location
                *grid[i][j].lion = *grid_buffer[i][j].lion; //deepcopying the specifications
                free(grid_buffer[i][j].lion); //freeing the lion that is inside of grid_buffer to empty(not have any animal) grid_buffer 
                grid_buffer[i][j].lion = NULL; //making it null to prevent dangling pointers 
            } 
            else { //if our original 2D grid_buffer doesnt have lion 
                grid[i][j].lion = NULL;  //in grid making it NULL 
            }

            if(grid_buffer[i][j].zebra){ //if our original 2D grid_buffer has zebra copies it to grid
                grid[i][j].zebra = malloc(sizeof(Zebra)); //allocating a zebra in that location
                *grid[i][j].zebra = *grid_buffer[i][j].zebra; //deepcopying the specifications
                free(grid_buffer[i][j].zebra); //freeing the zebra that is inside of grid_buffer to empty(not have any animal) grid_buffer 
                grid_buffer[i][j].zebra = NULL; //making it null to prevent dangling pointers 
            }
            else{ //if our original 2D grid_buffer doesnt have zebra 
                grid[i][j].zebra = NULL; //in grid making it NULL 
            }

            if(grid_buffer[i][j].grass.is_available == 1 || grid_buffer[i][j].grass.is_available == 0){ //if our 2D ecosytem grid have grass (even if it is grown(available) or not)
                grid[i][j].grass = grid_buffer[i][j].grass; // copying the grass to grid
                grid[i][j].grass.is_available = grid_buffer[i][j].grass.is_available; //copying the availability
                grid[i][j].grass.regrowth_timer = grid_buffer[i][j].grass.regrowth_timer; //copying the regrowth timer

                //making lion and zebra pointers null inside of grid_buffer to make sure grid_buffer is empty
                grid_buffer[i][j].lion = NULL;
                grid_buffer[i][j].zebra = NULL;
            }
        }
    }
}

/*
* Displays our 2D ecosystem grid
*/
void display_grid(Cell **grid, int height, int width) {
    // Display grid
    int i; //variable to loop through rows (heights)
    int j; //variable to loop through columns(widths)

    for(i = 0; i<height; i++){ //looping through height(rows)
        for(j= 0; j<width; j++){ //looping through width(columns)
            if(grid[i][j].zebra){ //if it has zebra prints Z
                printf("%c ", 'Z'); 
            }
            else if(grid[i][j].lion){ //if it has lion prints L
                printf("%c ", 'L');
            }
            else if(grid[i][j].grass.is_available == 0){ //if the grass is non-grown(availability = 0) prints -
                printf("%c ", '-');
            }
            else if(grid[i][j].grass.is_available == 1){ //if the grass is available(grown) prints = G
                printf("%c ", 'G');
            }
        }
        printf("\n"); //prints newline between rows
    }
}

/*
* copies only grass data to buffer_grid and updates
*/
void update_grass(Cell **grid, Cell **grid_buffer, int height, int width) {
    // Copy grass data to buffer grid and update (This is just a way to implement this function)
    int i; //variable to loop through rows
    int j; //variable to loop through columns

    for(i = 0; i<height; i++){ //looping through rows
        for(j = 0; j < width; j++){ //looping through columns
            if(grid[i][j].grass.is_available || !(grid[i][j].grass.is_available)){ //even if it is available or not it copies grass data to grid_buffer
                grid_buffer[i][j].grass = grid[i][j].grass; //copies grass to grid_buffer
                grid_buffer[i][j].grass.is_available = grid[i][j].grass.is_available; //copies grass availability to grid_buffer
                grid_buffer[i][j].grass.regrowth_timer = grid[i][j].grass.regrowth_timer; //copies grass regrowth_timer to grid_buffer
                //making lion and zebra pointer in that location of grid_buffer to Null to not get any errors for initialize
                grid_buffer[i][j].lion = NULL; 
                grid_buffer[i][j].zebra = NULL;
            }
            if(!grid_buffer[i][j].grass.is_available){ //if grass is not grown
                grid_buffer[i][j].grass.regrowth_timer +=1; //increasing regrowth timer
            }
            if(!grid_buffer[i][j].grass.is_available && grid_buffer[i][j].grass.regrowth_timer == GRASS_REGROWTH_PERIOD){ //if grass is not available and regrowth timer is the max
                grid_buffer[i][j].grass.is_available = 1; // make the grass available
            }
        }
    }
}

/*
* Reproducts lion and zebra
*/
void reproduction(int i, int j, Cell** grid_buffer, int height, int width){
    //to track if the current cell contains a lion or zebra because it is important to find out which one is gonna reproduce
    int lion = 0;
    int zebra = 0;
    //to track the the reproduction direction
    int up = 0;
    int right = 0;
    int down  = 0;
    int left  = 0;

    if( i >= 0 && i < height && j >= 0 && j < width){ //check the cell indexes to make sure if it is right
        //this is for deciding if we gonna reproduce lion or zebra
        if(grid_buffer[i][j].lion != NULL){
            lion = 1; //current cell has lion
        }
        if(grid_buffer[i][j].zebra != NULL){
            zebra = 1; //current cell has zebra
        }

    }
    if(i >= 0 && i < height && j >= 0 && j < width ){
        //reproduction to upwards
        if(up == 0){ 
            if(i == 0 ){ //if it is at top edge, reproduct at the bottom of 2d cell grid
                if(lion == 1 && !grid_buffer[height-1][j].lion && !grid_buffer[height-1][j].zebra){ //if it is lion and if the bottom doesnt have any other animal
                    grid_buffer[height-1][j].lion = malloc(sizeof(Lion)); //allocating lion
                    grid_buffer[height-1][j].lion->age = 0; //initializing new born baby's age to 0 
                    grid_buffer[height-1][j].lion->hunger_counter = 0; //initializing new born baby's hunger to 0 
                }
                else if(zebra ==1 &&lion == 0 && !grid_buffer[height-1][j].lion && !grid_buffer[height-1][j].zebra){ //if it is zebra and if the bottom doesnt have any other animal
                    grid_buffer[height-1][j].zebra = malloc(sizeof(Zebra));
                    grid_buffer[height-1][j].zebra->age = 0;
                    grid_buffer[height-1][j].zebra->hunger_counter = 0;
                }
                else{
                    up = 1; //if it cannot move to upwards mark it as complete
                }
            }
            else{ //if it is not at top edge, general movement to up
                if(lion == 1 && !grid_buffer[i-1][j].lion && !grid_buffer[i-1][j].zebra){
                    grid_buffer[i-1][j].lion = malloc(sizeof(Lion));
                    grid_buffer[i-1][j].lion->age = 0;
                    grid_buffer[i-1][j].lion->hunger_counter = 0;
                }
                else if(zebra ==1 &&lion == 0 && !grid_buffer[i-1][j].lion && !grid_buffer[i-1][j].zebra){
                    grid_buffer[i-1][j].zebra = malloc(sizeof(Zebra));
                    grid_buffer[i-1][j].zebra->age = 0;
                    grid_buffer[i-1][j].zebra->hunger_counter = 0;
                }
                else{
                    up = 1;
                }
            }
        }
        else if(right == 0){
            if(j+1 == width){ //if it is at right edge, reproduct at the first column of 2d cell grid
                if(lion == 1 && !grid_buffer[i][0].lion && !grid_buffer[i][0].zebra){
                    grid_buffer[i][0].lion = malloc(sizeof(Lion));
                    grid_buffer[i][0].lion->age = 0;
                    grid_buffer[i][0].lion->hunger_counter = 0;
                }
                else if(zebra ==1 &&lion == 0 && !grid_buffer[i][0].lion && !grid_buffer[i][0].zebra){
                    grid_buffer[i][0].zebra = malloc(sizeof(Zebra));
                    grid_buffer[i][0].zebra->age = 0;
                    grid_buffer[i][0].zebra->hunger_counter = 0;
                }
                else{
                    right = 1;
                }
            }
            else{ //if it is not at right edge, general movement to right
                if(j+1 < width &&lion == 1 && !grid_buffer[i][j+1].lion && !grid_buffer[i][j+1].zebra){
                    grid_buffer[i][j+1].lion = malloc(sizeof(Lion));
                    grid_buffer[i][j+1].lion->age = 0;
                    grid_buffer[i][j+1].lion->hunger_counter = 0;
                }
                else if(j+1 < width &&zebra ==1 &&lion == 0 && !grid_buffer[i][j+1].lion && !grid_buffer[i][j+1].zebra){
                    grid_buffer[i][j+1].zebra = malloc(sizeof(Zebra));
                    grid_buffer[i][j+1].zebra->age = 0;
                    grid_buffer[i][j+1].zebra->hunger_counter = 0;
                }
                else{
                    right = 1;
                }
            }
        }

        else if(i >= 0 && i < height && j >= 0 && j < width &&down == 0){
            if(i +1 == height ){ //if it is at bottom edge, reproduct at the top of 2d cell grid
                if(lion == 1 && !grid_buffer[0][j].lion && !grid_buffer[0][j].zebra){
                    grid_buffer[0][j].lion = malloc(sizeof(Lion));
                    grid_buffer[0][j].lion->age = 0;
                    grid_buffer[0][j].lion->hunger_counter = 0;
                }
                else if(zebra ==1 &&lion == 0 && !grid_buffer[height-1][j].lion && !grid_buffer[height-1][j].zebra){
                    grid_buffer[0][j].zebra = malloc(sizeof(Zebra));
                    grid_buffer[0][j].zebra->age = 0;
                    grid_buffer[0][j].zebra->hunger_counter = 0;
                }
                else{
                    down = 1;
                }
            }
            else{ //if it is not at bottom edge, general movement to down
                if(i+1 < height && lion == 1 && !grid_buffer[i+1][j].lion && !grid_buffer[i+1][j].zebra){
                    grid_buffer[i+1][j].lion = malloc(sizeof(Lion));
                    grid_buffer[i+1][j].lion->age = 0;
                    grid_buffer[i+1][j].lion->hunger_counter = 0;
                }
                else if(i+1 < height && zebra ==1 &&lion == 0 && !grid_buffer[i+1][j].lion && !grid_buffer[i+1][j].zebra){
                    grid_buffer[i+1][j].zebra = malloc(sizeof(Zebra));
                    grid_buffer[i+1][j].zebra->age = 0;
                    grid_buffer[i+1][j].zebra->hunger_counter = 0;
                }
                else{
                    down = 1;
                }
            }

        }
        else if(left == 0){
            if(j == 0){ //if it is at left edge, reproduct at the right of 2d cell grid
                if(lion == 1 && !grid_buffer[i][width -1].lion && !grid_buffer[i][width-1].zebra){
                    grid_buffer[i][width-1].lion = malloc(sizeof(Lion));
                    grid_buffer[i][width-1].lion->age = 0;
                    grid_buffer[i][width-1].lion->hunger_counter = 0;
                }
                else if(zebra ==1 &&lion == 0 && !grid_buffer[i][width-1].lion && !grid_buffer[i][width-1].zebra){
                    grid_buffer[i][width-1].zebra = malloc(sizeof(Zebra));
                    grid_buffer[i][width-1].zebra->age = 0;
                    grid_buffer[i][width-1].zebra->hunger_counter = 0;
                }
                else{
                    left = 1;
                }
            }
            else{ //if it is not at left edge, general movement to right
                if(lion == 1 && !grid_buffer[i][j-1].lion && !grid_buffer[i][j-1].zebra){
                    grid_buffer[i][j-1].lion = malloc(sizeof(Lion));
                    grid_buffer[i][j-1].lion->age = 0;
                    grid_buffer[i][j-1].lion->hunger_counter = 0;
                }
                else if(zebra ==1 &&lion == 0 && !grid_buffer[i][j-1].lion && !grid_buffer[i][j-1].zebra){
                    grid_buffer[i][j-1].zebra = malloc(sizeof(Zebra));
                    grid_buffer[i][j-1].zebra->age = 0;
                    grid_buffer[i][j-1].zebra->hunger_counter = 0;
                }
                else{
                    left = 1;
                }
            }
        }
    }

}

/*
* moves lion to downwards way and if it needs to reproduce it reproduces and if there is zebra it eats it  
* and if the hunger counter is full it dies
*/
void move_lions(Cell **grid, Cell **grid_buffer, int height, int width) {
    // Logic to move lions
    int i; //variable to loop through rows
    int j; //variable to loop through columns

    for(i = 0; i < height; i++){ //looping through row
        for(j = 0; j < width; j++){ //looping through columns to get cells
            int height_helper = i+1; // helper to calculate the next row (it helps to decide if our lion is at the bottomedge )

            if(grid[i][j].lion){ //if our cell has lion
                if(height_helper == height){ //if our lion is at the bottom edge (which means it will move top edge with keeping column)
                    if(grid[0][j].zebra){ //if the cell that lion is gonna move has zebra
                        grid_buffer[0][j].lion = malloc(sizeof(Lion)); //allocating lion at the new cell
                        *grid_buffer[0][j].lion = *grid[i][j].lion; //deepcopying from grid
                        grid_buffer[0][j].lion->age += 1; //increasing age
                        grid_buffer[0][j].lion->hunger_counter = 0; // making hungercounter to 0 because it eats zebra

                        free(grid[0][j].zebra); //freeing zebra because it dies
                        grid[0][j].zebra = NULL; //preventing from dangling pointers

                        free(grid[i][j].lion); //freeing lion in the GRID because it moved and new information is in grid_buffer
                        grid[i][j].lion = NULL; //preventing from dangling pointers
                        if(0<=i && i<height && 0<=j && j<width){ 
                            if(grid_buffer[0][j].lion->age >= REPRODUCTION_AGE){ //if it is at reproduction age
                                reproduction(0,j,grid_buffer,height,width); //reproducts
                                continue;
                            }
                        }
                    }
                    else if(grid[0][j].lion){ //if the cell that lion is gonna move has lion (it wont move)
                        grid_buffer[i][j].lion = malloc(sizeof(Lion)); //allocation lion at the same index cell but in our buffer_grid
                        *grid_buffer[i][j].lion = *grid[i][j].lion; //deepcopies the lion
                        grid_buffer[i][j].lion->age += 1; //increasing the age
                        grid_buffer[i][j].lion->hunger_counter += 1; //increasing the hunger_counter
                        free(grid[i][j].lion); //freeing lion in the GRID because new information is in grid_buffer
                        grid[i][j].lion = NULL; //preventing from dangling pointers

                        if(grid_buffer[i][j].lion->hunger_counter == LION_HUNGER_LIMIT){ //if the hunger_counter is at the max it dies
                            free(grid_buffer[i][j].lion); //freeing it in the grid_buffer since the new info is at there
                            grid_buffer[i][j].lion = NULL; //preventing from dangling pointers
                            continue;
                        }
                        if(0<=i && i<height && 0<=j && j<width){
                            if(grid_buffer[i][j].lion->age >= REPRODUCTION_AGE){ //if it is at reproduction age
                                reproduction(i,j,grid_buffer,height,width); //reproducts
                                continue;
                            }
                        }
                        continue;
                    }
                    else{ //normal movement 
                        grid_buffer[0][j].lion = malloc(sizeof(Lion)); //allocating lion at the new cell in grid_buffer
                        *grid_buffer[0][j].lion = *grid[i][j].lion; //deepcopying it
                        grid_buffer[0][j].lion->age += 1; //increasing the age
                        grid_buffer[0][j].lion->hunger_counter += 1; //increasing the hunger_counter
                        free(grid[i][j].lion); //freeing lion in the GRID because new information is in grid_buffer
                        grid[i][j].lion = NULL; //preventing from dangling pointers
                        if(grid_buffer[0][j].lion->hunger_counter == LION_HUNGER_LIMIT){ //if the hunger_counter is at the max it dies
                            free(grid_buffer[0][j].lion); //freeing it in the grid_buffer since the new info is at there
                            grid_buffer[0][j].lion = NULL;
                            continue;
                        }
                        if(0<=i && i<height && 0<=j && j<width){
                            if(grid_buffer[0][j].lion->age >= REPRODUCTION_AGE){ //if it is at reproduction age
                                reproduction(0,j,grid_buffer,height,width); //reproducts
                                continue;
                            }
                        }
                    }
                }
                else if(height_helper <height){ //if the lion is not at the bottom edge moves down normally
                //it is same logic with the moving it to the up edge that you can see at above
                    if(i+1 < height && grid[i+1][j].zebra){ //if the cell that our lion is gonna move has zebra it eats it
                        grid_buffer[i+1][j].lion = malloc(sizeof(Lion));
                        *grid_buffer[i+1][j].lion = *grid[i][j].lion;
                        grid_buffer[i+1][j].lion->age += 1;
                        grid_buffer[i+1][j].lion->hunger_counter = 0;
                        free(grid[i+1][j].zebra);
                        grid[i+1][j].zebra = NULL;
                        free(grid[i][j].lion);
                        grid[i][j].lion = NULL;
                        if(0<=i && i<height && 0<=j && j<width && i+1 < height){
                            if(grid_buffer[i+1][j].lion->age >= REPRODUCTION_AGE){ //if it is at reproduction age
                                reproduction(i+1,j,grid_buffer,height,width); //reproducts
                                continue;
                            }
                        }
                    }

                    else if(grid[i+1][j].lion){ //if the cell that our lion is gonna move has lion it stays at the same place 
                        grid_buffer[i][j].lion = malloc(sizeof(Lion));
                        *grid_buffer[i][j].lion = *grid[i][j].lion;
                        grid_buffer[i][j].lion->age += 1;
                        grid_buffer[i][j].lion->hunger_counter += 1;
                        free(grid[i][j].lion);
                        grid[i][j].lion = NULL;
                        if(grid_buffer[i][j].lion->hunger_counter == LION_HUNGER_LIMIT){//if the hunger_counter is at the max it dies
                            free(grid_buffer[i][j].lion);
                            grid_buffer[i][j].lion = NULL;
                            continue;
                        }
                        if(0<=i && i<height && 0<=j && j<width){
                            if(grid_buffer[i][j].lion->age >= REPRODUCTION_AGE){ //if it is at reproduction age
                                reproduction(i,j,grid_buffer,height,width); //reproducts
                                continue;
                            }
                        }
                        
                        continue;
                    }
                    else{ //if it doesnt have anything it moves normally at downwards
                        grid_buffer[i+1][j].lion = malloc(sizeof(Lion));
                        *grid_buffer[i+1][j].lion = *grid[i][j].lion;
                        grid_buffer[i+1][j].lion->age += 1;
                        grid_buffer[i+1][j].lion->hunger_counter += 1;
                        free(grid[i][j].lion);
                        grid[i][j].lion = NULL;
                        if(grid_buffer[i+1][j].lion->hunger_counter == LION_HUNGER_LIMIT){//if the hunger_counter is at the max it dies
                            free(grid_buffer[i+1][j].lion);
                            grid_buffer[i+1][j].lion = NULL;
                            continue;
                        }
                        if(0<=i && i<height && 0<=j && j<width && i+1 < height){
                            if(grid_buffer[i+1][j].lion->age >= REPRODUCTION_AGE){ //if it is at reproduction age
                                reproduction(i+1,j,grid_buffer,height,width); //reproducts
                                continue;
                            }
                        }
                    }
                    
                }
            }
            else{
                continue;
            }
        }
    }
}

/*
* moves zebra to rightwards and if it needs to reproduce it reproduces and if there is grown grass(isavailable = 1) it eats it  
* and if the hunger counter is full it dies
*/
void move_zebras(Cell **grid, Cell **grid_buffer, int height, int width) {
    // Logic to move zebras
    int i; //variable to loop through rows
    int j; //variable to loop through columns

    for(i = 0 ; i<height; i++){ //iterating through rows
        for(j = 0 ; j<width ; j++){ //iterating through columns
            int width_helper  =j+1; // to find out if it is at the right edge
            
            if(grid[i][j].zebra){ //if our current cell has zebra
                if(width_helper == width){ //if it is at the right edge it moves to the left col
                    if(!grid[i][0].zebra && !grid_buffer[i][0].lion && !grid_buffer[i][0].zebra && grid_buffer[i][0].grass.is_available == 1){ //if the cell that our  zebra is gonna move doesnt have zebra and lion and there is only grown grass it will eat
                        grid_buffer[i][0].zebra = malloc(sizeof(Zebra)); //allocating zebra in grid_buffer
                        *grid_buffer[i][0].zebra = *grid[i][j].zebra; //deepcopying to grid_buffer
                        grid_buffer[i][0].grass.is_available = 0; // resetting grass availability since it is eaten
                        grid_buffer[i][0].grass.regrowth_timer = 0; //resetting regrowth_timer since it is eaten
                        //upgrading zebra's specifications
                        grid_buffer[i][0].zebra->age +=1;
                        grid_buffer[i][0].zebra->hunger_counter = 0;
                        free(grid[i][j].zebra); //freeing zebra in our original grid since grid_buffer has new informations
                        grid[i][j].zebra = NULL;

                        if(0<=i && i<height && 0<=j && j<width){
                            if(grid_buffer[i][0].zebra->age >= REPRODUCTION_AGE){ //if it is at reproduction age
                                reproduction(i,0,grid_buffer,height,width); //reproducts
                                continue;
                            }
                        }
                        continue;
                    }
                    
                    else if(grid[i][0].zebra || grid_buffer[i][0].lion || grid_buffer[i][0].zebra){ //if the destination cell has lion or zebra  it doesnt move
                        if(grid_buffer[i][j].grass.is_available == 1){ //if the cell that our zebra is gonna stay has grown grass it stays and eats it
                            grid_buffer[i][j].zebra = malloc(sizeof(Zebra)); //allocating zebra in the cell that is gonna stay in our grid_buffer
                            *grid_buffer[i][j].zebra = *grid[i][j].zebra; //deepcopying zebra from grid
                            //the grass is getting eaten on grid_buffer since it keeps our new information
                            grid_buffer[i][j].grass.is_available = 0; 
                            grid_buffer[i][j].grass.regrowth_timer = 0;
                            //updating our zebra's stats
                            grid_buffer[i][j].zebra->age +=1;
                            grid_buffer[i][j].zebra->hunger_counter = 0;
                            free(grid[i][j].zebra); //freeing zebra in our original grid since grid_buffer has new informations
                            grid[i][j].zebra = NULL;
                        }
                        else{ //if the cell that our zebra is gonna stay doesnt have grown grass it just stays
                            grid_buffer[i][j].zebra = malloc(sizeof(Zebra)); //allocating zebra in the cell that is gonna stay in our grid_buffer
                            *grid_buffer[i][j].zebra = *grid[i][j].zebra;//deepcopying zebra from grid
                            //updating our zebra's stats
                            grid_buffer[i][j].zebra->age +=1;
                            grid_buffer[i][j].zebra->hunger_counter +=1;
                            free(grid[i][j].zebra);//freeing zebra in our original grid since grid_buffer has new informations
                            grid[i][j].zebra = NULL;
                            if(grid_buffer[i][j].zebra->hunger_counter == ZEBRA_HUNGER_LIMIT){ //if our zebra's hungercounter at max it dies
                                free(grid_buffer[i][j].zebra);
                                grid_buffer[i][j].zebra = NULL;
                                continue;
                            }
                        }
                        if(0<=i && i<height && 0<=j && j<width){
                            if(grid_buffer[i][j].zebra->age >= REPRODUCTION_AGE){ //if the age is at reproduction age 
                                reproduction(i,j,grid_buffer,height,width); //reproduces
                                continue;
                            }
                        }
                        
                        continue;
                    }

                    else{ //normal movement while it is on right edge
                        grid_buffer[i][0].zebra = malloc(sizeof(Zebra)); //allocating zebra in grid_buffer
                        *grid_buffer[i][0].zebra = *grid[i][j].zebra; //deepcopying from grid
                        //updating zebra's stats
                        grid_buffer[i][0].zebra->age +=1; 
                        grid_buffer[i][0].zebra->hunger_counter += 1;
                        free(grid[i][j].zebra); //freeing zebra in our original grid since grid_buffer has new informations
                        grid[i][j].zebra = NULL;
                        if(grid_buffer[i][0].zebra->hunger_counter == ZEBRA_HUNGER_LIMIT){  //if our zebra's hungercounter at max it dies
                            free(grid_buffer[i][0].zebra);
                            grid_buffer[i][0].zebra = NULL;
                            continue;
                        }
                        if(0<=i && i<height && 0<=j && j<width){
                            if(grid_buffer[i][0].zebra->age >= REPRODUCTION_AGE){ //if the age is at reproduction age 
                                reproduction(i,0,grid_buffer,height,width); //reproduces
                                continue;
                            }
                        }
                    }
                    
                }
                else if(width_helper < width){ //if it is not at the right edge it moves normally to right
                //this has same logic with the right edge one except it moves normally to one right depends on the conditions
                    if(!grid[i][j+1].zebra && !grid_buffer[i][j+1].lion && !grid_buffer[i][j+1].zebra && grid_buffer[i][j+1].grass.is_available == 1){ //this means there is grown grass but no animals occupies the destination cell so it will move and eat
                        grid_buffer[i][j+1].zebra = malloc(sizeof(Zebra));
                        *grid_buffer[i][j+1].zebra = *grid[i][j].zebra;
                        grid_buffer[i][j+1].grass.is_available = 0;
                        grid_buffer[i][j+1].grass.regrowth_timer = 0;
                        grid_buffer[i][j+1].zebra->age +=1;
                        grid_buffer[i][j+1].zebra->hunger_counter = 0;
                        free(grid[i][j].zebra);
                        grid[i][j].zebra = NULL;

                        if(0<=i && i<height && 0<=j && j<width && j+1 < width){
                            if(grid_buffer[i][j+1].zebra->age >= REPRODUCTION_AGE){
                                reproduction(i,j+1,grid_buffer,height,width);
                                continue;
                            }
                        }
                    }

                    else if(grid[i][j+1].zebra || grid_buffer[i][j+1].lion || grid_buffer[i][j+1].zebra){ //this means there is animal in the cell that our zebra is gonna move so it stays at the same place
                        if(grid_buffer[i][j].grass.is_available == 1){  //if there is grown grass it eats it
                            grid_buffer[i][j].zebra = malloc(sizeof(Zebra));
                            *grid_buffer[i][j].zebra = *grid[i][j].zebra;
                            grid_buffer[i][j].grass.is_available = 0;
                            grid_buffer[i][j].grass.regrowth_timer = 0;
                            grid_buffer[i][j].zebra->age +=1;
                            grid_buffer[i][j].zebra->hunger_counter = 0;
                            free(grid[i][j].zebra);
                            grid[i][j].zebra = NULL;
                        }
                        else{
                            grid_buffer[i][j].zebra = malloc(sizeof(Zebra));
                            *grid_buffer[i][j].zebra = *grid[i][j].zebra;
                            grid_buffer[i][j].zebra->age +=1;
                            grid_buffer[i][j].zebra->hunger_counter +=1;
                            free(grid[i][j].zebra);
                            grid[i][j].zebra = NULL;
                            if(grid_buffer[i][j].zebra->hunger_counter == ZEBRA_HUNGER_LIMIT){
                                free(grid_buffer[i][j].zebra);
                                grid_buffer[i][j].zebra = NULL;
                                continue;
                            }
                        }
                        if(0<=i && i<height && 0<=j && j<width && j+1 < width){
                            if(grid_buffer[i][j].zebra->age >= REPRODUCTION_AGE){
                                reproduction(i,j,grid_buffer,height,width);
                                continue;
                            }
                        }
                        continue;
                    }
                    else{ //moves normally no eat 
                        grid_buffer[i][j+1].zebra = malloc(sizeof(Zebra));
                        *grid_buffer[i][j+1].zebra = *grid[i][j].zebra;
                        grid_buffer[i][j+1].zebra->age +=1;
                        grid_buffer[i][j+1].zebra->hunger_counter += 1;
                        free(grid[i][j].zebra);
                        grid[i][j].zebra = NULL;
                        if(grid_buffer[i][j+1].zebra->hunger_counter == ZEBRA_HUNGER_LIMIT){
                            free(grid_buffer[i][j+1].zebra);
                            grid_buffer[i][j+1].zebra = NULL;
                            continue;
                        }
                        if(0<=i && i<height && 0<=j && j<width && j+1 < width){
                            if(grid_buffer[i][j+1].zebra->age >= REPRODUCTION_AGE){
                                reproduction(i,j+1,grid_buffer,height,width);
                                continue;
                            }
                        }
                    }
                }
            }
            else{
                continue;
            }
            
        }
    }
}

/*
* creating our simulation
*/
void simulate(Cell **grid, int height, int width) {
    // Simulate the ecosystem
    int turns = 1; //track the turns
    Cell **buffer_grid = allocate_grid(height,width); //allocating our grid_buffer which keeps our movements aka updated grid then it is copied back to grid and getting emptied to get prepared for new turn

    while(turns <= MAX_TURNS){ //keep the simulation loop till the max turns
        printf("Turn %d\n", turns);
        update_grass(grid, buffer_grid, height,width); //updates grass to buffer_grid since we gonna use buffer_grid as updated grid
        move_lions(grid,buffer_grid,height,width); //move lions and putting them into buffer_grid 
        move_zebras(grid,buffer_grid,height,width); //move zebras and putting them into buffer_grid
        copy_grid(grid,buffer_grid,height,width); //copying our buffer_grid(updated grid) to our original grid
        display_grid(grid,height,width); //displaying our grid
        printf("\n");
        turns +=1;
    }
    free_grid(buffer_grid,height,width); //freeing our buffer_grid

} 

/*
*freeing every allocated memory for lions and zebras in our ecosystem grid
*/
void free_memory(Cell **grid, int height, int width) {
    // Free any remaining entities
    int i; //variable for looping through rows
    int j; //variable for looping through columns
    //looping through cells
    for(i = 0; i <height; i++){
        for(j = 0; j<width; j++){
            //if the cell contains lion free it's memory
            if(grid[i][j].lion){
                free(grid[i][j].lion);
                grid[i][j].lion = NULL;
            }
            //if the cell contains zebra free it's memory
            else{
                free(grid[i][j].zebra);
                grid[i][j].zebra = NULL;
            }
        }
    }
}
