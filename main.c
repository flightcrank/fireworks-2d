//main.c 

//Using libs SDL, glibc
#include <SDL.h>	//SDL version 2.0
#include <stdio.h>
#include <stdlib.h>
#include "renderer.h"
#include "vector.h"

#define SCREEN_WIDTH 640	//window height
#define SCREEN_HEIGHT 480	//window width
#define FIREWORKS 10		//number of fireworks
#define PARTICALS 50		//number of particles a firework explodes into
#define SCALE .02			//adjust how high the fireworks will go to suit your screen resolution

struct partical {

	struct vector2d pos;	//position
	struct vector2d vel;	//velocity
	float alpha;			//acceleration
};

struct firework {

	struct partical property;			//contains fireworks position,vel and accel property's
	struct partical ptcls[PARTICALS];	//particles firework will explode into
	uint8_t r;							//firework colour
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
struct firework fwrk[FIREWORKS];
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
	
	int i, j;
	
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
			SDL_SetTextureColorMod(ball_t, fwrk[i].r, fwrk[i].g, fwrk[i].b);
			
			//draw particles
			for (j = 0; j < PARTICALS; j++) {
			
				dest.x = fwrk[i].ptcls[j].pos.x;
				dest.y = fwrk[i].ptcls[j].pos.y;
				dest.w = 4;
				dest.h = 4;
				
				//draw particles 
				SDL_SetTextureAlphaMod(ball_t, (uint8_t) fwrk[i].ptcls[j].alpha);
				SDL_RenderCopy(renderer, ball_t, NULL, &dest);
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
	
	int j;
	
	//fireworks has reached its peak
	if (fwrk[i].property.vel.y >= 0 ) {
			
			fwrk[i].property.alpha = 0;
		
		//calculate particle physics
		for(j = 0; j < PARTICALS; j++) {
			
			if (fwrk[i].ptcls[j].alpha > 0) {
				
				fwrk[i].ptcls[j].alpha -= 5;

			} else {
				
				fwrk[i].ptcls[j].alpha -= 0;
			}
			
			//particles have faded away
			if(fwrk[i].ptcls[0].alpha <= 0) {
				
				init_firework(i);
			}
			
			//change position based on velocity
			add_vector(&fwrk[i].ptcls[j].pos, &fwrk[i].ptcls[j].vel);
			
			//change velocity based on acceleration
			add_vector(&fwrk[i].ptcls[j].vel, &gravity);
		}
		
	//calculate firework physics
	} else {
	
		//change position based on velocity
		add_vector(&fwrk[i].property.pos, &fwrk[i].property.vel);
		
		//change velocity based on acceleration
		add_vector(&fwrk[i].property.vel, &gravity);
		
		for(j = 0; j < PARTICALS; j++) {
		
			fwrk[i].ptcls[j].pos.x = fwrk[i].property.pos.x;
			fwrk[i].ptcls[j].pos.y = fwrk[i].property.pos.y;
		}
	}
}

void init_firework(int i) {
	
	int j;
	
	float vel_scale = SCALE * height;
	
	//set up firework property's
	fwrk[i].property.pos.x = rand() % width;
	fwrk[i].property.pos.y = height;
	fwrk[i].property.vel.x = sin(rand());
	fwrk[i].property.vel.y = (float) (rand() % 5 + (int) vel_scale)  * -1;
	fwrk[i].property.alpha = 255;
	fwrk[i].r = rand() % 155 + 100;
	fwrk[i].g = rand() % 155 + 100;
	fwrk[i].b = rand() % 155 + 100;

	for (j = 0; j < PARTICALS; j++) {
		
		fwrk[i].ptcls[j].pos.x = fwrk[i].property.pos.x;
		fwrk[i].ptcls[j].pos.y = fwrk[i].property.pos.y;
		fwrk[i].ptcls[j].vel.x = sin(rand() % 360);
		fwrk[i].ptcls[j].vel.y = sin(rand() % 360);
		fwrk[i].ptcls[j].alpha = 255;
		multiply_vector(&fwrk[i].ptcls[j].vel, rand() % 4 + 1);
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
