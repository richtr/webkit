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

#ifndef UniqueIDBDatabase_h
#define UniqueIDBDatabase_h

#if ENABLE(INDEXED_DATABASE)

#include "IDBBackingStore.h"
#include "IDBBindingUtilities.h"
#include "IDBDatabaseIdentifier.h"
#include "IDBDatabaseInfo.h"
#include "IDBGetResult.h"
#include "IDBServerOperation.h"
#include "ThreadSafeDataBuffer.h"
#include "Timer.h"
#include "UniqueIDBDatabaseConnection.h"
#include "UniqueIDBDatabaseTransaction.h"
#include <wtf/Deque.h>
#include <wtf/HashCountedSet.h>
#include <wtf/HashSet.h>
#include <wtf/Ref.h>
#include <wtf/ThreadSafeRefCounted.h>

namespace WebCore {

class IDBError;
class IDBRequestData;
class IDBTransactionInfo;

namespace IndexedDB {
enum class IndexRecordType;
}

namespace IDBServer {

class IDBConnectionToClient;
class IDBServer;

typedef std::function<void(const IDBError&)> ErrorCallback;
typedef std::function<void(const IDBError&, const IDBKeyData&)> KeyDataCallback;
typedef std::function<void(const IDBError&, const IDBGetResult&)> GetResultCallback;
typedef std::function<void(const IDBError&, uint64_t)> CountCallback;

class UniqueIDBDatabase : public ThreadSafeRefCounted<UniqueIDBDatabase> {
public:
    static Ref<UniqueIDBDatabase> create(IDBServer& server, const IDBDatabaseIdentifier& identifier)
    {
        return adoptRef(*new UniqueIDBDatabase(server, identifier));
    }

    void openDatabaseConnection(IDBConnectionToClient&, const IDBRequestData&);

    const IDBDatabaseInfo& info() const;
    IDBServer& server() { return m_server; }
    const IDBDatabaseIdentifier& identifier() const { return m_identifier; }

    void createObjectStore(UniqueIDBDatabaseTransaction&, const IDBObjectStoreInfo&, ErrorCallback);
    void deleteObjectStore(UniqueIDBDatabaseTransaction&, const String& objectStoreName, ErrorCallback);
    void clearObjectStore(UniqueIDBDatabaseTransaction&, uint64_t objectStoreIdentifier, ErrorCallback);
    void createIndex(UniqueIDBDatabaseTransaction&, const IDBIndexInfo&, ErrorCallback);
    void deleteIndex(UniqueIDBDatabaseTransaction&, uint64_t objectStoreIdentifier, const String& indexName, ErrorCallback);
    void putOrAdd(const IDBRequestData&, const IDBKeyData&, const ThreadSafeDataBuffer& valueData, IndexedDB::ObjectStoreOverwriteMode, KeyDataCallback);
    void getRecord(const IDBRequestData&, const IDBKeyRangeData&, GetResultCallback);
    void getCount(const IDBRequestData&, const IDBKeyRangeData&, CountCallback);
    void deleteRecord(const IDBRequestData&, const IDBKeyRangeData&, ErrorCallback);
    void openCursor(const IDBRequestData&, const IDBCursorInfo&, GetResultCallback);
    void iterateCursor(const IDBRequestData&, const IDBKeyData&, unsigned long count, GetResultCallback);
    void commitTransaction(UniqueIDBDatabaseTransaction&, ErrorCallback);
    void abortTransaction(UniqueIDBDatabaseTransaction&, ErrorCallback);
    void transactionDestroyed(UniqueIDBDatabaseTransaction&);
    void connectionClosedFromClient(UniqueIDBDatabaseConnection&);

    void enqueueTransaction(Ref<UniqueIDBDatabaseTransaction>&&);

    void handleDelete(IDBConnectionToClient&, const IDBRequestData&);
    bool deletePending() const { return m_deletePending; }

    static JSC::VM& databaseThreadVM();
    static JSC::ExecState& databaseThreadExecState();

private:
    UniqueIDBDatabase(IDBServer&, const IDBDatabaseIdentifier&);
    
