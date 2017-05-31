#pragma once

#include "ofMain.h"
#include <Box2D/Box2D.h>

struct CircleData {
	ofPoint p;
	float rad;
	float angleDeg;
	int groupIndex;		//тип круга - для рендеринга
	int groupIndex0;	//исходный тип круга - чтоб восстанавливать

	float density;
	float friction;		//трение
	float restitution;	//отскок

	//---- внутренняя переменная
	b2Body *_body;	//для обновления координат, делается само в update()
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


	//Constructor
	ofxKuBox2dWorldParams() {
		scr_w = 1024;
		scr_h = 768;
		pixel_size_in_millimeters = 100.0;

		gravity = -5.0;

		fps = 60.0;

	}
};

//Physics World
class ofxKuBox2dWorld
{
public:
	void setup(ofxKuBox2dWorldParams prm);

	//Apply mask to physics world
	void applyForces(vector<unsigned char> &mask, int w, int h);

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
	void setCirclePosAndVelocity( int i, ofPoint pos, ofPoint vel, float angVel ); //используется в случае, когда улетает за границы
	void setCircleRadAndTexture( int i, float rad, int texture );
	int circleClass( int i, bool old = false ) { return ( !old ) ? _circles[i].groupIndex : _circles[i].groupIndex0; }
	void setCircleClass( int i, int cl ) { _circles[i].groupIndex = cl; }

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
	vector<TriangleData> _triangles;

	
	//найти вектор сдвига шара
	void findMoveFromMask(vector<unsigned char> &mask, int w, int h, ofPoint p, int rad, ofVec2f &newPos, bool &moved);

};

