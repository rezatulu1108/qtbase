// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QTest>
#include <QBuffer>
#include <QtEndian>
#if QT_CONFIG(process)
#include <QProcess>
#endif

#include <qfont.h>
#include <private/qfont_p.h>
#include <qfontdatabase.h>
#include <qfontinfo.h>
#include <qstringlist.h>
#include <qguiapplication.h>
#ifndef QT_NO_WIDGETS
#include <qwidget.h>
#endif
#include <qlist.h>
#include <QtTest/private/qemulationdetector_p.h>

class tst_QFont : public QObject
{
Q_OBJECT

private slots:
    void getSetCheck();
    void exactMatch();
    void compare();
    void resolve();
#ifndef QT_NO_WIDGETS
    void resetFont();
#endif
    void isCopyOf();
    void italicOblique();
    void insertAndRemoveSubstitutions();
    void serialize_data();
    void serialize();
    void deserializeQt515();

    void styleName();
    void defaultFamily_data();
    void defaultFamily();
    void toAndFromString();
    void fromStringWithoutStyleName();
    void fromDegenerateString_data();
    void fromDegenerateString();

    void sharing();
    void familyNameWithCommaQuote_data();
    void familyNameWithCommaQuote();
    void setFamilies_data();
    void setFamilies();
    void setFamiliesAndFamily_data();
    void setFamiliesAndFamily();
    void featureAccessors();
};

// Testing get/set functions
void tst_QFont::getSetCheck()
{
    QFont obj1;
    // Style QFont::style()
    // void QFont::setStyle(Style)
    obj1.setStyle(QFont::Style(QFont::StyleNormal));
    QCOMPARE(QFont::Style(QFont::StyleNormal), obj1.style());
    obj1.setStyle(QFont::Style(QFont::StyleItalic));
    QCOMPARE(QFont::Style(QFont::StyleItalic), obj1.style());
    obj1.setStyle(QFont::Style(QFont::StyleOblique));
    QCOMPARE(QFont::Style(QFont::StyleOblique), obj1.style());

    // StyleStrategy QFont::styleStrategy()
    // void QFont::setStyleStrategy(StyleStrategy)
    obj1.setStyleStrategy(QFont::StyleStrategy(QFont::PreferDefault));
    QCOMPARE(QFont::StyleStrategy(QFont::PreferDefault), obj1.styleStrategy());
    obj1.setStyleStrategy(QFont::StyleStrategy(QFont::PreferBitmap));
    QCOMPARE(QFont::StyleStrategy(QFont::PreferBitmap), obj1.styleStrategy());
    obj1.setStyleStrategy(QFont::StyleStrategy(QFont::PreferDevice));
    QCOMPARE(QFont::StyleStrategy(QFont::PreferDevice), obj1.styleStrategy());
    obj1.setStyleStrategy(QFont::StyleStrategy(QFont::PreferOutline));
    QCOMPARE(QFont::StyleStrategy(QFont::PreferOutline), obj1.styleStrategy());
    obj1.setStyleStrategy(QFont::StyleStrategy(QFont::ForceOutline));
    QCOMPARE(QFont::StyleStrategy(QFont::ForceOutline), obj1.styleStrategy());
    obj1.setStyleStrategy(QFont::StyleStrategy(QFont::PreferMatch));
    QCOMPARE(QFont::StyleStrategy(QFont::PreferMatch), obj1.styleStrategy());
    obj1.setStyleStrategy(QFont::StyleStrategy(QFont::PreferQuality));
    QCOMPARE(QFont::StyleStrategy(QFont::PreferQuality), obj1.styleStrategy());
    obj1.setStyleStrategy(QFont::StyleStrategy(QFont::PreferAntialias));
    QCOMPARE(QFont::StyleStrategy(QFont::PreferAntialias), obj1.styleStrategy());
    obj1.setStyleStrategy(QFont::StyleStrategy(QFont::NoAntialias));
    QCOMPARE(QFont::StyleStrategy(QFont::NoAntialias), obj1.styleStrategy());
}

