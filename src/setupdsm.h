#ifndef SETUPDSM_H
#define SETUPDSM_H

class SCTimeXMLSettings;
class DatasourceManager;
class QString;
class QStringList;
void setupDatasources(const QStringList& datasourceNames,
                      const SCTimeXMLSettings& settings,
                      const QString &kontenPath, const QString &bereitPath, const QString& specialremunfile);
extern DatasourceManager* kontenDSM;
extern DatasourceManager* bereitDSM;
extern DatasourceManager* specialRemunDSM;

#endif // SETUPDSM_H
