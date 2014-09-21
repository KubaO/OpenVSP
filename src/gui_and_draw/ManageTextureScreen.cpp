//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ManageTextureScreen.h"

#include "ScreenMgr.h"
#include "SubGLWindow.h"
#include "GraphicEngine.h"
#include "Display.h"
#include "Viewport.h"
#include "Background.h"
#include "GraphicSingletons.h"
#include "GuiDeviceQt.h"

#include "Geom.h"
#include "TextureMgr.h"
#include "StlHelper.h"
#include "ui_ManageTextureScreen.h"
#include "VspScreenQt_p.h"
#include <cassert>

namespace {

struct CompDropDownItem
{
    std::string GeomName;
    std::string GeomID;
    int GUIIndex;
};

struct TexDropDownItem
{
    Texture * TexInfo;
    int GUIIndex;
};

}

class ManageTextureScreenPrivate : public QDialog, public VspScreenQtPrivate
{
    Q_OBJECT
    Q_DECLARE_PUBLIC( ManageTextureScreen )
    Q_PRIVATE_SLOT( self(), void SetUpdateFlag() )

    Ui::ManageTextureScreen Ui;

    SliderInput m_UPosSlider;
    SliderInput m_WPosSlider;

    SliderInput m_UScaleSlider;
    SliderInput m_WScaleSlider;

    SliderInput m_TransparencySlider;

    ToggleButton m_FlipUButton;
    ToggleButton m_FlipWButton;

    TexDropDownItem * m_SelectedTexItem;
    VSPGUI::VspSubGlWindow m_GlWin;

    std::vector<CompDropDownItem> m_CompDropDownList;
    std::vector<TexDropDownItem> m_TexDropDownList;

    ManageTextureScreenPrivate( ManageTextureScreen * );
    QWidget * widget() Q_DECL_OVERRIDE { return this; }
    bool Update() Q_DECL_OVERRIDE;

    void UpdateCurrentSelected();
    void ResetCurrentSelected();

    Q_SLOT void on_compChoice_currentIndexChanged( int selectedIndex )
    {
        const_foreach( auto item, m_CompDropDownList )
        {
            if( item.GUIIndex == selectedIndex )
            {
                veh()->SetActiveGeom( item.GeomID );
                ResetCurrentSelected();
                break;
            }
        }
    }
    Q_SLOT void on_textureChoice_currentIndexChanged( int )
    {
        UpdateCurrentSelected();
    }
    Q_SLOT void on_textureNameInput_textChanged( const QString & val )
    {
        vector< Geom* > select_vec = veh()->GetActiveGeomPtrVec();
        Texture * info = select_vec[0]->m_GuiDraw.getTextureMgr()->FindTexture( m_SelectedTexItem->TexInfo->GetID() );
        info->SetName( val.toStdString() );
    }
    Q_SLOT void on_addTextureButton_clicked()
    {
        vector< Geom* > select_vec = veh()->GetActiveGeomPtrVec();

        std::string fp = GetScreenMgr()->GetSelectFileScreen()->FileOpen( "Read Texture?", "TGA, JPG Files (*.{tga,jpg})" );
        if ( fp.empty() ) return;
        select_vec[0]->m_GuiDraw.getTextureMgr()->AttachTexture( fp );

        ResetCurrentSelected();
    }
    Q_SLOT void on_delTextureButton_clicked()
    {
        if( m_SelectedTexItem )
        {
            vector< Geom* > select_vec = veh()->GetActiveGeomPtrVec();
            select_vec[0]->m_GuiDraw.getTextureMgr()->RemoveTexture( m_SelectedTexItem->TexInfo->GetID() );

            ResetCurrentSelected();
        }
    }
};
VSP_DEFINE_PRIVATE( ManageTextureScreen )

ManageTextureScreenPrivate::ManageTextureScreenPrivate( ManageTextureScreen * q ) :
    VspScreenQtPrivate( q ),
    m_GlWin( DrawObj::VSP_TEX_PREVIEW )
{
    Ui.setupUi( this );
    move( 775, 50 );

    m_UPosSlider.Init( q, Ui.uPosSlider, Ui.uPosInput, 1, 5, Ui.uPosButton );
    m_WPosSlider.Init( q, Ui.wPosSlider, Ui.wPosInput, 1, 5, Ui.wPosButton );

    m_UScaleSlider.Init( q, Ui.uScaleSlider, Ui.uScaleInput, 1, 5, Ui.uScaleButton );
    m_WScaleSlider.Init( q, Ui.wScaleSlider, Ui.wScaleInput, 1, 5, Ui.wScaleButton );

    m_TransparencySlider.Init( q, Ui.alphaSlider, Ui.alphaInput, 1, 5, Ui.alphaButton );

    m_FlipUButton.Init( q, Ui.flipUButton );
    m_FlipWButton.Init( q, Ui.flipWButton );

    QLayoutItem * spacer = Ui.textureLayout->itemAtPosition( 0, 2 );
    Ui.textureLayout->removeItem(spacer);
    delete spacer;
    Ui.textureLayout->addWidget( &m_GlWin, 0, 2, 5, 1 );
    m_GlWin.setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    ResetCurrentSelected();
    BlockSignalsInUpdates();
    ConnectUpdateFlag();
}