void tst_QFont::exactMatch()
{
    QFont font;

    // Check if a non-existing font hasn't an exact match
    font = QFont( "BogusFont", 33 );
    QVERIFY( !font.exactMatch() );
    QVERIFY(!QFont("sans").exactMatch());
    QVERIFY(!QFont("sans-serif").exactMatch());
    QVERIFY(!QFont("serif").exactMatch());
    QVERIFY(!QFont("monospace").exactMatch());

    font.setFamilies(QStringList() << "BogusFont");
    QVERIFY(!font.exactMatch());
    QVERIFY(!QFont("sans").exactMatch());
    QVERIFY(!QFont("sans-serif").exactMatch());
    QVERIFY(!QFont("serif").exactMatch());
    QVERIFY(!QFont("monospace").exactMatch());

    // Confirm that exactMatch is true for a valid font
    const QString family = QFontDatabase::families().first();
    const QString style = QFontDatabase::styles(family).first();
    const int pointSize = QFontDatabase::pointSizes(family, style).first();
    font = QFontDatabase::font(family, style, pointSize);
    QVERIFY(font.exactMatch());

    if (QFontDatabase::families().contains("Arial")) {
        font = QFont("Arial");
        QVERIFY(font.exactMatch());
        font = QFont(QString());
        font.setFamilies({"Arial"});
        QVERIFY(font.exactMatch());
    }
}

void tst_QFont::italicOblique()
{
    QStringList families = QFontDatabase::families();
    if (families.isEmpty())
        return;

    QStringList::ConstIterator f_it, f_end = families.end();
    for (f_it = families.begin(); f_it != f_end; ++f_it) {

        QString family = *f_it;
        QStringList styles = QFontDatabase::styles(family);
        QStringList::ConstIterator s_it, s_end = styles.end();
        for (s_it = styles.begin(); s_it != s_end; ++s_it) {
            QString style = *s_it;

            if (QFontDatabase::isSmoothlyScalable(family, style)) {
                if (style.contains("Oblique")) {
                    style.replace("Oblique", "Italic");
                } else if (style.contains("Italic")) {
                    style.replace("Italic", "Oblique");
                } else {
                    continue;
                }
                QFont f = QFontDatabase::font(family, style, 12);
                QVERIFY2(f.italic(), qPrintable(QString::asprintf("Failed for font \"%ls\"", qUtf16Printable(f.family()))));
            }
        }
    }
}

void tst_QFont::compare()
{
    QFont font;
    {
        QFont font2 = font;
        font2.setPointSize(24);
        QVERIFY(font != font2);
        QCOMPARE(font < font2,!(font2 < font));
    }
    {
        QFont font2 = font;
        font2.setPixelSize(24);
        QVERIFY(font != font2);
        QCOMPARE(font < font2,!(font2 < font));
    }

    font.setPointSize(12);
    font.setItalic(false);
    font.setWeight(QFont::Normal);
    font.setUnderline(false);
    font.setStrikeOut(false);
    font.setOverline(false);
    {
        QFont font2 = font;
        font2.setPointSize(24);
        QVERIFY(font != font2);
        QCOMPARE(font < font2,!(font2 < font));
    }
    {
        QFont font2 = font;
        font2.setPixelSize(24);
        QVERIFY(font != font2);
        QCOMPARE(font < font2,!(font2 < font));
    }
    {
        QFont font2 = font;

        font2.setItalic(true);
        QVERIFY(font != font2);
        QCOMPARE(font < font2,!(font2 < font));
        font2.setItalic(false);
        QCOMPARE(font, font2);
        QVERIFY(!(font < font2));

        font2.setWeight(QFont::Bold);
        QVERIFY(font != font2);
        QCOMPARE(font < font2,!(font2 < font));
        font2.setWeight(QFont::Normal);
        QCOMPARE(font, font2);
        QVERIFY(!(font < font2));

        font.setUnderline(true);
        QVERIFY(font != font2);
        QCOMPARE(font < font2,!(font2 < font));
        font.setUnderline(false);
        QCOMPARE(font, font2);
        QVERIFY(!(font < font2));

        font.setStrikeOut(true);
        QVERIFY(font != font2);
        QCOMPARE(font < font2,!(font2 < font));
        font.setStrikeOut(false);
        QCOMPARE(font, font2);
        QVERIFY(!(font < font2));

        font.setOverline(true);
        QVERIFY(font != font2);
        QCOMPARE(font < font2,!(font2 < font));
        font.setOverline(false);
        QCOMPARE(font, font2);
        QVERIFY(!(font < font2));

        font.setCapitalization(QFont::SmallCaps);
        QVERIFY(font != font2);
        QCOMPARE(font < font2,!(font2 < font));
        font.setCapitalization(QFont::MixedCase);
        QCOMPARE(font, font2);
        QVERIFY(!(font < font2));
    }
}

