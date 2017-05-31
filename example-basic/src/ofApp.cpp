#include "ofApp.h"
#include "ofxKuBox2d.h"

ofxKuBox2dWorld circWorld_;

int _w = 1024;		//screen size
int _h = 768;

float fps = 60;

int maskw=400;		//mask size
int maskh=300;
vector<unsigned char> mask;

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetWindowTitle("ofxKuBox2d example");

	ofSetFrameRate(fps);

	mask.resize(maskw*maskh);

	//physics parameters. See its definition to find more parameters for adjusting
	ofxKuBox2dWorldParams prm;
	prm.scr_w = _w;
	prm.scr_h = _h;
	prm.fps = fps;
	prm.pixel_size_in_millimeters = 100;	//speed of objects
	prm.gravity = -5;
	circWorld_.setup(prm);

	//TEST
	//circWorld_.addTriangle( ofPoint( 0, h ), ofPoint( 100, h ), ofPoint( 0, h-100 ) );

	//bottom line
	circWorld_.addStaticEdge( ofPoint( 0, _h - 1 ), ofPoint( _w, _h - 1 ), 0 );

	//circles
	int n = 70; 

	float sizeMin = 10;
	float sizeMax = 25;
	for (int i=0; i<n; i++) {
		CircleData c;
		c.rad		= ofRandom( sizeMin, sizeMax ); 
		c.p			= ofPoint( ofRandom( c.rad, _w - c.rad ), ofRandom( c.rad, _h / 2 ) );
		c.angleDeg		= 0.0;
		c.groupIndex	= i; 
		c.groupIndex0 = c.groupIndex;
		c.density		= 10.0;	//density, affects massk
		c.friction		= 0.2; 
		c.restitution	= 0.4; 

		circWorld_.addCircle( c );
	}

	//pull up all circles
	//for (int i=0; i<n; i++) {
	//	circleRestartPos( i, true );
	//}
}

//--------------------------------------------------------------
void ofApp::update(){
	//update mask
	updateMask();

	//apply mask to the world
	circWorld_.applyForces(mask, maskw, maskh);
	
	//restore falling circles
	physicsControl();						
	
	//update physics
	circWorld_.update( true );
}

//--------------------------------------------------------------
void ofApp::updateMask() {
	for (int i=0; i<maskw * maskh; i++) {
		mask[i] = 0;
	}

	float t = ofGetElapsedTimef();
	float freq = 0.3;
	int x0 = ofMap(sin(t*TWO_PI*freq),-1,1,0.25,0.75)*maskw;
	int rx = maskw*0.15;

	//horizontal
	int y0 = maskh*0.5;
	int y1 = maskh*0.55;
	for (int y=y0; y<y1; y++) {
		for (int x=x0-rx; x<=x0+rx; x++) {
			mask[x + maskw * y] = 255;
		}
	}

	//vertical
	int rx2 = maskw*0.03;
	int ry2 = maskh*0.1;
	int ry3 = maskh*0.2;


	for (int y=y0-ry2; y<y1+ry3; y++) {
		for (int x=x0-rx2; x<=x0+rx2; x++) {
			mask[x + maskw * y] = 255;
		}
	}

}

//--------------------------------------------------------------
//restart circle
void ofApp::circleRestartPos(int i, bool farStart)
{
	float fromUp = (farStart) ? 2.0 : 0.1;			
	float fromUpY = fromUp * _h;

	CircleData &c = circWorld_.circles()[i];
	ofPoint pNew = ofPoint( ofRandom( c.rad, _w - c.rad ), -c.rad + ofRandom( -fromUpY, 0 ) );
	circWorld_.setCirclePosAndVelocity( i, pNew, ofPoint( 0, 0 ), 0.0 );
}

//--------------------------------------------------------------
//restore falling circles
void ofApp::physicsControl()
{	
	vector<CircleData> &circles = circWorld_.circles();

	for (int i=0; i<circles.size(); i++) {
		CircleData &c = circles[ i ];
		ofPoint &p = c.p;
		if ( p.y - c.rad > _h ) { 
			circleRestartPos( i );
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	//mask
	ofImage mask_img;
	mask_img.setFromPixels(&mask[0],maskw,maskh,OF_IMAGE_GRAYSCALE);
	ofSetColor(255,128);
	mask_img.draw(0,0,_w,_h);

	//borders
	ofSetColor( 255, 0, 0 );
	vector<EdgeData> staticEdges = circWorld_.staticEdges();
	for (int i = 0; i < staticEdges.size(); i++ ) {
		EdgeData &edge = staticEdges[ i ];
		if ( edge.used ) {
			ofPoint p1 = edge.p1;
			ofPoint p2 = edge.p2;
			ofLine( p1.x, p1.y, p2.x, p2.y );
		}
	}

	//triangles
	ofFill();
	ofSetPolyMode(OF_POLY_WINDING_ODD);	// this is the normal mode
	ofSetColor( 128, 0, 0 );
	vector<TriangleData> triangles = circWorld_.triangles();
	for (int i = 0; i < triangles.size(); i++ ) {
		TriangleData &tri = triangles[ i ];
		ofBeginShape();
		for (int k=0; k<3; k++) {
			ofVertex( tri.p[k].x, tri.p[k].y);
		}
		ofEndShape();
	}
	ofNoFill();

	//circles
	ofNoFill();
	vector<CircleData> circles = circWorld_.circles();
	for (int i=0; i<circles.size(); i++) {
		CircleData &c = circles[i];

		ofSetColor( 255, 255, 0 );
		ofFill();
		ofCircle( c.p.x, c.p.y, c.rad );

		ofNoFill();
		ofSetColor( 0, 0, 0 );
		ofCircle( c.p.x, c.p.y, c.rad );
		float angle = c._body->GetAngle();
		float dx = cos( -angle );
		float dy = sin( -angle );
		ofLine( c.p.x + dx * c.rad * 0.5, c.p.y + dy * c.rad * 0.5, c.p.x + dx * c.rad, c.p.y + dy * c.rad );
	}	

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
