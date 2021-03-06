/* Copyright (c) 2013, AOYAMA Kazuharu
 * All rights reserved.
 *
 * This software may be used and distributed according to the terms of
 * the New BSD License, which is incorporated herein by reference.
 */

#include <TWebApplication>
#include <THttpRequestHeader>
#include "thttpbuffer.h"
#include "tsystemglobal.h"

static int limitBodyBytes = -1;


THttpBuffer::THttpBuffer()
    : lengthToRead(-1)
{
    httpBuffer.reserve(1024);
}


THttpBuffer::~THttpBuffer()
{ }


THttpBuffer::THttpBuffer(const THttpBuffer &other)
    : httpBuffer(other.httpBuffer),
      lengthToRead(other.lengthToRead),
      clientAddr(other.clientAddr)
{ }


THttpBuffer &THttpBuffer::operator=(const THttpBuffer &other)
{
    httpBuffer = other.httpBuffer;
    lengthToRead = other.lengthToRead;
    clientAddr = other.clientAddr;
    return *this;
}


QByteArray THttpBuffer::read(int maxSize)
{
    int size = qMin(httpBuffer.length(), maxSize);
    QByteArray res(size, 0);
    read(res.data(), size);
    return res;
}


int THttpBuffer::read(char *data, int maxSize)
{
    int size = qMin(httpBuffer.length(), maxSize);
    memcpy(data, httpBuffer.data(), size);
    httpBuffer.remove(0, size);
    return size;
}


int THttpBuffer::write(const char *data, int maxSize)
{
    httpBuffer.append(data, maxSize);

    if (lengthToRead < 0) {
        parse();
    } else {
        if (limitBodyBytes > 0 && httpBuffer.length() > limitBodyBytes) {
            throw ClientErrorException(413);  // Request Entity Too Large
        }

        lengthToRead -= qMin((qint64)maxSize, lengthToRead);
    }
    return maxSize;
}


int THttpBuffer::write(const QByteArray &byteArray)
{
    return write(byteArray.data(), byteArray.length());
}


void THttpBuffer::parse()
{
    if (limitBodyBytes < 0) {
        limitBodyBytes = Tf::app()->appSettings().value("LimitRequestBody", "0").toInt();
    }

    if (lengthToRead < 0) {
        int idx = httpBuffer.indexOf("\r\n\r\n");
        if (idx > 0) {
            THttpRequestHeader header(httpBuffer.left(idx + 4));
            tSystemDebug("content-length: %d", header.contentLength());

            if (limitBodyBytes > 0 && header.contentLength() > (uint)limitBodyBytes) {
                throw ClientErrorException(413);  // Request Entity Too Large
            }

            lengthToRead = qMax(idx + 4 + (qint64)header.contentLength() - httpBuffer.length(), 0LL);
            tSystemDebug("lengthToRead: %d", (int)lengthToRead);
        }
    } else {
        tSystemWarn("Unreachable code in normal communication");
    }
}


bool THttpBuffer::canReadHttpRequest() const
{
    return (lengthToRead == 0);
}


void THttpBuffer::clear()
{
    lengthToRead = -1;
    httpBuffer.truncate(0);
    httpBuffer.reserve(1024);
    clientAddr.clear();
}
