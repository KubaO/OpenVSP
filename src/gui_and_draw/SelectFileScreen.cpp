//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SelectFileScreen.cpp: implementation of the SelectFileScreen class.
//
//////////////////////////////////////////////////////////////////////

#define QPoint QQPoint
#include "SelectFileScreen.h"
#include <FL/Fl_Preferences.H>
#undef QPoint
#include <QFileDialog>
#include <string>
using std::string;

/* Note: QFileDialog saves its favorite list automatically in QSettings.
 * We do it ourselves, nevertheless. */
class SelectFileScreen::Private {
    Q_DISABLE_COPY( Private )
public:
    VSP_DECLARE_PUBLIC( SelectFileScreen )
    SelectFileScreen* const q_ptr;

    QFileDialog Window;

    Private(SelectFileScreen*);
    void LoadFavorites();
    void SaveFavorites();
    string FileChooser( const char * title, const char * filter, const char * dir = 0);
};
VSP_DEFINE_PRIVATE( SelectFileScreen )

SelectFileScreen::SelectFileScreen() :
    d_ptr(new SelectFileScreen::Private(this))
{
    V_D(SelectFileScreen);
    d->Window.setDirectory(QDir::currentPath());
    d->Window.setNameFilter("(*.*)");;
    d->LoadFavorites();
}

void SelectFileScreen::Private::LoadFavorites()
{
    QList<QUrl> favorites;
    Fl_Preferences prefs( Fl_Preferences::USER, "NASA", "VSP" );
    Fl_Preferences favs( prefs, "favorites" );

    char str[64];
    for ( int i = 0; i < favs.entries(); ++i )
    {
        if ( favs.get( favs.entry(i), str, "", 256 ) )
        {
            favorites << QUrl::fromLocalFile(str);
        }
    }
    Window.setSidebarUrls(favorites);
    prefs.flush();
}

void SelectFileScreen::Private::SaveFavorites()
{
    QList<QUrl> favorites = Window.sidebarUrls();
    Fl_Preferences prefs( Fl_Preferences::USER, "NASA", "VSP" );
    Fl_Preferences favs( prefs, "favorites" );

    favs.deleteAllEntries();
    char favstr[64];
    for (int i = 0; i < favorites.size(); ++ i)
    {
        sprintf( favstr, "fav%d", static_cast<int>( i ) );
        favs.set( favstr, favorites[i].toLocalFile().toLocal8Bit().constData() );
    }
    prefs.flush();
}

string SelectFileScreen::Private::FileChooser( const char* title, const char* filter, const char * dir )
{
    string file_name;

    if (dir) Window.setDirectory( dir );
    QString filter_str = QString("  (%1)").arg(filter);
    QString title_str = QString("%1%2").arg(title).arg(filter_str);

    Window.setWindowTitle( title_str );
    Window.setNameFilter( filter_str );
    Window.setOption( QFileDialog::DontUseNativeDialog );

    Window.exec();
    if (Window.result() == QDialog::Accepted)
    {
        SaveFavorites();
    }

    QStringList const files = Window.selectedFiles();
    if (! files.isEmpty() )
    {
        file_name = files.first().toStdString();
    }
    return file_name;
}

string SelectFileScreen::FileOpen( const char* title, const char* filter, const char* dir )
{
    V_D(SelectFileScreen);
    d->Window.setFileMode( QFileDialog::ExistingFile );
    d->Window.setAcceptMode( QFileDialog::AcceptOpen );
    return d->FileChooser( title, filter, dir );
}

string SelectFileScreen::FileSave( const char* title, const char* filter, const char* dir )
{
    V_D(SelectFileScreen);
    d->Window.setFileMode( QFileDialog::AnyFile );
    d->Window.setAcceptMode( QFileDialog::AcceptSave );
    return d->FileChooser( title, filter, dir );
}

SelectFileScreen::~SelectFileScreen()
{}

SelectFileScreen::Private::Private(SelectFileScreen * parent) :
    q_ptr(parent)
{}
