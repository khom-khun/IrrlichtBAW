#include "Utility.h"

BaseReceiver::BaseReceiver() : quit(false){

}

bool BaseReceiver::OnEvent(const irr::SEvent &e){
	switch(e.KeyInput.Key)
	{
	case irr::KEY_KEY_Q:
	case irr::KEY_ESCAPE:
		quit = true;
		break;
	}
}