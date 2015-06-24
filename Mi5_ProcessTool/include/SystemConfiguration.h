#ifndef SYSTEMCONFIGURATION_H
#define SYSTEMCONFIGURATION_H
#include <qstring.h>
#include <qlist.h>

class ModuleConfiguration
{
public:
    ModuleConfiguration(QString Name, QString Ip, bool Enable);
    ~ModuleConfiguration();

public:
    QString name;
    QString ip;
    bool enable;

};

class SystemConfiguration
{
public:
    SystemConfiguration();
    ~SystemConfiguration();

public:
    bool init;
    QString mainServerUri;
    QList<ModuleConfiguration> moduleList;
};

#endif //SYSTEMCONFIGURATION_H