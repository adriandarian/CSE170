
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>

GsModel* Torus = new GsModel;
SnModel* sn;

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

	for (float theta = 0.0f; theta < GS_2PI; theta += float(GS_2PI / n)) {
		for (float phi = 0.0f; phi < GS_2PI; phi += float(GS_2PI / n)) { //132234			
			Torus->V.push() = GsVec(float(R + r * cos(theta)) * cos(phi), float(R + r * cos(theta)) * sin(phi), r * sin(theta));
			Torus->V.push() = GsVec(float(R + r * cos(theta)) * cos(phi + float(GS_2PI / n)), (R + r * cos(theta)) * sin(phi + (GS_2PI / n)), r * sin(theta));
			Torus->V.push() = GsVec(float(R + r * cos(theta + (GS_2PI / n))) * cos(phi), float(R + r * cos(theta + (GS_2PI / n))) * sin(phi), r * sin(theta + (GS_2PI / n)));

			Torus->V.push() = GsVec(float(R + r * cos(theta + (GS_2PI / n))) * cos(phi), float(R + r * cos(theta + (GS_2PI / n))) * sin(phi), r * sin(theta + (GS_2PI / n)));
			Torus->V.push() = GsVec(float(R + r * cos(theta)) * cos(phi + (GS_2PI / n)), float(R + r * cos(theta)) * sin(phi + (GS_2PI / n)), r * sin(theta));
			Torus->V.push() = GsVec(float(R + r * cos(theta + (GS_2PI / n))) * cos(phi + (GS_2PI / n)), float(R + r * cos(theta + (GS_2PI / n))) * sin(phi + (GS_2PI / n)), r * sin(theta + (GS_2PI / n)));

			Torus->F.push() = GsModel::Face(a, b, c);
			Torus->F.push() = GsModel::Face(a + 3, b + 3, c + 3);

			if (_smooth) { //132143
				Torus->N.push() = GsVec(float(R + r * cosf(GS_TORAD(theta))) * cosf(GS_TORAD(phi)), float(R + r * cosf(GS_TORAD(theta))) * sinf(GS_TORAD(phi)), float(r * sinf(GS_TORAD(theta))));
				Torus->N.push() = GsVec(float(R + r * cosf(GS_TORAD(theta + (GS_2PI / n)))) * cosf(GS_TORAD(phi + (GS_2PI / n))), float(R + r * cosf(GS_TORAD(theta + (GS_2PI / n)))) * sinf(GS_TORAD(phi + (GS_2PI / n))), float(r * sinf(GS_TORAD(theta + (GS_2PI / n)))));
				Torus->N.push() = GsVec(float(R + r * cosf(GS_TORAD(theta + (GS_2PI / n)))) * cosf(GS_TORAD(phi)), float(R + r * cosf(GS_TORAD(theta + (GS_2PI / n)))) * sinf(GS_TORAD(phi)), float(r * sinf(GS_TORAD(theta + (GS_2PI / n)))));

				Torus->N.push() = GsVec(float(R + r * cosf(GS_TORAD(theta))) * cosf(GS_TORAD(phi)), float(R + r * cosf(GS_TORAD(theta))) * sinf(GS_TORAD(phi)), float(r * sinf(GS_TORAD(theta))));
				Torus->N.push() = GsVec(float(R + r * cosf(GS_TORAD(theta))) * cosf(GS_TORAD(phi + (GS_2PI / n))), float(R + r * cosf(GS_TORAD(theta))) * sinf(GS_TORAD(phi + (GS_2PI / n))), float(r * sinf(GS_TORAD(theta))));
				Torus->N.push() = GsVec(float(R + r * cosf(GS_TORAD(theta + (GS_2PI / n)))) * cosf(GS_TORAD(phi + (GS_2PI / n))), float(R + r * cosf(GS_TORAD(theta + (GS_2PI / n)))) * sinf(GS_TORAD(phi + (GS_2PI / n))), float(r * sinf(GS_TORAD(theta + (GS_2PI / n)))));

				Torus->set_mode(GsModel::Smooth, GsModel::NoMtl);
			} else {
				Torus->set_mode(GsModel::Flat, GsModel::NoMtl);
			}

			a += 6;
			b += 6;
			c += 6;
		}
	}

	sn = new SnModel(Torus);
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

void MyViewer::compute_segments(bool smooth, bool view) {
	SnLines* l = new SnLines;
	l->init();
	l->color(GsColor::orange);

	if (!view) { 
		l->visible(false);
	} else {
		l->visible(true);
	}

	gsout << view << gsnl;

	if (!smooth) {
		GsModel& m = *sn->model();
		for (int i = 0; i < m.F.size(); i++) {
			const GsVec& a = m.V[m.F[i].a];
			const GsVec& b = m.V[m.F[i].b];
			const GsVec& c = m.V[m.F[i].c];
			GsVec fcenter = (a + b + c) / 3.0f;
			l->push(fcenter, fcenter + (m.N[i] * 2));
		}
	} else {
		GsModel& m = *sn->model();
		for (int i = 0; i < m.V.size(); i++) {
			l->push(m.V[i], m.V[i] + m.N[i] * 2);
		}
	}

	rootg()->add(l);
}

void MyViewer::show_normals ( bool view )
{
	// Note that primitives are only converted to meshes in GsModel
	// at the first draw call.
	GsArray<GsVec> fn;
	SnGroup* r = (SnGroup*)root();
	for ( int k=0; k<r->size(); k++ )
	{	
		SnManipulator* manip = r->get<SnManipulator>(k);
		SnShape* s = manip->child<SnGroup>()->get<SnShape>(0);
		SnLines* l = manip->child<SnGroup>()->get<SnLines>(1);
		if ( !view ) { l->visible(false); continue; }
		l->visible ( true );
		if ( !l->empty() ) continue; // build only once
		l->init();
		if ( s->instance_name()==SnPrimitive::class_name )
		{	
			GsModel& m = *((SnModel*)s)->model();
			m.get_normals_per_face ( fn );
			const GsVec* n = fn.pt();
			float f = 0.33f;
			for ( int i=0; i<m.F.size(); i++ )
			{	
				const GsVec& a=m.V[m.F[i].a]; l->push ( a, a+(*n++)*f );
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
		case 'q':
			n += 1.0f;
			redraw();
			return 1;
		case 'a': 
			if (n - 1.0f == 3.0f)
				return 1;
			n -= 1.0f;
			redraw();
			return 1;
		case 'w': 
			r += 0.01f;
			redraw();
			return 1;
		case 's': 
			if (r - 0.01f == 0.2f)
				return 1;
			r -= 0.01f;
			redraw();
			return 1;
		case 'e': 
			R += 0.01f;
			redraw();
			return 1;
		case 'd': 
			if (R - 0.1f <= r)
				return 1;
			R -= 0.01f;
			redraw();
			return 1;
		case 'z':
			_smooth = false;
			redraw();
			return 1;
		case 'x':
			_smooth = true;
			redraw();
			return 1;
		case 'c':
			compute_segments(_smooth, true);
			redraw();
			return 1;
		case 'v':
			compute_segments(_smooth, false);
			redraw();
			return 1;
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