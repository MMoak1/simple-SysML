#include "../../headers/io/modelserializer.h"
#include "../../headers/models/blockdefinition.h"
#include "../../headers/models/partproperty.h"
#include "../../headers/models/statemachinemodel.h"
#include "../../headers/models/statemodel.h"
#include "../../headers/models/transitionmodel.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QMap>

bool ModelSerializer::saveToFile(const QString &filePath, 
                                const QList<BlockDefinition*> &definitions)
{
    QJsonObject rootObject;
    rootObject["version"] = "2.0"; // Semantic version
    rootObject["name"] = "SysML Model";

    // Serialize definitions (blocks)
    QJsonArray definitionsArray;
    for (BlockDefinition *def : definitions)
    {
        definitionsArray.append(serializeDefinition(def));
    }
    rootObject["definitions"] = definitionsArray;

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
                                  QList<BlockDefinition*> &outDefinitions)
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

    QString version = rootObject["version"].toString();
    if (version != "2.0")
    {
        qWarning() << "Unknown or legacy file version:" << version;
        // Logic for upgrading legacy (1.0) could go here if needed
        return false;
    }

    QJsonArray definitionsArray = rootObject["definitions"].toArray();
    QMap<QString, BlockDefinition*> definitionMap;

    // Pass 1: Create Definition objects
    for (const QJsonValue &defValue : definitionsArray)
    {
        BlockDefinition *def = deserializeDefinition(defValue.toObject());
        if (def)
        {
            outDefinitions.append(def);
            definitionMap[def->id()] = def;
        }
    }

    // Pass 2: Deserialize Parts (Connections) and State Machines
    // We iterate the JSON array again because we need the map fully populated to resolve types
    for (const QJsonValue &defValue : definitionsArray)
    {
        QJsonObject defJson = defValue.toObject();
        QString id = defJson["id"].toString();
        
        BlockDefinition *def = definitionMap.value(id);
        if (def)
        {
            // Deserialize Parts
            if (defJson.contains("parts"))
            {
                QJsonArray partsArray = defJson["parts"].toArray();
                for (const QJsonValue &partValue : partsArray)
                {
                    deserializePart(partValue.toObject(), def, definitionMap);
                }
            }
            
            // Deserialize State Machine
            if (defJson.contains("stateMachine"))
            {
                deserializeStateMachine(defJson["stateMachine"].toObject(), def);
            }
        }
    }

    return true;
}

QJsonObject ModelSerializer::serializeDefinition(BlockDefinition *def)
{
    QJsonObject json;
    json["id"] = def->id();
    json["typeName"] = def->typeName();
    json["color"] = def->color().name();
    
    QJsonObject posJson;
    posJson["x"] = def->position().x();
    posJson["y"] = def->position().y();
    json["position"] = posJson;

    QJsonObject sizeJson;
    sizeJson["width"] = def->size().width();
    sizeJson["height"] = def->size().height();
    json["size"] = sizeJson;

    // Serialize Parts
    QJsonArray partsArray;
    for (PartProperty *part : def->partProperties())
    {
        partsArray.append(serializePart(part));
    }
    json["parts"] = partsArray;

    // Serialize State Machine
    if (def->hasStateMachine())
    {
        json["stateMachine"] = serializeStateMachine(def->stateMachine());
    }

    return json;
}

QJsonObject ModelSerializer::serializePart(PartProperty *part)
{
    QJsonObject json;
    json["id"] = part->id();
    json["name"] = part->name();
    json["multiplicity"] = part->multiplicity();
    
    if (part->type())
    {
        json["typeId"] = part->type()->id();
    }
    
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

BlockDefinition* ModelSerializer::deserializeDefinition(const QJsonObject &json)
{
    QString typeName = json["typeName"].toString();
    QColor color(json["color"].toString());
    
    QJsonObject posJson = json["position"].toObject();
    QPointF position(posJson["x"].toDouble(), posJson["y"].toDouble());
    
    // Create definition
    BlockDefinition *def = new BlockDefinition(typeName, color, position);
    
    // Set ID
    if (json.contains("id"))
    {
        def->setId(json["id"].toString());
    }
    
    // Set Size
    QJsonObject sizeJson = json["size"].toObject();
    QSizeF size(sizeJson["width"].toDouble(), sizeJson["height"].toDouble());
    def->setSize(size);

    return def;
}

void ModelSerializer::deserializePart(const QJsonObject &json, BlockDefinition *owner, const QMap<QString, BlockDefinition*> &definitionMap)
{
    QString name = json["name"].toString();
    QString typeId = json["typeId"].toString();
    QString multiplicity = json["multiplicity"].toString();
    
    BlockDefinition *type = definitionMap.value(typeId);
    if (!type)
    {
        qWarning() << "Part" << name << "refers to missing type ID:" << typeId;
        return;
    }
    
    PartProperty *part = owner->addPartProperty(name, type, multiplicity);
    
    // Set Part ID if present
    if (json.contains("id"))
    {
        part->setId(json["id"].toString());
    }
}

void ModelSerializer::deserializeStateMachine(const QJsonObject &json, BlockDefinition *def)
{
    StateMachineModel *sm = def->getOrCreateStateMachine();
    
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
        // Set ID
        // transition->setId(json["id"].toString()); // Assuming setters exist or ID logic preserved
        return transition;
    }
    
    return nullptr;
}

BlockDefinition* ModelSerializer::findDefinitionById(const QString &id, const QList<BlockDefinition*> &definitions)
{
    for (BlockDefinition *def : definitions)
    {
        if (def->id() == id)
        {
            return def;
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
