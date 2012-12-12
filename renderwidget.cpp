#include "renderwidget.h"
#include "mainwindow.h"
#include <QDebug>
#include <QtOpenGL/QGLShader>


RenderWidget::RenderWidget(std::mutex & mutex, uint page, uint index, std::shared_ptr<const MangaVolume> volume, MainWindow *parent) :
    QGLWidget(parent), m_volume(volume), m_index(index), m_page_num(page+index), m_mutex(mutex)
  , m_vertexBuffer(QGLBuffer::VertexBuffer), m_program(new QGLShaderProgram(this))
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setPage(m_page_num);
}

void RenderWidget::setMangaVolume(std::shared_ptr<const MangaVolume> volume)
{
    m_volume = volume;
    setPage(0);
}

void RenderWidget::setPage(uint page)
{
    m_page_num = page + m_index;
    auto volume_ptr = m_volume.lock();
    qWarning() << "Openning page: " << page;
    if(!volume_ptr)
        return;

    const QImage & img = volume_ptr->getImage(m_page_num);
    if(img.isNull())
    {
        qWarning() << "Index " << m_index << " reports that there is no page " << m_page_num;
        return;
    }
    makeCurrent();
    m_page_texture_id = bindTexture(img);

}

void RenderWidget::initializeGL()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    qglClearColor(QColor(0,0,0));



}
void RenderWidget::resizeGL(int w, int h)
{
    glViewport(0,0,w,h);
}

void RenderWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW_MATRIX);
    glLoadIdentity();
    glScalef(m_scale,m_scale,1.0);

    glBindTexture(GL_TEXTURE_2D, m_page_texture_id);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2d(0.0,0.0);    glVertex2f(-1,-1);
    glTexCoord2d(1.0,0.0);    glVertex2f(1,-1);
    glTexCoord2d(0.0,1.0);    glVertex2f(-1,1);
    glTexCoord2d(1.0,1.0);    glVertex2f(1,1);
    glEnd();
}

void RenderWidget::setIndex(int index)
{
    m_index = index;
}

RenderWidget::~RenderWidget()
{
    m_mutex.lock();
    emit renderWidgetClosing(m_index);
    m_mutex.unlock();
}
