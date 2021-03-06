#include "KinectSensors.h"
#define PORT 12345

KinectSensors::KinectSensors() {
	//Hardcoded values after sampling from the kinect
	kWidth = 512;// kinect->getDepth()->getPixelsRef().getWidth();
	kHeight = 424;// kinect->getDepth()->getPixelsRef().getHeight();

	//Source and destination points for warpIntoMe() openCV function
	src_cam_warp[0] = ofPoint(0, 0);
	src_cam_warp[1] = ofPoint(kWidth, 0);
	src_cam_warp[2] = ofPoint(kWidth, kHeight);
	src_cam_warp[3] = ofPoint(0, kHeight);

	dest_cam_warp[0] = ofPoint(0, 0);
	dest_cam_warp[1] = ofPoint(kWidth, 0);
	dest_cam_warp[2] = ofPoint(kWidth, kHeight);
	dest_cam_warp[3] = ofPoint(0, kHeight);

	//Allocating ofPixels and ofxCVGrayscaleImage instances
	normalPixels.allocate(kWidth, kHeight, OF_PIXELS_MONO);
	diffPixels.allocate(kWidth, kHeight, OF_PIXELS_MONO); //not used for now

	grayScale.allocate(kWidth, kHeight);
	grayImage.allocate(kWidth, kHeight);
	CV_calc.allocate(kWidth, kHeight);
	CV_diff.allocate(kWidth, kHeight);

	//Allocated onto the heap for copying
	orig_shorts_diff = new unsigned short[kWidth * kHeight];

	sender.setup("localhost", PORT);
}

KinectSensors::~KinectSensors() {
	//delete references on exit to prevent any memory leaks
	delete[] orig_shorts_diff;
	orig_shorts_diff = NULL;
	orig_shorts = NULL;
}

void KinectSensors::draw() {
	grayScale.draw(0, 0, ofGetWidth() / 2, ofGetHeight());
	CV_diff.draw(ofGetWidth() / 2, 0, ofGetWidth() / 2, ofGetHeight() / 2);
	grayImage.draw(ofGetWidth() / 2, ofGetHeight() / 2, ofGetWidth() / 2, ofGetHeight() / 2);
	drawBlobs(ofGetWidth() / 2, ofGetHeight() / 2, ofGetWidth() / 2, ofGetHeight() / 2);

	//grayImage.draw(0, 0, ofGetWidth(), ofGetHeight());
	//grayScale.draw(0, 0, ofGetWidth(), ofGetHeight());
	//drawBlobs(0, 0, ofGetWidth(), ofGetHeight());
}

void KinectSensors::keyPressed(int key) {
	switch (key) {
		
	//32 is the ascii decimal value for the space character
	case 32:	
		bLearnBackground = true; break;

	case '+':		
		thresh_high += 5;
		cout << "ThesholdHigh = " << thresh_high << "\n"; break;

	case '_':
		if (thresh_high - 5 >= thresh_low)
			thresh_high -= 5;
		cout << "ThesholdHigh = " << thresh_high << "\n"; break;

	case '=':
		if (thresh_low + 5 <= thresh_high)
			thresh_low += 5;
		cout << "ThesholdLow = " << thresh_low << "\n"; break;

	case '-':
		thresh_low -= 5;
		cout << "ThesholdLow = " << thresh_low << "\n"; break;

	case '>':
		blur_amt += 2;
		cout << "blur_amt = " << blur_amt << "\n"; break;

	case '<':
		if (blur_amt != 1)
			blur_amt -= 2;
		cout << "blue_amt = " << blur_amt << "\n"; break;

	case ']':
		if (blob_min_area + 20 <= blob_max_area)
			blob_min_area += 20;
		cout << "blob_IR_min_area = " << blob_min_area << "\n"; break;

	case '[':
		if (blob_min_area - 20 >= 0)
			blob_min_area -= 20;
		cout << "blob_min_area = " << blob_min_area << "\n"; break;

	case '}':
		blob_max_area += 20;
		cout << "blob_IR_max_area = " << blob_max_area << "\n"; break;

	case '{':
		if (blob_max_area - 20 >= blob_min_area)
			blob_max_area -= 20;
		cout << "blob_max_area = " << blob_max_area << "\n"; break;
	}

	//If a number from 0-9 is pressed
	if (key >= 48 && key <= 57) {
		blob_max_blobs = (int)key - 48;
		cout << "blob_max_blobs = " << blob_max_blobs << "\n";
	}
}

