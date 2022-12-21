#include <PT.h>
#include "core/Application.h"

int main() 
{
	using namespace PT;
	Application app;
	
	app.Init();
	app.Run();
	app.Shutdown();

	return 0;
}
