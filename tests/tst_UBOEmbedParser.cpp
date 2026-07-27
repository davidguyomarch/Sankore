#include "tst_UBOEmbedParser.h"
#include "web/UBOEmbedUtils.h"

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
