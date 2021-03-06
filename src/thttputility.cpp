/* Copyright (c) 2010-2013, AOYAMA Kazuharu
 * All rights reserved.
 *
 * This software may be used and distributed according to the terms of
 * the New BSD License, which is incorporated herein by reference.
 */

#include <QHash>
#include <QTextCodec>
#include <QLocale>
#include "tsystemglobal.h"
#include "thttputility.h"
#if defined(Q_OS_WIN)
#include <qt_windows.h>
#else
#include <time.h>
#endif

#define HTTP_DATE_TIME_FORMAT "ddd, d MMM yyyy hh:mm:ss"

typedef QHash<int, QByteArray> IntHash;

Q_GLOBAL_STATIC_WITH_INITIALIZER(IntHash, reasonPhrase,
{
    // Informational 1xx
    x->insert(Tf::Continue, "Continue");
    x->insert(Tf::SwitchingProtocols, "Switching Protocols");
    // Successful 2xx
    x->insert(Tf::OK, "OK");
    x->insert(Tf::Created, "Created");
    x->insert(Tf::Accepted, "Accepted");
    x->insert(Tf::NonAuthoritativeInformation, "Non-Authoritative Information");
    x->insert(Tf::NoContent, "No Content");
    x->insert(Tf::ResetContent, "Reset Content");
    x->insert(Tf::PartialContent, "Partial Content");
    // Redirection 3xx
    x->insert(Tf::MultipleChoices, "Multiple Choices");
    x->insert(Tf::MovedPermanently, "Moved Permanently");
    x->insert(Tf::Found, "Found");
    x->insert(Tf::SeeOther, "See Other");
    x->insert(Tf::NotModified, "Not Modified");
    x->insert(Tf::UseProxy, "Use Proxy");
    x->insert(Tf::TemporaryRedirect, "Temporary Redirect");
    // Client Error 4xx
    x->insert(Tf::BadRequest, "Bad Request");
    x->insert(Tf::Unauthorized, "Unauthorized");
    x->insert(Tf::PaymentRequired, "Payment Required");
    x->insert(Tf::Forbidden, "Forbidden");
    x->insert(Tf::NotFound, "Not Found");
    x->insert(Tf::MethodNotAllowed, "Method Not Allowed");
    x->insert(Tf::NotAcceptable, "Not Acceptable");
    x->insert(Tf::ProxyAuthenticationRequired, "Proxy Authentication Required");
    x->insert(Tf::RequestTimeout, "Request Timeout");
    x->insert(Tf::Conflict, "Conflict");
    x->insert(Tf::Gone, "Gone");
    x->insert(Tf::LengthRequired, "Length Required");
    x->insert(Tf::PreconditionFailed, "Precondition Failed");
    x->insert(Tf::RequestEntityTooLarge, "Request Entity Too Large");
    x->insert(Tf::RequestURITooLong, "Request-URI Too Long");
    x->insert(Tf::UnsupportedMediaType, "Unsupported Media Type");
    x->insert(Tf::RequestedRangeNotSatisfiable, "Requested Range Not Satisfiable");
    x->insert(Tf::ExpectationFailed, "Expectation Failed");
    // Server Error 5xx
    x->insert(Tf::InternalServerError, "Internal Server Error");
    x->insert(Tf::NotImplemented, "Not Implemented");
    x->insert(Tf::BadGateway, "Bad Gateway");
    x->insert(Tf::ServiceUnavailable, "Service Unavailable");
    x->insert(Tf::GatewayTimeout, "Gateway Timeout");
    x->insert(Tf::HTTPVersionNotSupported, "HTTP Version Not Supported");
});

/*!
  \class THttpUtility
  \brief The THttpUtility class contains utility functions.
*/

/*!
  Returns a decoded copy of \a enc. \a enc is first decoded from URL
  encoding, then converted from UTF-8 to unicode.
  @sa toUrlEncoding(const QString &, const QByteArray &)
*/
QString THttpUtility::fromUrlEncoding(const QByteArray &enc)
{
    QByteArray d = enc;
    d = QByteArray::fromPercentEncoding(d.replace("+", "%20"));
    return QString::fromUtf8(d.constData(), d.length());
}

/*!
  Returns an encoded copy of \a input. \a input is first converted to UTF-8,
  and all ASCII-characters that are not in the unreserved group are URL
  encoded.
  @sa fromUrlEncoding(const QByteArray &)
*/
QByteArray THttpUtility::toUrlEncoding(const QString &input, const QByteArray &exclude)
{
    return input.toUtf8().toPercentEncoding(exclude, "~").replace("%20", "+");
}

