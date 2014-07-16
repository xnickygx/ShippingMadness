////////////////////////////////////////////////////////////////
//DirectInput class member function definitions
////////////////////////////////////////////////////////////////


#include "DirectInput.h"


DirectInput::DirectInput(DWORD keyboardCoopFlags,DWORD mouseCoopFlags, HINSTANCE hInst,	HWND hWind)
{
	//assign main handle to instance to our private member  for ease of use
	m_Instance		=hInst;
	m_hWind			=hWind;
	mDInput			= 0;
	mKeyboard		= 0;
	mMouse			= 0;
	

	//Zero out the memory for these keyboardstate and mouseState
	SecureZeroMemory(mKeyboardState,sizeof(mKeyboardState));
	SecureZeroMemory(&mMouseState,sizeof(mMouseState) );

	//create pointer to IDIRECTINPUT8 interface
	DirectInput8Create(m_Instance,DIRECTINPUT_VERSION,IID_IDirectInput8,(void**) &mDInput,0);

	//pointer to keybaord device
	mDInput->CreateDevice(GUID_SysKeyboard,&mKeyboard,0);
	//set device data formats
	mKeyboard->SetDataFormat(&c_dfDIKeyboard);
	//set cooperative levels for each input device
	mKeyboard->SetCooperativeLevel(m_hWind, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	//mKeyboard->Acquire();


	//pointer to mouse device
	mDInput->CreateDevice(GUID_SysMouse,&mMouse,0);
	//set device data formats
	mMouse->SetDataFormat(&c_dfDIMouse2);
	//set cooperative levels for each input device
	mMouse->SetCooperativeLevel(m_hWind,DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	//Acquire the mouse device
	//mMouse->Acquire();

}

DirectInput::~DirectInput()
{
	//Release everything
	mDInput->Release();
	mKeyboard->Unacquire();
	mMouse->Unacquire();
	mKeyboard->Release();
	mMouse->Release();
	
}

void DirectInput::poll()
{
	//lets acquire the mouse
	mKeyboard->Acquire();
	SecureZeroMemory(mKeyboardState,sizeof(mKeyboardState) );

	HRESULT hr=mKeyboard->GetDeviceState(sizeof(mKeyboardState),mKeyboardState) ;
	
	//if get device state failed, zero out memory and try to reacquire
	if(FAILED(hr) )
	{
		//kb lost, zero out memory and try to reacquire
		SecureZeroMemory(mKeyboardState,sizeof(mKeyboardState) );

		//try to reacquire
		hr=mKeyboard->Acquire();


	}

	//POLL MOUSE
	mMouse->Acquire();
	//hr stores success or fail of device state request
	hr=mMouse->GetDeviceState(sizeof(DIMOUSESTATE2),(void**)&mMouseState);

	if(FAILED(hr) )
	{
		SecureZeroMemory(&mMouseState,sizeof(mMouseState) );

		//try to reacquire mouse
		mMouse->Acquire();
	}

}

////////////////////////////////////////////////
//Returns true if a supplied key is pressed by checking the keyboard state
////////////////////////////////////////////////
//is a key down on keyboard
bool DirectInput::keyDown(unsigned char key)
{
	return (mKeyboardState[key] & 0x80) != 0;
}

//is a mousebutton down
bool DirectInput::mouseButtonDown(int button)
{
	return (mMouseState.rgbButtons[button] & 0x80) != 0;
}

//change in direction since last mouse movement
float DirectInput::mouseDX()
{
	return (float)mMouseState.lX;
}

//change in direction since last movment mouse y direction
float DirectInput::mouseDY()
{
	return (float)mMouseState.lY;
}

//movment in z on mouse (scroll wheel)
float DirectInput::mouseDZ()
{
	return (float)mMouseState.lZ;
}