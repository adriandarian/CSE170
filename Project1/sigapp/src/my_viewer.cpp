
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	_nbut=0;
	_animating=false;
	build_ui ();
	build_scene ();
}

void MyViewer::build_ui ()
{
	UiPanel *p, *sp;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "View", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( _nbut=new UiCheckButton ( "Normals", EvNormals ) ); 
	}
	p->add ( new UiButton ( "Animate", EvAnimate ) );
	p->add ( new UiButton ( "Exit", EvExit ) ); p->top()->separate();
}

void MyViewer::add_model ( SnShape* s, GsVec p )
{
	// This method demonstrates how to add some elements to our scene graph: lines,
	// and a shape, and all in a group under a SnManipulator.
	// Therefore we are also demonstrating the use of a manipulator to allow the user to
	// change the position of the object with the mouse. If you do not need mouse interaction,
	// you can just use a SnTransform to apply a transformation instead of a SnManipulator.
	// You would then add the transform as 1st element of the group, and set g->separator(true).
	// Your scene graph should always be carefully designed according to your application needs.

	SnManipulator* manip = new SnManipulator;
	GsMat m;
	m.translation ( p );
	manip->initial_mat ( m );

	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	g->add(l);
	manip->child(g);
	// manip->visible(false); // call this to turn off mouse interaction

	rootg()->add(manip);
}

void MyViewer::build_scene ()
{
	pieces = {
		"Head/tinker.obj",
		"Body/tinker.obj",
		"Right-Arm/tinker.obj",
		"Left-Arm/tinker.obj",
		"Fan/tinker.obj"
	};
	sep = {
		"Head",
		"Body",
		"RightArm",
		"LeftArm",
		"Fan"
	};

	for (int i = 0; i < pieces.size(); ++i) {
		switch (i) {
			case 0: (Head)->separator(true); 
			case 1: (Body)->separator(true); 
			case 2: (RightArm)->separator(true); 
			case 3: (LeftArm)->separator(true);
			case 4: (Fan)->separator(true);  
		}

		model[i] = new SnModel;
		t[i] = new SnTransform;

		if (!model[i]->model()->load(("../../China-Women/" + pieces[i]).c_str())) {
			gsout << (pieces[i] + " was not loaded").c_str() << gsnl;
		}

		model[i]->model()->get_bounding_box(b[i]);
		m[i].translation(GsVec(x, y, z));
		// m[i].rotx((float)GS_PI/60);
		t[i]->set(m[i]);

		switch (i) {
			case 0: {
				Head->add(t[i]);
				Head->add(model[i]);
				rootg()->add(Head);
			} 
			case 1: {
				Body->add(t[i]);
				Body->add(model[i]);
				rootg()->add(Body);
			} 
			case 2: {
				RightArm->add(t[i]);
				RightArm->add(model[i]);
				rootg()->add(RightArm);
			}  
			case 3: {
				LeftArm->add(t[i]);
				LeftArm->add(model[i]);
				rootg()->add(LeftArm);
			} 
			case 4: {
				Fan->add(t[i]);
				Fan->add(model[i]);
				rootg()->add(Fan);
			}  
		}
	}
}

// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation ()
{
	if ( _animating ) return; // avoid recursive calls
	_animating = true;
	
	int ind = gs_random ( 0, rootg()->size()-1 ); // pick one child
	SnManipulator* manip = rootg()->get<SnManipulator>(ind); // access one of the manipulators
	GsMat m = manip->mat();

	double frdt = 1.0/30.0; // delta time to reach given number of frames per second
	double v = 4; // target velocity is 1 unit per second
	double t=0, lt=0, t0=gs_time();
	do // run for a while:
	{	while ( t-lt<frdt ) { ws_check(); t=gs_time()-t0; } // wait until it is time for next frame
		lt = gs_time() - t0;
		if (cameraMovement) {
			camera().eye.x += 0.01f;
			camera().center.x += 0.01f;
			camera().up.x += 0.01f;
		}
		message().setf("localtime=%f", lt);
		render(); // notify it needs redraw
		ws_check(); // redraw now
	}	while ( _animating );
	_animating = false;
}

