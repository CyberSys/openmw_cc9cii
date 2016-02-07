#include "scenesubview.hpp"

#include <sstream>
#include <stdlib.h> // FIXME

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <cassert>

#include <extern/esm4/common.hpp> // isFormId

#include "../../model/doc/document.hpp"

#include "../../model/world/cellselection.hpp"

#include "../filter/filterbox.hpp"

#include "../render/foreignworldspacewidget.hpp"
#include "../render/foreigninteriorwidget.hpp"
#include "../render/pagedworldspacewidget.hpp"
#include "../render/unpagedworldspacewidget.hpp"

#include "../widget/scenetoolbar.hpp"
#include "../widget/scenetoolmode.hpp"
#include "../widget/scenetooltoggle.hpp"
#include "../widget/scenetooltoggle2.hpp"
#include "../widget/scenetoolrun.hpp"

#include "tablebottombox.hpp"
#include "creator.hpp"

CSVWorld::SceneSubView::SceneSubView (const CSMWorld::UniversalId& id, CSMDoc::Document& document)
: SubView (id), mScene(NULL), mLayout(new QHBoxLayout), mDocument(document), mToolbar(NULL)
{
    QVBoxLayout *layout = new QVBoxLayout;

    layout->addWidget (mBottom = new TableBottomBox (NullCreatorFactory(), document, id, this), 0);

    mLayout->setContentsMargins (QMargins (0, 0, 0, 0));

    CSVRender::WorldspaceWidget* worldspaceWidget = NULL;
    widgetType whatWidget;

    if (id.getId()=="sys::foreign")
    {
        whatWidget = widget_Paged;

        CSVRender::ForeignWorldspaceWidget *newWidget = new CSVRender::ForeignWorldspaceWidget (this, document);

        worldspaceWidget = newWidget;

        makeConnections(newWidget);
    }
#if 0
    else if (id.getId() == "sys::foreignInterior")
#endif
    else if (ESM4::isFormId(id.getId()))
    {
        // searchId() is not available from CSVRender, so we can't use it to determine whether to
        // create a normal cell widget or a foreign cell widget.  That approach is a little
        // problematic anyway, since an interior cell name may match on both.
        whatWidget = widget_Unpaged;

        CSVRender::ForeignInteriorWidget *newWidget = new CSVRender::ForeignInteriorWidget (id.getId(), this, document);

        worldspaceWidget = newWidget;

        makeConnections(newWidget);
    }
    else if (id.getId()=="sys::default")
    {
        whatWidget = widget_Paged;

        CSVRender::PagedWorldspaceWidget *newWidget = new CSVRender::PagedWorldspaceWidget (this, document);

        worldspaceWidget = newWidget;

        makeConnections(newWidget);
    }
    else
    {
        whatWidget = widget_Unpaged;

        CSVRender::UnpagedWorldspaceWidget *newWidget = new CSVRender::UnpagedWorldspaceWidget (id.getId(), document, this);

        worldspaceWidget = newWidget;

        makeConnections(newWidget);
    }

    replaceToolbarAndWorldspace(worldspaceWidget, makeToolbar(worldspaceWidget, whatWidget));

    layout->insertLayout (0, mLayout, 1);

    CSVFilter::FilterBox *filterBox = new CSVFilter::FilterBox (document.getData(), this);

    layout->insertWidget (0, filterBox);

    QWidget *widget = new QWidget;

    widget->setLayout (layout);

    setWidget (widget);
}

void CSVWorld::SceneSubView::makeConnections (CSVRender::UnpagedWorldspaceWidget* widget)
{
    connect (widget, SIGNAL (closeRequest()), this, SLOT (closeRequest()));

    connect(widget, SIGNAL(dataDropped(const std::vector<CSMWorld::UniversalId>&)),
            this, SLOT(handleDrop(const std::vector<CSMWorld::UniversalId>&)));

    connect(widget, SIGNAL(cellChanged(const CSMWorld::UniversalId&)),
            this, SLOT(cellSelectionChanged(const CSMWorld::UniversalId&)));
}

