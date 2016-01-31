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

#include "CameraController.h"
#include "ErrorCodeToMessage.h"

using namespace AVT::VmbAPI;

// TODO move this to JSON config.  You'll want it to be at least 2
#define NUM_FRAMES 3

CameraController::CameraController()
{ 
	if( NUM_FRAMES < 2 ) {
		throw CameraControllerException( __FUNCTION__, "NUM_FRAMES must be > 2.", VmbErrorOther );
	}
}

CameraController::~CameraController()
{
	if( mCamera ) {
		mCamera->Close();
	}
}

FramePtr CameraController::getCurrFrame()
{
	// lock isn't really needed
	//std::lock_guard<std::mutex> lock( mFrameMutex );
	return mCurrFrame;
}

void frameObservedCallback( AVT::VmbAPI::FramePtr &frame )
{
	// lock isn't really needed
	//std::lock_guard<std::mutex> lock( mFrameMutex );
	mFrame = frame;
}

void CameraController::startContinuousImageAcquisition( const config::ProgramConfig &config )
{
	if( mFrameObserver ) {
		throw CameraControllerException( __FUNCTION__, "Simultaneous calls to startContinuousImageAcquisition are illegal.", VmbErrorOther );
	}
	// Create a frame observer for this camera (This will be wrapped in a shared_ptr so we don't delete it)
    mFrameObserver = std::unique_ptr<FrameObserver>( new FrameObserver( mCamera, config.getFrameInfos(), config.getColorProcessing() ) );
    // Start streaming
    VmbErrorType res = mCamera->StartContinuousImageAcquisition( NUM_FRAMES, IFrameObserverPtr( mFrameObserver.get() ));

    if( res != VmbErrorSuccess ) {
		throw CameraControllerException( __FUNCTION__, ErrorCodeToMessage( res ), VmbErrorOther );    	
    }
}
    

void CameraController::stopContinuousImageAcquisition()
{
    // Stop streaming
    mCamera->StopContinuousImageAcquisition();
    mFrameObserver.reset();
}