    void handleOpenDatabaseOperations();
    void addOpenDatabaseConnection(Ref<UniqueIDBDatabaseConnection>&&);
    bool maybeDeleteDatabase(IDBServerOperation*);
    bool hasAnyOpenConnections() const;

    void startVersionChangeTransaction();
    void notifyConnectionsOfVersionChangeForUpgrade();
    void notifyConnectionsOfVersionChange(uint64_t requestedVersion);

    void activateTransactionInBackingStore(UniqueIDBDatabaseTransaction&);
    void inProgressTransactionCompleted(const IDBResourceIdentifier&);

    // Database thread operations
    void openBackingStore(const IDBDatabaseIdentifier&);
    void performCommitTransaction(uint64_t callbackIdentifier, const IDBResourceIdentifier& transactionIdentifier);
    void performAbortTransaction(uint64_t callbackIdentifier, const IDBResourceIdentifier& transactionIdentifier);
    void beginTransactionInBackingStore(const IDBTransactionInfo&);
    void performCreateObjectStore(uint64_t callbackIdentifier, const IDBResourceIdentifier& transactionIdentifier, const IDBObjectStoreInfo&);
    void performDeleteObjectStore(uint64_t callbackIdentifier, const IDBResourceIdentifier& transactionIdentifier, const String& objectStoreName);
    void performClearObjectStore(uint64_t callbackIdentifier, const IDBResourceIdentifier& transactionIdentifier, uint64_t objectStoreIdentifier);
    void performCreateIndex(uint64_t callbackIdentifier, const IDBResourceIdentifier& transactionIdentifier, const IDBIndexInfo&);
    void performDeleteIndex(uint64_t callbackIdentifier, const IDBResourceIdentifier& transactionIdentifier, uint64_t objectStoreIdentifier, const String& indexName);
    void performPutOrAdd(uint64_t callbackIdentifier, const IDBResourceIdentifier& transactionIdentifier, uint64_t objectStoreIdentifier, const IDBKeyData&, const ThreadSafeDataBuffer& valueData, IndexedDB::ObjectStoreOverwriteMode);
    void performGetRecord(uint64_t callbackIdentifier, const IDBResourceIdentifier& transactionIdentifier, uint64_t objectStoreIdentifier, const IDBKeyRangeData&);
    void performGetIndexRecord(uint64_t callbackIdentifier, const IDBResourceIdentifier& transactionIdentifier, uint64_t objectStoreIdentifier, uint64_t indexIdentifier, IndexedDB::IndexRecordType, const IDBKeyRangeData&);
    void performGetCount(uint64_t callbackIdentifier, const IDBResourceIdentifier& transactionIdentifier, uint64_t objectStoreIdentifier, uint64_t indexIdentifier, const IDBKeyRangeData&);
    void performDeleteRecord(uint64_t callbackIdentifier, const IDBResourceIdentifier& transactionIdentifier, uint64_t objectStoreIdentifier, const IDBKeyRangeData&);
    void performOpenCursor(uint64_t callbackIdentifier, const IDBResourceIdentifier& transactionIdentifier, const IDBCursorInfo&);
    void performIterateCursor(uint64_t callbackIdentifier, const IDBResourceIdentifier& transactionIdentifier, const IDBResourceIdentifier& cursorIdentifier, const IDBKeyData&, unsigned long count);
    void performActivateTransactionInBackingStore(uint64_t callbackIdentifier, const IDBTransactionInfo&);

