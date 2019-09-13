
# include "my_viewer.h"
# include "sn_mynode.h"

# include <sigogl/gl_tools.h>

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	add_ui ();
	add_mynode (4);
}

void MyViewer::add_ui ()
{
	UiPanel *p;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "Add", EvAdd ) );
	p->add ( mcolorbut=new UiCheckButton ( "Multicolor", EvMultiColor ) );
	p->add ( new UiButton ( "OpenGL", EvInfo ) );
	p->add ( new UiButton ( "Exit", EvExit ) );
}

void MyViewer::add_mynode ( int n )
{
	SnMyNode* c;

	float r=0.15f; // position range
		c = new SnMyNode;
		if ( mcolorbut->value() )
			c->multicolor = true;
		else
			c->color ( GsColor::random() );

		c->init.set(0, 0, 0);


		rootg()->add(c);
}

int MyViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	SnMyNode* c = (SnMyNode*)rootg()->get(0);

	switch ( e.key )
	{	case GsEvent::KeyEsc : gs_exit(); return 1;
		case GsEvent::KeyLeft: gsout<<"Left\n"; return 1;
		case GsEvent::KeyQ: 
			gsout << "Q\n";
			c->n += 1.0f;
			c->touch();
			redraw();
			return 1;
		case GsEvent::KeyA: 
			gsout << "A\n";
			if (c->n - 1.0f == 3.0f)
				return 1;
			c->n -= 1.0f;
			c->touch();
			redraw();
			return 1;
		case GsEvent::KeyW: 
			gsout << "W\n"; 
			c->r += 0.01f;
			c->touch();
			redraw();
			return 1;
		case GsEvent::KeyS: 
			gsout << "S\n";
			c->r -= 0.01f;
			c->touch();
			redraw();
			return 1;
		case GsEvent::KeyE: 
			gsout << "E\n"; 
			c->R += 0.01f;
			c->touch();
			redraw();
			return 1;
		case GsEvent::KeyD: 
			gsout << "D\n";
			c->R -= 0.01f;
			c->touch();
			redraw();
			return 1;
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvAdd: add_mynode(1); return 1;

		case EvInfo:
		{	if ( output().len()>0 ) { output(""); return 1; }
			output_pos(0,30);
			activate_ogl_context(); // we need an active context
			GsOutput o; o.init(output()); gl_print_info(&o); // print info to viewer
			return 1;
		}

		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
