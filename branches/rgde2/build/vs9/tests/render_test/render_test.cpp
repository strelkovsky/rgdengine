#include "stdafx.h"

#include "application.h"


int __stdcall WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, int )
{
	rgde::application app(100, 100, 800, 600, L"render Test");

	app.run();
	
	return 0;
}



//GUI thoughts: