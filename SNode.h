//
// Created by minch on 2016-12-17.
//

#ifndef MINIC_COMPILER_SNODE_H
#define MINIC_COMPILER_SNODE_H

typedef struct symbolName{
    int index;
    int length;
}SName;

typedef struct symbolTableNode{
    SName name;
    int type;
    int block;
    int offset;
    int dimen;
    struct symbolTableNode* next;
}SNode;

#endif //MINIC_COMPILER_SNODE_H
