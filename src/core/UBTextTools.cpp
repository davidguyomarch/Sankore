#include <QDebug>
#include "UBTextTools.h"

QString UBTextTools::cleanHtmlCData(const QString &_html){

    QString clean = "";


    for(int i = 0; i < _html.length(); i+=1){
        if(_html.at(i) != '\0')
            clean.append(_html.at(i));
    }
    return clean;
}


QString UBTextTools::cleanHtml(const QString& _html)
{
    const QString START_TAG = "<body";
    const QString END_TAG = "</body";

    QString cleanSource = "";
    QString simplifiedHtml = _html;
    const QString lowerHtml = simplifiedHtml.toLower();

    int start = lowerHtml.indexOf(START_TAG);
    int endTag = lowerHtml.indexOf(END_TAG);

    if (start != -1 && endTag != -1) {
        // endTag is the index of "</body"; advance past it so the closing tag
        // name is included, then slice [start, end) using a *length*, not an
        // absolute index (issue #228).
        int end = endTag + END_TAG.size();
        cleanSource = simplifiedHtml.mid(start, end - start);
    } else {
        cleanSource = _html;
    }

    return cleanSource;
}

void UBTextTools::cleanHtmlClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if(mimeData->hasHtml()){
        QMimeData *myMime = new QMimeData();
        QTextDocument doc;
        doc.setHtml(mimeData->html());

        QString cleanHtml = doc.toPlainText();

        myMime->setHtml(cleanHtml);
        clipboard->setMimeData(myMime);
    }
}