void tst_QFont::resolve()
{
    QFont font;
    font.setPointSize(font.pointSize() * 2);
    font.setItalic(false);
    font.setWeight(QFont::Normal);
    font.setUnderline(false);
    font.setStrikeOut(false);
    font.setOverline(false);
    font.setStretch(QFont::Unstretched);

    QFont font1;
    font1.setWeight(QFont::Bold);
    QFont font2 = font1.resolve(font);

    QCOMPARE(font2.weight(), font1.weight());

    QCOMPARE(font2.pointSize(), font.pointSize());
    QCOMPARE(font2.italic(), font.italic());
    QCOMPARE(font2.underline(), font.underline());
    QCOMPARE(font2.overline(), font.overline());
    QCOMPARE(font2.strikeOut(), font.strikeOut());
    QCOMPARE(font2.stretch(), font.stretch());

    QFont font3;
    font3.setStretch(QFont::UltraCondensed);
    QFont font4 = font3.resolve(font1).resolve(font);

    QCOMPARE(font4.stretch(), font3.stretch());

    QCOMPARE(font4.weight(), font.weight());
    QCOMPARE(font4.pointSize(), font.pointSize());
    QCOMPARE(font4.italic(), font.italic());
    QCOMPARE(font4.underline(), font.underline());
    QCOMPARE(font4.overline(), font.overline());
    QCOMPARE(font4.strikeOut(), font.strikeOut());


    QFont f1,f2,f3;
    f2.setPointSize(45);
    f3.setPointSize(55);

    QFont f4 = f1.resolve(f2);
    QCOMPARE(f4.pointSize(), 45);
    f4 = f4.resolve(f3);
    QCOMPARE(f4.pointSize(), 55);

    QFont font5, font6;
    const QStringList fontFamilies = { QStringLiteral("Arial") };
    font5.setFamilies(fontFamilies);
    font6 = font6.resolve(font5);
    QCOMPARE(font6.families(), fontFamilies);

    QFont font7, font8;
    // This will call setFamilies() directly now
    font7.setFamily(QLatin1String("Helvetica"));
    font8.setFamilies(fontFamilies);
    font7 = font7.resolve(font8);
    QCOMPARE(font7.families(), QStringList({"Helvetica"}));
    QCOMPARE(font7.family(), "Helvetica");
}

#ifndef QT_NO_WIDGETS
void tst_QFont::resetFont()
{
    QWidget parent;
    QWidget firstChild(&parent);
    QFont parentFont = parent.font();
    parentFont.setPointSize(parentFont.pointSize() + 2);
    parent.setFont(parentFont);

    QFont childFont = firstChild.font();
    childFont.setBold(!childFont.bold());
    firstChild.setFont(childFont);

    QWidget secondChild(&parent);
    secondChild.setFont(childFont);

    QVERIFY(parentFont.resolveMask() != 0);
    QVERIFY(childFont.resolveMask() != 0);
    QVERIFY(childFont != parentFont);

    // reset font on both children
    firstChild.setFont(QFont());
    secondChild.setFont(QFont());

    QCOMPARE(firstChild.font().resolveMask(), QFont::SizeResolved);
    QCOMPARE(secondChild.font().resolveMask(), QFont::SizeResolved);
    QCOMPARE(firstChild.font().pointSize(), parent.font().pointSize());
    QCOMPARE(secondChild.font().pointSize(), parent.font().pointSize());
    QVERIFY(parent.font().resolveMask() != 0);
}
#endif

void tst_QFont::isCopyOf()
{
    QFont font;
    QVERIFY(font.isCopyOf(QGuiApplication::font()));

    QFont font2("bogusfont",  23);
    QVERIFY(! font2.isCopyOf(QGuiApplication::font()));

    QFont font3 = font;
    QVERIFY(font3.isCopyOf(font));

    font3.setPointSize(256);
    QVERIFY(!font3.isCopyOf(font));
    font3.setPointSize(font.pointSize());
    QVERIFY(!font3.isCopyOf(font));
}

