//////////////////////////////////////////////////////////////////////////
// Name:	DirectXFramework.h
// Date:	April 2nd, 2010
// Author:	Kyle Lauing [klauing@devry.edu] or [kylelauing@gmail.com]
// Purpose: This file is used to create a very simple framework for using
//			DirectX 9 for the GSP 381 course for DeVry University.
// Disclaimer:	
//			Copyright © 2010 by DeVry Educational Development Corporation.
//			All rights reserved.  No part of this work may be reproduced 
//			or used in any form or by any means – graphic, electronic, or 
//			mechanical, including photocopying, recording, Web distribution 
//			or information storage and retrieval systems – without the 
//			prior consent of DeVry Educational Development Corporation.
//////////////////////////////////////////////////////////////////////////
#pragma once
#pragma comment(lib, "winmm.lib")
//////////////////////////////////////////////////////////////////////////
// Direct3D 9 headers and libraries required
//////////////////////////////////////////////////////////////////////////
#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#include <tchar.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <list>
#include "Timer.h" // for self made timer class

#include "DirectInput.h"

//FMOD
#pragma comment(lib,"Fmodex_vc.lib")
#include "fmod.hpp"
//#include "fmod_errors.h"

//DirectShow 
#include "dshow.h"
#pragma comment(lib,"strmiids.lib")


// Macro to release COM objects fast and safely
#define SAFE_RELEASE(x) if(x){x->Release(); x = 0;}
#define MAX_OBJECTS 6 //For max number of sprite objects


class CDirectXFramework
{
	//////////////////////////////////////////////////////////////////////////
	// Application Variables
	//////////////////////////////////////////////////////////////////////////
	HWND				m_hWnd;			// Handle to the window
	bool				m_bVsync;		// Boolean for vertical syncing
	float				m_currTime;		//Time to render the current frame
	float				m_prevTime;		//Time to render the previous frame
	int					m_FPS;			//Frames per second
			
	//////////////////////////////////////////////////////////////////////////
	// Direct3D Variables
	//////////////////////////////////////////////////////////////////////////
	IDirect3D9*			m_pD3DObject;	// Direct3D 9 Object
	IDirect3DDevice9*	m_pD3DDevice;	// Direct3D 9 Device
	D3DCAPS9			m_D3DCaps;		// Device Capabilities
	D3DPRESENT_PARAMETERS D3Dpp;		// Used for editing device options

	//////////////////////////////////////////////////////////////////////////
	//DirectInput Variables
	//////////////////////////////////////////////////////////////////////////
	DirectInput* g_DInput;
	
	//////////////////////////////////////////////////////////////////////////
	// Font Variables
	//////////////////////////////////////////////////////////////////////////
	ID3DXFont*			m_pD3DFont;		// Font Object
	ID3DXFont*			m_pD3DFontTitle;		// Font Object for titles

	//////////////////////////////////////////////////////////////////////////
	// Sprite Variables
	//////////////////////////////////////////////////////////////////////////
	ID3DXSprite*		m_pD3DSprite;	// Sprite Object


	//////////////////////////////////////////////////////////////////////////
	//Textures
	//////////////////////////////////////////////////////////////////////////
	IDirect3DTexture9*	m_pTexture;		// Texture Object for a sprite

	/////////////////////////////////////////////////////////////////////////
	//Image info structures for each texture
	/////////////////////////////////////////////////////////////////////////
	D3DXIMAGE_INFO		m_imageInfo;	// File details of a texture
	
	/////////////////////////////////////////////////////////////////////////
	//Fmod system variables
	/////////////////////////////////////////////////////////////////////////
	FMOD::System*	system;

	FMOD::Sound*	sound_Tada;
	FMOD::Sound*	sound_Chord;
	FMOD::Sound*	sound_Ding;
	FMOD::Sound*	sound_Jaguar;
	FMOD::Sound*	sound_Swish;
	FMOD::Sound*	sound_Wave;
	FMOD::Channel*	channel_Wave;
	FMOD::Sound*	sound_explode;
	FMOD::Sound*	sound_background;
	FMOD::Channel*	channel_background;

