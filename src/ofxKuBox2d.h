#pragma once

#include "ofMain.h"
#include <Box2D/Box2D.h>

struct CircleData {
	ofPoint p;
	float rad;
	float angleDeg;
	int id;		//id - for external usage, for custom drawing
	int id0;	//for additional external use

	float density;
	float friction;		
	float restitution;	

	int custom_i1, custom_i2, custom_i3, custom_i4;	//user's data for texture coords and so on
	float custom_f1, custom_f2, custom_f3, custom_f4;	

	//---- 
	b2Body *_body;

	CircleData() {
		rad			= 10;
		angleDeg	= 0;
		id			= 0;
		id0			= 0;
		density		= 10.0;	
		friction	= 0.2; 
		restitution	= 0.4; 
	}
};

struct RectData {
	ofPoint p;
	float radx, rady;
	float angleDeg;
	int id;		//id - for external usage, for custom drawing
	int id0;	//for additional external use

	float density;
	float friction;
	float restitution;

	//---- 
	b2Body *_body;

	RectData() {
		radx = rady = 10;
		angleDeg = 0;
		id = 0;
		id0 = 0;
		density = 10.0;
		friction = 0.2;
		restitution = 0.4;
	}
};

struct EdgeData {
	EdgeData() { used = false; }
	EdgeData( const ofPoint &p01, const ofPoint &p02, b2Fixture* fixture, bool used0 = true ) 
	{
			p1 = p01; 
			p2 = p02; 
			_fixture = fixture;
			used = used0; 
	}
	ofPoint p1, p2;
	bool used;

	b2Fixture* _fixture;

};

struct TriangleData {
	TriangleData( const ofPoint &p01, const ofPoint &p02, const ofPoint &p03, b2Fixture* fixture, b2Body *body ) 
	{
		p[0] = p01;
		p[1] = p02;
		p[2] = p03;
		_fixture = fixture;
		_body = body;
	}
	ofPoint p[3];

	b2Body *_body;
	b2Fixture* _fixture;

};

//Parameters of the Physics World
//feel free to adjust them at your own before calling ofxKuBox2dWorld::setup and later

struct ofxKuBox2dWorldParams {
	//--------- Constants
	//Changing them in update doesn't have effect
	//Space
	float scr_w, scr_h;						//Screen size in pixels
	float pixel_size_in_millimeters;		//Increase it speeding up the objects

	//Forces
	float gravity;						//Gravity force

	//--------- Variables
	//You can change them dynamically
	//Time
	float fps;							//Frame rate
	float timeStep() { return (fps>0)?(1.0/fps):(1.0/60.0); }

	//Applying mask to world
	int velocity_iterations;	//Number of iterations for compute physics			
	int position_iterations;			
	int rolling_iterations;		//Iterations in rolling ball method for searching free space in mask
	float force_boost;	//Amplifying force from mask

	//Constructor
	ofxKuBox2dWorldParams() {
		scr_w = 1024;
		scr_h = 768;
		pixel_size_in_millimeters = 100.0;

		gravity = -5.0;

		fps = 60.0;

		velocity_iterations = 10;				
		position_iterations = 10;	
		rolling_iterations = 3;
		force_boost = 20.0;
	}
};

//Physics World
class ofxKuBox2dWorld
{
public:
	void setup(ofxKuBox2dWorldParams prm);

	//Apply mask to physics world
	void applyMask(vector<unsigned char> &mask, int w, int h);

	void update( bool physics = true );
	void clear();

	ofxKuBox2dWorldParams prm;		//Parameters

	//------------------------------------
	b2World *world() { return _world; }

	 b2Vec2 toWorld( const ofPoint &p );
	 ofPoint toScreen( const b2Vec2 &p );
	 b2Vec2 forceToWorld( const ofPoint &p );

	inline float sizeToWorld( float size ) { return size * _invScale; }
	inline float sizeToScreen( float size ) { return size * _scale; }
	inline float angleToWorld( float angle ) { return - ofDegToRad( angle ); }
	inline float angleToScreen( float angle ) { return - ofRadToDeg( angle ); }

	//void beginOglTransform();		//установить трансформацию openGL для вывода в его системе координат
	//void endOglTransform();			//вернуть трансформацию


	//добавление элементов
	int addStaticEdge( const ofPoint &point0, const ofPoint &point1, int id );	//возвращает id или -1
	void removeStaticEdge( int id );			//удаляет ребро
	vector<EdgeData> &staticEdges() { return _staticEdges; }
	void beginChanges();		//начало изменения элементов
	void endChanges();			//конец изменения элементов

	void addCircle( const CircleData &circle ); 
	vector<CircleData> &circles() { return _circles; }
	void setCirclePos(int i, ofPoint pos);
	void setCirclePosAndVelocity( int i, ofPoint pos, ofPoint vel, float angVel ); //используется в случае, когда улетает за границы
	void setCircleRadAndTexture( int i, float rad, int texture );
	int circleId( int i, bool old = false ) { return ( !old ) ? _circles[i].id : _circles[i].id0; }
	void setCircleId( int i, int cl ) { _circles[i].id = cl; }


	void addRect(const RectData &rect);
	vector<RectData> &rects() { return _rects; }
	void setRectPos(int i, ofPoint pos);
	void setRectPosAndVelocity(int i, ofPoint pos, ofPoint vel, float angVel); //используется в случае, когда улетает за границы
	//void setRectRadAndTexture(int i, float rad, int texture);
	int RectId(int i, bool old = false) { return (!old) ? _rects[i].id : _rects[i].id0; }
	void setRectId(int i, int cl) { _rects[i].id = cl; }



	void addTriangle( const ofPoint &point0, const ofPoint &point1, const ofPoint &point2 );
	void clearTriangles();
	vector<TriangleData> &triangles() { return _triangles; }


protected:
	float _scale, _invScale;
	float _w, _h; 

	b2World *_world;

	static const int maxStaticEdges = 1000;
	vector<EdgeData> _staticEdges;
	b2Body *_groundBody;

	vector<CircleData> _circles;
	vector<RectData> _rects;
	vector<TriangleData> _triangles;
	
	//find vector of circle move
	void findMoveFromMask(vector<unsigned char> &mask, int w, int h, ofPoint p, int rad, ofVec2f &newPos, bool &moved);

};

