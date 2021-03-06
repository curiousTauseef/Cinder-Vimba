/*
 Copyright (c) 2016, Lucas Vickers

 This code is intended to be used with the Cinder C++ library, http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"

#include "civimba/CiVimba.h"

using namespace ci;
using namespace ci::app;

// We'll create a new Cinder Application by deriving from the App class.
class AsyncGrab : public App {
public:

	void mouseDrag( MouseEvent event ) override;

	void keyDown( KeyEvent event ) override;

	void draw() override;

	void update() override;

	void setup() override;

	void cleanup() override;

private:
	civimba::ApiController mController;

	std::vector<civimba::CameraControllerRef> mCameras;
	std::vector<gl::Texture2dRef> mTextures;
	params::InterfaceGlRef mParams;
	float mFps;

};

void prepareSettings( AsyncGrab::Settings *settings )
{
#if defined( CINDER_ANDROID )
	settings->setMultiTouchEnabled( false );
#endif
}

void AsyncGrab::mouseDrag( MouseEvent event )
{
	// Store the current mouse position in the list.
}

void AsyncGrab::keyDown( KeyEvent event )
{
	if( event.getChar() == 'a' || event.getChar() == 'A' ) {
		// stArt
		for( auto &cam : mCameras ) {
			cam->startContinuousImageAcquisition();
		}
		console() << "Started acquisition." << std::endl;
	} else if( event.getChar() == 'o' || event.getChar() == 'O' ) {
		// stOp
		for( auto &cam : mCameras ) {
			cam->stopContinuousImageAcquisition();
		}
		console() << "Stopped acquisition." << std::endl;
	}
}

void AsyncGrab::setup()
{
	mController.startup();
	auto cameras = mController.getCameraList();
	for( auto &cam : cameras ) {
		std::string id;
		cam->GetID( id );
		mCameras.push_back( mController.getCamera( id ));
		mCameras.back()->setFrameLogging( civimba::FRAME_INFO_ERRORS );
	}
	mTextures.resize( mCameras.size());
	for( auto &cam : mCameras ) {
		cam->startContinuousImageAcquisition();
	}

	mParams = params::InterfaceGl::create( "App Params", ivec2( 100, 100 ));
	mParams->addParam<float>( "FPS", &mFps );
}

void AsyncGrab::cleanup()
{
	for( auto &cam : mCameras ) {
		cam->stopContinuousImageAcquisition();
	}
	mController.shutdown();
}

void AsyncGrab::draw()
{
	// Clear the contents of the window. This call will clear
	// both the color and depth buffers.
	gl::clear();
	int width = app::getWindowWidth() / mTextures.size();

	int offset = 0;
	for( auto &tex : mTextures ) {
		if( tex ) {
			float ratio = (( float ) tex->getWidth()) / tex->getHeight();
			int height = width / ratio;
			gl::draw( tex, Area( offset, 0, offset + width, height ));
		}
		offset += width;
	}

	mParams->draw();
}

void AsyncGrab::update()
{
	int index = 0;
	for( auto &cam : mCameras ) {
		if( cam->checkNewFrame()) {
			mTextures[index] = gl::Texture2d::create( *cam->getCurrentFrame());
		}
		++ index;
	}

	mFps = getAverageFps();
}

// This line tells Cinder to actually create and run the application.
CINDER_APP( AsyncGrab, RendererGl, prepareSettings )