	//keyboard boolean array for UpdateFmod() member function
	bool m_bKeydown[256];


	/////////////////////////////////////////////////////////////////////////
	//DirectShow Variables
	/////////////////////////////////////////////////////////////////////////
	IGraphBuilder*		m_pGraphBuilder;
	IMediaControl*		m_pMediaControl;
	IMediaEvent*		m_pMediaEvent;
	IVideoWindow*		m_pVideoWindow;
	bool				isVidPlaying;

	//////////////////////////////////////////////////////////////////////////
	//Menu Variables
	//////////////////////////////////////////////////////////////////////////
	IDirect3DTexture9*				menuBackground;
	IDirect3DTexture9*				LevelOneBackground;  //Level 1 background 
	IDirect3DTexture9*				LevelOnePlayBoundary; //Level 1 play boundary
	IDirect3DTexture9*				PlayGameButton;
	IDirect3DTexture9*				CreditsButton;
	IDirect3DTexture9*				OptionsButton;
	IDirect3DTexture9*				QuitButton;

	IDirect3DTexture9*				HL_PlayGameButton; //highlighted buttons to show when selected
	IDirect3DTexture9*				HL_CreditsButton;
	IDirect3DTexture9*				HL_OptionsButton;
	IDirect3DTexture9*				HL_QuitButton;

	D3DXIMAGE_INFO					img_menuBackground;
	D3DXIMAGE_INFO					img_LevelOneBackground; // level one bkground image info
	D3DXIMAGE_INFO					img_LevelOnePlayBoundary; //level 1 play boundary (playing field outline)
	D3DXIMAGE_INFO					img_PlayGameButton;
	D3DXIMAGE_INFO					img_CreditsButton;
	D3DXIMAGE_INFO					img_OptionsButton;
	D3DXIMAGE_INFO					img_QuitButton;

	D3DXIMAGE_INFO					img_HL_PlayGameButton; //highlighted buttons to show when selected
	D3DXIMAGE_INFO					img_HL_CreditsButton;
	D3DXIMAGE_INFO					img_HL_OptionsButton;
	D3DXIMAGE_INFO					img_HL_QuitButton;

	//////////////////////////////////////////////////////////////////////////
	//Game Play Object Textures and ImageInfo structs
	//////////////////////////////////////////////////////////////////////////
	D3DXIMAGE_INFO					Player_Ship;
	D3DXIMAGE_INFO					Enemy_Ship;
	D3DXIMAGE_INFO					Bullet_Image;
	IDirect3DTexture9*				PlayerShip_Texture;
	IDirect3DTexture9*				EnemyShip_Texture;
	IDirect3DTexture9*				Bullet_Texture;

	//////////////////////////////////////////////////////////////////////////
	//Game Object Structures
	//////////////////////////////////////////////////////////////////////////
	
	struct EnemyPositions //stores all positions of each enemy used for drawing and collision tests
	{
		D3DXVECTOR3 pos;
	};

	//structure for bullets
	struct Bullet
	{
		Bullet(D3DXVECTOR3 po)
		{
			pos=po;
		}
		D3DXVECTOR3 pos; //bullet position
	};

	enum						{LEFT,RIGHT}; //Used for movment direction of enemy units
	static const int			MAX_ENEMIES = 6;
	EnemyPositions				Enemies[MAX_ENEMIES];
	std::list<EnemyPositions>	EnemyList; //list of enemies , arrays wouldn't allow removal from middle of array
	int							MovementDirection;
	bool						SideReached; //determines if enemies moved to side of screen and need to be turned around
	bool						GameEnded;// Did game end?

	D3DXVECTOR3 PlayerPosition; //position of player ship
	//list of bullets
	std::list<Bullet>			bullets;
	//can ship fire
	bool canFire;
	timer t; //timer class to track when bullets can and can't fire


	//////////////////////////////////////////////////////////////////////////
	//Game and Menu Finite State Machine Variables
	//////////////////////////////////////////////////////////////////////////
	int gameState;
	int menuState;

