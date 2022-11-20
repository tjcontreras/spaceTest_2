/*Game title: Space Wars
* By Group 8
* ANTOC, Jan Luis V.
* Contreras, Teofilo Jr. M.
* Megino, Kyle Jomar C.
* Presented to: Prof. Carlo Ochotorena
* In partial fullfillment of course CPECOG2
* Date Submitted: September 17, 2021
*/


#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include<windows.h>
#include<time.h>

#include "CImg.h"
#include "MiniFB.h"

//dimensions of the framebuffer
#define WIDTH	1280
#define HEIGHT	720

//Player SpaceShip Global Vars
#define X_OFFSET 25 //x center of spaceship
#define Y_OFFSET 25 //y center of spaceship
#define SHIP_SPEED 25 // pixel displacement of the ship per arrow keypress
#define KILLS_NEEDED 5 // Kills required for the player to win
int16_t X_REF = WIDTH / 2;  // initial x position of the spaceship
int16_t Y_REF = HEIGHT / 2; // initial y position of the spaceship
bool moved = true; // set when arrow keys are pressed
bool isfired = false; //set when spacebar is pressed

//Game State Variable
int gamestate = 0;

//Frame buffer global variables
uint8_t* bg_mem;
uint8_t* bg_mem_temp;
uint32_t* framebuffer;

//Insert Zedboard Globar Variables Here

//

using namespace cimg_library;
using namespace std;

//Class for player and enemy bullet
class bullet {
public:
	//location variables
	int16_t xc; //bullet x-axis position
	int16_t yc; //bullet y-axis position
	int16_t dy; // change of direction in y-axis

	bullet() // default constructor
	{

	};

	bullet(int16_t xc, int16_t yc, int  n) // constructor
	{
		this->xc = xc;
		this->yc = yc;
		if (n == 1) dy = -10; // speed for player bullets (set n to 1)
		else dy = 10; //speed for enemy bullets (set n to 0)
	};

	void updateLoc() { // update the y location of the bullet for next framebuffer display 
		yc = yc + dy;
	}

	bool isRemove() { // get remove flag
		return removeFlag;
	}

	void setRemove(bool x) { //set flag
		removeFlag = x;
	}

private:
	bool removeFlag = 0; // flag for deletion of bullet
};

class enemyShip {
public:
	//public variables of enemy Ship
	int16_t xc; //ship x-axis position
	int16_t yc; //ship y-axis position
	int16_t spawnY; //ship y-axis stopping point
	int indexLocs; //spawning index according to spawningLocs[] variable 
	bool state = 1; // 1 = go to left; 0 = go to right

	enemyShip(int16_t xc, int16_t yc, int16_t spawnY, int indexLocs) {
		this->xc = xc;
		this->yc = yc;
		this->spawnY = spawnY; // stopping point of animation in y axis
		this->indexLocs = indexLocs;
	};

	void moveLeft() { // for moving left animation
		xc = xc - speed;
	}
	void moveRight() { // for moving right animation
		xc = xc + speed;
	}
	void moveForward() { //for forward animation
		yc = yc + speed;
	}
	void decrementTimer() { // store the previous locations for patching
		bulletTimer--;
	}
	void resetTimer() { //reset enemy fire bullet timer
		bulletTimer = 50;
	}
	void decHealth() { // decrease enemy health
		health--;
	}
	bool isDed() { // return if enemy has no life remaining
		return health <= 0;
	}
	int16_t getBulletTimer() { // get bullet timer
		return bulletTimer;
	}
	void decExplosionTimer() { // get Explosion timer (used for explosion of enemy frames)
		explosionTimer--;
	}
	bool toDelete() { //if explosion timer is 0 then delete the spaceship object
		return explosionTimer <= 0;
	}


private:
	//Private variables of enemy ship class
	int16_t speed = 3;
	int16_t bulletTimer = 50;
	int16_t explosionTimer = 30;
	int16_t health = 5;
};

//keypress function
void keypress(struct mfb_window* window, mfb_key key, mfb_key_mod mod, bool isPressed);

//collision check between player bullet and enemy
bool isHit_player2enem(int16_t pBullet_x, int16_t pBullet_y, int16_t eShip_x, int16_t eShip_y);

//collision check between enemy bullet and player
bool isHit_enem2player(int16_t pShip_x, int16_t pShip_y, int16_t eBullet_x, int16_t eBullet_y);

// function for changing the display of enemy shit whenever got hit by a bullet or collide with enemy
void playerShipHitEffect(uint8_t* sprite_mem);

//collision check for player ship and enemy ship
bool isHitShip(int16_t pShip_x, int16_t pShip_y, int16_t eShip_x, int16_t eShip_y);

