//////////////////////////////////////////////////////////////////////////
// Name:	DirectXFramework.cpp
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
#include "DirectXFramework.h"



CDirectXFramework::CDirectXFramework(void)
{
	// Init or NULL objects before use to avoid any undefined behavior
	m_bVsync		= false;
	m_pD3DObject	= 0;
	m_pD3DDevice	= 0;
	g_DInput		= 0;
	system			= 0; //initialize FMOD pointer to 0 first

	//Set Direct Show pointers to null and bool to false
	m_pGraphBuilder	= 0;
	m_pMediaControl	= 0;
	m_pMediaEvent	= 0;
	m_pVideoWindow	= 0;
	isVidPlaying	= false;
	

}

CDirectXFramework::~CDirectXFramework(void)
{
	// If Shutdown is not explicitly called correctly, call it when 
	// this class is destroyed or falls out of scope as an error check.
	system->release(); //release FMOD sound info
	CoUninitialize(); //Release COM library used by DirectShow
	Shutdown(); //release DirectX COM objects
}

void CDirectXFramework::Init(HWND& hWnd, HINSTANCE& hInst, bool bWindowed)
{
	//Initialize a DirectInput Object
	//Declare it static so it doesn't go out of scope when Init finishes, its a singleton class so this is okay
	static DirectInput di(DISCL_NONEXCLUSIVE | DISCL_FOREGROUND, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND,hInst,hWnd);
	g_DInput=&di; //g_DInput is our pointer to a DirectInput object in our framework

	InitFmod(); //Initialize FMOD


	//Initialize all private variables
	m_hWnd			= hWnd;
	m_bVsync		=false;
	m_pD3DObject	=0;
	m_pD3DDevice	=0;
	m_currTime		=0;
	m_prevTime		=0;
	//////////////////////////////////////////////////////////////////////////
	// Direct3D Foundations - D3D Object, Present Parameters, and D3D Device
	//////////////////////////////////////////////////////////////////////////

	// Create the D3D Object
	m_pD3DObject = Direct3DCreate9(D3D_SDK_VERSION);

	// Find the width and height of window using hWnd and GetWindowRect()
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	// Set D3D Device presentation parameters before creating the device
	//D3DPRESENT_PARAMETERS D3Dpp;
	ZeroMemory(&D3Dpp, sizeof(D3Dpp));  // NULL the structure's memory

	D3Dpp.hDeviceWindow					= hWnd;										// Handle to the focus window
	D3Dpp.Windowed						= bWindowed;								// Windowed or Full-screen boolean
	D3Dpp.AutoDepthStencilFormat		= D3DFMT_D24S8;								// Format of depth/stencil buffer, 24 bit depth, 8 bit stencil
	D3Dpp.EnableAutoDepthStencil		= TRUE;										// Enables Z-Buffer (Depth Buffer)
	D3Dpp.BackBufferCount				= 1;										// Change if need of > 1 is required at a later date
	D3Dpp.BackBufferFormat				= D3DFMT_X8R8G8B8;							// Back-buffer format, 8 bits for each pixel
	D3Dpp.BackBufferHeight				= height;									// Make sure resolution is supported, use adapter modes
	D3Dpp.BackBufferWidth				= width;									// (Same as above)
	D3Dpp.SwapEffect					= D3DSWAPEFFECT_DISCARD;					// Discard back-buffer, must stay discard to support multi-sample
	D3Dpp.PresentationInterval			= m_bVsync ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE; // Present back-buffer immediately, unless V-Sync is on								
	D3Dpp.Flags							= D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;		// This flag should improve performance, if not set to NULL.
	D3Dpp.FullScreen_RefreshRateInHz	= bWindowed ? 0 : D3DPRESENT_RATE_DEFAULT;	// Full-screen refresh rate, use adapter modes or default
	D3Dpp.MultiSampleQuality			= 0;										// MSAA currently off, check documentation for support.
	D3Dpp.MultiSampleType				= D3DMULTISAMPLE_NONE;						// MSAA currently off, check documentation for support.

	// Check device capabilities
	DWORD deviceBehaviorFlags = 0;
	m_pD3DObject->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &m_D3DCaps);

	// Determine vertex processing mode
	if(m_D3DCaps.DevCaps & D3DCREATE_HARDWARE_VERTEXPROCESSING)
	{
		// Hardware vertex processing supported? (Video Card)
		deviceBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;	
	}
	else
	{
		// If not, use software (CPU)
		deviceBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING; 
	}
	
	// If hardware vertex processing is on, check pure device support
	if(m_D3DCaps.DevCaps & D3DDEVCAPS_PUREDEVICE && deviceBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
	{
		deviceBehaviorFlags |= D3DCREATE_PUREDEVICE;	
	}
	
	// Create the D3D Device with the present parameters and device flags above
	m_pD3DObject->CreateDevice(
		D3DADAPTER_DEFAULT,		// which adapter to use, set to primary
		D3DDEVTYPE_HAL,			// device type to use, set to hardware rasterization
		hWnd,					// handle to the focus window
		deviceBehaviorFlags,	// behavior flags
		&D3Dpp,					// presentation parameters
		&m_pD3DDevice);			// returned device pointer

	//*************************************************************************
	

	//////////////////////////////////////////////////////////////////////////
	// Create a Font Object
	//////////////////////////////////////////////////////////////////////////
	
	// Load a font for private use for this process
	D3DXFONT_DESC fontDesc;
	fontDesc.Height			=30;
	fontDesc.Width			=10;
	fontDesc.Weight			=FW_BOLD;
	fontDesc.MipLevels		=0;
	fontDesc.Italic			=true;
	fontDesc.CharSet		=DEFAULT_CHARSET;
	fontDesc.OutputPrecision=OUT_DEFAULT_PRECIS;
	fontDesc.Quality		=DEFAULT_QUALITY;
	fontDesc.PitchAndFamily	=DEFAULT_PITCH | FF_DONTCARE;
	
	AddFontResourceEx(L"Delicious-Roman.otf",FR_PRIVATE,0); //add our custom font
	_tcscpy(fontDesc.FaceName,L"Delicious-Roman.otf");

	// Load D3DXFont, each font style you want to support will need an ID3DXFont
	D3DXCreateFontIndirect(m_pD3DDevice,&fontDesc,&m_pD3DFont); //Create create font and assign it to our font device pointer m_pD3DFont


	//////////////////////////////////////////////////////////////////////////
	// Create Sprite Object and Textures
	//////////////////////////////////////////////////////////////////////////

	// Create a sprite object, note you will only need one for all 2D sprites
	D3DXCreateSprite(m_pD3DDevice,&m_pD3DSprite);

	//Create Background Image
	D3DXCreateTextureFromFileEx
		(m_pD3DDevice,L"shipping_madness_background.png",0,0,0,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT,D3DCOLOR_XRGB(0,255,0),&img_menuBackground,0,&menuBackground);

	//Level 1 Boundary
	D3DXCreateTextureFromFileEx
		(m_pD3DDevice,L"PlayingBounds.png",0,0,0,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT,D3DCOLOR_XRGB(0,255,0),&img_LevelOnePlayBoundary,0,&LevelOnePlayBoundary);

	//Player Ship Texture
	D3DXCreateTextureFromFileEx
		(m_pD3DDevice,L"PlayerShip.png",0,0,0,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT,D3DCOLOR_XRGB(0,255,0),&Player_Ship,0,&PlayerShip_Texture);
	
	//Enemy Ship Texture
	D3DXCreateTextureFromFileEx
		(m_pD3DDevice,L"EnemyShip.png",0,0,0,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT,D3DCOLOR_XRGB(0,255,0),&Enemy_Ship,0,&EnemyShip_Texture);

	//Bullet Texture
	D3DXCreateTextureFromFileEx
		(m_pD3DDevice,L"Bullet.png",0,0,0,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT,D3DCOLOR_XRGB(0,255,0),&Bullet_Image,0,&Bullet_Texture);
	
	///////////////////////////////////////////////////////////////////
	//Menu Button Textures
	///////////////////////////////////////////////////////////////////
	D3DXCreateTextureFromFileEx
		(m_pD3DDevice,L"playgame.png",0,0,0,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT,D3DCOLOR_XRGB(0,255,0),&img_PlayGameButton,0,&PlayGameButton);

	D3DXCreateTextureFromFileEx
		(m_pD3DDevice,L"hl_playgame.png",0,0,0,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT,D3DCOLOR_XRGB(0,255,0),&img_HL_PlayGameButton,0,&HL_PlayGameButton);

	D3DXCreateTextureFromFileEx
		(m_pD3DDevice,L"credits.png",0,0,0,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT,D3DCOLOR_XRGB(0,255,0),&img_CreditsButton,0,&CreditsButton);
	D3DXCreateTextureFromFileEx
		(m_pD3DDevice,L"hl_credits.png",0,0,0,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT,D3DCOLOR_XRGB(0,255,0),&img_HL_CreditsButton,0,&HL_CreditsButton);
	D3DXCreateTextureFromFileEx
		(m_pD3DDevice,L"options.png",0,0,0,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT,D3DCOLOR_XRGB(0,255,0),&img_OptionsButton,0,&OptionsButton);

	D3DXCreateTextureFromFileEx
		(m_pD3DDevice,L"hl_options.png",0,0,0,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT,D3DCOLOR_XRGB(0,255,0),&img_HL_OptionsButton,0,&HL_OptionsButton);
	D3DXCreateTextureFromFileEx
		(m_pD3DDevice,L"quit.png",0,0,0,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT,D3DCOLOR_XRGB(0,255,0),&img_QuitButton,0,&QuitButton);
	D3DXCreateTextureFromFileEx
		(m_pD3DDevice,L"hl_quit.png",0,0,0,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT,D3DCOLOR_XRGB(0,255,0),&img_HL_QuitButton,0,&HL_QuitButton);
	//////////////////////////////////////////////////////////////////////////
	//Set All Enemy and Player positions, setup EnemyList
	//////////////////////////////////////////////////////////////////////////
	RECT padrect;
	GetClientRect(m_hWnd,&padrect);
	
	Enemies[0].pos=D3DXVECTOR3(padrect.left + 100.0f,padrect.top + 150.0f,0.0f);
	D3DXVECTOR3 prevPos=Enemies[0].pos;
	for(int i=1; i < MAX_ENEMIES; i++)
	{
		Enemies[i].pos=D3DXVECTOR3(prevPos.x + (Enemy_Ship.Width + 50.0f),padrect.top + 150.0f,0.0f);
		prevPos=Enemies[i].pos;
	}
	MovementDirection=RIGHT; //Set initial movement of all enemies to right of screen

	//Starting Player Position
	PlayerPosition=D3DXVECTOR3(float(padrect.right / 2 ), float(padrect.bottom - 100.0f),0.0f);

	//Construct list from initialized array of enemies
	std::list<EnemyPositions> ENEMIES (Enemies,Enemies + sizeof(Enemies) / sizeof(EnemyPositions) ); //create list of enemies called ENEMIES

	EnemyList=ENEMIES; //assign ENEMIES list to our main list EnemyList
	//bullets.push_back(Bullet(D3DXVECTOR3(PlayerPosition.x,PlayerPosition.y - 25.0f,0.0f) ) );

	//Set Game and Menu States
	gameState=MENU;
	menuState=PLAY;
	//Timers to move enemies in the game
	enemyCurrTimer=0.0f;
	enemyPrevTimer=0.0f;
	bulletCurrTimer=0.0f;
	bulletPrevTimer=0.0f;

	SideReached=false; //enemies haven't moved to edge of screen yet
	GameEnded  =false; //game has not ended yet
	
	//Now that everything is initialized call directShow to play video
	//InitDirectShow();

	//Start playing background music
	system->playSound(FMOD_CHANNEL_FREE,sound_Wave,true,&channel_Wave); //played but paused
	channel_Wave->setMode(FMOD_LOOP_NORMAL);
	channel_Wave->setVolume(0.5f);
	channel_Wave->setPaused(false);

	system->playSound(FMOD_CHANNEL_FREE,sound_background,true,&channel_background); //played but paused
	channel_background->setMode(FMOD_LOOP_NORMAL);
	channel_background->setVolume(0.8f);
	channel_background->setPaused(true);

}