	enum {MENU,GAME,QUIT,CREDS,OPTS,END,ENDFAIL}; //GAME STATES
	enum {PLAY,CREDITS,OPTIONS,QUITGAME}; //MENU STATES
	
	float enemyPrevTimer; //We use this to move enemies every 2.0 seconds
	float enemyCurrTimer;

	//Timers to regulate amount of bullets fired every 1.0 seconds
	float bulletPrevTimer;
	float bulletCurrTimer;

	
public:
	//////////////////////////////////////////////////////////////////////////
	// Init and Shutdown are preferred to constructors and destructor,
	// due to having more control when to explicitly call them when global.
	//////////////////////////////////////////////////////////////////////////
	CDirectXFramework(void);
	~CDirectXFramework(void);

	//////////////////////////////////////////////////////////////////////////
	// Name:		Init
	// Parameters:	HWND hWnd - Handle to the window for the application
	//				HINSTANCE hInst - Handle to the application instance
	//				bool bWindowed - Boolean to control windowed or full-screen
	// Return:		void
	// Description:	Ran once at the start.  Initialize DirectX components and 
	//				variables to control the application.  
	//////////////////////////////////////////////////////////////////////////
	void Init(HWND& hWnd, HINSTANCE& hInst, bool bWindowed);

	//////////////////////////////////////////////////////////////////////////
	// Name:		Update
	// Parameters:	float elapsedTime - Time that has elapsed since the last
	//					update call.
	// Return:		void
	// Description: Runs every frame, use dt to limit functionality called to
	//				a certain amount of elapsed time that has passed.  Used 
	//				for updating variables and processing input commands prior
	//				to calling render.
	//////////////////////////////////////////////////////////////////////////
	void Update(float dt);

	//////////////////////////////////////////////////////////////////////////
	// Name:		Render
	// Parameters:	float elapsedTime - Time that has elapsed since the last
	//					render call.
	// Return:		void
	// Description: Runs every frame, use dt to limit functionality called to
	//				a certain amount of elapsed time that has passed.  Render
	//				calls all draw call to render objects to the screen.
	//////////////////////////////////////////////////////////////////////////
	void Render(float dt);

	void RenderMenu();
	

	//////////////////////////////////////////////////////////////////////////
	// Name:		Shutdown
	// Parameters:	void
	// Return:		void
	// Description:	Runs once at the end of an application.  Destroy COM 
	//				objects and deallocate dynamic memory.
	//////////////////////////////////////////////////////////////////////////
	void Shutdown();

	//////////////////////////////////////////////////////////////////////////
	//Name:			ProcessKeyboard
	//Parameters:	void
	//Return		void
	//Description:	Runs in the update method to test for key presses, simply a 
	//				method to encapsulate input device handling in our update method
	///////////////////////////////////////////////////////////////////////////
	void ProcessKeyboard(float dt);

	bool KeyPressed(char key);

	///////////////////////////////////////////////////////////////////////////
	//Name			InitFmod
	//parameters:	void
	//return		void
	//Description:  Initializes FMOD for playing sounds
	///////////////////////////////////////////////////////////////////////////
	void InitFmod();

	///////////////////////////////////////////////////////////////////////////
	//Name			UpdateFmod
	//parameters	void
	//return		void
	//Description	Updates fmod every frame and tests for fmod specific functions
	///////////////////////////////////////////////////////////////////////////
	void UpdateFmod();

	///////////////////////////////////////////////////////////////////////////
	//Name			InitDirectShow
	//parameters	void
	//return		void
	//Description	Initializes DirectShow components and plays first video
	//////////////////////////////////////////////////////////////////////////
	void InitDirectShow();

	///////////////////////////////////////////////////////////////////////////
	//Name			PlayMenuSound
	//parameters	void
	//return		void
	//Description	Plays a sound when menu scrolled up or down
	///////////////////////////////////////////////////////////////////////////
	void PlayMenuSound();

	

	
};


