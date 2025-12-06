#ifndef MODELSERIALIZER_H
#define MODELSERIALIZER_H

#include <QString>
#include <QList>
#include <QJsonObject>

class BlockDefinition;
class PartProperty;
class StateMachineModel;
class StateModel;
class TransitionModel;

class ModelSerializer
{
public:
    // Save the entire model to a JSON file
    static bool saveToFile(const QString &filePath, 
                          const QList<BlockDefinition*> &definitions);

    // Load the model from a JSON file
    static bool loadFromFile(const QString &filePath, 
                            QList<BlockDefinition*> &outDefinitions);

private:
    // Helper methods for serialization
    static QJsonObject serializeDefinition(BlockDefinition *def);
    static QJsonObject serializePart(PartProperty *part);
    static QJsonObject serializeStateMachine(StateMachineModel *sm);
    static QJsonObject serializeState(StateModel *state);
    static QJsonObject serializeTransition(TransitionModel *transition);

    // Helper methods for deserialization
    static BlockDefinition* deserializeDefinition(const QJsonObject &json);
    static void deserializePart(const QJsonObject &json, BlockDefinition *owner, const QMap<QString, BlockDefinition*> &definitionMap);
    static void deserializeStateMachine(const QJsonObject &json, BlockDefinition *def);
    static StateModel* deserializeState(const QJsonObject &json);
    static TransitionModel* deserializeTransition(const QJsonObject &json, const QList<StateModel*> &states);
    
    // Helper to find a definition by ID
    static BlockDefinition* findDefinitionById(const QString &id, const QList<BlockDefinition*> &definitions);
    // Helper to find a state by ID
    static StateModel* findStateById(const QString &id, const QList<StateModel*> &states);
};

#endif // MODELSERIALIZER_H
