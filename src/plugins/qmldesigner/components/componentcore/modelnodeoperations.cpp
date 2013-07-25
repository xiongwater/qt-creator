/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "modelnodeoperations.h"
#include "modelnodecontextmenu_helper.h"

#include <cmath>
#include <QMessageBox>
#include <nodeabstractproperty.h>
#include <nodemetainfo.h>
#include <modelnode.h>
#include <qmlitemnode.h>
#include <variantproperty.h>
#include <rewritingexception.h>
#include <rewritertransaction.h>
#include <qmlanchors.h>

namespace QmlDesigner {

const PropertyName auxDataString("anchors_");

static inline bool isItem(const ModelNode &node)
{
    return node.isValid() && node.metaInfo().isValid() && node.metaInfo().isSubclassOf("QtQuick.Item", -1, -1);
}

static inline QList<QmlItemNode> siblingsForNode(const QmlItemNode &itemNode)
{
    QList<QmlItemNode> siblingList;

    if (itemNode.isValid() && itemNode.modelNode().parentProperty().isValid()) {
        QList<ModelNode> modelNodes = itemNode.modelNode().parentProperty().parentModelNode().allDirectSubModelNodes();
        foreach (const ModelNode &node, modelNodes) {
            QmlItemNode childItemNode = node;
            if (childItemNode.isValid())
                siblingList.append(childItemNode);
        }
    }

    return siblingList;
}

static signed int getMaxZValue(const QList<QmlItemNode> &siblingList)
{
    signed int maximum = INT_MIN;
    foreach (const QmlItemNode &node, siblingList) {
        signed int z  = node.instanceValue("z").toInt();
        if (z > maximum)
            maximum = z;
    }
    return maximum;
}

static signed int getMinZValue(const QList<QmlItemNode> &siblingList)
{
    signed int minimum = INT_MAX;
    foreach (const QmlItemNode &node, siblingList) {
        signed int z  = node.instanceValue("z").toInt();
        if (z < minimum)
            minimum = z;
    }
    return minimum;
}

static inline void getWidthHeight(const ModelNode &node, int &width, int &height)
{
    QmlItemNode itemNode(node);
    if (itemNode.isValid()) {
        width = itemNode.instanceValue("width").toInt();
        height = itemNode.instanceValue("height").toInt();
    }
}

static inline bool modelNodesHaveProperty(const QList<ModelNode> &modelNodeList, const PropertyName &propertyName)
{
    foreach (const ModelNode &modelNode, modelNodeList)
        if (modelNode.hasProperty(propertyName))
            return true;

    return false;
}


namespace ModelNodeOperations {

void goIntoComponent(const ModelNode &modelNode)
{
    ComponentUtils::goIntoComponent(modelNode);
}

void select(const SelectionContext &selectionState)
{
    if (selectionState.view())
        selectionState.view()->setSelectedModelNodes(QList<ModelNode>() << selectionState.targetNode());
}

void deSelect(const SelectionContext &selectionState)
{
    if (selectionState.view()) {
        QList<ModelNode> selectedNodes = selectionState.view()->selectedModelNodes();
        foreach (const ModelNode &node, selectionState.selectedModelNodes()) {
            if (selectedNodes.contains(node))
                selectedNodes.removeAll(node);
        }
        selectionState.view()->setSelectedModelNodes(selectedNodes);
    }
}

void cut(const SelectionContext &)
{
}


void copy(const SelectionContext &)
{
}

void deleteSelection(const SelectionContext &)
{
}

void toFront(const SelectionContext &selectionState)
{
    if (!selectionState.view())
        return;

    try {
        QmlItemNode node = selectionState.firstSelectedModelNode();
        if (node.isValid()) {
            signed int maximumZ = getMaxZValue(siblingsForNode(node));
            maximumZ++;
            node.setVariantProperty("z", maximumZ);
        }
    } catch (RewritingException &e) { //better save then sorry
        QMessageBox::warning(0, "Error", e.description());
    }
}


void toBack(const SelectionContext &selectionState)
{
    if (!selectionState.view())
        return;
    try {
        QmlItemNode node = selectionState.firstSelectedModelNode();
        if (node.isValid()) {
            signed int minimumZ = getMinZValue(siblingsForNode(node));
            minimumZ--;
            node.setVariantProperty("z", minimumZ);
        }

    } catch (RewritingException &e) { //better save then sorry
        QMessageBox::warning(0, "Error", e.description());
    }
}

void raise(const SelectionContext &selectionState)
{
    if (!selectionState.view())
        return;

    try {
        RewriterTransaction transaction(selectionState.view());
        foreach (ModelNode modelNode, selectionState.selectedModelNodes()) {
            QmlItemNode node = modelNode;
            if (node.isValid()) {
                signed int z  = node.instanceValue("z").toInt();
                z++;
                node.setVariantProperty("z", z);
            }
        }
    } catch (RewritingException &e) { //better save then sorry
         QMessageBox::warning(0, "Error", e.description());
    }
}

void lower(const SelectionContext &selectionState)
{

    if (!selectionState.view())
        return;

    try {
        RewriterTransaction transaction(selectionState.view());
        foreach (ModelNode modelNode, selectionState.selectedModelNodes()) {
            QmlItemNode node = modelNode;
            if (node.isValid()) {
                signed int z  = node.instanceValue("z").toInt();
                z--;
                node.setVariantProperty("z", z);
            }
        }
    } catch (RewritingException &e) { //better save then sorry
        QMessageBox::warning(0, "Error", e.description());
    }
}

void paste(const SelectionContext &)
{
}

void undo(const SelectionContext &)
{
}

void redo(const SelectionContext &)
{
}

void setVisible(const SelectionContext &selectionState)
{
    if (!selectionState.view())
        return;

    try {
        selectionState.selectedModelNodes().first().variantProperty("visible").setValue(selectionState.toggled());
    } catch (RewritingException &e) { //better save then sorry
        QMessageBox::warning(0, "Error", e.description());
    }
}

void setFillWidth(const SelectionContext &selectionState)
{
    if (!selectionState.view()
            || !selectionState.hasSingleSelectedModelNode())
        return;

    try {
        selectionState.firstSelectedModelNode().variantProperty("Layout.fillWidth").setValue(selectionState.toggled());
    } catch (RewritingException &e) { //better save then sorry
        QMessageBox::warning(0, "Error", e.description());
    }
}

void setFillHeight(const SelectionContext &selectionState)
{
    if (!selectionState.view()
            || !selectionState.hasSingleSelectedModelNode())
        return;

    try {
        selectionState.firstSelectedModelNode().variantProperty("Layout.fillHeight").setValue(selectionState.toggled());
    } catch (RewritingException &e) { //better save then sorry
        QMessageBox::warning(0, "Error", e.description());
    }
}

void resetSize(const SelectionContext &selectionState)
{
    if (!selectionState.view())
        return;

    try {
        RewriterTransaction transaction(selectionState.view());
        foreach (ModelNode node, selectionState.selectedModelNodes()) {
            node.removeProperty("width");
            node.removeProperty("height");
        }
    } catch (RewritingException &e) { //better save then sorry
        QMessageBox::warning(0, "Error", e.description());
    }
}

void resetPosition(const SelectionContext &selectionState)
{
    if (!selectionState.view())
        return;

    try {
        RewriterTransaction transaction(selectionState.view());
        foreach (ModelNode node, selectionState.selectedModelNodes()) {
            node.removeProperty("x");
            node.removeProperty("y");
        }
    } catch (RewritingException &e) { //better save then sorry
        QMessageBox::warning(0, "Error", e.description());
    }
}

void goIntoComponent(const SelectionContext &selectionState)
{
    goIntoComponent(selectionState.currentSingleSelectedNode());
}

void setId(const SelectionContext &)
{
}

void resetZ(const SelectionContext &selectionState)
{
    if (!selectionState.view())
        return;

    RewriterTransaction transaction(selectionState.view());
    foreach (ModelNode node, selectionState.selectedModelNodes()) {
        node.removeProperty("z");
    }
}

static inline void backupPropertyAndRemove(ModelNode node, const PropertyName &propertyName)
{
    if (node.hasVariantProperty(propertyName)) {
        node.setAuxiliaryData(auxDataString + propertyName, node.variantProperty(propertyName).value());
        node.removeProperty(propertyName);

    }
    if (node.hasBindingProperty(propertyName)) {
        node.setAuxiliaryData(auxDataString + propertyName, QmlItemNode(node).instanceValue(propertyName));
        node.removeProperty(propertyName);
    }
}


static inline void restoreProperty(ModelNode node, const PropertyName &propertyName)
{
    if (node.hasAuxiliaryData(auxDataString + propertyName))
        node.variantProperty(propertyName).setValue(node.auxiliaryData(auxDataString + propertyName));
}

void anchorsFill(const SelectionContext &selectionState)
{
    if (!selectionState.view())
        return;

    RewriterTransaction transaction(selectionState.view());

    ModelNode modelNode = selectionState.currentSingleSelectedNode();

    QmlItemNode node = modelNode;
    if (node.isValid()) {
        node.anchors().fill();
        backupPropertyAndRemove(modelNode, "x");
        backupPropertyAndRemove(modelNode, "y");
        backupPropertyAndRemove(modelNode, "width");
        backupPropertyAndRemove(modelNode, "height");
    }
}

void anchorsReset(const SelectionContext &selectionState)
{
    if (!selectionState.view())
        return;

    RewriterTransaction transaction(selectionState.view());

    ModelNode modelNode = selectionState.currentSingleSelectedNode();

    QmlItemNode node = modelNode;
    if (node.isValid()) {
        node.anchors().removeAnchors();
        node.anchors().removeMargins();
        restoreProperty(node, "x");
        restoreProperty(node, "y");
        restoreProperty(node, "width");
        restoreProperty(node, "height");
    }
}

static inline void reparentTo(const ModelNode &node, const QmlItemNode &parent)
{

    if (parent.isValid() && node.isValid()) {
        NodeAbstractProperty parentProperty;

        if (parent.hasDefaultProperty())
            parentProperty = parent.nodeAbstractProperty(parent.defaultPropertyName());
        else
            parentProperty = parent.nodeAbstractProperty("data");

        parentProperty.reparentHere(node);
    }
}


bool compareByX(const ModelNode &node1, const ModelNode &node2)
{
        QmlItemNode itemNode1 = QmlItemNode(node1);
        QmlItemNode itemNode2 = QmlItemNode(node2);
        if (itemNode1.isValid() && itemNode2.isValid())
            return itemNode1.instancePosition().x() < itemNode2.instancePosition().x();
        return false;
}

bool compareByY(const ModelNode &node1, const ModelNode &node2)
{
        QmlItemNode itemNode1 = QmlItemNode(node1);
        QmlItemNode itemNode2 = QmlItemNode(node2);
        if (itemNode1.isValid() && itemNode2.isValid())
            return itemNode1.instancePosition().y() < itemNode2.instancePosition().y();
        return false;
}

bool compareByGrid(const ModelNode &node1, const ModelNode &node2)
{
        QmlItemNode itemNode1 = QmlItemNode(node1);
        QmlItemNode itemNode2 = QmlItemNode(node2);
        if (itemNode1.isValid() && itemNode2.isValid()) {
            if ((itemNode1.instancePosition().y() + itemNode1.instanceSize().height())  < itemNode2.instancePosition().y())
                return true;
            if ((itemNode2.instancePosition().y() + itemNode2.instanceSize().height())  < itemNode1.instancePosition().y())
                return false; //first sort y (rows)
            return itemNode1.instancePosition().x() < itemNode2.instancePosition().x();
        }
        return false;
}

static inline QPoint getUpperLeftPosition(const QList<ModelNode> &modelNodeList)
{
    QPoint p(INT_MAX, INT_MAX);
    foreach (ModelNode modelNode, modelNodeList) {
        QmlItemNode itemNode = QmlItemNode(modelNode);
        if (itemNode.isValid()) {
            if (itemNode.instancePosition().x() < p.x())
                p.setX(itemNode.instancePosition().x());
            if (itemNode.instancePosition().y() < p.y())
                p.setY(itemNode.instancePosition().y());
        }
    }
    return p;
}

void layoutRowPositioner(const SelectionContext &selectionState)
{
    if (!selectionState.view())
        return;

    NodeMetaInfo rowMetaInfo = selectionState.view()->model()->metaInfo("QtQuick.Row");

    if (!rowMetaInfo.isValid())
        return;

    QList<ModelNode> modelNodeList = selectionState.selectedModelNodes();

    ModelNode row;
    {
        RewriterTransaction transaction(selectionState.view());

        QmlItemNode parent = QmlItemNode(modelNodeList.first()).instanceParent().toQmlItemNode();
        if (!parent.isValid())
            return;

        qDebug() << parent.modelNode().majorQtQuickVersion();

        row = selectionState.view()->createModelNode("QtQuick.Row", rowMetaInfo.majorVersion(), rowMetaInfo.minorVersion());

        reparentTo(row, parent);
    }

    {
        RewriterTransaction transaction(selectionState.view());

        QPoint pos = getUpperLeftPosition(modelNodeList);
        row.variantProperty("x").setValue(pos.x());
        row.variantProperty("y").setValue(pos.y());

        QList<ModelNode> sortedList = modelNodeList;
        qSort(sortedList.begin(), sortedList.end(), compareByX);

        foreach (ModelNode modelNode, sortedList) {
            reparentTo(modelNode, row);
            modelNode.removeProperty("x");
            modelNode.removeProperty("y");
        }
    }
}

void layoutColumnPositioner(const SelectionContext &selectionState)
{
    if (!selectionState.view())
        return;

    NodeMetaInfo columnMetaInfo = selectionState.view()->model()->metaInfo("QtQuick.Column");

    if (!columnMetaInfo.isValid())
        return;

    QList<ModelNode> modelNodeList = selectionState.selectedModelNodes();

    ModelNode column;
    {
        RewriterTransaction transaction(selectionState.view());

        QmlItemNode parent = QmlItemNode(modelNodeList.first()).instanceParent().toQmlItemNode();
        if (!parent.isValid())
            return;

        column = selectionState.view()->createModelNode("QtQuick.Column", columnMetaInfo.majorVersion(), columnMetaInfo.minorVersion());

        reparentTo(column, parent);
    }

    {
        RewriterTransaction transaction(selectionState.view());

        QPoint pos = getUpperLeftPosition(modelNodeList);
        column.variantProperty("x").setValue(pos.x());
        column.variantProperty("y").setValue(pos.y());

        QList<ModelNode> sortedList = modelNodeList;
        qSort(sortedList.begin(), sortedList.end(), compareByY);

        foreach (ModelNode modelNode, sortedList) {
            reparentTo(modelNode, column);
            modelNode.removeProperty("x");
            modelNode.removeProperty("y");
        }
    }
}

void layoutGridPositioner(const SelectionContext &selectionState)
{
    if (!selectionState.view())
        return;

    NodeMetaInfo gridMetaInfo = selectionState.view()->model()->metaInfo("QtQuick.Grid");

    if (!gridMetaInfo.isValid())
        return;

    QList<ModelNode> modelNodeList = selectionState.selectedModelNodes();

    ModelNode grid;
    {
        RewriterTransaction transaction(selectionState.view());

        QmlItemNode parent = QmlItemNode(modelNodeList.first()).instanceParent().toQmlItemNode();
        if (!parent.isValid())
            return;

        grid = selectionState.view()->createModelNode("QtQuick.Grid", gridMetaInfo.majorVersion(), gridMetaInfo.minorVersion());
        grid.variantProperty("columns").setValue(int(sqrt(double(modelNodeList.count()))));

        reparentTo(grid, parent);
    }

    {
        RewriterTransaction transaction(selectionState.view());

        QPoint pos = getUpperLeftPosition(modelNodeList);
        grid.variantProperty("x").setValue(pos.x());
        grid.variantProperty("y").setValue(pos.y());

        QList<ModelNode> sortedList = modelNodeList;
        qSort(sortedList.begin(), sortedList.end(), compareByGrid);

        foreach (ModelNode modelNode, sortedList) {
            reparentTo(modelNode, grid);
            modelNode.removeProperty("x");
            modelNode.removeProperty("y");
        }
    }
}

void layoutFlowPositioner(const SelectionContext &selectionState)
{
    if (!selectionState.view())
        return;

    NodeMetaInfo flowMetaInfo = selectionState.view()->model()->metaInfo("QtQuick.Flow");

    if (!flowMetaInfo.isValid())
        return;

    QList<ModelNode> modelNodeList = selectionState.selectedModelNodes();

    ModelNode flow;
    {
        RewriterTransaction transaction(selectionState.view());

        QmlItemNode parent = QmlItemNode(modelNodeList.first()).instanceParent().toQmlItemNode();
        if (!parent.isValid())
            return;

        flow = selectionState.view()->createModelNode("QtQuick.Flow", flowMetaInfo.majorVersion(), flowMetaInfo.minorVersion());

        reparentTo(flow, parent);
    }

    {
        RewriterTransaction transaction(selectionState.view());

        QPoint pos = getUpperLeftPosition(modelNodeList);
        flow.variantProperty("x").setValue(pos.x());
        flow.variantProperty("y").setValue(pos.y());

        QList<ModelNode> sortedList = modelNodeList;
        qSort(sortedList.begin(), sortedList.end(), compareByGrid);

        foreach (ModelNode modelNode, sortedList) {
            reparentTo(modelNode, flow);
            modelNode.removeProperty("x");
            modelNode.removeProperty("y");
        }
    }
}

void layoutRowLayout(const SelectionContext &selectionState)
{
    if (!selectionState.view()
            || !selectionState.hasSingleSelectedModelNode())
        return;

    static TypeName rowLayoutType = "QtQuick.Layouts.RowLayout";

    if (!selectionState.view()->model()->hasNodeMetaInfo(rowLayoutType))
        return;

    NodeMetaInfo rowMetaInfo = selectionState.view()->model()->metaInfo(rowLayoutType);

    QList<ModelNode> selectedNodeList = selectionState.selectedModelNodes();
    QmlItemNode qmlItemNode = QmlItemNode(selectedNodeList.first());

    if (qmlItemNode.isValid() && qmlItemNode.hasInstanceParentItem()) {

        ModelNode rowNode;
        {
            RewriterTransaction transaction(selectionState.view());

            QmlItemNode parentNode = qmlItemNode.instanceParentItem();

            rowNode = selectionState.view()->createModelNode(rowLayoutType, rowMetaInfo.majorVersion(), rowMetaInfo.minorVersion());

            reparentTo(rowNode, parentNode);
        }

        {
            RewriterTransaction transaction(selectionState.view());

            QPoint upperLeftPosition = getUpperLeftPosition(selectedNodeList);
            rowNode.variantProperty("x").setValue(upperLeftPosition.x());
            rowNode.variantProperty("y").setValue(upperLeftPosition.y());

            QList<ModelNode> sortedSelectedNodes =  selectedNodeList;
            qSort(sortedSelectedNodes.begin(), sortedSelectedNodes.end(), compareByX);

            foreach (ModelNode selectedNode, sortedSelectedNodes) {
                reparentTo(selectedNode, rowNode);
                selectedNode.removeProperty("x");
                selectedNode.removeProperty("y");
            }
        }
    }
}

void layoutColumnLayout(const SelectionContext &selectionState)
{
    if (!selectionState.view()
            || !selectionState.hasSingleSelectedModelNode())
        return;

    static TypeName columnLayoutType = "QtQuick.Layouts.ColumnLayout";

    if (!selectionState.view()->model()->hasNodeMetaInfo(columnLayoutType))
        return;

    NodeMetaInfo columnMetaInfo = selectionState.view()->model()->metaInfo(columnLayoutType);

    QList<ModelNode> selectedNodeList = selectionState.selectedModelNodes();
    QmlItemNode qmlItemNode = QmlItemNode(selectedNodeList.first());

    if (qmlItemNode.isValid() && qmlItemNode.hasInstanceParentItem()) {

        ModelNode columnNode;
        {
            RewriterTransaction transaction(selectionState.view());

            QmlItemNode parentNode = qmlItemNode.instanceParentItem();

            columnNode = selectionState.view()->createModelNode(columnLayoutType, columnMetaInfo.majorVersion(), columnMetaInfo.minorVersion());

            reparentTo(columnNode, parentNode);
        }

        {
            RewriterTransaction transaction(selectionState.view());

            QPoint upperLeftPosition = getUpperLeftPosition(selectedNodeList);
            columnNode.variantProperty("x").setValue(upperLeftPosition.x());
            columnNode.variantProperty("y").setValue(upperLeftPosition.y());

            QList<ModelNode> sortedSelectedNodes = selectedNodeList;
            qSort(sortedSelectedNodes.begin(), sortedSelectedNodes.end(), compareByY);

            foreach (ModelNode selectedNode, sortedSelectedNodes) {
                reparentTo(selectedNode, columnNode);
                selectedNode.removeProperty("x");
                selectedNode.removeProperty("y");
            }
        }
    }
}

void layoutGridLayout(const SelectionContext &selectionState)
{
    if (!selectionState.view()
            || !selectionState.hasSingleSelectedModelNode())
        return;

    static TypeName gridLayoutType = "QtQuick.Layouts.GridLayout";

    if (!selectionState.view()->model()->hasNodeMetaInfo(gridLayoutType))
        return;

    NodeMetaInfo gridMetaInfo = selectionState.view()->model()->metaInfo(gridLayoutType);

    QList<ModelNode> selectedNodeList = selectionState.selectedModelNodes();
    QmlItemNode qmlItemNode = QmlItemNode(selectedNodeList.first());

    if (qmlItemNode.isValid() && qmlItemNode.hasInstanceParentItem()) {

        ModelNode gridNode;
        {
            RewriterTransaction transaction(selectionState.view());

            QmlItemNode parentNode = qmlItemNode.instanceParentItem();

            gridNode = selectionState.view()->createModelNode(gridLayoutType, gridMetaInfo.majorVersion(), gridMetaInfo.minorVersion());
            gridNode.variantProperty("columns").setValue(int(sqrt(double(selectedNodeList.count()))));

            reparentTo(gridNode, parentNode);
        }

        {
            RewriterTransaction transaction(selectionState.view());

            QPoint upperLeftPosition = getUpperLeftPosition(selectedNodeList);
            gridNode.variantProperty("x").setValue(upperLeftPosition.x());
            gridNode.variantProperty("y") .setValue(upperLeftPosition.y());

            QList<ModelNode> sortedSelectedNodes = selectedNodeList;
            qSort(sortedSelectedNodes.begin(), sortedSelectedNodes.end(), compareByGrid);

            foreach (ModelNode selectedNode, sortedSelectedNodes) {
                reparentTo(selectedNode, gridNode);
                selectedNode.removeProperty("x");
                selectedNode.removeProperty("y");
            }
        }
    }
}

} // namespace Mode

} //QmlDesigner
