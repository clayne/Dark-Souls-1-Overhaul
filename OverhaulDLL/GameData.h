/*
	DARK SOULS OVERHAUL
	
	Contributors to this file:
		Sean Pesce	-	C++
	
*/

#pragma once

#ifndef _DS1_OVERHAUL_GAME_DATA_H_
	#define _DS1_OVERHAUL_GAME_DATA_H_


#include "SpPointer.h"



enum DS1_GAME_VERSION_ENUM {
	DS1_VERSION_RELEASE = 0x2E,
	DS1_VERSION_DEBUG = 0x2C,
	DS1_VERSION_OVERHAUL = 0x3C
};

enum DS1_PLAYER_STATUS_ENUM {
	DS1_PLAYER_STATUS_LOADING = -1,
	DS1_PLAYER_STATUS_HUMAN = 0,
	DS1_PLAYER_STATUS_COOP = 1,
	DS1_PLAYER_STATUS_INVADER = 2,
	DS1_PLAYER_STATUS_HOLLOW = 8
};




class GameData
{
public:

	// Base address of Dark Souls game process
	static void *ds1_base;

	// Base address for player character data
	static void *player_char_base;

	// Player character status (loading, human, co-op, invader, hollow)
	static SpPointer player_char_status;
	



	// Initializes pointers that depend on the game's base address
	static void init_pointers();

	// Changes the game version number to avoid compatibility issues with different game builds
	static void set_game_version(uint8_t version_number);
	
	// Fixes input bug that causes players to be stuck at a bonfire (usually when turning human)
	static int fix_bonfire_input();

	// Adds current multiplayer node count to the text feed info string and returns node count as an int.
	//		If argument is NULL, simply returns node count (or -1 if player is not online)
	static int get_node_count(std::string *text_feed_info_header = NULL);



	/////////////////////////////////////////
	//////////////// PATCHES ////////////////
	/////////////////////////////////////////

	// Disables automatic game disconnection when low framerate is detected
	static void disable_low_fps_disconnect();

	// Two-part patch to increase the multiplayer phantom limit:
	static void increase_phantom_limit1(); // Called from on_process_attach()
	static void increase_phantom_limit2(); // Called from initialize_plugin()
};




#endif // _DS1_OVERHAUL_GAME_DATA_H_