# pragma once

# include <sig/sn_poly_editor.h>
# include <sig/sn_lines2.h>

# include <sigogl/ui_button.h>
# include <sigogl/ws_viewer.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>

// Viewer for this example:
class MyViewer : public WsViewer
{  protected :
	enum MenuEv { EvNormals, EvAnimate, EvExit };
	UiCheckButton* _nbut;
	bool _animating;
	SnGroup* g = new SnGroup;
	GsMat m[3];
	GsMat mt[3];
	SnTransform * t[3];
	//SnModel* model[3];
	GsBox b0, b1, b2;

	float upperarm = (float)(GS_PI / 60);
	float lowerarm = (float)(GS_PI / 80);
	float hand = (float)(GS_PI / 100);
   public :
	MyViewer ( int x, int y, int w, int h, const char* l );
	void build_ui ();
	void add_model ( SnShape* s, GsVec p );
	void build_scene ();
	void show_normals ( bool view );
	void run_animation ();
	void upperArmUp();
	void upperArmDown();
	void lowerArmUp();
	void lowerArmDown();
	void handUp();
	void handDown();
	virtual int handle_keyboard ( const GsEvent &e ) override;
	virtual int uievent ( int e ) override;
};

