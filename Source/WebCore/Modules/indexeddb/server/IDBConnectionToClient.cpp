/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "IDBConnectionToClient.h"

#if ENABLE(INDEXED_DATABASE)

#include "UniqueIDBDatabaseConnection.h"

namespace WebCore {
namespace IDBServer {

Ref<IDBConnectionToClient> IDBConnectionToClient::create(IDBConnectionToClientDelegate& delegate)
{
    return adoptRef(*new IDBConnectionToClient(delegate));
}

IDBConnectionToClient::IDBConnectionToClient(IDBConnectionToClientDelegate& delegate)
    : m_delegate(delegate)
{
}

uint64_t IDBConnectionToClient::identifier() const
{
    return m_delegate->identifier();
}

void IDBConnectionToClient::didDeleteDatabase(const IDBResultData& result)
{
    m_delegate->didDeleteDatabase(result);
}

void IDBConnectionToClient::didOpenDatabase(const IDBResultData& result)
{
    m_delegate->didOpenDatabase(result);
}

void IDBConnectionToClient::didAbortTransaction(const IDBResourceIdentifier& transactionIdentifier, const IDBError& error)
{
    m_delegate->didAbortTransaction(transactionIdentifier, error);
}

void IDBConnectionToClient::didCreateObjectStore(const IDBResultData& result)
{
    m_delegate->didCreateObjectStore(result);
}

void IDBConnectionToClient::didDeleteObjectStore(const IDBResultData& result)
{
    m_delegate->didDeleteObjectStore(result);
}

void IDBConnectionToClient::didClearObjectStore(const IDBResultData& result)
{
    m_delegate->didClearObjectStore(result);
}

void IDBConnectionToClient::didCreateIndex(const IDBResultData& result)
{
    m_delegate->didCreateIndex(result);
}

void IDBConnectionToClient::didDeleteIndex(const IDBResultData& result)
{
    m_delegate->didDeleteIndex(result);
}

void IDBConnectionToClient::didPutOrAdd(const IDBResultData& result)
{
    m_delegate->didPutOrAdd(result);
}

void IDBConnectionToClient::didGetRecord(const IDBResultData& result)
{
    m_delegate->didGetRecord(result);
}

void IDBConnectionToClient::didGetCount(const IDBResultData& result)
{
    m_delegate->didGetCount(result);
}

void IDBConnectionToClient::didDeleteRecord(const IDBResultData& result)
{
    m_delegate->didDeleteRecord(result);
}

void IDBConnectionToClient::didOpenCursor(const IDBResultData& result)
{
    m_delegate->didOpenCursor(result);
}

void IDBConnectionToClient::didIterateCursor(const IDBResultData& result)
{
    m_delegate->didIterateCursor(result);
}

void IDBConnectionToClient::didCommitTransaction(const IDBResourceIdentifier& transactionIdentifier, const IDBError& error)
{
    m_delegate->didCommitTransaction(transactionIdentifier, error);
}

void IDBConnectionToClient::fireVersionChangeEvent(UniqueIDBDatabaseConnection& connection, uint64_t requestedVersion)
{
    m_delegate->fireVersionChangeEvent(connection, requestedVersion);
}

void IDBConnectionToClient::didStartTransaction(const IDBResourceIdentifier& transactionIdentifier, const IDBError& error)
{
    m_delegate->didStartTransaction(transactionIdentifier, error);
}

void IDBConnectionToClient::notifyOpenDBRequestBlocked(const IDBResourceIdentifier& requestIdentifier, uint64_t oldVersion, uint64_t newVersion)
{
    m_delegate->notifyOpenDBRequestBlocked(requestIdentifier, oldVersion, newVersion);
}

} // namespace IDBServer
} // namespace WebCore

#endif // ENABLE(INDEXED_DATABASE)