void MyViewer::show_normals ( bool view )
{
	// Note that primitives are only converted to meshes in GsModel
	// at the first draw call.
	GsArray<GsVec> fn;
	SnGroup* r = (SnGroup*)root();
	for ( int k=0; k<r->size(); k++ )
	{	SnManipulator* manip = r->get<SnManipulator>(k);
		SnShape* s = manip->child<SnGroup>()->get<SnShape>(0);
		SnLines* l = manip->child<SnGroup>()->get<SnLines>(1);
		if ( !view ) { l->visible(false); continue; }
		l->visible ( true );
		if ( !l->empty() ) continue; // build only once
		l->init();
		if ( s->instance_name()==SnPrimitive::class_name )
		{	GsModel& m = *((SnModel*)s)->model();
			m.get_normals_per_face ( fn );
			const GsVec* n = fn.pt();
			float f = 0.33f;
			for ( int i=0; i<m.F.size(); i++ )
			{	const GsVec& a=m.V[m.F[i].a]; l->push ( a, a+(*n++)*f );
				const GsVec& b=m.V[m.F[i].b]; l->push ( b, b+(*n++)*f );
				const GsVec& c=m.V[m.F[i].c]; l->push ( c, c+(*n++)*f );
			}
		}  
	}
}

int MyViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	switch ( e.key )
	{	
		// Close Program
		case GsEvent::KeyEsc: {
			gs_exit(); 
			return 1;
		}

		// Show Normals
		case 'n': { 
			bool b=!_nbut->value(); 
			_nbut->value(b); 
			show_normals(b); 
			return 1; 
		}

		// Model Movements and Translations
		case GsEvent::KeyUp: {
			for (int i = 0; i < pieces.size(); ++i) {
				m[i].translation(GsVec(x, y + (float)(10), z));
				m[i] = m[i] * mt[i];
				t[i]->set(m[i]);
				redraw();
			}
			return 1;
		}
		case GsEvent::KeyRight: {
			x += 10.0f;
			for (int i = 0; i < pieces.size(); ++i) {
				m[i].translation(GsVec(x, y, z));
				m[i] = m[i] * mt[i];
				t[i]->set(m[i]);
				redraw();
			}
			return 1;
		}
		case GsEvent::KeyDown: {
			for (int i = 0; i < pieces.size(); ++i) {
				m[i].translation(GsVec(x, y - (float)(10), z));
				m[i] = m[i] * mt[i];
				t[i]->set(m[i]);
				redraw();
			}
			return 1;
		}
		case GsEvent::KeyLeft: {
			for (int i = 0; i < pieces.size(); ++i) {
				m[i].translation(GsVec(x - (float)(10), y, z));
				m[i] = m[i] * mt[i];
				t[i]->set(m[i]);
				redraw();
			}
			return 1;
		}

		// Rotate the Head Left and Right
		case 'q': {
			m[0].rotx((float)GS_PI/60);
			m[0] = m[0] * mt[0];
			t[0]->set(m[0]);
			redraw();
			return 1;
		}
		case 'a': {
			m[0].rotx(-(float)GS_PI/60);
			m[0] = m[0] * mt[0];
			t[0]->set(m[0]);
			redraw();
			return 1;
		}

		// Roate the Body Left and Right
		case 'w': {
			m[1].rotx((float)GS_PI/60);
			m[1] = m[1] * mt[1];
			t[1]->set(m[1]);
			redraw();
			return 1;
		}
		case 's': {
			m[1].rotx(-(float)GS_PI/60);
			m[1] = m[1] * mt[1];
			t[1]->set(m[1]);
			redraw();
			return 1;
		}

		// Rotate the Right Arm Up and Down
		case 'e': {
			m[2].rotx((float)GS_PI/60);
			m[2] = m[2] * mt[2];
			t[2]->set(m[2]);
			redraw();
			return 1;
		}
		case 'd': {
			m[2].rotx(-(float)GS_PI/60);
			m[2] = m[2] * mt[2];
			t[2]->set(m[2]);
			redraw();
			return 1;
		}

		// Rotate the Left Arm Up and Down
		case 'r': {
			m[3].rotx((float)GS_PI/60);
			m[3] = m[3] * mt[3];
			t[3]->set(m[3]);
			redraw();
			return 1;
		}
		case 'f': {
			m[3].rotx(-(float)GS_PI/60);
			m[3] = m[3] * mt[3];
			t[3]->set(m[3]);
			redraw();
			return 1;
		}

		// Rotate Fan Back anf Forth
		case 't': {
			m[4].rotx((float)GS_PI/60);
			m[4] = m[4] * mt[4];
			t[4]->set(m[4]);
			redraw();
			return 1;
		}
		case 'g': {
			m[4].rotx(-(float)GS_PI/60);
			m[4] = m[4] * mt[4];
			t[4]->set(m[4]);
			redraw();
			return 1;
		}

		// Camera Movement
		case ' ': {
			cameraMovement = !cameraMovement; 
			// return 1;
		}
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvNormals: show_normals(_nbut->value()); return 1;
		case EvAnimate: run_animation(); return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