void tst_QFont::insertAndRemoveSubstitutions()
{
    QFont::removeSubstitutions("BogusFontFamily");
    // make sure it is empty before we start
    QVERIFY(QFont::substitutes("BogusFontFamily").isEmpty());
    QVERIFY(QFont::substitutes("bogusfontfamily").isEmpty());

    // inserting Foo
    QFont::insertSubstitution("BogusFontFamily", "Foo");
    QCOMPARE(QFont::substitutes("BogusFontFamily").size(), 1);
    QCOMPARE(QFont::substitutes("bogusfontfamily").size(), 1);

    // inserting Bar and Baz
    QStringList moreFonts;
    moreFonts << "Bar" << "Baz";
    QFont::insertSubstitutions("BogusFontFamily", moreFonts);
    QCOMPARE(QFont::substitutes("BogusFontFamily").size(), 3);
    QCOMPARE(QFont::substitutes("bogusfontfamily").size(), 3);

    QFont::removeSubstitutions("BogusFontFamily");
    // make sure it is empty again
    QVERIFY(QFont::substitutes("BogusFontFamily").isEmpty());
    QVERIFY(QFont::substitutes("bogusfontfamily").isEmpty());
}

Q_DECLARE_METATYPE(QDataStream::Version)

void tst_QFont::serialize_data()
{
    QTest::addColumn<QFont>("font");
    // The version in which the tested feature was added.
    QTest::addColumn<QDataStream::Version>("minimumStreamVersion");

    QFont basicFont;
    // Versions <= Qt 2.1 had broken point size serialization,
    // so we set an integer point size.
    basicFont.setPointSize(9);
    // Versions <= Qt 5.4 didn't serialize styleName, so clear it
    basicFont.setStyleName(QString());

    QFont font = basicFont;
    QTest::newRow("defaultConstructed") << font << QDataStream::Qt_1_0;

    font.setLetterSpacing(QFont::AbsoluteSpacing, 105);
    QTest::newRow("letterSpacing=105") << font << QDataStream::Qt_4_5;

    font = basicFont;
    font.setWordSpacing(50.0);
    QTest::newRow("wordSpacing") << font << QDataStream::Qt_4_5;

    font = basicFont;
    font.setPointSize(20);
    QTest::newRow("pointSize") << font << QDataStream::Qt_1_0;

    font = basicFont;
    font.setPixelSize(32);
    QTest::newRow("pixelSize") << font << QDataStream::Qt_3_0;

    font = basicFont;
    font.setStyleHint(QFont::Monospace);
    QTest::newRow("styleHint") << font << QDataStream::Qt_1_0;

    font = basicFont;
    font.setStretch(4000);
    QTest::newRow("stretch") << font << QDataStream::Qt_4_3;

    font = basicFont;
    font.setWeight(QFont::Light);
    QTest::newRow("weight") << font << QDataStream::Qt_1_0;

    font = basicFont;
    font.setUnderline(true);
    QTest::newRow("underline") << font << QDataStream::Qt_1_0;

    font = basicFont;
    font.setStrikeOut(true);
    QTest::newRow("strikeOut") << font << QDataStream::Qt_1_0;

    font = basicFont;
    font.setFixedPitch(true);
    // This fails for versions less than this, as ignorePitch is set to false
    // whenever setFixedPitch() is called, but ignorePitch is considered an
    // extended bit, which were apparently not available until 4.4.
    QTest::newRow("fixedPitch") << font << QDataStream::Qt_4_4;

    font = basicFont;
    font.setLetterSpacing(QFont::AbsoluteSpacing, 10);
    // Fails for 4.4 because letterSpacing wasn't read until 4.5.
    QTest::newRow("letterSpacing=10") << font << QDataStream::Qt_4_5;

    font = basicFont;
    font.setKerning(false);
    QTest::newRow("kerning") << font << QDataStream::Qt_4_0;

    font = basicFont;
    font.setStyleStrategy(QFont::NoFontMerging);
    // This wasn't read properly until 5.4.
    QTest::newRow("styleStrategy") << font << QDataStream::Qt_5_4;

    font = basicFont;
    font.setHintingPreference(QFont::PreferFullHinting);
    // This wasn't read until 5.4.
    QTest::newRow("hintingPreference") << font << QDataStream::Qt_5_4;

    font = basicFont;
    font.setStyleName("Regular Black Condensed");
    // This wasn't read until 5.4.
    QTest::newRow("styleName") << font << QDataStream::Qt_5_4;

    font = basicFont;
    font.setCapitalization(QFont::AllUppercase);
    // This wasn't read until 5.6.
    QTest::newRow("capitalization") << font << QDataStream::Qt_5_6;
}

