// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "splashscreen.h"
#include "clientversion.h"
#include "util.h"

#include <QPainter>
#undef loop /* ugh, remove this when the #define loop is gone from util.h */
#include <QApplication>

SplashScreen::SplashScreen(const QPixmap &pixmap, Qt::WindowFlags f) :
    QSplashScreen(pixmap, f)
{
    const int splashWidth  = 397;
    const int splashHeight = 397;

    // set reference point, paddings
    float fontFactor            = 1.0;

    // define text to place
    QString titleText       = QString(QApplication::applicationName()).replace(QString("-testnet"), QString(""), Qt::CaseSensitive); // cut of testnet, place it as single object further down
    QString splashTitle     = QString("e-Gulden Core");
    QString versionText     = QString("%1").arg(QString::fromStdString(FormatFullVersion()));

    QString font            = "Arial";

    // load the bitmap for writing some text over it
    QPixmap newPixmap;
    if(GetBoolArg("-testnet")) {
        newPixmap     = QPixmap(":/images/splash_testnet");
    }
    else {
        newPixmap     = QPixmap(":/images/splash");
    }

    QPainter pixPaint(&newPixmap);
    pixPaint.setPen(QColor(70,70,70));

    pixPaint.setFont(QFont(font, 18 * fontFactor));
    pixPaint.drawText(0, 20,      splashWidth, 25, Qt::AlignCenter, splashTitle);

    pixPaint.setFont(QFont(font, 8 * fontFactor));
    pixPaint.drawText(0, 20 + 25, splashWidth, 20, Qt::AlignCenter, versionText);

    pixPaint.end();

    this->setPixmap(newPixmap);
}
