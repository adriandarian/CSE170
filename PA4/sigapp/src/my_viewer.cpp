
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
	GsModel* myTorus;
	//GsArray<GsVec> P(0, 6);
	myTorus = new GsModel;
	SnModel* sn;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float thRad = GS_TORAD(float(th));
	float phRad = GS_TORAD(float(ph));
	tSmooth = true;
	th = 0.0f;
	ph = 0.0f;
	R = 1.0f;
	r = 0.2f;
	n = 10.0f;
	pi = 2.0f * 3.14156f;
	dth = pi / n;

	int a = 0;
	int b = 1;
	int c = 2;

	for (th = 0.0; th < 2 * pi; th += dth) {
		for (ph = 0.0; ph < 2 * pi; ph += dth) {
			//Peset locations for the 6 points based on Torus formulas
			GsVec p1((R + r * cos(th)) * cos(ph), (R + r * cos(th)) * sin(ph), r * sin(th));
			GsVec p2((R + (r * cos(th + dth))) * cos(ph), (R + r * cos(th + dth)) * sin(ph), r * sin(th + dth));
			GsVec p3((R + r * cos(th)) * cos(ph + dth), (R + r * cos(th)) * sin(ph + dth), r * sin(th));
			GsVec p4((R + r * cos(th + dth)) * cos(ph + dth), (R + r * cos(th + dth)) * sin(ph + dth), r * sin(th + dth));
			
			GsModel::Face f1 = GsModel::Face(0, 1, 2);
			GsModel::Face f2 = GsModel::Face(a + 3, b + 3, c + 3);

			//First triangle
			myTorus->V.push() = p1;
			myTorus->V.push() = p3;
			myTorus->V.push() = p4;
			//Second triangle
			myTorus->V.push() = p3;
			myTorus->V.push() = p4;
			myTorus->V.push() = p2;

			myTorus->F.push() = f1;
			myTorus->F.push() = f2;

			if (tSmooth) {
				
				p1 = GsVec(float(R + r * cosf(phRad)) * cosf(thRad), float(R + r * cosf(phRad)) * sinf(thRad), float(r * sinf(phRad)));
				p2 = GsVec(float(R + r * cosf(phRad)) * cosf(thRad), float(R + r * cosf(phRad)) * sinf(thRad), float(r * sinf(phRad)));
				p3 = GsVec(float(R + r * cosf(phRad)) * cosf(thRad), float(R + r * cosf(phRad)) * sinf(thRad), float(r * sinf(phRad)));
				p4 = GsVec(float(R + r * cosf(phRad)) * cosf(thRad), float(R + r * cosf(phRad)) * sinf(thRad), float(r * sinf(phRad)));

				myTorus->N.push() = p1;
				myTorus->N.push() = p3;
				myTorus->N.push() = p4;

				myTorus->N.push() = p3;
				myTorus->N.push() = p4;
				myTorus->N.push() = p2;

				myTorus->set_mode(GsModel::Smooth, GsModel::NoMtl);
			}

			a += 6;
			b += 6;
			c += 6;
		}
	}

	sn = new SnModel(myTorus);
	add_model(sn, GsVec(0, 0, 0));
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
		double yinc = (t-lt)*v;
		if ( t>2 ) yinc=-yinc; // after 2 secs: go down
		lt = t;
		m.e24 += (float)yinc;
		if ( m.e24<0 ) m.e24=0; // make sure it does not go below 0
		manip->initial_mat ( m );
		render(); // notify it needs redraw
		ws_check(); // redraw now
	}	while ( m.e24>0 );
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
	{	case GsEvent::KeyEsc : gs_exit(); return 1;
		case 'n' : { bool b=!_nbut->value(); _nbut->value(b); show_normals(b); return 1; }
		case 'q': ++n;
			render();
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

//Andrew Ng