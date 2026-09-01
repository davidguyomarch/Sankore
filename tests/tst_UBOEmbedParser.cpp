/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBOEmbedParser.h"
#include "web/UBOEmbedUtils.h"
#include "web/UBOEmbedParser.h"

#include <QSignalSpy>

#include <new>
#include <cstring>

Q_DECLARE_METATYPE(sOEmbedContent)
Q_DECLARE_METATYPE(QVector<sOEmbedContent>)

void TestUBOEmbedParser::testGetJSONInfos_video()
{
    QString json = R"({
        "provider_url": "http://www.youtube.com/",
        "title": "My Video Title",
        "html": "<iframe src=\"http://www.youtube.com/embed/ABC123\" frameborder=\"0\"></iframe>",
        "author_name": "TestAuthor",
        "height": 270,
        "thumbnail_width": 480,
        "width": 480,
        "version": "1.0",
        "author_url": "http://www.youtube.com/user/TestAuthor",
        "provider_name": "YouTube",
        "thumbnail_url": "http://i4.ytimg.com/vi/ABC123/hqdefault.jpg",
        "type": "video",
        "thumbnail_height": 360
    })";

    sOEmbedContent content = UBOEmbedUtils::parseJSON(json);

    QCOMPARE(content.providerUrl, QString("http://www.youtube.com/"));
    QCOMPARE(content.title, QString("My Video Title"));
    QCOMPARE(content.author, QString("TestAuthor"));
    QCOMPARE(content.height, 270);
    QCOMPARE(content.width, 480);
    QCOMPARE(content.thumbWidth, 480);
    QCOMPARE(content.providerName, QString("YouTube"));
    QCOMPARE(content.type, QString("video"));
    QCOMPARE(content.url, QString("http://www.youtube.com/embed/ABC123"));
}

void TestUBOEmbedParser::testGetJSONInfos_photo()
{
    QString json = R"({
        "provider_url": "http://www.flickr.com/",
        "title": "A Photo",
        "type": "photo",
        "url": "http://farm4.static.flickr.com/photo.jpg",
        "width": 1024,
        "height": 768,
        "version": "1.0",
        "provider_name": "Flickr",
        "author_name": "Photographer",
        "thumbnail_url": "http://farm4.static.flickr.com/thumb.jpg",
        "thumbnail_width": 100,
        "thumbnail_height": 75
    })";

    sOEmbedContent content = UBOEmbedUtils::parseJSON(json);

    QCOMPARE(content.type, QString("photo"));
    QCOMPARE(content.url, QString("http://farm4.static.flickr.com/photo.jpg"));
    QCOMPARE(content.width, 1024);
    QCOMPARE(content.height, 768);
    QCOMPARE(content.providerName, QString("Flickr"));
}

void TestUBOEmbedParser::testGetJSONInfos_emptyJson()
{
    sOEmbedContent content = UBOEmbedUtils::parseJSON("{}");

    QCOMPARE(content.title, QString(""));
    QCOMPARE(content.width, 0);
    QCOMPARE(content.height, 0);
    QCOMPARE(content.type, QString(""));
}

void TestUBOEmbedParser::testGetXMLInfos_video()
{
    QString xml = R"(
        <oembed>
            <provider_url>http://www.youtube.com/</provider_url>
            <title>XML Video Test</title>
            <html>&lt;iframe src=&quot;http://www.youtube.com/embed/XYZ789&quot; frameborder=&quot;0&quot;&gt;&lt;/iframe&gt;</html>
            <author_name>XMLAuthor</author_name>
            <height>360</height>
            <thumbnail_width>480</thumbnail_width>
            <width>640</width>
            <version>1.0</version>
            <author_url>http://www.youtube.com/user/XMLAuthor</author_url>
            <provider_name>YouTube</provider_name>
            <thumbnail_url>http://i4.ytimg.com/vi/XYZ789/hqdefault.jpg</thumbnail_url>
            <type>video</type>
            <thumbnail_height>360</thumbnail_height>
        </oembed>
    )";

    sOEmbedContent content = UBOEmbedUtils::parseXML(xml);

    QCOMPARE(content.providerUrl, QString("http://www.youtube.com/"));
    QCOMPARE(content.title, QString("XML Video Test"));
    QCOMPARE(content.author, QString("XMLAuthor"));
    QCOMPARE(content.height, 360);
    QCOMPARE(content.width, 640);
    QCOMPARE(content.providerName, QString("YouTube"));
    QCOMPARE(content.type, QString("video"));
    QVERIFY(content.url.contains("youtube.com/embed/XYZ789"));
}

void TestUBOEmbedParser::testGetXMLInfos_photo()
{
    QString xml = R"(
        <oembed>
            <title>A Photo</title>
            <type>photo</type>
            <url>http://example.com/photo.jpg</url>
            <width>800</width>
            <height>600</height>
            <provider_name>Example</provider_name>
            <version>1.0</version>
        </oembed>
    )";

    sOEmbedContent content = UBOEmbedUtils::parseXML(xml);

    QCOMPARE(content.type, QString("photo"));
    QCOMPARE(content.url, QString("http://example.com/photo.jpg"));
    QCOMPARE(content.width, 800);
    QCOMPARE(content.height, 600);
}

void TestUBOEmbedParser::testGetXMLInfos_emptyXml()
{
    sOEmbedContent content = UBOEmbedUtils::parseXML("<oembed></oembed>");

    QCOMPARE(content.title, QString(""));
    QCOMPARE(content.width, 0);
    QCOMPARE(content.height, 0);
}

