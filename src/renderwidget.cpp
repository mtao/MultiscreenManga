#include "include/renderwidget.h"
#include "include/mainwindow.h"
#include <QDebug>
#include <QtOpenGL/QGLShader>
#include <QMatrix4x4>
#include <QKeyEvent>
#include <memory>

RenderWidget::RenderWidget(
        std::mutex & mutex, uint page, uint index,
        std::shared_ptr<const MangaVolume> volume, MainWindow *parent)
    : QGLWidget(parent)
    , color(0)
    , m_volume(volume), m_index(index), m_page_num(page+index)
    , m_mutex(mutex)
    , m_fit_mode(FM_BEST), m_rotation(0), m_no_index_cleanup(false)
    , m_vertex_attribute(false)
    , m_vertexBuffer(QGLBuffer::VertexBuffer)
    , m_program(new QGLShaderProgram(this)) {
    setAttribute(Qt::WA_DeleteOnClose, true);
    setPage(page);
    setFocusPolicy(Qt::StrongFocus);
}

void RenderWidget::setMangaVolume(std::shared_ptr<const MangaVolume> volume) {
    m_volume = volume;
    setPage(0);
}

void RenderWidget::setPage(uint page) {
    qDebug() << __FUNCTION__ << page;
    m_page_num = page + m_index;
    auto volume_ptr = m_volume.lock();
    qWarning() << "Opening page: " << page;
    if (!volume_ptr) {
        return;
    }

    std::shared_ptr<const QImage> img = volume_ptr->getImage(m_page_num);
    if (!img) {
        qWarning() << "Index " << m_index
                   << " reports that there is no page " << m_page_num;
        return;
    }
    makeCurrent();
    m_resolution = QPoint(img->width(), img->height());
    m_page_texture_id = bindTexture(*img);
    checkScale();

    qWarning() << "Done loading page";
    update();
}

void RenderWidget::initializeGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    qglClearColor(QColor(0, 0, 0));
}

void RenderWidget::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case  Qt::Key_F:
        if (m_index == 0) {
            emit passKeyPressEvent(event);
        } else {
            if (isFullScreen()) {
                showNormal();
            } else {
                showFullScreen();
            }
        }
        break;
    case Qt::Key_R:
        if (event->modifiers() & Qt::ShiftModifier) {
            rotatePage(-1);
        } else {
            rotatePage(1);
        }
        break;
    default:
        emit passKeyPressEvent(event);
    }
}

void RenderWidget::rotatePage(int i) {
    m_rotation = (m_rotation + i) % 4;
    checkScale();

    update();
}

void RenderWidget::checkScale() {
    float image_ratio = 1;
    if (m_rotation % 2 == 0) {
        image_ratio = m_resolution.x() / static_cast<float>(m_resolution.y());
    } else {
        image_ratio = m_resolution.y() / static_cast<float>(m_resolution.x());
    }
    float window_ratio = width() / static_cast<float>(height());
    switch (m_fit_mode) {
    case FM_HEIGHT:
        m_scale = QPointF(1.0*image_ratio, 1.0);
        break;
    case FM_WIDTH:
        m_scale = QPointF(window_ratio, window_ratio/image_ratio);
        break;
    case FM_BEST:
        if (window_ratio /image_ratio > 1) {
            m_scale = QPointF(1.0*image_ratio, 1.0);
        } else {
            m_scale = QPointF(window_ratio, window_ratio/image_ratio);
        }
    }
}

void RenderWidget::resizeGL(int w, int h) {
    qreal ratio = qreal(w)/h;
    glViewport(0, 0, w, h);
    QMatrix4x4 projection;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    projection.ortho(-ratio, ratio, -1, 1, -1, 1);
    glMultMatrixd(projection.data());
    checkScale();
    if (auto vol_ptr = m_volume.lock()) {
        if(vol_ptr->refreshOnResize()) {
            vol_ptr->getImage(m_page_num, m_scale);
        }
    }
}

void RenderWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (m_volume.expired()) {
        return;
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(m_scale.x(), m_scale.y(), 1.0);
    glRotatef(90*m_rotation, 0, 0, 1);

    glBindTexture(GL_TEXTURE_2D, m_page_texture_id);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2d(0.0, 0.0);    glVertex2f(-1, -1);
    glTexCoord2d(1.0, 0.0);    glVertex2f(1, -1);
    glTexCoord2d(0.0, 1.0);    glVertex2f(-1, 1);
    glTexCoord2d(1.0, 1.0);    glVertex2f(1, 1);
    glEnd();
}

void RenderWidget::setIndex(int index) {
    m_index = index;
}

RenderWidget::~RenderWidget() {
    m_mutex.lock();
    emit renderWidgetClosing(m_index);
    m_mutex.unlock();
}
