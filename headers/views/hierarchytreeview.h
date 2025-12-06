#ifndef HIERARCHYTREEVIEW_H
#define HIERARCHYTREEVIEW_H

#include <QTreeWidget>
#include <QMap>
#include <QColor>
class BlockDefinition;
class PartProperty;

class HierarchyTreeView : public QTreeWidget
{
    Q_OBJECT

public:
    explicit HierarchyTreeView(QWidget *parent = nullptr);

    // Definition management
    void addDefinition(BlockDefinition *definition);
    void removeDefinition(BlockDefinition *definition);
    QTreeWidgetItem *findDefinitionItem(BlockDefinition *definition);
    
    // Part management
    void updatePart(PartProperty *part); // Add or update part under its owner definition
    // Note: Parts are usually children of Definitions. 
    // If we want recursive (parts of parts' types), we need more complex logic.
    // For now, let's assume root items are Definitions, and children are Parts.
    
    // Hierarchy management
    void clearHierarchy();

    // Selection and highlighting
    void selectDefinition(BlockDefinition *definition);
    void clearSelection();

signals:
    void definitionItemClicked(BlockDefinition *definition);
    void definitionItemDoubleClicked(BlockDefinition *definition);
    // Maybe signal for part clicked?

private slots:
    void onItemClicked(QTreeWidgetItem *item, int column);
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onDefinitionTypeNameChanged(const QString &typeName);
    void onColorChanged(const QColor &color); // If we assume definition has color

private:
    QMap<QString, QTreeWidgetItem *> m_definitionItems;       // definitionId -> tree item
    QMap<QTreeWidgetItem *, BlockDefinition *> m_itemToDefinition; // tree item -> definition
    // For parts, we might need a map if we want to update them individually
    // QMap<PartProperty*, QTreeWidgetItem*> m_partItems;

    void setupTreeWidget();
    QIcon createColorIcon(const QColor &color);
};

#endif // HIERARCHYTREEVIEW_H