void CDirectXFramework::Update(float dt)
{
	

	//Operates movement and collision of enemy objects in main game area. 
	if(gameState == GAME)
	{
		/*
		if(EnemyList.size() < 1)
		{
			//all enemies gone 
			gameState=END;
			return;
		}
		std::list<EnemyPositions>::iterator it; //iterator for EnemyList
		std::list<Bullet>::iterator bulletIter; //iterator for bulletList
		std::list<EnemyPositions>::iterator iter; //for testing end element
		enemyCurrTimer=timeGetTime();
		if( (enemyCurrTimer - enemyPrevTimer) >= 800.0f)
		{
			enemyPrevTimer=enemyCurrTimer;
			RECT rect;
			GetClientRect(m_hWnd,&rect);

			//CHECK TO SEE IF ENEMIES MOVED TO EDGE OF SCREEN
			for (it=EnemyList.begin(); it != EnemyList.end(); ++it)
			{
				if(MovementDirection == RIGHT && ((it->pos.x ) > rect.right - 50.0f) )
				{
					MovementDirection= LEFT;
					SideReached=true;
				}
				else if(MovementDirection == LEFT && ( (it->pos.x) < rect.left + 50.0f) )
				{
					MovementDirection= RIGHT;
					SideReached=true;
				}
			}
			//IF SIDE IS REACHED MOVE DOWN AND SWITCH DIRECTIONS
			if(SideReached)//enemies on edge of screen bounds
			{
				for(it=EnemyList.begin(); it != EnemyList.end(); ++it)
				{
					it->pos.y= it->pos.y + Enemy_Ship.Height;
				}
				SideReached=false; //we moved them down one
			}

			//MOVE SWARM IN DIRECTION OF MOVEMENT
			if(MovementDirection==RIGHT)
			{
				for(it=EnemyList.begin(); it != EnemyList.end(); ++it)
				{
					it->pos.x= it->pos.x + Enemy_Ship.Width;
				}
			}
			else if(MovementDirection==LEFT)
			{
				for(it=EnemyList.begin(); it != EnemyList.end(); ++it)
				{
					it->pos.x= it->pos.x - Enemy_Ship.Width;
				}
			}
		
		}//end if time passed > 2.0 seconds

		//Test bullet logic
		
		if(bullets.size() < 1) //if bullets less than 0 nothing to test
			{
				//do nothing
			}
		else //test collisions
		{
			//for(it=EnemyList.begin(); it != EnemyList.end();

			it=EnemyList.begin();
			int enemySize=EnemyList.size();
			//while( it != EnemyList.end() )
			for(int i=0; i < enemySize; i++)
			{
				for(bulletIter=bullets.begin(); bulletIter != bullets.end();)
				{
					bulletIter->pos.y= bulletIter->pos.y - (120 * dt); //move each bullet up fast
					//euclidean distance
					float dist= sqrt( (it->pos.x - bulletIter->pos.x) * (it->pos.x - bulletIter->pos.x) + (it->pos.y - bulletIter->pos.y) * (it->pos.y - bulletIter->pos.y) );
					//if distance is shorter than half length of both texture images, collision has occurred destroy both
					if (dist < (Bullet_Image.Height / 2) + (Enemy_Ship.Height /2) )
					{		
						//collision happened destroy bullet and enemy and play sound
						enemySize--;
						HRESULT result=system->playSound(FMOD_CHANNEL_FREE,sound_explode,false, 0);
						bulletIter=bullets.erase(bulletIter);
						it=EnemyList.erase(it);			
					}
					else
					{
						++bulletIter;
					}
				}//end inner for loop
				if(it != EnemyList.end() )
					++it;
			}//end primary for loop for bullet tests

		}//end if else for bullet collision logic

		//Test to see if invaders made it.
		RECT rect;
		GetClientRect(m_hWnd,&rect);
		for(it=EnemyList.begin(); it != EnemyList.end(); ++it)
				{
					float dist= sqrt( (it->pos.x - PlayerPosition.x) * (it->pos.x - PlayerPosition.x) + (it->pos.y - PlayerPosition.y) * (it->pos.y - PlayerPosition.y) );
					if (dist < (Player_Ship.Height / 2) + (Enemy_Ship.Height /2) )
					{
						gameState=ENDFAIL;
					}
					else if( it->pos.y > rect.bottom - 50.0f )
					{
						gameState=ENDFAIL;
					}
				}
    */
	}//end gameState decision logic
	
	ProcessKeyboard(dt); //process keyboard input
	UpdateFmod();
}

