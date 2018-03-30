//main.c 

//Using libs SDL, glibc
#include <SDL.h>	//SDL version 2.0
#include <stdio.h>
#include <stdlib.h>
#include "renderer.h"
#include "vector.h"

#define SCREEN_WIDTH 1290	//window height
#define SCREEN_HEIGHT 720	//window width
#define FIREWORKS 20		//number of fireworks
#define PARTICALS 50		//number of particles a firework explodes into
#define SCALE .018			//adjust how high the fireworks will go to suit your screen resolution
#define TRAIL 2				//trail the particles leave behind

struct partical {

	struct vector2d pos;		//position
	struct vector2d vel;		//velocity
	struct vector2d trail[TRAIL];	//array of previous positions
	float alpha;				//acceleration
};

struct firework {

	struct partical property;				//contains fireworks position,vel and accel property's
	struct partical particles[PARTICALS];	//particles firework will explode into
	uint8_t r;								//firework colour
	uint8_t g;
	uint8_t b;
};

//function prototypes
int init(int w, int h, int argc, char *args[]);
void force(struct vector2d force);
void update(int index);
void init_firework(int index);

//globals
SDL_Window* window = NULL;		//The window we'll be rendering to
SDL_Renderer *renderer;			//The renderer SDL will use to draw to the screen
SDL_Texture *ball_t;
struct pix_buff ball_pb;
int width, height;				//used if fullscreen
struct firework fireworks[FIREWORKS];
struct vector2d gravity;

int main (int argc, char *args[]) {
	
	//SDL Window setup
	if (init(SCREEN_WIDTH, SCREEN_HEIGHT, argc, args) == 1) {
		
		return 0;
	}
	
	SDL_GetWindowSize(window, &width, &height);
	
	//create firework sprite
	draw_circle(&ball_pb, ball_pb.width / 2, ball_pb.height / 2, ball_pb.width / 2, 0xffffffff);
	SDL_UpdateTexture(ball_t, NULL, ball_pb.pixels, ball_pb.width * sizeof(uint32_t));
	
	//define the world gravity
	gravity.x = 0;
	gravity.y = 0.2;
	
	int i, j, k;
	
	//populate fireworks property's
	for (i = 0; i < FIREWORKS; i++) {
		
		init_firework(i);
	}
	
	int sleep = 0;
	int quit = 0;
	Uint32 next_game_tick = SDL_GetTicks();
		
	//render loop
	while(quit == 0) {
		
		//check for new events every frame
		SDL_PumpEvents();

		const Uint8 *state = SDL_GetKeyboardState(NULL);
		
		if (state[SDL_SCANCODE_ESCAPE]) {
		
			quit = 1;
		}
	
		//draw background
		SDL_RenderClear(renderer);
		
		SDL_Rect dest;
		
		//draw the fireworks
		for (i = 0; i < FIREWORKS; i++) {
		
			//draw firework
			dest.x = fireworks[i].property.pos.x;
			dest.y = fireworks[i].property.pos.y;
			dest.w = 5;
			dest.h = 5;
		
			SDL_SetTextureColorMod(ball_t, fireworks[i].r, fireworks[i].g, fireworks[i].b);
			SDL_SetTextureAlphaMod(ball_t, (uint8_t) fireworks[i].property.alpha);
			float alpha = fireworks[i].property.alpha;
			
			//draw fireworks if they have not yet exploded
			if (alpha) {
				
				SDL_RenderCopy(renderer, ball_t, NULL, &dest);
			}
			
			//draw firework trails
			for(k = 0; k < TRAIL; k++) {
			
				dest.x = fireworks[i].property.trail[k].x;
				dest.y = fireworks[i].property.trail[k].y;
				dest.w = 3;
				dest.h = 3;
				
				SDL_RenderCopy(renderer, ball_t, NULL, &dest);
			}
			
			//draw particles
			for (j = 0; j < PARTICALS; j++) {
			
				dest.x = fireworks[i].particles[j].pos.x;
				dest.y = fireworks[i].particles[j].pos.y;
				dest.w = 3;
				dest.h = 3;
				
				//draw particles if the firework has exploded and has no alpha
				
				if (!alpha) {
					
					SDL_SetTextureAlphaMod(ball_t, (uint8_t) fireworks[i].particles[j].alpha);
					SDL_RenderCopy(renderer, ball_t, NULL, &dest);
				
					//draw particle trails
					for(k = 0; k < TRAIL; k++) {
					
						dest.x = fireworks[i].particles[j].trail[k].x;
						dest.y = fireworks[i].particles[j].trail[k].y;
						dest.w = 3;
						dest.h = 3;
						
						SDL_RenderCopy(renderer, ball_t, NULL, &dest);
					}
				}
			}
			
			//update physics
			update(i);
		}
		
		//draw to the screen
		SDL_RenderPresent(renderer);
		
		//time it takes to render 1 frame in milliseconds
		next_game_tick += 1000 / 60;
		sleep = next_game_tick - SDL_GetTicks();
		
		if( sleep >= 0 ) {
            				
			SDL_Delay(sleep);
		} 
	}

	//free renderer and all textures used with it
	SDL_DestroyRenderer(renderer);
	
	//Destroy window 
	SDL_DestroyWindow(window);
	
	free_pixel_buffer(&ball_pb);
	
	//Quit SDL subsystems 
	SDL_Quit(); 
	 
	return 0;
}