int main() {

	//set pointer for frambuffer
	framebuffer = (uint32_t*)malloc(WIDTH * HEIGHT * 4);

	//initialize vector for player bullet class
	vector<bullet> shipBullet_vec;

	//initialize vector for player bullet class
	vector<bullet> enemBullet_vec;

	//initialize vector for enemy ship class
	vector<enemyShip> enemyShip_vec;

	struct mfb_window* window;
	window = mfb_open("Spaceship", WIDTH, HEIGHT);

	//set pointer for sprites
	uint8_t* sprite_mem = (uint8_t*)malloc(WIDTH * HEIGHT * sizeof(uint8_t));

	//get data for backgroumd img
	CImg<unsigned char> bg("newbg.bmp");
	bg_mem = bg.data();

	CImg<unsigned char> bg_temp("newbg.bmp");
	bg_mem_temp = bg_temp.data();

	//get data for spaceshipe img
	CImg<unsigned char> sprite("sprite.bmp");

	//get data for bullet image
	CImg<unsigned char> sprite2("bullet.bmp");

	//get data for enemy1 sprite
	CImg<unsigned char> sprite3("enemy1.bmp");

	//get data for bullet enemy bullet
	CImg<unsigned char> sprite4("bullet2.bmp");

	//get data for player ship display for hit
	CImg<unsigned char> sprite5("shipHit.bmp");

	//get data for player ship display for hit
	CImg<unsigned char> sprite6("explosion.bmp");

	//get data for red heart
	CImg<unsigned char> sprite7("heart.bmp");

	//get data for black heart
	CImg<unsigned char> sprite8("heartBlack.bmp");
	uint8_t* heartBlack_mem = sprite8.data();

	//Logo Sprite
	CImg <unsigned char> logosprite("SPACE WARS.bmp");
	uint8_t* logo_mem = logosprite.data();

	//get data for Main menu
	CImg <unsigned char> subtextsprite("MainText.bmp");
	uint8_t* subtext_mem = subtextsprite.data();

	//Game Over Sprite
	CImg <unsigned char> GameOversprite("GameOver.bmp");
	uint8_t* gameover_mem = GameOversprite.data();

	//You Win Sprite
	CImg <unsigned char> YouWinSprite("YouWin.bmp");
	uint8_t* youwin_mem = YouWinSprite.data();

	//Game over subtext sprite 
	CImg <unsigned char> gosub("GOsubtext.bmp");
	uint8_t* gosub_mem = gosub.data();

	//Win Condition text
	CImg <unsigned char> wintext("winText.bmp");
	uint8_t* wintext_mem = wintext.data();

	//place the sprite spaceship and bullet into the contiguous memory
	memcpy(sprite_mem, sprite.data(), 50 * 50 * 3);
	memcpy(sprite_mem + 50 * 50 * 3, sprite2.data(), 20 * 20 * 3);
	memcpy(sprite_mem + 50 * 50 * 3 + 20 * 20 * 3, sprite3.data(), 60 * 60 * 3);
	memcpy(sprite_mem + 50 * 50 * 3 + 20 * 20 * 3 + 60 * 60 * 3, sprite4.data(), 20 * 20 * 3);
	memcpy(sprite_mem + 50 * 50 * 3 + 20 * 20 * 3 + 60 * 60 * 3 + 20 * 20 * 3, sprite5.data(), 50 * 50 * 3);
	memcpy(sprite_mem + 50 * 50 * 3 + 20 * 20 * 3 + 60 * 60 * 3 + 20 * 20 * 3 + 50 * 50 * 30, sprite6.data(), 60 * 60 * 3);
	memcpy(sprite_mem + 50 * 50 * 3 + 20 * 20 * 3 + 60 * 60 * 3 + 20 * 20 * 3 + 50 * 50 * 30 + 60 * 60 * 3, sprite7.data(), 30 * 30 * 3);

	//Game Environment Variables//
	srand(time(NULL)); // random generator; used for x-location spawning of enemies

	//Game Timers
	int16_t spawnTimer = 100;
	int16_t fireCounter = 5;
	int16_t playerHealth = 10;
	int16_t playerHitCounter = 5;
	int16_t shipsDestroyed = 0;
	bool	winCondition = false;
	bool	winText = true;
	int invCount = 10;


	//Size Containers
	int enemBullsize = 0;
	int enemySize = 0;
	int player_bulletSize = 0;

	//Spawn Locations
	int16_t spawnLocs[] = { 75, 140, 205, 270, 335, 400 };
	bool spawnBool[] = { 0, 0, 0, 0, 0 };

	//Game Flag
	bool isPlayerHit = false;

	//For Background scrolling
	int count = 0;

	if (!window) return -1;
	//callback function for keypress 
	mfb_set_keyboard_callback(window, keypress);

	do {
		//update enemy, player bullet , and enemy bullet size
		enemySize = enemyShip_vec.size();
		player_bulletSize = shipBullet_vec.size();
		enemBullsize = enemBullet_vec.size();

		//Back ground scrolling
		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				// Get the RGB data from the background
				bg_mem[WIDTH * y + x] = bg_mem_temp[WIDTH * abs((y - count) % 720) + x];
				bg_mem[WIDTH * y + x + WIDTH * HEIGHT] = bg_mem_temp[WIDTH * abs((y - count) % 720) + x + WIDTH * HEIGHT];
				bg_mem[WIDTH * y + x + 2 * WIDTH * HEIGHT] = bg_mem_temp[WIDTH * abs((y - count) % 720) + x + 2 * WIDTH * HEIGHT];

				// Get the RGB data from the background
				uint8_t r = bg_mem[WIDTH * y + x];
				uint8_t g = bg_mem[WIDTH * y + x + WIDTH * HEIGHT];
				uint8_t b = bg_mem[WIDTH * y + x + 2 * WIDTH * HEIGHT];

				// Combine the RGB data into a single int
				framebuffer[WIDTH * y + x] = (r << 16) + (g << 8) + b;
			}
		}
		count += 3;

		//Main menu state
		if (gamestate == 0) {
			// Reset Health
			playerHealth = 10;

			// Main Menu Sprite (WIDTH - 500, HEIGHT - 500)
			for (int y = 0; y < 500; y++) {
				for (int x = 0; x < 500; x++) {
					//rgb data from bg
					uint8_t r = bg_mem[WIDTH * (50 + y) + (390 + x)];
					uint8_t g = bg_mem[WIDTH * (50 + y) + (390 + x) + WIDTH * HEIGHT];
					uint8_t b = bg_mem[WIDTH * (50 + y) + (390 + x) + 2 * WIDTH * HEIGHT];

					//rgb data from sprite
					uint8_t rs = logo_mem[500 * y + x];
					uint8_t gs = logo_mem[500 * y + x + 500 * 500];
					uint8_t bs = logo_mem[500 * y + x + 2 * 500 * 500];

					//combine rgb to single int
					uint32_t pixel_bg = (r << 16) + (g << 8) + b;
					uint32_t pixel_sprite = (rs << 16) + (gs << 8) + bs;
					if (pixel_sprite == 0)
						framebuffer[WIDTH * (50 + y) + (390 + x)] = pixel_bg;
					else
						framebuffer[WIDTH * (50 + y) + (390 + x)] = pixel_sprite;

				}
			}
			// Subtext Sprite (WIDTH - 500, HEIGHT - 126)
			for (int y = 0; y < 126; y++) {
				for (int x = 0; x < 500; x++) {
					//rgb data from bg
					uint8_t r = bg_mem[WIDTH * (550 + y) + (390 + x)];
					uint8_t g = bg_mem[WIDTH * (550 + y) + (390 + x) + WIDTH * HEIGHT];
					uint8_t b = bg_mem[WIDTH * (550 + y) + (390 + x) + 2 * WIDTH * HEIGHT];

					//rgb data from sprite
					uint8_t rs = subtext_mem[500 * y + x];
					uint8_t gs = subtext_mem[500 * y + x + 500 * 126];
					uint8_t bs = subtext_mem[500 * y + x + 2 * 500 * 126];

					//combine rgb to single int
					uint32_t pixel_bg = (r << 16) + (g << 8) + b;
					uint32_t pixel_sprite = (rs << 16) + (gs << 8) + bs;
					if (pixel_sprite == 0)
						framebuffer[WIDTH * (550 + y) + (390 + x)] = pixel_bg;
					else
						framebuffer[WIDTH * (550 + y) + (390 + x)] = pixel_sprite;

				}
			}
		}
		//Playing the game State
		else if (gamestate == 1) {

			if (winText) {
				//win condition display
				for (int y = 0; y < 143; y++) { //WIDTH = 499, HEIGHT = 143
					for (int x = 0; x < 499; x++) {
						//rgb data from bg
						uint8_t r = bg_mem[WIDTH * (100 + y) + (391 + x)];
						uint8_t g = bg_mem[WIDTH * (100 + y) + (391 + x) + WIDTH * HEIGHT];
						uint8_t b = bg_mem[WIDTH * (100 + y) + (391 + x) + 2 * WIDTH * HEIGHT];

						//rgb data from sprite
						uint8_t rs = wintext_mem[499 * y + x];
						uint8_t gs = wintext_mem[499 * y + x + 499 * 143];
						uint8_t bs = wintext_mem[499 * y + x + 2 * 499 * 143];

						//combine rgb to single int
						uint32_t pixel_bg = (r << 16) + (g << 8) + b;
						uint32_t pixel_sprite = (rs << 16) + (gs << 8) + bs;
						if (pixel_sprite == 0)
							framebuffer[WIDTH * (100 + y) + (391 + x)] = pixel_bg;
						else
							framebuffer[WIDTH * (100 + y) + (391 + x)] = pixel_sprite;

					}
				}
			}

			//if the space keyboard is pressed execute then create player bullet
			if (isfired) {
				int size = shipBullet_vec.size();
				if (fireCounter < 0) { // 

					bullet newBullet = bullet(X_REF, Y_REF, 1); //create new class for bullet where the location is the same as spaceship
					shipBullet_vec.push_back(newBullet); // insert the bullet object to the vector
					fireCounter = 10;
				}
				isfired = false; //reset the bool isfired
			}

			//Firing of bullet by enemy ship
			for (int i = 0; i < enemySize; i++) {
				int16_t count = enemyShip_vec.at(i).getBulletTimer();
				int16_t enembullet_xc = enemyShip_vec.at(i).xc;
				int16_t enembullet_yc = enemyShip_vec.at(i).yc + 30;
				if (count < 0) {
					bullet newBullet = bullet(enembullet_xc, enembullet_yc, 0);
					enemBullet_vec.push_back(newBullet);
					enemyShip_vec.at(i).resetTimer();
				}
				enemyShip_vec.at(i).decrementTimer();
			}

			//Enemy Spawning
			int index = 0;
			if (spawnTimer < 0 && enemySize < 6) {
				for (int i = 0; i < 6; i++) {
					if (!spawnBool[i]) {
						if (winText) winText = false;
						spawnBool[i] = true;
						int16_t randX = rand() % (1180 - 100 + 1) + 100;
						enemyShip newShip = enemyShip(randX, 65, spawnLocs[i], i);
						enemyShip_vec.push_back(newShip);
						spawnTimer = 100;
						break;
					}
				}
			}

			//Display player bullet sprite
			for (int i = 0; i < player_bulletSize; i++) {
				int16_t bullet_xc = shipBullet_vec.at(i).xc; //get x location of the bullet
				int16_t bullet_yc = shipBullet_vec.at(i).yc; //get y location of the bullet

				if (bullet_yc - 10 <= 0) { //if the bullet goes beyond the boundary of the frames
					shipBullet_vec.at(i).setRemove(1);

				}
				else { // display bullet if its location is still in the boundary

					//bullet dimensions is 20px by 20px
					for (int y = 0; y < 20; y++) {
						for (int x = 0; x < 20; x++) {

							// Get the RGB data from the background
							uint8_t r = bg_mem[WIDTH * (bullet_yc + y - 10) + (bullet_xc + x - 10)];
							uint8_t g = bg_mem[WIDTH * (bullet_yc + y - 10) + (bullet_xc + x - 10) + WIDTH * HEIGHT];
							uint8_t b = bg_mem[WIDTH * (bullet_yc + y - 10) + (bullet_xc + x - 10) + 2 * WIDTH * HEIGHT];


							// Get the RGB data from the sprite
							uint8_t rs = sprite_mem[20 * y + x + (50 * 50 * 3)];
							uint8_t gs = sprite_mem[20 * y + x + 20 * 20 + (50 * 50 * 3)];
							uint8_t bs = sprite_mem[20 * y + x + 2 * 20 * 20 + (50 * 50 * 3)];

							// Combine the RGB data into a single int
							uint32_t pixel_bg = (r << 16) + (g << 8) + b;
							uint32_t pixel_sprite = (rs << 16) + (gs << 8) + bs;

							if (pixel_sprite == 0)
								framebuffer[WIDTH * (bullet_yc + y - 10) + (bullet_xc + x - 10)] = pixel_bg;
							else
								framebuffer[WIDTH * (bullet_yc + y - 10) + (bullet_xc + x - 10)] = pixel_sprite;
						}
					}
					shipBullet_vec.at(i).updateLoc(); //update bullet position
				}
			}

			//Display enemy bullet
			int enembullPoint = 0; //puts an offset pointer of the vector whenever the elements are resized
			for (int i = 0; i < enemBullsize; i++) {

				int16_t bullet_xc = enemBullet_vec.at(i).xc; //get x location of the bullet
				int16_t bullet_yc = enemBullet_vec.at(i).yc; //get y location of the bullet

				if (bullet_yc + 10 >= 680) { //if the bullet goes beyond the boundary of the frames
					enemBullet_vec.at(i).setRemove(1);
				}

				else {
					for (int y = 0; y < 20; y++) {
						for (int x = 0; x < 20; x++) {
							// Get the RGB data from the background
							uint8_t r = bg_mem[WIDTH * (bullet_yc + y - 10) + (bullet_xc + x - 10)];
							uint8_t g = bg_mem[WIDTH * (bullet_yc + y - 10) + (bullet_xc + x - 10) + WIDTH * HEIGHT];
							uint8_t b = bg_mem[WIDTH * (bullet_yc + y - 10) + (bullet_xc + x - 10) + 2 * WIDTH * HEIGHT];

							// Get the RGB data from the sprite
							uint8_t rs = sprite_mem[20 * y + x + (50 * 50 * 3) + (20 * 20 * 3) + (60 * 60 * 3)];
							uint8_t gs = sprite_mem[20 * y + x + 20 * 20 + (50 * 50 * 3) + (20 * 20 * 3) + (60 * 60 * 3)];
							uint8_t bs = sprite_mem[20 * y + x + 2 * 20 * 20 + (50 * 50 * 3) + (20 * 20 * 3) + (60 * 60 * 3)];

							// Combine the RGB data into a single int
							uint32_t pixel_bg = (r << 16) + (g << 8) + b;
							uint32_t pixel_sprite = (rs << 16) + (gs << 8) + bs;
							if (pixel_sprite < 0x808080)
								framebuffer[WIDTH * (bullet_yc + y - 10) + (bullet_xc + x - 10)] = pixel_bg;
							else
								framebuffer[WIDTH * (bullet_yc + y - 10) + (bullet_xc + x - 10)] = pixel_sprite;
						}
					}
					enemBullet_vec.at(i - enembullPoint).updateLoc(); //update bullet position
				}
			}

			// Display enemy ship
			for (int i = 0; i < enemySize; i++) {
				int16_t yref = enemyShip_vec.at(i).yc;
				int16_t xref = enemyShip_vec.at(i).xc;
				bool isDed = enemyShip_vec.at(i).isDed();
				//if ded then show explosion
				if (isDed) {

					//show explosion frame
					// Get the RGB data from the background
					for (int y = 0; y < 60; y++) {
						for (int x = 0; x < 60; x++) {

							// Get the RGB data from the background
							uint8_t r = bg_mem[WIDTH * (yref + y - 30) + (xref + x - 30)];
							uint8_t g = bg_mem[WIDTH * (yref + y - 30) + (xref + x - 30) + WIDTH * HEIGHT];
							uint8_t b = bg_mem[WIDTH * (yref + y - 30) + (xref + x - 30) + 2 * WIDTH * HEIGHT];

							// Get the RGB data from the sprite
							uint8_t rs = sprite_mem[60 * y + x + 50 * 50 * 3 + 20 * 20 * 3 + 60 * 60 * 3 + 20 * 20 * 3 + 50 * 50 * 30];
							uint8_t gs = sprite_mem[60 * y + x + 60 * 60 + 50 * 50 * 3 + 20 * 20 * 3 + 60 * 60 * 3 + 20 * 20 * 3 + 50 * 50 * 30];
							uint8_t bs = sprite_mem[60 * y + x + 2 * 60 * 60 + 50 * 50 * 3 + 20 * 20 * 3 + 60 * 60 * 3 + 20 * 20 * 3 + 50 * 50 * 30];

							// Combine the RGB data into a single int
							uint32_t pixel_bg = (r << 16) + (g << 8) + b;
							uint32_t pixel_sprite = (rs << 16) + (gs << 8) + bs;
							if (pixel_sprite < 0x808080)
								framebuffer[WIDTH * (yref + y - 30) + (xref + x - 30)] = pixel_bg;
							else
								framebuffer[WIDTH * (yref + y - 30) + (xref + x - 30)] = pixel_sprite;
						}
					}
					enemyShip_vec.at(i).decExplosionTimer();
				}
				//else if not dead show this
				else {
					for (int y = 0; y < 60; y++) {
						for (int x = 0; x < 60; x++) {

							// Get the RGB data from the background
							uint8_t r = bg_mem[WIDTH * (yref + y - 30) + (xref + x - 30)];
							uint8_t g = bg_mem[WIDTH * (yref + y - 30) + (xref + x - 30) + WIDTH * HEIGHT];
							uint8_t b = bg_mem[WIDTH * (yref + y - 30) + (xref + x - 30) + 2 * WIDTH * HEIGHT];

							// Get the RGB data from the sprite
							uint8_t rs = sprite_mem[60 * y + x + (50 * 50 * 3) + (20 * 20 * 3)];
							uint8_t gs = sprite_mem[60 * y + x + 60 * 60 + (50 * 50 * 3) + (20 * 20 * 3)];
							uint8_t bs = sprite_mem[60 * y + x + 2 * 60 * 60 + (50 * 50 * 3) + (20 * 20 * 3)];

							// Combine the RGB data into a single int
							uint32_t pixel_bg = (r << 16) + (g << 8) + b;
							uint32_t pixel_sprite = (rs << 16) + (gs << 8) + bs;
							if (pixel_sprite == 0)
								framebuffer[WIDTH * (yref + y - 30) + (xref + x - 30)] = pixel_bg;
							else
								framebuffer[WIDTH * (yref + y - 30) + (xref + x - 30)] = pixel_sprite;

						}
					}
				}
			}

			// Display the spaceship sprite 
			if (isPlayerHit && playerHitCounter > 0) {
				playerShipHitEffect(sprite_mem);
				playerHitCounter--;
			}
			else {
				for (int y = 0; y < 50; y++) {
					for (int x = 0; x < 50; x++) {

						// Get the RGB data from the background
						uint8_t r = bg_mem[WIDTH * (Y_REF + y - 25) + (X_REF + x - 25)];
						uint8_t g = bg_mem[WIDTH * (Y_REF + y - 25) + (X_REF + x - 25) + WIDTH * HEIGHT];
						uint8_t b = bg_mem[WIDTH * (Y_REF + y - 25) + (X_REF + x - 25) + 2 * WIDTH * HEIGHT];

						// Get the RGB data from the sprite
						uint8_t rs = sprite_mem[50 * y + x];
						uint8_t gs = sprite_mem[50 * y + x + 50 * 50];
						uint8_t bs = sprite_mem[50 * y + x + 2 * 50 * 50];

						// Combine the RGB data into a single int
						uint32_t pixel_bg = (r << 16) + (g << 8) + b;
						uint32_t pixel_sprite = (rs << 16) + (gs << 8) + bs;
						if (pixel_sprite == 0)
							framebuffer[WIDTH * (Y_REF + y - 25) + (X_REF + x - 25)] = pixel_bg;
						else
							framebuffer[WIDTH * (Y_REF + y - 25) + (X_REF + x - 25)] = pixel_sprite;
					}
				}
			}

			//Display Health Bar
			int16_t health_xc = 30;
			int16_t health_yc = 700;
			//Display for red hearts
			for (int i = 0; i < playerHealth; i++) {
				for (int y = 0; y < 30; y++) {
					for (int x = 0; x < 30; x++) {

						// Get the RGB data from the background
						uint8_t r = bg_mem[WIDTH * (health_yc + y - 15) + (health_xc + x - 15)];
						uint8_t g = bg_mem[WIDTH * (health_yc + y - 15) + (health_xc + x - 15) + WIDTH * HEIGHT];
						uint8_t b = bg_mem[WIDTH * (health_yc + y - 15) + (health_xc + x - 15) + 2 * WIDTH * HEIGHT];

						// Get the RGB data from the sprite
						uint8_t rs = sprite_mem[30 * y + x + 50 * 50 * 3 + 20 * 20 * 3 + 60 * 60 * 3 + 20 * 20 * 3 + 50 * 50 * 30 + 60 * 60 * 3];
						uint8_t gs = sprite_mem[30 * y + x + 30 * 30 + 50 * 50 * 3 + 20 * 20 * 3 + 60 * 60 * 3 + 20 * 20 * 3 + 50 * 50 * 30 + 60 * 60 * 3];
						uint8_t bs = sprite_mem[30 * y + x + 2 * 30 * 30 + 50 * 50 * 3 + 20 * 20 * 3 + 60 * 60 * 3 + 20 * 20 * 3 + 50 * 50 * 30 + 60 * 60 * 3];

						// Combine the RGB data into a single int
						uint32_t pixel_bg = (r << 16) + (g << 8) + b;
						uint32_t pixel_sprite = (rs << 16) + (gs << 8) + bs;
						if (pixel_sprite < 0x808080)
							framebuffer[WIDTH * (health_yc + y - 15) + (health_xc + x - 15)] = pixel_bg;
						else
							framebuffer[WIDTH * (health_yc + y - 15) + (health_xc + x - 15)] = pixel_sprite;
					}
				}
				health_xc = health_xc + 40;
			}
			//Display for black hearts
			for (int i = 0; i < 10 - playerHealth; i++) {
				for (int y = 0; y < 30; y++) {
					for (int x = 0; x < 30; x++) {

						// Get the RGB data from the background
						uint8_t r = bg_mem[WIDTH * (health_yc + y - 15) + (health_xc + x - 15)];
						uint8_t g = bg_mem[WIDTH * (health_yc + y - 15) + (health_xc + x - 15) + WIDTH * HEIGHT];
						uint8_t b = bg_mem[WIDTH * (health_yc + y - 15) + (health_xc + x - 15) + 2 * WIDTH * HEIGHT];

						// Get the RGB data from the sprite
						uint8_t rs = heartBlack_mem[30 * y + x];
						uint8_t gs = heartBlack_mem[30 * y + x + 30 * 30];
						uint8_t bs = heartBlack_mem[30 * y + x + 2 * 30 * 30];

						// Combine the RGB data into a single int
						uint32_t pixel_bg = (r << 16) + (g << 8) + b;
						uint32_t pixel_sprite = (rs << 16) + (gs << 8) + bs;
						if (pixel_sprite == 0)
							framebuffer[WIDTH * (health_yc + y - 15) + (health_xc + x - 15)] = pixel_bg;
						else
							framebuffer[WIDTH * (health_yc + y - 15) + (health_xc + x - 15)] = pixel_sprite;
					}
				}
				health_xc = health_xc + 40;
			}

			//enemy animation
			for (int i = 0; i < enemySize; i++) {
				int16_t enem_Y = enemyShip_vec.at(i).yc;
				int16_t enem_X = enemyShip_vec.at(i).xc;
				int16_t spawnY = enemyShip_vec.at(i).spawnY;
				bool isDed = enemyShip_vec.at(i).isDed();
				if (!isDed) {
					if (enem_Y < spawnY) {
						enemyShip_vec.at(i).moveForward();
					}
					else {
						if (enem_X < 60) enemyShip_vec.at(i).state = 0;
						else if (enem_X > 1220) enemyShip_vec.at(i).state = 1;
						if (enemyShip_vec.at(i).state == 1) {
							enemyShip_vec.at(i).moveLeft();
						}
						else {
							enemyShip_vec.at(i).moveRight();
						}
					}
				}
			}

			//collision check between player bullet and enemyShip
			for (int i = 0; i < enemySize; i++) {
				int16_t eShip_x = enemyShip_vec.at(i).xc;
				int16_t eShip_y = enemyShip_vec.at(i).yc;
				for (int k = 0; k < player_bulletSize; k++) {
					int16_t pBullet_x = shipBullet_vec.at(k).xc;
					int16_t pBullet_y = shipBullet_vec.at(k).yc;
					bool isHit = isHit_player2enem(pBullet_x, pBullet_y, eShip_x, eShip_y);
					if (isHit) {
						enemyShip_vec.at(i).decHealth(); // decrease the health of ship 
						shipBullet_vec.at(k).setRemove(1);
					}
				}
			}

			//collision check between enemy bullet and player Ship
			for (int i = 0; i < enemBullsize; i++) {
				int16_t eBullet_x = enemBullet_vec.at(i).xc;
				int16_t eBullet_y = enemBullet_vec.at(i).yc;
				bool isHit = isHit_enem2player(X_REF, Y_REF, eBullet_x, eBullet_y);
				if (isHit) {
					isPlayerHit = true;
					playerHitCounter = 5;
					playerHealth--;
					enemBullet_vec.at(i).setRemove(1);
				}
			}

			//collision check for player ship and enemy ship
			for (int i = 0; i < enemySize; i++) {
				int16_t eShip_x = enemyShip_vec.at(i).xc;
				int16_t eShip_y = enemyShip_vec.at(i).yc;
				bool isHit = isHitShip(X_REF, Y_REF, eShip_x, eShip_y);
				if (isHit && invCount < 0) {
					isPlayerHit = true;
					playerHitCounter = 5;
					invCount = 10;
					playerHealth--;
					enemyShip_vec.at(i).decHealth();
				}
			}
			// Player Death
			if (playerHealth == 0) {
				gamestate = 2;
			}
			//remove bullet that is in boundary or hit
			for (int i = 0; i < player_bulletSize; i++) {
				bool isRemove = shipBullet_vec.at(i).isRemove();
				if (isRemove) {
					shipBullet_vec.erase(shipBullet_vec.begin() + i);
					player_bulletSize--; // adjust the bulletSize directly to prevent out of range error in for loop
				}
			}
			for (int i = 0; i < enemBullsize; i++) {
				bool isRemove = enemBullet_vec.at(i).isRemove();
				if (isRemove) {
					enemBullet_vec.erase(enemBullet_vec.begin() + i);
					enemBullsize--; // adjust the bulletSize directly to prevent out of range error in for loop
				}
			}

			//remove ship with no heath
			for (int i = 0; i < enemySize; i++) {
				int index = enemyShip_vec.at(i).indexLocs;
				bool toDelete = enemyShip_vec.at(i).toDelete();
				if (toDelete) {
					shipsDestroyed++;
					enemyShip_vec.erase(enemyShip_vec.begin() + i);
					enemySize--; // adjust the bulletSize directly to prevent out of range error in for loop
					spawnBool[index] = false;
				}
			}
			if (shipsDestroyed == KILLS_NEEDED) {
				winCondition = true;
				gamestate = 2;
			}

			// game counter update
			if (playerHitCounter < 0) {
				playerHitCounter = 5;
				isPlayerHit = false;
			}
			spawnTimer--;
			fireCounter--;
			invCount--;
		}
		// Game Over
		else if (gamestate == 2) {
			//Game Over Text
			if (winCondition) {
				for (int y = 0; y < 500; y++) { //WIDTH - 500, HEIGHT - 500
					for (int x = 0; x < 500; x++) {
						//rgb data from bg
						uint8_t r = bg_mem[WIDTH * (50 + y) + (390 + x)];
						uint8_t g = bg_mem[WIDTH * (50 + y) + (390 + x) + WIDTH * HEIGHT];
						uint8_t b = bg_mem[WIDTH * (50 + y) + (390 + x) + 2 * WIDTH * HEIGHT];

						//rgb data from sprite
						uint8_t rs = youwin_mem[500 * y + x];
						uint8_t gs = youwin_mem[500 * y + x + 500 * 500];
						uint8_t bs = youwin_mem[500 * y + x + 2 * 500 * 500];

						//combine rgb to single int
						uint32_t pixel_bg = (r << 16) + (g << 8) + b;
						uint32_t pixel_sprite = (rs << 16) + (gs << 8) + bs;
						if (pixel_sprite == 0)
							framebuffer[WIDTH * (50 + y) + (390 + x)] = pixel_bg;
						else
							framebuffer[WIDTH * (50 + y) + (390 + x)] = pixel_sprite;

					}
				}
			}
			else {
				for (int y = 0; y < 500; y++) { //WIDTH - 500, HEIGHT - 500
					for (int x = 0; x < 500; x++) {
						//rgb data from bg
						uint8_t r = bg_mem[WIDTH * (50 + y) + (390 + x)];
						uint8_t g = bg_mem[WIDTH * (50 + y) + (390 + x) + WIDTH * HEIGHT];
						uint8_t b = bg_mem[WIDTH * (50 + y) + (390 + x) + 2 * WIDTH * HEIGHT];

						//rgb data from sprite
						uint8_t rs = gameover_mem[500 * y + x];
						uint8_t gs = gameover_mem[500 * y + x + 500 * 500];
						uint8_t bs = gameover_mem[500 * y + x + 2 * 500 * 500];

						//combine rgb to single int
						uint32_t pixel_bg = (r << 16) + (g << 8) + b;
						uint32_t pixel_sprite = (rs << 16) + (gs << 8) + bs;
						if (pixel_sprite == 0)
							framebuffer[WIDTH * (50 + y) + (390 + x)] = pixel_bg;
						else
							framebuffer[WIDTH * (50 + y) + (390 + x)] = pixel_sprite;

					}
				}
			}

			// Subtext Sprite (WIDTH - 500, HEIGHT - 101)
			for (int y = 0; y < 101; y++) {
				for (int x = 0; x < 500; x++) {
					//rgb data from bg
					uint8_t r = bg_mem[WIDTH * (550 + y) + (390 + x)];
					uint8_t g = bg_mem[WIDTH * (550 + y) + (390 + x) + WIDTH * HEIGHT];
					uint8_t b = bg_mem[WIDTH * (550 + y) + (390 + x) + 2 * WIDTH * HEIGHT];

					//rgb data from sprite
					uint8_t rs = gosub_mem[500 * y + x];
					uint8_t gs = gosub_mem[500 * y + x + 500 * 101];
					uint8_t bs = gosub_mem[500 * y + x + 2 * 500 * 101];

					//combine rgb to single int
					uint32_t pixel_bg = (r << 16) + (g << 8) + b;
					uint32_t pixel_sprite = (rs << 16) + (gs << 8) + bs;
					if (pixel_sprite == 0)
						framebuffer[WIDTH * (550 + y) + (390 + x)] = pixel_bg;
					else
						framebuffer[WIDTH * (550 + y) + (390 + x)] = pixel_sprite;

				}
			}
		}
		else if (gamestate == 3) {

			//clear all bullet and enemy ship vectors
			shipBullet_vec.clear();
			enemBullet_vec.clear();
			enemyShip_vec.clear();

			//reset all timers
			spawnTimer = 150;
			fireCounter = 10;
			playerHealth = 10;
			playerHitCounter = 5;
			shipsDestroyed = 0;
			winText = true;
			gamestate = 1;


			X_REF = WIDTH / 2;
			Y_REF = HEIGHT / 2;
			for (int i = 0; i < 5; i++) {
				spawnBool[i] = false;
			}

		}
		mfb_update(window, framebuffer);
	} while (mfb_wait_sync(window));
}

