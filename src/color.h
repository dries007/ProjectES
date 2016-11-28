//
// Created by dries on 27/11/16.
//

#ifndef PROJECT_COLOR_H
#define PROJECT_COLOR_H

typedef struct {
    double r;       // percent
    double g;       // percent
    double b;       // percent
} ColorRGB;

typedef struct {
    double h;       // angle in degrees
    double s;       // percent
    double v;       // percent
} ColorHSV;

ColorRGB hsv2rgb(ColorHSV in);

#endif //PROJECT_COLOR_H