void update(int i) {
	
	int j, k;
	
	//fireworks has reached its peak
	if (fireworks[i].property.vel.y >= 0 ) {
			
			fireworks[i].property.alpha = 0;
		
		//calculate particle physics
		for(j = 0; j < PARTICALS; j++) {
			
			if (fireworks[i].particles[j].alpha > 0) {
				
				fireworks[i].particles[j].alpha -= 5;

			} else {
				
				fireworks[i].particles[j].alpha -= 0;
			}
			
			//particles have faded away
			if(fireworks[i].particles[0].alpha <= 0) {
				
				init_firework(i);
			}
			
			//save particle previous position
			float prev_pos_x = fireworks[i].particles[j].pos.x;
			float prev_pos_y = fireworks[i].particles[j].pos.y;
			
			//change position based on velocity
			add_vector(&fireworks[i].particles[j].pos, &fireworks[i].particles[j].vel);
			
			//change velocity based on acceleration
			add_vector(&fireworks[i].particles[j].vel, &gravity);

			//keep track of fireworks previous positions
			for(k = 0; k < TRAIL; k++) {
				
				//store current cell position in a temp variable
				float temp_x = fireworks[i].particles[j].trail[k].x;
				float temp_y = fireworks[i].particles[j].trail[k].y;
				
				//update current cell with the last known prev position
				fireworks[i].particles[j].trail[k].x = prev_pos_x;
				fireworks[i].particles[j].trail[k].y = prev_pos_y;
				
				//update the prev position for the next iteration of the loop
				prev_pos_x = temp_x;
				prev_pos_y = temp_y;
			}
		}
		
	//calculate firework physics
	} else {
		
		//save fireworks previous position
		float prev_pos_x = fireworks[i].property.pos.x;
		float prev_pos_y = fireworks[i].property.pos.y;
		
		//change position based on velocity
		add_vector(&fireworks[i].property.pos, &fireworks[i].property.vel);
		
		//change velocity based on acceleration
		add_vector(&fireworks[i].property.vel, &gravity);
		
		//make all particle follow the fire work
		for(j = 0; j < PARTICALS; j++) {
		
			fireworks[i].particles[j].pos.x = fireworks[i].property.pos.x;
			fireworks[i].particles[j].pos.y = fireworks[i].property.pos.y;
		}
		
		//keep track of fireworks previous positions
		for(k = 0; k < TRAIL; k++) {
			
			//store current cell position in a temp variable
			float temp_x = fireworks[i].property.trail[k].x;
			float temp_y = fireworks[i].property.trail[k].y;
			
			//update current cell with the last known prev position
			fireworks[i].property.trail[k].x = prev_pos_x;
			fireworks[i].property.trail[k].y = prev_pos_y;
			
			//update the prev position for the next iteration of the loop
			prev_pos_x = temp_x;
			prev_pos_y = temp_y;
		}
	}
}

void init_firework(int i) {
	
	int j, k;
	
	float vel_scale = SCALE * height;
	
	//set up firework property's
	fireworks[i].property.pos.x = rand() % width;
	fireworks[i].property.pos.y = height;
	fireworks[i].property.vel.x = sin(rand());
	fireworks[i].property.vel.y = (float) (rand() % 5 + (int) vel_scale)  * -1;
	fireworks[i].property.alpha = 255;
	fireworks[i].r = rand() % 155 + 100;
	fireworks[i].g = rand() % 155 + 100;
	fireworks[i].b = rand() % 155 + 100;
	
	//record of previous positions for each firework
	for (k = 0; k < TRAIL; k++) {
	
		fireworks[i].property.trail[k].x = fireworks[i].property.pos.x;
		fireworks[i].property.trail[k].y = fireworks[i].property.pos.y;
	}

	for (j = 0; j < PARTICALS; j++) {
		
		//set up firework's particle
		fireworks[i].particles[j].pos.x = fireworks[i].property.pos.x;
		fireworks[i].particles[j].pos.y = fireworks[i].property.pos.y;
		fireworks[i].particles[j].vel.x = sin(rand() % 360);
		fireworks[i].particles[j].vel.y = sin(rand() % 360);
		fireworks[i].particles[j].alpha = 255;
		multiply_vector(&fireworks[i].particles[j].vel, rand() % 4 + 1);
		
		//record of previous positions for each particle
		for (k = 0; k < TRAIL; k++) {
		
			fireworks[i].particles[j].trail[k].x = 0;
			fireworks[i].particles[j].trail[k].y = 0;
		}
	}
}

int init(int width, int height, int argc, char *args[]) {

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {

		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		
		return 1;
	} 
	
	int i;
	
	for (i = 0; i < argc; i++) {
		
		//Create window	
		if(strcmp(args[i], "-f")) {
			
			SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN, &window, &renderer);
		
		} else {
		
			SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);
		}
	}
	
	if (window == NULL) { 
		
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		
		return 1;
	}

	//create and allocate custom pixel buffer
	create_pixel_buffer(&ball_pb, 100, 100);
	
	//set up screen texture
	ball_t = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, ball_pb.width, ball_pb.height);
	SDL_SetTextureBlendMode(ball_t, SDL_BLENDMODE_ADD);
	
	if (ball_pb.pixels == NULL) { 
		
		printf("could not allocate ball pixel buffer\n");
		
		return 1;
	}
		
	if (ball_t == NULL) { 
		
		printf("could not create ball texture SDL_Error: %s\n", SDL_GetError());
		
		return 1;
	}
	
	return 0;
}