ManageTextureScreen::ManageTextureScreen( ScreenMgr * mgr ) :
    VspScreenQt( * new ManageTextureScreenPrivate( this ), mgr )
{
}

bool ManageTextureScreenPrivate::Update()
{
    vector< Geom* > select_vec = veh()->GetActiveGeomPtrVec();

    if ( select_vec.size() != 1 )
    {
        q_func()->Hide();
        return false;
    }

    // Redo list on each update.
    Ui.compChoice->clear();
    m_CompDropDownList.clear();

    std::vector<Geom *> geomVec = veh()->FindGeomVec( veh()->GetGeomVec() );
    for( int i = 0; i < ( int )geomVec.size(); i++ )
    {
        CompDropDownItem item;
        item.GeomName = geomVec[i]->GetName();
        item.GeomID = geomVec[i]->GetID();

        // Hack to add duplicate names
        Ui.compChoice->addItem( QString::number( i ) );
        item.GUIIndex = Ui.compChoice->count() - 1;

        m_CompDropDownList.push_back( item );
    }
    // Fill Hacked char array with correct names.
    const_foreach( auto item, m_CompDropDownList )
    {
        Ui.compChoice->setItemText( item.GUIIndex, item.GeomName.c_str() );
    }

    // Set compChoice to current selected and update texture dropdown list.
    for( int i = 0; i < ( int )m_CompDropDownList.size(); i++ )
    {
        if( m_CompDropDownList[i].GeomID == select_vec[0]->GetID() )
        {
            Ui.compChoice->setCurrentIndex( m_CompDropDownList[i].GUIIndex );

            // Update Texture Dropdown List. //

            // Redo texture list on each update.
            Ui.textureChoice->clear();
            m_TexDropDownList.clear();

            // Clear preview window.
            VSPGraphic::Viewport * viewport = m_GlWin.getGraphicEngine()->getDisplay()->getViewport();
            assert( viewport );
            viewport->getBackground()->removeImage();

            // Load Textures...
            TextureMgr * texMgr = select_vec[0]->m_GuiDraw.getTextureMgr();
            std::vector<Texture*> texInfos = texMgr->FindTextureVec( texMgr->GetTextureVec() );
            for( int j = 0; j < ( int )texInfos.size(); j++ )
            {
                TexDropDownItem item;
                item.TexInfo = texInfos[j];

                // Hack to add duplicate names
                Ui.textureChoice->addItem( QString::number( j ) );

                m_TexDropDownList.push_back( item );
            }
            // Fill Hacked char array with correct names.
            const_foreach( auto item, m_TexDropDownList )
            {
                Ui.textureChoice->setItemText( item.GUIIndex, item.TexInfo->GetName().c_str() );
            }
            if( !m_TexDropDownList.empty() )
            {
                if( m_SelectedTexItem )
                {
                    Ui.textureChoice->setCurrentIndex( m_SelectedTexItem->GUIIndex );
                }
                else
                {
                    // On refresh list, if nothing is selected, pick last item on list.
                    Ui.textureChoice->setCurrentIndex( Ui.textureChoice->count() - 1 );
                }
                UpdateCurrentSelected();

                assert( m_SelectedTexItem );

                viewport->getBackground()->attachImage( VSPGraphic::GlobalTextureRepo()->
                    get2DTexture( m_SelectedTexItem->TexInfo->m_FileName.c_str() ) );
            }
            else
            {
                ResetCurrentSelected();
            }

            // Update Sliders and Buttons.
            if( m_SelectedTexItem )
            {
                Texture * info = select_vec[0]->m_GuiDraw.getTextureMgr()->FindTexture( m_SelectedTexItem->TexInfo->GetID() );

                Ui.textureNameInput->setText( info->GetName().c_str() );

                m_UScaleSlider.Update( info->m_UScale.GetID() );
                m_WScaleSlider.Update( info->m_WScale.GetID() );

                m_UPosSlider.Update( info->m_U.GetID() );
                m_WPosSlider.Update( info->m_W.GetID() );

                m_TransparencySlider.Update( info->m_Transparency.GetID() );

                m_FlipUButton.Update( info->m_FlipU.GetID() );
                m_FlipWButton.Update( info->m_FlipW.GetID() );
            }
            break;
        }
    }
    m_GlWin.update();
    return true;
}

void ManageTextureScreenPrivate::UpdateCurrentSelected()
{
    m_SelectedTexItem = NULL;

    for( int i = 0; i < ( int )m_TexDropDownList.size(); i++ )
    {
        if( m_TexDropDownList[i].GUIIndex == Ui.textureChoice->currentIndex() )
        {
            m_SelectedTexItem = &m_TexDropDownList[i];
            break;
        }
    }
}

void ManageTextureScreenPrivate::ResetCurrentSelected()
{
    m_SelectedTexItem = NULL;
}

ManageTextureScreen::~ManageTextureScreen() {}

#include "ManageTextureScreen.moc"
