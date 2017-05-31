#include "ofxKuBox2d.h"

//--------------------------------------------------------------------
void ofxKuBox2dWorld::setup(ofxKuBox2dWorldParams prm0)
{
	prm = prm0;
	//_scale = 1.0 / ( 50.0 / 1000.0 );	//чтоб объект в 50 пикселов на экране в 1000 был 1 метр
	_scale = prm.pixel_size_in_millimeters;

	_invScale = 1.0 / _scale;
	_w = prm.scr_w / _scale;
	_h = prm.scr_h / _scale;


	// Define the gravity vector.
	b2Vec2 gravity(0.0f, prm.gravity);

	// Do we want to let bodies sleep?
	bool doSleep = true;

	// Construct a world object, which will hold and simulate the rigid bodies.
	_world = new b2World(gravity, doSleep);

	// Выделяем память под ограничения
	_staticEdges.resize( maxStaticEdges );
	_groundBody = 0;
}

//--------------------------------------------------------------------
void ofxKuBox2dWorld::clear() 
{
	delete _world;

}
//--------------------------------------------------------------------
b2Vec2 ofxKuBox2dWorld::toWorld( const ofPoint &p )
{
	return b2Vec2( p.x * _invScale, _h - p.y * _invScale );

}

//--------------------------------------------------------------------
ofPoint ofxKuBox2dWorld::toScreen( const b2Vec2 &p )
{
	return ofPoint( p.x * _scale, ( _h - p.y ) * _scale );
}

//--------------------------------------------------------------------
b2Vec2 ofxKuBox2dWorld::forceToWorld( const ofPoint &p )
{
	return b2Vec2( p.x * _invScale, - p.y * _invScale );
}

//--------------------------------------------------------------------
/*void ofxKuBox2dWorld::beginOglTransform()		//установить трансформацию openGL для вывода в его системе координат
{
ofPushMatrix();			
ofScale( _scale, -_scale );	
ofTranslate( 0, -_h );
}*/

//--------------------------------------------------------------------
/*void ofxKuBox2dWorld::endOglTransform()			//вернуть трансформацию
{
ofPopMatrix();
}
*/

//--------------------------------------------------------------------
void ofxKuBox2dWorld::update( bool physics )
{
	if ( physics ) {
		// Prepare for simulation. Typically we use a time step of 1/60 of a
		// second (60Hz) and 10 iterations. This provides a high quality simulation
		// in most game scenarios.
		float32 timeStep = prm.timeStep();
		int32 velocityIterations = prm.velocity_iterations;			
		int32 positionIterations = prm.position_iterations;				 

		// Instruct the world to perform a single step of simulation.
		// It is generally best to keep the time step and iterations fixed.
		_world->Step(timeStep, velocityIterations, positionIterations);

		// Clear applied body forces. We didn't apply any forces, but you
		// should know about this function.
		_world->ClearForces();

		/*
		// Now print the position and angle of the body.
		b2Vec2 position = body->GetPosition();
		float32 angle = body->GetAngle();	

		printf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);
		*/
	}

	//Обновление списка кругов
	for (int i=0; i<_circles.size(); i++) {
		CircleData &c = _circles[ i ];
		b2Vec2 position = c._body->GetPosition();
		float32 angle = c._body->GetAngle();
		c.angleDeg = angleToScreen( angle );
		c.p = toScreen( position );
	}	

}

