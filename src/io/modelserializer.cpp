#include "../../headers/io/modelserializer.h"
#include "../../headers/models/blockmodel.h"
#include "../../headers/models/connectionmodel.h"
#include "../../headers/models/statemachinemodel.h"
#include "../../headers/models/statemodel.h"
#include "../../headers/models/transitionmodel.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

bool ModelSerializer::saveToFile(const QString &filePath, 
                                const QList<BlockModel*> &blocks, 
                                const QList<ConnectionModel*> &connections)
{
    QJsonObject rootObject;
    rootObject["version"] = "1.0";
    rootObject["name"] = "SysML Model"; // Could be parameterized

    // Serialize blocks
    QJsonArray blocksArray;
    for (BlockModel *block : blocks)
    {
        blocksArray.append(serializeBlock(block));
    }
    rootObject["blocks"] = blocksArray;

    // Serialize connections
    QJsonArray connectionsArray;
    for (ConnectionModel *connection : connections)
    {
        connectionsArray.append(serializeConnection(connection));
    }
    rootObject["connections"] = connectionsArray;

    // Write to file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }

    QJsonDocument saveDoc(rootObject);
    file.write(saveDoc.toJson());
    return true;
}

bool ModelSerializer::loadFromFile(const QString &filePath, 
                                  QList<BlockModel*> &outBlocks, 
                                  QList<ConnectionModel*> &outConnections)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open file for reading:" << filePath;
        return false;
    }

    QByteArray saveData = file.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    QJsonObject rootObject = loadDoc.object();

    if (rootObject["version"].toString() != "1.0")
    {
        qWarning() << "Unknown file version";
        return false;
    }

    // Load blocks
    QJsonArray blocksArray = rootObject["blocks"].toArray();
    for (const QJsonValue &blockValue : blocksArray)
    {
        BlockModel *block = deserializeBlock(blockValue.toObject());
        if (block)
        {
            outBlocks.append(block);
        }
    }

    // Load connections
    QJsonArray connectionsArray = rootObject["connections"].toArray();
    for (const QJsonValue &connValue : connectionsArray)
    {
        ConnectionModel *conn = deserializeConnection(connValue.toObject(), outBlocks);
        if (conn)
        {
            outConnections.append(conn);
        }
    }

    return true;
}

QJsonObject ModelSerializer::serializeBlock(BlockModel *block)
{
    QJsonObject json;
    json["id"] = block->id();
    json["label"] = block->label();
    json["color"] = block->color().name();
    
    QJsonObject posJson;
    posJson["x"] = block->position().x();
    posJson["y"] = block->position().y();
    json["position"] = posJson;

    QJsonObject sizeJson;
    sizeJson["width"] = block->size().width();
    sizeJson["height"] = block->size().height();
    json["size"] = sizeJson;

    if (block->hasStateMachine())
    {
        json["stateMachine"] = serializeStateMachine(block->stateMachine());
    }

    return json;
}

QJsonObject ModelSerializer::serializeConnection(ConnectionModel *connection)
{
    QJsonObject json;
    // Assuming ConnectionModel has an ID, if not we might need to rely on start/end
    // But for now let's just save start and end block IDs
    json["startBlockId"] = connection->startBlockId();
    json["endBlockId"] = connection->endBlockId();
    return json;
}

QJsonObject ModelSerializer::serializeStateMachine(StateMachineModel *sm)
{
    QJsonObject json;
    
    QJsonArray statesArray;
    for (StateModel *state : sm->states())
    {
        statesArray.append(serializeState(state));
    }
    json["states"] = statesArray;

    QJsonArray transitionsArray;
    for (TransitionModel *transition : sm->transitions())
    {
        transitionsArray.append(serializeTransition(transition));
    }
    json["transitions"] = transitionsArray;

    return json;
}

QJsonObject ModelSerializer::serializeState(StateModel *state)
{
    QJsonObject json;
    json["id"] = state->id();
    json["label"] = state->label();
    json["type"] = static_cast<int>(state->stateType());
    
    QJsonObject posJson;
    posJson["x"] = state->position().x();
    posJson["y"] = state->position().y();
    json["position"] = posJson;

    QJsonObject sizeJson;
    sizeJson["width"] = state->size().width();
    sizeJson["height"] = state->size().height();
    json["size"] = sizeJson;

    return json;
}

