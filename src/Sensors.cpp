#include "Sensors.h"

Sensors::Sensors() {
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
}

Sensors::~Sensors() {
	//delete references on exit to prevent any memory leaks
	delete[] orig_shorts_diff;
	orig_shorts_diff = NULL;
	orig_shorts = NULL;
}

void Sensors::draw() {
	grayScale.draw(0, 0, ofGetWidth() / 2, ofGetHeight());
	CV_diff.draw(ofGetWidth() / 2, 0, ofGetWidth() / 2, ofGetHeight() / 2);
	grayImage.draw(ofGetWidth() / 2, ofGetHeight() / 2, ofGetWidth() / 2, ofGetHeight() / 2);

	ofPushStyle();
	ofSetLineWidth(3);
	ofSetHexColor(0xFF0000);//Red
	ofNoFill();

	//Draw all available blobs
	for (int i = 0; i < contours.blobs.size(); i++) {

		ofRect((ofGetWidth() / 2) + ofMap(centroid[i].boundingRect.x, 0, grayImage.width, 0, ofGetWidth() / 2),
			(ofGetHeight() / 2) + ofMap(centroid[i].boundingRect.y, 0, grayImage.height, 0, ofGetHeight() / 2),
			ofMap(centroid[i].boundingRect.width, 0, grayImage.width, 0, ofGetWidth() / 2),
			ofMap(centroid[i].boundingRect.height, 0, grayImage.height, 0, ofGetHeight() / 2));
	}
	ofPopStyle();
}

void Sensors::keyPressed(int key) {
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