void tst_QFont::serialize()
{
    QFETCH(QFont, font);
    QFETCH(QDataStream::Version, minimumStreamVersion);

    QDataStream stream;
    const int thisVersion = stream.version();

    for (int version = minimumStreamVersion; version <= thisVersion; ++version) {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        stream.setDevice(&buffer);
        stream.setVersion(version);
        stream << font;
        buffer.close();

        buffer.open(QIODevice::ReadOnly);
        QFont readFont;
        stream >> readFont;
        QVERIFY2(readFont == font, qPrintable(QString::fromLatin1("Fonts do not compare equal for QDataStream version ") +
            QString::fromLatin1("%1:\nactual: %2\nexpected: %3").arg(version).arg(readFont.toString()).arg(font.toString())));
    }
}

void tst_QFont::deserializeQt515()
{
    QFile file;
    file.setFileName(QFINDTESTDATA("datastream.515"));
    QVERIFY(file.open(QIODevice::ReadOnly));

    QFont font;
    {
        QDataStream stream(&file);
        stream.setVersion(QDataStream::Qt_5_15);
        stream >> font;
    }

    QCOMPARE(font.family(), QStringLiteral("FirstFamily"));
    QCOMPARE(font.families().size(), 3);
    QCOMPARE(font.families().at(0), QStringLiteral("FirstFamily"));
    QCOMPARE(font.families().at(1), QStringLiteral("OtherFamily1"));
    QCOMPARE(font.families().at(2), QStringLiteral("OtherFamily2"));
    QCOMPARE(font.pointSize(), 12);

    QVERIFY(file.reset());
    QByteArray fileContent = file.readAll();
    QByteArray serializedContent;
    {
        QBuffer buffer(&serializedContent);
        QVERIFY(buffer.open(QIODevice::WriteOnly));

        QDataStream stream(&buffer);
        stream.setVersion(QDataStream::Qt_5_15);
        stream << font;
    }

    QCOMPARE(serializedContent, fileContent);

    file.close();
}

void tst_QFont::styleName()
{
#if !defined(Q_OS_MAC)
    QSKIP("Only tested on Mac");
#else
    QFont font("Helvetica Neue");
    font.setStyleName("UltraLight");

    QCOMPARE(QFontInfo(font).styleName(), QString("UltraLight"));
#endif
}

QString getPlatformGenericFont(const char* genericName)
{
#if defined(Q_OS_UNIX) && !defined(QT_NO_FONTCONFIG) && QT_CONFIG(process)
    QProcess p;
    p.start(QLatin1String("fc-match"), (QStringList() << "-f%{family}" << genericName));
    if (!p.waitForStarted())
        qWarning("fc-match cannot be started: %s", qPrintable(p.errorString()));
    if (p.waitForFinished())
        return QString::fromLatin1(p.readAllStandardOutput());
#endif
    return QLatin1String(genericName);
}

static inline QByteArray msgNotAcceptableFont(const QString &defaultFamily, const QStringList &acceptableFamilies)
{
    QString res = QString::fromLatin1("Font family '%1' is not one of the following acceptable results: ").arg(defaultFamily);
    Q_FOREACH (const QString &family, acceptableFamilies)
        res += QLatin1String("\n ") + family;
    return res.toLocal8Bit();
}

Q_DECLARE_METATYPE(QFont::StyleHint)
void tst_QFont::defaultFamily_data()
{
    QTest::addColumn<QFont::StyleHint>("styleHint");
    QTest::addColumn<QStringList>("acceptableFamilies");

    QTest::newRow("serif") << QFont::Serif << (QStringList() << "Times New Roman" << "Times" << "Droid Serif" << getPlatformGenericFont("serif").split(","));
    QTest::newRow("monospace") << QFont::Monospace << (QStringList() << "Courier New" << "Monaco" << "Menlo" << "Droid Sans Mono" << getPlatformGenericFont("monospace").split(","));
    QTest::newRow("cursive") << QFont::Cursive << (QStringList() << "Comic Sans MS" << "Apple Chancery" << "Roboto" << "Droid Sans" << getPlatformGenericFont("cursive").split(","));
    QTest::newRow("fantasy") << QFont::Fantasy << (QStringList() << "Impact" << "Zapfino"  << "Roboto" << "Droid Sans" << getPlatformGenericFont("fantasy").split(","));
    QTest::newRow("sans-serif") << QFont::SansSerif << (QStringList() << "Arial" << "Lucida Grande" << "Helvetica" << "Roboto" << "Droid Sans" << "Segoe UI" << getPlatformGenericFont("sans-serif").split(","));
}