QJsonObject ModelSerializer::serializeTransition(TransitionModel *transition)
{
    QJsonObject json;
    json["id"] = transition->id();
    json["startStateId"] = transition->startStateId();
    json["endStateId"] = transition->endStateId();
    json["label"] = transition->label();
    return json;
}

BlockModel* ModelSerializer::deserializeBlock(const QJsonObject &json)
{
    QString label = json["label"].toString();
    QColor color(json["color"].toString());
    
    QJsonObject posJson = json["position"].toObject();
    QPointF position(posJson["x"].toDouble(), posJson["y"].toDouble());
    
    QJsonObject sizeJson = json["size"].toObject();
    QSizeF size(sizeJson["width"].toDouble(), sizeJson["height"].toDouble());

    BlockModel *block = new BlockModel(color, label, position, size);
    
    // We need to set the ID to match the saved one
    if (json.contains("id"))
    {
        block->setId(json["id"].toString());
    }
    
    if (json.contains("stateMachine"))
    {
        deserializeStateMachine(json["stateMachine"].toObject(), block);
    }

    return block;
}

ConnectionModel* ModelSerializer::deserializeConnection(const QJsonObject &json, const QList<BlockModel*> &blocks)
{
    QString startId = json["startBlockId"].toString();
    QString endId = json["endBlockId"].toString();
    
    BlockModel *startBlock = findBlockById(startId, blocks);
    BlockModel *endBlock = findBlockById(endId, blocks);
    
    if (startBlock && endBlock)
    {
        return new ConnectionModel(startBlock, endBlock);
    }
    
    return nullptr;
}

void ModelSerializer::deserializeStateMachine(const QJsonObject &json, BlockModel *block)
{
    StateMachineModel *sm = block->getOrCreateStateMachine();
    
    QJsonArray statesArray = json["states"].toArray();
    for (const QJsonValue &stateValue : statesArray)
    {
        StateModel *state = deserializeState(stateValue.toObject());
        if (state)
        {
            sm->addState(state);
        }
    }
    
    QJsonArray transitionsArray = json["transitions"].toArray();
    for (const QJsonValue &transValue : transitionsArray)
    {
        TransitionModel *transition = deserializeTransition(transValue.toObject(), sm->states());
        if (transition)
        {
            sm->addTransition(transition);
        }
    }
}

StateModel* ModelSerializer::deserializeState(const QJsonObject &json)
{
    QString label = json["label"].toString();
    StateType type = static_cast<StateType>(json["type"].toInt());
    
    QJsonObject posJson = json["position"].toObject();
    QPointF position(posJson["x"].toDouble(), posJson["y"].toDouble());
    
    QJsonObject sizeJson = json["size"].toObject();
    QSizeF size(sizeJson["width"].toDouble(), sizeJson["height"].toDouble());
    
    StateModel *state = new StateModel(label, type, position, size);
    
    if (json.contains("id"))
    {
        state->setId(json["id"].toString());
    }
    
    return state;
}

TransitionModel* ModelSerializer::deserializeTransition(const QJsonObject &json, const QList<StateModel*> &states)
{
    QString startId = json["startStateId"].toString();
    QString endId = json["endStateId"].toString();
    QString label = json["label"].toString();
    
    StateModel *startState = findStateById(startId, states);
    StateModel *endState = findStateById(endId, states);
    
    if (startState && endState)
    {
        TransitionModel *transition = new TransitionModel(startState, endState, label);
        // ID is generated in constructor, but we should probably set it if we want to preserve it
        // transition->setId(json["id"].toString()); 
        return transition;
    }
    
    return nullptr;
}

BlockModel* ModelSerializer::findBlockById(const QString &id, const QList<BlockModel*> &blocks)
{
    for (BlockModel *block : blocks)
    {
        if (block->id() == id)
        {
            return block;
        }
    }
    return nullptr;
}

StateModel* ModelSerializer::findStateById(const QString &id, const QList<StateModel*> &states)
{
    for (StateModel *state : states)
    {
        if (state->id() == id)
        {
            return state;
        }
    }
    return nullptr;
}
