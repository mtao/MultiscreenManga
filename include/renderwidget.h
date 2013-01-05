#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QtOpenGL/QGLWidget>
#include <memory>
#include "mangavolume.h"
#include <mutex>
#include <QtOpenGL/QGLFunctions>
#include <QtOpenGL/QGLShaderProgram>
#include <QtOpenGL/QGLBuffer>
#include <QPointF>
#include <QPoint>


class MainWindow;

class RenderWidget : public QGLWidget, protected QGLFunctions
{
    Q_OBJECT
public:
    enum FIT_MODE{FM_WIDTH, FM_HEIGHT, FM_BEST};
    explicit RenderWidget(std::mutex & mutex
            , uint page_num
            , uint index=0
            , std::shared_ptr<MangaVolume> volume = std::shared_ptr< MangaVolume>()
            , MainWindow *parent = 0
            );
    void setIndex(int index);
    void setMessyCleanup(){m_no_index_cleanup = true;}
    ~RenderWidget();
    
    protected:
    void keyPressEvent(QKeyEvent *);
signals:
    void renderWidgetClosing(uint index);
    void passKeyPressEvent(QKeyEvent *);

public slots:
    void setPage(uint page);
    void setMangaVolume(std::shared_ptr<MangaVolume> volume);

    protected:
    void QCloseEvent();
    void paintGL();
    void initializeGL();
    void resizeGL(int w, int h);
    void rotatePage(int i);
    float color;
    
    private:
    std::weak_ptr<MangaVolume> m_volume;
    uint m_index;
    uint m_page_num;
    GLuint m_page_texture_id;
    std::mutex & m_mutex;
    QPointF m_scale;
    QPoint m_resolution;
    FIT_MODE m_fit_mode;
    uint m_rotation;
    bool m_no_index_cleanup ;
    const GLuint m_vertex_attribute;
    void checkScale();




    private:
    QGLBuffer m_vertexBuffer;
    QGLShaderProgram * m_program;
};

#include <QDebug>
struct RenderWidgetResource
{
    RenderWidget * widget;
    uint index;
    RenderWidgetResource(RenderWidget*widget, uint index): widget(widget), index(index){}
    void decrementIndex()
    {
        index--;
        widget->setIndex(index);
    }
    bool operator<(const RenderWidgetResource & other) const
    {return this->index < other.index;}
    bool operator<=(const RenderWidgetResource & other) const
    {return this->index <= other.index;}

    bool operator==(const RenderWidgetResource & other) const
    {return this->index == other.index;}
    bool operator!=(const RenderWidgetResource & other) const
    {return this->index != other.index;}

    bool operator>(const RenderWidgetResource & other) const
    {return this->index > other.index;}
    bool operator>=(const RenderWidgetResource & other) const
    {return this->index >= other.index;}

};
#endif // RENDERWIDGET_H
