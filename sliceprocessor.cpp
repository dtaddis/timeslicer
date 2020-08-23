#include "sliceprocessor.h"

#include <QDebug>
#include <QImage>
#include <QColor>
#include "math.h"

#define DEG2RAD  0.01745329251
#define RAD2DEG  (1.0 / DEG2RAD)


SliceProcessor::SliceProcessor(QObject *parent) :
    QThread(parent)
{
    preview = true;
    blending = 1.0f;
    reverse = false;
    angle = 40.0f;
    scale_x = 1.0f;
    scale_y = 1.0f;
    slice_type = SliceType::Linear;
    radial_coverage = 180.0f;
    radial_start = -90.0f;
    origin_x = 0.5f;
    origin_y = 0.0f;
    grid_rows = 2;
}

// QSize thumbnail = size.scaled(200, 200, Qt::KeepAspectRatio);

float SliceProcessor::blend(float pos) {

    if (blending < 0.01) {
        return pos < 1 ? 1 : 0;
    }

    float s = 0.5 - (abs(pos) - 1) * (1. / (2. * blending));

    // Clamp value between 0 and 1
    if (s < 0.0)
        s = 0.0;
    else if (s > 1.0)
        s = 1.0;

    // Reserve luminance
    s /= sqrt(s * s + (1 - s) * (1 - s));

    return s;
}




void SliceProcessor::run() {

    if (!preview)
        emit info("Started");

    if (images.size() < 2) {
        emit error("Too few images!");
        return;
    }
    blending = 1.0f;

    QImage output;

    if (preview) {

        // TODO: Better checking here!
        if (thumbs.empty()) {
            qDebug() << "Generating smaller images for preview...";

            if (images.size() > 20) {
                // TODO: drop the number of images if they are
            }

            for (int i = 0; i < images.size(); i++) {
                QImage im(images[i]);
                thumbs.append(im.scaled(800, 800, Qt::KeepAspectRatio));
            }
        }
        output = QImage(thumbs[0].size(), QImage::Format_RGB32);

    }
    else {

        qDebug() << "Starting final rendering..";
        qDebug() << images.size() << "images to be procesed!";

        output = QImage(images[0]);
    }





    // Rotation
    float sn = sin(DEG2RAD * angle);
    float cn = cos(DEG2RAD * angle);

    // Radial values
    float segment_angle = radial_coverage / images.size();

    // Grid values
    int grid_columns = images.size() / grid_rows;
    float pixels_per_column = output.width() / grid_columns;
    float pixels_per_row = output.height() / grid_rows;

    QImage layer;

    int i, j;
    for (int p = 0; p < images.size(); ++p) {

        // Layer indexing
        int pi = reverse ? (images.size() - p - 1) : p;

        // Radial segment
        float this_segment_min = pi * segment_angle;
        float this_segment_max = (pi + 1) * segment_angle;

        if (preview) {
            layer = thumbs[pi];
        }
        else {
            // Load full resolution image
            qDebug() << "Processing" << images[pi];
            layer = QImage(images[pi]);

            if (layer.size() != output.size()) {
                emit error(QString("Image %s is wrong size!").arg(images[p]));
                return;
            }
        }

        float origin_px = origin_x * output.width();
        float origin_py = origin_y * output.height();

        QColor r, o;

        for (i = 0; i < output.width(); ++i) {
          for (j = 0; j < output.height(); ++j) {

            float b = 1;

            if (slice_type == SliceType::Linear)
            {
              // To [-1; 1] space
              float ox = (float)i / output.width() - 0.5;
              float oy = (float)j / output.height() - 0.5;

              // Transform (rotation + scaling)
              float cx = scale_x * cn * ox - scale_x * sn * oy;
              float cy = scale_y * sn * ox + scale_y * cn * oy;

              // Convert back to pixel space
              int px = (cx + 0.5) * output.width();
              int py = (cy + 0.5) * output.height();

              // Fiqure out should we copy the pixel to the final version or not
              /*if (px < int(w*p) + 1 * oy && p > 0)
                  continue;
              if (px < int(w*(p+1)) + 1 + 1 * oy && p != images.size())
                  continue;*/

              if (pi == 0 && cx < -0.5) {

              }
              else if (pi == images.size() - 1 && cx > 0.5) {

              }
              else {

                b = blend(2 * ((images.size() - 1) * (cx + 0.5) - pi - 1));
                if (b < 0.01)
                  continue;
              }
            }
            else if (slice_type == SliceType::Radial)
            {
              int px = i;
              int py = output.height() - 1 - j;

              float opp = (float)px - origin_px;
              float adj = (float)py - origin_py;
              float angle = RAD2DEG * atan2(opp, adj);

              // Allow to start from any angle
              angle -= radial_start;

              // Wrap angle
              if (angle < 0.0f) {
                angle += 360.0f;
              }
              else if (angle > 360.0f)
              {
                angle -= 360.0f;
              }

              if (angle < this_segment_min) {
                continue;
              }
              else if (angle > this_segment_max) {
                continue;
              }
            }
            else if (slice_type == SliceType::Grid)
            {
              int px = i;
              int py = j;

              // Find out which cell we are in
              int column = px / pixels_per_column;
              int row = py / pixels_per_row;

              // Find out which cell is valid for this image
              int this_image_row = pi / grid_columns;
              int this_image_column = pi - (this_image_row * grid_columns);

              if ((row != this_image_row) || (column != this_image_column)) {
                continue;
              }
            }

            // Sample the layer
            r = layer.pixel(i, j);

            if (1) {
              output.setPixel(i, j, r.rgb());
            }
            else {
              //b = 0.3;
              o = output.pixel(i, j);
              output.setPixel(i, j, qRgb(o.red() + b * r.red(), o.green() + b * r.green(), o.blue() + b * r.blue()));
            }

          }
        }


        if (preview) {
            preview_image = output.copy();
            emit preview_ready();
        }
        else {
            emit progress(p);
        }

        if (isInterruptionRequested()) {
            return;
        }

    }


    if (preview) {

    }
    else {
        qDebug() << "Exporting \"time.jpg\"";
        output.save("time.jpg", "jpg", 90);
        emit info("Done!");
    }

}
