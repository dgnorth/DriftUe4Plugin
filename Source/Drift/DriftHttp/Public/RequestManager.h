// Copyright 2015-2017 Directive Games Limited - All Rights Reserved

#pragma once

#include "HttpRequest.h"

#include "Engine.h"
#include "Http.h"


// Don't change the order of the values!
enum class HttpMethods
{
    XGET=0,
    XPUT,
    XPOST,
    XPATCH,
    XDELETE,
    XHEAD,
    XOPTIONS
};


class DRIFTHTTP_API RequestManager : public TSharedFromThis<RequestManager>, public FTickableGameObject
{
public:
    TSharedRef<HttpRequest> Get(const FString& url);
    TSharedRef<HttpRequest> Get(const FString& url, HttpStatusCodes expectedCode);
    TSharedRef<HttpRequest> Delete(const FString& url);
    TSharedRef<HttpRequest> Delete(const FString& url, HttpStatusCodes expectedCode);
    TSharedRef<HttpRequest> Patch(const FString& url, const FString& payload);
    TSharedRef<HttpRequest> Patch(const FString& url, const FString& payload, HttpStatusCodes expectedCode);
    TSharedRef<HttpRequest> Put(const FString& url, const FString& payload);
    TSharedRef<HttpRequest> Put(const FString& url, const FString& payload, HttpStatusCodes expectedCode);
    TSharedRef<HttpRequest> Post(const FString& url, const FString& payload);
    TSharedRef<HttpRequest> Post(const FString& url, const FString& payload, HttpStatusCodes expectedCode);
    
    RequestManager();
    virtual ~RequestManager();
    
    TSharedRef<HttpRequest> CreateRequest(HttpMethods method, const FString& url);
    TSharedRef<HttpRequest> CreateRequest(HttpMethods method, const FString& url, HttpStatusCodes expectedCode);
    TSharedRef<HttpRequest> CreateRequest(HttpMethods method, const FString& url, const FString& payload);
    TSharedRef<HttpRequest> CreateRequest(HttpMethods method, const FString& url, const FString& payload, HttpStatusCodes expectedCode);

    void SetDefaultRetries(int32 retries) { defaultRetries_ = retries; }

    void SetMaxConcurrentRequests(int32 number)
    {
        maxConcurrentRequests_ = number;
    }

    void SetCache(TSharedPtr<IHttpCache> cache);

    void SetLogContext(TMap<FString, FString>&& context);
    void UpdateLogContext(TMap<FString, FString>& context);

    // FTickableObjectBase
    bool IsTickable() const override
    {
        return true;
    }
    void Tick(float DeltaTime) override;
    TStatId GetStatId() const override;

    /**
     * Default error handler assigned to new requests. Use this to capture any standard error
     * before the request handler itself gets it.
     */
    FRequestErrorDelegate DefaultErrorHandler;

    /**
     * Default unhandled error handler assigned to new requests. This will get called when
     * no other error handler marks an error as handled.
     */
    FUnhandledErrorDelegate DefaultUnhandledErrorHandler;

protected:
    friend class HttpRequest;

    /** Called when a request finishes processing regardless of success or failure */
    void OnRequestFinished(TSharedRef<HttpRequest> request);

    /** Add custom headers before returning the request */
    virtual void AddCustomHeaders(TSharedRef<HttpRequest> request) const {}
    
    /** Called to determine if a request should be retried */
    bool ShouldRetryCallback(FHttpRequestPtr request, FHttpResponsePtr response) const;

    bool ProcessRequest(TSharedRef<HttpRequest> request, bool forceQueued);

protected:
    /** Requests waiting to be processed in linear mode */
    TQueue<TSharedPtr<HttpRequest>> queuedRequests_;

    /** the requests currently being processed */
    TArray<TSharedRef<HttpRequest>> activeRequests_;

    /** The default retries count for all the requests */
    int32 defaultRetries_;

    /** How many requests can be running concurrently */
    int32 maxConcurrentRequests_;
    
    /** User-provided context to be attached to every call */
    TMap<FString, FString> userContext_;
    
    TSharedPtr<IHttpCache> cache_;
};