void tst_QFont::defaultFamily()
{
    QFETCH(QFont::StyleHint, styleHint);
    QFETCH(QStringList, acceptableFamilies);

    QFont f;
    f.setStyleHint(styleHint);
    const QString familyForHint(f.defaultFamily());

    // it should at least return a family that is available.
    QVERIFY(QFontDatabase::hasFamily(familyForHint));

    bool isAcceptable = false;
    Q_FOREACH (const QString& family, acceptableFamilies) {
        if (!familyForHint.compare(family, Qt::CaseInsensitive)) {
            isAcceptable = true;
            break;
        }
    }

#if defined(Q_OS_UNIX) && defined(QT_NO_FONTCONFIG)
    QSKIP("This platform does not support checking for default font acceptability");
#endif

#ifdef Q_PROCESSOR_ARM_32
    if (QTestPrivate::isRunningArmOnX86())
        QEXPECT_FAIL("", "Fails on ARMv7 QEMU (QTQAINFRA-4127)", Continue);
#endif

#ifdef Q_OS_ANDROID
    QEXPECT_FAIL("serif", "QTBUG-69215", Continue);
#endif
    QVERIFY2(isAcceptable, msgNotAcceptableFont(familyForHint, acceptableFamilies));
}

void tst_QFont::toAndFromString()
{
    QFont defaultFont = QGuiApplication::font();
    QString family = defaultFont.family();

    const QStringList stylesList = QFontDatabase::styles(family);
    if (stylesList.size() == 0)
        QSKIP("Default font doesn't have any styles");

    for (const QString &style : stylesList) {
        QFont result;
        QFont initial = QFontDatabase::font(family, style, defaultFont.pointSize());

        result.fromString(initial.toString());

        QCOMPARE(result, initial);
    }

    // Since Qt 6.0 it was changed to include more information in the description, so
    // this checks for compatibility
    const QString fontStringFrom515(QLatin1String("Times New Roman,18,-1,5,75,1,0,0,1,0,Regular"));
    QFont fontFrom515("Times New Roman", 18);
    fontFrom515.setBold(true);
    fontFrom515.setItalic(true);
    fontFrom515.setFixedPitch(true);
    fontFrom515.setStyleName("Regular");
    QFont from515String;
    from515String.fromString(fontStringFrom515);
    QCOMPARE(from515String, fontFrom515);

    const QString fontStringFrom60(
            QLatin1String("Times New Roman,18,-1,5,700,1,0,0,1,0,1,0,150.5,2.5,50,2,Regular"));
    QFont fontFrom60 = fontFrom515;
    fontFrom60.setStyleStrategy(QFont::PreferBitmap);
    fontFrom60.setCapitalization(QFont::AllUppercase);
    fontFrom60.setLetterSpacing(QFont::PercentageSpacing, 150.5);
    fontFrom60.setWordSpacing(2.5);
    fontFrom60.setStretch(50);
    QFont from60String;
    from60String.fromString(fontStringFrom60);
    QCOMPARE(fontFrom60.toString(), fontStringFrom60);
    QCOMPARE(from60String, fontFrom60);
}

void tst_QFont::fromStringWithoutStyleName()
{
    QFont font1;
    font1.fromString("Noto Sans,12,-1,5,50,0,0,0,0,0,Regular");

    QFont font2 = font1;
    const QString str = "Times,16,-1,5,400,0,0,0,0,0,0,0,0,0,0,1";
    font2.fromString(str);

    QCOMPARE(font2.toString(), str);

    const QString fontStringFrom60(
            QLatin1String("Times New Roman,18,-1,5,700,1,0,0,1,0,1,0,150.5,2.5,50,2"));
    QFont font3;
    font3.fromString("Noto Sans,12,-1,5,50,0,0,0,0,0,Regular");
    QFont font4 = font3;
    font4.fromString(fontStringFrom60);
    QCOMPARE(font4.toString(), fontStringFrom60);
}

void tst_QFont::fromDegenerateString_data()
{
    QTest::addColumn<QString>("string");

    QTest::newRow("empty") << QString();
    QTest::newRow("justAComma") << ",";
    QTest::newRow("commasAndSpaces") << " , ,    ";
    QTest::newRow("spaces") << "   ";
    QTest::newRow("spacesTabsAndNewlines") << " \t  \n";
}

void tst_QFont::fromDegenerateString()
{
    QFETCH(QString, string);
    QFont f;
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*Invalid description.*"));
    QCOMPARE(f.fromString(string), false);
    QCOMPARE(f, QFont());
}

