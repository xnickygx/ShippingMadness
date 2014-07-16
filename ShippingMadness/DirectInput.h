///////////////////////////////////////////////////////////////
//Direct Input Class, to encapsulate handling with DirectInput
///////////////////////////////////////////////////////////////


#define DIRECTINPUT_VERSION 0x0800
#include "dinput.h"
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

//extern DirectInput* g_DInput; //declared in WinMain.cpp

class DirectInput
{
public:

	DirectInput(DWORD keyboardCoopFlags,DWORD mouseCoopFlags,HINSTANCE hInst,HWND hWind);
	~DirectInput();

	bool	keyDown(unsigned char key);
	bool	mouseButtonDown(int button);
	float	mouseDX();
	float	mouseDY();
	float	mouseDZ();
	void	poll();

private:
	//prevent copying this is a singleton class design
	DirectInput(const DirectInput& direct); //copy constructor
	DirectInput& operator= (const DirectInput& direct); //assignment operator


private:
	//private data members
	IDirectInput8*			mDInput; //pointer to DI interface
	IDirectInputDevice8*	mKeyboard;//pointer to keyboard device
	unsigned char			mKeyboardState[256]; //stores state of the entire keyboard at any one moment in time
	IDirectInputDevice8*	mMouse; // IDirectInputDevice8* for each input device
	DIMOUSESTATE2			mMouseState;//state of mouse device at any moment
	


	//application vars
	HINSTANCE	m_Instance;
	HWND		m_hWind;

};