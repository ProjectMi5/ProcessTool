#ifndef IMODULE_H
#define IMODULE_H

class IModule
{

public:
    virtual void subscriptionDataChange(OpcUa_UInt32 clientSubscriptionHandle,
                                        const UaDataNotifications& dataNotifications,
                                        const UaDiagnosticInfos&   diagnosticInfos) = 0;
    virtual void startup() = 0;
    virtual void serverReconnected() = 0;

};
#endif // IMODULE_H