void keypress(struct mfb_window* window, mfb_key key, mfb_key_mod mod, bool isPressed) {

	if (key == KB_KEY_LEFT) {
		if ((X_REF - X_OFFSET) >= 0 + 25) {
			X_REF -= SHIP_SPEED;
			moved = true;
		}

	}

	else if (key == KB_KEY_RIGHT) {
		if ((X_REF + X_OFFSET) <= WIDTH - 25) {
			X_REF += SHIP_SPEED;
			moved = true;
		}
	}

	else if (key == KB_KEY_UP) {
		if ((Y_REF - Y_OFFSET) >= 0 + 25) {
			Y_REF -= SHIP_SPEED;
			moved = true;
		}
	}

	else if (key == KB_KEY_DOWN) {
		if ((Y_REF + Y_OFFSET) < 675) {
			Y_REF += SHIP_SPEED;
			moved = true;
		}
	}

	else if (key == KB_KEY_SPACE) {
		if (gamestate == 0) {
			gamestate = 1;
		}
		else if (gamestate == 1) {
			isfired = true;
		}
		else if (gamestate == 2) {
			gamestate = 3;
		}
	}

	else if (key == KB_KEY_ESCAPE) mfb_close(window);

}

bool isHit_player2enem(int16_t pBullet_x, int16_t pBullet_y, int16_t eShip_x, int16_t eShip_y) {
	if ((pBullet_y - 10 < eShip_y + 10 && pBullet_y - 10 > eShip_y - 20) &&
		(pBullet_x > eShip_x - 30) && (pBullet_x < eShip_x + 30)) {
		return true;
	}
	else return false;
}