//--------------------------------------------------------------------
//добавление элементов
int ofxKuBox2dWorld::addStaticEdge( const ofPoint &p1, const ofPoint &p2, int id )
{	
	if ( id == -1 ) return id;
	//if ( _staticEdges[ id ].used ) return -1;

	//ofPoint p1 = edge.p1;
	//ofPoint p2 = edge.p2;

	if ( !_staticEdges[ id ].used ) {	//еще не создан

		// Call the body factory which allocates memory for the ground body
		// from a pool and creates the ground box shape (also from a pool).
		// The body is also added to the world.
		if ( !_groundBody ) {
			// Define the ground body.
			b2BodyDef bodyDef;
			bodyDef.position.Set( 0.0f, 0.0f );
			_groundBody = world()->CreateBody(&bodyDef);
		}
		b2Body *body = _groundBody;

		// Define the ground box shape.
		b2PolygonShape shape;

		// The extents are the half-widths of the box.
		//groundBox.SetAsBox( _physics.sizeToWorld(500), _physics.sizeToWorld(100));
		shape.SetAsEdge( toWorld( p1 ), toWorld( p2 ) );

		// Add the ground fixture to the ground body.
		b2Fixture* fixture = body->CreateFixture(&shape, 0.0f);

		//запоминаем для будущей отрисовки
		EdgeData edge( p1, p2, fixture );
		_staticEdges[ id ] = edge; //.push_back( edge );
	}
	else {	//уже есть такой, просто перемещаем
		EdgeData &edge = _staticEdges[ id ];
		edge.p1 = p1;
		edge.p2 = p2;
		b2PolygonShape *shape = (b2PolygonShape *)edge._fixture->GetShape();
		shape->SetAsEdge( toWorld( p1 ), toWorld( p2 ) );
	}

	return id;

}

//--------------------------------------------------------------------
void ofxKuBox2dWorld::removeStaticEdge( int id )			//удаляет ребро
{
	addStaticEdge( ofPoint( -id * 10 - 10, 0), ofPoint( -id*10 - 10 + 1, 0 ), id );
	/*if ( id >= 0 && _staticEdges[ id ].used) {
		EdgeData &edge = _staticEdges[ id ];
		_groundBody->DestroyFixture( edge._fixture );
		edge.used = false;
	}*/
}

//--------------------------------------------------------------------
void ofxKuBox2dWorld::beginChanges()		//начало изменения элементов
{

}

void ofxKuBox2dWorld::endChanges()			//конец изменения элементов
{
	_groundBody->SetTransform( b2Vec2( 0.0, 0.0 ), 0.0 );		//чтоб обновил касания
}


//--------------------------------------------------------------------
void ofxKuBox2dWorld::addCircle( const CircleData &circle0 )
{
	CircleData circle = circle0;

	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = toWorld( circle.p ); //.Set(0.0f, 15.0f);
	bodyDef.angle = angleToWorld( circle.angleDeg );
	b2Body* body = world()->CreateBody(&bodyDef);
	circle._body = body;

	// Define another box shape for our dynamic body.
	b2CircleShape circleShape;
	circleShape.m_p.SetZero();
	circleShape.m_radius = sizeToWorld( circle.rad );

	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circleShape;

	// Set the box density to be non-zero, so it will be dynamic.
	fixtureDef.density = circle.density;

	// Override the default friction.
	fixtureDef.friction = circle.friction;			//0.3f;
	fixtureDef.restitution = circle.restitution;	//0.2;

	// Add the shape to the body.
	body->CreateFixture(&fixtureDef);

	//запись в список
	_circles.push_back( circle );

}

//--------------------------------------------------------------------
void ofxKuBox2dWorld::setCirclePosAndVelocity( int i, ofPoint pos, ofPoint vel, float angVel ) //используется в случае, когда улетает за границы
{
	CircleData &c = _circles[ i ];
	b2Body* body = c._body;
	body->SetTransform( toWorld( pos ), body->GetAngle() );
	body->SetLinearVelocity( forceToWorld( vel ) );
	body->SetAngularVelocity( angVel );

}

void ofxKuBox2dWorld::setCircleRadAndTexture( int i, float rad, int texture )
{
	CircleData &c = _circles[ i ];
	c.rad = rad;
	c.id = texture;
	b2Body *body = c._body;
	b2Fixture* fixture = body->GetFixtureList();
	b2CircleShape *shape = (b2CircleShape*) fixture->GetShape();
	shape->m_radius = sizeToWorld( rad );
	
}