void tst_QFont::sharing()
{
    // QFontCache references the engineData
    int refs_by_cache = 1;

    QFont f;
    f.setStyleHint(QFont::Serif);
    f.exactMatch(); // loads engine
    QCOMPARE(QFontPrivate::get(f)->ref.loadRelaxed(), 1);
    QVERIFY(QFontPrivate::get(f)->engineData);
    QCOMPARE(QFontPrivate::get(f)->engineData->ref.loadRelaxed(), 1 + refs_by_cache);

    QFont f2(f);
    QCOMPARE(QFontPrivate::get(f2), QFontPrivate::get(f));
    QCOMPARE(QFontPrivate::get(f2)->ref.loadRelaxed(), 2);
    QVERIFY(QFontPrivate::get(f2)->engineData);
    QCOMPARE(QFontPrivate::get(f2)->engineData, QFontPrivate::get(f)->engineData);
    QCOMPARE(QFontPrivate::get(f2)->engineData->ref.loadRelaxed(), 1 + refs_by_cache);

    f2.setKerning(!f.kerning());
    QVERIFY(QFontPrivate::get(f2) != QFontPrivate::get(f));
    QCOMPARE(QFontPrivate::get(f2)->ref.loadRelaxed(), 1);
    QVERIFY(QFontPrivate::get(f2)->engineData);
    QCOMPARE(QFontPrivate::get(f2)->engineData, QFontPrivate::get(f)->engineData);
    QCOMPARE(QFontPrivate::get(f2)->engineData->ref.loadRelaxed(), 2 + refs_by_cache);

    f2 = f;
    QCOMPARE(QFontPrivate::get(f2), QFontPrivate::get(f));
    QCOMPARE(QFontPrivate::get(f2)->ref.loadRelaxed(), 2);
    QVERIFY(QFontPrivate::get(f2)->engineData);
    QCOMPARE(QFontPrivate::get(f2)->engineData, QFontPrivate::get(f)->engineData);
    QCOMPARE(QFontPrivate::get(f2)->engineData->ref.loadRelaxed(), 1 + refs_by_cache);

    if (f.pointSize() > 0)
        f2.setPointSize(f.pointSize() * 2 / 3);
    else
        f2.setPixelSize(f.pixelSize() * 2 / 3);
    QVERIFY(QFontPrivate::get(f2) != QFontPrivate::get(f));
    QCOMPARE(QFontPrivate::get(f2)->ref.loadRelaxed(), 1);
    QVERIFY(!QFontPrivate::get(f2)->engineData);
    QVERIFY(QFontPrivate::get(f2)->engineData != QFontPrivate::get(f)->engineData);
}

void tst_QFont::familyNameWithCommaQuote_data()
{
    QTest::addColumn<QString>("enteredFamilyName");
    QTest::addColumn<QString>("familyName");
    QTest::addColumn<QString>("chosenFamilyName");

    const QString standardFont(QFont().defaultFamily());
    if (standardFont.isEmpty())
        QSKIP("No default font available on the system");
    const QString weirdFont(QLatin1String("'My, weird'' font name',"));
    const QString bogusFont(QLatin1String("BogusFont"));
    const QString commaSeparated(standardFont + QLatin1String(",Times New Roman"));
    const QString commaSeparatedWeird(weirdFont + QLatin1String(",") + standardFont);
    const QString commaSeparatedBogus(bogusFont +  QLatin1String(",") + standardFont);

    QTest::newRow("standard") << standardFont << standardFont << standardFont;
    QTest::newRow("weird") << weirdFont << QString("'My") << standardFont;
    QTest::newRow("commaSeparated") << commaSeparated << standardFont << standardFont;
    QTest::newRow("commaSeparatedWeird") << commaSeparatedWeird << QString("'My") << standardFont;
    QTest::newRow("commaSeparatedBogus") << commaSeparatedBogus << bogusFont << standardFont;
}

void tst_QFont::familyNameWithCommaQuote()
{
    QFETCH(QString, familyName);
    QFETCH(QString, chosenFamilyName);

    const int weirdFontId = QFontDatabase::addApplicationFont(":/weirdfont.otf");

    QVERIFY(weirdFontId != -1);
    QFont f(familyName);
    QCOMPARE(f.family(), familyName);
    QCOMPARE(QFontInfo(f).family(), chosenFamilyName);

    QFontDatabase::removeApplicationFont(weirdFontId);
}

