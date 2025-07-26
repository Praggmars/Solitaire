#include "application.hpp"
#include <exception>

int Main()
{
	try
	{
		return Application(L"Solitaire", 1000, 700).Run();
	}
	catch (std::exception& ex)
	{
		MessageBoxA(nullptr, ex.what(), "Error", MB_OK | MB_ICONERROR);
	}
	return 0;
}

int main()
{
	return Main();
}

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return Main();
}