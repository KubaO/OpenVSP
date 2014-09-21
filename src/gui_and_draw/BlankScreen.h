#if !defined(BLANKSCREEN__INCLUDED_)
#define BLANKSCREEN__INCLUDED_

#include "ScreenBaseFLTK.h"
#include "GuiDevice.h"
#include "GroupLayout.h"

#include <FL/Fl.H>

class BlankScreen : public GeomScreen
{
public:
    BlankScreen( ScreenMgr* mgr );
    virtual ~BlankScreen()                          {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );

protected:

    GroupLayout m_MassLayout;
    ToggleButton m_PointMassButton;
    Input m_MassInput;

};


#endif // !defined(BLANKSCREEN__INCLUDED_)
