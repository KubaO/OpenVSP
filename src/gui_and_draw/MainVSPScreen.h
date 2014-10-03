//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef MAINSCREEN__INCLUDED_
#define MAINSCREEN__INCLUDED_

#include "VSPScreenQt.h"

#include <string>

namespace VSPGUI
{
class VspGlWindow;
}

class MainVSPScreen : public VspScreenQt
{
    VSP_DECLARE_PRIVATE( MainVSPScreen )
public:
    MainVSPScreen( ScreenMgr* mgr );
    ~MainVSPScreen();

    void SetFileLabel( const std::string & fname );
    /*!
    * Return reference to OpenGL Window.
    */
    VSPGUI::VspGlWindow * GetGLWindow();
    void ShowReturnToAPI();
    void HideReturnToAPI();
};

#endif // MAINSCREEN__INCLUDED_
