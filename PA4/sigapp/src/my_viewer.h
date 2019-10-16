# pragma once

# include <sig/sn_poly_editor.h>
# include <sig/sn_lines2.h>

# include <sigogl/ui_button.h>
# include <sigogl/ws_viewer.h>

// Viewer for this example:
class MyViewer : public WsViewer
{  protected :
	enum MenuEv { EvNormals, EvAnimate, EvExit };
	UiCheckButton* _nbut;
	bool _animating;
	bool _smooth = true;
	float R = 1.0f;
	float r = 0.2f;
	float n = 10.0f;
	int a = 0;
	int b = 1;
	int c = 2;

   public :
	MyViewer ( int x, int y, int w, int h, const char* l );
	void build_ui ();
	void add_model ( SnShape* s, GsVec p );
	void build_scene ();
	void show_normals ( bool view );
	void run_animation ();
	void compute_segments(bool smooth, bool view);
	virtual int handle_keyboard ( const GsEvent &e ) override;
	virtual int uievent ( int e ) override;

};