//--------------------------------------------------------------------
void ofxKuBox2dWorld::addTriangle( const ofPoint &point0, const ofPoint &point1, const ofPoint &point2 )
{

	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; //b2_dynamicBody;
	bodyDef.position.Set(0.0f, 0.0f);
	bodyDef.angle = 0.0;
	b2Body* body = world()->CreateBody(&bodyDef);
	
	// Define another box shape for our dynamic body.
	vector<b2Vec2> v(3);
	v[0] = toWorld( point0 );
	v[1] = toWorld( point1 );
	v[2] = toWorld( point2 );
	b2PolygonShape shape;
	shape.Set(  &v[0], 3 );
	b2Fixture* fixture = body->CreateFixture(&shape, 0.0f);

	//запись в список

	TriangleData tri( point0, point1, point2, fixture, body ); 
	_triangles.push_back( tri );

}

//--------------------------------------------------------------------
void ofxKuBox2dWorld::clearTriangles()
{
	for (int i=0; i<_triangles.size(); i++) {
		world()->DestroyBody( _triangles[i]._body );
	}
	_triangles.clear();

}

//--------------------------------------------------------------------
void ofxKuBox2dWorld::findMoveFromMask(vector<unsigned char> &mask, int w, int h, ofPoint p, int rad, ofVec2f &newPos, bool &moved)
{
	const float scaleX = 1.0 * w / prm.scr_w;
	const float scaleY = 1.0 * h / prm.scr_h;
	
	int x, y;
	bool inside;
	//расчет вектора выталкивания

	//ofPoint p = c.p;
	newPos = ofVec2f( 0, 0 );
	moved = false;

	//метод сходящихся шаров
	const int iterations = prm.rolling_iterations;		//число итераций сходящихся шаров

	for ( int iter = 0; iter < iterations; iter++ ) {
		ofVec2f sum( 0, 0 );
		float sumCount = 0.0;
		bool ok = true;
		for ( int dy = -rad; dy <= rad; dy++ ) {
			for ( int dx = -rad; dx <= rad; dx++ ) {
				if ( dx * dx + dy * dy <= rad * rad ) {
					x = ( p.x + dx ) * scaleX;
					y = ( p.y + dy ) * scaleY;
					bool inside = ( x >= 0 && x < w && y >= 0 && y < h );
					if ( !inside || mask[x+w*y] == 0 ) {
						sumCount += 1.0;
						sum += ofVec2f( dx, dy );
					}
					else ok = false;

				}						
			}
		}
		//использование полученных данных
		if ( sumCount > 0.001 ) {								
			p = p + sum / sumCount;
			newPos = p;
			moved = (fabs(sum.x)+fabs(sum.y)>0.01);
		}
		if ( ok ) break;
	}

}

//--------------------------------------------------------------
//Apply mask to physics world
void ofxKuBox2dWorld::applyMask(vector<unsigned char> &mask, int w, int h)
{
	float timeStep = prm.timeStep();
	//Выталкивание частиц
	vector<CircleData> &circles = _circles;
	for (int i=0; i<circles.size(); i++) {
		CircleData &c = circles[ i ];

		//Найти вектор сдвига шара

		//поиск сдвига по фону
		ofVec2f newPos = c.p;
		bool moved;
		findMoveFromMask( mask, w, h, c.p, c.rad, newPos, moved ); 

		if ( moved ) {		//нужно воздействие
			ofVec2f bestPos = newPos; 
			//1. Меняем положение 
			c._body->SetTransform( toWorld( bestPos ), c._body->GetAngle() );	

			//2. Cила
			const float ForceBoost = prm.force_boost;				//ПАРАМЕТР усиления силы

			ofVec2f delta = bestPos - c.p;
			b2Vec2 force = forceToWorld( delta );
			force *= c._body->GetMass() * 1.0 / timeStep * ForceBoost; 
			c._body->ApplyForce( force , c._body->GetPosition() );

			//применяем нулевую силу... видимо, это требуется чтоб пересчитадись изменения...
			//c._body->ApplyForce( b2Vec2(0,0), c._body->GetPosition() );
		}

		//if ( debugDraw ) {
		//	if ( testVector.size() < circles.size() ) testVector.resize( circles.size() );
		//	testVector[ i ] = ( moved ) ? (newPos - c.p):ofPoint(0,0);
		//}

	}

}

//--------------------------------------------------------------