void CSVWorld::SceneSubView::makeConnections (CSVRender::ForeignInteriorWidget* widget)
{
    connect (widget, SIGNAL (closeRequest()), this, SLOT (closeRequest()));

    connect(widget, SIGNAL(dataDropped(const std::vector<CSMWorld::UniversalId>&)),
            this, SLOT(handleDrop(const std::vector<CSMWorld::UniversalId>&)));

    connect(widget, SIGNAL(cellChanged(const CSMWorld::UniversalId&)),
            this, SLOT(cellSelectionChanged(const CSMWorld::UniversalId&)));
}

void CSVWorld::SceneSubView::makeConnections (CSVRender::PagedWorldspaceWidget* widget)
{
    connect (widget, SIGNAL (closeRequest()), this, SLOT (closeRequest()));

    connect(widget, SIGNAL(dataDropped(const std::vector<CSMWorld::UniversalId>&)),
            this, SLOT(handleDrop(const std::vector<CSMWorld::UniversalId>&)));

    connect (widget, SIGNAL (cellSelectionChanged (const CSMWorld::CellSelection&)),
             this, SLOT (cellSelectionChanged (const CSMWorld::CellSelection&)));
}

void CSVWorld::SceneSubView::makeConnections (CSVRender::ForeignWorldspaceWidget* widget)
{
    connect (widget, SIGNAL (closeRequest()), this, SLOT (closeRequest()));

    connect(widget, SIGNAL(dataDropped(const std::vector<CSMWorld::UniversalId>&)),
            this, SLOT(handleDrop(const std::vector<CSMWorld::UniversalId>&)));

    connect (widget, SIGNAL (cellSelectionChanged (const CSMWorld::CellSelection&)),
             this, SLOT (cellSelectionChanged (const CSMWorld::CellSelection&)));
}

CSVWidget::SceneToolbar* CSVWorld::SceneSubView::makeToolbar (CSVRender::WorldspaceWidget* widget, widgetType type)
{
    CSVWidget::SceneToolbar* toolbar = new CSVWidget::SceneToolbar (48+6, this);

    CSVWidget::SceneToolMode *navigationTool = widget->makeNavigationSelector (toolbar);
    toolbar->addTool (navigationTool);

    CSVWidget::SceneToolMode *lightingTool = widget->makeLightingSelector (toolbar);
    toolbar->addTool (lightingTool);

    CSVWidget::SceneToolToggle2 *sceneVisibilityTool =
        widget->makeSceneVisibilitySelector (toolbar);
    toolbar->addTool (sceneVisibilityTool);

    if (type==widget_Paged)
    {
        CSVRender::PagedWorldspaceWidget *paged = dynamic_cast<CSVRender::PagedWorldspaceWidget *>(widget);
        if (paged != NULL)
        {
            CSVWidget::SceneToolToggle *controlVisibilityTool =
                dynamic_cast<CSVRender::PagedWorldspaceWidget&> (*widget).
                makeControlVisibilitySelector (toolbar);

            toolbar->addTool (controlVisibilityTool);
        }
        else
        {
            CSVWidget::SceneToolToggle *controlVisibilityTool =
                dynamic_cast<CSVRender::ForeignWorldspaceWidget&> (*widget).
                makeControlVisibilitySelector (toolbar);

            toolbar->addTool (controlVisibilityTool);
        }
    }

    CSVWidget::SceneToolRun *runTool = widget->makeRunTool (toolbar);
    toolbar->addTool (runTool);

    CSVWidget::SceneToolMode *editModeTool = widget->makeEditModeSelector (toolbar);
    toolbar->addTool (editModeTool);

    return toolbar;
}

void CSVWorld::SceneSubView::setEditLock (bool locked)
{

}

void CSVWorld::SceneSubView::setStatusBar (bool show)
{
    mBottom->setStatusBar (show);
}