/*!
  Returns a converted copy of \a input. All applicable characters in \a input
  are converted to HTML entities. The conversions performed are:
  - & (ampersand) becomes &amp;amp;.
  - " (double quote) becomes &amp;quot; when Tf::Compatible or Tf::Quotes
    is set.
  - ' (single quote) becomes &amp;#039; only when Tf::Quotes is set.
  - < (less than) becomes &amp;lt;.
  - > (greater than) becomes &amp;gt;.
*/
QString THttpUtility::htmlEscape(const QString &input, Tf::EscapeFlag flag)
{
    const QLatin1Char amp('&');
    const QLatin1Char lt('<');
    const QLatin1Char gt('>');
    const QLatin1Char dquot('"');
    const QLatin1Char squot('\'');
    const QLatin1String eamp("&amp;");
    const QLatin1String elt("&lt;");
    const QLatin1String egt("&gt;");
    const QString edquot("&quot;");
    const QString esquot("&#039;");

    QString escaped;
    escaped.reserve(int(input.length() * 1.1));
    for (int i = 0; i < input.length(); ++i) {
        if (input.at(i) == amp) {
            escaped += eamp;
        } else if (input.at(i) == lt) {
            escaped += elt;
        } else if (input.at(i) == gt) {
            escaped += egt;
        } else if (input.at(i) == dquot) {
            escaped += (flag == Tf::Compatible || flag == Tf::Quotes) ? edquot : input.at(i);
        } else if (input.at(i) == squot) {
            escaped += (flag == Tf::Quotes) ? esquot : input.at(i);
        } else {
            escaped += input.at(i);
        }
    }
    return escaped;
}

/*!
  This function overloads htmlEscape(const QString &, Tf::EscapeFlag).
*/
QString THttpUtility::htmlEscape(int n, Tf::EscapeFlag)
{
    return htmlEscape(QString::number(n));
}

/*!
  This function overloads htmlEscape(const QString &, Tf::EscapeFlag).
*/
QString THttpUtility::htmlEscape(const char *input, Tf::EscapeFlag flag)
{
    return htmlEscape(QString(input), flag);
}

/*!
  This function overloads htmlEscape(const QString &, Tf::EscapeFlag).
*/
QString THttpUtility::htmlEscape(const QByteArray &input, Tf::EscapeFlag flag)
{
    return htmlEscape(QString(input), flag);
}

/*!
  This function overloads htmlEscape(const QString &, Tf::EscapeFlag).
*/
QString THttpUtility::htmlEscape(const QVariant &input, Tf::EscapeFlag flag)
{
    return htmlEscape(input.toString(), flag);
}

/*!
  Returns a converted copy of \a input. All applicable characters in \a input
  are converted to JSON representation. The conversions
  performed are:
  - & (ampersand) becomes \\u0026.
  - < (less than) becomes \\u003C.
  - > (greater than) becomes \\u003E.
*/
QString THttpUtility::jsonEscape(const QString &input)
{
    const QLatin1Char amp('&');
    const QLatin1Char lt('<');
    const QLatin1Char gt('>');

    QString escaped;
    escaped.reserve(int(input.length() * 1.1));
    for (int i = 0; i < input.length(); ++i) {
        if (input.at(i) == amp) {
            escaped += QLatin1String("\\u0026");
        } else if (input.at(i) == lt) {
            escaped += QLatin1String("\\u003C");
        } else if (input.at(i) == gt) {
            escaped += QLatin1String("\\u003E");
        } else {
            escaped += input.at(i);
        }
    }
    return escaped;
}

/*!
  This function overloads jsonEscape(const QString &).
*/
QString THttpUtility::jsonEscape(const char *input)
{
    return jsonEscape(QString(input));
}

/*!
  This function overloads jsonEscape(const QString &).
*/
QString THttpUtility::jsonEscape(const QByteArray &input)
{
    return jsonEscape(QString(input));
}

/*!
  This function overloads jsonEscape(const QString &).
*/
QString THttpUtility::jsonEscape(const QVariant &input)
{
    return jsonEscape(input.toString());
}

/*!
  This function overloads toMimeEncoded(const QString &, QTextCodec *).
  @sa fromMimeEncoded(const QByteArray &)
*/
QByteArray THttpUtility::toMimeEncoded(const QString &input, const QByteArray &encoding)
{
    QTextCodec *codec = QTextCodec::codecForName(encoding);
    return toMimeEncoded(input, codec);
}

/*!
  Returns a byte array copy of \a input, encoded as MIME-Base64.
  @sa fromMimeEncoded(const QByteArray &)
*/
QByteArray THttpUtility::toMimeEncoded(const QString &input, QTextCodec *codec)
{
    QByteArray encoded;
    if (!codec)
        return encoded;

#if QT_VERSION < 0x050000
    QByteArray array = codec->fromUnicode(input);
#else
    QByteArray array;
    if (codec->name().toLower() == "iso-2022-jp") {
        array = codec->fromUnicode(input + ' ');  // append dummy ascii char
        array.chop(1);
    } else {
        array = codec->fromUnicode(input);
    }
#endif

    encoded += "=?";
    encoded += codec->name();
    encoded += "?B?";
    encoded += array.toBase64();
    encoded += "?=";
    return encoded;
}

