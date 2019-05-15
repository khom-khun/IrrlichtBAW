#ifndef _IRR_TUTORIAL_UTILITY_
#define _IRR_TUTORIAL_UTILITY_
#define _IRR_STATIC_LIB_
#include <irrlicht.h>

class BaseReceiver : public irr::IEventReceiver
{
public:
	BaseReceiver();
	bool OnEvent(const irr::SEvent &e);

	bool quit;

};

#endif