void KinectSensors::drawBlobs(float origin_x, float origin_y, float size_x, float size_y) {
	ofPushStyle();
	ofSetLineWidth(3);
	ofSetHexColor(0xFF0000);//Red
	ofNoFill();

	//Draw all available blobs
	for (int i = 0; i < contours.blobs.size(); i++) {
		ofxCvBlob centroid = contours.blobs[i];
		ofRect(origin_x + ofMap(centroid.boundingRect.x, 0, grayImage.width, 0, size_x),
			origin_y + ofMap(centroid.boundingRect.y, 0, grayImage.height, 0, size_y),
			ofMap(centroid.boundingRect.width, 0, grayImage.width, 0, size_x),
			ofMap(centroid.boundingRect.height, 0, grayImage.height, 0, size_y));
	}
	ofPopStyle();
}

bool KinectSensors::sampling(ofxKFW2::Device *kinect) {
	return (kinect->getInfrared()->getPixels() != NULL);
}

void KinectSensors::updatePixels(ofxKFW2::Device *kinect) {
	grayScale.setFromPixels(kinect->getInfrared()->getPixelsRef());
	orig_shorts = kinect->getInfrared()->getPixels(); //update depth array
}

void KinectSensors::firstReference(ofxKFW2::Device *kinect) {
	//Original calibration on start
	memcpy(orig_shorts_diff, kinect->getInfrared()->getPixels(), kWidth * kHeight * sizeof(unsigned short));
	first = false;
	cout << "first!\n";
}

void KinectSensors::recalibrate(ofxKFW2::Device *kinect) {
	memcpy(orig_shorts_diff, kinect->getInfrared()->getPixels(), kWidth * kHeight * sizeof(unsigned short));
	ofLog(OF_LOG_NOTICE, "Pixels Captured!");
	bLearnBackground = false;
}

void KinectSensors::retreiveAndBlur() {
	//Retreive difference Image
	CV_diff.setFromPixels(diffPixels);

	//Retreive threshold image
	CV_calc.setFromPixels(normalPixels);
	grayImage.warpIntoMe(CV_calc, src_cam_warp, dest_cam_warp);

	//Blur to reduce noise & find blobs of white pixels
	grayImage.blur(blur_amt);
}

void KinectSensors::findContours() {
	contours.findContours(grayImage, blob_min_area, blob_max_area, blob_max_blobs, true, false);
}

void KinectSensors::sendTouch() {
	
	if (contours.blobs.size() == 0) {
		////cout << "contours = 0\n";
		//ofxOscMessage message;

		//message.setAddress("/tuio/2Dcur");
		//message.addStringArg("delete");

		//sender.sendMessage(message);
	}
	else {
		for (int i = 0; i < contours.blobs.size(); i++) {
			//cout << "x: " << contours.blobs[i].centroid.x << "		y: " << contours.blobs[i].centroid.y << "\n";
			
			ofxOscMessage message;

			message.setAddress("/tuio/2Dcur");
			message.addStringArg("set");
			message.addIntArg(i+1);
			message.addFloatArg(contours.blobs[i].centroid.x / kWidth);
			message.addFloatArg(contours.blobs[i].centroid.y / kHeight);
			message.addFloatArg(0);
			message.addFloatArg(0);
			message.addFloatArg(0);

			sender.sendMessage(message);
		}
	}
}

//void KinectSensors::injectTouch(int x, int y) {
	/*
	POINTER_TOUCH_INFO contact;
	BOOL bRet = TRUE;

	//
	// assume a maximum of 10 contacts and turn touch feedback off
	//
	InitializeTouchInjection(10, TOUCH_FEEDBACK_NONE);

	//
	// initialize the touch info structure
	//
	memset(&contact, 0, sizeof(POINTER_TOUCH_INFO));

	contact.pointerInfo.pointerType = PT_TOUCH; //we're sending touch input
	contact.pointerInfo.pointerId = 0;          //contact 0
	contact.pointerInfo.ptPixelLocation.x = x * ofGetWidth();
	contact.pointerInfo.ptPixelLocation.y = y * ofGetHeight();
	contact.pointerInfo.pointerFlags = POINTER_FLAG_DOWN | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
	contact.touchFlags = TOUCH_FLAG_NONE;
	contact.touchMask = TOUCH_MASK_CONTACTAREA | TOUCH_MASK_ORIENTATION | TOUCH_MASK_PRESSURE;
	contact.orientation = 90;
	contact.pressure = 32000;

	//
	// set the contact area depending on thickness
	//
	contact.rcContact.top = 480 - 2;
	contact.rcContact.bottom = 480 + 2;
	contact.rcContact.left = 640 - 2;
	contact.rcContact.right = 640 + 2;

	//
	// inject a touch down
	//
	bRet = InjectTouchInput(1, &contact);

	//
	// if touch down was succesfull, send a touch up
	//
	if (bRet) {
		contact.pointerInfo.pointerFlags = POINTER_FLAG_UP;

		//
		// inject a touch up
		//
		//InjectTouchInput(1, &contact);
	}
	*/
//}