#include "scenepanel.h"

#include <QLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QDebug>
#include <QTimer>
#include <QShowEvent>
#include <QScrollArea>

#include "IManager.h"
#include "settingsmanager.h"

CScenePanel::CScenePanel(qint32 compkey, QWidget *parent) :
    QWidget(parent),
    _compkey(compkey),
    _sceneWidget(0)
{
    //QTimer::singleShot(100, this, SLOT(onShowSceneWidget())); // TODO: hack
    _sceneWidget = new CSceneWidget(_compkey, this);

}


void CScenePanel::addCamLayer(const QString &sourceName)
{
    addLayer("CAM://" + sourceName);
}

void CScenePanel::addImageLayer(QString fname)
{
    CLayerWidget *lw;
    lw = addLayer("IMAGE://" + fname);
    // TODO: установка флага что это image
}

void CScenePanel::addHtmlRenderLayer(const QString &url)
{
    qDebug() << "CScenePanel::addHtmlRenderLayer: url: " << url;
    addLayer("HTML://" + url);
}


void CScenePanel::addSubSceneLayer()
{
    CLayerWidget *lw;
    lw = addLayer("SUBSCENE://");
    // TODO: установка флага что это подсцена
}

void CScenePanel::addScreenCaptureLayer(const QString &rect)
{
    qDebug() << "CScenePanel::addScreenCaptureLayer: rect: " << rect;
    addLayer("SCREEN://" + rect);
}

CLayerWidget* CScenePanel::addLayer(const QString &sourceName)
{
    int zorder = 10*(_listLayerWidgets.count() + 1); // в микшер слои добавляем поверх друг друга
    int layer_compkey;

    CLayerWidget::LayerType lType = CLayerWidget::ELayerTypeDefault;

    if(sourceName.startsWith("SUBSCENE")){
        lType = CLayerWidget::ELayerTypeSUBSCENE;
        layer_compkey = global_manager->addScene();
    }else{
        layer_compkey = global_manager->addLayer(_sceneWidget->getCompkey(), sourceName.toLocal8Bit().data(), zorder);
    }

    CLayerWidget *lw = new CLayerWidget(layer_compkey, lType, this);
    connect(lw, SIGNAL(editLayer(qint32)), SLOT(onEditLayer(qint32)));
    connect(lw, SIGNAL(ultimateShow()), SLOT(onUltimateShow()));
    _listLayerWidgets.append(lw);
    rePosition();

    return lw;
}

void CScenePanel::onEditLayer(qint32 compkey)
{
    _sceneWidget->toggleBox(compkey);
}

void CScenePanel::onUltimateShow()
{
    if(CLayerWidget *curLW = qobject_cast<CLayerWidget*>(sender()))
    {
        QListIterator<CLayerWidget*> it(_listLayerWidgets);

        while (it.hasNext())
        {
            CLayerWidget *lw = it.next();
            
            if(curLW->compKey() == lw->compKey())
            {
                lw->setVisibleHide(true);
                continue;
            }

            if(lw->isPinEnable() || !lw->isVisibleHide())
                continue;

            lw->setVisibleHide(false);
        }
    }
}

void CScenePanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    rePosition();
}


void CScenePanel::rePosition()
{
    int cols, rows;
    int lwsize = _listLayerWidgets.size();

    // определяем кол-во столбцов и строк
    // 0,1 - один столбец
    // 2,3,4 - два столбца
    // > 5   - три столбца
    cols = 3; // default
    if(lwsize < 5){
        cols = lwsize < 2 ? 1:2;
    }

    rows =  lwsize / cols;
    rows = rows == 0? 1: rows;
    if(cols*rows < lwsize)
        rows += 1;

    // делим родительскую ширину между сценой и сеткой слоёв пополам.
    // и высоту сетки делим между слоями.
    int w,h, sw, sh, sx, sy, zw, zh;
    w = this->width();
    h = this->height();


    // параметры сетки
    zh = 7; // высота зазора между строками
    zw = 7; // ширина зазора между столбцами
    sw = (w/2 - (cols)*zw)/cols; // ширина ячейки с учётом зазоров
    sh = (h - (rows-1)*zh)/rows; //  высота ячейки с учётом зазоров

    sx = w/2;
    sy = 0;

    if(_sceneWidget != 0)
    {
        _sceneWidget->setGeometry(0, 0, w/2, h);
    }
    //_sceneWidget->setSceneRect(0,0,w/2,h);

    for(int i=0; i<_listLayerWidgets.size(); i++){
        if( i>0 && i % cols == 0 ){
            sy += sh + zh;
            sx = w/2;
        }

        sx += zw; // вставка зазора между столбцами

        _listLayerWidgets[i]->setGeometry(sx, sy, sw, sh);
        _listLayerWidgets[i]->show();
        sx += sw;
    }

}

void CScenePanel::start()
{
    if(_sceneWidget != 0)
    {
        _sceneWidget->start();
        _sceneWidget->startBox();
    }

    QListIterator<CLayerWidget*> it(_listLayerWidgets);

    while(it.hasNext())
    {
        it.next()->start();
    }
}

void CScenePanel::stop()
{
    if(_sceneWidget != 0)
    {
        _sceneWidget->stop();
        _sceneWidget->stopBox();
    }

    QListIterator<CLayerWidget*> it(_listLayerWidgets);

    while(it.hasNext())
    {
        it.next()->stop();
    }
}

void CScenePanel::applySetting()
{
    SettingsManager setting("Video");
    bool isEnabledOpenGL = setting.getBoolValue("OpenGL");
    //_sceneWidget->setEnabledOpenGl(isEnabledOpenGL);

    QListIterator<CLayerWidget*> it(_listLayerWidgets);
    while(it.hasNext())
    {
        //it.next()->setEnabledOpenGl(isEnabledOpenGL);
    }
}







