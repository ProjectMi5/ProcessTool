#ifndef OPCUACONFIGURATOR_H
#define OPCUACONFIGURATOR_H

#include "uabase.h"
#include "uaclientsdk.h"

class OpcuaConfigurator
{
    UA_DISABLE_COPY(OpcuaConfigurator);
public:
    OpcuaConfigurator();
    virtual ~OpcuaConfigurator();

    // get connection and session parameters
    UaString getServerUrl() const;
    UaString getDiscoveryUrl() const;
    UaString getApplicationName() const;
    UaString getProductUri() const;
    OpcUa_Boolean getAutomaticReconnect() const;
    OpcUa_Boolean getRetryInitialConnect() const;

    // get lists of NodeIds and values
    UaNodeIdArray getNodesToRead() const;
    UaNodeIdArray getNodesToWrite() const;
    UaVariantArray getWriteValues() const;
    UaNodeIdArray getNodesToSubscribe() const;

    // load configuration from file to fill connection parameters, NamespaceArray and NodeIds
    UaStatus loadConfiguration(const UaString& sConfigurationFile);

    // update the namespace indexes for all nodeIds and update the internal namespaceArray
    UaStatus updateNamespaceIndexes(const UaStringArray& namespaceArray);

private:
    // connection and session configuration
    UaString        m_applicationName;
    UaString        m_serverUrl;
    UaString        m_discoveryUrl;
    OpcUa_Boolean   m_bAutomaticReconnect;
    OpcUa_Boolean   m_bRetryInitialConnect;

    // NamespaceArray and NodeIds
    UaStringArray   m_namespaceArray;
    UaNodeIdArray   m_nodesToRead;
    UaNodeIdArray   m_nodesToWrite;
    UaVariantArray  m_writeValues;
    UaNodeIdArray   m_nodesToSubscribe;
};

#endif // OPCUACONFIGURATOR_H