void tst_QFont::setFamilies_data()
{
    QTest::addColumn<QStringList>("families");
    QTest::addColumn<QString>("chosenFamilyName");

    const QString weirdFont(QLatin1String("'My, weird'' font name',"));
    const QString standardFont(QFont().defaultFamily());
    if (standardFont.isEmpty())
        QSKIP("No default font available on the system");

    QTest::newRow("emptyFamily") << (QStringList()) << QString();
    QTest::newRow("standard") << (QStringList() << standardFont) << standardFont;
    QTest::newRow("weird") << (QStringList() << weirdFont) << weirdFont;
    QTest::newRow("standard-weird") << (QStringList() << standardFont << weirdFont) << standardFont;
    QTest::newRow("weird-standard") << (QStringList() << weirdFont << standardFont) << weirdFont;
    QTest::newRow("nonexist-weird") << (QStringList() << "NonExistentFont" << weirdFont) << weirdFont;
}

void tst_QFont::setFamilies()
{
    QFETCH(QStringList, families);
    QFETCH(QString, chosenFamilyName);

    const int weirdFontId = QFontDatabase::addApplicationFont(":/weirdfont.otf");

    QVERIFY(weirdFontId != -1);
    QFont f;
    f.setFamilies(families);
    if (!chosenFamilyName.isEmpty()) // Only check when it is not empty
        QCOMPARE(QFontInfo(f).family(), chosenFamilyName);

    QFontDatabase::removeApplicationFont(weirdFontId);
}

void tst_QFont::setFamiliesAndFamily_data()
{
    QTest::addColumn<QStringList>("families");
    QTest::addColumn<QString>("family");
    QTest::addColumn<QString>("chosenFamilyName");

    const QString weirdFont(QLatin1String("'My, weird'' font name',"));
    const QString defaultFont(QFont().defaultFamily());
    if (defaultFont.isEmpty())
        QSKIP("No default font available on the system");

    const QString timesFont(QLatin1String("Times"));
    const QString nonExistFont(QLatin1String("NonExistentFont"));

    QTest::newRow("emptyFamily") << (QStringList()) << QString() << QString();
    QTest::newRow("firstInFamilies") << (QStringList() << defaultFont << timesFont) << weirdFont << defaultFont;
    QTest::newRow("secondInFamilies") << (QStringList() << nonExistFont << weirdFont) << defaultFont << weirdFont;
    QTest::newRow("family") << (QStringList() << nonExistFont) << defaultFont << defaultFont;
}

void tst_QFont::setFamiliesAndFamily()
{
    QFETCH(QStringList, families);
    QFETCH(QString, family);
    QFETCH(QString, chosenFamilyName);

    const int weirdFontId = QFontDatabase::addApplicationFont(":/weirdfont.otf");

    QVERIFY(weirdFontId != -1);
    QFont f;
    f.setFamily(family);
    f.setFamilies(families);
    if (!family.isEmpty()) // Only check when it is not empty
        QCOMPARE(QFontInfo(f).family(), chosenFamilyName);

    QFontDatabase::removeApplicationFont(weirdFontId);
}

void tst_QFont::featureAccessors()
{
    QFont font;
    QVERIFY(font.featureTags().isEmpty());
    font.setFeature("abcd", 0xc0ffee);

    quint32 abcdTag = QFont::stringToTag("abcd");
    quint32 bcdeTag = QFont::stringToTag("bcde");
    QVERIFY(font.isFeatureSet(abcdTag));
    QVERIFY(!font.isFeatureSet(bcdeTag));
    QCOMPARE(font.featureTags().size(), 1);
    QCOMPARE(font.featureTags().first(), abcdTag);
    QCOMPARE(QFont::tagToString(font.featureTags().first()), QByteArray("abcd"));
    QCOMPARE(font.featureValue(abcdTag), 0xc0ffee);
    QCOMPARE(font.featureValue(bcdeTag), 0);
    font.setFeature(abcdTag, 0xf00d);
    QCOMPARE(font.featureTags().size(), 1);
    QCOMPARE(font.featureValue(abcdTag), 0xf00d);
    font.setFeature("abcde", 0xcaca0);
    QVERIFY(!font.isFeatureSet(QFont::stringToTag("abcde")));
    QCOMPARE(font.featureTags().size(), 1);
    QFont font2 = font;

    font.unsetFeature("abcd");
    QVERIFY(!font.isFeatureSet(QFont::stringToTag("abcd")));
    QVERIFY(font.featureTags().isEmpty());

    QVERIFY(font2.isFeatureSet(QFont::stringToTag("abcd")));
    font2.clearFeatures();
    QVERIFY(font.featureTags().isEmpty());
}

QTEST_MAIN(tst_QFont)
#include "tst_qfont.moc"