void TestUBOEmbedParser::testGetJSONInfos_partialData()
{
    QString json = R"({
        "title": "Partial",
        "type": "rich",
        "html": "<div>content</div>"
    })";

    sOEmbedContent content = UBOEmbedUtils::parseJSON(json);

    QCOMPARE(content.title, QString("Partial"));
    QCOMPARE(content.type, QString("rich"));
    QCOMPARE(content.width, 0);
    QCOMPARE(content.providerUrl, QString(""));
}

void TestUBOEmbedParser::testGetXMLInfos_allFields()
{
    QString xml = R"(
        <oembed>
            <provider_url>http://vimeo.com/</provider_url>
            <title>Complete Test</title>
            <html>&lt;iframe src=&quot;http://player.vimeo.com/video/123&quot;&gt;&lt;/iframe&gt;</html>
            <author_name>VimeoUser</author_name>
            <height>480</height>
            <thumbnail_width>640</thumbnail_width>
            <width>854</width>
            <version>1.0</version>
            <author_url>http://vimeo.com/user123</author_url>
            <provider_name>Vimeo</provider_name>
            <thumbnail_url>http://i.vimeocdn.com/video/thumb.jpg</thumbnail_url>
            <type>video</type>
            <thumbnail_height>480</thumbnail_height>
        </oembed>
    )";

    sOEmbedContent content = UBOEmbedUtils::parseXML(xml);

    QCOMPARE(content.providerUrl, QString("http://vimeo.com/"));
    QCOMPARE(content.title, QString("Complete Test"));
    QCOMPARE(content.author, QString("VimeoUser"));
    QCOMPARE(content.authorUrl, QString("http://vimeo.com/user123"));
    QCOMPARE(content.height, 480);
    QCOMPARE(content.width, 854);
    QCOMPARE(content.thumbWidth, 640);
    QCOMPARE(content.thumbUrl, QString("http://i.vimeocdn.com/video/thumb.jpg"));
    QCOMPARE(content.thumbHeight, QString("480"));
    QCOMPARE(content.providerName, QString("Vimeo"));
    QCOMPARE(content.version, 1.0f);
}

// --- Regression tests for #229 ---------------------------------------------
//
// UBOEmbedParser's constructor used to leave mpNam (QNetworkAccessManager*) and
// mPending (int) uninitialised. When parse() discovered an oembed <link> before
// setNetworkAccessManager() had been called, it emitted parseContent, whose slot
// onParseContent() checks `nullptr != mpNam` and then calls mpNam->get(). With
// an uninitialised pointer the null guard passes on garbage and mpNam->get()
// dereferences invalid memory -> crash.
//
// Making the reproduction deterministic: constructing a UBOEmbedParser on a
// fresh stack often reads mpNam as 0x0 by luck, so a naive test passes even on
// the buggy code (undefined behaviour is not guaranteed to crash). We therefore
// placement-new the parser into a buffer pre-filled with a non-zero poison
// pattern. On the buggy constructor mpNam keeps the poison value, the guard
// passes, and mpNam->get() dereferences the poison -> deterministic crash. The
// fixed constructor overwrites mpNam with nullptr, so the guard correctly skips
// the get() call and the test passes.
void TestUBOEmbedParser::testParseWithoutNamDoesNotCrash()
{
    // Poisoned storage: every byte non-zero so any uninitialised pointer member
    // reads as a non-null (invalid) address.
    alignas(UBOEmbedParser) unsigned char storage[sizeof(UBOEmbedParser)];
    memset(storage, 0xEF, sizeof(storage));

    UBOEmbedParser* parser = new (storage) UBOEmbedParser();

    // HTML with two <link> elements, the second being an oembed discovery link.
    // parse() skips the first captured result and inspects the rest, so we need
    // at least two <link> tags for the oembed one to be considered.
    const QString html =
        "<html><head>"
        "<link rel=\"stylesheet\" href=\"style.css\">"
        "<link rel=\"alternate\" type=\"application/json+oembed\" href=\"http://example.com/oembed?format=json\">"
        "</head></html>";

    // No setNetworkAccessManager(): with the fix mpNam is nullptr and the guard
    // in onParseContent() skips mpNam->get(). With the bug mpNam is the poison
    // pattern (non-null), so mpNam->get() dereferences garbage -> SIGSEGV.
    parser->parse(html);

    // Reaching this point means the uninitialised-pointer dereference did not
    // happen (i.e. the constructor initialised mpNam to nullptr).
    QVERIFY(true);

    parser->~UBOEmbedParser();
}

void TestUBOEmbedParser::testConstructorInitialisesMembers()
{
    // With no oembed links, mPending must be 0 and parse() must synchronously
    // emit oembedParsed with an empty content vector.
    qRegisterMetaType<QVector<sOEmbedContent>>("QVector<sOEmbedContent>");

    UBOEmbedParser parser;
    QSignalSpy spy(&parser, &UBOEmbedParser::oembedParsed);

    parser.parse("<html><head><title>no oembed here</title></head></html>");

    QCOMPARE(spy.count(), 1);
    const QList<QVariant> args = spy.takeFirst();
    const QVector<sOEmbedContent> contents = args.at(0).value<QVector<sOEmbedContent>>();
    QCOMPARE(contents.size(), 0);
}
