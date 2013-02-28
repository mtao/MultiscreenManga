#include "include/renderwidget.h"
#include "include/mainwindow.h"
#include <QDebug>
#include <QtOpenGL/QGLShader>
#include <QMatrix4x4>
#include <QKeyEvent>
#include <memory>

RenderWidget::RenderWidget(
        std::mutex & mutex, uint page, uint index,
        std::shared_ptr< MangaVolume> volume, MainWindow *parent)
    : QGLWidget(parent)
    , color(0)
    , m_volume(volume), m_index(index), m_page_num(page+index)
    , m_mutex(mutex)
    , m_fit_mode(FM_BEST), m_rotation(0), m_zoom(false)
    , m_window_size(.3)
    , m_magnification(.1)
    , m_no_index_cleanup(false)
    , m_vertex_attribute(false)
    , m_vertexBuffer(QGLBuffer::VertexBuffer)
    , m_program(new QGLShaderProgram(this)) {
    setAttribute(Qt::WA_DeleteOnClose, true);
    setPage(page);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

void RenderWidget::setMangaVolume(std::shared_ptr<MangaVolume> volume) {
    m_volume = volume;
    setPage(0);
}

void RenderWidget::setPage(uint page) {




    auto volume_ptr = m_volume.lock();

    if (!volume_ptr) {
        return;
    }
    volume_ptr->discardPage(m_page_num);
    m_page_num = page + m_index;
    qDebug() << __FUNCTION__ << m_page_num << "/" << volume_ptr->numPages();
    //qWarning() << "Opening page: " << m_page_num;
    std::shared_ptr<const QImage> img = volume_ptr->getImage(m_page_num);
    if (!img) {
        qWarning() << "Renderwidget " << m_index
                   << " reports that there is no page " << m_page_num;
        QImage * black = new QImage(1,1,QImage::Format_Mono);
        black->setPixel(0,0,0);
        img = std::shared_ptr<const QImage>(black);
    }
    makeCurrent();
    m_resolution = QPoint(img->width(), img->height());
    m_page_texture_id = bindTexture(*img);
    checkScale();

    update();
}

void RenderWidget::initializeGL() {
    glDisable(GL_DEPTH_TEST);
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
        case Qt::Key_Z:
        m_zoom = !m_zoom;
        update();
        update();
        break;
        case Qt::Key_Plus:
        m_magnification -= 0.005;
        m_magnification = std::max(m_magnification,0.01f);
        update();
        break;
        case Qt::Key_Minus:
        m_magnification += 0.005;
        update();
        break;
        case Qt::Key_0:
        m_window_size += 0.005;
        update();
        break;
        case Qt::Key_9:
        m_window_size-= 0.005;
        m_window_size = std::max(m_window_size,0.1f);
        update();
        break;
    default:
        emit passKeyPressEvent(event);
    }
}

void RenderWidget::mousePressEvent(QMouseEvent *event) {
    event->ignore();
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event) {
    float x = (float(width())/height())*2*(float(event->x())/width()-.5);
    float y = 2*(.5-float(event->y())/height());
    m_pos = QPointF(x,y);
    if(m_zoom) {
        update();
    }
    event->ignore();
}

void RenderWidget::wheelEvent(QWheelEvent *event) {
    event->ignore();
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
    if(m_zoom) {
        glLoadIdentity();
        glTranslatef(m_pos.x(),m_pos.y(),0.0f);
        glRotatef(90*m_rotation, 0, 0, 1);
        float imgx = m_pos.x() / m_scale.x()/2+.5;
        float imgy = m_pos.y() / m_scale.y()/2+.5;
        float ratio = m_scale.x() / m_scale.y();
        if(m_rotation % 2 == 1) {
             ratio = 1/ratio;
             float tmp = imgx;
             imgx = imgy;
             imgy = tmp;
             if(m_rotation == 1) {
                 imgy = -imgy;
             } else {
                 imgx = -imgx;
             }
        } else if(m_rotation == 2) {
                imgx = -imgx;
                imgy = -imgy;
        }
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2d(imgx-m_magnification/ratio, imgy-m_magnification);
        glVertex2f(-m_window_size, -m_window_size);
        glTexCoord2d(imgx+m_magnification/ratio, imgy-m_magnification);
        glVertex2f(m_window_size, -m_window_size);
        glTexCoord2d(imgx-m_magnification/ratio, imgy+m_magnification);
        glVertex2f(-m_window_size, m_window_size);
        glTexCoord2d(imgx+m_magnification/ratio, imgy+m_magnification);
        glVertex2f(m_window_size, m_window_size);
        glEnd();
    }
}

void RenderWidget::setIndex(int index) {
    m_index = index;
}

RenderWidget::~RenderWidget() {
    m_mutex.lock();
    emit renderWidgetClosing(m_index);
    m_mutex.unlock();
}