void CSVWorld::SceneSubView::useHint (const std::string& hint)
{
    mScene->useViewHint (hint);
}

std::string CSVWorld::SceneSubView::getTitle() const
{
    return mTitle;
}

void CSVWorld::SceneSubView::cellSelectionChanged (const CSMWorld::UniversalId& id)
{
    setUniversalId(id);
    std::ostringstream stream;
    stream << "Scene: " << getUniversalId().getId();

    mTitle = stream.str();
    setWindowTitle (QString::fromUtf8 (mTitle.c_str()));
    emit updateTitle();
}

// FIXME: this needs update for foreign cells
void CSVWorld::SceneSubView::cellSelectionChanged (const CSMWorld::CellSelection& selection)
{
    setUniversalId(CSMWorld::UniversalId(CSMWorld::UniversalId::Type_Scene, "sys::default"));
    int size = selection.getSize();

    std::ostringstream stream;
    stream << "Scene: " << getUniversalId().getId();

    if (size==0)
        stream << " (empty)";
    else if (size==1)
    {
        stream << " (" << *selection.begin() << ")";
    }
    else
    {
        stream << " (" << selection.getCentre() << " and " << size-1 << " more ";

        if (size>1)
            stream << "cells around it)";
        else
            stream << "cell around it)";
    }

    mTitle = stream.str();
    setWindowTitle (QString::fromUtf8 (mTitle.c_str()));
    emit updateTitle();
}

void CSVWorld::SceneSubView::handleDrop (const std::vector< CSMWorld::UniversalId >& dropData)
{
    CSVRender::PagedWorldspaceWidget* pagedNewWidget = NULL;
    CSVRender::UnpagedWorldspaceWidget* unPagedNewWidget = NULL;
    CSVWidget::SceneToolbar* toolbar = NULL;

    CSVRender::WorldspaceWidget::DropType type = CSVRender::WorldspaceWidget::getDropType (dropData);

    switch (mScene->getDropRequirements (type))
    {
        case CSVRender::WorldspaceWidget::canHandle:
            mScene->handleDrop (dropData, type);
            break;

        case CSVRender::WorldspaceWidget::needPaged:
            pagedNewWidget = new CSVRender::PagedWorldspaceWidget(this, mDocument);
            toolbar = makeToolbar(pagedNewWidget, widget_Paged);
            makeConnections(pagedNewWidget);
            replaceToolbarAndWorldspace(pagedNewWidget, toolbar);
            mScene->handleDrop (dropData, type);
            break;

        case CSVRender::WorldspaceWidget::needUnpaged:
            unPagedNewWidget = new CSVRender::UnpagedWorldspaceWidget(dropData.begin()->getId(), mDocument, this);
            toolbar = makeToolbar(unPagedNewWidget, widget_Unpaged);
            makeConnections(unPagedNewWidget);
            replaceToolbarAndWorldspace(unPagedNewWidget, toolbar);
            cellSelectionChanged(*(dropData.begin()));
            break;

        case CSVRender::WorldspaceWidget::ignored:
            return;
    }
}

void CSVWorld::SceneSubView::replaceToolbarAndWorldspace (CSVRender::WorldspaceWidget* widget, CSVWidget::SceneToolbar* toolbar)
{
    assert(mLayout);

    if (mScene)
    {
        mLayout->removeWidget(mScene);
        mScene->deleteLater();
    }

    if (mToolbar)
    {
        mLayout->removeWidget(mToolbar);
        mToolbar->deleteLater();
    }

    mScene = widget;
    mToolbar = toolbar;

    connect (mScene, SIGNAL (focusToolbarRequest()), mToolbar, SLOT (setFocus()));
    connect (mToolbar, SIGNAL (focusSceneRequest()), mScene, SLOT (setFocus()));

    mLayout->addWidget (mToolbar, 0);
    mLayout->addWidget (mScene, 1);

    mScene->selectDefaultNavigationMode();
    setFocusProxy (mScene);
}
