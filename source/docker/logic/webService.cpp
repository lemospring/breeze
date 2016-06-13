﻿#include "docker.h"
#include "webService.h"
#include <ProtoWebAgent.h>



WebService::WebService()
{
    slotting<RefreshServiceToMgrNotice>([](Tracing, ReadStream &rs) {});
    slotting<WebAgentToService>(std::bind(&WebService::onWebAgentToService, this, _1, _2)); //不需要shared_from_this
}

WebService::~WebService()
{
    
}

void WebService::onTick()
{
}

void WebService::_checkSafeDestroy()
{
    auto service = Docker::getRef().peekService(ServiceUserMgr, InvalidServiceID);
    if (!service)
    {
        finishUnload();
        return;
    }
    SessionManager::getRef().createTimer(500, std::bind(&WebService::_checkSafeDestroy, this));
}

void WebService::onUnload()
{
    _checkSafeDestroy();
}

bool WebService::onLoad()
{
    finishLoad();
    return true;
}

void WebService::onClientChange()
{
    return ;
}



void WebService::onWebAgentToService(Tracing trace, ReadStream &rs)
{
    WebAgentToService notice;
    rs >> notice;
    if (compareStringIgnCase(notice.method, "get"))
    {
        WriteHTTP wh;
        wh.addHead("Content-type", "application/json");
        wh.addHead("Keep-Alive", "timeout=2, max=200");
        wh.response("200", R"({"online":)" + toString(Docker::getRef().peekService(ServiceUser).size()) + "}");
        Docker::getRef().packetToClientViaDocker(trace.fromDockerID, notice.webClientID, wh.getStream(), wh.getStreamLen());
    }
    
}



