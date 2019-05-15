#include "Utility.h"
#include "../source/Irrlicht/COpenGLExtensionHandler.h"

using namespace irr;

BaseReceiver::BaseReceiver()
{}

bool BaseReceiver::OnEvent(const irr::SEvent &e)
{
	switch(e.KeyInput.Key)
	{
	case KEY_KEY_Q:
	case KEY_ESCAPE:
		exit(0);
		return true;
	}
	return false;
}

void writeScreenshot(irr::IrrlichtDevice* device, core::dimension2d<uint32_t> size)
{
	video::IImage* screenshot = device->getVideoDriver()->createImage(asset::EF_B8G8R8A8_UNORM, size);
	glReadPixels(0, 0, size.Width, size.Height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, screenshot->getData()); {
		// images are horizontally flipped, so we have to fix that here.
		uint8_t* pixels = (uint8_t*)screenshot->getData();

		const int32_t pitch = screenshot->getPitch();
		uint8_t* p2 = pixels + (size.Height - 1) * pitch;
		uint8_t * tmpBuffer = new uint8_t[pitch];
		for (uint32_t i = 0; i < size.Height; i += 2) {
			memcpy(tmpBuffer, pixels, pitch);
			memcpy(pixels, p2, pitch);
			memcpy(p2, tmpBuffer, pitch);
			pixels += pitch;
			p2 -= pitch;
		}
		delete[] tmpBuffer;
	}

	asset::CImageData* img = new asset::CImageData(screenshot);
	asset::IAssetWriter::SAssetWriteParams wparams(img);
	device->getAssetManager().writeAsset("screenshot.png", wparams);
	img->drop();
	screenshot->drop();
}