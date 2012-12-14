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
    explicit RenderWidget(
            std::mutex & mutex
            , uint page_num
            , uint index=0
            , std::shared_ptr<const MangaVolume> volume = std::shared_ptr<const MangaVolume>()
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
    void setMangaVolume(std::shared_ptr<const MangaVolume> volume);

    protected:
    void QCloseEvent();
    void paintGL();
    void initializeGL();
    void resizeGL(int w, int h);
    void rotatePage(int i);
    float color=0;
    
    private:
    std::weak_ptr<const MangaVolume> m_volume;
    uint m_index;
    uint m_page_num = 0;
    GLuint m_page_texture_id;
    std::mutex & m_mutex;
    QPointF m_scale;
    QPoint m_resolution;
    FIT_MODE m_fit_mode = FM_BEST;
    uint m_rotation = 0;
    bool m_no_index_cleanup = false;
    const GLuint m_vertex_attribute = 0;
    void checkScale();




    private:
    QGLBuffer m_vertexBuffer;
    QGLShaderProgram * m_program;
};

#include <QDebug>
struct RenderWidgetResource
{
    RenderWidget * widget=0;
    uint index=0;
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