/*!
  Returns a decoded copy of the MIME-Base64 array \a mime.
  @sa toMimeEncoded(const QString &, QTextCodec *)
*/
QString THttpUtility::fromMimeEncoded(const QByteArray &mime)
{
    QString text;

    if (!mime.startsWith("=?"))
        return text;

    int i = 2;
    int j = mime.indexOf('?', i);
    if (j > i) {
        QByteArray encoding = mime.mid(i, j - i);
        QTextCodec *codec = QTextCodec::codecForName(encoding);
        if (!codec)
            return text;

        i = ++j;
        int j = mime.indexOf('?', i);
        if (j > i) {
            QByteArray enc = mime.mid(i, j - i);
            i = ++j;
            j = mime.indexOf("?=", i);
            if (j > i) {
                if (enc == "B" || enc == "b") {
                    QByteArray base = mime.mid(i, i - j);
                    text = codec->toUnicode(QByteArray::fromBase64(base));
                } else if (enc == "Q" || enc == "q") {
                    // no implement..
                } else {
                    // bad parameter
                }
            }
        }
    }
    return text;
}

/*!
  Returns a reason phrase of the HTTP status code \a statusCode.
*/
QByteArray THttpUtility::getResponseReasonPhrase(int statusCode)
{
    return reasonPhrase()->value(statusCode);
}

/*!
  Returns the numeric timezone "[+|-]hhmm" of the current computer
  as a bytes array.
*/
QByteArray THttpUtility::timeZone()
{
    long offset = 0;  // minutes

#if defined(Q_OS_WIN)
    TIME_ZONE_INFORMATION tzi;
    memset(&tzi, 0, sizeof(tzi));
    GetTimeZoneInformation(&tzi);
    offset = -tzi.Bias;

#elif defined(Q_OS_UNIX)
    time_t ltime = 0;
    tm *t = 0;
# if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    tzset();
    tm res;
    t = localtime_r(&ltime, &res);
# else
    t = localtime(&ltime);
# endif // _POSIX_THREAD_SAFE_FUNCTIONS
    offset = t->tm_gmtoff / 60;
#endif // Q_OS_UNIX

    QByteArray tz;
    tz += (offset > 0) ? '+' : '-';
    offset = qAbs(offset);
    tz += QString("%1%2").arg(offset / 60, 2, 10, QLatin1Char('0')).arg(offset % 60, 2, 10, QLatin1Char('0')).toLatin1();
    tSystemDebug("tz: %s", tz.data());
    return tz;
}

/*!
  Returns a byte array for Date field of an HTTP header, containing
  the datetime equivalent of \a localTime.
*/
QByteArray THttpUtility::toHttpDateTimeString(const QDateTime &localTime)
{
    QByteArray d = QLocale(QLocale::C).toString(localTime, HTTP_DATE_TIME_FORMAT).toLatin1();
    d += ' ';
    d += timeZone();
    return d;
}

/*!
  Parses the HTTP datetime array given in \a localTime and returns
  the datetime.
*/
QDateTime THttpUtility::fromHttpDateTimeString(const QByteArray &localTime)
{
    QByteArray tz = localTime.mid(localTime.length() - 5).trimmed();
    if (!tz.contains("GMT") && tz != timeZone()) {
        tWarn("Time zone not match: %s", tz.data());
    }
    return QLocale(QLocale::C).toDateTime(localTime.left(localTime.lastIndexOf(' ')), HTTP_DATE_TIME_FORMAT);
}

/*!
  Returns a byte array for Date field of an HTTP header, containing
  the UTC datetime equivalent of \a utc.
*/
QByteArray THttpUtility::toHttpDateTimeUTCString(const QDateTime &utc)
{
    QByteArray d = QLocale(QLocale::C).toString(utc, HTTP_DATE_TIME_FORMAT).toLatin1();
    d += " +0000";
    return d;
}

/*!
  Parses the UTC datetime array given in \a utc and returns
  the datetime.
*/
QDateTime THttpUtility::fromHttpDateTimeUTCString(const QByteArray &utc)
{
    if (!utc.endsWith(" +0000") && !utc.endsWith(" GMT")) {
        tWarn("HTTP Date-Time format error: %s", utc.data());
    }
    return QLocale(QLocale::C).toDateTime(utc.left(utc.lastIndexOf(' ')), HTTP_DATE_TIME_FORMAT);
}
