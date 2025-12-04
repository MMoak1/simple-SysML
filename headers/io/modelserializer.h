#ifndef MODELSERIALIZER_H
#define MODELSERIALIZER_H

#include <QString>
#include <QList>
#include <QJsonObject>

class BlockModel;
class ConnectionModel;
class StateMachineModel;
class StateModel;
class TransitionModel;

class ModelSerializer
{
public:
    // Save the entire model to a JSON file
    static bool saveToFile(const QString &filePath, 
                          const QList<BlockModel*> &blocks, 
                          const QList<ConnectionModel*> &connections);

    // Load the model from a JSON file
    // Note: The caller is responsible for clearing existing models before calling this
    static bool loadFromFile(const QString &filePath, 
                            QList<BlockModel*> &outBlocks, 
                            QList<ConnectionModel*> &outConnections);

private:
    // Helper methods for serialization
    static QJsonObject serializeBlock(BlockModel *block);
    static QJsonObject serializeConnection(ConnectionModel *connection);
    static QJsonObject serializeStateMachine(StateMachineModel *sm);
    static QJsonObject serializeState(StateModel *state);
    static QJsonObject serializeTransition(TransitionModel *transition);

    // Helper methods for deserialization
    static BlockModel* deserializeBlock(const QJsonObject &json);
    static ConnectionModel* deserializeConnection(const QJsonObject &json, const QList<BlockModel*> &blocks);
    static void deserializeStateMachine(const QJsonObject &json, BlockModel *block);
    static StateModel* deserializeState(const QJsonObject &json);
    static TransitionModel* deserializeTransition(const QJsonObject &json, const QList<StateModel*> &states);
    
    // Helper to find a block by ID
    static BlockModel* findBlockById(const QString &id, const QList<BlockModel*> &blocks);
    // Helper to find a state by ID
    static StateModel* findStateById(const QString &id, const QList<StateModel*> &states);
};

#endif // MODELSERIALIZER_H
