#include "../../headers/views/hierarchytreeview.h"
#include "../../headers/models/blockdefinition.h"
#include "../../headers/models/partproperty.h"
#include <QPixmap>
#include <QPainter>
#include <QHeaderView>

HierarchyTreeView::HierarchyTreeView(QWidget *parent)
    : QTreeWidget(parent)
{
    setupTreeWidget();
}

void HierarchyTreeView::setupTreeWidget()
{
    // Column configuration
    setColumnCount(1);
    setHeaderLabel("System Hierarchy");

    // Visual styling
    setAlternatingRowColors(true);
    setAnimated(true);
    setExpandsOnDoubleClick(true);
    setIndentation(20);

    // Selection behavior
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    // Disable drag-drop for now
    setDragEnabled(false);
    setAcceptDrops(false);

    // Connect signals
    connect(this, &QTreeWidget::itemClicked, this, &HierarchyTreeView::onItemClicked);
    connect(this, &QTreeWidget::itemDoubleClicked, this, &HierarchyTreeView::onItemDoubleClicked);
}

void HierarchyTreeView::addDefinition(BlockDefinition *definition)
{
    if (!definition || m_definitionItems.contains(definition->id()))
    {
        return;
    }

    // Create tree item
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, definition->typeName());
    item->setIcon(0, createColorIcon(definition->color()));
    // Store pointer for easy retrieval (variant cast)
    item->setData(0, Qt::UserRole, QVariant::fromValue(static_cast<void*>(definition)));

    // Add to root level
    addTopLevelItem(item);

    // Store mappings
    m_definitionItems[definition->id()] = item;
    m_itemToDefinition[item] = definition;

    // Connect to model updates
    connect(definition, &BlockDefinition::typeNameChanged, this, &HierarchyTreeView::onDefinitionTypeNameChanged);
    // connect(definition, &BlockDefinition::colorChanged, this, &HierarchyTreeView::onColorChanged); // Assuming colorChanged exists now or will exist
}

void HierarchyTreeView::removeDefinition(BlockDefinition *definition)
{
    if (!definition)
        return;

    QTreeWidgetItem *item = m_definitionItems.value(definition->id());
    if (item)
    {
        // Disconnect signals
        disconnect(definition, &BlockDefinition::typeNameChanged, this, &HierarchyTreeView::onDefinitionTypeNameChanged);
        // disconnect definition colorChanged

        // Remove from tree
        int index = indexOfTopLevelItem(item);
        if (index >= 0)
        {
            takeTopLevelItem(index);
        }

        // Clean up mappings
        m_definitionItems.remove(definition->id());
        m_itemToDefinition.remove(item);

        delete item;
    }
}

QTreeWidgetItem *HierarchyTreeView::findDefinitionItem(BlockDefinition *definition)
{
    if (!definition)
        return nullptr;

    return m_definitionItems.value(definition->id(), nullptr);
}

void HierarchyTreeView::updatePart(PartProperty *part)
{
    if (!part || !part->owner())
        return;

    // Find custom item for owner
    QTreeWidgetItem *ownerItem = findDefinitionItem(part->owner());
    if (!ownerItem)
        return; // Owner not in tree (maybe filtered out?)

    // Find existing item for this part
    QTreeWidgetItem *partItem = nullptr;
    for (int i = 0; i < ownerItem->childCount(); ++i)
    {
        QTreeWidgetItem *child = ownerItem->child(i);
        // Check ID stored in item data
        QString storedId = child->data(0, Qt::UserRole + 1).toString();
        if (storedId == part->id())
        {
            partItem = child;
            break;
        }
    }

    // Format text: [mult] name : Type
    QString text = part->displayText();

    if (!partItem)
    {
        // Create new item
        partItem = new QTreeWidgetItem(ownerItem);
        partItem->setData(0, Qt::UserRole + 1, part->id()); // Store Part ID
        // Maybe store pointer too?
    }

    partItem->setText(0, text);
    // Icon? Maybe a small "part" icon or dot
    // partItem->setIcon(0, ...);
    
    // Expand owner to show new part
    ownerItem->setExpanded(true);
}

void HierarchyTreeView::clearHierarchy()
{
    clear();
    m_definitionItems.clear();
    m_itemToDefinition.clear();
}

void HierarchyTreeView::selectDefinition(BlockDefinition *definition)
{
    if (!definition)
        return;

    QTreeWidgetItem *item = m_definitionItems.value(definition->id());
    if (item)
    {
        setCurrentItem(item);
        scrollToItem(item);
    }
}

void HierarchyTreeView::clearSelection()
{
    QTreeWidget::clearSelection();
    setCurrentItem(nullptr);
}

void HierarchyTreeView::onItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    // Check if it's a definition
    BlockDefinition *def = m_itemToDefinition.value(item);
    if (def)
    {
        emit definitionItemClicked(def);
        return;
    }
    
    // If it's a part, maybe select the part?
    // For now we only have definition selection logic in main window diagram view.
}

void HierarchyTreeView::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    BlockDefinition *def = m_itemToDefinition.value(item);
    if (def)
    {
        emit definitionItemDoubleClicked(def);
    }
}

void HierarchyTreeView::onDefinitionTypeNameChanged(const QString &typeName)
{
    BlockDefinition *def = qobject_cast<BlockDefinition *>(sender());
    if (!def)
        return;

    QTreeWidgetItem *item = m_definitionItems.value(def->id());
    if (item)
    {
        item->setText(0, typeName);
    }
}

void HierarchyTreeView::onColorChanged(const QColor &color)
{
    // Implementation pending BlockDefinition signal
    Q_UNUSED(color);
}

QIcon HierarchyTreeView::createColorIcon(const QColor &color)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(color);

    // Add border
    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, 15, 15);

    return QIcon(pixmap);
}