void CDirectXFramework::Render(float dt)
{
	// If the device was not created successfully, return
	if(!m_pD3DDevice)
		return;
	//*************************************************************************


	//////////////////////////////////////////////////////////////////////////
	// All draw calls between swap chain's functions, and pre-render and post- 
	// render functions (Clear and Present, BeginScene and EndScene)
	//////////////////////////////////////////////////////////////////////////

	// Clear the back buffer, call BeginScene()
	m_pD3DDevice->Clear(0,0,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,D3DXCOLOR(0.0f,0.4f,0.9f,1.0f),1.0f,0);

	m_pD3DDevice->BeginScene(); //start scene
			//////////////////////////////////////////////////////////////////////////
			// Draw 3D Objects (for future labs - not used in Week #1)
			//////////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////////////
			// Draw 2D sprites
			//////////////////////////////////////////////////////////////////////////
			
			// Call Sprite's Begin to start rendering 2D sprite objects
	m_pD3DSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_DEPTH_FRONTTOBACK);
				//////////////////////////////////////////////////////////////////////////
				// Matrix Transformations to control sprite position, scale, and rotate
				// Set these matrices for each object you want to render to the screen
				//////////////////////////////////////////////////////////////////////////
				
				RECT mRect;
				GetClientRect(m_hWnd,&mRect);
				float m_width= ( mRect.right-mRect.left) / 2;
				float m_height=(mRect.bottom - mRect.top) /2;
				float width= ( mRect.right-mRect.left) ;
				float height=(mRect.bottom - mRect.top);
				if(gameState == MENU) //if state is menu state
				{
					
					if(menuState == PLAY) //if play
						{
							m_pD3DSprite->Draw(menuBackground,0,&D3DXVECTOR3(0.0f,0.0f,0.0f),&D3DXVECTOR3(0,0.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(HL_PlayGameButton,0,&D3DXVECTOR3(img_HL_PlayGameButton.Width * 0.5f,img_HL_PlayGameButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,150.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(CreditsButton,0,&D3DXVECTOR3(img_CreditsButton.Width * 0.5f,img_CreditsButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,225.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(OptionsButton,0,&D3DXVECTOR3(img_OptionsButton.Width * 0.5f,img_OptionsButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,300.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(QuitButton,0,&D3DXVECTOR3(img_QuitButton.Width * 0.5f,img_QuitButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,375.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
						}
					if(menuState== CREDITS)
						{
							m_pD3DSprite->Draw(menuBackground,0,&D3DXVECTOR3(0.0f,0.0f,0.0f),&D3DXVECTOR3(0,0.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(PlayGameButton,0,&D3DXVECTOR3(img_PlayGameButton.Width * 0.5f,img_PlayGameButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,150.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(HL_CreditsButton,0,&D3DXVECTOR3(img_HL_CreditsButton.Width * 0.5f,img_HL_CreditsButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,225.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(OptionsButton,0,&D3DXVECTOR3(img_OptionsButton.Width * 0.5f,img_OptionsButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,300.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(QuitButton,0,&D3DXVECTOR3(img_QuitButton.Width * 0.5f,img_QuitButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,375.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
						}
					if(menuState == OPTIONS)
						{
							m_pD3DSprite->Draw(menuBackground,0,&D3DXVECTOR3(0.0f,0.0f,0.0f),&D3DXVECTOR3(0,0.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(PlayGameButton,0,&D3DXVECTOR3(img_PlayGameButton.Width * 0.5f,img_PlayGameButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,150.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(CreditsButton,0,&D3DXVECTOR3(img_CreditsButton.Width * 0.5f,img_CreditsButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,225.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(HL_OptionsButton,0,&D3DXVECTOR3(img_HL_OptionsButton.Width * 0.5f,img_HL_OptionsButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,300.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(QuitButton,0,&D3DXVECTOR3(img_QuitButton.Width * 0.5f,img_QuitButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,375.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
						}
					if(menuState == QUITGAME)
						{
							m_pD3DSprite->Draw(menuBackground,0,&D3DXVECTOR3(0.0f,0.0f,0.0f),&D3DXVECTOR3(0,0.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(PlayGameButton,0,&D3DXVECTOR3(img_PlayGameButton.Width * 0.5f,img_PlayGameButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,150.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(CreditsButton,0,&D3DXVECTOR3(img_CreditsButton.Width * 0.5f,img_CreditsButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,225.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(OptionsButton,0,&D3DXVECTOR3(img_OptionsButton.Width * 0.5f,img_OptionsButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,300.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
							m_pD3DSprite->Draw(HL_QuitButton,0,&D3DXVECTOR3(img_HL_QuitButton.Width * 0.5f,img_HL_QuitButton.Height * 0.5f,0.0f),&D3DXVECTOR3((float)m_width - 20.0f,375.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
						}	
				}//end menu draw
				else if(gameState == GAME) //if current game state is game draw game screen
				{
					m_pD3DSprite->Draw(menuBackground,0,&D3DXVECTOR3(0.0f,0.0f,0.0f),&D3DXVECTOR3(0,0.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));
					m_pD3DSprite->Draw(LevelOnePlayBoundary,0,&D3DXVECTOR3(0.0f,0.0f,0.0f),&D3DXVECTOR3(150.0f,50.0f,0.0f),D3DCOLOR_ARGB(255,255,255,255));


					/*
					std::list<EnemyPositions>::iterator it; //iterator for EnemyList
					std::list<Bullet>::iterator bulletIter; //iterator for bullets list
					//Draw All Enemies in EnemyList
					for(it=EnemyList.begin(); it != EnemyList.end(); ++it)
					{
						m_pD3DSprite->Draw(EnemyShip_Texture,0,&D3DXVECTOR3(Enemy_Ship.Width * 0.5f,Enemy_Ship.Height * 0.5f,0.0f),&it->pos,D3DCOLOR_ARGB(255,255,255,255));
					}
					m_pD3DSprite->Draw(PlayerShip_Texture,0,&D3DXVECTOR3(Player_Ship.Width * 0.5f,Player_Ship.Height * 0.5f,0.0f),&PlayerPosition,D3DCOLOR_ARGB(255,255,255,255));
					//Draw all player bullets if they exist
					for(bulletIter=bullets.begin(); bulletIter != bullets.end(); ++bulletIter)
					{
						m_pD3DSprite->Draw(Bullet_Texture,0,&D3DXVECTOR3(Bullet_Image.Width * 0.5f,Bullet_Image.Height * 0.5f,0.0f),&bulletIter->pos,D3DCOLOR_ARGB(255,255,255,255));
					}

					*/
					
				}
				

				m_pD3DSprite->End();

			//////////////////////////////////////////////////////////////////////////
			// Draw Text
			//////////////////////////////////////////////////////////////////////////

			// Calculate RECT structure for text drawing placement, using whole screen
			RECT rect;
			//GetClientRect(m_hWnd,&formatRect);
			GetClientRect(m_hWnd, &rect);
			
			// Draw Text, using DT_TOP, DT_RIGHT for placement in the top right of the
			// screen.  DT_NOCLIP can improve speed of text rendering, but allows text
			// to be drawn outside of the rect specified to draw text in.
			m_pD3DFont->DrawTextW(NULL,L"GSP 362 - Course Project!",-1,&rect,DT_TOP | DT_RIGHT | DT_WORDBREAK,D3DCOLOR_ARGB(255,255,0,0) ); //draw text in upper right hand corner


			//Draw FPS Counter
			wchar_t buffer[64];
			wchar_t bufferBig[512];
			swprintf(buffer,64,L"FPS: %d",m_FPS);//m_FPS
			m_pD3DFont->DrawTextW(0,buffer,-1,&rect,DT_TOP | DT_NOCLIP ,D3DCOLOR_ARGB(255,255,0,0));

			swprintf(buffer,64,L"Press F1 for Menu/Pause");
			m_pD3DFont->DrawTextW(0,buffer,-1,&rect,DT_BOTTOM | DT_LEFT | DT_NOCLIP ,D3DCOLOR_ARGB(255,255,0,0));

			if(gameState == MENU)
			{
				RECT rectangle;
				GetClientRect(m_hWnd,&rectangle);
				rectangle.left=(float)m_width -80.0f;
				rectangle.right=(float)m_width - 20.0f + 220.0f;
				rectangle.top=50.0f;
				rectangle.bottom=200.0f;
				m_pD3DFont->DrawTextW(NULL,L"Shipping Madness!!!",-1,&rectangle,  DT_CALCRECT,D3DCOLOR_ARGB(255,255,0,0) ); //draw text in upper right hand corner
				m_pD3DFont->DrawTextW(NULL,L"Shipping Madness!!!",-1,&rectangle, DT_CENTER,D3DCOLOR_ARGB(255,255,0,0) );
			}

			if(gameState == CREDS) //if current game state is credits
				{
					swprintf(bufferBig,512,L"\n\nCredits!\n\n\n Design, Development, Programming, and Testing done by...\n Team A\n Robert Evans\nNicholas Grande\nJustin Atkinson\nTylor Emmett\nJeromy Jones\nLeseth Mitchell");
					m_pD3DFont->DrawTextW(0,bufferBig,-1,&rect,DT_CENTER | DT_NOCLIP ,D3DCOLOR_ARGB(255,25,255,255));
				}
			if(gameState == OPTS)
			{
				swprintf(buffer,64,L"\n\nOPTIONS\n\n\n\n FULL SCREEN - Press Y for FullScreen or N for Windowed Mode\n");
				m_pD3DFont->DrawTextW(0,buffer,-1,&rect,DT_CENTER | DT_NOCLIP ,D3DCOLOR_ARGB(255,25,255,255));
			}
			if(gameState == END)
			{
				swprintf(buffer,64,L"\n\nCONGRATULATIONS!\n\n\n\n YOU HAVE DEFEATED THE ATTACKERS\n");
				m_pD3DFont->DrawTextW(0,buffer,-1,&rect,DT_CENTER | DT_NOCLIP ,D3DCOLOR_ARGB(255,25,255,255));
			}
			if(gameState == ENDFAIL)
			{
				swprintf(buffer,64,L"\n\nYOU LOST!!!!!\n\n\n\n YOU WERE DOMINATED!\n");
				m_pD3DFont->DrawTextW(0,buffer,-1,&rect,DT_CENTER | DT_NOCLIP ,D3DCOLOR_ARGB(255,25,255,255));
			}
			
			// EndScene, and Present the back buffer to the display buffer
			m_pD3DDevice->EndScene();
			m_pD3DDevice->Present(0,0,0,0);

	//Calculate Frames Per Second
	m_currTime = timeGetTime();
	static int fpsCounter = 0;
	if( (m_currTime - m_prevTime) >= 1000.0f)
	{
		m_prevTime=m_currTime;
		m_FPS=fpsCounter;
		fpsCounter=0;
	}
	else
	{
		fpsCounter++;
	}
	//*************************************************************************
	
}

void CDirectXFramework::Shutdown()
{
	//*************************************************************************
	// Release COM objects in the opposite order they were created in
	SAFE_RELEASE(m_pTexture);//texture com for test.tga
	// Sprite
	m_pD3DSprite->Release();//for sprite com for rendering various sprites
	// Font
	m_pD3DFont->Release();
	// 3DDevice	
	m_pD3DDevice->Release();
	// 3DObject
	m_pD3DObject->Release();
	//*************************************************************************
}



void CDirectXFramework::ProcessKeyboard(float dt)
{
	//Rect used by code to limit leaving viewing area
	RECT rect;
	GetClientRect(m_hWnd,&rect);
	//used a speed  because I thought of the roughly 5000 fps I was getting
	float speed = 100.0f * dt; 
	float mouseSpeed=2.0f ;
	//poll mouse and keyboard states
	g_DInput->poll();

	////////////////////////////////////////////////////////////////////
	//GAME STATE MENU
	////////////////////////////////////////////////////////////////////

	//MENU STATE INPUT PROCESSING
	if(gameState == MENU) //if current state is menu
	{
		//turn off main game background music and play menu music
		channel_background->setPaused(true);
		channel_Wave->setPaused(false);

		//handles menu selection
		if(g_DInput->keyDown(DIK_UP) && menuState == PLAY) //if up key pressed and current selected is Play
		{
			menuState=QUITGAME;
			PlayMenuSound();
			Sleep(150);
		}
		else if(g_DInput->keyDown(DIK_DOWN) && menuState == QUITGAME) //if up key pressed and current selected is QUIT
		{
			menuState=PLAY;
			PlayMenuSound();
			Sleep(150);
		}
		else if(g_DInput->keyDown(DIK_DOWN) && (menuState < QUITGAME) ) //otherwise just decrement
		{
			menuState+=1;
			PlayMenuSound();
			Sleep(150);
		}
		else if(g_DInput->keyDown(DIK_UP) && menuState > PLAY)
		{
			menuState-= 1;
			PlayMenuSound();
			Sleep(150);
		}

		//HANDLE SELECTION OF MENU ITEMS
		if(g_DInput->keyDown(DIK_RETURN) )// enter was pressed in the menu
		{
			if(menuState == PLAY )//if play
			{
				gameState=GAME; //game state to play game
			}
			else if(menuState == CREDITS ) //if credits selected
			{
				gameState=CREDS; //CREDS game state display credits
			}
			else if(menuState== OPTIONS) //if options selected
			{
				gameState=OPTS; //game state OPTS options
			}
			else if(menuState == QUITGAME) //if quit entered exit game
			{
				PostQuitMessage(0);
				
			}
		}
	} //END MENU STATE PROCESSING
	//START GAME STATE INPUT PROCESSING
	else if(gameState== GAME)
	{
		channel_Wave->setPaused(true);
		channel_background->setPaused(false);

		RECT rect;
		GetClientRect(m_hWnd,&rect);
		//if left arrow is pressed

		/*
		if(g_DInput->keyDown(DIK_LEFT) )
		{
			PlayerPosition.x= PlayerPosition.x - 100.0f * dt;
			if(PlayerPosition.x < rect.left + 10.0f)
			{
				PlayerPosition.x=rect.left + 10.0f;
			}
		}

		//if Right arrow is pressed
		if(g_DInput->keyDown(DIK_RIGHT) )
		{
			PlayerPosition.x= PlayerPosition.x + 100.0f * dt;
			if(PlayerPosition.x > rect.right - 10.0f)
			{
				PlayerPosition.x=rect.right - 10.0f;
			}
		}

		//if Space key is pressed
		if(g_DInput->keyDown(DIK_SPACE) )//g_DInput->keyDown(DIK_SPACE
		{
			//if(canFire == true)
			//{
				bullets.push_back(Bullet(D3DXVECTOR3(PlayerPosition.x,PlayerPosition.y - 50.0f,0.0f) ) );
				Sleep(200);
			//}
			
		}
		*/
		//IF f1 pressed set menu
		if(g_DInput->keyDown(DIK_F1) )
		{
			gameState=MENU;
			menuState=PLAY;
			Sleep(50);
		}
	}//END GAME STATE KEYBOARD PROCESSING

	if(g_DInput->keyDown(DIK_F1) )
		{
			gameState=MENU;
			menuState=PLAY;
			Sleep(50);
		}

}


void CDirectXFramework::InitFmod()
{
	FMOD_RESULT result;

	result=FMOD::System_Create(&system); //system is a CDirectXFramework data member

	if(result != FMOD_OK)
	{
		printf("FMOD Error! (%d) %s\n",result);
		exit(-1);
	}
	
	result=system->init(100,FMOD_INIT_NORMAL,0); //initialize FMOD

	if(result != FMOD_OK)
	{
		printf("FMOD Error! (%d) %s\n",result);
		exit(-1);
	}

	//Load sounds
	system->createSound("tada.wav",FMOD_DEFAULT,0,&sound_Tada);
	system->createSound("chord.wav",FMOD_DEFAULT,0,&sound_Chord);
	system->createSound("ding.wav",FMOD_DEFAULT,0,&sound_Ding);
	system->createSound("jaguar.wav",FMOD_DEFAULT,0,&sound_Jaguar);
	system->createSound("swish.wav",FMOD_DEFAULT,0,&sound_Swish);
	system->createStream("wave.mp3",FMOD_DEFAULT,0,&sound_Wave);
	system->createSound("Explosion1.wav",FMOD_DEFAULT,0,&sound_explode);
	system->createStream("DXclub.mp3",FMOD_DEFAULT,0,&sound_background);

	//initialize bool keyboard press tracker array for UpdateFmod function to false
	for(int i=0; i < 256 ; i++)
	{
		m_bKeydown[i]=false;
	}

}

void CDirectXFramework::UpdateFmod()
{
	
	FMOD_RESULT result; //for error checking
	g_DInput->poll(); //get kb data


	//////////////////////////////////////////////////////////
	//Space Key
	/////////////////////////////////////////////////////////
	if(g_DInput->keyDown(DIK_SPACE) ) //if space key pressed
	{
		if(! m_bKeydown[DIK_SPACE] ) //if the space key wasn't held down previously
		{
			m_bKeydown[DIK_SPACE]=true; //mark key as pressed in bool array

			result=system->playSound(FMOD_CHANNEL_FREE,sound_Tada,false, 0); //last param for channel if wanted for manipulation
			//FMOD::ERRCHECK(result);

		}
	}
	else //if space key isn't down
	{
		if(m_bKeydown[DIK_SPACE] ) //if space key was previously down, then a key up event just happened
		{
			m_bKeydown[DIK_SPACE]=false;
		}
	}
	////////////////////////////////////////////////////////////////
	//1 Key Press
	////////////////////////////////////////////////////////////////
	if(g_DInput->keyDown(DIK_1) ) //if space key pressed
	{
		if(! m_bKeydown[DIK_1] ) //if the space key wasn't held down previously
		{
			m_bKeydown[DIK_1]=true; //mark key as pressed in bool array

			result=system->playSound(FMOD_CHANNEL_FREE,sound_Chord,false, 0); //last param for channel if wanted for manipulation
			//FMOD::ERRCHECK(result);

		}
	}
	else //if space key isn't down
	{
		if(m_bKeydown[DIK_1] ) //if space key was previously down, then a key up event just happened
		{
			m_bKeydown[DIK_1]=false;
		}
	}
	////////////////////////////////////////////////////////////////
	//2 Key Press
	////////////////////////////////////////////////////////////////
	if(g_DInput->keyDown(DIK_2) ) //if space key pressed
	{
		if(! m_bKeydown[DIK_2] ) //if the space key wasn't held down previously
		{
			m_bKeydown[DIK_2]=true; //mark key as pressed in bool array

			result=system->playSound(FMOD_CHANNEL_FREE,sound_Ding,false, 0); //last param for channel if wanted for manipulation
			//FMOD::ERRCHECK(result);

		}
	}
	else //if space key isn't down
	{
		if(m_bKeydown[DIK_2] ) //if space key was previously down, then a key up event just happened
		{
			m_bKeydown[DIK_2]=false;
		}
	}
	////////////////////////////////////////////////////////////////
	//3 Key Press
	////////////////////////////////////////////////////////////////
	if(g_DInput->keyDown(DIK_3) ) //if space key pressed
	{
		if(! m_bKeydown[DIK_3] ) //if the space key wasn't held down previously
		{
			m_bKeydown[DIK_3]=true; //mark key as pressed in bool array

			result=system->playSound(FMOD_CHANNEL_FREE,sound_Jaguar,false, 0); //last param for channel if wanted for manipulation
			//FMOD::ERRCHECK(result);

		}
	}
	else //if space key isn't down
	{
		if(m_bKeydown[DIK_3] ) //if space key was previously down, then a key up event just happened
		{
			m_bKeydown[DIK_3]=false;
		}
	}
	////////////////////////////////////////////////////////////////
	//4 Key Press
	////////////////////////////////////////////////////////////////
	if(g_DInput->keyDown(DIK_4) ) //if space key pressed
	{
		if(! m_bKeydown[DIK_4] ) //if the space key wasn't held down previously
		{
			m_bKeydown[DIK_4]=true; //mark key as pressed in bool array

			result=system->playSound(FMOD_CHANNEL_FREE,sound_Swish,false, 0); //last param for channel if wanted for manipulation
			//FMOD::ERRCHECK(result);

		}
	}
	else //if space key isn't down
	{
		if(m_bKeydown[DIK_4] ) //if space key was previously down, then a key up event just happened
		{
			m_bKeydown[DIK_4]=false;
		}
	}

	//////////////////////////////////////////
	system->update(); //FMOD object update

}

void CDirectXFramework::InitDirectShow()
{

	CoInitialize(NULL); //init COM library

	CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&m_pGraphBuilder); //init graph builder filter

	m_pGraphBuilder->QueryInterface(IID_IMediaControl,(void**)&m_pMediaControl); //init media control filter

	m_pGraphBuilder->QueryInterface(IID_IMediaEvent,(void**)&m_pMediaEvent); //init media event filter

	m_pGraphBuilder->QueryInterface(IID_IVideoWindow,(void**)&m_pVideoWindow); //init video window filter

	
	//Render the video
	m_pGraphBuilder->RenderFile(L"intro.wmv",NULL);

	//setup window
	m_pVideoWindow->put_Owner( (OAHWND)m_hWnd );

	//set style
	m_pVideoWindow->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE);

	RECT WinRect;

	GetClientRect(m_hWnd,&WinRect);

	m_pVideoWindow->SetWindowPosition(WinRect.left,WinRect.top,WinRect.right,WinRect.bottom );

	m_pMediaControl->Run(); //play video

	isVidPlaying=true; //set our bool value
	//test when user hits enter or video finishes
	while(isVidPlaying)
	{
		long evCode;
		LONG_PTR eventParam1,eventParam2;

		m_pMediaEvent->GetEvent(&evCode,&eventParam1,&eventParam2,0);

		g_DInput->poll(); //get input state

		if(g_DInput->keyDown(DIK_RETURN) ) //If enter was pressed
		{
			isVidPlaying=false;
			m_pMediaControl->Stop();

			m_pVideoWindow->put_Visible(OAFALSE);
			m_pVideoWindow->put_Owner((OAHWND)m_hWnd );
			m_pMediaControl->Release();
			m_pMediaEvent->Release();
			m_pGraphBuilder->Release();
		}
		else if(evCode == EC_COMPLETE)
		{
			isVidPlaying=false;
			m_pMediaControl->Stop();

			m_pVideoWindow->put_Visible(OAFALSE);
			m_pVideoWindow->put_Owner((OAHWND)m_hWnd );
			m_pMediaControl->Release();
			m_pMediaEvent->Release();
			m_pGraphBuilder->Release();
		}

	

	}
	Sleep(250);


}

bool CDirectXFramework::KeyPressed(char key)
{
	g_DInput->poll();

	if(g_DInput->keyDown(key) )
	{
		return true;
	}
	else
	{
		return false;
	}

	
}


void CDirectXFramework::PlayMenuSound()
{
	system->playSound(FMOD_CHANNEL_FREE,sound_Ding,false, 0);

}

