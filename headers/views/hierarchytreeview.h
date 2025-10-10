#ifndef HIERARCHYTREEVIEW_H
#define HIERARCHYTREEVIEW_H

#include <QTreeWidget>
#include <QMap>
#include <QColor>
#include "../models/blockmodel.h"

class HierarchyTreeView : public QTreeWidget
{
    Q_OBJECT

public:
    explicit HierarchyTreeView(QWidget *parent = nullptr);

    // Block management
    void addBlock(BlockModel *block);
    void removeBlock(BlockModel *block);
    QTreeWidgetItem *findBlockItem(BlockModel *block);

    // Hierarchy management
    void clearHierarchy();
    void setBlockAsRoot(BlockModel *block);
    void setBlockAsChild(BlockModel *parent, BlockModel *child);

    // Selection and highlighting
    void selectBlock(BlockModel *block);
    void clearBlockSelection();

signals:
    void blockItemClicked(BlockModel *block);
    void blockItemDoubleClicked(BlockModel *block);

private slots:
    void onItemClicked(QTreeWidgetItem *item, int column);
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onBlockLabelChanged(const QString &label);
    void onBlockColorChanged(const QColor &color);

private:
    QMap<QString, QTreeWidgetItem *> m_blockItems;       // blockId -> tree item
    QMap<QTreeWidgetItem *, BlockModel *> m_itemToBlock; // tree item -> block

    void setupTreeWidget();
    QIcon createColorIcon(const QColor &color);
};

#endif // HIERARCHYTREEVIEW_H