bool isHit_enem2player(int16_t pShip_x, int16_t pShip_y, int16_t eBullet_x, int16_t eBullet_y) {
	//inner hit box
	if ((eBullet_y + 10 > pShip_y - 5 && eBullet_y + 10 < pShip_y + 25) &&
		(eBullet_x > pShip_x - 30 && eBullet_x < pShip_x + 30)) {
		return true;
	}
	else return false;
}

bool isHitShip(int16_t pShip_x, int16_t pShip_y, int16_t eShip_x, int16_t eShip_y) {
	if ((pShip_y - 25 < eShip_y + 20 && pShip_y - 25 > eShip_y - 30) &&
		(pShip_x > eShip_x - 30) && (pShip_x < eShip_x + 30)) {
		return true;
	}
	else return false;
}

void playerShipHitEffect(uint8_t* sprite_mem) {
	for (int y = 0; y < 50; y++) {
		for (int x = 0; x < 50; x++) {

			// Get the RGB data from the background
			uint8_t r = bg_mem[WIDTH * (Y_REF + y - 25) + (X_REF + x - 25)];
			uint8_t g = bg_mem[WIDTH * (Y_REF + y - 25) + (X_REF + x - 25) + WIDTH * HEIGHT];
			uint8_t b = bg_mem[WIDTH * (Y_REF + y - 25) + (X_REF + x - 25) + 2 * WIDTH * HEIGHT];

			// Get the RGB data from the sprite
			uint8_t rs = sprite_mem[50 * y + x + (50 * 50 * 3) + (20 * 20 * 3) + (60 * 60 * 3) + (20 * 20 * 3)];
			uint8_t gs = sprite_mem[50 * y + x + 50 * 50 + (50 * 50 * 3) + (20 * 20 * 3) + (60 * 60 * 3) + (20 * 20 * 3)];
			uint8_t bs = sprite_mem[50 * y + x + 2 * 50 * 50 + (50 * 50 * 3) + (20 * 20 * 3) + (60 * 60 * 3) + (20 * 20 * 3)];

			// Combine the RGB data into a single int
			uint32_t pixel_bg = (r << 16) + (g << 8) + b;
			uint32_t pixel_sprite = (rs << 16) + (gs << 8) + bs;
			if (pixel_sprite < 0x0e1111)
				framebuffer[WIDTH * (Y_REF + y - 25) + (X_REF + x - 25)] = pixel_bg;
			else
				framebuffer[WIDTH * (Y_REF + y - 25) + (X_REF + x - 25)] = pixel_sprite;
		}
	}
}