    // Main thread callbacks
    void didOpenBackingStore(const IDBDatabaseInfo&);
    void didPerformCreateObjectStore(uint64_t callbackIdentifier, const IDBError&, const IDBObjectStoreInfo&);
    void didPerformDeleteObjectStore(uint64_t callbackIdentifier, const IDBError&, const String& objectStoreName);
    void didPerformClearObjectStore(uint64_t callbackIdentifier, const IDBError&);
    void didPerformCreateIndex(uint64_t callbackIdentifier, const IDBError&, const IDBIndexInfo&);
    void didPerformDeleteIndex(uint64_t callbackIdentifier, const IDBError&, uint64_t objectStoreIdentifier, const String& indexName);
    void didPerformPutOrAdd(uint64_t callbackIdentifier, const IDBError&, const IDBKeyData&);
    void didPerformGetRecord(uint64_t callbackIdentifier, const IDBError&, const IDBGetResult&);
    void didPerformGetCount(uint64_t callbackIdentifier, const IDBError&, uint64_t);
    void didPerformDeleteRecord(uint64_t callbackIdentifier, const IDBError&);
    void didPerformOpenCursor(uint64_t callbackIdentifier, const IDBError&, const IDBGetResult&);
    void didPerformIterateCursor(uint64_t callbackIdentifier, const IDBError&, const IDBGetResult&);
    void didPerformCommitTransaction(uint64_t callbackIdentifier, const IDBError&, const IDBResourceIdentifier& transactionIdentifier);
    void didPerformAbortTransaction(uint64_t callbackIdentifier, const IDBError&, const IDBResourceIdentifier& transactionIdentifier);
    void didPerformActivateTransactionInBackingStore(uint64_t callbackIdentifier, const IDBError&);

    uint64_t storeCallback(ErrorCallback);
    uint64_t storeCallback(KeyDataCallback);
    uint64_t storeCallback(GetResultCallback);
    uint64_t storeCallback(CountCallback);

    void performErrorCallback(uint64_t callbackIdentifier, const IDBError&);
    void performKeyDataCallback(uint64_t callbackIdentifier, const IDBError&, const IDBKeyData&);
    void performGetResultCallback(uint64_t callbackIdentifier, const IDBError&, const IDBGetResult&);
    void performCountCallback(uint64_t callbackIdentifier, const IDBError&, uint64_t);

    bool hasAnyPendingCallbacks() const;

    void invokeDeleteOrRunTransactionTimer();
    void deleteOrRunTransactionsTimerFired();
    RefPtr<UniqueIDBDatabaseTransaction> takeNextRunnableTransaction(bool& hadDeferredTransactions);

    IDBServer& m_server;
    IDBDatabaseIdentifier m_identifier;
    
    Deque<Ref<IDBServerOperation>> m_pendingOpenDatabaseOperations;
    Deque<Ref<IDBServerOperation>> m_pendingDeleteDatabaseOperations;

    HashSet<RefPtr<UniqueIDBDatabaseConnection>> m_openDatabaseConnections;
    HashSet<RefPtr<UniqueIDBDatabaseConnection>> m_closePendingDatabaseConnections;

    RefPtr<IDBServerOperation> m_versionChangeOperation;
    RefPtr<UniqueIDBDatabaseConnection> m_versionChangeDatabaseConnection;
    UniqueIDBDatabaseTransaction* m_versionChangeTransaction { nullptr };

    bool m_isOpeningBackingStore { false };
    std::unique_ptr<IDBBackingStore> m_backingStore;
    std::unique_ptr<IDBDatabaseInfo> m_databaseInfo;

    HashMap<uint64_t, ErrorCallback> m_errorCallbacks;
    HashMap<uint64_t, KeyDataCallback> m_keyDataCallbacks;
    HashMap<uint64_t, GetResultCallback> m_getResultCallbacks;
    HashMap<uint64_t, CountCallback> m_countCallbacks;

    Timer m_deleteOrRunTransactionsTimer;
    Timer m_handleOpenDatabaseOperationsTimer;

    Deque<RefPtr<UniqueIDBDatabaseTransaction>> m_pendingTransactions;
    HashMap<IDBResourceIdentifier, RefPtr<UniqueIDBDatabaseTransaction>> m_inProgressTransactions;

    // The key into this set is the object store ID.
    // The set counts how many transactions are open to the given object store.
    // This helps make sure opening narrowly scoped transactions (one or two object stores)
    // doesn't continuously block widely scoped write transactions.
    HashCountedSet<uint64_t> m_objectStoreTransactionCounts;

    bool m_deletePending { false };
    bool m_hasNotifiedConnectionsOfDelete { false };
};

} // namespace IDBServer
} // namespace WebCore

#endif // ENABLE(INDEXED_DATABASE)
#endif // UniqueIDBDatabase_h
